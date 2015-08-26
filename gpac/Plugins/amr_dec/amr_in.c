/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / AMR reader plugin
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

	Bool is_remote, is_wb;
	u32 start_offset;

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

	Double start_range, end_range;
	u32 current_time, nb_samp;
	/*file downloader*/
	LPFILEDOWNLOADER dnload;

	//Bool is_live;
} AMRReader;

static const u32 amr_nb_frame_size[16] = {
	12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};

static const u32 amr_wb_frame_size[16] = {
	17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 
};

static Bool AMR_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (NM_CheckExtension(plug, "audio/amr", "amr", "AMR Speech Data", sExt)) return 1;
	return 0;
}

static Bool amr_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}


static Bool AMR_ConfigureFromFile(AMRReader *read)
{
	char magic[20];

	if (!read->stream) return 0;
	read->is_wb = 0;
	read->start_offset = 6;
	fread(magic, 1, 20, read->stream);

	if (!strnicmp(magic, "#!AMR\n", 6)) fseek(read->stream, 6, SEEK_SET);
	else if (!strnicmp(magic, "#!AMR_MC1.0\n", 12)) return 0;
	else if (!strnicmp(magic, "#!AMR-WB\n", 9)) {
		read->is_wb = 1;
		read->start_offset = 9;
		fseek(read->stream, 9, SEEK_SET);
	}
	else if (!strnicmp(magic, "#!AMR-WB_MC1.0\n", 15)) return 0;
	else return 0;


	read->duration = 0;
	
	if (!read->is_remote) {
		u32 size;
		while (!feof(read->stream)) {
			u8 ft = fgetc(read->stream);
			ft = (ft >> 3) & 0x0F;
			size = read->is_wb ? amr_wb_frame_size[ft] : amr_nb_frame_size[ft];
			if (size) fseek(read->stream, size, SEEK_CUR);
			read->duration += read->is_wb ? 320 : 160;
		}
	}
	fseek(read->stream, read->start_offset, SEEK_SET);
	return 1;
}

void AMR_OnStatus(void *cbk)
{
	M4Err e;
	u32 bytes_done, net_status;
	AMRReader *read = (AMRReader *) cbk;
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

void AMR_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	const char *szCache;
	u32 total_size, bytes_done, net_status;
	AMRReader *read = (AMRReader *) cbk;

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
				if (!AMR_ConfigureFromFile(read)) {
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

void amr_download_file(NetClientPlugin *plug, char *url)
{
	M4Err e;
	AMRReader *read = (AMRReader*) plug->priv;

	read->needs_connection = 1;

	e = NM_FetchFile(read->service, url, 0, AMR_OnStatus, AMR_OnData, read, &read->dnload);
	if (e) {
		read->needs_connection = 0;
		NM_OnConnect(read->service, NULL, e);
	}
	/*service confirm is done once fetched*/
}


static M4Err AMR_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char szURL[2048];
	char *ext;
	M4Err reply;
	AMRReader *read = plug->priv;
	read->service = serv;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	strcpy(szURL, url);
	ext = strrchr(szURL, '#');
	if (ext) ext[0] = 0;

	/*remote fetch*/
	read->is_remote = !amr_is_local(szURL);
	if (read->is_remote) {
		amr_download_file(plug, (char *) szURL);
		return M4OK;
	}

	reply = M4OK;
	read->stream = fopen(szURL, "rb");
	if (!read->stream) {
		reply = M4URLNotFound;
	} else if (!AMR_ConfigureFromFile(read)) {
		fclose(read->stream);
		read->stream = NULL;
		reply = M4UnsupportedURL;
	}
	NM_OnConnect(serv, NULL, reply);
	return M4OK;
}

static M4Err AMR_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	AMRReader *read = plug->priv;
	if (read->stream) fclose(read->stream);
	read->stream = NULL;
	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	if (read->es_data) free(read->es_data);
	read->es_data = NULL;
	NM_OnDisconnect(read->service, NULL, M4OK);
	return M4OK;
}

static ESDescriptor *AMR_GetESD(AMRReader *read)
{
	BitStream *dsi;
	ESDescriptor *esd;
	esd = OD_NewESDescriptor(0);
	esd->decoderConfig->streamType = M4ST_AUDIO;
	esd->decoderConfig->objectTypeIndication = 0x80;
	esd->ESID = 2;
	esd->OCRESID = 1;
	esd->slConfig->timestampResolution = read->is_wb ? 16000 : 8000;
	/*all packets are complete AUs*/
	esd->slConfig->useAccessUnitEndFlag = esd->slConfig->useAccessUnitStartFlag = 0;
	dsi = NewBitStream(NULL, 0, BS_WRITE);
	if (read->is_wb)
		BS_WriteInt(dsi, FOUR_CHAR_INT('s', 'a', 'w', 'b'), 32);
	else 
		BS_WriteInt(dsi, FOUR_CHAR_INT('a', 'm', 'r', ' '), 32);
	BS_GetContent(dsi, (unsigned char **) & esd->decoderConfig->decoderSpecificInfo->data, & esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(dsi);
	return esd;
}

static M4Err AMR_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	AMRReader *read = plug->priv;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	/*since we don't handle multitrack in aac, we don't need to check sub_url, only use expected type*/

	od->objectDescriptorID = 1;
	/*audio object*/
	if (expect_type==NM_OD_AUDIO) {
		esd = AMR_GetESD(read);
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

static M4Err AMR_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID;
	M4Err e;
	AMRReader *read = plug->priv;

	e = M4ServiceError;
	if ((read->es_ch==channel) || (read->od_ch==channel)) goto exit;

	e = M4ChannelNotFound;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ES_ID);
	}
	/*URL setup*/
	else if (!read->es_ch && AMR_CanHandleURL(plug, url)) ES_ID = 2;

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

static M4Err AMR_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	AMRReader *read = plug->priv;

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

static M4Err AMR_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	AMRReader *read = plug->priv;

	if (!com->base.on_channel) return M4NotSupported;
	switch (com->command_type) {
	case CHAN_SET_PULL:
		return M4OK;
	case CHAN_INTERACTIVE:
		return M4OK;
	case CHAN_BUFFER:
		return M4OK;
	case CHAN_SET_PADDING:
		read->pad_bytes = com->pad.padding_bytes;
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = read->duration;
		com->duration.duration /= read->is_wb ? 16000 : 8000;
		return M4OK;
	case CHAN_PLAY:
		read->start_range = com->play.start_range;
		read->end_range = com->play.end_range;
		read->current_time = 0;
		if (read->stream) fseek(read->stream, read->start_offset, SEEK_SET);

		if (read->es_ch == com->base.on_channel) { 
			read->es_done = 0; 
			/*PLAY after complete download, estimate duration*/
			if (!read->is_remote && !read->duration) {
				AMR_ConfigureFromFile(read);
				if (read->duration) {
					NetworkCommand rcfg;
					rcfg.base.on_channel = read->es_ch;
					rcfg.base.command_type = CHAN_DURATION;
					rcfg.duration.duration = read->duration;
					rcfg.duration.duration /= read->is_wb ? 16000 : 8000;
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


static M4Err AMR_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	u32 pos, start_from;
	u8 toc, ft;
	BitStream *bs;
	AMRReader *read = plug->priv;

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

			esd = AMR_GetESD(read);
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

		toc = fgetc(read->stream);
		ft = (toc >> 3) & 0x0F;
		
		if (read->is_wb) {
	        read->es_data_size = amr_wb_frame_size[ft];
		} else {
	        read->es_data_size = amr_nb_frame_size[ft];
		}

		read->nb_samp = read->is_wb ? 320 : 160;
		/*we're seeking*/
		if (read->start_range && read->duration) {
			start_from = (u32) (read->start_range * (read->is_wb ? 16000 : 8000) );
			if (read->current_time + read->nb_samp < start_from) {
				read->current_time += read->nb_samp;
				fseek(read->stream, read->es_data_size, SEEK_CUR);
				goto fetch_next;
			} else {
				read->start_range = 0;
			}
		}
		
		read->es_data_size++;
		read->sl_hdr.compositionTimeStamp = read->current_time;
		read->es_data = malloc(sizeof(char) * (read->es_data_size+read->pad_bytes));
		read->es_data[0] = toc;
		if (read->es_data_size>1) fread(read->es_data + 1, read->es_data_size-1, 1, read->stream);
		if (read->pad_bytes) memset(read->es_data + read->es_data_size, 0, sizeof(char) * read->pad_bytes);
	}
	*out_sl_hdr = read->sl_hdr;
	*out_data_ptr = read->es_data;
	*out_data_size = read->es_data_size;
	return M4OK;
}

static M4Err AMR_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	AMRReader *read = plug->priv;

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

NetClientPlugin *NewAMRReader()
{
	AMRReader *reader;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC AMR Reader", "gpac distribution", 0)

	plug->CanHandleURL = AMR_CanHandleURL;
	plug->ConnectService = AMR_ConnectService;
	plug->CloseService = AMR_CloseService;
	plug->Get_MPEG4_IOD = AMR_Get_MPEG4_IOD;
	plug->ConnectChannel = AMR_ConnectChannel;
	plug->DisconnectChannel = AMR_DisconnectChannel;
	plug->ServiceCommand = AMR_ServiceCommand;
	/*we do support pull mode*/
	plug->ChannelGetSLP = AMR_ChannelGetSLP;
	plug->ChannelReleaseSLP = AMR_ChannelReleaseSLP;

	reader = malloc(sizeof(AMRReader));
	memset(reader, 0, sizeof(AMRReader));
	plug->priv = reader;
	return plug;
}

void DeleteAMRReader(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	AMRReader *read = plug->priv;
	free(read);
	free(plug);
}
