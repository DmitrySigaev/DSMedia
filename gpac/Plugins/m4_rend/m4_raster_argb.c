/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / software rasterizer plugin
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
 *
 *
 *	Most of this module is a C code rewrite of the charcoal lib (mupdf)
 *	written by Tor Andersson <tor.andersson@dsek.lth.se> published under the GPL license
 *		
 */


#include "m4_priv.h"

/*
	Note: this we force ARGB textures to be endian-dependent (eg BGRA on little-endian and ARGB on big-endian)
	and we always handle pixel mem as (u32 *) for 32 bits (ARGB/XRGB), we don't need to bother about endianness
	here.
*/

static s32
mul255(s32 a, s32 b)
{
	return ((a+1) * b) >> 8;
}

/*
		32 bit ARGB
*/

static u32 overmask_argb(u32 src, u32 dst, u32 alpha)
{
	u32 resa, resr, resg, resb;
	
	s32 srca = (src >> 24) & 0xff;
	s32 srcr = (src >> 16) & 0xff;
	s32 srcg = (src >> 8) & 0xff;
	s32 srcb = (src >> 0) & 0xff;
	s32 dsta = (dst >> 24) & 0xff;
	srca = mul255(srca, alpha);
	if (dsta) {
		s32 dstr = (dst >> 16) & 0xff;
		s32 dstg = (dst >> 8) & 0xff;
		s32 dstb = (dst >> 0) & 0xff;
		resa = mul255(srca, srca) + mul255(255-srca, dsta);
		resr = mul255(srca, srcr - dstr) + dstr;
		resg = mul255(srca, srcg - dstg) + dstg;
		resb = mul255(srca, srcb - dstb) + dstb;
		return (resa << 24) | (resr << 16) | (resg << 8) | (resb);
	}
	/*special case for ARGB: if dst alpha is 0, consider the surface is empty and copy pixel*/
	return (srca << 24) | (srcr << 16) | (srcg << 8) | (srcb);
}

static void overmask_argb_const_run(u32 src, u32 *dst, u32 count)
{
	u32 resa ,resr, resg, resb;
	
	s32 srca = (src >> 24) & 0xff;
	s32 srcr = (src >> 16) & 0xff;
	s32 srcg = (src >> 8) & 0xff;
	s32 srcb = (src >> 0) & 0xff;


	while (count) {
		u32 val = *dst;
		s32 dsta = (val >> 24) & 0xff;
		/*special case for ARGB: if dst alpha is 0, consider the surface is empty and copy pixel*/
		if (dsta) {
			s32 dstr = (val >> 16) & 0xff;
			s32 dstg = (val >> 8) & 0xff;
			s32 dstb = (val) & 0xff;

			resa = mul255(srca, srca) + mul255(255-srca, dsta);
			resr = mul255(srca, srcr - dstr) + dstr;
			resg = mul255(srca, srcg - dstg) + dstg;
			resb = mul255(srca, srcb - dstb) + dstb;
	
			*dst = (resa << 24) | (resr << 16) | (resg << 8) | (resb);
		} else {
			*dst = src;
		}
		dst++;
		count--;
	}
}

void m4_argb_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	s32 i;
	u32 x, len;
	u8 aa_lev = surf->AALevel;

	col_no_a = col & 0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
	
		if (spans[i].coverage != 0xFF) {
			a = mul255(0xFF, spans[i].coverage);
			fin = (a<<24) | col_no_a;
			overmask_argb_const_run(fin, &dst[x], len);
		} else {
			while (len--) {
				dst[x] = col;
				x ++;
			}
		}
	}
}

void m4_argb_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	u8 aa_lev = surf->AALevel;
	s32 i;

	a = (col>>24)&0xFF;
	col_no_a = col & 0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		fin = mul255(a, spans[i].coverage);
		fin = (fin<<24) | col_no_a;
		overmask_argb_const_run(fin, &dst[spans[i].x], spans[i].len);
	}
}


void m4_argb_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	u8 spanalpha, col_a;
	s32 i;
	u32 x, len;
	u32 *col;
	u8 aa_lev = surf->AALevel;

	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
		spanalpha = spans[i].coverage;
		surf->sten->fill_run(surf->sten, surf, x, y, len);
		col = surf->stencil_pix_run;
		while (len--) {
			col_a = M4C_A(*col);
			if (col_a) {
				if ((spanalpha!=0xFF) || (col_a != 0xFF)) {
					dst[x] = overmask_argb(*col, dst[x], spanalpha);
				} else {
					dst[x] = *col;
				}
			}
			col++;
			x ++;
		}
	}
}

M4Err m4_surface_clear_argb(LPM4SURFACE surf, M4IRect rc, M4Color col)
{
	u32 x, y, w, h, st, sx, sy;
	M4Surface *_this = (M4Surface *)surf;
	st = _this->stride;

	h = rc.height;
	w = rc.width;
	sx = rc.x;
	sy = rc.y;
	
	for (y = 0; y < h; y++) {
		u32 *data = (u32 *) (_this ->pixels + (sy+y)* st + 4*sx);
		for (x = 0; x < w; x++) {
			*data++ = col;
		}
	}
	return M4OK;
}


/*
		32 bit RGB
*/

static u32 overmask_rgb32(u32 src, u32 dst, u32 alpha)
{
	u32 resr, resg, resb;
	
	s32 srca = (src >> 24) & 0xff;
	s32 srcr = (src >> 16) & 0xff;
	s32 srcg = (src >> 8) & 0xff;
	s32 srcb = (src >> 0) & 0xff;
	s32 dstr = (dst >> 16) & 0xff;
	s32 dstg = (dst >> 8) & 0xff;
	s32 dstb = (dst) & 0xff;

	srca = mul255(srca, alpha);
	resr = mul255(srca, srcr - dstr) + dstr;
	resg = mul255(srca, srcg - dstg) + dstg;
	resb = mul255(srca, srcb - dstb) + dstb;
	return (0xFF << 24) | (resr << 16) | (resg << 8) | resb;
}

M4INLINE void overmask_rgb32_const_run(u32 src, u32 *dst, u32 count)
{
	u32 val, res;
	s32 srca = (src>>24) & 0xff;
	u32 srcr = mul255(srca, ((src >> 16) & 0xff)) ;
	u32 srcg = mul255(srca, ((src >> 8) & 0xff)) ;
	u32 srcb = mul255(srca, ((src) & 0xff)) ;
	u32 inva = 1 + 0xFF - srca;

	while (count) {
		val = *dst;
		res = 0xFF00;
		res |= srcr + ((inva*((val >> 16) & 0xff))>>8);
		res <<=8;
		res |= srcg + ((inva*((val >> 8) & 0xff))>>8);
		res <<=8;
		res |= srcb + ((inva*((val) & 0xff))>>8);
		*dst = res;
		dst++;
		count--;
	}
}

void m4_rgb32_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a, col2;
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	s32 i;
	u32 x, len;
	u8 aa_lev = surf->AALevel;

	col_no_a = col & 0x00FFFFFF;
	col2 = (0xFF<<24) | col_no_a;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
	
		if (spans[i].coverage != 0xFF) {
			a = mul255(0xFF, spans[i].coverage);
			fin = (a<<24) | col_no_a;
			overmask_rgb32_const_run(fin, &dst[x], len);
		} else {
			while (len--) {
				dst[x] = col2;
				x ++;
			}
		}
	}
}

void m4_rgb32_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	u8 aa_lev = surf->AALevel;
	s32 i;

	a = (col>>24)&0xFF;
	col_no_a = col & 0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		fin = mul255(a, spans[i].coverage);
		fin = (fin<<24) | col_no_a;
		overmask_rgb32_const_run(fin, &dst[spans[i].x], spans[i].len);
	}
}


void m4_rgb32_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u32 *dst = (u32 *) (surf->pixels + y * surf->stride);
	u8 spanalpha, col_a;
	s32 i;
	u32 x, len;
	u32 *col;
	u8 aa_lev = surf->AALevel;

	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
		spanalpha = spans[i].coverage;
		surf->sten->fill_run(surf->sten, surf, x, y, len);
		col = surf->stencil_pix_run;
		while (len--) {
			col_a = M4C_A(*col);
			if (col_a) {
				if ((spanalpha!=0xFF) || (col_a != 0xFF)) {
					dst[x] = overmask_rgb32(*col, dst[x], spanalpha);
				} else {
					dst[x] = *col;
				}
			}
			col++;
			x ++;
		}
	}
}

M4Err m4_surface_clear_rgb32(LPM4SURFACE surf, M4IRect rc, M4Color col)
{
	u32 x, y, w, h, st, sx, sy;
	M4Surface *_this = (M4Surface *)surf;
	st = _this->stride;

	h = rc.height;
	w = rc.width;
	sx = rc.x;
	sy = rc.y;

	col = 0xFF000000 | (col & 0x00FFFFFF);
	for (y = 0; y < h; y++) {
		u32 *data = (u32 *) (_this ->pixels + (y + sy) * st + 4*sx);
		for (x = 0; x < w; x++) {
			*data++ = col;
		}
	}
	return M4OK;
}


