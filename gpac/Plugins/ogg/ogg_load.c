/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / XIPH.org plugin
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

#include "ogg_in.h"


static Bool OGG_CanHandleStream(BaseDecoder *dec, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	/*video decs*/
	if (StreamType == M4ST_VISUAL) {
		switch (ObjectType) {
#ifdef M4_HAS_THEORA
		case GPAC_OGG_MEDIA_OTI: 
			if (decSpecInfo && (decSpecInfoSize>=7)  && !strncmp((char *) &decSpecInfo[1], "theora", 6)) {
				return NewTheoraDecoder(dec);
			}
			return 0;
		case 0xDF:
			return NewTheoraDecoder(dec);
		case 0: return 1;/*query for types*/
#endif
		default: return 0;
		}
	}
	/*audio decs*/	
	if (StreamType == M4ST_AUDIO) {
		switch (ObjectType) {
#ifdef M4_HAS_VORBIS
		case GPAC_OGG_MEDIA_OTI: 
			if (decSpecInfo && (decSpecInfoSize>=7)  && !strncmp((char *) &decSpecInfo[1], "vorbis", 6)) {
				return NewVorbisDecoder(dec);
			}
			return 0;
		case 0xDE: 
			return NewVorbisDecoder(dec);
		case 0: return 1;
#endif
		default: return 0;
		}
	}
	return 0;
}


BaseDecoder *OGG_LoadDecoder()
{
	MediaDecoder *ifce;
	OGGWraper *wrap;
	SAFEALLOC(ifce, sizeof(MediaDecoder));
	SAFEALLOC(wrap, sizeof(OGGWraper));
	ifce->privateStack = wrap;
	ifce->CanHandleStream = OGG_CanHandleStream;
	M4_REG_PLUG(ifce, M4MEDIADECODERINTERFACE, "GPAC XIPH.org package", "gpac distribution", 0)

	/*other interfaces will be setup at run time*/
	return (BaseDecoder *)ifce;
}

void DeleteOGGDecoder(BaseDecoder *ifcd)
{
	OGGWraper *wrap = (OGGWraper *)ifcd->privateStack;
	switch (wrap->type) {
#ifdef M4_HAS_VORBIS
	case OGG_VORBIS: DeleteVorbisDecoder(ifcd); break;
#endif
#ifdef M4_HAS_THEORA
	case OGG_THEORA: DeleteTheoraDecoder(ifcd); break;
#endif
	default:
		break;
	}
	free(wrap);
	free(ifcd);
}


Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
	if (InterfaceType == M4MEDIADECODERINTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return OGG_LoadDemux();
	if (InterfaceType == M4MEDIADECODERINTERFACE) return OGG_LoadDecoder();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4STREAMINGCLIENT:
		OGG_DeleteDemux(ptr);
		break;
	case M4MEDIADECODERINTERFACE:
		DeleteOGGDecoder(ifce);
		break;
	}
}
