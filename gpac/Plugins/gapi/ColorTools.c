/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / GAPI WinCE video render plugin
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


#include "ColorTools.h"

#define RGB555(r,g,b) (((r&248)<<7) + ((g&248)<<2)  + (b>>3))
#define RGB565(r,g,b) (((r&248)<<8) + ((g&252)<<3)  + (b>>3))

void CopyPrevRow(u8 *src, u8 *dst, u32 dst_w, u32 BPP)
{
	s32 size;
	switch (BPP) {
	case 15:
	case 16:
		size = sizeof(unsigned char)*2;
		break;
	case 24:
		size = sizeof(unsigned char)*3;
		break;
	case 32:
		size = sizeof(unsigned char)*4;
		break;
	default:
		return;
	}
	memcpy(dst, src, dst_w*size);
}

/*copy and stretch a row from src to dst in 8bpp mode (NOT TESTED)*/
void CopyRow_8bpp(u8 *src, u32 src_w, u8 *dst, u32 dst_w)
{
	s32 i;
	s32 pos, inc;
	u8 pixel = 0;

	if (src_w == dst_w) {
		memcpy(dst, src, sizeof(u8)*src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel;
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 16bpp mode*/
void CopyRow_16bpp(u16 *src, u32 src_w, u16 *dst, u32 dst_w)
{
	s32 i;
	s32 pos, inc;
	u16 pixel = 0;

	if (src_w == dst_w) {
		memcpy(dst, src, sizeof(u16)*src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel;
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 24bpp mode*/
void CopyRow_24bpp(u8 *src, u32 src_w, u8 *dst, u32 dst_w)
{
	s32 i;
	s32 pos, inc;
	u8 pixel[3];

	if (src_w == dst_w) {
		memcpy(dst, src, sizeof(u8)*3*src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel[0] = *src++;
			pixel[1] = *src++;
			pixel[2] = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel[0];
		*dst++ = pixel[1];
		*dst++ = pixel[2];
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 32bpp mode*/
void CopyRow_32bpp(u32 *src, u32 src_w, u32 *dst, u32 dst_w)
{
	s32 i;
	s32 pos, inc;
	u32 pixel = 0;

	if (src_w == dst_w) {
		memcpy(dst, src, sizeof(u32)*src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for ( i=dst_w; i>0; --i ) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		*dst++ = pixel;
		pos += inc;
	}
}

/*converts a pixel row from src_bpp to dst_bpp (no stretch)*/
void ConvertRGBLine(u8 *src_bits, u32 src_bpp, 
					u8 *dst_bits, u32 dst_bpp, 
					u32 width)
{
	u32 i;
	u8 r, g, b, a;

	for (i=0; i<width; i++) {
		switch (src_bpp) {
		case 16:
			r = ((u8)(*((u16*)src_bits + i)&31))<<3;
			g = ((u8)((*((u16*)src_bits + i)&2016)>>5))<<2;
			b = ((u8)((*((u16*)src_bits + i)&63488)>>11))<<3;
			a = 0;
			break;
		case 24:
			r = *(src_bits + i*3);
			g = *(src_bits + i*3 + 1);
			b = *(src_bits + i*3 + 2);
			a = 0;
			break;
		case 32:
			r = *(src_bits + i*4);
			g = *(src_bits + i*4 + 1);
			b = *(src_bits + i*4 + 2);
			a = *(src_bits + i*4 + 3);
			break;
		default:
			return;
		}

		switch (dst_bpp) {
		case 15:
			*((u16 *) (dst_bits + i*2)) = RGB555(r, g, b);
			break;
		case 16:
			* ( (u16 *) (dst_bits + i*2)) = RGB565(b, g, r);
			break;
		case 24:
			*(dst_bits + i*3) = r;
			*(dst_bits + i*3 + 1) = g;
			*(dst_bits + i*3 + 2) = b;
			break;
		case 32:
			*(dst_bits + i*4) = r;
			*(dst_bits + i*4 + 1) = g;
			*(dst_bits + i*4 + 2) = b;
			*(dst_bits + i*4 + 3) = a;
			break;
		default:
			return;
		}
	}
}

void StretchBits(void *dst, u32 dst_bpp, u32 dst_w, u32 dst_h, u32 dst_pitch,
				void *src, u32 src_bpp, u32 src_w, u32 src_h, u32 src_pitch,
				Bool FlipIt)
{
	u8 *tmp;
	s32 src_row;
	u32 j;
	s32 pos, inc, prev_row;
	u8 *src_bits = NULL, *dst_bits = NULL, *copyfrom;


	/*we need a local buffer for RGB line conversion*/
	if (dst_bpp != src_bpp) {
		tmp = malloc(sizeof(char) * src_w * dst_bpp/8);
	} else {
		tmp = NULL;
	}

	pos = 0x10000;
	inc = (src_h << 16) / dst_h;
	src_row = 0;
	prev_row = -1;

	/*copy raw by raw*/
	for (j = 0; j<dst_h; j++) {
		if (FlipIt) {
			dst_bits = (u8 *) dst + ((dst_h-j-1) * dst_pitch);
		} else {
			dst_bits = (u8 *) dst + (j * dst_pitch);
		}

		while ( pos >= 0x10000L ) {
			src_bits = (u8 *) src + (src_row * src_pitch);
			src_row++;
			pos -= 0x10000L;
		}
		/*if same row, do a brutal memcpy*/
		if ( prev_row == src_row) {
			CopyPrevRow(((u8 *) dst + ((j - 1) * dst_pitch) ), dst_bits, dst_w, (u8) dst_bpp);
			pos += inc;
			continue;
		}
		/*new row, check if conversion is needed*/
		if (tmp) {
			ConvertRGBLine(src_bits, src_bpp, tmp, dst_bpp, src_w);
			copyfrom = tmp;
		} else {
			copyfrom = src_bits;
		}

		/*finally draw to destination*/
		switch (dst_bpp) {
	    case 8:
			CopyRow_8bpp(copyfrom, src_w, dst_bits, dst_w);
			break;
	    case 15:
	    case 16:
			CopyRow_16bpp((u16 *) copyfrom, src_w, (u16 *) dst_bits, dst_w);
			break;
	    case 24:
			CopyRow_24bpp(copyfrom, src_w, dst_bits, dst_w);
			break;
	    case 32:
			CopyRow_32bpp((u32 *) copyfrom, src_w, (u32 *) dst_bits, dst_w);
			break;
		}
		pos += inc;

		prev_row = src_row;
	}
	if (tmp) free(tmp);
}


/*copy and stretch a row from src to dst in 8bpp mode (NOT TESTED)*/
void RotateRow_8bpp(u8 *src, u32 src_w, u8 *dst, u32 dst_w, u32 dst_h, u32 pitch, u32 col_num, u32 Rotate)
{
	u32 i;
	s32 pos, inc;
	u8 *val, pixel = 0;

	if ( (src_w == dst_w) && (Rotate==SB_ROT0)) {
		memcpy(dst + col_num * pitch, src, sizeof(u8) * src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for (i=0; i<dst_w; i++) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		/*dst is always the top-left of the output (rotated) buffer*/
		switch (Rotate) {
		case SB_ROT90:
			val = (dst + dst_h - col_num) + pitch*i;
			break;
		case SB_ROT270:
			val = dst + (dst_h - col_num)*pitch + (dst_w-i);
			break;
		case SB_ROT180:
			val = (dst + col_num) + pitch*(dst_w-i);
			break;
		/*no rotation*/
		default:
			val = dst + col_num*pitch + i;
			break;
		}
		*val = pixel;
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 16bpp mode*/
void RotateRow_16bpp(u16 *src, u32 src_w, u16 *dst, u32 dst_w, u32 dst_h, u32 pitch, u32 col_num, u32 Rotate)
{
	u32 i;
	s32 pos, inc;
	u16 *val, pixel = 0;

	/*pitch is in bytes, our buffers are in pixels - scale down*/
	pitch /= 2;

	if ( (src_w == dst_w) && (Rotate==SB_ROT0)) {
		memcpy(dst + col_num * pitch, src, sizeof(u16)*src_w);
		return;
	}
	
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for (i=0; i<dst_w; i++) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		/*dst is always the top-left of the output (rotated) buffer*/
		switch (Rotate) {
		case SB_ROT90:
			val = (dst + dst_h - col_num) + pitch*i;
			break;
		case SB_ROT270:
			val = dst + (dst_h - col_num)*pitch + (dst_w-i);
			break;
		case SB_ROT180:
			val = (dst + col_num) + pitch*(dst_w-i);
			break;
		/*no rotation*/
		default:
			val = dst + col_num*pitch + i;
			break;
		}
		*val = pixel;
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 24bpp mode*/
void RotateRow_24bpp(u8 *src, u32 src_w, u8 *dst, u32 dst_w, u32 dst_h, u32 pitch, u32 col_num, u32 Rotate)
{
	u32 i;
	s32 pos, inc;
	u8 *val, pixel[3];

	/*pitch is in bytes, our buffers are in butes too - no scale down*/
	
	if (src_w == dst_w) {
		memcpy(dst + col_num * pitch, src, sizeof(u8)*3*src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for (i=0; i<dst_w; i++) {
		while ( pos >= 0x10000L ) {
			pixel[0] = *src++;
			pixel[1] = *src++;
			pixel[2] = *src++;
			pos -= 0x10000L;
		}
		/*dst is always the top-left of the output (rotated) buffer*/
		switch (Rotate) {
		case SB_ROT90:
			val = (dst + dst_h - col_num) + pitch*i;
			break;
		case SB_ROT270:
			val = dst + (dst_h - col_num)*pitch + (dst_w-i);
			break;
		case SB_ROT180:
			val = (dst + col_num) + pitch*(dst_w-i);
			break;
		/*no rotation*/
		default:
			val = dst + col_num*pitch + i;
			break;
		}
		val[0] = pixel[0];
		val[1] = pixel[1];
		val[2] = pixel[2];
		pos += inc;
	}
}

/*copy and stretch a row from src to dst in 32bpp mode*/
void RotateRow_32bpp(u32 *src, u32 src_w, u32 *dst, u32 dst_w, u32 dst_h, u32 pitch, u32 col_num, u32 Rotate)
{
	u32 i;
	s32 pos, inc;
	u32 *val, pixel = 0;

	/*pitch is in bytes, our buffers are in pixels - scale down*/
	pitch /= 4;

	if ( (src_w == dst_w) && (Rotate==SB_ROT0)) {
		memcpy(dst + col_num * pitch, src, sizeof(u32) * src_w);
		return;
	}
	pos = 0x10000;
	inc = (src_w << 16) / dst_w;
	for (i=0; i<dst_w; i++) {
		while ( pos >= 0x10000L ) {
			pixel = *src++;
			pos -= 0x10000L;
		}
		/*dst is always the top-left of the output (rotated) buffer*/
		switch (Rotate) {
		case SB_ROT90:
			val = (dst + dst_h - col_num) + pitch*i;
			break;
		case SB_ROT270:
			val = dst + (dst_h - col_num)*pitch + (dst_w-i);
			break;
		case SB_ROT180:
			val = (dst + col_num) + pitch*(dst_w-i);
			break;
		/*no rotation*/
		default:
			val = dst + col_num*pitch + i;
			break;
		}
		*val = pixel;
		pos += inc;
	}
}

void RotateBits(void *dst, u32 dst_bpp, u32 dst_w, u32 dst_h, u32 dst_pitch,
				void *src, u32 src_bpp, u32 src_w, u32 src_h, u32 src_pitch, 
				u32 Rotate)
{
	u8 *tmp;
	s32 src_row;
	u32 j;
	s32 pos, inc, prev_row;
	u8 *src_bits = NULL, *copyfrom;


	/*we need a local buffer for RGB line conversion*/
	if (dst_bpp != src_bpp) {
		tmp = malloc(sizeof(char) * src_w * dst_bpp/8);
	} else {
		tmp = NULL;
	}

	pos = 0x10000;
	inc = (src_h << 16) / dst_h;
	src_row = 0;
	prev_row = -1;

	/*copy raw by raw*/
	for (j = 0; j<dst_h; j++) {
		while ( pos >= 0x10000L ) {
			src_bits = (u8 *) src + (src_row * src_pitch);
			src_row++;
			pos -= 0x10000L;
		}

		/*new row, check if conversion is needed*/
		if (prev_row != src_row) {
			if (tmp) {
				ConvertRGBLine(src_bits, src_bpp, tmp, dst_bpp, src_w);
				copyfrom = tmp;
			} else {
				copyfrom = src_bits;
			}
		}

		/*finally draw to destination*/
		switch (dst_bpp) {
	    case 8:
			RotateRow_8bpp(copyfrom, src_w, dst, dst_w, dst_h, dst_pitch, j, Rotate);
			break;
	    case 15:
	    case 16:
			RotateRow_16bpp((u16 *) copyfrom, src_w, (u16 *) dst, dst_w, dst_h, dst_pitch, j, Rotate);
			break;
	    case 24:
			RotateRow_24bpp(copyfrom, src_w, dst, dst_w, dst_h, dst_pitch, j, Rotate);
			break;
	    case 32:
			RotateRow_32bpp((u32 *) copyfrom, src_w, (u32 *) dst, dst_w, dst_h, dst_pitch, j, Rotate);
			break;
		}
		pos += inc;
		prev_row = src_row;
	}
	if (tmp) free(tmp);
}



static s32 BPP;
u32 matrix_coefficients = 6;

const s32 Inverse_Table_6_9[8][4] = {
    {117504, 138453, 13954, 34903}, /* no sequence_display_extension */
    {117504, 138453, 13954, 34903}, /* ITU-R Rec. 709 (1990) */
    {104597, 132201, 25675, 53279}, /* unspecified */
    {104597, 132201, 25675, 53279}, /* reserved */
    {104448, 132798, 24759, 53109}, /* FCC */
    {104597, 132201, 25675, 53279}, /* ITU-R Rec. 624-4 System B, G */
    {104597, 132201, 25675, 53279}, /* SMPTE 170M */
    {117579, 136230, 16907, 35559}  /* SMPTE 240M (1987) */
};

/*static funct used assigned upon init (depends on BPP)*/
static void (* yuv2rgb_c_internal) (unsigned char *, unsigned char *,
				    unsigned char *, unsigned char *,
				    void *, void *, s32);


void StretchYUVtoDest(void * dst,
					unsigned char * py, unsigned char * pu, unsigned char * pv, 
					u32 src_w, u32 src_h, 
					u32 dst_w, u32 dst_h, u32 dst_pitch)
{
	void *ptr;
	s32 y_stride, uv_stride;
	s32 loc_pitch, DoYUV, loc_h;
	s32 pos, inc, loc_inc;
	u32 src_row, dst_row, logical_row, prev_row;
	u8 *srcp = NULL, *dstp = NULL;

	y_stride = src_w;
	uv_stride=y_stride >> 1;

	/*no stretch involved*/
	if ((dst_w == src_w) && (dst_h == src_h)) {
		src_h>>= 1;
		while (src_h--) {
			yuv2rgb_c_internal (py, py + y_stride, pu, pv, dst, (void*)((s32)dst + dst_pitch), src_w);
			py += 2 * y_stride;
			pu += uv_stride;
			pv += uv_stride;
			dst = (void*)((s32)dst + 2 * dst_pitch);
		}
		return;
	}

	/*stretch the surface. the YUV2RGB processes 2 rows at a time. We just create a temp buffer for the 2
	RGB rows and we will stretch after*/
	switch (BPP) {
	case 15:
	case 16:
		ptr = malloc(sizeof(unsigned char)*2*src_w * 2);
		loc_pitch = src_w*2;
		break;
	case 24:
		ptr = malloc(sizeof(unsigned char)*3*src_w * 2);
		loc_pitch = src_w*3;
		break;	
	case 32:
		ptr = malloc(sizeof(unsigned char)*4*src_w * 2);
		loc_pitch = src_w*4;
		break;
	default:
		return;
	}

	pos = 0x10000;
	inc = (src_h << 16) / dst_h;
	logical_row = 0;
	src_row = dst_row = 0;
	prev_row = -1;

	loc_inc = 0;
	DoYUV = 1;

	loc_h = src_h >> 1;
	
	while (1) {
yuv_rgb_loc:

		/*do the YUV to RGB - NO PITCH*/
		if (loc_h && DoYUV) {
			yuv2rgb_c_internal (py, py + y_stride, pu, pv, ptr, (void*)((s32)ptr + loc_pitch), src_w);
			py += 2 * y_stride;
			pu += uv_stride;
			pv += uv_stride;

			loc_h--;
			DoYUV = 0;
		}

		/*jump to the right line in the dst*/
		dstp = (u8 *) dst + (dst_row * dst_pitch);

		/*check wether the src line has to be copied*/
		while ( pos >= 0x10000L ) {
			logical_row++;
			loc_inc++;
			pos -= 0x10000L;
		}
		
		/*do we need to do some YUV ?*/
		if (loc_inc >= 2) {
			loc_inc -= 2;
			src_row = 0;
			DoYUV = 1;
			goto yuv_rgb_loc;
		}
		/*we copy the previous row, do a brute memcpy*/
		if ( dst_row && (prev_row == logical_row)) {
			CopyPrevRow(((u8 *) dst + ((dst_row - 1) * dst_pitch) ), dstp, dst_w, (u8) BPP);
			goto go_on;
		}
		src_row = loc_inc;
		srcp = (u8 *)ptr + (src_row*loc_pitch);
		
		switch (BPP) {
		case 15:
		case 16:
			CopyRow_16bpp((u16 *)srcp, src_w, (u16 *)dstp, dst_w);
			break;
		case 24:
			CopyRow_24bpp(srcp, src_w, dstp, dst_w);
			break;
		case 32:
			CopyRow_32bpp((u32 *)srcp, src_w, (u32 *)dstp, dst_w);
			break;
		}
go_on:
		pos += inc;
		dst_row ++;
		prev_row = logical_row;

		if (src_row > 1) {
			src_row = 0;
			DoYUV = 1;
		}
		/*check for exit*/
		if (dst_row == dst_h) break;
	}

	free(ptr);
}




void RotateYUVtoDest(void * dst,
					unsigned char * py, unsigned char * pu, unsigned char * pv, 
					u32 src_w, u32 src_h, 
					u32 dst_w, u32 dst_h, u32 dst_pitch, u32 Rotate)
{
	void *ptr;
	s32 y_stride, uv_stride;
	s32 loc_pitch, DoYUV, loc_h;
	s32 pos, inc, loc_inc;
	u32 src_row, dst_col, logical_row;
	u8 *srcp = NULL;

	y_stride = src_w;
	uv_stride=y_stride >> 1;

	/*no stretch/rotation involved*/
	if ((dst_w == src_w) && (dst_h == src_h) && (Rotate==SB_ROT0) ) {
		src_h>>= 1;
		while (src_h--) {
			yuv2rgb_c_internal (py, py + y_stride, pu, pv, dst, (void*)((s32)dst + dst_pitch), src_w);
			py += 2 * y_stride;
			pu += uv_stride;
			pv += uv_stride;
			dst = (void*)((s32)dst + 2 * dst_pitch);
		}
		return;
	}

	/*stretch the surface. the YUV2RGB processes 2 rows at a time. We just create a temp buffer for the 2
	RGB rows and we will stretch after*/
	switch (BPP) {
	case 15:
	case 16:
		ptr = malloc(sizeof(unsigned char)*2*src_w * 2);
		loc_pitch = src_w*2;
		break;
	case 24:
		ptr = malloc(sizeof(unsigned char)*3*src_w * 2);
		loc_pitch = src_w*3;
		break;	
	case 32:
		ptr = malloc(sizeof(unsigned char)*4*src_w * 2);
		loc_pitch = src_w*4;
		break;
	default:
		return;
	}

	/*we assume the surface we've been given is the begining of the draw*/
	pos = 0x10000;
	inc = (src_h << 16) / dst_h;
	logical_row = 0;
	src_row = dst_col = 0;

	loc_inc = 0;
	DoYUV = 1;

	loc_h = src_h >> 1;
	
	while (1) {

yuv_rgb_loc:

		/*do the YUV to RGB - NO PITCH*/
		if (loc_h && DoYUV) {
			yuv2rgb_c_internal (py, py + y_stride, pu, pv, ptr, (void*)((s32)ptr + loc_pitch), src_w);
			py += 2 * y_stride;
			pu += uv_stride;
			pv += uv_stride;
			loc_h--;
			DoYUV = 0;
		}

		/*check wether the src line has to be copied*/
		while ( pos >= 0x10000L ) {
			logical_row++;
			loc_inc++;
			pos -= 0x10000L;
		}
		
		/*do we need to do some YUV ?*/
		if (loc_inc >= 2) {
			loc_inc -= 2;
			src_row = 0;
			DoYUV = 1;
			goto yuv_rgb_loc;
		}
		/*select the right RGB row*/
		src_row = loc_inc;
		srcp = (u8 *)ptr + (src_row*loc_pitch);
		
		switch (BPP) {
		case 15:
		case 16:
			RotateRow_16bpp((u16 *)srcp, src_w, (u16 *) dst, dst_w, dst_h, dst_pitch, dst_col, Rotate);
			break;
		case 24:
			RotateRow_24bpp(srcp, src_w, dst, dst_w, dst_h, dst_pitch, dst_col, Rotate);
			break;
		case 32:
			RotateRow_32bpp((u32 *)srcp, src_w, (u32 *)dst, dst_w, dst_h, dst_pitch, dst_col, Rotate);
			break;
		}

		pos += inc;
		dst_col ++;

		if (src_row > 1) {
			src_row = 0;
			DoYUV = 1;
		}
		/*check for exit*/
		if (dst_col == dst_h) break;
	}
	free(ptr);
}



#ifdef RGB
#undef RGB
#endif


void * table_rV[256];
void * table_gU[256];
void * table_bU[256];
s32 table_gV[256];
void * table;

#define RGB(i)					\
	U = pu[i];				\
	V = pv[i];				\
	r = table_rV[V];			\
	g = (void*)(((s32)table_gU[U]) + table_gV[V]);		\
	b = table_bU[U];

#define DST1(i)					\
	Y = py_1[2*i];				\
	dst_1[2*i] = r[Y] + g[Y] + b[Y];	\
	Y = py_1[2*i+1];			\
	dst_1[2*i+1] = r[Y] + g[Y] + b[Y];

#define DST2(i)					\
	Y = py_2[2*i];				\
	dst_2[2*i] = r[Y] + g[Y] + b[Y];	\
	Y = py_2[2*i+1];			\
	dst_2[2*i+1] = r[Y] + g[Y] + b[Y];

#define DST1RGB(i)							\
	Y = py_1[2*i];							\
	dst_1[6*i] = r[Y]; dst_1[6*i+1] = g[Y]; dst_1[6*i+2] = b[Y];	\
	Y = py_1[2*i+1];						\
	dst_1[6*i+3] = r[Y]; dst_1[6*i+4] = g[Y]; dst_1[6*i+5] = b[Y];

#define DST2RGB(i)							\
	Y = py_2[2*i];							\
	dst_2[6*i] = r[Y]; dst_2[6*i+1] = g[Y]; dst_2[6*i+2] = b[Y];	\
	Y = py_2[2*i+1];						\
	dst_2[6*i+3] = r[Y]; dst_2[6*i+4] = g[Y]; dst_2[6*i+5] = b[Y];

#define DST1BGR(i)							\
	Y = py_1[2*i];							\
	dst_1[6*i] = b[Y]; dst_1[6*i+1] = g[Y]; dst_1[6*i+2] = r[Y];	\
	Y = py_1[2*i+1];						\
	dst_1[6*i+3] = b[Y]; dst_1[6*i+4] = g[Y]; dst_1[6*i+5] = r[Y];

#define DST2BGR(i)							\
	Y = py_2[2*i];							\
	dst_2[6*i] = b[Y]; dst_2[6*i+1] = g[Y]; dst_2[6*i+2] = r[Y];	\
	Y = py_2[2*i+1];						\
	dst_2[6*i+3] = b[Y]; dst_2[6*i+4] = g[Y]; dst_2[6*i+5] = r[Y];

static void yuv2rgb_c_32 (unsigned char * py_1, unsigned char * py_2,
			  unsigned char * pu, unsigned char * pv,
			  void * _dst_1, void * _dst_2, s32 h_size)
{
    s32 U, V, Y;
    u32 * r, * g, * b;
    u32 * dst_1, * dst_2;

    h_size >>= 3;
    dst_1 = (u32*)_dst_1;
    dst_2 = (u32*)_dst_2;

    while (h_size--) {
		RGB(0);
		DST1(0);
		DST2(0);

		RGB(1);
		DST2(1);
		DST1(1);

		RGB(2);
		DST1(2);
		DST2(2);

		RGB(3);
		DST2(3);
		DST1(3);

		pu += 4;
		pv += 4;
		py_1 += 8;
		py_2 += 8;
		dst_1 += 8;
		dst_2 += 8;
    }
}

/* This is very near from the yuv2rgb_c_32 code*/
static void yuv2rgb_c_24_rgb (unsigned char * py_1, unsigned char * py_2,
			      unsigned char * pu, unsigned char * pv,
			      void * _dst_1, void * _dst_2, s32 h_size)
{
    s32 U, V, Y;
    unsigned char * r, * g, * b;
    unsigned char * dst_1, * dst_2;

    h_size >>= 3;
    dst_1 = _dst_1;
    dst_2 = _dst_2;

    while (h_size--) {
		RGB(0);
		DST1RGB(0);
		DST2RGB(0);

		RGB(1);
		DST2RGB(1);
		DST1RGB(1);

		RGB(2);
		DST1RGB(2);
		DST2RGB(2);

		RGB(3);
		DST2RGB(3);
		DST1RGB(3);

		pu += 4;
		pv += 4;
		py_1 += 8;
		py_2 += 8;
		dst_1 += 24;
		dst_2 += 24;
    }
}

/* only trivial mods from yuv2rgb_c_24_rgb*/
static void yuv2rgb_c_24_bgr (unsigned char * py_1, unsigned char * py_2,
			      unsigned char * pu, unsigned char * pv,
			      void * _dst_1, void * _dst_2, s32 h_size)
{
    s32 U, V, Y;
    unsigned char * r, * g, * b;
    unsigned char * dst_1, * dst_2;

    h_size >>= 3;
    dst_1 = _dst_1;
    dst_2 = _dst_2;

    while (h_size--) {
		RGB(0);
		DST1BGR(0);
		DST2BGR(0);

		RGB(1);
		DST2BGR(1);
		DST1BGR(1);

		RGB(2);
		DST1BGR(2);
		DST2BGR(2);

		RGB(3);
		DST2BGR(3);
		DST1BGR(3);

		pu += 4;
		pv += 4;
		py_1 += 8;
		py_2 += 8;
		dst_1 += 24;
		dst_2 += 24;
    }
}

/* This is exactly the same code as yuv2rgb_c_32 except for the types of
 r, g, b, dst_1, dst_2*/
static void yuv2rgb_c_16 (unsigned char * py_1, unsigned char * py_2,
			  unsigned char * pu, unsigned char * pv,
			  void * _dst_1, void * _dst_2, s32 h_size)
{
    s32 U, V, Y;
    unsigned short * r, * g, * b;
    unsigned short * dst_1, * dst_2;

    h_size >>= 3;
    dst_1 = _dst_1;
    dst_2 = _dst_2;

    while (h_size--) {
		RGB(0);
		DST1(0);
		DST2(0);

		RGB(1);
		DST2(1);
		DST1(1);

		RGB(2);
		DST1(2);
		DST2(2);

		RGB(3);
		DST2(3);
		DST1(3);

		pu += 4;
		pv += 4;
		py_1 += 8;
		py_2 += 8;
		dst_1 += 8;
		dst_2 += 8;
    }
}

static s32 div_round (s32 dividend, s32 divisor)
{
    if (dividend > 0)
	return (dividend + (divisor>>1)) / divisor;
    else
	return -((-dividend + (divisor>>1)) / divisor);
}


void yuv_stretch_init(s32 bpp, s32 mode) 
{
    s32 i, j;
    unsigned char table_Y[1024];
    u32 *table_32 = 0;
    unsigned short *table_16 = 0;
    unsigned char *table_8 = 0;
    u32 entry_size = 0;
    void *table_r = 0, *table_g = 0, *table_b = 0;

    s32 crv = Inverse_Table_6_9[matrix_coefficients][0];
    s32 cbu = Inverse_Table_6_9[matrix_coefficients][1];
    s32 cgu = -Inverse_Table_6_9[matrix_coefficients][2];
    s32 cgv = -Inverse_Table_6_9[matrix_coefficients][3];

	table=0;

    for (i = 0; i < 1024; i++) {
		j = (76309 * (i - 384 - 16) + 32768) >> 16;
		j = (j < 0) ? 0 : ((j > 255) ? 255 : j);
		table_Y[i] = j;
    }

    switch (bpp) {
    case 32:
		yuv2rgb_c_internal = yuv2rgb_c_32;

		table_32 = malloc ((197 + 2*682 + 256 + 132) * sizeof (u32));
		table = table_32;

		entry_size = sizeof (u32);
		table_r = table_32 + 197;
		table_b = table_32 + 197 + 685;
		table_g = table_32 + 197 + 2*682;

		for (i = -197; i < 256+197; i++)
			((u32 *)table_r)[i] = table_Y[i+384] << ((mode==YUV_MODE_RGB) ? 16 : 0);
		for (i = -132; i < 256+132; i++)
			((u32 *)table_g)[i] = table_Y[i+384] << 8;
		for (i = -232; i < 256+232; i++)
			((u32 *)table_b)[i] = table_Y[i+384] << ((mode==YUV_MODE_RGB) ? 0 : 16);
	break;

    case 24:
		yuv2rgb_c_internal = (mode==YUV_MODE_RGB) ? yuv2rgb_c_24_rgb : yuv2rgb_c_24_bgr;

		table_8 = malloc ((256 + 2*232) * sizeof (unsigned char));
		table = table_8;

		entry_size = sizeof (unsigned char);
		table_r = table_g = table_b = table_8 + 232;

		for (i = -232; i < 256+232; i++)
			((unsigned char * )table_b)[i] = table_Y[i+384];
		break;

    case 15:
    case 16:
		yuv2rgb_c_internal = yuv2rgb_c_16;

		table_16 = malloc ((197 + 2*682 + 256 + 132) * sizeof (unsigned short));
		table = table_16;

		entry_size = sizeof (unsigned short);
		table_r = table_16 + 197;
		table_b = table_16 + 197 + 685;
		table_g = table_16 + 197 + 2*682;

		for (i = -197; i < 256+197; i++) {
			s32 j = table_Y[i+384] >> 3;

			if (mode == YUV_MODE_RGB)
			j <<= ((bpp==16) ? 11 : 10);

			((unsigned short *)table_r)[i] = j;
		}
		for (i = -132; i < 256+132; i++) {
			s32 j = table_Y[i+384] >> ((bpp==16) ? 2 : 3);

			((unsigned short *)table_g)[i] = j << 5;
		}
		for (i = -232; i < 256+232; i++) {
			s32 j = table_Y[i+384] >> 3;

			if (mode == YUV_MODE_BGR)
			j <<= ((bpp==16) ? 11 : 10);

			((unsigned short *)table_b)[i] = j;
		}
		break;

    default:
		exit (1);
    }

    for (i = 0; i < 256; i++) {
		table_rV[i] = (void*)((s32)table_r + entry_size * div_round (crv * (i-128), 76309));
		table_gU[i] = (void*)((s32)table_g + entry_size * div_round (cgu * (i-128), 76309));
		table_gV[i] = entry_size * div_round (cgv * (i-128), 76309);
		table_bU[i] = (void*)((s32)table_b + entry_size * div_round (cbu * (i-128), 76309));
    }
	BPP = bpp;
}

void yuv_stretch_close()
{
	if (table) free(table);
}
