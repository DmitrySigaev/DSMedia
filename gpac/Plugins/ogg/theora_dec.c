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

#ifdef M4_HAS_THEORA

#include <theora/theora.h>

typedef struct
{
    theora_info ti;
	theora_state td;
    theora_comment tc;
    ogg_packet op;
	
	u16 ES_ID;
	Bool has_reconfigured;
	u32 need_init;
} TheoraDec;

#define THEORACTX() TheoraDec *ctx = (TheoraDec *) ((OGGWraper *)ifcg->privateStack)->opaque

static M4Err THEO_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
    ogg_packet oggpacket;

	THEORACTX();
	if (ctx->ES_ID) return M4BadParam;
	
	if (!decSpecInfo) return M4NonCompliantBitStream;

	if (objectTypeIndication == GPAC_OGG_MEDIA_OTI) {
		if ( (decSpecInfoSize<7) || strncmp(&decSpecInfo[1], "theora", 6)) return M4NonCompliantBitStream;
	} else if (objectTypeIndication != 0xDF) return M4NonCompliantBitStream;

	oggpacket.granulepos = -1;
	oggpacket.b_o_s = 1;
	oggpacket.e_o_s = 0;
	oggpacket.packetno = 0;

	ctx->ES_ID = ES_ID;

    theora_info_init(&ctx->ti);
    theora_comment_init(&ctx->tc);


	if (objectTypeIndication == 0xDF) {
		BitStream *bs = NewBitStream(decSpecInfo, decSpecInfoSize, BS_READ);
		while (BS_Available(bs)) {
			oggpacket.bytes = BS_ReadInt(bs, 16);;
			oggpacket.packet = malloc(sizeof(char) * oggpacket.bytes);
			BS_ReadData(bs, oggpacket.packet, oggpacket.bytes);
			if (theora_decode_header(&ctx->ti, &ctx->tc, &oggpacket) < 0 ) {
				free(oggpacket.packet);
				DeleteBitStream(bs);
				return M4NonCompliantBitStream;
			}
			free(oggpacket.packet);
		}
        theora_decode_init(&ctx->td, &ctx->ti);
		ctx->need_init = 0;
		DeleteBitStream(bs);
		return M4OK;
	}

	oggpacket.packet = decSpecInfo;
	oggpacket.bytes = decSpecInfoSize;

	if (theora_decode_header(&ctx->ti, &ctx->tc, &oggpacket) < 0 ) return M4NonCompliantBitStream;

	/*need 3 ogg packets to init theora*/
	ctx->need_init = 3;

	return M4OK;
}

static M4Err THEO_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	THEORACTX();
	if (ctx->ES_ID != ES_ID) return M4BadParam;

	if (!ctx->need_init) theora_clear(&ctx->td); 
    theora_info_clear(&ctx->ti);
    theora_comment_clear(&ctx->tc);

	ctx->ES_ID = 0;
	return M4OK;
}
static M4Err THEO_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	THEORACTX();
	switch (capability->CapCode) {
	case CAP_WIDTH:
		capability->cap.valueINT = ctx->ti.width;
		break;
	case CAP_HEIGHT:
		capability->cap.valueINT = ctx->ti.height;
		break;
	case CAP_STRIDE:
		capability->cap.valueINT = ctx->ti.width;
		break;
	case CAP_FPS:
		capability->cap.valueFT = (Float) ctx->ti.fps_numerator;
		capability->cap.valueFT /= ctx->ti.fps_denominator;
		break;
	case CAP_COLORMODE:
		capability->cap.valueINT = M4PF_YV12;
		break;
	case CAP_VID_SHAPE:
		capability->cap.valueINT = 0;
		break;
	case CAP_VID_REORDER:
		capability->cap.valueINT = 0;
		break;
	case CAP_HASRESILIENCE:
		capability->cap.valueINT = 1;
		break;
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = 3*ctx->ti.width * ctx->ti.height / 2;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = 1;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = 4;
		break;
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 0;
		break;
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}

static M4Err THEO_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like SR changing ...) */
	return M4NotSupported;
}


static M4Err THEO_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	ogg_packet op;
	yuv_buffer yuv;
	u32 i;
	char *pYO, *pUO, *pVO;
	unsigned char *pYD, *pUD, *pVD;

	THEORACTX();
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
				theora_info_clear(&ctx->ti) ;
				theora_comment_clear(&ctx->tc) ;
				theora_info_init(&ctx->ti);
				theora_comment_init(&ctx->tc);
				if (theora_decode_header(&ctx->ti, &ctx->tc, &op) < 0 ) return M4NonCompliantBitStream;

				ctx->has_reconfigured = 1;
				*outBufferLength = 3 * ctx->ti.width*ctx->ti.height/2;
				return M4BufferTooSmall;
			}
			ctx->has_reconfigured = 0;
			ctx->need_init = 2;
			return M4OK;
		} else {
			if (theora_decode_header(&ctx->ti, &ctx->tc, &op) < 0 ) return M4NonCompliantBitStream;
		}
		ctx->need_init--;
		if (!ctx->need_init) {
            theora_decode_init(&ctx->td, &ctx->ti);
			theora_decode_packetin(&ctx->td, &op);
		}
		return M4OK;
	}

    if (theora_decode_packetin(&ctx->td, &op)<0) return M4NonCompliantBitStream;
	if (mmlevel	== MM_LEVEL_SEEK) return M4OK;
    if (theora_decode_YUVout(&ctx->td, &yuv)<0) return M4OK;

	pYO = yuv.y;
	pUO = yuv.u;
	pVO = yuv.v;
	pYD = outBuffer;
	pUD = outBuffer + ctx->ti.width * ctx->ti.height;
	pVD = outBuffer + 5 * ctx->ti.width * ctx->ti.height / 4;
	
	for (i=0; i<(u32)yuv.y_height; i++) {
		memcpy(pYD, pYO, sizeof(char) * yuv.y_width);
		pYD += ctx->ti.width;
		pYO += yuv.y_stride;
		if (i%2) continue;

		memcpy(pUD, pUO, sizeof(char) * yuv.uv_width);
		memcpy(pVD, pVO, sizeof(char) * yuv.uv_width);
		pUD += ctx->ti.width/2;
		pVD += ctx->ti.width/2;
		pUO += yuv.uv_stride;
		pVO += yuv.uv_stride;
	}
	*outBufferLength = 3*ctx->ti.width*ctx->ti.height/2;
	return M4OK;
}



static const char *THEO_GetCodecName(BaseDecoder *dec)
{
	return "Theora Decoder";
}

u32 NewTheoraDecoder(BaseDecoder *ifcd)
{
	TheoraDec *dec;
	SAFEALLOC(dec, sizeof(TheoraDec));
	((OGGWraper *)ifcd->privateStack)->opaque = dec;
	((OGGWraper *)ifcd->privateStack)->type = OGG_THEORA;
	/*setup our own interface*/	
	ifcd->AttachStream = THEO_AttachStream;
	ifcd->DetachStream = THEO_DetachStream;
	ifcd->GetCapabilities = THEO_GetCapabilities;
	ifcd->SetCapabilities = THEO_SetCapabilities;
	((MediaDecoder*)ifcd)->ProcessData = THEO_ProcessData;
	ifcd->GetName = THEO_GetCodecName;
	return 1;
}

void DeleteTheoraDecoder(BaseDecoder *ifcg)
{
	THEORACTX();
	free(ctx);
}

#endif

