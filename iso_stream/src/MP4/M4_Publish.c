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

//set shadowing on/off
M4Err M4_SetSyncShadowEnabled(M4File *the_file, u32 trackNumber, u8 SyncShadowEnabled)
{
	TrackAtom *trak;
	SampleTableAtom *stbl;

	if (((M4Movie *)the_file)->openMode == M4_OPEN_READ) return M4InvalidMP4Mode;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stbl = trak->Media->information->sampleTable;
	if (SyncShadowEnabled) {
		if (!stbl->ShadowSync) stbl->ShadowSync = (ShadowSyncAtom *) CreateAtom(ShadowSyncAtomType);
	} else {
		if (stbl->ShadowSync) DelAtom((Atom *) stbl->ShadowSync);
	}
	return M4OK;
}

//fill the sync shadow table
M4Err M4_SetSyncShadow(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 syncSample)
{
	TrackAtom *trak;
	SampleTableAtom *stbl;
	u8 isRAP;
	M4Err e;

	if (((M4Movie *)the_file)->openMode == M4_OPEN_READ) return M4InvalidMP4Mode;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !sampleNumber || !syncSample) return M4BadParam;

	stbl = trak->Media->information->sampleTable;
	if (!stbl->ShadowSync) return M4BadParam;

	//if no sync, skip
	if (!stbl->SyncSample) return M4OK;
	//else set the sync shadow.
	//if the sample is sync, ignore
	e = stbl_GetSampleRAP(stbl->SyncSample, sampleNumber, &isRAP, NULL, NULL);
	if (e) return e;
	if (isRAP) return M4OK;
	//if the shadowing sample is not sync, error
	e = stbl_GetSampleRAP(stbl->SyncSample, syncSample, &isRAP, NULL, NULL);
	if (e) return e;
	if (!isRAP) return M4BadParam;

	return stbl_SetSyncShadow(stbl->ShadowSync, sampleNumber, syncSample);
}

//set the GroupID of a track (only used for interleaving)
M4Err M4_SetTrackGroup(M4File *the_file, u32 trackNumber, u32 GroupID)
{
	TrackAtom *trak;

	if (((M4Movie *)the_file)->openMode != M4_OPEN_EDIT) return M4InvalidMP4Mode;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !GroupID) return M4BadParam;

	trak->Media->information->sampleTable->groupID = GroupID;
	return M4OK;
}


//set the Priority of a track within a Group (only used for tight interleaving)
//Priority ranges from 1 to 9
M4Err M4_SetTrackPriorityInGroup(M4File *the_file, u32 trackNumber, u32 Priority)
{
	TrackAtom *trak;

	if (((M4Movie *)the_file)->openMode != M4_OPEN_EDIT) return M4InvalidMP4Mode;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !Priority) return M4BadParam;

	trak->Media->information->sampleTable->trackPriority = Priority > 255 ? 255 : Priority;
	return M4OK;
}

//set the max SamplesPerChunk (for file optimization)
M4Err M4_SetMaxSamplesPerChunk(M4File *the_file, u32 trackNumber, u32 maxSamplesPerChunk)
{
	TrackAtom *trak;

	if (((M4Movie *)the_file)->openMode == M4_OPEN_READ) return M4InvalidMP4Mode;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !maxSamplesPerChunk) return M4BadParam;

	trak->Media->information->sampleTable->MaxSamplePerChunk = maxSamplesPerChunk;
	return M4OK;
}


M4Err M4_SetExtractionSLConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, SLConfigDescriptor *slConfig)
{
	TrackAtom *trak;
	SampleEntryAtom *entry;
	M4Err e;
	SLConfigDescriptor **slc;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, StreamDescriptionIndex, &entry, NULL);
	if (e) return e;

	//we must be sure we are not using a remote ESD
	switch (entry->type) {
	case MPEGSampleEntryAtomType:
		if (((MPEGSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = & ((MPEGSampleEntryAtom *)entry)->slc;
		break;
	case MPEGAudioSampleEntryAtomType:
		if (((MPEGAudioSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = & ((MPEGAudioSampleEntryAtom *)entry)->slc;
		break;
	case MPEGVisualSampleEntryAtomType:
		if (((MPEGVisualSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = & ((MPEGVisualSampleEntryAtom *)entry)->slc;
		break;
	default:
		return M4BadParam;
	}

	if (*slc) {
		OD_DeleteDescriptor((Descriptor **)slc);
		*slc = NULL;
	}
	if (!slConfig) return M4OK;
	//finally duplicate the SL
	return OD_DuplicateDescriptor((Descriptor *) slConfig, (Descriptor **) slc);
}


M4Err M4_GetExtractionSLConfig(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, SLConfigDescriptor **slConfig)
{
	TrackAtom *trak;
	SampleEntryAtom *entry;
	M4Err e;
	SLConfigDescriptor *slc;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, StreamDescriptionIndex, &entry, NULL);
	if (e) return e;

	//we must be sure we are not using a remote ESD
	slc = NULL;
	*slConfig = NULL;
	switch (entry->type) {
	case MPEGSampleEntryAtomType:
		if (((MPEGSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = ((MPEGSampleEntryAtom *)entry)->slc;
		break;
	case MPEGAudioSampleEntryAtomType:
		if (((MPEGAudioSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = ((MPEGAudioSampleEntryAtom *)entry)->slc;
		break;
	case MPEGVisualSampleEntryAtomType:
		if (((MPEGVisualSampleEntryAtom *)entry)->esd->desc->slConfig->predefined != SLPredef_MP4) return M4BadParam;
		slc = ((MPEGVisualSampleEntryAtom *)entry)->slc;
		break;
	default:
		return M4BadParam;
	}

	if (!slc) return M4OK;
	//finally duplicate the SL
	return OD_DuplicateDescriptor((Descriptor *) slc, (Descriptor **) slConfig);
}


u32 M4_GetTrackGroup(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;
	return trak->Media->information->sampleTable->groupID;
}


u32 M4_GetTrackPriorityInGroup(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	return trak->Media->information->sampleTable->trackPriority;
	return M4OK;
}

#endif

