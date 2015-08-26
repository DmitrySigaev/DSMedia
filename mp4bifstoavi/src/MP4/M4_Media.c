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

M4Err Media_GetSampleDesc(MediaAtom *mdia, u32 SampleDescIndex, SampleEntryAtom **out_entry, u32 *dataRefIndex)
{
	SampleDescriptionAtom *stsd;
	SampleEntryAtom *entry = NULL;

	if (!mdia) return M4InvalidMP4File;

	stsd = mdia->information->sampleTable->SampleDescription;
	if (!stsd) return M4InvalidMP4File;
	if (!SampleDescIndex || (SampleDescIndex > ChainGetCount(stsd->atomList)) ) return M4BadParam;

	entry = (SampleEntryAtom*)ChainGetEntry(stsd->atomList, SampleDescIndex - 1);
	if (!entry) return M4InvalidMP4File;

	if (out_entry) *out_entry = entry;
	if (dataRefIndex) *dataRefIndex = entry->dataReferenceIndex;
	return M4OK;
}

M4Err Media_GetSampleDescIndex(MediaAtom *mdia, u64 DTS, u32 *sampleDescIndex)
{
	M4Err e;
	u32 sampleNumber, prevSampleNumber, num;
	u64 offset;
	u8 isEdited;
	if (sampleDescIndex == NULL) return M4BadParam;

	//find the sample for this time
	e = findEntryForTime(mdia->information->sampleTable, (u32) DTS, 0, &sampleNumber, &prevSampleNumber);
	if (e) return e;

	if (!sampleNumber && !prevSampleNumber) {
		//we have to assume the track was created to be used... If we have a sampleDesc, OK
		if (ChainGetCount(mdia->information->sampleTable->SampleDescription->atomList)) {
			(*sampleDescIndex) = 1;
			return M4OK;
		}		
		return M4BadParam;
	}
	return stbl_GetSampleInfos(mdia->information->sampleTable, ( sampleNumber ? sampleNumber : prevSampleNumber), &offset, &num, sampleDescIndex, &isEdited);
}

M4Err M4_GetAMRESD(SampleEntryAtom *entry, ESDescriptor **out_esd)
{
	BitStream *bs;
	(*out_esd) = OD_NewESDescriptor(2);
	(*out_esd)->decoderConfig->streamType = 0x05;
	/*this is a user-reserved used in gpac - we need a std OTI for AMR/AMRWB*/
	(*out_esd)->decoderConfig->objectTypeIndication = 0x80;
	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, FOUR_CHAR_INT('a', 'm', 'r', ' '), 32);
	BS_GetContent(bs, (unsigned char **) & (*out_esd)->decoderConfig->decoderSpecificInfo->data, & (*out_esd)->decoderConfig->decoderSpecificInfo->dataLength);
	DeleteBitStream(bs);
	return M4OK;
}

M4Err Media_GetESD(MediaAtom *mdia, u32 sampleDescIndex, ESDescriptor **out_esd, Bool true_desc_only)
{
	ESDescriptor *esd;
	MPEGSampleEntryAtom *entry = NULL;
	ESDAtom *ESDa;
	SampleDescriptionAtom *stsd = mdia->information->sampleTable->SampleDescription;
	
	if (!stsd || !stsd->atomList || !sampleDescIndex || (sampleDescIndex > ChainGetCount(stsd->atomList)) )
		return M4BadParam;

	esd = NULL;
	*out_esd = NULL;
	entry = (MPEGSampleEntryAtom*)ChainGetEntry(stsd->atomList, sampleDescIndex - 1);
	if (! entry) return M4InvalidMP4Media;

	ESDa = NULL;
	switch (entry->type) {
	case MPEGVisualSampleEntryAtomType:
		ESDa = ((MPEGVisualSampleEntryAtom*)entry)->esd;
		if (ESDa) esd = (ESDescriptor *) ESDa->desc;
		break;
	case MPEGAudioSampleEntryAtomType:
		ESDa = ((MPEGAudioSampleEntryAtom*)entry)->esd;
		if (ESDa) esd = (ESDescriptor *) ESDa->desc;
		break;
	case MPEGSampleEntryAtomType:
		ESDa = entry->esd;
		if (ESDa) esd = (ESDescriptor *) ESDa->desc;
		break;
	case AVCSampleEntryAtomType:
		esd = ((AVCSampleEntryAtom *) entry)->esd;
		break;
	case TextSampleEntryAtomType:
		if (!true_desc_only && mdia->mediaTrack->moov->mov->convert_streaming_text) {
			M4Err e = M4_GetStreamingTextESD(mdia, out_esd);
			if (e) return e;
			break;
		}
		else return M4InvalidMP4Media;

	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		if (!true_desc_only) {
			M4Err e = M4_GetAMRESD((SampleEntryAtom*)entry, out_esd);
			if (e) return e;
			break;
		} else return M4InvalidMP4Media;

	default: return M4InvalidMP4Media;
	}

	if (true_desc_only) {
		if (!esd) return M4InvalidMP4Media;
	} else {
		if (!esd && !*out_esd) 
			return M4InvalidMP4Media;
	
		/*need a copy*/
		if (!*out_esd) {
			OD_DuplicateDescriptor((Descriptor *)esd, (Descriptor **)out_esd);
		}
	}
	return M4OK;
}

Bool Media_IsSampleSyncShadow(ShadowSyncAtom *stsh, u32 sampleNumber)
{
	u32 i;
	stshEntry *ent;
	if (!stsh) return 0;
	for (i=0; i<ChainGetCount(stsh->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(stsh->entries, i);
		if ((u32) ent->syncSampleNumber == sampleNumber) return 1;
		else if ((u32) ent->syncSampleNumber > sampleNumber) return 0;
	}
	return 0;
}

M4Err Media_GetSample(MediaAtom *mdia, u32 sampleNumber, M4Sample **samp, u32 *sampleDescriptionIndex, Bool no_data, u64 *out_offset)
{
	M4Err e;
	u32 bytesRead;
	u32 dataRefIndex, chunkNumber;
	u64 offset, new_size;
	u8 isEdited;
	SampleEntryAtom *entry;

	(*sampleDescriptionIndex) = 0;
	
	if (!mdia || !mdia->information->sampleTable) return M4BadParam;

	//OK, here we go....
	if (sampleNumber > mdia->information->sampleTable->SampleSize->sampleCount) return M4BadParam;

	//get the DTS
	e = stbl_GetSampleDTS(mdia->information->sampleTable->TimeToSample, sampleNumber, &(*samp)->DTS);
	if (e) return e;
	//the CTS offset
	if (mdia->information->sampleTable->CompositionOffset) {
		e = stbl_GetSampleCTS(mdia->information->sampleTable->CompositionOffset , sampleNumber, &(*samp)->CTS_Offset);
		if (e) return e;
	} else {
		(*samp)->CTS_Offset = 0;
	}
	//the size
	e = stbl_GetSampleSize(mdia->information->sampleTable->SampleSize, sampleNumber, &(*samp)->dataLength);
	if (e) return e;
	//the RAP
	if (mdia->information->sampleTable->SyncSample) {
		e = stbl_GetSampleRAP(mdia->information->sampleTable->SyncSample, sampleNumber, &(*samp)->IsRAP, NULL, NULL);
		if (e) return e;
	} else {
		//if no SyncSample, all samples are sync (cf spec)
		(*samp)->IsRAP = 1;
	}
	/*get sync shadow*/
	if (Media_IsSampleSyncShadow(mdia->information->sampleTable->ShadowSync, sampleNumber)) (*samp)->IsRAP = 2;

	//the data info
	e = stbl_GetSampleInfos(mdia->information->sampleTable, sampleNumber, &offset, &chunkNumber, sampleDescriptionIndex, &isEdited);
	if (e) return e;

	//then get the DataRef
	e = Media_GetSampleDesc(mdia, *sampleDescriptionIndex, &entry, &dataRefIndex);
	if (e) return e;

	// Open the data handler - check our mode, don't reopen in read only if this is 
	//the same entry. In other modes we have no choice because the main data map is 
	//divided into the original and the edition files
	if (mdia->mediaTrack->moov->mov->openMode == M4_OPEN_READ) {	
		//same as last call in read mode
		if (!mdia->information->dataHandler || (mdia->information->dataEntryIndex != dataRefIndex)) {
			e = DataMap_Open(mdia, dataRefIndex, isEdited);
			if (e) return e;
		}
	} else {
		e = DataMap_Open(mdia, dataRefIndex, isEdited);
		if (e) return e;
	}

	if (out_offset) *out_offset = offset;
	if (no_data) return M4OK;

	/*and finally get the data, include padding if needed*/
 	(*samp)->data = (char *) malloc(sizeof(char) * ( (*samp)->dataLength + mdia->mediaTrack->padding_bytes) );
	if (mdia->mediaTrack->padding_bytes)
		memset((*samp)->data + (*samp)->dataLength, 0, sizeof(char) * mdia->mediaTrack->padding_bytes);

	//check if we can get the sample (make sure we have enougth data...)
	new_size = BS_GetSize(mdia->information->dataHandler->bs);
	if (offset + (*samp)->dataLength > new_size) {
		//always refresh the size to avoid wrong info on http/ftp 
		new_size = BS_GetRefreshedSize(mdia->information->dataHandler->bs);
		if (offset + (*samp)->dataLength > new_size) {
			mdia->BytesMissing = offset + (*samp)->dataLength - new_size;
			return M4UncompleteFile;
		}	
	}

	bytesRead = DataMap_GetData(mdia->information->dataHandler, (*samp)->data, (*samp)->dataLength, offset);
	//if bytesRead != sampleSize, we have an IO err
	if (bytesRead < (*samp)->dataLength) {
		return M4IOErr;
	}
	mdia->BytesMissing = 0;
	//finally rewrite the sample if this is an OD Access Unit
	if (mdia->handler->handlerType == M4_ODMediaType) {
		e = Media_RewriteODFrame(mdia, *samp);
		if (e) return e;
	}
	else if (mdia->mediaTrack->moov->mov->convert_streaming_text 
		&& (mdia->handler->handlerType == M4_TimedTextMediaType) ) {
		u32 dur;
		if (sampleNumber == mdia->information->sampleTable->SampleSize->sampleCount) {
			dur = (u32) mdia->mediaHeader->duration - (*samp)->DTS;
		} else {
			stbl_GetSampleDTS(mdia->information->sampleTable->TimeToSample, sampleNumber+1, &dur);
			dur -= (*samp)->DTS;
		}
		e = M4_RewriteTextSample(*samp, *sampleDescriptionIndex, dur);
		if (e) return e;
	}
	return M4OK;
}



M4Err Media_CheckDataEntry(MediaAtom *mdia, u32 dataEntryIndex)
{

	DataEntryURLAtom *entry;
	DataMap *map;
	M4Err e;
	if (!mdia || !dataEntryIndex || dataEntryIndex > ChainGetCount(mdia->information->dataInformation->dref->atomList)) return M4BadParam;

	entry = (DataEntryURLAtom*)ChainGetEntry(mdia->information->dataInformation->dref->atomList, dataEntryIndex - 1);
	if (!entry) return M4InvalidMP4File;
	if (entry->flags == 1) return M4OK;
	
	//ok, not self contained, let's go for it...
	//we don't know what's a URN yet
	if (entry->type == DataEntryURNAtomType) return M4NotSupported;
	if (mdia->mediaTrack->moov->mov->openMode == M4_OPEN_WRITE) {
		e = DataMap_New(entry->location, NULL, DM_MODE_R, &map);
	} else {
		e = DataMap_New(entry->location, mdia->mediaTrack->moov->mov->fileName, DM_MODE_R, &map);
	}
	if (e) return e;
	DataMap_Delete(map);
	return M4OK;
}


Bool Media_IsSelfContained(MediaAtom *mdia, u32 StreamDescIndex)
{
	u32 drefIndex;
	FullAtom *a;
	SampleEntryAtom *se = NULL;

	Media_GetSampleDesc(mdia, StreamDescIndex, &se, &drefIndex);
	if (!drefIndex) return 0;
	a = (FullAtom*)ChainGetEntry(mdia->information->dataInformation->dref->atomList, drefIndex - 1);
	if (a->flags & 1) return 1;
	/*QT specific*/
	if (a->type == FOUR_CHAR_INT('a', 'l', 'i', 's')) return 1;
	return 0;
}



//look for a sync sample from a given point in media time
M4Err Media_FindSyncSample(SampleTableAtom *stbl, u32 searchFromSample, u32 *sampleNumber, u8 mode)
{
	u8 isRAP;
	u32 next, prev;
	if (!stbl || !stbl->SyncSample) return M4BadParam;

	//set to current sample if we don't find a RAP		
	*sampleNumber = searchFromSample;

	//this is not the exact sample, but the prev move to next sample if enough samples....
	if ( (mode == M4_SearchSyncForward) && (searchFromSample == stbl->SampleSize->sampleCount) ) {
		return M4OK;
	}
	if ( (mode == M4_SearchSyncBackward) && !searchFromSample) {
		*sampleNumber = 1;
		return M4OK;
	}
	//get the entry
	stbl_GetSampleRAP(stbl->SyncSample, searchFromSample, &isRAP, &prev, &next);
	if (isRAP) {
		(*sampleNumber) = searchFromSample;
		return M4OK;
	}
	//nothing yet, go for next time...
	if (mode == M4_SearchSyncForward) {
		if (next) *sampleNumber = next;
	} else {
		if (prev) *sampleNumber = prev;
	}
	return M4OK;
}

//create a DataReference if not existing (only for WRITE-edit mode)
M4Err Media_FindDataRef(DataReferenceAtom *dref, char *URLname, char *URNname, u32 *dataRefIndex)
{
	u32 i;
	DataEntryURLAtom *entry;

	if (!dref) return M4BadParam;
	*dataRefIndex = 0;
	for (i = 0; i < ChainGetCount(dref->atomList); i++) {
		entry = (DataEntryURLAtom*)ChainGetEntry(dref->atomList, i);
		
		if (entry->type == DataEntryURLAtomType) {
			//self-contained case
			if (entry->flags == 1) {
				//if nothing specified, get the dataRef
				if (!URLname && !URNname) {
					*dataRefIndex = i+1;
					return M4OK;
				}
			} else {
				//OK, check if we have URL
				if (URLname && !strcmp(URLname, entry->location)) {
					*dataRefIndex = i+1;
					return M4OK;
				}
			}
		} else {
			//this is a URN one, only check the URN name (URL optional)
			if (URNname && !strcmp(URNname, ((DataEntryURNAtom *)entry)->nameURN)) {
				*dataRefIndex = i+1;
				return M4OK;
			}
		}
	}
	return M4OK;
}

//Get the total media duration based on the TimeToSample table
M4Err Media_SetDuration(TrackAtom *trak)
{
	ESDescriptor *esd;
	u32 DTS, DTSprev;
	u32 nbSamp = trak->Media->information->sampleTable->SampleSize->sampleCount;

	//we need to check how many samples we have. 
	// == 1 -> last sample duration == default duration
	// > 1 -> last sample duration == prev sample duration
	switch (nbSamp) {
	case 0:
		trak->Media->mediaHeader->duration = 0;
		//we have an URL stream, set duration to its max
		if (!Track_IsMPEG4Stream(trak->Media->handler->handlerType)) {
			Media_GetESD(trak->Media, 1, &esd, 1);
			if (esd && esd->URLString) trak->Media->mediaHeader->duration = -1;
		}
		return M4OK;

	case 1:
		trak->Media->mediaHeader->duration = trak->Media->mediaHeader->timeScale;
		return M4OK;

	default:
		//we assume a constant frame rate for the media and assume the last sample
		//will be hold the same time as the prev one
		stbl_GetSampleDTS(trak->Media->information->sampleTable->TimeToSample, nbSamp, &DTS);
		stbl_GetSampleDTS(trak->Media->information->sampleTable->TimeToSample, nbSamp-1, &DTSprev);
		trak->Media->mediaHeader->duration = DTS + (DTS - DTSprev);
		return M4OK;
	}
}




#ifndef M4_READ_ONLY

	
M4Err Media_CreateDataRef(DataReferenceAtom *dref, char *URLname, char *URNname, u32 *dataRefIndex)
{	
	M4Err e;
	DataEntryURLAtom *entry;

	M4Err dref_AddDataEntry(DataReferenceAtom *ptr, Atom *entry);

	if (!URLname && !URNname) {
		//THIS IS SELF CONTAIN, create a regular entry if needed
		entry = (DataEntryURLAtom *) CreateAtom(DataEntryURLAtomType);
		entry->location = NULL;
		entry->flags = 0;
		entry->flags |= 1;
		e = dref_AddDataEntry(dref, (Atom *)entry);
		if (e) return e;
		*dataRefIndex = ChainGetCount(dref->atomList);
		return M4OK;
	} else if (!URNname && URLname) {
		//THIS IS URL
		entry = (DataEntryURLAtom *) CreateAtom(DataEntryURLAtomType);
		entry->flags = 0;
		entry->location = (char*)malloc(strlen(URLname)+1);
		if (! entry->location) {
			DelAtom((Atom *)entry);
			return M4OutOfMem;
		}
		strcpy(entry->location, URLname);
		e = dref_AddDataEntry(dref, (Atom *)entry);
		if (e) return e;
		*dataRefIndex = ChainGetCount(dref->atomList);
		return M4OK;
	} else {
		//THIS IS URN
		entry = (DataEntryURLAtom *) CreateAtom(DataEntryURNAtomType);
		((DataEntryURNAtom *)entry)->flags = 0;
		((DataEntryURNAtom *)entry)->nameURN = (char*)malloc(strlen(URNname)+1);
		if (! ((DataEntryURNAtom *)entry)->nameURN) {
			DelAtom((Atom *)entry);
			return M4OutOfMem;
		}
		strcpy(((DataEntryURNAtom *)entry)->nameURN, URNname);
		//check for URL
		if (URLname) {
			((DataEntryURNAtom *)entry)->location = (char*)malloc(strlen(URLname)+1);
			if (! ((DataEntryURNAtom *)entry)->location) {
				DelAtom((Atom *)entry);
				return M4OutOfMem;
			}
			strcpy(((DataEntryURNAtom *)entry)->location, URLname);
		}
		e = dref_AddDataEntry(dref, (Atom *)entry);
		if (e) return e;
		*dataRefIndex = ChainGetCount(dref->atomList);
		return M4OK;
	}
	return M4OK;
}


M4Err Media_AddSample(MediaAtom *mdia, u64 data_offset, M4Sample *sample, u32 StreamDescIndex, u32 syncShadowNumber)
{
	M4Err e;
	SampleTableAtom *stbl;
	u32 sampleNumber, i;
	if (!mdia || !sample) return M4BadParam;

	stbl = mdia->information->sampleTable;

	//get a valid sampleNumber for this new guy
	e = stbl_AddDTS(stbl, sample->DTS, &sampleNumber, mdia->mediaHeader->timeScale);
	if (e) return e;

	//add size
	e = stbl_AddSize(stbl->SampleSize, sampleNumber, sample->dataLength);
	if (e) return e;

	//adds CTS offset
	if (sample->CTS_Offset) {
		//if we don't have a CTS table, add it...
		if (!stbl->CompositionOffset) stbl->CompositionOffset = (CompositionOffsetAtom *) CreateAtom(CompositionOffsetAtomType);
		//then add our CTS (the prev samples with no CTS offset will be automatically added...
		e = stbl_AddCTS(stbl, sampleNumber, sample->CTS_Offset);
		if (e) return e;
	} else if (stbl->CompositionOffset) {
		e = stbl_AddCTS(stbl, sampleNumber, sample->CTS_Offset);
		if (e) return e;
	}

	//The first non sync sample we see must create a syncTable
	if (sample->IsRAP) {
		//insert it only if we have a sync table
		if (stbl->SyncSample) {
			e = stbl_AddRAP(stbl->SyncSample, sampleNumber);
			if (e) return e;
		}
	} else {
		//non-sync sample. Create a SyncSample table if needed
		if (!stbl->SyncSample) {
			stbl->SyncSample = (SyncSampleAtom *) CreateAtom(SyncSampleAtomType);
			//all the prev samples are sync
			for (i=0; i<stbl->SampleSize->sampleCount; i++) {
				if (i+1 != sampleNumber) {
					e = stbl_AddRAP(stbl->SyncSample, i+1);
					if (e) return e;
				}
			}
		}
	}

	//and update the chunks
	e = stbl_AddChunkOffset(mdia, sampleNumber, StreamDescIndex, data_offset);
	if (e) return e;

	if (!syncShadowNumber) return M4OK;
	if (!stbl->ShadowSync) stbl->ShadowSync = (ShadowSyncAtom *) CreateAtom(ShadowSyncAtomType);
	return stbl_AddShadow(mdia->information->sampleTable->ShadowSync, sampleNumber, syncShadowNumber);
}


M4Err UpdateSample(MediaAtom *mdia, u32 sampleNumber, u32 size, u32 CTS, u64 offset, u8 isRap)
{
	u32 i;
	SampleTableAtom *stbl = mdia->information->sampleTable;

	//set size, offset, RAP, CTS ...
	stbl_SetSampleSize(stbl->SampleSize, sampleNumber, size);
	stbl_SetChunkOffset(mdia, sampleNumber, offset);

	//do we have a CTS?
	if (stbl->CompositionOffset) {
		stbl_SetSampleCTS(stbl, sampleNumber, CTS);
	} else {
		//do we need one ??
		if (CTS) {
			stbl->CompositionOffset = (CompositionOffsetAtom *) CreateAtom(CompositionOffsetAtomType);
			stbl_AddCTS(stbl, sampleNumber, CTS);
		}
	}
	//do we have a sync ???
	if (stbl->SyncSample) {
		stbl_SetSampleRAP(stbl->SyncSample, sampleNumber, isRap);
	} else {
		//do we need one
		if (! isRap) {
			stbl->SyncSample = (SyncSampleAtom *) CreateAtom(SyncSampleAtomType);
			//what a pain: all the sample we had have to be sync ...
			for (i=0; i<stbl->SampleSize->sampleCount; i++) {
				if (i+1 != sampleNumber) stbl_AddRAP(stbl->SyncSample, i+1);
			}
		}
	}
	return M4OK;
}

M4Err Media_UpdateSample(MediaAtom *mdia, u32 sampleNumber, M4Sample *sample)
{
	M4Err e;
	u32 drefIndex, chunkNum, descIndex, DTS;
	u64 newOffset;
	u8 isEdited;
	DataEntryURLAtom *Dentry;
	SampleTableAtom *stbl;

	M4Err stbl_AddAtom(SampleTableAtom *ptr, Atom *a);

	if (!mdia || !sample || !sampleNumber || !mdia->mediaTrack->moov->mov->editFileMap)
		return M4BadParam;
	
	stbl = mdia->information->sampleTable;

	//check we have the sampe dts
	e = stbl_GetSampleDTS(stbl->TimeToSample, sampleNumber, &DTS);
	if (e) return e;
	if (DTS != sample->DTS) return M4BadParam;

	//get our infos
	stbl_GetSampleInfos(stbl, sampleNumber, &newOffset, &chunkNum, &descIndex, &isEdited);

	//then check the data ref
	e = Media_GetSampleDesc(mdia, descIndex, NULL, &drefIndex);
	if (e) return e;
	Dentry = (DataEntryURLAtom*)ChainGetEntry(mdia->information->dataInformation->dref->atomList, drefIndex - 1);
	if (!Dentry) return M4InvalidMP4File;

	if (Dentry->flags != 1) return M4BadParam;

	//MEDIA DATA EDIT: write this new sample to the edit temp file
	newOffset = DataMap_GetTotalOffset(mdia->mediaTrack->moov->mov->editFileMap);
	e = DataMap_AddData(mdia->mediaTrack->moov->mov->editFileMap, sample->data, sample->dataLength);
	if (e) return e;

	return UpdateSample(mdia, sampleNumber, sample->dataLength, sample->CTS_Offset, newOffset, sample->IsRAP);
}

M4Err Media_UpdateSampleReference(MediaAtom *mdia, u32 sampleNumber, M4Sample *sample, u64 data_offset)
{
	M4Err e;
	u32 drefIndex, DTS, chunkNum, descIndex;
	u64 off;
	u8 isEdited;
	DataEntryURLAtom *Dentry;
	SampleTableAtom *stbl;
	M4Err stbl_AddAtom(SampleTableAtom *ptr, Atom *a);

	if (!mdia) return M4BadParam;
	stbl = mdia->information->sampleTable;

	//check we have the sampe dts
	e = stbl_GetSampleDTS(stbl->TimeToSample, sampleNumber, &DTS);
	if (e) return e;
	if (DTS != sample->DTS) return M4BadParam;

	//get our infos
	stbl_GetSampleInfos(stbl, sampleNumber, &off, &chunkNum, &descIndex, &isEdited);

	//then check the data ref
	e = Media_GetSampleDesc(mdia, descIndex, NULL, &drefIndex);
	if (e) return e;
	Dentry = (DataEntryURLAtom*)ChainGetEntry(mdia->information->dataInformation->dref->atomList, drefIndex - 1);
	if (!Dentry) return M4InvalidMP4File;

	//we only modify self-contained data
	if (Dentry->flags == 1) return M4InvalidMP4Mode;

	//and we don't modify the media data
	return UpdateSample(mdia, sampleNumber, sample->dataLength, sample->CTS_Offset, data_offset, sample->IsRAP);
}


#endif	//M4_READ_ONLY
