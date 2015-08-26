/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / MP3 reader plugin
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *		
 */

#include <gpac/m4_network.h>
#include <gpac/m4_author.h>
#include <gpac/m4_decoder.h>


typedef struct
{
	LPNETSERVICE service;

	u32 needs_connection;
	Bool is_remote;
	
	FILE *stream;
	u32 duration;

	u32 pad_bytes;
	Bool es_done, od_done;
	LPNETCHANNEL es_ch, od_ch;

	char *es_data;
	u32 es_data_size;

	char *od_data;
	u32 od_data_size;


	SLHeader sl_hdr;

	u32 sample_rate, oti;
	Double start_range, end_range;
	u32 current_time, nb_samp;
	/*file downloader*/
	LPFILEDOWNLOADER dnload;

	Bool is_live;
	char prev_data[1000];
	u32 prev_size;
} MP3Reader;


static Bool MP3_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (NM_CheckExtension(plug, "audio/mpeg", "mp3", "MP3 Music", sExt)) return 1;
	return 0;
}

static Bool mp3_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}


static Bool MP3_ConfigureFromFile(MP3Reader *read)
{
	u32 hdr, size, pos;
	if (!read->stream) return 0;

	hdr = MP3_GetNextHeader(read->stream);
	if (!hdr) return 0;
	read->sample_rate = MP3_GetSamplingRate(hdr);
	read->oti = MP3_GetObjectTypeIndication(hdr);
	fseek(read->stream, 0, SEEK_SET);
	if (!read->oti) return 0;

	/*we don't have the full file...*/
	if (read->is_remote) return	1;

	read->duration = MP3_GetSamplesPerFrame(hdr);
	size = MP3_GetFrameSize(hdr);
	pos = ftell(read->stream);
	fseek(read->stream, pos + size - 4, SEEK_SET);
	while (1) {
		hdr = MP3_GetNextHeader(read->stream);
		if (!hdr) break;
		read->duration += MP3_GetSamplesPerFrame(hdr);
		size = MP3_GetFrameSize(hdr);
		pos = ftell(read->stream);
		fseek(read->stream, pos + size - 4, SEEK_SET);
	}
	fseek(read->stream, 0, SEEK_SET);
	return 1;
}

static void MP3_OnLiveData(MP3Reader *read, char *data, u32 data_size)
{
	u32 hdr, size, pos;

	if (read->needs_connection) {
		hdr = MP3_GetNextHeaderMem(data, data_size, &pos);
		if (!hdr) return;
		read->sample_rate = MP3_GetSamplingRate(hdr);
		read->oti = MP3_GetObjectTypeIndication(hdr);
		read->is_live = 1;
		memset(&read->sl_hdr, 0, sizeof(SLHeader));

		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, M4OK);
	}
	if (!read->es_data) {
		read->es_data = malloc(sizeof(char)*data_size);
		read->es_data_size = data_size;
		memcpy(read->es_data, data, sizeof(char)*data_size);
	} else {
		read->es_data = realloc(read->es_data, sizeof(char)*(read->es_data_size+data_size) );
		memcpy(read->es_data + read->es_data_size, data, sizeof(char)*data_size);
		read->es_data_size += data_size;
	}
	if (!read->es_ch) return;

	data = read->es_data;
	data_size = read->es_data_size;

	while (1) {
		hdr = MP3_GetNextHeaderMem(data, data_size, &pos);

		if (hdr) size = MP3_GetFrameSize(hdr);

		/*not enough data, copy over*/
		if (!hdr || (pos+size>data_size)) {
			char *d = malloc(sizeof(char) * data_size);
			memcpy(d, data, sizeof(char) * data_size);
			free(read->es_data);
			read->es_data = d;
			read->es_data_size = data_size;
			return;
		}

		read->sl_hdr.accessUnitStartFlag = 1;
		read->sl_hdr.accessUnitEndFlag = 1;
		read->sl_hdr.AU_sequenceNumber++;
		read->sl_hdr.compositionTimeStampFlag = 1;
		read->sl_hdr.compositionTimeStamp += MP3_GetSamplesPerFrame(hdr);
		NM_OnSLPRecieved(read->service, read->es_ch, data + pos, size, &read->sl_hdr, M4OK);
		data += pos + size;
		assert(data_size>=pos+size);
		data_size -= pos+size;
	}
}

void MP3_OnStatus(void *cbk)
{
	M4Err e;
	u32 bytes_done, net_status;
	MP3Reader *read = (MP3Reader *) cbk;
	e = NM_GetDownloaderStats(read->dnload, NULL, &bytes_done, NULL, &net_status);

	if (!bytes_done) {
		switch (net_status) {
		case DL_WaitingForAck:
			NM_OnMessage(read->service, M4OK, "Connecting...");
			return;
		case DL_Connected:
			NM_OnMessage(read->service, M4OK, "Connected");
			return;
		}
	}
	else if ((e == M4EOF) && read->stream) {
		read->is_remote = 0;
		return;
	}
}

void MP3_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	const char *szCache;
	u32 total_size, bytes_done, net_status;
	MP3Reader *read = (MP3Reader *) cbk;

	e = NM_GetDownloaderStats(read->dnload, &total_size, &bytes_done, &bytes_per_sec, &net_status);
	
	if ((e == M4EOF) && read->stream) {
		read->is_remote = 0;
		return;
	}
	else if (e >= M4OK) {
		/*notify some connection / ...*/
		if (total_size) {
			perc = (Float) (100 * bytes_done) / (Float) total_size;
			sprintf(sMsg, "Download %.2f %% (%.2f kBps)", perc, bytes_per_sec/1024);
			NM_OnMessage(read->service, M4OK, sMsg);
		} 
		/*looks like a live stream*/
		else {
			MP3_OnLiveData(read, data, data_size);
			return;
		}

		if (read->stream) return;

		/*open service*/
		szCache = NM_GetCacheFileName(read->dnload);
		if (!szCache) e = M4InvalidPlugin;
		else {
			read->stream = fopen((char *) szCache, "rb");
			if (!read->stream) e = M4ServiceError;
			else {
				e = M4OK;
				/*if full file at once (in cache) parse duration*/
				if (bytes_done==total_size) read->is_remote = 0;
				/*not enough data*/
				if (!MP3_ConfigureFromFile(read)) {
					/*bad data - there's likely some ID3 around...*/
					if (bytes_done>10*1024) {
						e = M4CorruptedData;
					} else {
						fclose(read->stream);
						read->stream = NULL;
						return;
					}
				}
			}
		}
	}

	/*OK confirm*/
	if (read->needs_connection) {
		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, e);
	}
}

void mp3_download_file(NetClientPlugin *plug, char *url)
{
	M4Err e;
	MP3Reader *read = (MP3Reader*) plug->priv;

	read->needs_connection = 1;

	e = NM_FetchFile(read->service, url, 0, MP3_OnStatus, MP3_OnData, read, &read->dnload);
	if (e) {
		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, e);
	}
	/*service confirm is done once fetched*/
}


static M4Err MP3_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char szURL[2048];
	char *ext;
	M4Err reply;
	MP3Reader *read = plug->priv;
	read->service = serv;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	strcpy(szURL, url);
	ext = strrchr(szURL, '#');
	if (ext) ext[0] = 0;

	/*remote fetch*/
	read->is_remote = !mp3_is_local(szURL);
	if (read->is_remote) {
		mp3_download_file(plug, (char *) szURL);
		return M4OK;
	}

	reply = M4OK;
	read->stream = fopen(szURL, "rb");
	if (!read->stream) {
		reply = M4URLNotFound;
	} else if (!MP3_ConfigureFromFile(read)) {
		fclose(read->stream);
		read->stream = NULL;
		reply = M4UnsupportedURL;
	}
	NM_OnConnect(serv, NULL, reply);
	return M4OK;
}

static M4Err MP3_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	MP3Reader *read = plug->priv;
	if (read->stream) fclose(read->stream);
	read->stream = NULL;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	if (read->es_data) free(read->es_data);
	read->es_data = NULL;
	NM_OnDisconnect(read->service, NULL, M4OK);
	return M4OK;
}

static M4Err MP3_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	MP3Reader *read = plug->priv;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	od->objectDescriptorID = 1;
	/*since we don't handle multitrack in mp3, we don't need to check sub_url, only use expected type*/

	/*audio object*/
	if (expect_type==NM_OD_AUDIO) {
		esd = OD_NewESDescriptor(0);
		esd->slConfig->timestampResolution = read->sample_rate;
		esd->decoderConfig->streamType = M4ST_AUDIO;
		esd->decoderConfig->objectTypeIndication = read->oti;
		esd->ESID = 3;
		ChainAddEntry(od->ESDescriptors, esd);
		e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor **)&od);
		return e;
	}
	/*inline scene*/
	/*OD ESD*/
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = GPAC_STATIC_OD_OTI;
	esd->ESID = 1;
	ChainAddEntry(od->ESDescriptors, esd);
	e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **)&od);
	return e;
}

static M4Err MP3_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID;
	M4Err e;
	MP3Reader *read = plug->priv;

	e = M4ServiceError;
	if ((read->es_ch==channel) || (read->od_ch==channel)) goto exit;

	e = M4ChannelNotFound;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ES_ID);
	}
	/*URL setup*/
	else if (!read->es_ch && MP3_CanHandleURL(plug, url)) ES_ID = 2;

	switch (ES_ID) {
	case 1:
		read->od_ch = channel;
		e = M4OK;
		break;
	case 2:
		read->es_ch = channel;
		e = M4OK;
		break;
	}

exit:
	NM_OnConnect(read->service, channel, e);
	return e;
}

static M4Err MP3_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	MP3Reader *read = plug->priv;
	M4Err e = M4ChannelNotFound;
	if (read->es_ch == channel) {
		read->es_ch = NULL;
		if (read->es_data) free(read->es_data);
		read->es_data = NULL;
		e = M4OK;
	} else if (read->od_ch == channel) {
		read->od_ch = NULL;
		e = M4OK;
	}
	NM_OnDisconnect(read->service, channel, e);
	return M4OK;
}

static M4Err MP3_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	MP3Reader *read = plug->priv;

	if (!com->base.on_channel) return M4NotSupported;
	switch (com->command_type) {
	case CHAN_SET_PULL:
		if ((read->es_ch == com->base.on_channel) && read->is_live) return M4NotSupported;
		return M4OK;
	case CHAN_INTERACTIVE:
		if ((read->es_ch == com->base.on_channel) && read->is_live) return M4NotSupported;
		return M4OK;
	case CHAN_BUFFER:
		if ((read->es_ch == com->base.on_channel) && read->is_live) {
			if (com->buffer.max<2000) com->buffer.max = 2000;
			com->buffer.min = com->buffer.max/2;
		}
		return M4OK;
	case CHAN_SET_PADDING:
		read->pad_bytes = com->pad.padding_bytes;
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = read->duration;
		com->duration.duration /= read->sample_rate;
		return M4OK;
	case CHAN_PLAY:
		read->start_range = com->play.start_range;
		read->end_range = com->play.end_range;
		read->current_time = 0;
		if (read->stream) fseek(read->stream, 0, SEEK_SET);

		if (read->es_ch == com->base.on_channel) { 
			read->es_done = 0; 
			/*PLAY after complete download, estimate duration*/
			if (!read->is_remote && !read->duration) {
				MP3_ConfigureFromFile(read);
				if (read->duration) {
					NetworkCommand rcfg;
					rcfg.base.on_channel = read->es_ch;
					rcfg.base.command_type = CHAN_DURATION;
					rcfg.duration.duration = read->duration;
					rcfg.duration.duration /= read->sample_rate;
					NM_OnCommand(read->service, &rcfg, M4OK);
				}
			}
		}
		else if (read->od_ch == com->base.on_channel) { read->od_done = 0; }
		return M4OK;
	case CHAN_STOP:
		return M4OK;
	default:
		return M4OK;
	}
}


static M4Err MP3_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	u32 pos, hdr, start_from;
	MP3Reader *read = plug->priv;

	*out_reception_status = M4OK;
	*sl_compressed = 0;
	*is_new_data = 0;

	memset(&read->sl_hdr, 0, sizeof(SLHeader));
	read->sl_hdr.randomAccessPointFlag = 1;
	read->sl_hdr.compositionTimeStampFlag = 1;

	if (read->od_ch == channel) {
		LPODCODEC codec;
		ObjectDescriptor *od;
		ObjectDescriptorUpdate *odU;
		ESDescriptor *esd;
		if (read->od_done) {
			*out_reception_status = M4EOF;
			return M4OK;
		}
		read->sl_hdr.compositionTimeStamp = (u64) (read->start_range * 1000);
		*out_sl_hdr = read->sl_hdr;
		if (!read->od_data) {
			*is_new_data = 1;
			odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
			od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
			od->objectDescriptorID = ISMA_AUDIO_OD_ID;
			esd = OD_NewESDescriptor(0);
			esd->slConfig->timestampResolution = read->sample_rate;
			/*we signal start/end of units in live mode*/
			if (read->is_live) esd->slConfig->useAccessUnitEndFlag = esd->slConfig->useAccessUnitStartFlag = 1;
			esd->slConfig->useTimestampsFlag = 1;

			esd->ESID = 2;
			esd->OCRESID = 1;
			esd->decoderConfig->streamType = M4ST_AUDIO;
			esd->decoderConfig->objectTypeIndication = read->oti;
			ChainAddEntry(od->ESDescriptors, esd);
			ChainAddEntry(odU->objectDescriptors, od);
			codec = OD_NewCodec(OD_WRITE);
			OD_AddCommand(codec, (ODCommand *)odU);
			OD_EncodeAU(codec);
			OD_GetEncodedAU(codec, &read->od_data, &read->od_data_size);
			OD_DeleteCodec(codec);
		}
		*out_data_ptr = read->od_data;
		*out_data_size = read->od_data_size;
		return M4OK;
	}

	if (read->es_ch != channel) return M4ChannelNotFound;

	/*fetching es data*/
	if (read->es_done) {
		*out_reception_status = M4EOF;
		return M4OK;
	}
	if (!read->es_data) {
		if (!read->stream) {
			*out_data_ptr = NULL;
			*out_data_size = 0;
			return M4OK;
		}
		*is_new_data = 1;

		pos = ftell(read->stream);
		hdr = MP3_GetNextHeader(read->stream);
		if (!hdr) {
			if (!read->dnload) {
				*out_reception_status = M4EOF;
				read->es_done = 1;
			} else {
				fseek(read->stream, pos, SEEK_SET);
				*out_reception_status = M4OK;
			}
			return M4OK;
		}
		read->es_data_size = MP3_GetFrameSize(hdr);
		if (!read->es_data_size) {
			*out_reception_status = M4EOF;
			read->es_done = 1;
			return M4OK;
		}

		read->current_time += MP3_GetSamplesPerFrame(hdr);

		/*we're seeking*/
		if (read->start_range && read->duration) {
			read->current_time = 0;
			start_from = (u32) (read->start_range * read->sample_rate);
			fseek(read->stream, 0, SEEK_SET);
			while (read->current_time<start_from) {
				hdr = MP3_GetNextHeader(read->stream);
				if (!hdr) {
					read->start_range = 0;
					*out_reception_status = M4ServiceError;
					return M4OK;
				}
				read->current_time += MP3_GetSamplesPerFrame(hdr);
				read->es_data_size = MP3_GetFrameSize(hdr);
				fseek(read->stream, read->es_data_size-4, SEEK_CUR);
			}
			read->start_range = 0;
		}

		read->sl_hdr.compositionTimeStamp = read->current_time;

		read->es_data = malloc(sizeof(char) * (read->es_data_size+read->pad_bytes));
		read->es_data[0] = (hdr >> 24) & 0xFF;
		read->es_data[1] = (hdr >> 16) & 0xFF;
		read->es_data[2] = (hdr >> 8) & 0xFF;
		read->es_data[3] = (hdr ) & 0xFF;
		/*end of file*/
		if (fread(&read->es_data[4], 1, read->es_data_size - 4, read->stream) != read->es_data_size-4) {
			free(read->es_data);
			read->es_data = NULL;
			if (read->is_remote) {
				fseek(read->stream, pos, SEEK_SET);
				*out_reception_status = M4OK;
			} else {
				*out_reception_status = M4EOF;
			}
			return M4OK;
		}
		if (read->pad_bytes) memset(read->es_data + read->es_data_size, 0, sizeof(char) * read->pad_bytes);
	}
	*out_sl_hdr = read->sl_hdr;
	*out_data_ptr = read->es_data;
	*out_data_size = read->es_data_size;
	return M4OK;
}

static M4Err MP3_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	MP3Reader *read = plug->priv;

	if (read->es_ch == channel) {
		if (!read->es_data) return M4BadParam;
		free(read->es_data);
		read->es_data = NULL;
		return M4OK;
	}
	if (read->od_ch == channel) {
		if (!read->od_data) return M4BadParam;
		free(read->od_data);
		read->od_data = NULL;
		read->od_done = 1;
		return M4OK;
	}
	return M4OK;
}

NetClientPlugin *MP3_LoadPlugin()
{
	MP3Reader *reader;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC MP3 Reader", "gpac distribution", 0)

	plug->CanHandleURL = MP3_CanHandleURL;
	plug->ConnectService = MP3_ConnectService;
	plug->CloseService = MP3_CloseService;
	plug->Get_MPEG4_IOD = MP3_Get_MPEG4_IOD;
	plug->ConnectChannel = MP3_ConnectChannel;
	plug->DisconnectChannel = MP3_DisconnectChannel;
	plug->ServiceCommand = MP3_ServiceCommand;
	/*we do support pull mode*/
	plug->ChannelGetSLP = MP3_ChannelGetSLP;
	plug->ChannelReleaseSLP = MP3_ChannelReleaseSLP;

	reader = malloc(sizeof(MP3Reader));
	memset(reader, 0, sizeof(MP3Reader));
	plug->priv = reader;
	return plug;
}

void MP3_Delete(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	MP3Reader *read = plug->priv;
	free(read);
	free(plug);
}


#ifdef M4_HAS_MAD
BaseDecoder *NewMADDec();
void DeleteMADDec(BaseDecoder *ifcg);
#endif

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
#ifdef M4_HAS_MAD
	if (InterfaceType == M4MEDIADECODERINTERFACE) return 1;
#endif
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return MP3_LoadPlugin();
#ifdef M4_HAS_MAD
	if (InterfaceType == M4MEDIADECODERINTERFACE) return NewMADDec();
#endif
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
#ifdef M4_HAS_MAD
	case M4MEDIADECODERINTERFACE:
		DeleteMADDec(ifce);
		break;
#endif
	case M4STREAMINGCLIENT:
		MP3_Delete(ptr);
		break;
	}
}
