/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / 2D rendering plugin
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

#include "render2d.h"

static u32 get_yuv_base(u32 in_pf) 
{
	switch (in_pf) {
	case M4PF_I420:
	case M4PF_IYUV:
	case M4PF_YV12:
		return M4PF_YV12;
	case M4PF_Y422:
	case M4PF_UYNV:
	case M4PF_UYVY:
		return M4PF_UYVY;
	case M4PF_YUNV:
	case M4PF_V422:
	case M4PF_YUY2:
		return M4PF_YUY2;
	case M4PF_YVYU:
		return M4PF_YVYU;
	default:
		return 0;
	}
}

static Bool formx2d_is_yuv(u32 in_pf)
{
	switch (in_pf) {
	case M4PF_YUY2:
	case M4PF_YVYU:
	case M4PF_UYVY:
	case M4PF_VYUY:
	case M4PF_Y422:
	case M4PF_UYNV:
	case M4PF_YUNV:
	case M4PF_V422:
	case M4PF_YV12:
	case M4PF_IYUV:
	case M4PF_I420:
		return 1;
	/*not supported yet*/
	case M4PF_YUVA:
	default:
		return 0;
	}
}

static Bool is_planar_yuv(u32 pf)
{
	switch (pf) {
	case M4PF_YV12:
	case M4PF_I420:
	case M4PF_IYUV:
		return 1;
	}
	return 0;
}


static void VR_write_yv12_to_yuv(M4VideoSurface *vs,  unsigned char *src, u32 src_stride, u32 src_pf,
								 u32 src_width, u32 src_height, M4Window *src_wnd)
{
	unsigned char *pY, *pU, *pV;
	pY = src;
	pU = src + src_stride * src_height;
	pV = src + 5*src_stride * src_height/4;

	pY = pY + src_stride * src_wnd->y + src_wnd->x;
	pU = pU + (src_stride * src_wnd->y / 2 + src_wnd->x) / 2;
	pV = pV + (src_stride * src_wnd->y / 2 + src_wnd->x) / 2;


	if (is_planar_yuv(vs->pixel_format)) {
		/*complete source copy*/
		if ( (vs->pitch == src_stride) && (src_wnd->w == src_width) && (src_wnd->h == src_height)) {
			assert(!src_wnd->x);
			assert(!src_wnd->y);
			memcpy(vs->video_buffer, pY, sizeof(unsigned char)*src_width*src_height);
			if (vs->pixel_format == M4PF_YV12) {
				memcpy(vs->video_buffer + vs->pitch * vs->height, pV, sizeof(unsigned char)*src_width*src_height/4);
				memcpy(vs->video_buffer + 5 * vs->pitch * vs->height/4, pU, sizeof(unsigned char)*src_width*src_height/4);
			} else {
				memcpy(vs->video_buffer + vs->pitch * vs->height, pU, sizeof(unsigned char)*src_width*src_height/4);
				memcpy(vs->video_buffer + 5 * vs->pitch * vs->height/4, pV, sizeof(unsigned char)*src_width*src_height/4);
			}
		} else {
			u32 i;
			unsigned char *dst, *src, *dst2, *src2, *dst3, *src3;

			src = pY;
			dst = vs->video_buffer;
			
			src2 = (vs->pixel_format != M4PF_YV12) ? pU : pV;
			dst2 = vs->video_buffer + vs->pitch * vs->height;
			src3 = (vs->pixel_format != M4PF_YV12) ? pV : pU;
			dst3 = vs->video_buffer + 5*vs->pitch * vs->height/4;
			for (i=0; i<src_wnd->h; i++) {
				memcpy(dst, src, src_wnd->w);
				src += src_stride;
				dst += vs->pitch;
				if (i<src_wnd->h/2) {
					memcpy(dst2, src2, src_wnd->w/2);
					src2 += src_stride/2;
					dst2 += vs->pitch/2;
					memcpy(dst3, src3, src_wnd->w/2);
					src3 += src_stride/2;
					dst3 += vs->pitch/2;
				}
			}
		}
	} else if (vs->pixel_format==M4PF_UYVY) {
		u32 i, j;
		unsigned char *dst, *y, *u, *v;
		for (i=0; i<src_wnd->h; i++) {
			y = pY + i*src_stride;
			u = pU + (i/2) * src_stride/2;
			v = pV + (i/2) * src_stride/2;
			dst = vs->video_buffer + i*vs->pitch;

			for (j=0; j<src_wnd->w/2;j++) {
				*dst = *u;
				dst++;
				u++;
				*dst = *y;
				dst++;
				y++;
				*dst = *v;
				dst++;
				v++;
				*dst = *y;
				dst++;
				y++;
			}
		}
	} else if (vs->pixel_format==M4PF_YUY2) {
		u32 i, j;
		unsigned char *dst, *y, *u, *v;
		for (i=0; i<src_wnd->h; i++) {
			y = pY + i*src_stride;
			u = pU + (i/2) * src_stride/2;
			v = pV + (i/2) * src_stride/2;
			dst = vs->video_buffer + i*vs->pitch;

			for (j=0; j<src_wnd->w/2;j++) {
				*dst = *y;
				dst++;
				y++;
				*dst = *u;
				dst++;
				u++;
				*dst = *y;
				dst++;
				y++;
				*dst = *v;
				dst++;
				v++;
			}
		}
	} else if (vs->pixel_format==M4PF_YVYU) {
		u32 i, j;
		unsigned char *dst, *y, *u, *v;
		for (i=0; i<src_wnd->h; i++) {
			y = pY + i*src_stride;
			u = pU + (i/2) * src_stride/2;
			v = pV + (i/2) * src_stride/2;
			dst = vs->video_buffer + i*vs->pitch;

			for (j=0; j<src_wnd->w/2;j++) {
				*dst = *y;
				dst++;
				y++;
				*dst = *v;
				dst++;
				v++;
				*dst = *y;
				dst++;
				y++;
				*dst = *u;
				dst++;
				u++;
			}
		}
	}

}

u32 get_bpp(u32 pf)
{
	switch (pf) {
	case M4PF_RGB_555:
	case M4PF_RGB_565:
		return 2;
	case M4PF_RGB_24:
	case M4PF_BGR_24:
		return 3;
	case M4PF_RGB_32:
	case M4PF_BGR_32:
	case M4PF_ARGB:
		return 4;
	}
	return 0;
}

void rgb_to_24(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd)
{
	u32 i;
	u32 BPP = get_bpp(src_pf);
	if (!BPP) return;

	/*go to start of src*/
	src += src_stride*src_wnd->y + BPP * src_wnd->x;

	if (src_pf==vs->pixel_format) {
		for (i=0; i<src_wnd->h; i++) {
			memcpy(vs->video_buffer + i*vs->pitch, src, sizeof(unsigned char) * BPP * src_wnd->w);
			src += src_stride;
		}
		return;
	}
}


void rgb_to_555(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd)
{
	u32 i, j, r, g, b;
	u32 BPP = get_bpp(src_pf);
	unsigned char *dst, *cur;
	if (!BPP) return;

	/*go to start of src*/
	src += src_stride*src_wnd->y + BPP * src_wnd->x;

	if (src_pf==vs->pixel_format) {
		for (i=0; i<src_wnd->h; i++) {
			memcpy(vs->video_buffer + i*vs->pitch, src, sizeof(unsigned char) * BPP * src_wnd->w);
		}
		return;
	}
	/*nope get all pixels*/
	for (i=0; i<src_wnd->h; i++) {
		dst = vs->video_buffer + i*vs->pitch;
		cur = src + i*src_stride;
		for (j=0; j<src_wnd->w; j++) {
			switch (src_pf) {
			case M4PF_RGB_24:
				r = *cur++;
				g = *cur++;
				b = *cur++;
				* ((unsigned short *)dst) = M4C_555(r, g, b);
				dst += 2;
				break;
			}
		}
	}
}

void rgb_to_565(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd)
{
	u32 i, j, r, g, b;
	u32 BPP = get_bpp(src_pf);
	unsigned char *dst, *cur;
	if (!BPP) return;

	/*go to start of src*/
	src += src_stride*src_wnd->y + BPP * src_wnd->x;

	if (src_pf==vs->pixel_format) {
		for (i=0; i<src_wnd->h; i++) {
			memcpy(vs->video_buffer + i*vs->pitch, src, sizeof(unsigned char) * BPP * src_wnd->w);
		}
		return;
	}
	/*nope get all pixels*/
	for (i=0; i<src_wnd->h; i++) {
		dst = vs->video_buffer + i*vs->pitch;
		cur = src + i*src_stride;
		for (j=0; j<src_wnd->w; j++) {
			switch (src_pf) {
			case M4PF_RGB_24:
				r = *cur++;
				g = *cur++;
				b = *cur++;
				* ((unsigned short *)dst) = M4C_565(r, g, b);
				dst += 2;
				break;
			}
		}
	}
}

void rgb_to_32(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd)
{
	u32 i, j;
	Bool isBGR;
	u32 BPP = get_bpp(src_pf);
	unsigned char *dst, *cur;
	if (!BPP) return;

	/*go to start of src*/
	src += src_stride*src_wnd->y + BPP * src_wnd->x;

	if (src_pf==vs->pixel_format) {
		for (i=0; i<src_wnd->h; i++) {
			memcpy(vs->video_buffer + i*vs->pitch, src, sizeof(unsigned char) * BPP * src_wnd->w);
		}
		return;
	}
	/*get all pixels*/
	isBGR = vs->pixel_format==M4PF_BGR_32;
	if (isBGR) {
		for (i=0; i<src_wnd->h; i++) {
			dst = vs->video_buffer + i*vs->pitch;
			cur = src + i*src_stride;
			for (j=0; j<src_wnd->w; j++) {
				switch (src_pf) {
				case M4PF_RGB_24:
					dst[0] = *cur++;
					dst[1] = *cur++;
					dst[2] = *cur++;
					dst += 4;
					break;
				case M4PF_BGR_24:
					dst[2] = *cur++;
					dst[1] = *cur++;
					dst[0] = *cur++;
					dst += 4;
					break;
				}
			}
		}
	} else {
		for (i=0; i<src_wnd->h; i++) {
			dst = vs->video_buffer + i*vs->pitch;
			cur = src + i*src_stride;
			for (j=0; j<src_wnd->w; j++) {
				switch (src_pf) {
				case M4PF_RGB_24:
					dst[2] = *cur++;
					dst[1] = *cur++;
					dst[0] = *cur++;
					dst += 4;
					break;
				case M4PF_BGR_24:
					dst[0] = *cur++;
					dst[1] = *cur++;
					dst[2] = *cur++;
					dst += 4;
					break;
				}
			}
		}
	}
}

void R2D_copyPixels(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd)
{
	/*handle YUV input*/
	if (get_yuv_base(src_pf)==M4PF_YV12) {
		if (formx2d_is_yuv(vs->pixel_format)) {
			/*generic YV planar to YUV (planar or not) */
			VR_write_yv12_to_yuv(vs, src, src_stride, src_pf, src_w, src_h, src_wnd);
		} else {
			/*YVplanar to RGB*/
			unsigned char *pY, *pU, *pV, *pTmp;

			/*setup start point in src*/
			pY = src;
			pU = pY + src_w*src_h;
			pV = pY + 5*src_w*src_h/4;

			pY += src_stride*src_wnd->y + src_wnd->x;
			pU += (src_stride * src_wnd->y / 2 + src_wnd->x) / 2;
			pV += (src_stride * src_wnd->y / 2 + src_wnd->x) / 2;
			assert(src_stride==src_w);
			
			if (src_pf != M4PF_YV12) {
				pTmp = pU;
				pU = pV;
				pV = pTmp;
			}


			switch (vs->pixel_format) {
			case M4PF_RGB_555:
				yuv2rgb_555(vs->video_buffer, vs->pitch, pY, pU, pV, src_stride, src_stride/2, src_wnd->w, src_wnd->h);
				break;
			case M4PF_RGB_565:
				yuv2rgb_565(vs->video_buffer, vs->pitch, pY, pU, pV, src_stride, src_stride/2, src_wnd->w, src_wnd->h);
				break;
			/*to do: add RB flip*/
			case M4PF_RGB_24:
			case M4PF_BGR_24:
				yuv2bgr_24(vs->video_buffer, vs->pitch, pY, pU, pV, src_stride, src_stride/2, src_wnd->w, src_wnd->h);
				break;
			/*to do: add RB flip*/
			case M4PF_RGB_32:
			case M4PF_BGR_32:
				yuv2rgb_32(vs->video_buffer, vs->pitch, pY, pU, pV, src_stride, src_stride/2, src_wnd->w, src_wnd->h);
				break;
			}
		}
		/*other output formats are ignored*/
		return;
	}

	switch (vs->pixel_format) {
	case M4PF_RGB_555:
		rgb_to_555(vs, src, src_stride, src_w, src_w, src_pf, src_wnd);
		break;
	case M4PF_RGB_565:
		rgb_to_565(vs, src, src_stride, src_w, src_w, src_pf, src_wnd);
		break;
	case M4PF_RGB_24:
	case M4PF_BGR_24:
		rgb_to_24(vs, src, src_stride, src_w, src_w, src_pf, src_wnd);
		break;
	case M4PF_RGB_32:
	case M4PF_BGR_32:
		rgb_to_32(vs, src, src_stride, src_w, src_w, src_pf, src_wnd);
		break;
	}
}


