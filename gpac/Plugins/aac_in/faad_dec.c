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

#ifdef M4_HAS_FAAD

#include <faad.h>

#include <gpac/m4_decoder.h>
#include <gpac/m4_author.h>

typedef struct
{
	faacDecHandle codec;
	faacDecFrameInfo info;
	u32 sample_rate, out_size, num_samples;
	u8 num_channels;
	/*no support for scalability in FAAD yet*/
	u16 ES_ID;
	Bool signal_mc;
	Bool is_sbr;
	
	char ch_reorder[16];
} FAADDec;


#define FAADCTX() FAADDec *ctx = (FAADDec *) ifcg->privateStack

static M4Err FAAD_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	M4ADecoderSpecificInfo a_cfg;
	FAADCTX();
	
	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;
	if (!decSpecInfoSize || !decSpecInfo) return M4NonCompliantBitStream;

	if (ctx->codec) faacDecClose(ctx->codec);
	ctx->codec = faacDecOpen();
	if (!ctx->codec) return M4IOErr;

	if (faacDecInit2(ctx->codec, (unsigned char *) decSpecInfo, decSpecInfoSize, (unsigned long *) &ctx->sample_rate, (u8 *) &ctx->num_channels) < 0)
		return M4NonCompliantBitStream;

	M4A_GetConfig((unsigned char *) decSpecInfo, decSpecInfoSize, &a_cfg);
	ctx->is_sbr = a_cfg.has_sbr;

	ctx->num_samples = 1024;
	ctx->out_size = 2 * ctx->num_samples * ctx->num_channels;
	ctx->ES_ID = ES_ID;
	ctx->signal_mc = ctx->num_channels>2 ? 1 : 0;
	return M4OK;
}

static M4Err FAAD_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	FAADCTX();
	if (ES_ID != ctx->ES_ID) return M4BadParam;
	if (ctx->codec) faacDecClose(ctx->codec);
	ctx->codec = NULL;
	ctx->ES_ID = 0;
	ctx->sample_rate = ctx->out_size = ctx->num_samples = 0;
	ctx->num_channels = 0;
	return M4OK;
}
static M4Err FAAD_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	u32 i;
	FAADCTX();
	switch (capability->CapCode) {
	/*not tested yet*/
	case CAP_HASRESILIENCE:
		capability->cap.valueINT = 1;
		break;
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = ctx->out_size;
		break;
	case CAP_SAMPLERATE:
		capability->cap.valueINT = ctx->sample_rate;
		break;
	case CAP_NBCHANNELS:
		capability->cap.valueINT = ctx->num_channels;
		break;
	case CAP_BITSPERSAMPLE:
		capability->cap.valueINT = 16;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = 4;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = 12;
		break;
	/*by default AAC access unit lasts num_samples (timescale being sampleRate)*/
	case CAP_CU_DURATION:
		capability->cap.valueINT = ctx->num_samples;
		break;
	/*to refine, it seems that 4 bytes padding is not enough on all streams ?*/
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 8;
		break;
	case CAP_CHANNEL_CONFIG:
		capability->cap.valueINT = 0;
		for (i=0; i<ctx->num_channels; i++) {
			switch (ctx->info.channel_position[i]) {
			case FRONT_CHANNEL_CENTER: capability->cap.valueINT |= CHANNEL_FRONT_CENTER; break;
			case FRONT_CHANNEL_LEFT: capability->cap.valueINT |= CHANNEL_FRONT_LEFT; break;
			case FRONT_CHANNEL_RIGHT: capability->cap.valueINT |= CHANNEL_FRONT_RIGHT; break;
			case SIDE_CHANNEL_LEFT: capability->cap.valueINT |= CHANNEL_SIDE_LEFT; break;
			case SIDE_CHANNEL_RIGHT: capability->cap.valueINT |= CHANNEL_SIDE_RIGHT; break;
			case BACK_CHANNEL_LEFT: capability->cap.valueINT |= CHANNEL_BACK_LEFT; break;
			case BACK_CHANNEL_RIGHT: capability->cap.valueINT |= CHANNEL_BACK_RIGHT; break;
			case BACK_CHANNEL_CENTER: capability->cap.valueINT |= CHANNEL_BACK_CENTER; break;
			case LFE_CHANNEL: capability->cap.valueINT |= CHANNEL_LFE; break;
			default: break;
			}
		}
		break;
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}
static M4Err FAAD_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like SR changing ...) */
	return M4NotSupported;
}

static s8 FAAD_GetChannelPos(FAADDec *ffd, u32 ch_cfg)
{
	u32 i;
	for (i=0; i<ffd->info.channels; i++) {
		switch (ffd->info.channel_position[i]) {
		case FRONT_CHANNEL_CENTER: if (ch_cfg==CHANNEL_FRONT_CENTER) return i; break;
		case FRONT_CHANNEL_LEFT: if (ch_cfg==CHANNEL_FRONT_LEFT) return i; break;
		case FRONT_CHANNEL_RIGHT: if (ch_cfg==CHANNEL_FRONT_RIGHT) return i; break;
		case SIDE_CHANNEL_LEFT: if (ch_cfg==CHANNEL_SIDE_LEFT) return i; break;
		case SIDE_CHANNEL_RIGHT: if (ch_cfg==CHANNEL_SIDE_RIGHT) return i; break;
		case BACK_CHANNEL_LEFT: if (ch_cfg==CHANNEL_BACK_LEFT) return i; break;
		case BACK_CHANNEL_RIGHT: if (ch_cfg==CHANNEL_BACK_RIGHT) return i; break;
		case BACK_CHANNEL_CENTER: if (ch_cfg==CHANNEL_BACK_CENTER) return i; break;
		case LFE_CHANNEL: if (ch_cfg==CHANNEL_LFE) return i; break;
		}
	}
	return -1;
}

static M4Err FAAD_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	void *buffer;
	unsigned short *conv_in, *conv_out;
	u32 i, j;
	FAADCTX();

	/*check not using scalabilty*/
	if (ctx->ES_ID != ES_ID) return M4BadParam;

	/*if late or seeking don't decode*/
	switch (mmlevel) {
	case MM_LEVEL_SEEK:
	case MM_LEVEL_DROP:
		*outBufferLength = 0;
		return M4OK;
	default:
		break;
	}

	if (ctx->out_size > *outBufferLength) {
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}

	buffer = faacDecDecode(ctx->codec, &ctx->info, inBuffer, inBufferLength);
	if (ctx->info.error>0) {
		*outBufferLength = 0;
		return M4NonCompliantBitStream;
	}
	/*FAAD froces us to decode a frame to get channel cfg*/
	if (ctx->signal_mc) {
		s32 ch, idx;
		ctx->signal_mc = 0;
		idx = 0;
		/*get cfg*/
		ch = FAAD_GetChannelPos(ctx, CHANNEL_FRONT_LEFT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_FRONT_RIGHT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_FRONT_CENTER);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_LFE);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_BACK_LEFT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_BACK_RIGHT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_BACK_CENTER);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_SIDE_LEFT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		ch = FAAD_GetChannelPos(ctx, CHANNEL_SIDE_RIGHT);
		if (ch>=0) { ctx->ch_reorder[idx] = ch; idx++; }
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}

	if (!ctx->info.samples || !buffer || !ctx->info.bytesconsumed) {
		*outBufferLength = 0;
		return M4OK;
	}

	if (sizeof(short) * ctx->info.samples > *outBufferLength) {
		*outBufferLength = sizeof(short)*ctx->info.samples; 
		return M4BufferTooSmall;
	} 
	/*we assume left/right order*/
	if (ctx->num_channels<=2) {
		memcpy(outBuffer, buffer, sizeof(short)* ctx->info.samples);
		*outBufferLength = sizeof(short)*ctx->info.samples;
		return M4OK;
	}
	conv_in = (unsigned short *) buffer;
	conv_out = (unsigned short *) outBuffer;
	for (i=0; i<ctx->info.samples; i+=ctx->info.channels) {
		for (j=0; j<ctx->info.channels; j++) {
			conv_out[i + j] = conv_in[i + ctx->ch_reorder[j]];
		}
	}
	*outBufferLength = sizeof(short)*ctx->info.samples;
	return M4OK;
}

static const char *FAAD_GetCodecName(BaseDecoder *ifcg)
{
	FAADCTX();
	if (ctx->is_sbr) return "FAAD2 " FAAD2_VERSION " SBR mode";
	return "FAAD2 " FAAD2_VERSION;
}

static Bool FAAD_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	/*audio decs*/	
	if (StreamType != M4ST_AUDIO) return 0;

	switch (ObjectType) {
	/*MPEG2 aac*/
	case 0x66:
	case 0x67:
	case 0x68:
	/*MPEG4 aac*/
	case 0x40: 
		return 1;
	/*cap query*/
	case 0:
		return 1;
	}
	return 0;
}

BaseDecoder *NewFAADDec()
{
	MediaDecoder *ifce;
	FAADDec *dec;

	SAFEALLOC(ifce, sizeof(MediaDecoder));
	SAFEALLOC(dec, sizeof(FAADDec));
	M4_REG_PLUG(ifce, M4MEDIADECODERINTERFACE, "FAAD2 Decoder", "gpac distribution", 0)

	ifce->privateStack = dec;

	/*setup our own interface*/	
	ifce->AttachStream = FAAD_AttachStream;
	ifce->DetachStream = FAAD_DetachStream;
	ifce->GetCapabilities = FAAD_GetCapabilities;
	ifce->SetCapabilities = FAAD_SetCapabilities;
	ifce->ProcessData = FAAD_ProcessData;
	ifce->CanHandleStream = FAAD_CanHandleStream;
	ifce->GetName = FAAD_GetCodecName;
	return (BaseDecoder *) ifce;
}

void DeleteFAADDec(BaseDecoder *ifcg)
{
	FAADCTX();
	if (ctx->codec) faacDecClose(ctx->codec);
	free(ctx);
	free(ifcg);
}


#endif

