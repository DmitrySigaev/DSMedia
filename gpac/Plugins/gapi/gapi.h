/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / DirectX audio and video render plugin
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


#ifndef _GAPI_H
#define _GAPI_H

/*driver interfaces*/
#include <gpac/m4_avhw.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u32 width, height, pitch, pixel_format;
	unsigned char *buffer;
	u32 ID;
} GAPISurface;

typedef struct
{
	HWND hWnd;
	M4Mutex *mx;
	struct GXDisplayProperties g_prop;
	Bool gapi_open;
	Chain *surfaces;
	
	u32 screen_w, screen_h, stride_x, stride_y;
	u32 fs_w, fs_h;
	/*store w and h for fullscreen*/
	u32 disp_w, disp_h;

	Bool fullscreen;
	Bool is_init;

	/*main surface info*/
	unsigned char *backbuffer;
	u32 bb_size, bb_width, bb_height, bb_pitch;
	u32 pixel_format;
	u32 BPP, bitsPP;

	M4Window dst_blt;

	Bool is_resizing;
	DWORD ThreadID;
	HANDLE hThread;
	Bool owns_hwnd;
} GAPIPriv;


#ifdef __cplusplus
}
#endif

#endif 
