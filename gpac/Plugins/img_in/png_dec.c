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

#ifdef M4_HAS_PNG

#include "png.h"


typedef struct
{
	/*io part for PNG lib*/
	void *in_data;
	u32 in_length, current_pos;

	/*no support for scalability with PNG yet*/
	u16 ES_ID;
	u32 BPP, width, height, out_size, pixel_format;
} PNGDec;
	
#define PNGCTX()	PNGDec *ctx = (PNGDec *) ((IMGDec *)ifcg->privateStack)->opaque


static M4Err PNG_AttachStream(BaseDecoder *ifcg, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream)
{
	PNGCTX();
	if (ctx->ES_ID && ctx->ES_ID!=ES_ID) return M4NotSupported;
	ctx->ES_ID = ES_ID;
	return M4OK;
}
static M4Err PNG_DetachStream(BaseDecoder *ifcg, u16 ES_ID)
{
	PNGCTX();
	if (ctx->ES_ID != ES_ID) return M4BadParam;
	ctx->ES_ID = ES_ID;
	return M4OK;
}
static M4Err PNG_GetCapabilities(BaseDecoder *ifcg, CapObject *capability)
{
	PNGCTX();
	switch (capability->CapCode) {
	case CAP_WIDTH:
		capability->cap.valueINT = ctx->width;
		break;
	case CAP_HEIGHT:
		capability->cap.valueINT = ctx->height;
		break;
	case CAP_STRIDE:
		capability->cap.valueINT = ctx->width * ctx->BPP;
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
static M4Err PNG_SetCapabilities(BaseDecoder *ifcg, CapObject capability)
{
	/*return unsupported to avoid confusion by the player (like color space changing ...) */
	return M4NotSupported;
}

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNGDec *ctx = (PNGDec *)png_ptr->io_ptr;

	if (ctx->current_pos + length > ctx->in_length) {
		png_error(png_ptr, "Read Error");
	} else {
		memcpy(data, (char*) ctx->in_data + ctx->current_pos, length);
		ctx->current_pos += length;
	}
}
static void user_error_fn(png_structp png_ptr,png_const_charp error_msg)
{
	longjmp(png_ptr->jmpbuf, 1);
}

static M4Err PNG_ProcessData(MediaDecoder *ifcg, 
		unsigned char *inBuffer, u32 inBufferLength,
		u16 ES_ID,
		unsigned char *outBuffer, u32 *outBufferLength,
		u8 PaddingBits, u32 mmlevel)
{
	png_struct *png_ptr;
	png_info *info_ptr;
	png_byte **rows;
	u32 i, stride;

	PNGCTX();
	if ((inBufferLength<8) || png_sig_cmp(inBuffer, 0, 8) ) return M4NonCompliantBitStream;

	ctx->in_data = inBuffer;
	ctx->in_length = inBufferLength;
	ctx->current_pos = 0;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp) ctx, NULL, NULL);
	if (!png_ptr) return M4IOErr;
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return M4IOErr;
	}
	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_info_struct(png_ptr,(png_infopp) & info_ptr);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return M4IOErr;
	}
    png_set_read_fn(png_ptr, ctx, (png_rw_ptr) user_read_data);
	png_set_error_fn(png_ptr, ctx, (png_error_ptr) user_error_fn, NULL);

	png_read_info(png_ptr, info_ptr);

	/*unpaletize*/
	if (info_ptr->color_type==PNG_COLOR_TYPE_PALETTE) {
		png_set_expand(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
	}

	ctx->BPP = info_ptr->pixel_depth / 8;
	ctx->width = info_ptr->width;
	ctx->height = info_ptr->height;

	switch (ctx->BPP) {
	case 1:
		ctx->pixel_format = M4PF_GREYSCALE;
		break;
	case 2:
		ctx->pixel_format = M4PF_ALPHAGREY;
		break;
	case 3:
		ctx->pixel_format = M4PF_RGB_24;
		break;
	case 4:
		ctx->pixel_format = M4PF_RGBA;
		break;
	}

	/*new cfg, reset*/
	if (ctx->out_size != ctx->width * ctx->height * ctx->BPP) {
		ctx->out_size = ctx->width * ctx->height * ctx->BPP;
		*outBufferLength = ctx->out_size;
		png_destroy_info_struct(png_ptr,(png_infopp) & info_ptr);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return M4BufferTooSmall;
	}

	/*read*/
	stride = png_get_rowbytes(png_ptr, info_ptr);
	rows = (png_bytepp) malloc(sizeof(png_bytep) * ctx->height);
	for (i=0; i<ctx->height; i++) {
		rows[i] = outBuffer + i*stride;
	}
	png_read_image(png_ptr, rows);
	png_read_end(png_ptr, NULL);
	free(rows);

	png_destroy_info_struct(png_ptr,(png_infopp) & info_ptr);
	png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	*outBufferLength = ctx->out_size;

	
	return M4OK;
}

static const char *PNG_GetCodecName(BaseDecoder *dec)
{
	return "LibPNG " PNG_LIBPNG_VER_STRING;
}

u32 NewPNGDec(BaseDecoder *ifcd)
{
	IMGDec *wrap = (IMGDec *) ifcd->privateStack;
	PNGDec *dec = (PNGDec *) malloc(sizeof(PNGDec));
	memset(dec, 0, sizeof(PNGDec));
	wrap->opaque = dec;
	wrap->type = DEC_PNG;

	/*setup our own interface*/	
	ifcd->AttachStream = PNG_AttachStream;
	ifcd->DetachStream = PNG_DetachStream;
	ifcd->GetCapabilities = PNG_GetCapabilities;
	ifcd->SetCapabilities = PNG_SetCapabilities;
	ifcd->GetName = PNG_GetCodecName;
	((MediaDecoder *)ifcd)->ProcessData = PNG_ProcessData;
	return 1;
}

void DeletePNGDec(BaseDecoder *ifcg)
{
	PNGCTX();
	free(ctx);
}

#endif
