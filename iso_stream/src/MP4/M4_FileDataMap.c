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

#include <intern/m4_isomedia_dev.h>

#ifndef M4_READ_ONLY
DataMap *FDM_NewTemp(const char *sPath)
{
	FileDataMap *tmp = (FileDataMap *) malloc(sizeof(FileDataMap));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(FileDataMap));
	tmp->type = DM_FILE;
	tmp->mode = DM_MODE_W;

	if (!sPath) {
		tmp->stream = M4NewTMPFile();
	} else {
		char szPath[M4_MAX_PATH];
		if ((sPath[strlen(sPath)-1] != '\\') && (sPath[strlen(sPath)-1] != '/')) {
			sprintf(szPath, "%s%c%d_isotmp", sPath, M4_PATH_SEPARATOR, (u32) tmp);
		} else {
			sprintf(szPath, "%s%d_isotmp", sPath, (u32) tmp);
		}
		tmp->stream = fopen(szPath, "w+b");
		tmp->temp_file = strdup(szPath);
	}
	if (!tmp->stream) {
		free(tmp);
		return NULL;
	}
	tmp->bs = NewBitStreamFromFile(tmp->stream, BS_FILE_READ_WRITE);
	if (!tmp->bs) {
		fclose(tmp->stream);
		free(tmp);
		return NULL;
	}
	return (DataMap *)tmp;
}

#endif

DataMap *FDM_New(const char *sPath, u8 mode)
{
	u8 bs_mode;

	FileDataMap *tmp = (FileDataMap *) malloc(sizeof(FileDataMap));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(FileDataMap));
	tmp->type = DM_FILE;
	tmp->mode = mode;

#ifndef M4_READ_ONLY
	//open a temp file
	if (!strcmp(sPath, "mp4_tmp_edit")) {
		//create  a temp file (that only occurs in EDIT/WRITE mode)
		tmp->stream = M4NewTMPFile();
		bs_mode = BS_FILE_READ_WRITE;
	}
#endif

	switch (mode) {
	case DM_MODE_R:
		if (!tmp->stream) tmp->stream = fopen(sPath, "rb");
		bs_mode = BS_FILE_READ;
		break;
	///we open the file in READ/WRITE mode, in case 
	case DM_MODE_W:
		if (!tmp->stream) tmp->stream = fopen(sPath, "w+b");
		if (!tmp->stream) tmp->stream = fopen(sPath, "wb");
		bs_mode = BS_FILE_READ_WRITE;
		break;
	default:
		free(tmp);
		return NULL;
	}
	if (!tmp->stream) {
		free(tmp);
		return NULL;
	}
	tmp->bs = NewBitStreamFromFile(tmp->stream, bs_mode);
	if (!tmp->bs) {
		fclose(tmp->stream);
		free(tmp);
		return NULL;
	}
	return (DataMap *)tmp;
}

void FDM_Delete(FileDataMap *ptr)
{
	if (!ptr || (ptr->type != DM_FILE)) return;
	if (ptr->bs) DeleteBitStream(ptr->bs);
	if (ptr->stream) fclose(ptr->stream);

#ifndef M4_READ_ONLY
	if (ptr->temp_file) {
		M4_DeleteFile(ptr->temp_file);
		free(ptr->temp_file);
	}
#endif
	free(ptr);
}



u32 FDM_GetData(FileDataMap *ptr, char *buffer, u32 bufferLength, u64 fileOffset)
{
	u32 bytesRead;

	//can we seek till that point ???
	if (fileOffset > BS_GetSize(ptr->bs)) return 0;

	//ouch, we are not at the previous location, do a seek
	if (ptr->curPos != fileOffset) {
		if (BS_Seek(ptr->bs, fileOffset) != M4OK) return 0;
		ptr->curPos = fileOffset;
	}
	//read our data.
	bytesRead = BS_ReadData(ptr->bs, (unsigned char*)buffer, bufferLength);
	//update our cache
	if (bytesRead == bufferLength) {
		ptr->curPos += bytesRead;
	} else {
		//rewind to original (if seek fails, return 0 cause this means:
		//1- no support for seek on the platform
		//2- corrupted file for the OS
		fflush(ptr->stream);
		BS_Seek(ptr->bs, ptr->curPos);
	}
	ptr->last_acces_was_read = 1;
	return bytesRead;
}



#ifndef M4_READ_ONLY


u64 FDM_GetTotalOffset(FileDataMap *ptr)
{
	if (!ptr) return 0;
	//the pos is not always at the end
	//this function is called to set up the chunks
	//so we need the next WRITE offset
	return BS_GetSize(ptr->bs);
}



M4Err FDM_AddData(FileDataMap *ptr, char *data, u32 dataSize)
{
	u32 ret;
	u64 orig;
	if (ptr->mode == DM_MODE_R) return M4BadParam;

	orig = BS_GetSize(ptr->bs);

	/*last access was read, seek to end of file*/
	if (ptr->last_acces_was_read) {
		BS_Seek(ptr->bs, orig);
		ptr->last_acces_was_read = 0;
	}
	//OK, write our stuff to the datamap...
	//we don't use bs here cause we want to know more about what has been written
	ret = BS_WriteData(ptr->bs, (unsigned char*)data, dataSize);
	if (ret != dataSize) {
		ptr->curPos = orig;
		BS_Seek(ptr->bs, orig);
		return M4IOErr;
	}
	ptr->curPos = BS_GetPosition(ptr->bs);
	//flush the stream !!
	fflush(ptr->stream);
	return M4OK;
}

#endif	//M4_READ_ONLY

