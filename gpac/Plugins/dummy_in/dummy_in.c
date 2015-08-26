/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / Dummy input plugin
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
/*for M4ST_PRIVATE_SCENE definition*/
#include <gpac/m4_decoder.h>

typedef struct
{
	u32 ESID;
	LPNETCHANNEL ch;
	u32 start, end;
} DummyChannel;

typedef struct
{
	/*the service we're responsible for*/
	LPNETSERVICE service;
	char szURL[2048];
	char szCache[2048];
	u32 oti;
	Chain *channels;

	/*file downloader*/
	LPFILEDOWNLOADER dnload;
} DCReader;

DummyChannel *DC_GetChannel(DCReader *read, LPNETCHANNEL ch)
{
	u32 i;
	for (i=0; i<ChainGetCount(read->channels); i++) {
		DummyChannel *dc = ChainGetEntry(read->channels, i);
		if (dc->ch && dc->ch==ch) return dc;
	}
	return NULL;
}

Bool DC_RemoveChannel(DCReader *read, LPNETCHANNEL ch)
{
	u32 i;
	for (i=0; i<ChainGetCount(read->channels); i++) {
		DummyChannel *dc = ChainGetEntry(read->channels, i);
		if (dc->ch && dc->ch==ch) {
			ChainDeleteEntry(read->channels, i);
			free(dc);
			return 1;
		}
	}
	return 0;
}

Bool DC_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (!strnicmp(sExt, ".gz", 3)) sExt = strrchr(sExt, '.');

	/*the mpeg-4 mime types for bt/xmt are NOT registered at all :)*/
	if (NM_CheckExtension(plug, "application/x-bt", "bt bt.gz btz", "MPEG-4 Text (BT)", sExt)) return 1;
	if (NM_CheckExtension(plug, "application/x-xmt", "xmt xmt.gz xmtz", "MPEG-4 Text (XMT)", sExt)) return 1;
	//if (NM_CheckExtension(plug, "application/x-xmta", "xmta xmta.gz xmtaz", "MPEG-4 Text (XMT)", sExt)) return 1;
	/*but all these ones are*/
	if (NM_CheckExtension(plug, "model/vrml", "wrl wrl.gz", "VRML World", sExt)) return 1;
	if (NM_CheckExtension(plug, "x-model/x-vrml", "wrl wrl.gz", "VRML World", sExt)) return 1;
	if (NM_CheckExtension(plug, "model/x3d+vrml", "x3dv x3dv.gz x3dvz", "X3D/VRML World", sExt)) return 1;
	if (NM_CheckExtension(plug, "model/x3d+xml", "x3d x3d.gz x3dz", "X3D/XML World", sExt)) return 1;
	if (NM_CheckExtension(plug, "application/x-shockwave-flash", "swf", "Macromedia Flash Movie", sExt)) return 1;
	if (NM_CheckExtension(plug, "image/svg+xml", "svg", "SVG Movie", sExt)) return 1;
	return 0;
}

void DC_OnStatus(void *cbk)
{
}

void DC_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	u32 total_size, bytes_done, net_status;
	const char *szCache;
	DCReader *read = (DCReader *) cbk;

	e = NM_GetDownloaderStats(read->dnload, &total_size, &bytes_done, &bytes_per_sec, &net_status);

	/*wait to get the whole file*/
	if (e == M4OK) {
		/*notify some connection / ...*/
		if (total_size) {
			perc = (Float) (100 * bytes_done) / (Float) total_size;
			sprintf(sMsg, "Download %.2f %% (%.2f kBps)", perc, bytes_per_sec/1024);
			NM_OnMessage(read->service, M4OK, sMsg);
		}
		return;
	}
	else if (e==M4EOF) {
		szCache = NM_GetCacheFileName(read->dnload);
		if (!szCache) e = M4InvalidPlugin;
		else {
			e = M4OK;
			strcpy(read->szCache, szCache);
		}
	}
	/*OK confirm*/
	NM_OnConnect(read->service, NULL, e);
}

void DC_DownloadFile(NetClientPlugin *plug, char *url)
{
	M4Err e;
	DCReader *read = (DCReader *) plug->priv;

	e = NM_FetchFile(read->service, url, 0, DC_OnStatus, DC_OnData, read, &read->dnload);
	if (e) NM_OnConnect(read->service, NULL, e);
}


static Bool DC_IsLocal(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}

M4Err DC_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	DCReader *read = (DCReader *) plug->priv;
	FILE *test;
	char *tmp, *ext;

	if (!read || !serv || !url) return M4BadParam;

	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;

	strcpy(read->szURL, url);
	ext = strrchr(read->szURL, '.');
	ext += 1;
	if (ext) {
		tmp = strchr(ext, '#');
		if (tmp) tmp[0] = 0;
	}
	read->service = serv;

	if (ext) {
		if (!stricmp(ext, "bt") || !stricmp(ext, "xmta") || !stricmp(ext, "xmt") 
			|| !stricmp(ext, "wrl") || !stricmp(ext, "x3d") || !stricmp(ext, "x3dv") 
			|| !stricmp(ext, "swf")
			) 
			read->oti = 0x01;

		if (!stricmp(ext, "svg"))
			read->oti = 0x02;
	}

	/*remote fetch*/
	if (!DC_IsLocal(url)) {
		DC_DownloadFile(plug, read->szURL);
		return M4OK;
	}

	test = fopen(read->szURL, "rt");
	if (!test) {
		NM_OnConnect(serv, NULL, M4URLNotFound);
		return M4OK;
	}
	fclose(test);
	NM_OnConnect(serv, NULL, M4OK);
	return M4OK;
}

M4Err DC_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	DCReader *read = (DCReader *) plug->priv;
	if (read->dnload) NM_CloseDownloader(read->dnload);
	read->dnload = NULL;
	NM_OnDisconnect(read->service, NULL, M4OK);
	return M4OK;
}

/*Dummy input just send a file name, no multitrack to handle so we don't need to check sub_url nor expected type*/
M4Err DC_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	DCReader *read = (DCReader *) plug->priv;
	InitialObjectDescriptor *iod = (InitialObjectDescriptor *) OD_NewDescriptor(InitialObjectDescriptor_Tag);
	iod->scene_profileAndLevel = 1;
	iod->graphics_profileAndLevel = 1;
	iod->OD_profileAndLevel = 1;
	iod->audio_profileAndLevel = 0xFE;
	iod->visual_profileAndLevel = 0xFE;
	iod->objectDescriptorID = 1;

	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->slConfig->useTimestampsFlag = 1;
	esd->ESID = 0xFFFE;
	esd->decoderConfig->streamType = M4ST_PRIVATE_SCENE;
	esd->decoderConfig->objectTypeIndication = read->oti;
	if (read->dnload) {
		esd->decoderConfig->decoderSpecificInfo->dataLength = strlen(read->szCache) + 1;
		esd->decoderConfig->decoderSpecificInfo->data = strdup(read->szCache);
	} else {
		esd->decoderConfig->decoderSpecificInfo->dataLength = strlen(read->szURL) + 1;
		esd->decoderConfig->decoderSpecificInfo->data = strdup(read->szURL);
	}
	ChainAddEntry(iod->ESDescriptors, esd);
	OD_EncDesc((Descriptor *)iod, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **) &iod);
	return M4OK;
}

M4Err DC_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	DCReader *read = (DCReader *) plug->priv;
	DummyChannel *dc;

	if (!com->base.on_channel) return M4OK;

	dc = DC_GetChannel(read, com->base.on_channel);
	if (!dc) return M4ChannelNotFound;

	switch (com->command_type) {
	case CHAN_SET_PULL: return M4OK;
	case CHAN_INTERACTIVE: return M4OK;
	case CHAN_SET_PADDING: return M4NotSupported;
	case CHAN_BUFFER:
		com->buffer.max = com->buffer.min = 0;
		return M4OK;
	case CHAN_DURATION:
		/*this plugin is not made for updates, use undefined duration*/
		com->duration.duration = 0;
		return M4OK;
	case CHAN_PLAY:
		dc->start = (u32) (1000 * com->play.start_range);
		dc->end = (u32) (1000 * com->play.end_range);
		return M4OK;
	case CHAN_STOP:
		return M4OK;
	case CHAN_CONFIG: return M4OK;
	case CHAN_GET_DSI:
		com->get_dsi.dsi = NULL;
		com->get_dsi.dsi_len = 0;
		return M4OK;
	}
	return M4OK;
}

M4Err DC_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ESID;
	DCReader *read = (DCReader *) plug->priv;
	
	sscanf(url, "ES_ID=%d", &ESID);
	if (!ESID) {
		NM_OnConnect(read->service, channel, M4ChannelNotFound);
	} else {
		DummyChannel *dc;
		SAFEALLOC(dc, sizeof(DummyChannel));
		dc->ch = channel;
		dc->ESID = ESID;
		ChainAddEntry(read->channels, dc);
		NM_OnConnect(read->service, channel, M4OK);
	}
	return M4OK;
}

M4Err DC_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	Bool had_ch;
	DCReader *read = (DCReader *) plug->priv;

	had_ch = DC_RemoveChannel(read, channel);
	NM_OnDisconnect(read->service, channel, had_ch ? M4OK : M4ChannelNotFound);
	return M4OK;
}

M4Err DC_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	DummyChannel *dc;
	DCReader *read = (DCReader *) plug->priv;
	dc = DC_GetChannel(read, channel);
	if (!dc) return M4ChannelNotFound;

	memset(out_sl_hdr, 0, sizeof(struct tagSLHeader));
	out_sl_hdr->compositionTimeStampFlag = 1;
	out_sl_hdr->compositionTimeStamp = dc->start;
	out_sl_hdr->accessUnitStartFlag = 1;
	*sl_compressed = 0;
	*out_reception_status = M4OK;
	*is_new_data = 1;
	return M4OK;
}

M4Err DC_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	return M4OK;
}

Bool DC_CanHandleURLInService(NetClientPlugin *plug, const char *url)
{
	return 0;
}

Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType==M4STREAMINGCLIENT) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType)
{
	DCReader *read;
	NetClientPlugin *plug;
	if (InterfaceType != M4STREAMINGCLIENT) return NULL;

	SAFEALLOC(plug, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC Dummy Loader", "gpac distribution", 0)

	plug->CanHandleURL = DC_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = DC_ConnectService;
	plug->CloseService = DC_CloseService;
	plug->Get_MPEG4_IOD = DC_Get_MPEG4_IOD;
	plug->ConnectChannel = DC_ConnectChannel;
	plug->DisconnectChannel = DC_DisconnectChannel;
	plug->ServiceCommand = DC_ServiceCommand;
	plug->CanHandleURLInService = DC_CanHandleURLInService;
	plug->ChannelGetSLP = DC_ChannelGetSLP;
	plug->ChannelReleaseSLP = DC_ChannelReleaseSLP;
	SAFEALLOC(read, sizeof(DCReader));
	read->channels = NewChain();
	plug->priv = read;
	return plug;
}

void ShutdownInterface(BaseInterface *bi)
{
	NetClientPlugin *ifcn = (NetClientPlugin*)bi;
	if (ifcn->InterfaceType==M4STREAMINGCLIENT) {
		DCReader *read = (DCReader*)ifcn->priv;
		assert(!ChainGetCount(read->channels));
		DeleteChain(read->channels);
		free(read);
		free(bi);
	}
}
