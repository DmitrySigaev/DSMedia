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

//		NOTE: MPEG-4 OD Framework cannot be used with 3GPP files. Stream Descriptions
//	are not ESDescriptor, just generic config options as specified in this file

#include <gpac/intern/m4_isomedia_dev.h>

LPAMRCONFIGURATION M4_AMR_GetStreamConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	AMRConfiguration *config;
	TrackAtom *trak;
	AMRSampleEntryAtom *entry;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !StreamDescriptionIndex) return NULL;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex-1);
	if (!entry) return NULL;
	switch (entry->type) {
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		break;
	default:
		return NULL;
	}
	if (!entry->amr_info) return NULL;

	config = malloc(sizeof(AMRConfiguration));
	memset(config, 0, sizeof(AMRConfiguration));

	if (entry->type == WB_AMRSampleEntryAtomType) config->WideBandAMR = 1;
	config->decoder_version = entry->amr_info->decoder_version;
	config->vendor = entry->amr_info->vendor;
	config->frames_per_sample = entry->amr_info->frames_per_sample;
	config->mode_change_period = entry->amr_info->mode_change_period;
	config->mode_set = entry->amr_info->mode_set;
	return config;
}




LPH263CONFIGURATION M4_H263_GetStreamConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	H263Configuration *config;
	TrackAtom *trak;
	H263SampleEntryAtom *entry;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !StreamDescriptionIndex) return NULL;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex-1);
	if (!entry) return NULL;
	if (entry->type != H263SampleEntryAtomType) return NULL;
	if (!entry->h263_config) return NULL;

	config = malloc(sizeof(H263Configuration));
	memset(config, 0, sizeof(H263Configuration));

	config->decoder_version = entry->h263_config->decoder_version;
	config->vendor = entry->h263_config->vendor;
	config->Profile = entry->h263_config->Profile;
	config->Level = entry->h263_config->Level;
	return config;
}


#ifndef M4_READ_ONLY

M4Err M4_AMR_NewStreamConfig(M4File *the_file, u32 trackNumber, LPAMRCONFIGURATION param, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	u32 dataRefIndex;
	TrackReferenceTypeAtom *dpnd;
	TrackReferenceAtom *tref;
	AMRSampleEntryAtom *entry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !param) return M4BadParam;

	dpnd = NULL;
	tref = NULL;

	//get or create the data ref
	e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
	if (e) return e;
	if (!dataRefIndex) {
		e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
		if (e) return e;
	}
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	//create a new entry
	entry = (AMRSampleEntryAtom *) CreateAtom(param->WideBandAMR ? WB_AMRSampleEntryAtomType : AMRSampleEntryAtomType);
	if (!entry) return M4OutOfMem;
	entry->amr_info = (AMRConfigAtom *) CreateAtom(AMRConfigAtomType);
	if (!entry->amr_info) {
		DelAtom((Atom *) entry);
		return M4OutOfMem;
	}
	entry->samplerate_hi = trak->Media->mediaHeader->timeScale;
	entry->dataReferenceIndex = dataRefIndex;
	entry->amr_info->decoder_version = param->decoder_version;
	entry->amr_info->vendor = param->vendor;
	entry->amr_info->frames_per_sample = param->frames_per_sample;
	entry->amr_info->mode_change_period = param->mode_change_period;
	entry->amr_info->mode_set = param->mode_set;
	e = ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, entry);
	*outDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
	return e;
}

M4Err M4_AMR_UpdateStreamConfig(M4File *the_file, u32 trackNumber, LPAMRCONFIGURATION param, u32 DescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	AMRSampleEntryAtom *entry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !param || !DescriptionIndex) return M4BadParam;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, DescriptionIndex-1);
	if (!entry) return M4BadParam;
	switch (entry->type) {
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		break;
	default:
		return M4BadParam;
	}

	entry->amr_info->decoder_version = param->decoder_version;
	entry->amr_info->frames_per_sample = param->frames_per_sample;
	entry->amr_info->mode_change_period = param->mode_change_period;
	entry->amr_info->mode_set = param->mode_set;
	entry->amr_info->vendor = param->vendor;
	//change atom type
	entry->type = param->WideBandAMR ? WB_AMRSampleEntryAtomType : AMRSampleEntryAtomType;
	return M4OK;
}


M4Err M4_H263_NewStreamConfig(M4File *the_file, u32 trackNumber, LPH263CONFIGURATION param, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	u32 dataRefIndex;
	TrackReferenceTypeAtom *dpnd;
	TrackReferenceAtom *tref;
	H263SampleEntryAtom *entry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !param) return M4BadParam;

	dpnd = NULL;
	tref = NULL;

	//get or create the data ref
	e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
	if (e) return e;
	if (!dataRefIndex) {
		e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
		if (e) return e;
	}
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	//create a new entry
	entry = (H263SampleEntryAtom *) CreateAtom(H263SampleEntryAtomType);
	if (!entry) return M4OutOfMem;
	entry->h263_config = (H263ConfigAtom *) CreateAtom(H263ConfigAtomType);
	if (!entry->h263_config) {
		DelAtom((Atom *) entry);
		return M4OutOfMem;
	}
	entry->dataReferenceIndex = dataRefIndex;
	entry->h263_config->decoder_version = param->decoder_version;
	entry->h263_config->vendor = param->vendor;
	entry->h263_config->Level = param->Level;
	entry->h263_config->Profile = param->Profile;
	e = ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, entry);
	*outDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
	return e;
}

M4Err M4_H263_UpdateStreamConfig(M4File *the_file, u32 trackNumber, LPH263CONFIGURATION param, u32 DescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	H263SampleEntryAtom *entry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !param || !DescriptionIndex) return M4BadParam;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, DescriptionIndex-1);
	if (!entry || (entry->type!=H263SampleEntryAtomType) ) return M4BadParam;

	entry->h263_config->decoder_version = param->decoder_version;
	entry->h263_config->vendor = param->vendor;
	entry->h263_config->Profile = param->Profile;
	entry->h263_config->Level = param->Level;
	return M4OK;
}


#endif	//M4_READ_ONLY

