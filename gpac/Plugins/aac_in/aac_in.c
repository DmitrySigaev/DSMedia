/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / AAC reader plugin
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

#include <gpac/m4_author.h>
#include <gpac/m4_network.h>
#include <gpac/m4_decoder.h>

typedef struct
{
	LPNETSERVICE service;

	Bool is_remote;
	
	FILE *stream;
	u32 duration;

	Bool needs_connection;
	u32 pad_bytes;
	Bool es_done, od_done;
	u32 es_status, od_status;
	LPNETCHANNEL es_ch, od_ch;

	unsigned char *es_data;
	u32 es_data_size;

	char *od_data;
	u32 od_data_size;

	SLHeader sl_hdr;

	u32 sample_rate, oti, sr_idx, nb_ch, prof;
	Double start_range, end_range;
	u32 current_time, nb_samp;
	/*file downloader*/
	LPFILEDOWNLOADER dnload;

	Bool is_live;
	char prev_data[1000];
	u32 prev_size;
} AACReader;

typedef struct
{
	Bool is_mp2, no_crc;
	u32 profile, sr_idx, nb_ch, frame_size, hdr_size;
} ADTSHeader;

static Bool AAC_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (NM_CheckExtension(plug, "audio/x-m4a", "aac", "MPEG-4 AAC Music", sExt)) return 1;
	if (NM_CheckExtension(plug, "audio/aac", "aac", "MPEG-4 AAC Music", sExt)) return 1;
	if (NM_CheckExtension(plug, "audio/aacp", "aac", "MPEG-4 AACPlus Music", sExt)) return 1;
	return 0;
}

static Bool aac_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}

static Bool ADTS_SyncFrame(BitStream *bs, Bool is_complete, ADTSHeader *hdr)
{
	u32 val, pos, start_pos;

	start_pos = (u32) BS_GetPosition(bs);
	while (BS_Available(bs)) {
		val = BS_ReadInt(bs, 8);
		if (val!=0xFF) continue;
		val = BS_ReadInt(bs, 4);
		if (val != 0x0F) {
			BS_ReadInt(bs, 4);
			continue;
		}
		hdr->is_mp2 = BS_ReadInt(bs, 1);
		BS_ReadInt(bs, 2);
		hdr->no_crc = BS_ReadInt(bs, 1);
		pos = (u32) BS_GetPosition(bs) - 2;

		hdr->profile = BS_ReadInt(bs, 2);
		if (!hdr->is_mp2) hdr->profile += 1;
		hdr->sr_idx = BS_ReadInt(bs, 4);
		BS_ReadInt(bs, 1);
		hdr->nb_ch = BS_ReadInt(bs, 3);
		BS_ReadInt(bs, 4);
		hdr->frame_size = BS_ReadInt(bs, 13);
		BS_ReadInt(bs, 11);
		BS_ReadInt(bs, 2);
		hdr->hdr_size = 7;
		if (!hdr->no_crc) {
			BS_ReadInt(bs, 16);
			hdr->hdr_size = 9;
		}
		if (hdr->frame_size < hdr->hdr_size) {
			BS_Seek(bs, pos+1);
			continue;
		}
		hdr->frame_size -= hdr->hdr_size;
		if (is_complete && (BS_Available(bs) == hdr->frame_size)) return 1;
		else if (BS_Available(bs) <= hdr->frame_size) break;

		BS_SkipBytes(bs, hdr->frame_size);
		val = BS_ReadInt(bs, 8);
		if (val!=0xFF) {
			BS_Seek(bs, pos+1);
			continue;
		}
		val = BS_ReadInt(bs, 4);
		if (val!=0x0F) {
			BS_ReadInt(bs, 4);
			BS_Seek(bs, pos+1);
			continue;
		}
		BS_Seek(bs, pos+hdr->hdr_size);
		return 1;
	}
	BS_Seek(bs, start_pos);
	return 0;
}

static Bool AAC_ConfigureFromFile(AACReader *read)
{
	Bool sync;
	BitStream *bs;
	ADTSHeader hdr;
	if (!read->stream) return 0;
	bs = NewBitStreamFromFile(read->stream, BS_FILE_READ);

	sync = ADTS_SyncFrame(bs, !read->is_remote, &hdr);
	if (!sync) {
		DeleteBitStream(bs);
		return 0;
	}
	read->nb_ch = hdr.nb_ch;
	read->prof = hdr.profile;
	read->sr_idx = hdr.sr_idx;
	read->oti = hdr.is_mp2 ? read->prof+0x66 : 0x40;
	read->sample_rate = m4a_sample_rates[read->sr_idx];

	read->duration = 0;
	
	if (!read->is_remote) {
		read->duration = 1024;
		BS_SkipBytes(bs, hdr.frame_size);
		while (ADTS_SyncFrame(bs, !read->is_remote, &hdr)) {
			read->duration += 1024;
			BS_SkipBytes(bs, hdr.frame_size);
		}
	}
	DeleteBitStream(bs);
	fseek(read->stream, 0, SEEK_SET);
	return 1;
}

static void AAC_OnLiveData(AACReader *read, char *data, u32 data_size)
{
	u32 pos;
	Bool sync;
	BitStream *bs;
	ADTSHeader hdr;
	
	if (!read->es_data) {
		read->es_data = malloc(sizeof(char)*data_size);
		read->es_data_size = data_size;
		memcpy(read->es_data, data, sizeof(char)*data_size);
	} else {
		read->es_data = realloc(read->es_data, sizeof(char)*(read->es_data_size+data_size) );
		memcpy(read->es_data + read->es_data_size, data, sizeof(char)*data_size);
		read->es_data_size += data_size;
	}

	if (read->needs_connection) {
		read->needs_connection = 0;
		bs = NewBitStream(read->es_data, read->es_data_size, BS_READ);
		sync = ADTS_SyncFrame(bs, 0, &hdr);
		DeleteBitStream(bs);
		if (!sync) return;
		read->nb_ch = hdr.nb_ch;
		read->prof = hdr.profile;
		read->sr_idx = hdr.sr_idx;
		read->oti = hdr.is_mp2 ? read->prof+0x66 : 0x40;
		read->sample_rate = m4a_sample_rates[read->sr_idx];
		read->is_live = 1;
		memset(&read->sl_hdr, 0, sizeof(SLHeader));
		NM_OnConnect(read->service, NULL, M4OK);
	}
	if (!read->es_ch) return;

	/*need a full adts header*/
	if (read->es_data_size<=7) return;

	bs = NewBitStream(read->es_data, read->es_data_size, BS_READ);
	hdr.frame_size = pos = 0;
	while (ADTS_SyncFrame(bs, 0, &hdr)) {
		pos = (u32) BS_GetPosition(bs);
		read->sl_hdr.accessUnitStartFlag = 1;
		read->sl_hdr.accessUnitEndFlag = 1;
		read->sl_hdr.AU_sequenceNumber++;
		read->sl_hdr.compositionTimeStampFlag = 1;
		read->sl_hdr.compositionTimeStamp += 1024;
		NM_OnSLPRecieved(read->service, read->es_ch, read->es_data + pos, hdr.frame_size, &read->sl_hdr, M4OK);
		BS_SkipBytes(bs, hdr.frame_size);
	}

	pos = (u32) BS_GetPosition(bs);
	DeleteBitStream(bs);

	if (pos) {
		char *d;
		read->es_data_size -= pos;
		d = malloc(sizeof(char) * read->es_data_size);
		memcpy(d, read->es_data + pos, sizeof(char) * read->es_data_size);
		free(read->es_data);
		read->es_data = d;
	}
}

void AAC_OnStatus(void *cbk)
{
	M4Err e;
	u32 bytes_done, net_status;
	AACReader *read = (AACReader *) cbk;
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

void AAC_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	const char *szCache;
	u32 total_size, bytes_done, net_status;
	AACReader *read = (AACReader *) cbk;

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
			AAC_OnLiveData(read, data, data_size);
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
				if (!AAC_ConfigureFromFile(read)) {
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

void aac_download_file(NetClientPlugin *plug, char *url)
{
	M4Err e;
	AACReader *read = (AACReader*) plug->priv;

	read->needs_connection = 1;

	e = NM_FetchFile(read->service, url, 0, AAC_OnStatus, AAC_OnData, read, &read->dnload);
	if (e) {
		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, e);
	}
	/*service confirm is done once fetched*/
}


static M4Err AAC_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char szURL[2048];
	char *ext;
	M4Err reply;
	AACReader *read = plug->priv;
	read->service = serv;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	strcpy(szURL, url);
	ext = strrchr(szURL, '#');
	if (ext) ext[0] = 0;

	/*remote fetch*/
	read->is_remote = !aac_is_local(szURL);
	if (read->is_remote) {
		aac_download_file(plug, (char *) szURL);
		return M4OK;
	}

	reply = M4OK;
	read->stream = fopen(szURL, "rb");
	if (!read->stream) {
		reply = M4URLNotFound;
	} else if (!AAC_ConfigureFromFile(read)) {
		fclose(read->stream);
		read->stream = NULL;
		reply = M4UnsupportedURL;
	}
	NM_OnConnect(serv, NULL, reply);
	return M4OK;
}

static M4Err AAC_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	AACReader *read = plug->priv;
	if (read->stream) fclose(read->stream);
	read->stream = NULL;
	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	if (read->es_data) free(read->es_data);
	read->es_data = NULL;
	NM_OnDisconnect(read->service, NULL, M4OK);
	return M4OK;
}

static ESDescriptor *AAC_GetESD(AACReader *read)
{
	BitStream *dsi;
	ESDescriptor *esd;
	esd = OD_NewESDescriptor(0);
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = read->oti;
	esd->ESID = 2;
	esd->OCRESID = 1;
	esd->slConfig->timestampResolution = read->sample_rate;
	if (read->is_live) esd->slConfig->useAccessUnitEndFlag = esd->slConfig->useAccessUnitStartFlag = 1;
	dsi = NewBitStream(NULL, 0, BS_WRITE);

	/*write as regular AAC*/
	BS_WriteInt(dsi, read->prof, 5);
	BS_WriteInt(dsi, read->sr_idx, 4);
	BS_WriteInt(dsi, read->nb_ch, 4);
	BS_Align(dsi);

	/*write as SBR AAC*/
	if (0) {
		u32 i, sbr_sr_idx;
		sbr_sr_idx = read->sr_idx;
		for (i=0; i<16; i++) {
			if (m4a_sample_rates[i] == (u32) 2*read->sample_rate) {
				sbr_sr_idx = i;
				break;
			}
		}
		BS_WriteInt(dsi, 0x2b7, 11);
		BS_WriteInt(dsi, 5, 5);
		BS_WriteInt(dsi, 1, 1);
		BS_WriteInt(dsi, sbr_sr_idx, 4);
	}

	BS_Align(dsi);
	BS_GetContent(dsi, (unsigned char **)&esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(dsi);
	return esd;
}

static M4Err AAC_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	AACReader *read = plug->priv;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	/*since we don't handle multitrack in aac, we don't need to check sub_url, only use expected type*/

	od->objectDescriptorID = 1;
	/*audio object*/
	if (expect_type==NM_OD_AUDIO) {
		esd = AAC_GetESD(read);
		esd->OCRESID = 0;
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

static M4Err AAC_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID;
	M4Err e;
	AACReader *read = plug->priv;

	e = M4ServiceError;
	if ((read->es_ch==channel) || (read->od_ch==channel)) goto exit;

	e = M4ChannelNotFound;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ES_ID);
	}
	/*URL setup*/
	else if (!read->es_ch && AAC_CanHandleURL(plug, url)) ES_ID = 2;

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

static M4Err AAC_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	AACReader *read = plug->priv;

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

static M4Err AAC_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	AACReader *read = plug->priv;

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
			if (com->buffer.max<1000) com->buffer.max = 1000;
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
				AAC_ConfigureFromFile(read);
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


static M4Err AAC_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	u32 pos, start_from;
	Bool sync;
	BitStream *bs;
	ADTSHeader hdr;
	AACReader *read = plug->priv;

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

			esd = AAC_GetESD(read);
			/*we signal start/end of units in live mode*/
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
		bs = NewBitStreamFromFile(read->stream, BS_FILE_READ);
		*is_new_data = 1;

fetch_next:
		pos = ftell(read->stream);
		sync = ADTS_SyncFrame(bs, !read->is_remote, &hdr);
		if (!sync) {
			DeleteBitStream(bs);
			if (!read->dnload) {
				*out_reception_status = M4EOF;
				read->es_done = 1;
			} else {
				fseek(read->stream, pos, SEEK_SET);
				*out_reception_status = M4OK;
			}
			return M4OK;
		}

		if (!hdr.frame_size) {
			DeleteBitStream(bs);
			*out_reception_status = M4EOF;
			read->es_done = 1;
			return M4OK;
		}
		read->es_data_size = hdr.frame_size;
		read->nb_samp = 1024;
		/*we're seeking*/
		if (read->start_range && read->duration) {
			start_from = (u32) (read->start_range * read->sample_rate);
			if (read->current_time + read->nb_samp < start_from) {
				read->current_time += read->nb_samp;
				goto fetch_next;
			} else {
				read->start_range = 0;
			}
		}
		
		read->sl_hdr.compositionTimeStamp = read->current_time;

		read->es_data = malloc(sizeof(char) * (read->es_data_size+read->pad_bytes));
		BS_ReadData(bs, read->es_data, read->es_data_size);
		if (read->pad_bytes) memset(read->es_data + read->es_data_size, 0, sizeof(char) * read->pad_bytes);
		DeleteBitStream(bs);
	}
	*out_sl_hdr = read->sl_hdr;
	*out_data_ptr = read->es_data;
	*out_data_size = read->es_data_size;
	return M4OK;
}

static M4Err AAC_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	AACReader *read = plug->priv;

	if (read->es_ch == channel) {
		if (!read->es_data) return M4BadParam;
		free(read->es_data);
		read->es_data = NULL;
		read->current_time += read->nb_samp;
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

NetClientPlugin *AAC_LoadPlugin()
{
	AACReader *reader;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC AAC Reader", "gpac distribution", 0)

	plug->CanHandleURL = AAC_CanHandleURL;
	plug->ConnectService = AAC_ConnectService;
	plug->CloseService = AAC_CloseService;
	plug->Get_MPEG4_IOD = AAC_Get_MPEG4_IOD;
	plug->ConnectChannel = AAC_ConnectChannel;
	plug->DisconnectChannel = AAC_DisconnectChannel;
	plug->ServiceCommand = AAC_ServiceCommand;
	/*we do support pull mode*/
	plug->ChannelGetSLP = AAC_ChannelGetSLP;
	plug->ChannelReleaseSLP = AAC_ChannelReleaseSLP;

	reader = malloc(sizeof(AACReader));
	memset(reader, 0, sizeof(AACReader));
	plug->priv = reader;
	return plug;
}

void AAC_Delete(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	AACReader *read = plug->priv;
	free(read);
	free(plug);
}

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
#ifdef M4_HAS_FAAD
	if (InterfaceType == M4MEDIADECODERINTERFACE) return 1;
#endif
	return 0;
}

#ifdef M4_HAS_FAAD
BaseDecoder *NewFAADDec();
void DeleteFAADDec(BaseDecoder *ifcg);
#endif

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return AAC_LoadPlugin();
#ifdef M4_HAS_FAAD
	if (InterfaceType == M4MEDIADECODERINTERFACE) return NewFAADDec();
#endif
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
#ifdef M4_HAS_FAAD
	case M4MEDIADECODERINTERFACE:
		DeleteFAADDec(ifce);
		break;
#endif
	case M4STREAMINGCLIENT:
		AAC_Delete(ptr);
		break;
	}
}
