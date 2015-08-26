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

//used by the movie destructor to close the movie file and the edit file
//used by DataMap to close external references.
void DataMap_Delete(DataMap *ptr)
{
	if (!ptr) return;

	//then delete the structure itself....
	switch (ptr->type) {
	//file-based
	case DM_FILE:
		FDM_Delete((FileDataMap *)ptr);
		break;

	case DM_MAPPING:
		FMO_Delete((FileMappingDataMap *)ptr);
		break;

	//not implemented
	default:
		break;
	}
}

//Close a data entry
void DataMap_Close(MediaInformationAtom *minf)
{
	DataEntryAtom *ent;
	if (!minf || !minf->dataHandler) return;

	//get the entry atom 
	ent = (DataEntryAtom*)ChainGetEntry(minf->dataInformation->dref->atomList, minf->dataEntryIndex - 1);

	//if ent NULL, the data entry was not used (should never happen)
	if (ent == NULL) return;

	//self contained, do nothing
	switch (ent->type) {
	case DataEntryURLAtomType:
	case DataEntryURNAtomType:
		if (ent->flags == 1) return;
		break;
	default:
		return;
	}

	//finally close it
	DataMap_Delete(minf->dataHandler);
	minf->dataHandler = NULL;
}

/*cf below, we disable filedatamap since it tricks mem usage on w32*/
#if 0
static Bool MP4_IsLargeFile(char *path)
{
#ifndef _WIN32_WCE
	FILE *stream;
	s64 size;
	stream = fopen(path, "rb");
	if (!stream) return 0;
	f64_seek(stream, 0, SEEK_END);
	size = f64_tell(stream);
	fclose(stream);
	if (size == -1L) return 0;
	if (size > 0xFFFFFFFF) return 1;
#endif
	return 0;
}
#endif


//Special constructor, we need some error feedback...

M4Err DataMap_New(const char *location, const char *parentPath, u8 mode, DataMap **outDataMap)
{
	u32 prot_type;
	char *sPath;
	*outDataMap = NULL;

	//if nothing specified, this is a MEMORY data map
	if (!location) {
		//not supported yet
		return M4NotSupported;
	}
	//we need a temp file ...
	if (!strcmp(location, "mp4_tmp_edit")) {
#ifndef M4_READ_ONLY
		*outDataMap = FDM_NewTemp(parentPath);
		if (! (*outDataMap)) return M4IOErr;
		return M4OK;
#else
		return M4NotSupported;
#endif
	}

	prot_type = URL_GetProtocolType(location);

	if (mode == DM_MODE_E) {
		//we need a local file for edition!!!
		if (prot_type == URL_TYPE_ANY) return M4InvalidMP4Mode;
		//OK, switch back to READ mode
		mode = DM_MODE_R;
	}

	//TEMP: however, only support for file right now (we'd have to add some callback functions at some point)
	if (prot_type == URL_TYPE_ANY) return M4NotSupported;

	sPath = URL_GetAbsoluteFilePath(location, parentPath);
	if (sPath == NULL) return M4InvalidURL;

	if (mode == DM_MODE_RO) {
		mode = DM_MODE_R;
		/*It seems win32 file mapping is reported in prog mem usage -> large increases of occupancy. Should not be a pb 
		but unless you want mapping, only regular IO will be used...*/
#if 0
		if (MP4_IsLargeFile(sPath)) {
			*outDataMap = FDM_New(sPath, mode);
		} else {
			*outDataMap = FMO_New(sPath, mode);
		}
#else
		*outDataMap = FDM_New(sPath, mode);
#endif
	} else {
		*outDataMap = FDM_New(sPath, mode);
	}

	free(sPath);
	if (! (*outDataMap)) return M4URLNotFound;
	return M4OK;
}

//Open a data entry of a track
//Edit is used to switch between original and edition file
M4Err DataMap_Open(MediaAtom *mdia, u32 dataRefIndex, u8 Edit)
{
	DataEntryAtom *ent;
	MediaInformationAtom *minf;
	u32 SelfCont;
	M4Err e = M4OK;
	if ((mdia == NULL) || (! mdia->information) || !dataRefIndex)
		return M4InvalidMP4Media;

	minf = mdia->information;

	if (dataRefIndex > ChainGetCount(minf->dataInformation->dref->atomList))
		return M4BadParam;

	ent = (DataEntryAtom*)ChainGetEntry(minf->dataInformation->dref->atomList, dataRefIndex - 1);
	if (ent == NULL) return M4InvalidMP4Media;

	//if the current dataEntry is the desired one, and not self contained, return
	if ((minf->dataEntryIndex == dataRefIndex) && (ent->flags != 1)) {
		return M4OK;
	}

	//we need to open a new one
	//first close the existing one
	if (minf->dataHandler) DataMap_Close(minf);

	//check the entry atom type...
	SelfCont = 0;
	switch (ent->type) {
	case DataEntryURLAtomType:
	case DataEntryURNAtomType:
		if (ent->flags == 1) SelfCont = 1;
		break;
	default:
		SelfCont = 1;
		break;
	}
	//if self-contained, assign the input file
	if (SelfCont) {
		//if no edit, open the input file
		if (!Edit) {
			if (mdia->mediaTrack->moov->mov->movieFileMap == NULL) return M4InvalidMP4File;
			minf->dataHandler = mdia->mediaTrack->moov->mov->movieFileMap;
		} else {
#ifndef M4_READ_ONLY
			if (mdia->mediaTrack->moov->mov->editFileMap == NULL) return M4InvalidMP4File;
			minf->dataHandler = mdia->mediaTrack->moov->mov->editFileMap;
#else
			//this should never be the case in an read-only MP4 file
			return M4BadParam;
#endif		
		}
	//else this is a URL atom (read mode only)
	} else {
		e = DataMap_New(ent->location, mdia->mediaTrack->moov->mov->fileName, DM_MODE_R, & mdia->information->dataHandler);
		if (e) return (e==M4URLNotFound) ? M4DataRefNotFound : e;
	}
	//OK, set the data entry index
	minf->dataEntryIndex = dataRefIndex;
	return M4OK;
}

//return the NB of bytes actually read (used for HTTP, ...) in case file is uncomplete
u32 DataMap_GetData(DataMap *map, char *buffer, u32 bufferLength, u64 Offset)
{
	if (!map || !buffer) return 0;

	switch (map->type) {
	case DM_FILE:
		return FDM_GetData((FileDataMap *)map, buffer, bufferLength, Offset);

	case DM_MAPPING:
		return FMO_GetData((FileMappingDataMap *)map, buffer, bufferLength, Offset);

	default:
		return 0;
	}
}


#ifndef M4_READ_ONLY

u64 DataMap_GetTotalOffset(DataMap *map)
{
	if (!map) return 0;

	switch (map->type) {
	case DM_FILE:
		return FDM_GetTotalOffset((FileDataMap *)map);

	default:
		return 0;
	}
}


M4Err DataMap_AddData(DataMap *ptr, char *data, u32 dataSize)
{
	if (!ptr || !data|| !dataSize) return M4BadParam;

	switch (ptr->type) {
	case DM_FILE:
		return FDM_AddData((FileDataMap *)ptr, data, dataSize);
	default:
		return M4NotSupported;
	}
}

#endif

