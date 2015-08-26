/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / FFMPEG plugin
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

#include "ffmpeg_in.h"



Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4MEDIADECODERINTERFACE) return 1;
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4MEDIADECODERINTERFACE) return FFDEC_Load();
	if (InterfaceType == M4STREAMINGCLIENT) return New_FFMPEG_Demux();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4MEDIADECODERINTERFACE:
		FFDEC_Delete(ptr);
		break;
	case M4STREAMINGCLIENT:
		Delete_FFMPEG_Demux(ptr);
		break;
	}
}
