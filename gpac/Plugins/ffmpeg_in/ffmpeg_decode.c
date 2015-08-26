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

static AVCodec *ffmpeg_get_codec(u32 codec_4cc)
{
	char name[5];
	AVCodec *codec;
	name[4] = 0;
	MP4TypeToString(codec_4cc, name);

	codec = avcodec_find_decoder_by_name(name);
	if (!codec) {
		strupr(name);
		codec = avcodec_find_decoder_by_name(name);
		if (!codec) {
			strlwr(name);
			codec = avcodec_find_decoder_by_name(name);
		}
	}
	/*custom mapings*/
	if (!codec) {
		if (!stricmp(name, "s263")) codec = avcodec_find_decoder_by_name("h263");
		else if (!stricmp(name, "samr") || !stricmp(name, "amr ")) codec = avcodec_find_decoder_by_name("amr_nb");
		else if (!stricmp(name, "sawb")) codec = avcodec_find_decoder_by_name("amr_wb");
	}
	return codec;
}

static void FFDEC_LoadDSI(FFDec *ffd, BitStream *bs, Bool from_ff_demux)
{
	u32 dsi_size;
	if (!ffd->codec) return;

	dsi_size = (u32) BS_Available(bs);
	if (!dsi_size) return;

	/*demuxer is ffmpeg, extra data can be copied directly*/
	if (from_ff_demux) {
		av_free(ffd->ctx->extradata);
		ffd->ctx->extradata_size = dsi_size;
		ffd->ctx->extradata = (uint8_t*) av_mallocz(ffd->ctx->extradata_size);
		BS_ReadData(bs, ffd->ctx->extradata, ffd->ctx->extradata_size);
		return;
	}

	switch (ffd->codec->id) {
	case CODEC_ID_SVQ3:
	{
		u32 at_type, size;
		size = BS_ReadInt(bs, 32);
		/*there should be an 'SMI' entry*/
		at_type = BS_ReadInt(bs, 32);
		if (at_type == FOUR_CHAR_INT('S', 'M', 'I', ' ')) {
			av_free(ffd->ctx->extradata);
			ffd->ctx->extradata_size = 0x5a + size;
			ffd->ctx->extradata = (uint8_t*) av_mallocz(ffd->ctx->extradata_size);
			strcpy(ffd->ctx->extradata, "SVQ3");
			BS_ReadData(bs, (unsigned char *)ffd->ctx->extradata + 0x5a, size);
		}
	}
		break;
	default:
		av_free(ffd->ctx->extradata);
		ffd->ctx->extradata_size = dsi_size;
		ffd->ctx->extradata = (uint8_t*) av_mallocz(ffd->ctx->extradata_size);
		BS_ReadData(bs, ffd->ctx->extradata, ffd->ctx->extradata_size);
		break;
	}
}

static M4Err FFDEC_AttachStream(BaseDecoder *plug, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	u32 codec_id;
	BitStream *bs;
	M4VDecoderSpecificInfo dsi;
	M4Err e;
	FFDec *ffd = (FFDec *)plug->privateStack;
	if (ffd->ES_ID || DependsOnES_ID || UpStream) return M4NotSupported;
	if (!ffd->oti) return M4NotSupported;
	ffd->ES_ID = ES_ID;

	ffd->ctx = avcodec_alloc_context();

	if (ffd->st==4) ffd->frame = avcodec_alloc_frame();
	
	/*private FFMPEG DSI*/
	if (ffd->oti == GPAC_FFMPEG_CODECS_OTI) {
		bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		codec_id = BS_ReadInt(bs, 32);
		if (ffd->st==M4ST_AUDIO) {
			ffd->ctx->codec_type = CODEC_TYPE_AUDIO;
			ffd->ctx->sample_rate = BS_ReadInt(bs, 32);
			ffd->ctx->channels = BS_ReadInt(bs, 16);
			ffd->ctx->bits_per_sample = BS_ReadInt(bs, 16);
			/*force 16 bits output*/
			ffd->ctx->bits_per_sample = 16;
			ffd->ctx->frame_size = BS_ReadInt(bs, 16);
			ffd->ctx->block_align = BS_ReadInt(bs, 16);
		} else if (ffd->st==M4ST_VISUAL) {
			ffd->ctx->codec_type = CODEC_TYPE_VIDEO;
			ffd->ctx->width = BS_ReadInt(bs, 32);
			ffd->ctx->height = BS_ReadInt(bs, 32);
		}
		ffd->ctx->codec_tag = BS_ReadInt(bs, 32);
		ffd->ctx->bit_rate = BS_ReadInt(bs, 32);

		ffd->codec = avcodec_find_decoder(codec_id);
		FFDEC_LoadDSI(ffd, bs, 1);
		DeleteBitStream(bs);
	} 
	/*private QT DSI*/
	else if (ffd->oti == GPAC_QT_CODECS_OTI) {
		bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		codec_id = BS_ReadInt(bs, 32);
		if (ffd->st==M4ST_AUDIO) {
			ffd->ctx->codec_type = CODEC_TYPE_AUDIO;
			ffd->ctx->sample_rate = BS_ReadInt(bs, 32);
			ffd->ctx->channels = BS_ReadInt(bs, 32);
			ffd->ctx->bits_per_sample = BS_ReadInt(bs, 32);
			ffd->ctx->frame_size = BS_ReadInt(bs, 32);
			/*just in case...*/
			if (codec_id == FOUR_CHAR_INT('a', 'm', 'r', ' ')) {
			  ffd->ctx->sample_rate = 8000;
			  ffd->ctx->channels = 1;
			  ffd->ctx->bits_per_sample = 16;
			  ffd->ctx->frame_size = 160;
			}
		} else if (ffd->st==M4ST_VISUAL) {
			ffd->ctx->codec_type = CODEC_TYPE_VIDEO;
			ffd->ctx->width = BS_ReadInt(bs, 32);
			ffd->ctx->height = BS_ReadInt(bs, 32);
		}
		ffd->codec = ffmpeg_get_codec(codec_id);
		FFDEC_LoadDSI(ffd, bs, 0);
		DeleteBitStream(bs);
	}
	/*use std MPEG-4 st/oti*/
	else {
		u32 codec_id = 0;
		if (ffd->st==M4ST_VISUAL) {
			ffd->ctx->codec_type = CODEC_TYPE_VIDEO;
			switch (ffd->oti) {
			case 0x20:
				codec_id = CODEC_ID_MPEG4;
				break;
			case 0x21:
				codec_id = CODEC_ID_H264;
				/*ffmpeg H264/AVC needs that*/
				ffd->ctx->codec_tag = 0x31637661;
				break;
			case 0x6A:
			case 0x60:
			case 0x61:
			case 0x62:
			case 0x63:
			case 0x64:
			case 0x65:
				codec_id = CODEC_ID_MPEG2VIDEO;
				break;
			case 0x6C:
				codec_id = CODEC_ID_MJPEG;
				break;
			case 0xFF:
				codec_id = CODEC_ID_SVQ3;
				break;
			}
		} else if (ffd->st==M4ST_AUDIO) {
			ffd->ctx->codec_type = CODEC_TYPE_AUDIO;
			switch (ffd->oti) {
			case 0x69:
			case 0x6B:
				ffd->ctx->frame_size = 1152;
				codec_id = CODEC_ID_MP2;
				break;
			}
		}
		ffd->codec = avcodec_find_decoder(codec_id);
	}
	/*should never happen*/
	if (!ffd->codec) return M4InvalidPlugin;

	/*setup MPEG-4 video streams*/
	if (ffd->st==M4ST_VISUAL) {
		/*for all MPEG-4 variants get size*/
		if ((ffd->oti==0x20) 
			/*FFMPEG should be able to init with the AVCDecoderConfigRecord we use as a DSI - cannot test
			it for now, FFMPEG CVS is broken and prev version doesn't support it ...*/
			|| (ffd->oti == 0x21)) {
			if (!decSpecInfoSize || !decSpecInfo) return M4NonCompliantBitStream;

			/*for regular MPEG-4, try to decode and if this fails try H263 decoder at first frame*/
			if (ffd->oti==0x20) {
				e = M4V_GetConfig(decSpecInfo, decSpecInfoSize, &dsi);
				if (e) return e;
				ffd->ctx->width = dsi.width;
				ffd->ctx->height = dsi.height;
				if (!dsi.width && !dsi.height) ffd->check_short_header = 1;
			}

			/*setup dsi for FFMPEG context BEFORE attaching decoder (otherwise not proper init)*/
			ffd->ctx->extradata = av_mallocz(decSpecInfoSize);
			memcpy(ffd->ctx->extradata, decSpecInfo, decSpecInfoSize);
			ffd->ctx->extradata_size = decSpecInfoSize;
		}
	}

	if (avcodec_open(ffd->ctx, ffd->codec)<0) return M4InvalidPlugin;

	/*setup audio streams*/
	if (ffd->st==M4ST_AUDIO) {
		if (!ffd->ctx->bits_per_sample) ffd->ctx->bits_per_sample = 16;
		if (!ffd->ctx->sample_rate) ffd->ctx->sample_rate = 16000;

		if ((ffd->codec->type == CODEC_ID_MP3LAME) || (ffd->codec->type == CODEC_ID_MP2)) {
			ffd->ctx->frame_size = (ffd->ctx->sample_rate > 24000) ? 1152 : 576;
		}
		ffd->out_size = ffd->ctx->bits_per_sample * ffd->ctx->channels * ffd->ctx->frame_size / 8;
		ffd->frame_size = 0;
	} else {
		/*output in YV12 only - let the player handle conversion*/
		ffd->out_size = ffd->ctx->width * ffd->ctx->height * 3;
		if (ffd->codec->id!=CODEC_ID_MJPEG) ffd->out_size /= 2;
	}

	switch (ffd->codec->id) {
	case CODEC_ID_MJPEG:
	case CODEC_ID_MJPEGB:
	case CODEC_ID_LJPEG:
		ffd->pix_fmt = M4PF_RGB_24; 
		break;
	default:
		ffd->pix_fmt = M4PF_YV12; 
		break;
	}

#if 0
	ffd->ctx->debug = FF_DEBUG_PICT_INFO | FF_DEBUG_BITSTREAM | FF_DEBUG_STARTCODE;
	av_log_set_level(AV_LOG_DEBUG);
#endif
	
	return M4OK;
}
static M4Err FFDEC_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	FFDec *ffd = (FFDec *)plug->privateStack;
	if (!ffd->ES_ID) return M4BadParam;
	ffd->ES_ID = 0;

	if (ffd->ctx) {
		avcodec_close(ffd->ctx);
		ffd->ctx = NULL;
	}
	return M4OK;
}

static M4Err FFDEC_GetCapabilities(BaseDecoder *plug, CapObject *capability)
{
	FFDec *ffd = (FFDec *)plug->privateStack;

	/*base caps*/
	switch (capability->CapCode) {
	/*ffmpeg seems quite reliable*/
	case CAP_HASRESILIENCE:
		capability->cap.valueINT = 1;
		return M4OK;
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = FF_INPUT_BUFFER_PADDING_SIZE;
		return M4OK;
	case CAP_VID_REORDER:
		capability->cap.valueINT = 1;
		return M4OK;
	}

	if (!ffd->ctx) {
		capability->cap.valueINT = 0;
		return M4OK;
	}

	/*caps valid only if stream attached*/
	switch (capability->CapCode) {
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = ffd->out_size;
		break;
	case CAP_SAMPLERATE:
		capability->cap.valueINT = ffd->ctx->sample_rate;
		break;
	case CAP_NBCHANNELS:
		capability->cap.valueINT = ffd->ctx->channels;
		break;
	case CAP_BITSPERSAMPLE:
		capability->cap.valueINT = ffd->ctx->bits_per_sample;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = (ffd->st==M4ST_AUDIO) ? 4 : 1;
		break;
	case CAP_BUFFER_MAX:
	  /*for audio let the systems engine decide since we may have very large block size (1 sec with some QT movies)*/
		capability->cap.valueINT = (ffd->st==M4ST_AUDIO) ? 0 : 4;
		break;
	/*by default AAC access unit lasts num_samples (timescale being sampleRate)*/
	case CAP_CU_DURATION:
		capability->cap.valueINT =  (ffd->st==M4ST_AUDIO) ? ffd->ctx->frame_size : 0;
		break;
	case CAP_WIDTH:
		capability->cap.valueINT = ffd->ctx->width;
		break;
	case CAP_HEIGHT:
		capability->cap.valueINT = ffd->ctx->height;
		break;
	case CAP_STRIDE:
		capability->cap.valueINT = (ffd->pix_fmt==M4PF_RGB_24) ? ffd->ctx->width*3 : ffd->ctx->width;
		break;
	case CAP_FPS:
		if (ffd->ctx->frame_rate_base) {
			capability->cap.valueFT = (Float) ffd->ctx->frame_rate;
			capability->cap.valueFT /= (Float) ffd->ctx->frame_rate_base;
		} else {
			capability->cap.valueFT = 30.0;
		}
		break;
	case CAP_COLORMODE:
		if (ffd->ctx->width) {
			capability->cap.valueINT = ffd->pix_fmt;
		}
		break;
	/*ffmpeg performs frame reordering internally*/
	case CAP_VID_REORDER:
		capability->cap.valueINT = 1;
		break;
	case CAP_CHANNEL_CONFIG:
		/*currently unused in ffmpeg*/
		if (ffd->ctx->channels==1) {
			capability->cap.valueINT = CHANNEL_FRONT_CENTER;
		} else {
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT;
		}
		break;
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}

static M4Err FFDEC_SetCapabilities(BaseDecoder *plug, CapObject capability)
{
	FFDec *ffd = (FFDec *)plug->privateStack;
	switch (capability.CapCode) {
	case CAP_WAIT_RAP:
		ffd->frame_start = 0;
		if (ffd->st==M4ST_VISUAL) avcodec_flush_buffers(ffd->ctx);
		return M4OK;
	default:
		/*return unsupported to avoid confusion by the player (like color space changing ...) */
		return M4NotSupported;
	}
}

static M4Err FFDEC_ProcessData(MediaDecoder *plug, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{

	s32 gotpic;
	u32 outsize;
	FFDec *ffd = plug->privateStack;

#if 0
	switch (mmlevel) {
	case MM_LEVEL_SEEK:
	case MM_LEVEL_DROP:
		/*skip as much as possible*/
		ffd->ctx->hurry_up = 5;
		break;
	case MM_LEVEL_VERY_LATE:
	case MM_LEVEL_LATE:
		/*skip B-frames*/
		ffd->ctx->hurry_up = 1;
		break;
	default:
		ffd->ctx->hurry_up = 0;
		break;
	}
#endif
	ffd->ctx->hurry_up = 0;

	/*audio stream*/
	if (ffd->st==M4ST_AUDIO) {
		s32 len;
		char *buf;
		u32 buf_size = (*outBufferLength);
		(*outBufferLength) = 0;

		/*seeking don't decode*/
		if (!inBuffer || (mmlevel == MM_LEVEL_SEEK)) {
			*outBufferLength = 0;
			ffd->frame_start = 0;
			return M4OK;
		}
		if (ffd->frame_start>inBufferLength) ffd->frame_start = 0;

		buf = malloc(sizeof(char) * AVCODEC_MAX_AUDIO_FRAME_SIZE);

redecode:
		len = avcodec_decode_audio(ffd->ctx, (short *)buf, &gotpic, inBuffer + ffd->frame_start, inBufferLength - ffd->frame_start);

		if (len<0) { free(buf); ffd->frame_start = 0; return M4NonCompliantBitStream; }
		if (gotpic<0) { free(buf); ffd->frame_start = 0; return M4OK; }

		/*in-band (re)config*/
		if (ffd->out_size < (u32) gotpic) {
			ffd->ctx->bits_per_sample = 16;
			/*looks like relying on frame_size is not a good idea for all codecs, so we use gotpic*/
			ffd->out_size = gotpic;
		}
		if (ffd->out_size > buf_size) {
			/*don't use too small output chunks otherwise we'll never have enough when mixing - we could 
			also request more slots in the composition memory but let's not waste mem*/
			if (ffd->out_size < (u32) 576*ffd->ctx->channels) ffd->out_size=ffd->ctx->channels*576;
			(*outBufferLength) = ffd->out_size;
			free(buf);
			return M4BufferTooSmall;
		}
		if (ffd->frame_size < (u32) gotpic) ffd->frame_size = gotpic;
		
		memcpy(outBuffer, buf, sizeof(char) * gotpic);
		free(buf);
		(*outBufferLength) += gotpic;
		outBuffer += gotpic;

		ffd->frame_start += len;
		if (inBufferLength <= ffd->frame_start) {
			ffd->frame_start = 0;
			return M4OK;
		}
		/*still space go on*/
		if ((*outBufferLength)+ffd->frame_size<ffd->out_size) goto redecode;

		/*more frames in the current sample*/
		return M4PackedFrames;
	} else {
		if (avcodec_decode_video(ffd->ctx, ffd->frame, &gotpic, inBuffer, inBufferLength) < 0) {
			if (!ffd->check_short_header) return M4NonCompliantBitStream;

			/*switch to H263 (ffmpeg MPEG-4 codec doesn't understand short headers)*/
			{
				u32 old_codec = ffd->codec->id;
				ffd->check_short_header = 0;
				/*OK we loose the DSI stored in the codec context, but H263 doesn't need any, and if we're
				here this means the DSI was broken, so no big deal*/
				avcodec_close(ffd->ctx);
				ffd->codec = avcodec_find_decoder(CODEC_ID_H263);
				if (!ffd->codec || (avcodec_open(ffd->ctx, ffd->codec)<0)) return M4NonCompliantBitStream;
				if (avcodec_decode_video(ffd->ctx, ffd->frame, &gotpic, inBuffer, inBufferLength) < 0) {
					/*nope, stay in MPEG-4*/
					avcodec_close(ffd->ctx);
					ffd->codec = avcodec_find_decoder(old_codec);
					assert(ffd->codec);
					avcodec_open(ffd->ctx, ffd->codec);
					return M4NonCompliantBitStream;
				}
			}
		}
		outsize = (u32) ffd->ctx->width * ffd->ctx->height * 3;
		if (ffd->pix_fmt!=M4PF_RGB_24) outsize /= 2;
		if (ffd->out_size != outsize) {
			ffd->out_size = outsize;
			*outBufferLength = ffd->out_size;
			return M4BufferTooSmall;
		}
		*outBufferLength = 0;
		if (mmlevel	== MM_LEVEL_SEEK) return M4OK;

		if (gotpic) {
			AVPicture pict;
			u32 pix_out;
			if (ffd->pix_fmt==M4PF_RGB_24) {
				pict.data[0] = outBuffer;
				pict.data[1] = outBuffer + ffd->ctx->width * ffd->ctx->height;
				pict.data[2] = outBuffer + ffd->ctx->width * ffd->ctx->height;
				pict.linesize[0] = pict.linesize[1] = pict.linesize[2] = ffd->ctx->width;
				pix_out = PIX_FMT_RGB24;
			} else {
				pict.data[0] = outBuffer;
				pict.data[1] = outBuffer + ffd->ctx->width * ffd->ctx->height;
				pict.data[2] = outBuffer + 5 * ffd->ctx->width * ffd->ctx->height / 4;
				pict.linesize[0] = ffd->ctx->width;
				pict.linesize[1] = pict.linesize[2] = ffd->ctx->width/2;
				pix_out = PIX_FMT_YUV420P;
			}
			img_convert(&pict, pix_out, (AVPicture *) ffd->frame, ffd->ctx->pix_fmt, ffd->ctx->width, ffd->ctx->height);
			*outBufferLength = ffd->out_size;
		}
	}
	return M4OK;
}

static Bool FFDEC_CanHandleStream(BaseDecoder *plug, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	BitStream *bs;
	u32 codec_id;
	Bool check_4cc;
	FFDec *ffd = plug->privateStack;

	if (!ObjectType) {
		if ((StreamType==M4ST_VISUAL) || (StreamType==M4ST_AUDIO)) return 1;
		return 0;
	}

	/*store types*/
	ffd->oti = ObjectType;
	ffd->st = StreamType;

	codec_id = 0;
	check_4cc = 0;
	
	/*private from FFMPEG input*/
	if (ObjectType == GPAC_FFMPEG_CODECS_OTI) {
		bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		codec_id = BS_ReadInt(bs, 32);
		DeleteBitStream(bs);
	}
	/*private from IsoMedia input*/
	else if (ObjectType == GPAC_QT_CODECS_OTI) {
		bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		codec_id = BS_ReadInt(bs, 32);
		check_4cc = 1;
		DeleteBitStream(bs);
	}
	/*std MPEG-4 audio*/
	else if (StreamType==M4ST_AUDIO) {
		if ((ObjectType==0x69) || (ObjectType==0x6B)) codec_id = CODEC_ID_MP2;
	} 
	/*std MPEG-4 visual*/
	else if (StreamType==M4ST_VISUAL) {
		switch (ObjectType) {
		/*MPEG-4 v1 simple profile*/
		case 0x20: codec_id = CODEC_ID_MPEG4; break;
		/*H264 (not std OTI, just the way we use it internally)*/
		case 0x21: codec_id = CODEC_ID_H264; break;
		/*MPEG1 video*/
		case 0x6A:
		/*MPEG2 video*/
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
			codec_id = CODEC_ID_MPEG2VIDEO; break;
		/*JPEG*/
		case 0x6C:
			return 0; /*I'm having troubles with ffmpeg & jpeg, it appears to crash randomly*/
			return 1;
		default:
			return 0;
		}
	}
	if (!codec_id) return 0;
	if (check_4cc && (ffmpeg_get_codec(codec_id) != NULL)) return 1;
	if (avcodec_find_decoder(codec_id) != NULL) return 1;
	return 0;
}

static char szCodec[100];
static const char *FFDEC_GetCodecName(BaseDecoder *dec)
{
	FFDec *ffd = dec->privateStack;
	if (ffd->codec) {
		sprintf(szCodec, "FFMPEG %s (v%s)", ffd->codec->name ? ffd->codec->name : "unknown", FFMPEG_VERSION);
		return szCodec;
	}
	return NULL;
}


void *FFDEC_Load()
{
	MediaDecoder *ptr;
	FFDec *priv;

    avcodec_init();
	avcodec_register_all();

	SAFEALLOC(ptr , sizeof(MediaDecoder));
	SAFEALLOC(priv , sizeof(FFDec));
	ptr->privateStack = priv;

	ptr->AttachStream = FFDEC_AttachStream;
	ptr->DetachStream = FFDEC_DetachStream;
	ptr->GetCapabilities = FFDEC_GetCapabilities;
	ptr->SetCapabilities = FFDEC_SetCapabilities;
	ptr->CanHandleStream = FFDEC_CanHandleStream;
	ptr->GetName = FFDEC_GetCodecName;
	ptr->ProcessData = FFDEC_ProcessData;

	M4_REG_PLUG(ptr, M4MEDIADECODERINTERFACE, "FFMPEG decoder", "gpac distribution", 0);
	return (BaseInterface *) ptr;
}

void FFDEC_Delete(void *ifce)
{
	BaseDecoder *dec = ifce;
	FFDec *ffd = dec->privateStack;

	if (ffd->ctx) avcodec_close(ffd->ctx);
	free(ffd);
	free(dec);

}
