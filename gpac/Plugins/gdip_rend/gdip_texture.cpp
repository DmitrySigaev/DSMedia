/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / GDIplus rasterizer plugin
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


#include "gdip_priv.h"
/*YUV->RGB*/
#include <gpac/intern/m4_render_tools.h>


#define COL_565(c) ( ( ( (c>>16) & 248) << 8) + ( ( (c>>8) & 252) << 3)  + ( (c&0xFF) >> 3) )
#define COL_555(c) ((( (c>>16) & 248)<<7) + (((c>>8) & 248)<<2)  + ((c&0xFF)>>3))

static
M4Err gp_stencil_set_texture(LPM4STENCIL _this, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat, enum M4PixelFormats destination_format_hint, Bool no_copy)
{
	unsigned char *ptr;
	Bool is_yuv;
	u32 pFormat, isBGR, BPP, i, j, col;
	unsigned char a, r, g, b;
	unsigned short val;
	Bool copy;
	GPSTEN();
	CHECK_RET(M4StencilTexture);

	gp_cmat_reset(&_sten->cmat);
	isBGR = 0;
	BPP = 4;
	copy = 0;
	is_yuv = 0;
	/*is pixel format supported ?*/
	switch (pixelFormat) {
	case M4PF_GREYSCALE:
		pFormat = PixelFormat24bppRGB;
		BPP = 1;
		/*cannot get it to work without using 24bpp rgb*/
		copy = 1;
		break;
	case M4PF_ALPHAGREY:
		pFormat = PixelFormat32bppARGB;
		BPP = 2;
		/*cannot get it to work without using 32bpp argb*/
		copy = 1;
		break;
	case M4PF_RGB_555:
		pFormat = PixelFormat16bppRGB555;
		BPP = 2;
		break;
	case M4PF_RGB_565:
		pFormat = PixelFormat16bppRGB565;
		BPP = 2;
		break;
	case M4PF_RGB_24:
		pFormat = PixelFormat24bppRGB;
		BPP = 3;
		/*one day I'll hope to understand how color management works with GDIplus bitmaps...*/
		isBGR = 1;
//		copy = 1;
		break;
	case M4PF_BGR_24:
		pFormat = PixelFormat24bppRGB;
		BPP = 3;
		break;
	case M4PF_RGB_32:
		pFormat = PixelFormat32bppRGB;
		BPP = 4;
		break;
	case M4PF_ARGB:
		pFormat = PixelFormat32bppARGB;
		BPP = 4;
		break;
	case M4PF_RGBA:
		pFormat = PixelFormat32bppARGB;
		BPP = 4;
		copy = 1;
		break;
	case M4PF_YV12:
	case M4PF_IYUV:
	case M4PF_I420:
		if ( (width*3)%4) return M4NotSupported;
		_sten->orig_format = M4PF_YV12;
		is_yuv = 1;
		break;
	case M4PF_YUVA:
		_sten->orig_format = M4PF_YUVA;
		is_yuv = 1;
		break;
	default:
		return M4NotSupported;
	}

	if (_sten->pBitmap) GdipDisposeImage(_sten->pBitmap);
	_sten->pBitmap = NULL;
	_sten->width = width;
	_sten->height = height;
	_sten->destination_format = destination_format_hint;
	if (is_yuv) {
		_sten->orig_buf = pixels;
		_sten->orig_stride = stride;
		_sten->is_converted = 0;
		return M4OK;
	}

	_sten->is_converted = 1;
	_sten->format = pFormat;

	/*GDIplus limitation : horiz_stride shall be multiple of 4 and no support for pure grayscale without palette*/
	if (!copy && pixels && !(stride%4)) {
		if (no_copy && isBGR) return M4NotSupported;
		GdipCreateBitmapFromScan0(_sten->width, _sten->height, stride, pFormat, pixels, &_sten->pBitmap);
		_sten->invert_br = isBGR;
	}
	/*all other cases: create a local bitmap in desired format*/
	else {
		if (no_copy) return M4NotSupported;
		GdipCreateBitmapFromScan0(_sten->width, _sten->height, 0, pFormat, NULL, &_sten->pBitmap);
		ptr = pixels;
		for (j=0; j<_sten->height; j++) {
		for (i=0; i<_sten->width; i++) {
			switch (pixelFormat) {
			case M4PF_GREYSCALE:
				col = MAKEARGB(255, *ptr, *ptr, *ptr);
				ptr ++;
				break;
			case M4PF_ALPHAGREY:
				r = *ptr++;
				a = *ptr++;
				col = MAKEARGB(a, r, r, r);
				break;
			case M4PF_RGB_555:
				val = * (unsigned short *) (ptr);
				ptr+= 2;
				col = COL_555(val);
				break;
			case M4PF_RGB_565:
				val = * (unsigned short *) (ptr);
				ptr+= 2;
				col = COL_565(val);
				break;
			/*scan0 uses bgr...*/
			case M4PF_BGR_24:
			case M4PF_RGB_24:
				r = *ptr++;
				g = *ptr++;
				b = *ptr++;
				if (!isBGR) {
					col = MAKEARGB(255, b, g, r);
				} else {
					col = MAKEARGB(255, r, g, b);
				}
				break;
			/*NOTE: we assume little-endian only for GDIplus platforms, so BGRA/BGRX*/
			case M4PF_RGB_32:
			case M4PF_ARGB:
				b = *ptr++;
				g = *ptr++;
				r = *ptr++;
				a = *ptr++;
				if (pixelFormat==M4PF_RGB_32) a = 0xFF;
				col = MAKEARGB(a, r, g, b);
				break;
			case M4PF_RGBA:
				r = *ptr++;
				g = *ptr++;
				b = *ptr++;
				a = *ptr++;
				col = MAKEARGB(a, r, g, b);
				break;
			default:
				col = MAKEARGB(255, 255, 255, 255);
				break;
			}
			GdipBitmapSetPixel(_sten->pBitmap, i, j, col);
		}}
	}

	return M4OK;
}


static
M4Err gp_stencil_create_texture(LPM4STENCIL _this, u32 width, u32 height, enum M4PixelFormats pixelFormat)
{
	u32 pFormat;
	GPSTEN();
	CHECK_RET(M4StencilTexture);

	gp_cmat_reset(&_sten->cmat);
	/*is pixel format supported ?*/
	switch (pixelFormat) {
	case M4PF_BGR_24:
	case M4PF_GREYSCALE:
	case M4PF_RGB_24:
		pFormat = PixelFormat24bppRGB;
		break;
	case M4PF_ALPHAGREY:
		pFormat = PixelFormat32bppARGB;
		break;
	case M4PF_RGB_555:
		pFormat = PixelFormat16bppRGB555;
		break;
	case M4PF_RGB_565:
		pFormat = PixelFormat16bppRGB565;
		break;
	case M4PF_RGB_32:
		pFormat = PixelFormat32bppRGB;
		break;
	case M4PF_ARGB:
		pFormat = PixelFormat32bppARGB;
		break;
	default:
		return M4NotSupported;
	}

	if (_sten->pBitmap) GdipDisposeImage(_sten->pBitmap);
	_sten->pBitmap = NULL;
	_sten->width = width;
	_sten->height = height;
	_sten->is_converted = 1;
	_sten->format = pFormat;
	GdipCreateBitmapFromScan0(_sten->width, _sten->height, 0, pFormat, NULL, &_sten->pBitmap);
	return M4OK;
}


static
M4Err gp_set_texture_repeat_mode(LPM4STENCIL _this, enum M4TextureTiling mode)
{
	GPSTEN();
	/*not supported*/
	return M4OK;
}
static
M4Err gp_set_texture_filter(LPM4STENCIL _this, M4TextureFilterLevel filter_mode)
{
	GPSTEN();
	CHECK_RET(M4StencilTexture);
	_sten->tFilter = filter_mode;
	return M4OK;
}

static void gp_cmat_multiply(M4ColorMatrix *_this, M4ColorMatrix *w)
{
	Float res[20];
	if (!_this || !w || w->identity) return;
	res[0] = _this->m[0]*w->m[0] + _this->m[1]*w->m[5] + _this->m[2]*w->m[10] + _this->m[3]*w->m[15];
	res[1] = _this->m[0]*w->m[1] + _this->m[1]*w->m[6] + _this->m[2]*w->m[11] + _this->m[3]*w->m[16];
	res[2] = _this->m[0]*w->m[2] + _this->m[1]*w->m[7] + _this->m[2]*w->m[12] + _this->m[3]*w->m[17];
	res[3] = _this->m[0]*w->m[3] + _this->m[1]*w->m[8] + _this->m[2]*w->m[13] + _this->m[3]*w->m[18];
	res[4] = _this->m[0]*w->m[4] + _this->m[1]*w->m[9] + _this->m[2]*w->m[14] + _this->m[3]*w->m[19] + _this->m[4];
	
	res[5] = _this->m[5]*w->m[0] + _this->m[6]*w->m[5] + _this->m[7]*w->m[10] + _this->m[8]*w->m[15];
	res[6] = _this->m[5]*w->m[1] + _this->m[6]*w->m[6] + _this->m[7]*w->m[11] + _this->m[8]*w->m[16];
	res[7] = _this->m[5]*w->m[2] + _this->m[6]*w->m[7] + _this->m[7]*w->m[12] + _this->m[8]*w->m[17];
	res[8] = _this->m[5]*w->m[3] + _this->m[6]*w->m[8] + _this->m[7]*w->m[13] + _this->m[8]*w->m[18];
	res[9] = _this->m[5]*w->m[4] + _this->m[6]*w->m[9] + _this->m[7]*w->m[14] + _this->m[8]*w->m[19] + _this->m[9];
	
	res[10] = _this->m[10]*w->m[0] + _this->m[11]*w->m[5] + _this->m[12]*w->m[10] + _this->m[13]*w->m[15];
	res[11] = _this->m[10]*w->m[1] + _this->m[11]*w->m[6] + _this->m[12]*w->m[11] + _this->m[13]*w->m[16];
	res[12] = _this->m[10]*w->m[2] + _this->m[11]*w->m[7] + _this->m[12]*w->m[12] + _this->m[13]*w->m[17];
	res[13] = _this->m[10]*w->m[3] + _this->m[11]*w->m[8] + _this->m[12]*w->m[13] + _this->m[13]*w->m[18];
	res[14] = _this->m[10]*w->m[4] + _this->m[11]*w->m[9] + _this->m[12]*w->m[14] + _this->m[13]*w->m[19] + _this->m[14];
	
	res[15] = _this->m[15]*w->m[0] + _this->m[16]*w->m[5] + _this->m[17]*w->m[10] + _this->m[18]*w->m[15];
	res[16] = _this->m[15]*w->m[1] + _this->m[16]*w->m[6] + _this->m[17]*w->m[11] + _this->m[18]*w->m[16];
	res[17] = _this->m[15]*w->m[2] + _this->m[16]*w->m[7] + _this->m[17]*w->m[12] + _this->m[18]*w->m[17];
	res[18] = _this->m[15]*w->m[3] + _this->m[16]*w->m[8] + _this->m[17]*w->m[13] + _this->m[18]*w->m[18];
	res[19] = _this->m[15]*w->m[4] + _this->m[16]*w->m[9] + _this->m[17]*w->m[14] + _this->m[18]*w->m[19] + _this->m[19];
	
	memcpy(_this->m, res, sizeof(Float)*20);
}

static
M4Err gp_stencil_set_color_matrix(LPM4STENCIL _this, M4ColorMatrix *cmat)
{
	GPSTEN();
	if (!cmat || cmat->identity) {
		_sten->texture_invalid = _sten->has_cmat;
		_sten->has_cmat = 0;
	} else {
		if (_sten->invert_br) {
			M4ColorMatrix fin, rev;
			memcpy(&fin, cmat, sizeof(M4ColorMatrix));
			memset(&rev, 0, sizeof(M4ColorMatrix));
			rev.m[0] = 0;
			rev.m[2] = 1;
			rev.m[10] = 1;
			rev.m[12] = 0;
			rev.m[6] = rev.m[18] = 1;
			gp_cmat_multiply(&fin, &rev);
			cmat_m4_to_gdip(&fin, &_sten->cmat);
		} else {
			cmat_m4_to_gdip(cmat, &_sten->cmat);
		}
		_sten->has_cmat = 1;
	}
	_sten->texture_invalid = 1;
	return M4OK;
}
static
M4Err gp_stencil_reset_color_matrix(LPM4STENCIL _this)
{
	GPSTEN();
	if (_sten->has_cmat) {
		_sten->has_cmat = 0;
		_sten->texture_invalid = 1;
	}
	return M4OK;
}

static
M4Err gp_set_texture_view(LPM4STENCIL _this, M4Rect *rc)
{
	GPSTEN();

	/*avoid invalidating texture if possible, it is extremely slow building it*/
	if (rc) {
		if (!_sten->has_window) {
			_sten->wnd = *rc;
			_sten->has_window = 1;
			_sten->texture_invalid = 1;
		} else {
			if ( (rc->x==_sten->wnd.x) && (rc->y==_sten->wnd.y) 
				&& (rc->width==_sten->wnd.width) && (rc->height==_sten->wnd.height) ) return M4OK;
			_sten->wnd = *rc;
			_sten->texture_invalid = 1;
		}
	} else if (_sten->has_window) {
		_sten->has_window = 0;
		_sten->texture_invalid = 1;
	}
	return M4OK;
}

static
M4Err gp_set_texture_alpha(LPM4STENCIL _this, u8 alpha)
{
	GPSTEN();
	if (_sten->alpha != alpha) {
		_sten->alpha = alpha;
		_sten->texture_invalid = 1;
	}
	return M4OK;
}

void gp_convert_texture(struct _stencil *sten);

static
M4Err gp_get_pixel(LPM4STENCIL _this, u32 x, u32 y, u32 *col)
{
	ARGB v;
	GpStatus st;

	GPSTEN();
	if (!_sten->is_converted) gp_convert_texture(_sten);
	if (!_sten->pBitmap) return M4BadParam;

	st = GdipBitmapGetPixel(_sten->pBitmap, x, y, &v);

	if (_sten->invert_br) {
		*col = MAKEARGB( ((v>>24)&0xFF), ((v)&0xFF), ((v>>8)&0xFF), ((v>>16)&0xFF) );
	} else {
		*col = v;
	}
	return M4OK;
}


static
M4Err gp_set_pixel(LPM4STENCIL _this, u32 x, u32 y, u32 col)
{
	GpStatus st;
	ARGB v;
	GPSTEN();
	if (!_sten->pBitmap) return M4BadParam;
	if (!_sten->is_converted) gp_convert_texture(_sten);

	if (_sten->invert_br) {
		v = MAKEARGB( ((col>>24)&0xFF), ((col)&0xFF), ((col>>8)&0xFF), ((col>>16)&0xFF) );
	} else {
		v = col;
	}
	st = GdipBitmapSetPixel(_sten->pBitmap, x, y, v);
	return M4OK;
}

#if 0
static
M4Err gp_get_texture(LPM4STENCIL _this, unsigned char **pixels, u32 *width, u32 *height, u32 *stride, enum M4PixelFormats *pixelFormat)
{
	GpRect rc;
	BitmapData data;
	GPSTEN();
	if (!_sten->pBitmap) return M4BadParam;

	rc.X = rc.Y = 0;
	rc.Width = _sten->width;
	rc.Height = _sten->height;

	GdipBitmapLockBits(_sten->pBitmap, &rc, ImageLockModeRead, _sten->format, &data);
	*pixels = (unsigned char *) data.Scan0;
	*width = data.Width;
	*height = data.Height;
	*stride = data.Stride;
	switch (data.PixelFormat) {
	case PixelFormat16bppRGB555:
		*pixelFormat = M4PF_RGB_555;
		break;
	case PixelFormat16bppRGB565:
		*pixelFormat = M4PF_RGB_565;
		break;
	case PixelFormat32bppRGB:
		*pixelFormat = M4PF_RGB_32;
		break;
	case PixelFormat32bppARGB:
		*pixelFormat = M4PF_ARGB;
		break;
	case PixelFormat24bppRGB:
	default:
		*pixelFormat = M4PF_RGB_24;
		break;
	}
	return M4OK;
}
#endif


void gp_stencil_texture_modified(LPM4STENCIL _this)
{
	GPSTEN();
	_sten->texture_invalid = 1;
}

void gp_init_driver_texture(Graphics2DDriver *driver)
{
	driver->stencil_set_texture = gp_stencil_set_texture;
	driver->set_texture_repeat_mode = gp_set_texture_repeat_mode;
	driver->set_texture_filter = gp_set_texture_filter;
	driver->stencil_set_color_matrix = gp_stencil_set_color_matrix;
	driver->stencil_reset_color_matrix = gp_stencil_reset_color_matrix;
	driver->set_texture_view = gp_set_texture_view;
	driver->set_texture_alpha = gp_set_texture_alpha;
	driver->stencil_create_texture = gp_stencil_create_texture;
	driver->stencil_texture_modified = gp_stencil_texture_modified;
	driver->stencil_get_pixel = gp_get_pixel;
	driver->stencil_set_pixel = gp_set_pixel;
}


void gp_convert_texture(struct _stencil *sten)
{
	unsigned char *y, *u, *v, *a;
	u32 BPP, format;

	if (sten->orig_format == M4PF_YV12) {
		BPP = 3;
	} else {
		BPP = 4;
	}
	if (BPP*sten->width*sten->height > sten->conv_size) {
		if (sten->conv_buf) free(sten->conv_buf);
		sten->conv_size = BPP*sten->width*sten->height;
		sten->conv_buf = (unsigned char *) malloc(sizeof(unsigned char)*sten->conv_size);
	}
	y = sten->orig_buf;
	u = y + sten->orig_stride*sten->height;
	v = y + 5 * sten->orig_stride*sten->height / 4;

	if (BPP==4) {
		a = y + 3 * sten->orig_stride*sten->height / 2;
		yuva2rgb_32(sten->conv_buf, 4*sten->width, y, u, v, a, sten->orig_stride, sten->orig_stride/2, sten->width, sten->height);
		format = PixelFormat32bppARGB;
	} else {
		/*convert into BGR for scan0 bug...*/
		yuv2bgr_24(sten->conv_buf, 3*sten->width, y, u, v, sten->orig_stride, sten->orig_stride/2, sten->width, sten->height);
		format = PixelFormat24bppRGB;
	}
	if (sten->pBitmap) GdipDisposeImage(sten->pBitmap);
	GdipCreateBitmapFromScan0(sten->width, sten->height, BPP*sten->width, format, sten->conv_buf, &sten->pBitmap);
	sten->is_converted = 1;
}

void gp_load_texture(struct _stencil *sten)
{
	GpImageAttributes *attr;
	ColorMatrix _cmat;

	if (sten->texture_invalid && sten->pTexture) {
		GdipDeleteBrush(sten->pTexture);
		sten->pTexture = NULL;
	}
	/*nothing to do*/
	if (sten->is_converted && sten->pTexture) return;
	sten->texture_invalid = 0;

	/*convert*/
	if (!sten->is_converted) gp_convert_texture(sten);

	GdipCreateImageAttributes(&attr);
	if (sten->has_cmat) {
		memcpy(_cmat.m, sten->cmat.m, sizeof(REAL)*5*5);
	} else {
		memset(_cmat.m, 0, sizeof(REAL)*5*5);
		_cmat.m[0][0] = _cmat.m[1][1] = _cmat.m[2][2] = _cmat.m[3][3] = _cmat.m[4][4] = 1.0;
		if (sten->invert_br) {
			_cmat.m[0][0] = 0;
			_cmat.m[0][2] = 1;
			_cmat.m[2][2] = 0;
			_cmat.m[2][0] = 1;
		}
	}

	_cmat.m[3][3] *= ((REAL) sten->alpha) /255.0f;
	GdipSetImageAttributesColorMatrix(attr, ColorAdjustTypeDefault, TRUE, &_cmat, NULL, ColorMatrixFlagsDefault);

	if (sten->has_window) {
		GdipCreateTextureIA(sten->pBitmap, attr, sten->wnd.x, sten->wnd.y, sten->wnd.width, sten->wnd.height, &sten->pTexture);
	} else {
		GdipCreateTextureIAI(sten->pBitmap, attr, 0, 0, sten->width, sten->height, &sten->pTexture);
	}
	/*wrap mode is actually ignored in constructor...*/
	GdipSetTextureWrapMode(sten->pTexture, WrapModeTile);

	GdipDisposeImageAttributes(attr);
}

