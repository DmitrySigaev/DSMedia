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


#include "img_in.h"


static Bool DEC_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	IMGDec *wrap = (IMGDec *)dec->privateStack;

	if (StreamType != M4ST_VISUAL) return 0;

	/*should never happen*/
	assert(wrap->type == DEC_RESERVED);

	switch (ObjectType) {
#ifdef M4_HAS_PNG
	case 0x6D: return NewPNGDec(dec);
#endif
#ifdef M4_HAS_JPEG
	case 0x6C: return NewJPEGDec(dec);
#endif
	case GPAC_BMP_OTI: return NewBMPDec(dec);
	case 0: return 1;/*query for types*/
 	default: return 0;
	}
	return 0;
}


BaseDecoder *NewBaseDecoder()
{
	MediaDecoder *ifce;
	IMGDec *wrap;
	SAFEALLOC(ifce, sizeof(MediaDecoder));
	wrap = malloc(sizeof(IMGDec));
	memset(wrap, 0, sizeof(IMGDec));
	ifce->privateStack = wrap;
	ifce->CanHandleStream = DEC_CanHandleStream;

	M4_REG_PLUG(ifce, M4MEDIADECODERINTERFACE, "GPAC Image Decoder", "gpac distribution", 0)

	/*other interfaces will be setup at run time*/
	return (BaseDecoder *)ifce;
}

void DeleteBaseDecoder(BaseDecoder *ifcd)
{
	IMGDec *wrap = (IMGDec *)ifcd->privateStack;
	switch (wrap->type) {
#ifdef M4_HAS_PNG
	case DEC_PNG:
		DeletePNGDec(ifcd);
		break;
#endif

#ifdef M4_HAS_JPEG
	case DEC_JPEG:
		DeleteJPEGDec(ifcd);
		break;
#endif
	case DEC_BMP:
		DeleteBMPDec(ifcd);
		break;
	default:
		break;
	}
	free(wrap);
	free(ifcd);
}

Bool QueryInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4MEDIADECODERINTERFACE:
		return 1;
	case M4STREAMINGCLIENT:
		return 1;
	default:
		return 0;
	}
}

void *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4MEDIADECODERINTERFACE:
		return NewBaseDecoder();
	case M4STREAMINGCLIENT:
		return NewLoaderInterface();
	default:
		return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4MEDIADECODERINTERFACE:
		DeleteBaseDecoder(ifcd);
		break;
	case M4STREAMINGCLIENT:
		DeleteLoaderInterface(ifce);
		break;
	}
}
