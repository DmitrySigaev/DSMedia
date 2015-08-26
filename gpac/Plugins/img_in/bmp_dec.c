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


#include "img_in.h"


typedef struct
{
	u16 ES_ID;
	u32 width, height, out_size, pixel_format;
} BMPDec;
	
#define BMPCTX()	BMPDec *ctx = (BMPDec *) ((IMGDec *)ifcg->privateStack)->opaque


static M4Err BMP_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	BMPCTX();
	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;
	ctx->ES_ID = ES_ID;
	return M4OK;
}
static M4Err BMP_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	BMPCTX();
	if (ctx->ES_ID != ES_ID) return M4BadParam;
	ctx->ES_ID = ES_ID;
	return M4OK;
}
static M4Err BMP_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	BMPCTX();
	switch (capability->CapCode) {
	case CAP_WIDTH:
		capability->cap.valueINT = ctx->width;
		break;
	case CAP_HEIGHT:
		capability->cap.valueINT = ctx->height;
		break;
	case CAP_STRIDE:
		capability->cap.valueINT = (ctx->pixel_format == M4PF_RGB_24) ? 3 : 4;
		capability->cap.valueINT *= ctx->width;
		break;
	case CAP_FPS:
		capability->cap.valueFT = 0;
		break;
	case CAP_COLORMODE:
		capability->cap.valueINT = ctx->pixel_format;
		break;
	case CAP_OUTPUTSIZE:
		capability->cap.valueINT = 	ctx->out_size;
		break;
	case CAP_BUFFER_MIN:
		capability->cap.valueINT = 0;
		break;
	case CAP_BUFFER_MAX:
		capability->cap.valueINT = 1;
		break;
	/*no shape coding*/
	case CAP_VID_SHAPE:
		capability->cap.valueINT = 0;
		break;
	case CAP_PADDING_BYTES:
		capability->cap.valueINT = 0;
		break;
	default:
		return M4NotSupported;
	}
	return M4OK;
}
static M4Err BMP_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like color space changing ...) */
	return M4NotSupported;
}

static M4Err BMP_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	unsigned char *pix;
	u32 i, j, irow, in_stride, out_stride, BPP;
	BitStream *bs;
	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER fi;

	BMPCTX();
	if (inBufferLength<54) return M4NonCompliantBitStream;
	bs = NewBitStream(inBuffer, inBufferLength, BS_READ);

#ifdef WIN32
	BS_ReadData(bs, (unsigned char *) &fh, 14);
#else
	fh.bfType = BS_ReadInt(bs, 16);
	fh.bfSize = BS_ReadInt(bs, 32);
	fh.bfReserved1 = BS_ReadInt(bs, 16);
	fh.bfReserved2 = BS_ReadInt(bs, 16);
	fh.bfOffBits = BS_ReadInt(bs, 32);
	fh.bfOffBits = ntohl(fh.bfOffBits);
#endif
	BS_ReadData(bs, (unsigned char *) &fi, 40);
	DeleteBitStream(bs);

	if ((fi.biCompression != BI_RGB) || (fi.biPlanes!=1)) return M4NotSupported;
	if ((fi.biBitCount!=24) && (fi.biBitCount!=32)) return M4NotSupported;

	BPP = (fi.biBitCount==24) ? 3 : 4;
	ctx->width = fi.biWidth;
	ctx->height = fi.biHeight;
	ctx->pixel_format = (fi.biBitCount==24) ? M4PF_RGB_24 : M4PF_RGBA;

	/*new cfg, reset*/
	if (ctx->out_size != ctx->width * ctx->height * BPP) {
		ctx->out_size = ctx->width * ctx->height * BPP;
		*outBufferLength = ctx->out_size;
		return M4BufferTooSmall;
	}
	out_stride = ctx->width*BPP;
	in_stride = out_stride;
	while (in_stride % 4) in_stride++;

	/*read*/
	if (fi.biBitCount==24) {
		for (i=0; i<ctx->height; i++) {
			irow = (ctx->height-1-i)*out_stride;
			pix = inBuffer + fh.bfOffBits + i*in_stride;
			for (j=0; j<out_stride; j+=3) {
				outBuffer[j + irow] = pix[2];
				outBuffer[j+1 + irow] = pix[1];
				outBuffer[j+2 + irow] = pix[0];
				pix += 3;
			}
		}
	} else {
		for (i=0; i<ctx->height; i++) {
			irow = (ctx->height-1-i)*out_stride;
			pix = inBuffer + fh.bfOffBits + i*in_stride;
			for (j=0; j<out_stride; j+=4) {
				outBuffer[j + irow] = pix[2];
				outBuffer[j+1 + irow] = pix[1];
				outBuffer[j+2 + irow] = pix[0];
				outBuffer[j+3 + irow] = pix[3];
				pix += 4;
			}
		}
	}
	*outBufferLength = ctx->out_size;
	return M4OK;
}

static const char *BMP_GetCodecName(BaseDecoder *dec)
{
	return "BMP Decoder";
}

u32 NewBMPDec(BaseDecoder *ifcd)
{
	IMGDec *wrap = (IMGDec *) ifcd->privateStack;
	BMPDec *dec = (BMPDec *) malloc(sizeof(BMPDec));
	memset(dec, 0, sizeof(BMPDec));
	wrap->opaque = dec;
	wrap->type = DEC_BMP;

	/*setup our own interface*/	
	ifcd->AttachStream = BMP_AttachStream;
	ifcd->DetachStream = BMP_DetachStream;
	ifcd->GetCapabilities = BMP_GetCapabilities;
	ifcd->SetCapabilities = BMP_SetCapabilities;
	ifcd->GetName = BMP_GetCodecName;
	((MediaDecoder *)ifcd)->ProcessData = BMP_ProcessData;
	return 1;
}

void DeleteBMPDec(BaseDecoder *ifcg)
{
	BMPCTX();
	free(ctx);
}
