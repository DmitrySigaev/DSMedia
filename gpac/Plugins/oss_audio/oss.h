/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / linux_oss audio render plugin
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


#ifndef __LINUX_AUDIO_OSS_H_
#define __LINUX_AUDIO_OSS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <gpac/m4_avhw.h>

#define OSS_AUDIO_DEVICE	"/dev/dsp"

typedef struct 
{
	int audio_device;
	int buf_size;
	char *wav_buf;
} OSSContext;


#ifdef __cplusplus
}
#endif

#endif
