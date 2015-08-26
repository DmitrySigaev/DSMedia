/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / XIPH.org plugin
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


#include "ogg_in.h"

#ifdef M4_HAS_VORBIS

#include <vorbis/codec.h>

typedef struct
{
    vorbis_info vi;
	vorbis_dsp_state vd;
	vorbis_block vb;
    vorbis_comment vc;
    ogg_packet op;

	u16 ES_ID;
	Bool has_reconfigured;
	u32 need_init;
} VorbDec;

#define VORBISCTX() VorbDec *ctx = (VorbDec *) ((OGGWraper *)ifcg->privateStack)->opaque

static M4Err VORB_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
    ogg_packet oggpacket;

	VORBISCTX();
	if (ctx->ES_ID) return M4BadParam;
	
	if (!decSpecInfo || !decSpecInfoSize) return M4NonCompliantBitStream;
	if (objectTypeIndication == GPAC_OGG_MEDIA_OTI) {
		if ((decSpecInfoSize<7) || strncmp(&decSpecInfo[1], "vorbis", 6)) return M4NonCompliantBitStream;
	} else if (objectTypeIndication != 0xDE) return M4NonCompliantBitStream;

	ctx->ES_ID = ES_ID;

    vorbis_info_init(&ctx->vi);
    vorbis_comment_init(&ctx->vc);

	oggpacket.granulepos = -1;
	oggpacket.b_o_s = 1;
	oggpacket.e_o_s = 0;
	oggpacket.packetno = 0;

	if (objectTypeIndication == 0xDE) {
		BitStream *bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		while (BS_Available(bs)) {
			oggpacket.bytes = BS_ReadInt(bs, 16);;
			oggpacket.packet = malloc(sizeof(char) * oggpacket.bytes);
			BS_ReadData(bs, oggpacket.packet, oggpacket.bytes);
			if (vorbis_synthesis_headerin(&ctx->vi, &ctx->vc, &oggpacket) < 0 ) {
				free(oggpacket.packet);
				DeleteBitStream(bs);
				return M4NonCompliantBitStream;
			}
			free(oggpacket.packet);
		}
		vorbis_synthesis_init(&ctx->vd, &ctx->vi);
		vorbis_block_init(&ctx->vd, &ctx->vb); 
		ctx->need_init = 0;
		DeleteBitStream(bs);


		fprintf(stdout, "Vorbis min block size %d\n", vorbis_info_blocksize(&ctx->vi, 0));
		fprintf(stdout, "Vorbis max block size %d\n", vorbis_info_blocksize(&ctx->vi, 1));
	
		return M4OK;
	}

	oggpacket.packet = decSpecInfo;
	oggpacket.bytes = decSpecInfoSize;

	if (vorbis_synthesis_headerin(&ctx->vi, &ctx->vc, &oggpacket) < 0 ) return M4NonCompliantBitStream;


	/*need 3 ogg packets to init vorbis*/
	ctx->need_init = 3;

	ctx->ES_ID = ES_ID;
	return M4OK;
}

static M4Err VORB_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	VORBISCTX();
	if (ctx->ES_ID != ES_ID) return M4BadParam;

	if (!ctx->need_init) {
		vorbis_block_clear(&ctx->vb); 
        vorbis_dsp_clear(&ctx->vd);
	}
    vorbis_info_clear(&ctx->vi);
    vorbis_comment_clear(&ctx->vc);

	ctx->ES_ID = 0;
	return M4OK;
}
static M4Err VORB_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	VORBISCTX();
	switch (capability->CapCode) {
	case CAP_HASRESILIENCE:
		capability->cap.valueINT = 1;
		break;
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = vorbis_info_blocksize(&ctx->vi, 1) * 2 * ctx->vi.channels;
		break;
	case CAP_SAMPLERATE:
		capability->cap.valueINT = ctx->vi.rate;
		break;
	case CAP_NBCHANNELS:
		capability->cap.valueINT = ctx->vi.channels;
		break;
	case CAP_BITSPERSAMPLE:
		capability->cap.valueINT = 16;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = 4;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = ctx->vi.rate / 4 / vorbis_info_blocksize(&ctx->vi, 0);
		/*blocks are not of fixed size, so indicate a default CM size*/
		//capability->cap.valueINT = 12 * vorbis_info_blocksize(&ctx->vi, 1) / vorbis_info_blocksize(&ctx->vi, 0);
		break;
	case CAP_CU_DURATION:
		/*this CANNOT work with vorbis, blocks are not of fixed size*/
		capability->cap.valueINT = 0;
		break;
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 0;
		break;
	case CAP_CHANNEL_CONFIG:
		switch (ctx->vi.channels) {
		case 1: capability->cap.valueINT = CHANNEL_FRONT_CENTER; break;
		case 2:
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT;
			break;
		case 3:
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT | CHANNEL_FRONT_CENTER;
			break;
		case 4:
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT | CHANNEL_BACK_LEFT | CHANNEL_BACK_RIGHT;
			break;
		case 5:
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT | CHANNEL_FRONT_CENTER | CHANNEL_BACK_LEFT | CHANNEL_BACK_RIGHT;
			break;
		case 6:
			capability->cap.valueINT = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT | CHANNEL_FRONT_CENTER | CHANNEL_BACK_LEFT | CHANNEL_BACK_RIGHT | CHANNEL_LFE;
			break;
		}
		break;
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}

static M4Err VORB_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like SR changing ...) */
	return M4NotSupported;
}


static M4INLINE void vorbis_to_intern(u32 samples, Float **pcm, char *buf, u32 channels) 
{
	u32 i, j;
	s32 val;
	ogg_int16_t *ptr, *data = (ogg_int16_t*)buf ;
	Float *mono;
 
    for (i=0 ; i<channels ; i++) {
		ptr = &data[i];
		if (channels>2) {
			/*center is third in gpac*/
			if (i==1) ptr = &data[2];
			/*right is 2nd in gpac*/
			else if (i==2) ptr = &data[1];
			/*LFE is 4th in gpac*/
			if ((channels==6) && (i>3)) {
				if (i==6) ptr = &data[4];	/*LFE*/
				else ptr = &data[i+1];	/*back l/r*/
			}
		}

		mono = pcm[i];
		for (j=0; j<samples; j++) { 
			val = (s32) (mono[j] * 32767.f);
			if (val > 32767) val = 32767;
			if (val < -32768) val = -32768;
			*ptr = val;
			ptr += channels;
		}
    }    
}

static M4Err VORB_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	ogg_packet op;
    Float **pcm;
    u32 samples, total_samples, total_bytes;

	VORBISCTX();
	/*not using scalabilty*/
	assert(ctx->ES_ID == ES_ID);

	op.granulepos = -1;
	op.b_o_s = 0;
	op.e_o_s = 0;
	op.packetno = 0;
    op.packet = inBuffer;
    op.bytes = inBufferLength;


	*outBufferLength = 0;


	if (ctx->need_init) {
		if(ctx->need_init==3) {
			if (!ctx->has_reconfigured) {
				op.b_o_s = 1;
				vorbis_info_clear(&ctx->vi) ;
				vorbis_comment_clear(&ctx->vc) ;
				vorbis_info_init(&ctx->vi);
				vorbis_comment_init(&ctx->vc);
				if (vorbis_synthesis_headerin(&ctx->vi, &ctx->vc, &op)<0) return M4NonCompliantBitStream;
				ctx->has_reconfigured = 1;
				*outBufferLength = vorbis_info_blocksize(&ctx->vi, 1) * 2 * ctx->vi.channels;
				return M4BufferTooSmall;
			}
			ctx->has_reconfigured = 0;
			ctx->need_init = 2;
			return M4OK;
		} else {
			if (vorbis_synthesis_headerin(&ctx->vi, &ctx->vc, &op)<0) return M4NonCompliantBitStream;
		}
		ctx->need_init--;
		if (!ctx->need_init) {
			vorbis_synthesis_init(&ctx->vd, &ctx->vi);
			vorbis_block_init(&ctx->vd, &ctx->vb); 
		}
		return M4OK;
	}

	if (vorbis_synthesis(&ctx->vb, &op) == 0) 
		vorbis_synthesis_blockin(&ctx->vd, &ctx->vb) ;

	/*trust vorbis max block info*/
	total_samples = 0;
	total_bytes = 0;
	while ((samples = vorbis_synthesis_pcmout(&ctx->vd, &pcm)) > 0) {
		vorbis_to_intern(samples, pcm, (char*) outBuffer + total_bytes, ctx->vi.channels);
		total_bytes += samples * 2 * ctx->vi.channels;
		total_samples += samples;
		vorbis_synthesis_read(&ctx->vd, samples);
	}
	*outBufferLength = total_bytes;   
	return M4OK;
}


static const char *VORB_GetCodecName(BaseDecoder *dec)
{
	return "Vorbis Decoder";
}

u32 NewVorbisDecoder(BaseDecoder *ifcd)
{
	VorbDec *dec;
	SAFEALLOC(dec, sizeof(VorbDec));
	((OGGWraper *)ifcd->privateStack)->opaque = dec;
	((OGGWraper *)ifcd->privateStack)->type = OGG_VORBIS;
	/*setup our own interface*/	
	ifcd->AttachStream = VORB_AttachStream;
	ifcd->DetachStream = VORB_DetachStream;
	ifcd->GetCapabilities = VORB_GetCapabilities;
	ifcd->SetCapabilities = VORB_SetCapabilities;
	((MediaDecoder*)ifcd)->ProcessData = VORB_ProcessData;
	ifcd->GetName = VORB_GetCodecName;
	return 1;
}

void DeleteVorbisDecoder(BaseDecoder *ifcg)
{
	VORBISCTX();
	free(ctx);
}

#endif

