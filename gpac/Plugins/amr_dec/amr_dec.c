/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / AMR decoder plugin
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



/*decoder Interface*/
#include <gpac/m4_decoder.h>
#include <gpac/m4_network.h>
#include <gpac/m4_descriptors.h>

#define MMS_IO

/*AMR NB*/
#include "AMR_NB/sp_dec.h"
#include "AMR_NB/d_homing.h"

/*default size in CU of composition memory for audio*/
#define DEFAULT_AUDIO_CM_SIZE			12
/*default critical size in CU of composition memory for audio*/
#define DEFAULT_AUDIO_CM_TRIGGER		4

typedef struct
{
	Bool is_amr_wb;
	u32 sample_rate, out_size, num_samples;
	u8 num_channels;
	/*no support for scalability in AMR*/
	u16 ES_ID;
	u32 cb_size, cb_trig;

	/*AMR NB state vars*/
	Speech_Decode_FrameState * speech_decoder_state;
	u32 frame_count;
    enum RXFrameType rx_type;
    enum Mode mode;
    Word16 reset_flag;
    Word16 reset_flag_old;
} AMRDec;

static const u32 amr_nb_frame_size[16] = {
	12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};

static const u32 amr_wb_frame_size[16] = {
	17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 
};

#define AMRCTX() AMRDec *ctx = (AMRDec *) ifcg->privateStack


static M4Err AMR_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	BitStream *bs;
	char name[5];
	u32 codec_4cc;
	AMRCTX();
	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;

	ctx->is_amr_wb = 0;

	bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
	codec_4cc = BS_ReadInt(bs, 32);
	DeleteBitStream(bs);
	name[4] = 0;
	MP4TypeToString(codec_4cc, name);

	if (!stricmp(name, "sawb")) ctx->is_amr_wb = 1;

	ctx->frame_count=0;
	ctx->reset_flag = 0;
	ctx->reset_flag_old = 1;
	ctx->mode = 0;
	ctx->rx_type = 0;
	ctx->speech_decoder_state = NULL;
	if (Speech_Decode_Frame_init(&ctx->speech_decoder_state, "Decoder")) {
	  return M4IOErr;
	}

	/*we need a frame to init, so use default values*/
	ctx->num_samples = 160;
	ctx->num_channels = 1;
	ctx->sample_rate = ctx->is_amr_wb ? 16000 : 8000;
	
	/*max possible frames in a sample are seen in MP4, that's 15*/
	ctx->out_size = 15 * 2 * ctx->num_samples * ctx->num_channels;
	ctx->ES_ID = ES_ID;

	return M4OK;
}

static M4Err AMR_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	AMRCTX();
	if (ES_ID != ctx->ES_ID) return M4BadParam;
	Speech_Decode_Frame_exit(&ctx->speech_decoder_state);
	ctx->speech_decoder_state = NULL;
	ctx->ES_ID = 0;
	ctx->sample_rate = ctx->out_size = ctx->num_samples = 0;
	ctx->num_channels = 0;
	return M4OK;
}
static M4Err AMR_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	AMRCTX();
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

static M4Err AMR_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like SR changing ...) */
	return M4NotSupported;
}


/* frame size in serial bitstream file (frame type + serial stream + flags) */
#define SERIAL_FRAMESIZE (1+MAX_SERIAL_SIZE+5)

static M4Err AMR_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
    u32 offset = 0;
    UWord8 toc, q, ft;
    Word16 serial[SERIAL_FRAMESIZE];
    Word16 *synth;
    UWord8 *packed_bits;
    s32 i;
	AMRCTX();

	/*not using scalabilty*/
	assert(ctx->ES_ID == ES_ID);

	/*if late or seeking don't decode (each frame is a RAP)*/
	/*	switch (mmlevel) {
	case MM_LEVEL_SEEK:
	case MM_LEVEL_DROP:
		*outBufferLength = 0;
		return M4OK;
	default:
		break;
	}
	*/
	if (ctx->out_size > *outBufferLength) {
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}
	
    synth = (Word16 *) outBuffer;
	*outBufferLength = 0;

    while (offset < inBufferLength) {
        toc = inBuffer[offset];
        /* read rest of the frame based on ToC byte */
        q = (toc >> 2) & 0x01;
        ft = (toc >> 3) & 0x0F;
        offset++;
        packed_bits = inBuffer + offset;
        offset += amr_nb_frame_size[ft];

        /*Unsort and unpack bits*/
        ctx->rx_type = UnpackBits(q, ft, packed_bits, &ctx->mode, &serial[1]);
        ctx->frame_count++;

        if (ctx->rx_type == RX_NO_DATA) {
            ctx->mode = ctx->speech_decoder_state->prev_mode;
        } else {
            ctx->speech_decoder_state->prev_mode = ctx->mode;
        }
        
        /* if homed: check if this frame is another homing frame */
        if (ctx->reset_flag_old == 1) {
            /* only check until end of first subframe */
            ctx->reset_flag = decoder_homing_frame_test_first(&serial[1], ctx->mode);
        }
        /* produce encoder homing frame if homed & input=decoder homing frame */
        if ((ctx->reset_flag != 0) && (ctx->reset_flag_old != 0)) {
            for (i = 0; i < L_FRAME; i++) {
                synth[i] = EHF_MASK;
            }
        } else {     
            /* decode frame */
            Speech_Decode_Frame(ctx->speech_decoder_state, ctx->mode, &serial[1], ctx->rx_type, synth);
        }

        *outBufferLength += 160*2;
        synth += 160;
	if (*outBufferLength > ctx->out_size) return M4NonCompliantBitStream;
        
        /* if not homed: check whether current frame is a homing frame */
        if (ctx->reset_flag_old == 0) {
            ctx->reset_flag = decoder_homing_frame_test(&serial[1], ctx->mode);
        }
        /* reset decoder if current frame is a homing frame */
        if (ctx->reset_flag != 0) {
            Speech_Decode_Frame_reset(ctx->speech_decoder_state);
        }
        ctx->reset_flag_old = ctx->reset_flag;
    }
	return M4OK;
}


static u32 AMR_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	BitStream *bs;
	char name[5];
	u32 codec_4cc;

	/*we handle audio only*/
	if (!ObjectType) return (StreamType==M4ST_AUDIO) ? 1 : 0;

	/*audio dec*/
	if (!decSpecInfo || (StreamType != M4ST_AUDIO) || (ObjectType != GPAC_QT_CODECS_OTI)) return 0;
	bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
	codec_4cc = BS_ReadInt(bs, 32);
	DeleteBitStream(bs);

	name[4] = 0;
	MP4TypeToString(codec_4cc, name);

	if (!stricmp(name, "samr") || !stricmp(name, "amr ")) return 1;
	/*TODO, AMR_WB*/
	else if (!stricmp(name, "sawb")) return 0;
	
	return 0;
}


static const char *AMR_GetCodecName(BaseDecoder *dec)
{
	return "3GPP AMR NB";
}

MediaDecoder *NewAMRDecoder()
{
	AMRDec *dec;
	MediaDecoder *ifce;
	SAFEALLOC(ifce , sizeof(MediaDecoder));
	dec = malloc(sizeof(AMRDec));
	memset(dec, 0, sizeof(AMRDec));
	ifce->privateStack = dec;
	ifce->CanHandleStream = AMR_CanHandleStream;

	dec->cb_size = DEFAULT_AUDIO_CM_SIZE;
	dec->cb_trig = DEFAULT_AUDIO_CM_TRIGGER;

	/*setup our own interface*/	
	ifce->AttachStream = AMR_AttachStream;
	ifce->DetachStream = AMR_DetachStream;
	ifce->GetCapabilities = AMR_GetCapabilities;
	ifce->SetCapabilities = AMR_SetCapabilities;
	ifce->ProcessData = AMR_ProcessData;
	ifce->GetName = AMR_GetCodecName;

	M4_REG_PLUG(ifce, M4MEDIADECODERINTERFACE, "AMR 3GPP decoder", "gpac distribution", 0);

	return ifce;
}

void DeleteAMRDecoder(BaseDecoder *ifcg)
{
	AMRCTX();
	if (ctx->speech_decoder_state) Speech_Decode_Frame_exit(&ctx->speech_decoder_state);
	free(ctx);
	free(ifcg);
}

Bool QueryInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4MEDIADECODERINTERFACE: return 1;
	case M4STREAMINGCLIENT: return 1;
	default:
		return 0;
	}
}


NetClientPlugin *NewAMRReader();
void DeleteAMRReader(void *ifce);

void *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4MEDIADECODERINTERFACE: return NewAMRDecoder();
	case M4STREAMINGCLIENT: return NewAMRReader();
	default: return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4MEDIADECODERINTERFACE:
		DeleteAMRDecoder(ifcd);
		break;
	case M4STREAMINGCLIENT: 
		DeleteAMRReader(ifce);
		break;
	}
}
