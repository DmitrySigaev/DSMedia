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


#ifndef _YUV2RGB_H
#define _YUV2RGB_H


#ifdef __cplusplus
extern "C" {
#endif


#include <gpac/m4_tools.h>


void yuv2rgb_555(unsigned char *dst, s32 dst_stride,
				 unsigned char *y_src, unsigned char * u_src, unsigned char * v_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);


void yuv2rgb_565(unsigned char * dst, s32 dst_stride,
				 unsigned char* y_src, unsigned char* u_src, unsigned char* v_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);

void yuv2bgr_24(unsigned char *dst, s32 dststride, 
				unsigned char *y_src, unsigned char *u_src, unsigned char * v_src, 
				s32 y_stride, s32 uv_stride, s32 width, s32 height);

void yuv2rgb_32(unsigned char *dst, s32 dststride, 
				unsigned char *y_src, unsigned char *v_src, unsigned char * u_src,
				s32 y_stride, s32 uv_stride, s32 width, s32 height);

void yuva2rgb_32(unsigned char *dst, s32 dststride, 
				 unsigned char *y_src, unsigned char *v_src, unsigned char * u_src, unsigned char *a_src,
				 s32 y_stride, s32 uv_stride, s32 width, s32 height);




#ifdef __cplusplus
}
#endif

#endif
