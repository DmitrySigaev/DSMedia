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


#ifndef _IMG_DEC_H
#define _IMG_DEC_H

/*all codecs are regular */
#include <gpac/m4_decoder.h>
#include <gpac/m4_network.h>

enum
{
	DEC_RESERVED = 0,
	DEC_PNG,
	DEC_JPEG,
	DEC_BMP,
};

typedef struct
{
	u32 type;
	void *opaque;
} IMGDec;

/*all constructors shall setup the wraper type and handle
	return 1 for success, 0 otherwise
all destructors only destroy their private stacks (eg not the interface nor the wraper)
*/
#ifdef M4_HAS_PNG
u32 NewPNGDec(BaseDecoder *dec);
void DeletePNGDec(BaseDecoder *dec);
#endif

#ifdef M4_HAS_JPEG
u32 NewJPEGDec(BaseDecoder *dec);
void DeleteJPEGDec(BaseDecoder *dec);
#endif


#ifndef _WIN32
typedef struct tagBITMAPFILEHEADER 
{
    u16	bfType;
    u32	bfSize;
    u16	bfReserved1;
    u16	bfReserved2;
    u32 bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	u32	biSize;
	s32	biWidth;
	s32	biHeight;
	u16	biPlanes;
	u16	biBitCount;
	u32	biCompression;
	u32	biSizeImage;
	s32	biXPelsPerMeter;
	s32	biYPelsPerMeter;
	u32	biClrUsed;
	u32	biClrImportant;
} BITMAPINFOHEADER;

#define BI_RGB        0L

#endif

#define GPAC_BMP_OTI	0x81

u32 NewBMPDec(BaseDecoder *dec);
void DeleteBMPDec(BaseDecoder *dec);


void *NewLoaderInterface();
void DeleteLoaderInterface(void *ifce);

#endif
