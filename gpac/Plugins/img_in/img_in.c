/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / image format plugin
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
#include "img_in.h"

enum
{
	IMG_JPEG = 1,
	IMG_PNG,
	IMG_BMP,
};

typedef struct
{
	LPNETSERVICE service;
	/*service*/
	u32 srv_type;
	
	FILE *stream;
	u32 img_type;

	u32 pad_bytes;
	Bool es_done, bifs_done, od_done;
	LPNETCHANNEL es_ch, bifs_ch, od_ch;

	char *es_data;
	u32 es_data_size;

	char *od_data;
	u32 od_data_size;


	SLHeader sl_hdr;

	/*file downloader*/
	LPFILEDOWNLOADER dnload;
} IMGLoader;


static void IMG_GetImageSize(IMGLoader *jpl, u32 *width, u32 *height)
{
	BitStream *bs;
	u32 pos;
	pos = ftell(jpl->stream);
	fseek(jpl->stream, 0, SEEK_SET);
	bs = NewBitStreamFromFile(jpl->stream, BS_FILE_READ);
	
	*width = *height = 0;
	if (jpl->img_type==IMG_JPEG) {
		u32 type, w, h;
		/*check for SOI marker and JFIF marker*/
		if (BS_ReadInt(bs, 8) != 0xFF) goto exit;
		if (BS_ReadInt(bs, 8) != 0xD8) goto exit;
		if (BS_ReadInt(bs, 8) != 0xFF) goto exit;
		BS_ReadInt(bs, 8);
		/*get frame header FFC0*/
		while (BS_Available(bs)) {
			if (BS_ReadInt(bs, 8) != 0xFF) continue;
			type = BS_ReadInt(bs, 8);
			switch (type) {
			case 0xC0:
			case 0xC1:
			case 0xC2:
				BS_SkipBytes(bs, 3);
				h = BS_ReadInt(bs, 16);
				w = BS_ReadInt(bs, 16);
				if ((w > *width) || (h > *height)) {
					*width = w;
					*height = h;
				}
				break;
			case 0xD0:
			case 0xD1:
			case 0xD2:
			case 0xD3:
			case 0xD4:
			case 0xD5:
			case 0xD6:
			case 0xD7:
				break;
			}
		}
	} else if (jpl->img_type==IMG_PNG) {
		/*check for PNG sig*/
		if ( (BS_ReadInt(bs, 8) != 0x89) || (BS_ReadInt(bs, 8) != 0x50) || (BS_ReadInt(bs, 8) != 0x4E) 
			|| (BS_ReadInt(bs, 8) != 0x47) || (BS_ReadInt(bs, 8) != 0x0D) || (BS_ReadInt(bs, 8) != 0x0A) 
			|| (BS_ReadInt(bs, 8) != 0x1A) || (BS_ReadInt(bs, 8) != 0x0A) ) goto exit;
		BS_ReadInt(bs, 32);
		/*check for PNG IHDR*/
		if ( (BS_ReadInt(bs, 8) != 'I') || (BS_ReadInt(bs, 8) != 'H') 
			|| (BS_ReadInt(bs, 8) != 'D') || (BS_ReadInt(bs, 8) != 'R')) goto exit;

		*width = BS_ReadInt(bs, 32);
		*height = BS_ReadInt(bs, 32);

	} else if (jpl->img_type==IMG_BMP) {
		BITMAPFILEHEADER fh;
		BITMAPINFOHEADER fi;
		BS_ReadData(bs, (unsigned char *) &fh, 14);
		BS_ReadData(bs, (unsigned char *) &fi, 40);
		
		*width = fi.biWidth;
		*height = fi.biHeight;
	}

exit:
	fseek(jpl->stream, 0, pos);
	DeleteBitStream(bs);
	return;
}

static Bool IMG_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt;
	sExt = strrchr(url, '.');
	if (!sExt) return 0;
	if (NM_CheckExtension(plug, "image/jpeg", "jpeg jpg", "JPEG Images", sExt)) return 1;
	if (NM_CheckExtension(plug, "image/png", "png", "PNG Images", sExt)) return 1;
	if (NM_CheckExtension(plug, "image/bmp", "bmp", "MS Bitmap Images", sExt)) return 1;
	return 0;
}

static Bool jp_is_local(const char *url)
{
	if (!strnicmp(url, "file://", 7)) return 1;
	if (strstr(url, "://")) return 0;
	return 1;
}

void IMG_OnStatus(void *cbk)
{
}

void IMG_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	u32 total_size, bytes_done, net_status;
	const char *szCache;
	IMGLoader *jpl = (IMGLoader *) cbk;

	assert(jpl->dnload != NULL);
	e = NM_GetDownloaderStats(jpl->dnload, &total_size, &bytes_done, &bytes_per_sec, &net_status);
	/*wait to get the whole file*/
	if (e == M4OK) {
		/*notify some connection / ...*/
		if (total_size) {
			perc = (Float) (100 * bytes_done) / (Float) total_size;
			sprintf(sMsg, "Download %.2f %% (%.2f kBps)", perc, bytes_per_sec/1024);
			NM_OnMessage(jpl->service, M4OK, sMsg);
		}
		return;
	}
	else if (e==M4EOF) {
		szCache = NM_GetCacheFileName(jpl->dnload);
		if (!szCache) e = M4InvalidPlugin;
		else {
			jpl->stream = fopen((char *) szCache, "rb");
			if (!jpl->stream) e = M4ServiceError;
			else {
				e = M4OK;
				fseek(jpl->stream, 0, SEEK_END);
				jpl->es_data_size = ftell(jpl->stream);
				fseek(jpl->stream, 0, SEEK_SET);
			}
		}
	} 
	/*OK confirm*/
	NM_OnConnect(jpl->service, NULL, e);
}

void jp_download_file(NetClientPlugin *plug, char *url)
{
	M4Err e;
	IMGLoader *jpl = (IMGLoader *) plug->priv;

	e = NM_FetchFile(jpl->service, url, 0, IMG_OnStatus, IMG_OnData, jpl, &jpl->dnload);
	if (e) NM_OnConnect(jpl->service, NULL, e);
	/*service confirm is done once fetched*/
}

static M4Err IMG_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	char *sExt;
	IMGLoader *jpl = plug->priv;

	jpl->service = serv;

	sExt = strrchr(url, '.');
	if (!stricmp(sExt, ".jpeg") || !stricmp(sExt, ".jpg")) jpl->img_type = IMG_JPEG;
	else if (!stricmp(sExt, ".png")) jpl->img_type = IMG_PNG;
	else if (!stricmp(sExt, ".bmp")) jpl->img_type = IMG_BMP;

	if (jpl->dnload) NM_CloseDownloader(jpl->dnload);
	jpl->dnload = NULL;

	/*remote fetch*/
	if (!jp_is_local(url)) {
		jp_download_file(plug, (char *) url);
		return M4OK;
	}

	jpl->stream = fopen(url, "rb");
	if (jpl->stream) {
		fseek(jpl->stream, 0, SEEK_END);
		jpl->es_data_size = ftell(jpl->stream);
		fseek(jpl->stream, 0, SEEK_SET);
	}
	NM_OnConnect(serv, NULL, jpl->stream ? M4OK : M4URLNotFound);
	return M4OK;
}

static M4Err IMG_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	IMGLoader *jpl = plug->priv;
	if (jpl->stream) fclose(jpl->stream);
	jpl->stream = NULL;
	if (jpl->dnload) NM_CloseDownloader(jpl->dnload);
	jpl->dnload = NULL;

	NM_OnDisconnect(jpl->service, NULL, M4OK);
	return M4OK;
}

static M4Err IMG_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	ESDescriptor *esd;
	M4Err e;
	BitStream *bs;
	u32 w, h;
	IMGLoader *jpl = plug->priv;
	ObjectDescriptor *od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	/*we don't handle multitrack in aac, we don't need to check sub_url, only use expected type
	note we could (for JPEG) try to use thumbnails vs full image...*/

	jpl->srv_type = expect_type;

	od->objectDescriptorID = 1;
	/*visual object*/
	if (expect_type==NM_OD_VIDEO) {
		esd = OD_NewESDescriptor(0);
		esd->slConfig->timestampResolution = 1000;
		esd->decoderConfig->streamType = M4ST_VISUAL;
		if (jpl->img_type == IMG_JPEG) esd->decoderConfig->objectTypeIndication = 0x6c;
		else if (jpl->img_type == IMG_PNG) esd->decoderConfig->objectTypeIndication = 0x6d;
		else if (jpl->img_type == IMG_BMP) esd->decoderConfig->objectTypeIndication = GPAC_BMP_OTI;
		ChainAddEntry(od->ESDescriptors, esd);
		esd->ESID = 3;

		e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor **)&od);
		return e;
	}
	/*inline scene*/
	/*OD ESD*/
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = 0x01;
	esd->ESID = 1;
	ChainAddEntry(od->ESDescriptors, esd);
	/*BIFS ESD*/
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->decoderConfig->streamType = M4ST_SCENE;
	esd->decoderConfig->objectTypeIndication = 0x01;
	esd->ESID = 2;
	esd->OCRESID = 1;
	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, 0, 10);
	BS_WriteInt(bs, 1, 1);
	BS_WriteInt(bs, 1, 1);	/*pixel metrics*/
	IMG_GetImageSize(jpl, &w, &h);
	if (w && h) {
		BS_WriteInt(bs, 1, 1);
		BS_WriteInt(bs, w, 16);
		BS_WriteInt(bs, h, 16);
	} else {
		BS_WriteInt(bs, 0, 1);
	}
	BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(bs);
	ChainAddEntry(od->ESDescriptors, esd);
	e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **)&od);
	return e;
}

static M4Err IMG_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ES_ID;
	M4Err e;
	IMGLoader *jpl = plug->priv;

	e = M4ServiceError;
	if ((jpl->es_ch==channel) || (jpl->bifs_ch==channel) || (jpl->od_ch==channel)) goto exit;

	e = M4OK;
	if (strstr(url, "ES_ID")) {
		sscanf(url, "ES_ID=%d", &ES_ID);
	}
	/*URL setup*/
	else if (!jpl->es_ch && IMG_CanHandleURL(plug, url)) ES_ID = 3;

	switch (ES_ID) {
	case 2:
		jpl->bifs_ch = channel;
		break;
	case 1:
		jpl->od_ch = channel;
		break;
	case 3:
		jpl->es_ch = channel;
		break;
	}

exit:
	NM_OnConnect(jpl->service, channel, e);
	return e;
}

static M4Err IMG_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	IMGLoader *jpl = plug->priv;

	if (jpl->es_ch == channel) {
		jpl->es_ch = NULL;
	} else if (jpl->od_ch == channel) {
		jpl->od_ch = NULL;
	} else if (jpl->bifs_ch == channel) {
		jpl->bifs_ch = NULL;
	}
	NM_OnDisconnect(jpl->service, channel, M4OK);
	return M4OK;
}

static M4Err IMG_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	IMGLoader *jpl = plug->priv;

	if (!com->base.on_channel) return M4NotSupported;
	switch (com->command_type) {
	case CHAN_SET_PADDING:
		jpl->pad_bytes = com->pad.padding_bytes;
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = 0;
		return M4OK;
	case CHAN_PLAY:
		/*note we don't handle range since we're only dealing with images*/
		if (jpl->es_ch == com->base.on_channel) { jpl->es_done = 0; }
		else if (jpl->bifs_ch == com->base.on_channel) { jpl->bifs_done = 0; }
		else if (jpl->od_ch == com->base.on_channel) { jpl->od_done = 0; }
		return M4OK;
	case CHAN_STOP:
		return M4OK;
	default:
		return M4OK;
	}
}


static M4Err IMG_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	IMGLoader *jpl = plug->priv;

	*out_reception_status = M4OK;
	*sl_compressed = 0;
	*is_new_data = 0;

	memset(&jpl->sl_hdr, 0, sizeof(SLHeader));
	jpl->sl_hdr.randomAccessPointFlag = 1;
	jpl->sl_hdr.compositionTimeStampFlag = 1;
	*out_sl_hdr = jpl->sl_hdr;

	/*fetching es data*/
	if (jpl->es_ch == channel) {
		if (jpl->es_done) {
			*out_reception_status = M4EOF;
			return M4OK;
		}
		if (!jpl->es_data) {
			if (!jpl->stream) {
				*out_data_ptr = NULL;
				*out_data_size = 0;
				return M4OK;
			}
			*is_new_data = 1;
			fseek(jpl->stream, 0, SEEK_SET);
			jpl->es_data = malloc(sizeof(char) * (jpl->es_data_size + jpl->pad_bytes));
			fread(jpl->es_data, sizeof(char) * jpl->es_data_size, 1, jpl->stream);
			fseek(jpl->stream, 0, SEEK_SET);
			if (jpl->pad_bytes) memset(jpl->es_data + jpl->es_data_size, 0, sizeof(char) * jpl->pad_bytes);

		}
		*out_data_ptr = jpl->es_data;
		*out_data_size = jpl->es_data_size;
		return M4OK;
	}
	if (jpl->bifs_ch == channel) {
		if (jpl->bifs_done) {
			*out_reception_status = M4EOF;
			return M4OK;
		}
		*out_data_ptr = (char *) ISMA_BIFS_IMAGE;
		*out_data_size = 10;
		return M4OK;
	}
	if (jpl->od_ch == channel) {
		LPODCODEC codec;
		ObjectDescriptor *od;
		ObjectDescriptorUpdate *odU;
		ESDescriptor *esd;
		if (jpl->od_done) {
			*out_reception_status = M4EOF;
			return M4OK;
		}
		if (!jpl->od_data) {
			*is_new_data = 1;
			odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);
			od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
			od->objectDescriptorID = ISMA_VIDEO_OD_ID;
			esd = OD_NewESDescriptor(0);
			esd->slConfig->timestampResolution = 1000;
			esd->ESID = 3;
			esd->decoderConfig->streamType = M4ST_VISUAL;
			if (jpl->img_type==IMG_JPEG) esd->decoderConfig->objectTypeIndication = 0x6c;
			else if (jpl->img_type==IMG_PNG) esd->decoderConfig->objectTypeIndication = 0x6d;
			else if (jpl->img_type==IMG_BMP) esd->decoderConfig->objectTypeIndication = GPAC_BMP_OTI;
			ChainAddEntry(od->ESDescriptors, esd);
			ChainAddEntry(odU->objectDescriptors, od);
			codec = OD_NewCodec(OD_WRITE);
			OD_AddCommand(codec, (ODCommand *)odU);
			OD_EncodeAU(codec);
			OD_GetEncodedAU(codec, &jpl->od_data, &jpl->od_data_size);
			OD_DeleteCodec(codec);
		}
		*out_data_ptr = jpl->od_data;
		*out_data_size = jpl->od_data_size;
		return M4OK;
	}

	return M4ChannelNotFound;
}

static M4Err IMG_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	IMGLoader *jpl = plug->priv;

	if (jpl->es_ch == channel) {
		if (!jpl->es_data) return M4BadParam;
		free(jpl->es_data);
		jpl->es_data = NULL;
		jpl->es_done = 1;
		return M4OK;
	}
	if (jpl->bifs_ch == channel) {
		jpl->bifs_done = 1;
		return M4OK;
	}
	if (jpl->od_ch == channel) {
		if (!jpl->od_data) return M4BadParam;
		free(jpl->od_data);
		jpl->od_data = NULL;
		jpl->od_done = 1;
		return M4OK;
	}
	return M4OK;
}


void *NewLoaderInterface()
{
	IMGLoader *priv;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC Image Reader", "gpac distribution", 0)

	plug->CanHandleURL = IMG_CanHandleURL;
	plug->CanHandleURLInService = NULL;
	plug->ConnectService = IMG_ConnectService;
	plug->CloseService = IMG_CloseService;
	plug->Get_MPEG4_IOD = IMG_Get_MPEG4_IOD;
	plug->ConnectChannel = IMG_ConnectChannel;
	plug->DisconnectChannel = IMG_DisconnectChannel;
	plug->ChannelGetSLP = IMG_ChannelGetSLP;
	plug->ChannelReleaseSLP = IMG_ChannelReleaseSLP;
	plug->ServiceCommand = IMG_ServiceCommand;

	priv = malloc(sizeof(IMGLoader));
	memset(priv, 0, sizeof(IMGLoader));
	plug->priv = priv;
	return plug;
}

void DeleteLoaderInterface(void *ifce)
{
	NetClientPlugin *plug = (NetClientPlugin *) ifce;
	IMGLoader *jpl = plug->priv;
	free(jpl);
	free(plug);
}
