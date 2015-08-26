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
 *		
 */


#include "m4_priv.h"

static M4INLINE s32
mul255(s32 a, s32 b)
{
	return ((a + 1) * b) >> 8;
}


/*
			RGB 565 part
*/

static u16 overmask_565(u32 src, u16 dst, u32 alpha)
{
	u32 resr, resg, resb;
	s32 srca = (src >> 24) & 0xff;
	s32 srcr = (src >> 16) & 0xff;
	s32 srcg = (src >> 8) & 0xff;
	s32 srcb = (src >> 0) & 0xff;

	s32 dstr = (dst >> 8) & 0xf8;
	s32 dstg = (dst >> 3) & 0xfc;
	s32 dstb = (dst << 3) & 0xf8;

	srca = mul255(srca, alpha);
	resr = mul255(srca, srcr - dstr) + dstr;
	resg = mul255(srca, srcg - dstg) + dstg;
	resb = mul255(srca, srcb - dstb) + dstb;
	return M4C_565(resr, resg, resb);
}

void overmask_565_const_run(u32 src, u16 *dst, u32 count)
{
	u32 resr, resg, resb;
	u8 srca = (src >> 24) & 0xff;
	u8 srcr = (src >> 16) & 0xff;
	u8 srcg = (src >> 8) & 0xff;
	u8 srcb = (src >> 0) & 0xff;

	while (count) {
		u16 val = *dst;
		u8 dstr = (val >> 8) & 0xf8;
		u8 dstg = (val >> 3) & 0xfc;
		u8 dstb = (val << 3) & 0xf8;

		resr = mul255(srca, srcr - dstr) + dstr;
		resg = mul255(srca, srcg - dstg) + dstg;
		resb = mul255(srca, srcb - dstb) + dstb;
		*dst = M4C_565(resr, resg, resb);
		dst++;		
		count--;
	}
}

void m4_565_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 col565 = surf->fill_565;
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
	s32 i;
	u32 x, len;
	u8 aa_lev = surf->AALevel;

	col_no_a = col&0x00FFFFFF;

	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
		if (spans[i].coverage != 0xFF) {
			a = mul255(0xFF, spans[i].coverage);
			fin = (a<<24) | (col_no_a);
			overmask_565_const_run(fin, &dst[x], len);
		} else {
			while (len--) {
				dst[x] = col565;
				x++;
			}
		}
	}
}

void m4_565_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	s32 i;
	u8 aa_lev = surf->AALevel;

	a = (col>>24)&0xFF;
	col_no_a = col&0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		fin = mul255(a, spans[i].coverage);
		fin = (fin<<24) | col_no_a;
		overmask_565_const_run(fin, &dst[spans[i].x], spans[i].len);
	}
}


void m4_565_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
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
					dst[x] = overmask_565(*col, dst[x], spanalpha);
				} else {
					dst[x] = M4COL_565(*col);
				}
			}
			col++;
			x ++;
		}
	}
}

M4Err m4_surface_clear_565(LPM4SURFACE surf, M4IRect rc, M4Color col)
{
	u32 x, y, w, h, st, sx, sy;
	u16 val;
	M4Surface *_this = (M4Surface *)surf;
	st = _this->stride;

	h = rc.height;
	w = rc.width;
	sx = rc.x;
	sy = rc.y;

	/*convert to 565*/
	val = M4COL_565(col);

	for (y=0; y<h; y++) {
		u16 *data = (u16 *) (_this->pixels + (sy+y) * _this->stride + 2*sx);
		for (x=0; x<w; x++)  {
			*data++ = val;
		}
	}
	return M4OK;
}



/*
			RGB 555 part
*/

static u16 overmask_555(u32 src, u16 dst, u32 alpha)
{
	u32 resr, resg, resb;
	s32 srca = (src >> 24) & 0xff;
	s32 srcr = (src >> 16) & 0xff;
	s32 srcg = (src >> 8) & 0xff;
	s32 srcb = (src >> 0) & 0xff;

	s32 dstr = (dst >> 7) & 0xf8;
	s32 dstg = (dst >> 2) & 0xf8;
	s32 dstb = (dst << 3) & 0xf8;

	srca = mul255(srca, alpha);
	resr = mul255(srca, srcr - dstr) + dstr;
	resg = mul255(srca, srcg - dstg) + dstg;
	resb = mul255(srca, srcb - dstb) + dstb;
	return M4C_555(resr, resg, resb);
}

static void overmask_555_const_run(u32 src, u16 *dst, u32 count)
{
	u32 resr, resg, resb;
	u8 srca = (src >> 24) & 0xff;
	u8 srcr = (src >> 16) & 0xff;
	u8 srcg = (src >> 8) & 0xff;
	u8 srcb = (src >> 0) & 0xff;

	while (count) {
		u16 val = *dst;
		u8 dstr = (val >> 7) & 0xf8;
		u8 dstg = (val >> 2) & 0xf8;
		u8 dstb = (val << 3) & 0xf8;

		resr = mul255(srca, srcr - dstr) + dstr;
		resg = mul255(srca, srcg - dstg) + dstg;
		resb = mul255(srca, srcb - dstb) + dstb;
		*dst = M4C_555(resr, resg, resb);
		dst++;		
		count--;
	}
}

void m4_555_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 col555 = surf->fill_555;
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
	s32 i;
	u32 x, len;
	u8 aa_lev = surf->AALevel;

	col_no_a = col&0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		x = spans[i].x;
		len = spans[i].len;
		if (spans[i].coverage != 0xFF) {
			a = mul255(0xFF, spans[i].coverage);
			fin = (a<<24) | col_no_a;
			overmask_555_const_run(fin, &dst[x], len);
		} else {
			while (len--) {
				dst[x] = col555;
				x++;
			}
		}
	}
}

void m4_555_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
	u32 col = surf->fill_col;
	u32 a, fin, col_no_a;
	s32 i;
	u8 aa_lev = surf->AALevel;

	a = (col>>24)&0xFF;
	col_no_a = col & 0x00FFFFFF;
	for (i=0; i<count; i++) {
		if (spans[i].coverage<aa_lev) continue;
		fin = mul255(a, spans[i].coverage);
		fin = (fin<<24) | col_no_a;
		overmask_555_const_run(fin, &dst[spans[i].x], spans[i].len);
	}
}


void m4_555_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf)
{
	u16 *dst = (u16 *) (surf->pixels + y * surf->stride);
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
					dst[x] = overmask_555(*col, dst[x], spanalpha);
				} else {
					dst[x] = M4COL_555(*col);
				}
			}
			col++;
			x ++;
		}
	}
}

M4Err m4_surface_clear_555(LPM4SURFACE surf, M4IRect rc, M4Color col)
{
	u32 x, y, w, h, st, sx, sy;
	u16 val;
	M4Surface *_this = (M4Surface *)surf;
	st = _this->stride;

	h = rc.height;
	w = rc.width;
	sx = rc.x;
	sy = rc.y;

	/*convert to 565*/
	val = M4COL_555(col);

	for (y=0; y<h; y++) {
		u16 *data = (u16 *) (_this->pixels + (sy+y) * _this->stride + 2*sx);
		for (x=0; x<w; x++)  {
			*data++ = val;
		}
	}
	return M4OK;
}
