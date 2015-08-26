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


#ifndef __COLORCONV_H_
#define __COLORCONV_H_


#include <gpac/m4_tools.h>

#ifdef __cplusplus
extern "C" {
#endif

/*rotation flags when stretching and rotating*/
enum
{
	SB_ROT0,
	SB_ROT90,
	SB_ROT180,
	SB_ROT270,
};

void StretchBits(void *dst, u32 dst_bpp, u32 dst_w, u32 dst_h, u32 dst_pitch,
				void *src, u32 src_bpp, u32 src_w, u32 src_h, u32 src_pitch, 
				u32 FlipIt);

void RotateBits(void *dst, u32 dst_bpp, u32 dst_w, u32 dst_h, u32 dst_pitch,
				void *src, u32 src_bpp, u32 src_w, u32 src_h, u32 src_pitch,
				u32 Rotate);


enum 
{
	YUV_MODE_RGB  = 0x01,
	YUV_MODE_BGR,
};


void yuv_stretch_init(int bpp, int mode);
void yuv_stretch_close ();

void StretchYUVtoDest(void * dst,
					unsigned char * py, unsigned char * pu, unsigned char * pv, 
					u32 src_w, u32 src_h, 
					u32 dst_w, u32 dst_h, u32 dst_pitch);

void RotateYUVtoDest(void * dst,
					unsigned char * py, unsigned char * pu, unsigned char * pv, 
					u32 src_w, u32 src_h, 
					u32 dst_w, u32 dst_h, u32 dst_pitch, u32 Rotate);


void RotateBits565(void *dst, u32 dst_pitch, void *src, u32 src_pitch, u32 width, u32 height);

#ifdef __cplusplus
}
#endif

#endif