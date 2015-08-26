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

#include <winerror.h>

DataMap *FMO_New(const char *sPath, u8 mode)
{
	FileMappingDataMap *tmp;
	HANDLE fileH, fileMapH;
	DWORD err;
#ifdef _WIN32_WCE
	unsigned short sWPath[MAX_PATH];
#endif

	//only in read only
	if (mode != DM_MODE_R) return NULL;

	tmp = (FileMappingDataMap *) malloc(sizeof(FileMappingDataMap));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(FileMappingDataMap));
	tmp->type = DM_MAPPING;
	tmp->mode = mode;	
	tmp->name = _strdup(sPath);

	//
	//	Open the file 
	//
#ifdef _WIN32_WCE
	//convert to WIDE
	CE_CharToWide((char *)sPath, sWPath);

	fileH = CreateFileForMapping(sWPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
						(FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS), NULL );
#else
	fileH = CreateFile(sPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
						(FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS), NULL );
#endif

	
	if (fileH == INVALID_HANDLE_VALUE) {
		free(tmp->name);
		free(tmp);
		return NULL;
	}

	tmp->file_size = GetFileSize(fileH, NULL);
	if (tmp->file_size == 0xFFFFFFFF) {
		CloseHandle(fileH);
		free(tmp->name);
		free(tmp);
		return NULL;
	}

	//
	//	Create the mapping
	//
	fileMapH = CreateFileMapping(fileH, NULL, PAGE_READONLY, 0, 0, NULL);
	if (fileMapH == NULL) {
		CloseHandle(fileH);
		free(tmp->name);
		free(tmp);
		err = GetLastError();
		return NULL;
	}
		
	tmp->byte_map = MapViewOfFile(fileMapH, FILE_MAP_READ, 0, 0, 0);
	if (tmp->byte_map == NULL) {
		CloseHandle(fileMapH);
		CloseHandle(fileH);
		free(tmp->name);
		free(tmp);
		return NULL;
	}

	CloseHandle(fileH);
	CloseHandle(fileMapH);		

	//finaly open our bitstream (from buffer)
	tmp->bs = NewBitStream(tmp->byte_map, tmp->file_size, BS_READ);
	return (DataMap *)tmp;
}

void FMO_Delete(FileMappingDataMap *ptr)
{
	if (!ptr || (ptr->type != DM_MAPPING)) return;

	if (ptr->bs) DeleteBitStream(ptr->bs);
	if (ptr->byte_map) UnmapViewOfFile(ptr->byte_map);
	free(ptr->name);
	free(ptr);
}


u32 FMO_GetData(FileMappingDataMap *ptr, char *buffer, u32 bufferLength, u64 fileOffset)
{
	u32 size;

	//can we seek till that point ???
	if (fileOffset > ptr->file_size) return 0;
	size = (u32) fileOffset;

	//we do only read operations, so trivial
	memcpy(buffer, ptr->byte_map + fileOffset, bufferLength);
	return bufferLength;
}



