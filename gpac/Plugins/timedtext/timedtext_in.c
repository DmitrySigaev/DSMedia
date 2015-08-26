/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / 3GPP/MPEG4 timed text plugin
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

#ifndef M4_READ_ONLY

typedef struct
{
	LPNETSERVICE service;
	Bool od_done;
	Bool needs_connection;
	u32 es_status, od_status;
	LPNETCHANNEL es_ch, od_ch;

	char *od_data;
	u32 od_data_size;

	SLHeader sl_hdr;

	M4File *mp4;
	char *szFile;
	u32 tt_track;
	M4Sample *samp;
	u32 samp_num;

	u32 start_range;
	/*file downloader*/
	LPFILEDOWNLOADER dnload;
} TTIn;


static Bool TTIn_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (NM_CheckExtension(plug, "x-subtitle/SRT", "srt", "SRT SubTitles", sExt)) return 1;
	if (NM_CheckExtension(plug, "x-subtitle/TimedText", "ttxt", "3GPP TimedText", sExt)) return 1;
	return 0;
}

static Bool TTIn_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}

void tti_progress(M4TrackImporter *ti, u32 cur_sample, u32 sample_count) {}
void tti_message(M4TrackImporter *ti, M4Err e, const char *message) {}


M4Err TTIn_LoadFile(NetClientPlugin *plug, const char *url, Bool is_cache)
{
	M4Err e;
	M4TrackImporter import;

	char szFILE[M4_MAX_PATH];
	TTIn *tti = (TTIn *)plug->priv;
	char *cache_dir = PMI_GetOpt(plug, "General", "CacheDirectory");
	
	if (cache_dir && strlen(cache_dir)) {
		if (cache_dir[strlen(cache_dir)-1] != M4_PATH_SEPARATOR) {
			sprintf(szFILE, "%s%csrt_%d_mp4", cache_dir, M4_PATH_SEPARATOR, (u32) tti);
		} else {
			sprintf(szFILE, "%ssrt_%d_mp4", cache_dir, (u32) tti);
		}
	} else {
		sprintf(szFILE, "%d_temp_mp4", (u32) tti);
	}
	tti->mp4 = M4_MovieOpen(szFILE, M4_OPEN_WRITE);
	if (!tti->mp4) return M4_GetLastError(NULL);

	tti->szFile = strdup(szFILE);

	memset(&import, 0, sizeof(M4TrackImporter));
	import.import_progress = tti_progress;
	import.import_message = tti_message;
	import.dest = tti->mp4;
	import.user_data = tti;
	import.in_name = (char *) url;
	
	e = MP4T_ImportMedia(&import);
	if (!e) {
		tti->tt_track = 1;
		M4_SetStreamingTextMode(tti->mp4, 1);
	}
	return e;
}

void TTIn_OnStatus(void *cbk)
{
}

void TTIn_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	u32 total_size, bytes_done, net_status;
	const char *szCache;
	NetClientPlugin *plug = (NetClientPlugin *)cbk;
	TTIn *tti = (TTIn *) plug->priv;

	assert(tti->dnload != NULL);
	e = NM_GetDownloaderStats(tti->dnload, &total_size, &bytes_done, &bytes_per_sec, &net_status);
	/*wait to get the whole file*/
	if (e == M4OK) {
		/*notify some connection / ...*/
		if (total_size) {
			perc = (Float) (100 * bytes_done) / (Float) total_size;
			sprintf(sMsg, "Download %.2f %% (%.2f kBps)", perc, bytes_per_sec/1024);
			NM_OnMessage(tti->service, M4OK, sMsg);
		}
		return;
	}
	else if (e==M4EOF) {
		szCache = NM_GetCacheFileName(tti->dnload);
		if (!szCache) e = M4InvalidPlugin;
		else {
			e = TTIn_LoadFile(plug, szCache, 1);
		}
	} 
	/*OK confirm*/
	if (tti->needs_connection) {
		tti->needs_connection = 0;
		NM_OnConnect(tti->service, NULL, e);
	}
}

void TTIn_download_file(NetClientPlugin *plug, char *url)
{
	M4Err e;
	TTIn *tti = (TTIn *) plug->priv;

	tti->needs_connection = 1;
	e = NM_FetchFile(tti->service, url, 0, TTIn_OnStatus, TTIn_OnData, plug, &tti->dnload);
	if (e) {
		tti->needs_connection = 0;
		NM_OnConnect(tti->service, NULL, e);
	}
	/*service confirm is done once fetched*/
}

static M4Err TTIn_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	M4Err e;
	TTIn *tti = plug->priv;

	tti->service = serv;

	if (tti->dnload) NM_CloseDownloader(tti->dnload);
	tti->dnload = NULL;

	/*remote fetch*/
	if (!TTIn_is_local(url)) {
		TTIn_download_file(plug, (char *) url);
		return M4OK;
	}
	e = TTIn_LoadFile(plug, url, 0);
	NM_OnConnect(serv, NULL, e);
	return M4OK;
}

static M4Err TTIn_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	TTIn *tti = plug->priv;
	if (tti->samp) M4_DeleteSample(&tti->samp);
	if (tti->mp4) M4_MovieDelete(tti->mp4);
	tti->mp4 = NULL;
	if (tti->szFile) {
		M4_DeleteFile(tti->szFile);
		free(tti->szFile);
		tti->szFile = NULL;
	}
	if (tti->dnload) NM_CloseDownloader(tti->dnload);
	tti->dnload = NULL;

	NM_OnDisconnect(tti->service, NULL, M4OK);
	return M4OK;
}

static M4Err TTIn_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	TTIn *tti = plug->priv;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);

	od->objectDescriptorID = 1;
	/*visual object*/
	if (expect_type==NM_OD_TEXT) {
		esd = MP4T_MapESDescriptor(tti->mp4, tti->tt_track);
		ChainAddEntry(od->ESDescriptors, esd);
		esd->ESID = 2;
		e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor **)&od);
		return e;
	}

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

static M4Err TTIn_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID;
	M4Err e;
	TTIn *tti = plug->priv;

	e = M4ServiceError;
	if ((tti->es_ch==channel) || (tti->od_ch==channel)) goto exit;

	e = M4OK;
	ES_ID = 0;
	if (strstr(url, "ES_ID")) sscanf(url, "ES_ID=%d", &ES_ID);

	switch (ES_ID) {
	case 1:
		tti->od_ch = channel;
		break;
	case 2:
		tti->es_ch = channel;
		break;
	default:
		e = M4ChannelNotFound;
		break;
	}

exit:
	NM_OnConnect(tti->service, channel, e);
	return e;
}

static M4Err TTIn_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	TTIn *tti = plug->priv;
	M4Err e = M4ChannelNotFound;

	if (tti->es_ch == channel) {
		tti->es_ch = NULL;
		e = M4OK;
	} else if (tti->od_ch == channel) {
		tti->od_ch = NULL;
		e = M4OK;
	}
	NM_OnDisconnect(tti->service, channel, e);
	return M4OK;
}

static M4Err TTIn_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	TTIn *tti = plug->priv;

	if (!com->base.on_channel) return M4NotSupported;
	switch (com->command_type) {
	case CHAN_SET_PADDING:
		M4_SetSamplesPadding(tti->mp4, tti->tt_track, com->pad.padding_bytes);
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = (Double) (s64) M4_GetMediaDuration(tti->mp4, tti->tt_track);
		com->duration.duration /= M4_GetMediaTimeScale(tti->mp4, tti->tt_track);
		return M4OK;
	case CHAN_PLAY:
		tti->start_range = (com->play.start_range>0) ? (u32) (com->play.start_range * 1000) : 0;
		if (tti->od_ch == com->base.on_channel) { tti->od_done = 0; }
		else if (tti->es_ch == com->base.on_channel) {
			tti->samp_num = 0;
			if (tti->samp) M4_DeleteSample(&tti->samp);
		}
		return M4OK;
	case CHAN_STOP:
		return M4OK;
	default:
		return M4OK;
	}
}


static M4Err TTIn_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	TTIn *tti = plug->priv;

	*out_reception_status = M4OK;
	*sl_compressed = 0;
	*is_new_data = 0;

	memset(&tti->sl_hdr, 0, sizeof(SLHeader));
	tti->sl_hdr.randomAccessPointFlag = 1;
	tti->sl_hdr.compositionTimeStampFlag = 1;
	tti->sl_hdr.accessUnitStartFlag = tti->sl_hdr.accessUnitEndFlag = 1;

	/*fetching es data*/
	if (tti->es_ch == channel) {
		if (tti->samp_num>=M4_GetSampleCount(tti->mp4, tti->tt_track)) {
			*out_reception_status = M4EOF;
			return M4OK;
		}

		if (!tti->samp) {
			u32 di;
			if (tti->start_range) {
				u32 di;
				*out_reception_status = M4_GetSampleForMovieTime(tti->mp4, tti->tt_track, tti->start_range, &di, M4_SearchSyncBackward, &tti->samp, &tti->samp_num);
				tti->start_range = 0;
			} else {
				tti->samp = M4_GetSample(tti->mp4, tti->tt_track, tti->samp_num+1, &di);
			}
			if (!tti->samp) {
				*out_reception_status = M4CorruptedData;
				return M4OK;
			}
			*is_new_data = 1;
		}
		tti->sl_hdr.compositionTimeStamp = tti->sl_hdr.decodingTimeStamp = tti->samp->DTS;
		*out_data_ptr = tti->samp->data;
		*out_data_size = tti->samp->dataLength;
		*out_sl_hdr = tti->sl_hdr;
		return M4OK;
	}
	if (tti->od_ch == channel) {
		LPODCODEC codec;
		ObjectDescriptor *od;
		ObjectDescriptorUpdate *odU;
		ESDescriptor *esd;
		if (tti->od_done) {
			*out_reception_status = M4EOF;
			return M4OK;
		}
		if (!tti->od_data) {
			*is_new_data = 1;
			odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
			od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
			od->objectDescriptorID = 2;
			esd = MP4T_MapESDescriptor(tti->mp4, tti->tt_track);
			esd->ESID = 2;
			esd->OCRESID = 1;
			ChainAddEntry(od->ESDescriptors, esd);
			ChainAddEntry(odU->objectDescriptors, od);
			codec = OD_NewCodec(OD_WRITE);
			OD_AddCommand(codec, (ODCommand *)odU);
			OD_EncodeAU(codec);
			OD_GetEncodedAU(codec, &tti->od_data, &tti->od_data_size);
			OD_DeleteCodec(codec);
		}
		*out_data_ptr = tti->od_data;
		*out_data_size = tti->od_data_size;
		tti->sl_hdr.compositionTimeStamp = tti->sl_hdr.decodingTimeStamp = tti->start_range;
		*out_sl_hdr = tti->sl_hdr;
		return M4OK;
	}
	return M4ChannelNotFound;
}

static M4Err TTIn_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	TTIn *tti = plug->priv;

	if (tti->es_ch == channel) {
		if (!tti->samp) return M4BadParam;
		M4_DeleteSample(&tti->samp);
		tti->samp = NULL;
		tti->samp_num++;
		return M4OK;
	}
	if (tti->od_ch == channel) {
		if (!tti->od_data) return M4BadParam;
		free(tti->od_data);
		tti->od_data = NULL;
		tti->od_done = 1;
		return M4OK;
	}
	return M4OK;
}


void *NewTTReader()
{
	TTIn *priv;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC SubTitle Reader", "gpac distribution", 0)

	plug->CanHandleURL = TTIn_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = TTIn_ConnectService;
	plug->CloseService = TTIn_CloseService;
	plug->Get_MPEG4_IOD = TTIn_Get_MPEG4_IOD;
	plug->ConnectChannel = TTIn_ConnectChannel;
	plug->DisconnectChannel = TTIn_DisconnectChannel;
	plug->ChannelGetSLP = TTIn_ChannelGetSLP;
	plug->ChannelReleaseSLP = TTIn_ChannelReleaseSLP;
	plug->ServiceCommand = TTIn_ServiceCommand;

	priv = malloc(sizeof(TTIn));
	memset(priv, 0, sizeof(TTIn));
	plug->priv = priv;
	return plug;
}

void DeleteTTReader(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	TTIn *tti = plug->priv;
	free(tti);
	free(plug);
}

#endif
