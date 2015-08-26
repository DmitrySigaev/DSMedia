/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / FFMPEG plugin
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

#include "ffmpeg_in.h"
#include <gpac/m4_author.h>

/*default buffer is 200 ms per channel*/
#define FFD_DATA_BUFFER		800

static u32 FFDemux_Run(void *par)
{
	AVPacket pkt;
	s64 seek_to;
	Bool video_init, do_seek, map_audio_time;
	NetworkCommand com;
	NetworkCommand map;
	SLHeader slh;
	FFDemux *ffd = (FFDemux *) par;

	memset(&map, 0, sizeof(NetworkCommand));
	map.command_type = CHAN_MAP_TIME;
	
	memset(&com, 0, sizeof(NetworkCommand));
	com.command_type = CHAN_BUFFER_QUERY;

	memset(&slh, 0, sizeof(SLHeader));

	slh.compositionTimeStampFlag = slh.decodingTimeStampFlag = 1;
	seek_to = (s64) (AV_TIME_BASE*ffd->seek_time);

	video_init = (seek_to && ffd->video_ch) ? 0 : 1;

	/*it appears that ffmpeg has trouble resyncing on some mpeg files - we trick it by restarting to 0 to get the 
	first video frame, and only then seek*/
	av_seek_frame(ffd->ctx, -1, video_init ? seek_to : 0);
	do_seek = video_init ? 0 : 1;
	map_audio_time = video_init ? ffd->unreliable_audio_timing : 0;

	while (ffd->is_running) {

		pkt.stream_index = -1;
		/*EOF*/
        if (av_read_frame(ffd->ctx, &pkt) <0) break;
		if (pkt.pts == AV_NOPTS_VALUE) pkt.pts = pkt.dts;

		slh.compositionTimeStamp = pkt.pts;
		slh.decodingTimeStamp = pkt.dts;

		MX_P(ffd->mx);
		/*blindly send audio as soon as video is init*/
		if (ffd->audio_ch && (pkt.stream_index == ffd->audio_st) && !do_seek) {
			if (map_audio_time) {
				map.base.on_channel = ffd->audio_ch;
				map.map_time.media_time = ffd->seek_time;
				/*mapwith TS=0 since we don't use SL*/
				map.map_time.timestamp = 0;
				map.map_time.reset_buffers = 1;
				map_audio_time = 0;
				NM_OnCommand(ffd->service, &map, M4OK);
			}
			/*seeking*/
			if (slh.compositionTimeStamp < (u64) seek_to) {
				slh.compositionTimeStamp = slh.decodingTimeStamp = seek_to;
			}
			NM_OnSLPRecieved(ffd->service, ffd->audio_ch, pkt.data, pkt.size, &slh, M4OK);
		} 
		else if (ffd->video_ch && (pkt.stream_index == ffd->video_st)) {
			/*if we get pts = 0 after a seek the demuxer is reseting PTSs, so force map time*/
			if (!do_seek && seek_to && !slh.compositionTimeStamp) {
				seek_to = 0;

				map.base.on_channel = ffd->video_ch;
				map.map_time.timestamp = (u64) pkt.pts;
				map.map_time.media_time = ffd->seek_time;
				map.map_time.reset_buffers = 0;
				NM_OnCommand(ffd->service, &map, M4OK);
			} else if (slh.compositionTimeStamp < (u64) seek_to) {
				slh.compositionTimeStamp = slh.decodingTimeStamp = seek_to;
			}

			NM_OnSLPRecieved(ffd->service, ffd->video_ch, pkt.data, pkt.size, &slh, M4OK);
			video_init = 1;
		}
		MX_V(ffd->mx);
		av_free_packet(&pkt);

		/*here's the trick - only seek after sending the first packets of each stream - this allows ffmpeg video decoders
		to resync properly*/
		if (do_seek && video_init) {
			av_seek_frame(ffd->ctx, -1, seek_to);
			do_seek = 0;
			map_audio_time = ffd->unreliable_audio_timing;
		}
		/*sleep untill the buffer occupancy is too low - note that this work because all streams in this
		demuxer are synchronized*/
		while (1) {
			if (ffd->audio_ch) {
				com.base.on_channel = ffd->audio_ch;
				NM_OnCommand(ffd->service, &com, M4OK);
				if (com.buffer.occupancy < ffd->data_buffer_ms) break;
			}
			if (ffd->video_ch) {
				com.base.on_channel = ffd->video_ch;
				NM_OnCommand(ffd->service, &com, M4OK);
				if (com.buffer.occupancy < ffd->data_buffer_ms) break;
			}
			Sleep(10);
			
			/*escape if disconnect*/
			if (!ffd->audio_run && !ffd->video_run) break;
		}
		if (!ffd->audio_run && !ffd->video_run) break;
	}
	/*signal EOS*/
	if (ffd->audio_ch) NM_OnSLPRecieved(ffd->service, ffd->audio_ch, NULL, 0, NULL, M4EOF);
	if (ffd->video_ch) NM_OnSLPRecieved(ffd->service, ffd->video_ch, NULL, 0, NULL, M4EOF);
	ffd->is_running = 0;

	return 0;
}

static Bool FFD_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	Bool has_audio, has_video;
	s32 i;
	AVFormatContext *ctx;
	AVOutputFormat *fmt_out;
	Bool ret = 0;
	char *ext, szName[1000], szExt[20], *szExtList;

	strcpy(szName, url);
	ext = strrchr(szName, '#');
	if (ext) ext[0] = 0;

	/*disable RTP/RTSP from ffmpeg*/
	if (!strnicmp(szName, "rtsp://", 7)) return 0;
	if (!strnicmp(szName, "rtspu://", 8)) return 0;
	if (!strnicmp(szName, "rtp://", 6)) return 0;

	ext = strrchr(szName, '.');
	if (ext) {
		strcpy(szExt, &ext[1]);
		strlwr(szExt);
		/*note we forbid ffmpeg to handle files we support*/
		if (!strcmp(szExt, "mp4") || !strcmp(szExt, "mpg4") || !strcmp(szExt, "m4a") 
			|| !strcmp(szExt, "3gp") || !strcmp(szExt, "3gpp") || !strcmp(szExt, "3gp2") || !strcmp(szExt, "3g2") 
			|| !strcmp(szExt, "mp3") 
			|| !strcmp(szExt, "amr") 
			|| !strcmp(szExt, "bt") || !strcmp(szExt, "wrl") || !strcmp(szExt, "x3dv") 
			|| !strcmp(szExt, "xmt") || !strcmp(szExt, "xmta") || !strcmp(szExt, "x3d") 
			) return 0;

		/*check any default stuff that should work with ffmpeg*/
		if (NM_CheckExtension(plug, "video/mpeg", "mpg mpeg mp2 mpa mpe mpv2", "MPEG 1/2 Movies", ext)) return 1;
		if (NM_CheckExtension(plug, "audio/basic", "snd au", "Basic Audio", ext)) return 1;
		if (NM_CheckExtension(plug, "audio/x-wav", "wav", "WAV Audio", ext)) return 1;
		if (NM_CheckExtension(plug, "video/x-ms-asf", "asf wma wmv asx asr", "WindowsMedia Movies", ext)) return 1;
		if (NM_CheckExtension(plug, "video/x-ms-video", "avi", "AVI Movies", ext)) return 1;
		/*we let ffmpeg handle mov because some QT files with uncompressed or adpcm audio use 1 audio sample 
		per MP4 sample which is a killer for our MP4 lib, whereas ffmpeg handles these as complete audio chunks 
		moreover ffmpeg handles cmov, we don't*/
		if (NM_CheckExtension(plug, "video/quicktime", "mov qt", "QuickTime Movies", ext)) return 1;
	}

	ctx = NULL;
    if (av_open_input_file(&ctx, szName, NULL, 0, NULL)<0)
		return 0;

    if (!ctx || av_find_stream_info(ctx) <0) goto exit;
	/*figure out if we can use codecs or not*/
	has_video = has_audio = 0;
    for(i = 0; i < ctx->nb_streams; i++) {
        AVCodecContext *enc = &ctx->streams[i]->codec;
        switch(enc->codec_type) {
        case CODEC_TYPE_AUDIO:
            if (!has_audio) has_audio = 1;
            break;
        case CODEC_TYPE_VIDEO:
            if (!has_video) has_video= 1;
            break;
        default:
            break;
        }
    }
	if (!has_audio && !has_video) goto exit;
	ret = 1;

	fmt_out = guess_stream_format(NULL, url, NULL);
	if (fmt_out) NM_RegisterMimeType(plug, fmt_out->mime_type, fmt_out->extensions, fmt_out->name);
	else {
		ext = strrchr(szName, '.');
		if (ext) {
			strcpy(szExt, &ext[1]);
			strlwr(szExt);

			szExtList = PMI_GetOpt(plug, "MimeTypes", "video/ffmpeg-files");
			if (!szExtList) {
				NM_RegisterMimeType(plug, "video/ffmpeg-files", szExt, "Other Movies (FFMPEG)");
			} else if (!strstr(szExtList, szExt)) {
				u32 len;
				char *buf;
				len = strlen(szExtList) + strlen(szExt) + 1;
				buf = malloc(sizeof(char)*len);
				sprintf(buf, "\"%s ", szExt);
				strcat(buf, &szExtList[1]);
				PMI_SetOpt(plug, "MimeTypes", "video/ffmpeg-files", buf);
				free(buf);
			}
		}
	}

exit:
    if (ctx) av_close_input_file(ctx);
	return ret;
}

static M4Err FFD_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	M4Err e;
	s64 last_aud_pts;
	s32 i;
	char *sOpt;
	FFDemux *ffd = plug->priv;
	char *ext, szName[1000];

	if (ffd->ctx) return M4ServiceError;

	strcpy(szName, url);
	ext = strrchr(szName, '#');
	ffd->service_type = 0;
	e = M4UnsupportedURL;
	ffd->service = serv;

	if (ext) {
		if (!stricmp(&ext[1], "video")) ffd->service_type = 1;
		else if (!stricmp(&ext[1], "audio")) ffd->service_type = 2;
		ext[0] = 0;
	}
    switch (av_open_input_file(&ffd->ctx, szName, NULL, 0, NULL)) {
	case 0: e = M4OK; break;
	case AVERROR_IO: e = M4URLNotFound; goto err_exit;
	case AVERROR_INVALIDDATA: e = M4NonCompliantBitStream; goto err_exit;
	case AVERROR_NOMEM: e = M4OutOfMem; goto err_exit;
	case AVERROR_NOFMT: e = M4UnsupportedURL; goto err_exit;
	default: e = M4ServiceError; goto err_exit;
	}


    if (av_find_stream_info(ffd->ctx) <0) goto err_exit;
	/*figure out if we can use codecs or not*/
	ffd->audio_st = ffd->video_st = -1;
    for (i = 0; i < ffd->ctx->nb_streams; i++) {
        AVCodecContext *enc = &ffd->ctx->streams[i]->codec;
        switch(enc->codec_type) {
        case CODEC_TYPE_AUDIO:
            if (ffd->audio_st<0) ffd->audio_st = i;
            break;
        case CODEC_TYPE_VIDEO:
            if (ffd->video_st<0) ffd->video_st = i;
            break;
        default:
            break;
        }
    }
	if ((ffd->service_type==1) && (ffd->video_st<0)) goto err_exit;
	if ((ffd->service_type==2) && (ffd->audio_st<0)) goto err_exit;
	if ((ffd->video_st<0) && (ffd->audio_st<0)) goto err_exit;

	/*setup indexes for BIFS/OD*/
	ffd->od_es_id = 2 + MAX(ffd->video_st, ffd->audio_st);

	sOpt = PMI_GetOpt(plug, "FFMPEG", "DataBufferMS"); 
	ffd->data_buffer_ms = 0;
	if (sOpt) ffd->data_buffer_ms = atoi(sOpt);
	if (!ffd->data_buffer_ms) ffd->data_buffer_ms = FFD_DATA_BUFFER;

	/*check we do have increasing pts. If not we can't rely on pts, we must skip SL
	we assume video pts is always present*/
	last_aud_pts = 0;
	for (i=0; i<20; i++) {
		AVPacket pkt;
		pkt.stream_index = -1;
	    if (av_read_frame(ffd->ctx, &pkt) <0) break;
		if (pkt.pts == AV_NOPTS_VALUE) pkt.pts = pkt.dts;
		if (pkt.stream_index==ffd->audio_st) last_aud_pts = pkt.pts;
	}
	if (last_aud_pts == 0) ffd->unreliable_audio_timing = 1;

	/*build seek*/
	ffd->seekable = (av_seek_frame(ffd->ctx, -1, 0)<0) ? 0 : 1;
	if (!ffd->seekable) {
	    av_close_input_file(ffd->ctx);
		av_open_input_file(&ffd->ctx, szName, NULL, 0, NULL);
	    av_find_stream_info(ffd->ctx);
	}

	/*let's go*/
	NM_OnConnect(serv, NULL, M4OK);
	return M4OK;

err_exit:
    if (ffd->ctx) av_close_input_file(ffd->ctx);
	ffd->ctx = NULL;
	NM_OnConnect(serv, NULL, e);
	return M4OK;
}


static ESDescriptor *FFD_GetESDescriptor(FFDemux *ffd, Bool for_audio)
{
	BitStream *bs;
	Bool dont_use_sl;
	ESDescriptor *esd = (ESDescriptor *) OD_NewESDescriptor(0);
	esd->ESID = 1 + (for_audio ? ffd->audio_st : ffd->video_st);
	esd->decoderConfig->streamType = for_audio ? M4ST_AUDIO : M4ST_VISUAL;
	esd->decoderConfig->avgBitrate = esd->decoderConfig->maxBitrate = 0;

	/*remap std object types - depending on input formats, FFMPEG may not have separate DSI from initial frame. 
	In this case we have no choice but using FFMPEG decoders*/
	if (for_audio) {
		switch (ffd->ctx->streams[ffd->audio_st]->codec.codec_id) {
		case CODEC_ID_MP2:
			esd->decoderConfig->objectTypeIndication = 0x6B;
			break;
		case CODEC_ID_MP3:
			esd->decoderConfig->objectTypeIndication = 0x69;
			break;
		case CODEC_ID_MPEG4AAC:
		case CODEC_ID_AAC:
			if (!ffd->ctx->streams[ffd->audio_st]->codec.extradata_size) goto opaque_audio;
			esd->decoderConfig->objectTypeIndication = 0x40;
			esd->decoderConfig->decoderSpecificInfo->dataLength = ffd->ctx->streams[ffd->audio_st]->codec.extradata_size;
			esd->decoderConfig->decoderSpecificInfo->data = malloc(sizeof(char)*ffd->ctx->streams[ffd->audio_st]->codec.extradata_size);
			memcpy(esd->decoderConfig->decoderSpecificInfo->data, 
					ffd->ctx->streams[ffd->audio_st]->codec.extradata, 
					sizeof(char)*ffd->ctx->streams[ffd->audio_st]->codec.extradata_size);
			break;
		default:
opaque_audio:
			esd->decoderConfig->objectTypeIndication = GPAC_FFMPEG_CODECS_OTI;
			bs = NewBitStream(NULL, 0, BS_WRITE);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.codec_id, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.sample_rate, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.channels, 16);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.bits_per_sample, 16);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.frame_size, 16);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.block_align, 16);

			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.codec_tag, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->audio_st]->codec.bit_rate, 32);

			if (ffd->ctx->streams[ffd->audio_st]->codec.extradata_size) {
				BS_WriteData(bs, ffd->ctx->streams[ffd->audio_st]->codec.extradata, ffd->ctx->streams[ffd->audio_st]->codec.extradata_size);
			}
			BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
			DeleteBitStream(bs);
			break;
		}
		dont_use_sl = ffd->unreliable_audio_timing;
	} else {
		switch (ffd->ctx->streams[ffd->video_st]->codec.codec_id) {
		case CODEC_ID_MPEG4:
		case CODEC_ID_H264:
			/*if dsi not detected force use ffmpeg*/
			if (!ffd->ctx->streams[ffd->video_st]->codec.extradata_size) goto opaque_video;
			/*otherwise use any MPEG-4 Visual*/
			esd->decoderConfig->objectTypeIndication = (ffd->ctx->streams[ffd->video_st]->codec.codec_id==CODEC_ID_H264) ? 0x21 : 0x20;
			esd->decoderConfig->decoderSpecificInfo->dataLength = ffd->ctx->streams[ffd->video_st]->codec.extradata_size;
			esd->decoderConfig->decoderSpecificInfo->data = malloc(sizeof(char)*ffd->ctx->streams[ffd->video_st]->codec.extradata_size);
			memcpy(esd->decoderConfig->decoderSpecificInfo->data, 
					ffd->ctx->streams[ffd->video_st]->codec.extradata, 
					sizeof(char)*ffd->ctx->streams[ffd->video_st]->codec.extradata_size);
			break;
		case CODEC_ID_MPEG1VIDEO:
			esd->decoderConfig->objectTypeIndication = 0x6A;
			break;
		case CODEC_ID_MPEG2VIDEO:
			esd->decoderConfig->objectTypeIndication = 0x65;
			break;
		default:
opaque_video:
			esd->decoderConfig->objectTypeIndication = GPAC_FFMPEG_CODECS_OTI;
			bs = NewBitStream(NULL, 0, BS_WRITE);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->video_st]->codec.codec_id, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->video_st]->codec.width, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->video_st]->codec.height, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->video_st]->codec.codec_tag, 32);
			BS_WriteInt(bs, ffd->ctx->streams[ffd->video_st]->codec.bit_rate, 32);

			if (ffd->ctx->streams[ffd->video_st]->codec.extradata_size) {
				BS_WriteData(bs, ffd->ctx->streams[ffd->video_st]->codec.extradata, ffd->ctx->streams[ffd->video_st]->codec.extradata_size);
			}
			BS_GetContent(bs, (unsigned char **) &esd->decoderConfig->decoderSpecificInfo->data, &esd->decoderConfig->decoderSpecificInfo->dataLength);
			DeleteBitStream(bs);
			break;
		}
		dont_use_sl = 0;
	}

	if (dont_use_sl) {
		esd->slConfig->predefined = SLPredef_SkipSL;
	} else {
		esd->slConfig->useAccessUnitStartFlag = esd->slConfig->useAccessUnitEndFlag = 0;
		esd->slConfig->useRandomAccessUnitsOnlyFlag = 1;
		esd->slConfig->useTimestampsFlag = 1;
		esd->slConfig->timestampResolution = AV_TIME_BASE;
	}

	return esd;
}

static M4Err FFD_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	M4Err e;
	ObjectDescriptor *od;
	ESDescriptor *esd;
	FFDemux *ffd = plug->priv;

	if (!ffd->ctx) return M4ServiceError;

	od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	od->objectDescriptorID = 1;

	/*since we don't handle multitrack in ffmpeg, we don't need to check sub_url, only use expected type*/
	if (expect_type==NM_OD_AUDIO) {
		esd = FFD_GetESDescriptor(ffd, 1);
		/*if session join, setup sync*/
		if (ffd->video_ch) esd->OCRESID = ffd->video_st+1;
		ChainAddEntry(od->ESDescriptors, esd);
		e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor **)&od);
		return e;
	}
	if (expect_type==NM_OD_VIDEO) {
		esd = FFD_GetESDescriptor(ffd, 0);
		/*if session join, setup sync*/
		if (ffd->audio_ch) esd->OCRESID = ffd->audio_st+1;
		ChainAddEntry(od->ESDescriptors, esd);
		e = OD_EncDesc((Descriptor *) od, raw_iod, raw_iod_size);
		OD_DeleteDescriptor((Descriptor **)&od);
		return e;
	}

	/*setup OD stream (no DSI)*/
	esd = (ESDescriptor *) OD_NewESDescriptor(0);
	esd->ESID = ffd->od_es_id;
	esd->OCRESID = 0;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = GPAC_STATIC_OD_OTI;
	
	/*we only send 1 full AU RAP*/
	esd->slConfig->useAccessUnitStartFlag = esd->slConfig->useAccessUnitEndFlag = 0;
	esd->slConfig->useRandomAccessUnitsOnlyFlag = 1;
	esd->slConfig->useTimestampsFlag = 1;
	esd->slConfig->timestampResolution = 1000;
	OD_AddDescToDesc((Descriptor *)od, (Descriptor *) esd);

	e = OD_EncDesc((Descriptor *)od, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **) &od);
	return e;
}


static M4Err FFD_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	FFDemux *ffd = plug->priv;

	ffd->is_running = 0;

	if (ffd->ctx) av_close_input_file(ffd->ctx);
	ffd->ctx = NULL;
	ffd->audio_ch = ffd->video_ch = ffd->od_ch = NULL;
	ffd->audio_run = ffd->video_run = 0;

	NM_OnDisconnect(ffd->service, NULL, M4OK);
	return M4OK;
}

static M4Err FFD_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	M4Err e;
	u32 ESID;
	FFDemux *ffd = plug->priv;

	e = M4ChannelNotFound;
	if (upstream) {
		e = M4InvalidMP4File;
		goto exit;
	}
	if (!strstr(url, "ES_ID=")) {
		e = M4UnsupportedURL;
		goto exit;
	}
	sscanf(url, "ES_ID=%d", &ESID);

	if (ESID==ffd->od_es_id) {
		if (ffd->od_ch) {
			e = M4ServiceError;
			goto exit;
		}
		ffd->od_ch = channel;
		e = M4OK;
	}
	else if ((s32) ESID == 1 + ffd->audio_st) {
		if (ffd->audio_ch) {
			e = M4ServiceError;
			goto exit;
		}
		ffd->audio_ch = channel;
		e = M4OK;
	}
	else if ((s32) ESID == 1 + ffd->video_st) {
		if (ffd->video_ch) {
			e = M4ServiceError;
			goto exit;
		}
		ffd->video_ch = channel;
		e = M4OK;
	}

exit:
	NM_OnConnect(ffd->service, channel, e);
	return M4OK;
}

static M4Err FFD_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	M4Err e;
	FFDemux *ffd = plug->priv;

	e = M4ChannelNotFound;
	if (ffd->od_ch == channel) {
		e = M4OK;
		ffd->od_ch = NULL;
	}
	else if (ffd->audio_ch == channel) {
		e = M4OK;
		ffd->audio_ch = NULL;
		ffd->audio_run = 0;
	}
	else if (ffd->video_ch == channel) {
		e = M4OK;
		ffd->video_ch = NULL;
		ffd->video_run = 0;
	}
	NM_OnDisconnect(ffd->service, channel, e);
	return M4OK;
}

static M4Err FFD_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	FFDemux *ffd = plug->priv;

	if (!com->base.on_channel) return M4OK;

	switch (com->command_type) {
	/*only BIFS/OD work in pull mode (cf ffmpeg_in.h)*/
	case CHAN_SET_PULL:
		if (ffd->audio_ch==com->base.on_channel) return M4NotSupported;
		if (ffd->video_ch==com->base.on_channel) return M4NotSupported;
		return M4OK;
	case CHAN_INTERACTIVE:
		return ffd->seekable ? M4OK : M4NotSupported;
	case CHAN_BUFFER:
		com->buffer.max = com->buffer.min = 0;
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = (Double) ffd->ctx->duration / AV_TIME_BASE;
		return M4OK;
	/*fetch start time*/
	case CHAN_PLAY:
		if (com->play.speed<0) return M4NotSupported;

		MX_P(ffd->mx);
		ffd->seek_time = (com->play.start_range>=0) ? com->play.start_range : 0;
		
		if (ffd->audio_ch==com->base.on_channel) ffd->audio_run = 1;
		else if (ffd->video_ch==com->base.on_channel) ffd->video_run = 1;

		/*play on media stream, start thread*/
		if ((ffd->audio_ch==com->base.on_channel) || (ffd->video_ch==com->base.on_channel)) {
			if (!ffd->is_running) {
				ffd->is_running = 1;
				TH_Run(ffd->thread, FFDemux_Run, ffd);
			}
		}
		MX_V(ffd->mx);
		return M4OK;
	case CHAN_STOP:
		if (ffd->audio_ch==com->base.on_channel) ffd->audio_run = 0;
		else if (ffd->video_ch==com->base.on_channel) ffd->video_run = 0;
		else if (ffd->od_ch==com->base.on_channel) ffd->od_state = 0;
		return M4OK;
	/*note we don't handle PAUSE/RESUME/SET_SPEED, this is automatically handled by the demuxing thread 
	through buffer occupancy queries*/

	default:
		return M4OK;
	}

	return M4OK;
}


M4Err FFD_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	LPODCODEC odc;
	ODCommand *com;
	ObjectDescriptor *OD;
	ESDescriptor *esd;

	FFDemux *ffd = plug->priv;
	if (channel==ffd->audio_ch) return M4BadParam;
	if (channel==ffd->video_ch) return M4BadParam;

	*sl_compressed = 0;
	memset(out_sl_hdr, 0, sizeof(struct tagSLHeader));
	out_sl_hdr->accessUnitEndFlag = 1;
	out_sl_hdr->accessUnitStartFlag = 1;
	out_sl_hdr->compositionTimeStampFlag = 1;
	out_sl_hdr->compositionTimeStamp = (u32) (1000 * ffd->seek_time);

	*out_reception_status = (ffd->od_state == 2) ? M4EOF : M4OK;
	if (!ffd->od_au && (ffd->od_state != 2)) {
		/*compute OD AU*/
		com = OD_NewCommand(ODUpdate_Tag);

		if ((ffd->service_type!=1) && (ffd->audio_st>=0)) {
			OD = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
			OD->objectDescriptorID = ISMA_AUDIO_OD_ID;
			esd = FFD_GetESDescriptor(ffd, 1);
			esd->OCRESID = ffd->od_es_id;
			OD_AddDescToDesc((Descriptor*)OD, (Descriptor *) esd);
			ChainAddEntry( ((ObjectDescriptorUpdate*)com)->objectDescriptors, OD);

		}
		/*compute video */
		if ((ffd->service_type!=2) && (ffd->video_st>=0)) {
			OD = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
			OD->objectDescriptorID = ISMA_VIDEO_OD_ID;
			esd = FFD_GetESDescriptor(ffd, 0);
			esd->OCRESID = ffd->od_es_id;
			OD_AddDescToDesc((Descriptor*)OD, (Descriptor *) esd);
			ChainAddEntry( ((ObjectDescriptorUpdate*)com)->objectDescriptors, OD);
		}
		odc = OD_NewCodec(OD_WRITE);
		OD_AddCommand(odc, com);
		OD_EncodeAU(odc);
		OD_GetEncodedAU(odc, &ffd->od_au, &ffd->od_au_size);
		OD_DeleteCodec(odc);
		*is_new_data = 1;

	} else {
		*is_new_data = 0;
	}
	*out_data_ptr = ffd->od_au;
	*out_data_size = ffd->od_au_size;
	if (!ffd->od_state) ffd->od_state = 1;
	return M4OK;
}

M4Err FFD_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	FFDemux *ffd = plug->priv;
	if (channel!=ffd->od_ch) return M4BadParam;

	ffd->od_state = 2;
	if (ffd->od_au) free(ffd->od_au);
	ffd->od_au = NULL;
	ffd->od_au_size = 0;
	return M4OK;
}

static Bool FFD_CanHandleURLInService(NetClientPlugin *plug, const char *url)
{
	char szURL[2048], *sep;
	FFDemux *ffd = (FFDemux *)plug->priv;
	const char *this_url = NM_GetServiceURL(ffd->service);
	if (!this_url || !url) return 0;

	strcpy(szURL, this_url);
	sep = strrchr(szURL, '#');
	if (sep) sep[0] = 0;

	if ((url[0] != '#') && strnicmp(szURL, url, sizeof(char)*strlen(szURL))) return 0;
	sep = strrchr(url, '#');
	if (!stricmp(sep, "#video") && (ffd->video_st>=0)) return 1;
	if (!stricmp(sep, "#audio") && (ffd->audio_st>=0)) return 1;
	return 0;
}

void *New_FFMPEG_Demux() 
{
	FFDemux *priv;
	NetClientPlugin *ffd = malloc(sizeof(NetClientPlugin));
	memset(ffd, 0, sizeof(NetClientPlugin));

	priv = malloc(sizeof(FFDemux));
	memset(priv, 0, sizeof(FFDemux));

    /* register all codecs, demux and protocols */
    av_register_all();
	
	ffd->CanHandleURL = FFD_CanHandleURL;
	ffd->CloseService = FFD_CloseService;
	ffd->ConnectChannel = FFD_ConnectChannel;
	ffd->ConnectService = FFD_ConnectService;
	ffd->DisconnectChannel = FFD_DisconnectChannel;
	ffd->Get_MPEG4_IOD = FFD_Get_MPEG4_IOD;
	ffd->ServiceCommand = FFD_ServiceCommand;
	/*for OD only*/
	ffd->ChannelGetSLP = FFD_ChannelGetSLP;
	ffd->ChannelReleaseSLP = FFD_ChannelReleaseSLP;

	ffd->CanHandleURLInService = FFD_CanHandleURLInService;

	priv->thread = NewThread();
	priv->mx = NewMutex();

	M4_REG_PLUG(ffd, M4STREAMINGCLIENT, "FFMPEG Demuxer", "gpac distribution", 0);
	ffd->priv = priv;
	return ffd;
}

void Delete_FFMPEG_Demux(void *ifce)
{
	FFDemux *ffd;
	NetClientPlugin *ptr = (NetClientPlugin *)ifce;

	ffd = ptr->priv;

	TH_Delete(ffd->thread);
	MX_Delete(ffd->mx);

	free(ffd);
	free(ptr);
}


