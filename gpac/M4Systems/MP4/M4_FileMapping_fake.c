/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
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

#include <gpac/intern/m4_isomedia_dev.h>

DataMap *FMO_New(const char *sPath, u8 mode)
{
	return FDM_New(sPath, mode);
}

void FMO_Delete(FileMappingDataMap *ptr)
{
	FDM_Delete((FileDataMap *)ptr);
}


u32 FMO_GetData(FileMappingDataMap *ptr, char *buffer, u32 bufferLength, u64 fileOffset)
{
	return FDM_GetData((FileDataMap *)ptr, buffer, bufferLength, fileOffset);
}



