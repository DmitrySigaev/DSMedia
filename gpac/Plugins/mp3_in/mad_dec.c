/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / codec pack plugin
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

#ifdef M4_HAS_MAD

#include <gpac/m4_descriptors.h>
#include <gpac/m4_decoder.h>

#ifdef _WIN32_WCE
#ifndef FPM_DEFAULT 
#define FPM_DEFAULT 
#endif
#endif

#include "mad.h"

typedef struct
{
	Bool configured;

	u32 sample_rate, out_size, num_samples;
	u8 num_channels;
	/*no support for scalability in FAAD yet*/
	u16 ES_ID;
	u32 cb_size, cb_trig;

	unsigned char *buffer;
	u32 len;
	Bool first;


	struct mad_frame frame;
	struct mad_stream stream;
	struct mad_synth synth;

} MADDec;


#define MADCTX() MADDec *ctx = (MADDec *) ifcg->privateStack


static M4Err MAD_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	MADCTX();
	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;

	if (ctx->configured) {
		mad_stream_finish(&ctx->stream);
		mad_frame_finish(&ctx->frame);
		mad_synth_finish(&ctx->synth);
	}
	mad_stream_init(&ctx->stream);
	mad_frame_init(&ctx->frame);
	mad_synth_init(&ctx->synth);
	ctx->configured = 1;
	
	ctx->buffer = malloc(sizeof(char) * 2*MAD_BUFFER_MDLEN);
	
	/*we need a frame to init, so use default values*/
	ctx->num_samples = 1152;
	ctx->num_channels = 0;
	ctx->sample_rate = 0;
	ctx->out_size = 2 * ctx->num_samples * ctx->num_channels;
	ctx->ES_ID = ES_ID;
	ctx->first = 1;
	return M4OK;
}

static M4Err MAD_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	MADCTX();
	if (ES_ID != ctx->ES_ID) return M4BadParam;
	ctx->ES_ID = 0;
	if (ctx->buffer) free(ctx->buffer);
	ctx->buffer = NULL;
	ctx->sample_rate = ctx->out_size = ctx->num_samples = 0;
	ctx->num_channels = 0;
	if (ctx->configured) {
		mad_stream_finish(&ctx->stream);
		mad_frame_finish(&ctx->frame);
		mad_synth_finish(&ctx->synth);
	}
	ctx->configured = 0;
	return M4OK;
}
static M4Err MAD_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	MADCTX();
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
		capability->cap.valueINT = ctx->cb_trig;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = ctx->cb_size;
		break;
	/*FIXME: get exact sampling window*/
	case CAP_CU_DURATION:
		capability->cap.valueINT = ctx->num_samples;
		break;
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 0;
		break;
	case CAP_CHANNEL_CONFIG:
		if (ctx->num_channels==1) {
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

static M4Err MAD_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	MADCTX();
	switch (capability.CapCode) {
	/*reset storage buffer*/
	case CAP_WAIT_RAP:
		ctx->first = 1;
		ctx->len = 0;
		if (ctx->configured) {
			mad_stream_finish(&ctx->stream);
			mad_frame_finish(&ctx->frame);
			mad_synth_finish(&ctx->synth);

			mad_stream_finish(&ctx->stream);
			mad_frame_finish(&ctx->frame);
			mad_synth_finish(&ctx->synth);
		}
		return M4OK;
	default:
		/*return unsupported to avoid confusion by the player (like SR changing ...) */
		return M4NotSupported;
	}
}

/*from miniMad.c*/
#define MAD_SCALE(ret, s_chan)	\
	chan = s_chan;				\
	chan += (1L << (MAD_F_FRACBITS - 16));		\
	if (chan >= MAD_F_ONE)					\
		chan = MAD_F_ONE - 1;					\
	else if (chan < -MAD_F_ONE)				\
		chan = -MAD_F_ONE;				\
	ret = chan >> (MAD_F_FRACBITS + 1 - 16);		\

static M4Err MAD_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	mad_fixed_t *left_ch, *right_ch, chan;
	char *ptr;
	u32 num, outSize;
	MADCTX();

	/*check not using scalabilty*/
	assert(ctx->ES_ID == ES_ID);

	if (ctx->ES_ID != ES_ID) 
		return M4BadParam;

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

	if (ctx->first) {
		ctx->first = 0;
		memcpy(ctx->buffer, inBuffer, inBufferLength);
		ctx->len = inBufferLength;
		*outBufferLength = 0;
		return M4OK;
	}
	memcpy(ctx->buffer + ctx->len, inBuffer, inBufferLength);
	ctx->len += inBufferLength;
	mad_stream_buffer(&ctx->stream, ctx->buffer, ctx->len);

	if (mad_frame_decode(&ctx->frame, &ctx->stream) == -1) {
		memcpy(ctx->buffer, inBuffer, inBufferLength);
		ctx->len = inBufferLength;
		*outBufferLength = 0;
		return M4OK;
	}


	/*first cfg, reconfig composition buffer*/
	if (!ctx->sample_rate) {
		mad_synth_frame(&ctx->synth, &ctx->frame);
		ctx->len -= inBufferLength;
		ctx->sample_rate = ctx->synth.pcm.samplerate;
		ctx->num_channels = (u8) ctx->synth.pcm.channels;
		ctx->num_samples = ctx->synth.pcm.length;
		ctx->out_size = 2 * ctx->num_samples * ctx->num_channels;
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}
	
	if (ctx->stream.next_frame) {
		ctx->len = &ctx->buffer[ctx->len] - ctx->stream.next_frame;
	    memmove(ctx->buffer, ctx->stream.next_frame, ctx->len);
	}
	

	mad_synth_frame(&ctx->synth, &ctx->frame);
	num = ctx->synth.pcm.length;
	ptr = (char *) outBuffer;
	left_ch = ctx->synth.pcm.samples[0];
	right_ch = ctx->synth.pcm.samples[1];
	outSize = 0;

	while (num--) {
		s32 rs;
		MAD_SCALE(rs, (*left_ch++) );

		*ptr = (rs >> 0) & 0xff;
		ptr++;
		*ptr = (rs >> 8) & 0xff;
		ptr++;
		outSize += 2;

		if (ctx->num_channels == 2) {
			MAD_SCALE(rs, (*right_ch++) );
			*ptr = (rs >> 0) & 0xff;
			ptr++;
			*ptr = (rs >> 8) & 0xff;
			ptr++;
			outSize += 2;
		}
	}
	*outBufferLength = outSize;
	return M4OK;
}

static const char *MAD_GetCodecName(BaseDecoder *dec)
{
	return "MAD " \
		MAD_VERSION;
}

static Bool MAD_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	/*audio decs*/	
	if (StreamType != M4ST_AUDIO) return 0;

	switch (ObjectType) {
	/*MPEG1 audio*/
	case 0x69:
	/*MPEG2 audio*/
	case 0x6B:
		return 1;
	/*cap query*/
	case 0:
		return 1;
	}
	return 0;
}

BaseDecoder *NewMADDec()
{
	MediaDecoder *ifce;
	MADDec *dec;
	
	SAFEALLOC(ifce, sizeof(MediaDecoder));
	SAFEALLOC(dec, sizeof(MADDec));
	M4_REG_PLUG(ifce, M4MEDIADECODERINTERFACE, "MAD Decoder", "gpac distribution", 0)
	ifce->privateStack = dec;

	dec->cb_size = 12;
	dec->cb_trig = 4;

	/*setup our own interface*/	
	ifce->AttachStream = MAD_AttachStream;
	ifce->DetachStream = MAD_DetachStream;
	ifce->GetCapabilities = MAD_GetCapabilities;
	ifce->SetCapabilities = MAD_SetCapabilities;
	ifce->GetName = MAD_GetCodecName;
	ifce->ProcessData = MAD_ProcessData;
	ifce->CanHandleStream = MAD_CanHandleStream;
	return (BaseDecoder *)ifce;
}

void DeleteMADDec(MediaDecoder *ifcg)
{
	MADCTX();
	if (ctx->configured) {
		mad_stream_finish(&ctx->stream);
		mad_frame_finish(&ctx->frame);
		mad_synth_finish(&ctx->synth);
	}
	free(ctx);
	free(ifcg);
}

#endif
