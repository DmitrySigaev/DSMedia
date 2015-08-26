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

#ifndef M4_READ_ONLY

M4Err CanAccessMovie(M4Movie *movie, u32 Mode)
{
	if (!movie) return M4BadParam;
	if (movie->openMode < Mode) return M4InvalidMP4Mode;

#ifndef	M4_ISO_NO_FRAGMENTS
	if (movie->FragmentsFlags & FRAG_WRITE_READY) return M4InvalidMP4Mode;
#endif
	return M4OK;
}

M4Err moov_AddAtom(MovieAtom *ptr, Atom *a);
M4Err trak_AddAtom(TrackAtom *ptr, Atom *a);
M4Err udta_AddAtom(UserDataAtom *ptr, Atom *a);
M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex);
M4Err tref_AddAtom(TrackReferenceAtom *ptr, Atom *a);

static M4Err unpack_track(TrackAtom *trak)
{
	M4Err e = M4OK;
	if (!trak->is_unpacked) {
		e = stbl_UnpackOffsets(trak->Media->information->sampleTable);
		trak->is_unpacked = 1;
	}
	return e;
}

/**************************************************************
					File Writing / Editing
**************************************************************/
//quick function to add an IOD/OD to the file if not present (iods is optional)
M4Err AddMovieIOD(MovieAtom *moov, u8 isIOD)
{
	Descriptor *od;
	ObjectDescriptorAtom *iods;
	
	//do we have an IOD ?? If not, create one.
	if (moov->iods) return M4OK;

	if (isIOD) {
		od = OD_NewDescriptor(MP4_IOD_Tag);
	} else {
		od = OD_NewDescriptor(MP4_OD_Tag);
	}
	if (!od) return M4OutOfMem;
	((M4F_ObjectDescriptor *)od)->objectDescriptorID = 1;

	iods = (ObjectDescriptorAtom *) CreateAtom(ObjectDescriptorAtomType);
	iods->descriptor = od;
	return moov_AddAtom(moov, (Atom *)iods);
}

//add a track to the root OD
M4Err M4_AddTrackToRootOD(M4File *the_file, u32 trackNumber)
{
	M4Err e;
	ES_ID_Inc *inc;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	//oops, we need an ODAtom now
	if (!movie->moov->iods) AddMovieIOD(movie->moov, 0);

	if (M4_IsTrackInRootOD(the_file, trackNumber) == 1) return M4OK;

	inc = (ES_ID_Inc *) OD_NewDescriptor(ES_ID_IncTag);
	inc->trackID = M4_GetTrackID(the_file, trackNumber);
	if (!inc->trackID) {
		OD_DeleteDescriptor((Descriptor **)&inc);
		return movie->LastError;
	}
	if ( (movie->LastError = M4_AddDescriptorToRootOD(the_file, (Descriptor *)inc) ) ) {
		return movie->LastError;
	}
	return OD_DeleteDescriptor((Descriptor **)&inc);
}

//remove the root OD
M4Err M4_RemoveRootOD(M4File *the_file)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	//oops, we need an ODAtom now
	if (!movie->moov->iods) return M4OK;
	DelAtom((Atom *)movie->moov->iods);
	movie->moov->iods = NULL;
	return M4OK;
}

//remove a track to the root OD
M4Err M4_RemoveTrackFromRootOD(M4File *the_file, u32 trackNumber)
{
	Chain *esds;
	ES_ID_Inc *inc;
	u32 i;
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	if (!M4_IsTrackInRootOD(the_file, trackNumber)) return M4OK;

	//oops, we need an ODAtom now
	if (!movie->moov->iods) AddMovieIOD(movie->moov, 0);

	switch (movie->moov->iods->descriptor->tag) {
	case MP4_IOD_Tag:
		esds = ((M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor)->ES_ID_IncDescriptors;
		break;
	case MP4_OD_Tag:
		esds = ((M4F_ObjectDescriptor *)movie->moov->iods->descriptor)->ES_ID_IncDescriptors;
		break;
	default:
		return M4InvalidMP4File;
	}

	//get the desc
	for (i=0; i<ChainGetCount(esds); i++) {
		inc = (ES_ID_Inc*)ChainGetEntry(esds, i);
		if (inc->trackID == M4_GetTrackID(the_file, trackNumber)) {
			OD_DeleteDescriptor((Descriptor **)&inc);
			ChainDeleteEntry(esds, i);
			break;
		}
	}
	//we don't remove the iod for P&Ls and other potential info
	return M4OK;
}

//sets the enable flag of a track
M4Err M4_SetTrackEnabled(M4File *the_file, u32 trackNumber, u8 enableTrack)
{
	M4Err e;
	TrackAtom *trak;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	if (enableTrack) {
		trak->Header->flags |= 1;
	} else {
		trak->Header->flags &= ~1;
	}
	return M4OK;
}

M4Err M4_AddDescriptorToRootOD(M4File *the_file, Descriptor *theDesc)
{
	M4Err e;
	Descriptor *desc, *dupDesc;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	//oops, we need an ODAtom now
	if (!movie->moov->iods) AddMovieIOD(movie->moov, 0);

	desc = movie->moov->iods->descriptor;
	//the type of desc is handled at the OD/IOD level, we'll be notified
	//if the desc is not allowed
	switch (desc->tag) {
	case MP4_IOD_Tag:
	case MP4_OD_Tag:
		//duplicate the desc
		e = OD_DuplicateDescriptor(theDesc, &dupDesc);
		if (e) return e;
		//add it (MUST BE  (I)OD level desc)
		movie->LastError = OD_AddDescToDesc(desc, dupDesc);
		if (movie->LastError) OD_DeleteDescriptor((Descriptor **)&dupDesc);
		break;
	default:
		movie->LastError = M4InvalidMP4File;
		break;
	}
	return movie->LastError;
}


M4Err M4_SetTimeScale(M4File *the_file, u32 timeScale)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	movie->moov->mvhd->timeScale = timeScale;
	movie->interleavingTime = timeScale;
	return M4OK;
}

M4Err M4_SetMoviePLIndication(M4File *the_file, u8 PL_Code, u8 ProfileLevel)
{
	M4F_InitialObjectDescriptor *iod;
	M4F_ObjectDescriptor *od;
	M4Movie *movie;	
	M4Err e;
	movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	if (!movie->moov->iods) AddMovieIOD(movie->moov, 1);

	//if OD, switch to IOD
	if (movie->moov->iods->descriptor->tag == MP4_OD_Tag) {
		od = (M4F_ObjectDescriptor *) movie->moov->iods->descriptor;
		iod = (M4F_InitialObjectDescriptor*)malloc(sizeof(M4F_InitialObjectDescriptor));
		iod->ES_ID_IncDescriptors = od->ES_ID_IncDescriptors;
		od->ES_ID_IncDescriptors = NULL;
		//not used in root OD
		iod->ES_ID_RefDescriptors = NULL;
		iod->extensionDescriptors = od->extensionDescriptors;
		od->extensionDescriptors = NULL;
		iod->IPMPDescriptorPointers = od->IPMPDescriptorPointers;
		od->IPMPDescriptorPointers = NULL;
		iod->objectDescriptorID = od->objectDescriptorID;
		iod->OCIDescriptors = od->OCIDescriptors;
		od->OCIDescriptors = NULL;
		iod->tag = MP4_IOD_Tag;
		iod->URLString = od->URLString;
		od->URLString = NULL;

		OD_DeleteDescriptor((Descriptor **) &od);
		movie->moov->iods->descriptor = (Descriptor *)iod;
	}		
	iod = (M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor;

	switch (PL_Code) {
	case M4_PL_AUDIO:
		iod->audio_profileAndLevel = ProfileLevel;
		break;
	case M4_PL_GRAPHICS:
		iod->graphics_profileAndLevel = ProfileLevel;
		break;
	case M4_PL_OD:
		iod->OD_profileAndLevel = ProfileLevel;
		break;
	case M4_PL_SCENE:
		iod->scene_profileAndLevel = ProfileLevel;
		break;
	case M4_PL_VISUAL:
		iod->visual_profileAndLevel = ProfileLevel;
		break;
	case M4_PL_INLINE:
		iod->inlineProfileFlag = ProfileLevel ? 1 : 0;
		break;
	}
	return M4OK;
}


M4Err M4_SetRootOD_ID(M4File *the_file, u32 OD_ID)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	if (!movie->moov->iods) AddMovieIOD(movie->moov, 0);

	switch (movie->moov->iods->descriptor->tag) {
	case MP4_OD_Tag:
		((M4F_ObjectDescriptor *)movie->moov->iods->descriptor)->objectDescriptorID = OD_ID;
		break;
	case MP4_IOD_Tag:
		((M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor)->objectDescriptorID = OD_ID;
		break;
	default:
		return M4InvalidMP4File;
	}
	return M4OK;
}

M4Err M4_SetRootOD_URL(M4File *the_file, char *url_string)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	if (!movie->moov->iods) AddMovieIOD(movie->moov, 0);
	switch (movie->moov->iods->descriptor->tag) {
	case MP4_OD_Tag:
		if (((M4F_ObjectDescriptor *)movie->moov->iods->descriptor)->URLString) free(((M4F_ObjectDescriptor *)movie->moov->iods->descriptor)->URLString);
		((M4F_ObjectDescriptor *)movie->moov->iods->descriptor)->URLString = url_string ? strdup(url_string) : NULL;
		break;
	case MP4_IOD_Tag:
		if (((M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor)->URLString) free(((M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor)->URLString);
		((M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor)->URLString = url_string ? strdup(url_string) : NULL;
		break;
	default:
		return M4InvalidMP4File;
	}
	return M4OK;
}



//creates a new Track. If trackID = 0, the trackID is chosen by the API
//returns the track number or 0 if error
u32 M4_NewTrack(M4File *the_file, u32 trackID, u32 MediaType, u32 TimeScale)
{
	u32 trakID, nextID;
	M4Err e;
	u64 now;
	u8 isHint;
	TrackAtom *trak;
	TrackHeaderAtom *tkhd;
	MediaAtom *mdia;
	void DelAtom(Atom *ptr);

	M4Movie *movie = (M4Movie *) the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) {
		M4SetLastError(the_file, e);
		return 0;
	}


	isHint = 0;
	//we're creating a hint track... it's the same, but mode HAS TO BE EDIT
	if (MediaType == M4_HintMediaType) {
		if (movie->openMode != M4_OPEN_EDIT) return 0;
		isHint = 1;
	}

	mdia = NULL;
	tkhd = NULL;
	trak = NULL;
	if (trackID) {
		//check if we are in ES_ID boundaries
		if (!isHint && (trackID > 0xFFFF)) {
			M4SetLastError(the_file, M4ES_ID_Overflow);
			return 0;
		}
		//here we should look for available IDs ...
		if (!RequestTrack(movie->moov, trackID)) return 0;
		//OK, set the right track
		trakID = trackID;
		nextID = trackID > movie->moov->mvhd->nextTrackID ? trackID+1 : movie->moov->mvhd->nextTrackID + 1;
	} else {
		trakID = movie->moov->mvhd->nextTrackID;
		while (1) {
			if (RequestTrack(movie->moov, trakID)) break;
			trakID += 1;
			if (trakID == 0xFFFFFFFF) break;
		}
		if (trakID == 0xFFFFFFFF) {
			M4SetLastError(the_file, M4BadParam);
			return 0;
		}
		if (! isHint && (trakID > 0xFFFF)) {
			M4SetLastError(the_file, M4ES_ID_Overflow);
			return 0;
		}
		nextID = trakID + 1;
	}
	
	//OK, now create a track...
	trak = (TrackAtom *) CreateAtom(TrackAtomType);
	if (!trak) {
		M4SetLastError(the_file, M4OutOfMem);
		return 0;
	}
	tkhd = (TrackHeaderAtom *) CreateAtom(TrackHeaderAtomType);
	if (!tkhd) {
		M4SetLastError(the_file, M4OutOfMem);
		DelAtom((Atom *)trak);
		return 0;
	}
	now = GetMP4Time();
	tkhd->creationTime = now;
	tkhd->modificationTime = now;
	//OK, set up the media trak
	e = NewMedia(&mdia, MediaType, TimeScale);
	if (e) {
		DelAtom((Atom *)mdia);
		DelAtom((Atom *)trak);
		DelAtom((Atom *)tkhd);
		return 0;
	}
	//OK, add this media to our track
	mdia->mediaTrack = trak;

	e = trak_AddAtom(trak, (Atom *) tkhd); if (e) goto err_exit;
	e = trak_AddAtom(trak, (Atom *) mdia); if (e) goto err_exit;
	tkhd->trackID = trakID;

	
	//some default properties for Audio, Visual or private tracks
	if (MediaType == M4_VisualMediaType) {
		/*320-240 pix in 16.16*/
		tkhd->width = 0x01400000;
		tkhd->height = 0x00F00000;
	} else if (MediaType == M4_AudioMediaType) {
		tkhd->volume = 0x0100;
	}

	mdia->mediaHeader->creationTime = mdia->mediaHeader->modificationTime = now;
	trak->Header->creationTime = trak->Header->modificationTime = now;

	//OK, add our trak
	e = moov_AddAtom(movie->moov, (Atom *)trak); if (e) goto err_exit;
	//set the new ID available
	movie->moov->mvhd->nextTrackID = nextID;

	//and return our track number
	return M4_GetTrackByID(the_file, trakID);

err_exit:
	if (tkhd) DelAtom((Atom *)tkhd);
	if (trak) DelAtom((Atom *)trak);
	if (mdia) DelAtom((Atom *)mdia);
	return 0;
}


//Create a new StreamDescription in the file. The URL and URN are used to describe external media
M4Err M4_NewStreamDescription(M4File *the_file,
							   u32 trackNumber, 
							   ESDescriptor *esd, 
							   char *URLname, 
							   char *URNname, 
							   u32 *outDescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	u32 dataRefIndex;
	ESDescriptor *new_esd;
	TrackReferenceTypeAtom *dpnd;
	TrackReferenceAtom *tref;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || 
		!esd || !esd->decoderConfig || 
		!esd->slConfig) return M4BadParam;

	dpnd = NULL;
	tref = NULL;

	//get or create the data ref
	e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
	if (e) return e;
	if (!dataRefIndex) {
		e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
		if (e) return e;
	}
	//duplicate our desc
	e = OD_DuplicateDescriptor((Descriptor *)esd, (Descriptor **)&new_esd);
	if (e) return e;;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	e = Track_SetStreamDescriptor(trak, 0, dataRefIndex, new_esd, outDescriptionIndex);
	if (e) {
		OD_DeleteDescriptor((Descriptor **)&new_esd);
		return e;
	}
	if (new_esd->URLString) {

	}
	return e;
}

//Add samples to a track. Use streamDescriptionIndex to specify the desired stream (if several)
M4Err M4_AddSample(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, M4Sample *sample)
{
	M4Err e;
	TrackAtom *trak;
	SampleEntryAtom *entry;
	u32 dataRefIndex;
	u64 data_offset;
	u32 descIndex;
	DataEntryURLAtom *Dentry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = unpack_track(trak);
	if (e) return e;

	//REWRITE ANY OD STUFF
	if (trak->Media->handler->handlerType == M4_ODMediaType) {
		e = Media_ParseODFrame(trak->Media, sample);
		if (e) return e;
	}
	//OK, add the sample
	//1- Get the streamDescriptionIndex and dataRefIndex
	//not specified, get the latest used...
	descIndex = StreamDescriptionIndex;
	if (!StreamDescriptionIndex) {
		descIndex = trak->Media->information->sampleTable->currentEntryIndex;
	}
	e = Media_GetSampleDesc(trak->Media, descIndex, &entry, &dataRefIndex);
	if (e) return e;
	if (!entry || !dataRefIndex) return M4BadParam;
	//set the current to this one
	trak->Media->information->sampleTable->currentEntryIndex = descIndex;


	//get this dataRef and return false if not self contained
	Dentry = (DataEntryURLAtom*)ChainGetEntry(trak->Media->information->dataInformation->dref->atomList, dataRefIndex - 1);
	if (!Dentry || Dentry->flags != 1) return M4BadParam;

	//Open our data map. We are adding stuff, so use EDIT
	e = DataMap_Open(trak->Media, dataRefIndex, 1);
	if (e) return e;

	//Get the offset...
	data_offset = DataMap_GetTotalOffset(trak->Media->information->dataHandler);
	//add the meta data
	e = Media_AddSample(trak->Media, data_offset, sample, descIndex, 0);
	if (e) return e;
	//add the media data
	e = DataMap_AddData(trak->Media->information->dataHandler, sample->data, sample->dataLength);
	if (e) return e;
	//OK, update duration
	e = Media_SetDuration(trak);
	if (e) return e;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	return SetTrackDuration(trak);
}

M4Err M4_AddSampleShadow(M4File *the_file, u32 trackNumber, M4Sample *sample)
{
	M4Err e;
	TrackAtom *trak;
	M4Sample *prev;
	SampleEntryAtom *entry;
	u32 dataRefIndex;
	u64 data_offset;
	u32 descIndex;
	u32 sampleNum, prevSampleNum;
	DataEntryURLAtom *Dentry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !sample) return M4BadParam;

	e = unpack_track(trak);
	if (e) return e;

	/*REWRITE ANY OD STUFF*/
	if (trak->Media->handler->handlerType == M4_ODMediaType) {
		e = Media_ParseODFrame(trak->Media, sample);
		if (e) return e;
	}

	e = findEntryForTime(trak->Media->information->sampleTable, sample->DTS, 0, &sampleNum, &prevSampleNum);
	if (e) return e;
	/*we need the EXACT match*/
	if (!sampleNum) return M4BadParam;

	prev = M4_GetSampleInfo(the_file, trackNumber, sampleNum, &descIndex, NULL);
	if (!prev) return M4_GetLastError(the_file);
	M4_DeleteSample(&prev);

	e = Media_GetSampleDesc(trak->Media, descIndex, &entry, &dataRefIndex);
	if (e) return e;
	if (!entry || !dataRefIndex) return M4BadParam;
	trak->Media->information->sampleTable->currentEntryIndex = descIndex;

	//get this dataRef and return false if not self contained
	Dentry = (DataEntryURLAtom*)ChainGetEntry(trak->Media->information->dataInformation->dref->atomList, dataRefIndex - 1);
	if (!Dentry || Dentry->flags != 1) return M4BadParam;

	//Open our data map. We are adding stuff, so use EDIT
	e = DataMap_Open(trak->Media, dataRefIndex, 1);
	if (e) return e;

	data_offset = DataMap_GetTotalOffset(trak->Media->information->dataHandler);
	e = Media_AddSample(trak->Media, data_offset, sample, descIndex, sampleNum);
	if (e) return e;
	//add the media data
	e = DataMap_AddData(trak->Media->information->dataHandler, sample->data, sample->dataLength);
	if (e) return e;

	//OK, update duration
	e = Media_SetDuration(trak);
	if (e) return e;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	return SetTrackDuration(trak);
}

M4Err M4_AppendSampleData(M4File *the_file, u32 trackNumber, unsigned char *data, u32 data_size)
{
	M4Err e;
	TrackAtom *trak;
	SampleEntryAtom *entry;
	u32 dataRefIndex;
	u32 descIndex;
	DataEntryURLAtom *Dentry;

	if (!data_size) return M4OK;
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	if (trak->Media->handler->handlerType == M4_ODMediaType) return M4BadParam;

	//OK, add the sample
	descIndex = trak->Media->information->sampleTable->currentEntryIndex;

	e = Media_GetSampleDesc(trak->Media, descIndex, &entry, &dataRefIndex);
	if (e) return e;
	if (!entry || !dataRefIndex) return M4BadParam;

	//get this dataRef and return false if not self contained
	Dentry = (DataEntryURLAtom*)ChainGetEntry(trak->Media->information->dataInformation->dref->atomList, dataRefIndex - 1);
	if (!Dentry || Dentry->flags != 1) return M4BadParam;

	//Open our data map. We are adding stuff, so use EDIT
	e = DataMap_Open(trak->Media, dataRefIndex, 1);
	if (e) return e;

	//add the media data
	e = DataMap_AddData(trak->Media->information->dataHandler, data, data_size);
	if (e) return e;
	//update data size
	return stbl_SampleSizeAppend(trak->Media->information->sampleTable->SampleSize, data_size);
}


//Add sample reference to a track. The SampleOffset is the offset of the data in the referenced file
//you must have created a StreamDescription with URL or URN specifying your referenced file
//the data offset specifies the begining of the chunk
//Use streamDescriptionIndex to specify the desired stream (if several)
M4Err M4_AddSampleReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, M4Sample *sample, u64 dataOffset)
{
	TrackAtom *trak;
	SampleEntryAtom *entry;
	u32 dataRefIndex;
	u32 descIndex;
	DataEntryURLAtom *Dentry;
	M4Err e;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = unpack_track(trak);
	if (e) return e;

	//OD is not allowed as a data ref
	if (trak->Media->handler->handlerType == M4_ODMediaType) {
		return M4BadParam;
	}
	//OK, add the sample
	//1- Get the streamDescriptionIndex and dataRefIndex
	//not specified, get the latest used...
	descIndex = StreamDescriptionIndex;
	if (!StreamDescriptionIndex) {
		descIndex = trak->Media->information->sampleTable->currentEntryIndex;
	}
	e = Media_GetSampleDesc(trak->Media, descIndex, &entry, &dataRefIndex);
	if (e) return e;
	if (!entry || !dataRefIndex) return M4BadParam;
	//set the current to this one
	trak->Media->information->sampleTable->currentEntryIndex = descIndex;


	//get this dataRef and return false if self contained
	Dentry =(DataEntryURLAtom*) ChainGetEntry(trak->Media->information->dataInformation->dref->atomList, dataRefIndex - 1);
	if (Dentry->flags == 1) return M4BadParam;

	//add the meta data
	e = Media_AddSample(trak->Media, dataOffset, sample, descIndex, 0);
	if (e) return e;

	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	//OK, update duration
	e = Media_SetDuration(trak);
	if (e) return e;
	return SetTrackDuration(trak);

}

//set the duration of the last media sample. If not set, the duration of the last sample is the
//duration of the previous one if any, or 1000 (default value).
M4Err M4_SetLastSampleDuration(M4File *the_file, u32 trackNumber, u32 duration)
{
	TrackAtom *trak;
	sttsEntry *ent;
	M4Err e;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//get the last entry
	ent = (sttsEntry*)ChainGetEntry(trak->Media->information->sampleTable->TimeToSample->entryList, ChainGetCount(trak->Media->information->sampleTable->TimeToSample->entryList));
	if (!ent) return M4BadParam;
	//we only have one sample
	if (ent->sampleCount == 1) {
		ent->sampleDelta = duration;
	} else {
		if (ent->sampleDelta == duration) return M4OK;
		ent->sampleCount -= 1;
		ent = (sttsEntry*)malloc(sizeof(sttsEntry));
		ent->sampleCount = 1;
		ent->sampleDelta = duration;
		//add this entry
		ChainAddEntry(trak->Media->information->sampleTable->TimeToSample->entryList, ent);
		//and update the write cache
		trak->Media->information->sampleTable->TimeToSample->w_currentEntry = ent;
		trak->Media->information->sampleTable->TimeToSample->w_currentSampleNum = trak->Media->information->sampleTable->SampleSize->sampleCount;
	}
	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	//OK, update duration
	e = Media_SetDuration(trak);
	if (e) return e;
	return SetTrackDuration(trak);
}

//update a sample data in the media. Note that the sample MUST exists
M4Err M4_UpdateSample(M4File *the_file, u32 trackNumber, u32 sampleNumber, M4Sample *sample)
{
	M4Err e;
	TrackAtom *trak;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_EDIT);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (((M4Movie *)the_file)->openMode != M4_OPEN_EDIT) return M4InvalidMP4Mode;

	e = unpack_track(trak);
	if (e) return e;

	//block for hint tracks
	if (trak->Media->handler->handlerType == M4_HintMediaType) return M4BadParam;

	//REWRITE ANY OD STUFF
	if (trak->Media->handler->handlerType == M4_ODMediaType) {
		e = Media_ParseODFrame(trak->Media, sample);
		if (e) return e;
	}
	//OK, update it
	e = Media_UpdateSample(trak->Media, sampleNumber, sample);
	if (e) return e;

	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	return M4OK;
}

//update a sample data in the media. Note that the sample MUST exists,
//that sample->data MUST be NULL and sample->dataLength must be NON NULL;
M4Err M4_UpdateSampleReference(M4File *the_file, u32 trackNumber, u32 sampleNumber, M4Sample *sample, u64 data_offset)
{
	M4Err e;
	TrackAtom *trak;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_EDIT);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//block for hint tracks
	if (trak->Media->handler->handlerType == M4_HintMediaType) return M4BadParam;

	if (!sampleNumber || !sample) return M4BadParam;

	e = unpack_track(trak);
	if (e) return e;

	//OD is not allowed as a data ref
	if (trak->Media->handler->handlerType == M4_ODMediaType) {
		return M4BadParam;
	}
	//OK, update it
	e = Media_UpdateSampleReference(trak->Media, sampleNumber, sample, data_offset);
	if (e) return e;

	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	return M4OK;
}


//Remove a given sample
M4Err M4_RemoveSample(M4File *the_file, u32 trackNumber, u32 sampleNumber)
{
	M4Err e;
	TrackAtom *trak;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_EDIT);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !sampleNumber || (sampleNumber > trak->Media->information->sampleTable->SampleSize->sampleCount) )
		return M4BadParam;

	//block for hint tracks
	if (trak->Media->handler->handlerType == M4_HintMediaType) return M4BadParam;

	//remove DTS
	e = stbl_RemoveDTS(trak->Media->information->sampleTable, sampleNumber, trak->Media->mediaHeader->timeScale);
	if (e) return e;
	//remove CTS if any
	if (trak->Media->information->sampleTable->CompositionOffset) {
		e = stbl_RemoveCTS(trak->Media->information->sampleTable, sampleNumber);
		if (e) return e;
	}
	//remove size
	e = stbl_RemoveSize(trak->Media->information->sampleTable->SampleSize, sampleNumber);
	if (e) return e;
	//remove sampleToChunk and chunk
	e = stbl_RemoveChunk(trak->Media->information->sampleTable, sampleNumber);
	if (e) return e;
	//remove sync
	if (trak->Media->information->sampleTable->SyncSample) {
		e = stbl_RemoveRAP(trak->Media->information->sampleTable, sampleNumber);
		if (e) return e;
	}
	//remove shadow
	if (trak->Media->information->sampleTable->ShadowSync) {
		e = stbl_RemoveShadow(trak->Media->information->sampleTable->ShadowSync, sampleNumber);
		if (e) return e;
	}
	//remove padding
	e = stbl_RemovePaddingBits(trak->Media->information->sampleTable, sampleNumber);
	if (e) return e;
	
	return SetTrackDuration(trak);
}


M4Err M4_SetFinalFileName(M4File *the_file, char *filename)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie ) return M4BadParam;

	//if mode is not OPEN_EDIT file was created under the right name
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_EDIT);
	if (e) return e;

	if (filename) {
		//we don't allow file overwriting
		if ( (movie->openMode == M4_OPEN_EDIT) 
			&& movie->fileName && !strcmp(filename, movie->fileName))
			return M4BadParam;
		if (movie->finalName) free(movie->finalName);
		movie->finalName = strdup(filename);
		if (!movie->finalName) return M4OutOfMem;
	}
	return M4OK;
}

//Add a system descriptor to the ESD of a stream(EDIT or WRITE mode only)
M4Err M4_AddDescriptorToESD(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, Descriptor *theDesc)
{
	IPI_DescrPointer *ipiD;
	M4Err e;
	u16 tmpRef;
	TrackAtom *trak;
	Descriptor *desc;
	ESDescriptor *esd;
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *dpnd;
	
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	/*GETS NATIVE DESCRIPTOR ONLY*/
	e = Media_GetESD(trak->Media, StreamDescriptionIndex, &esd, 1);
	if (e) return e;

	//duplicate the desc
	e = OD_DuplicateDescriptor(theDesc, &desc);
	if (e) return e;

	//and add it to the ESD EXCEPT IPI PTR (we need to translate from ES_ID to TrackID!!!
	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	switch (desc->tag) {
	case IPIPtr_Tag:
		goto insertIPI;
	
	default:
		return OD_AddDescToDesc((Descriptor *)esd, desc);
	}

	
insertIPI:
	if (esd->ipiPtr) 
		OD_DeleteDescriptor((Descriptor **) &esd->ipiPtr);

	ipiD = (IPI_DescrPointer *) desc;	
	//find a tref
	if (!trak->References) {
		tref = (TrackReferenceAtom *) CreateAtom(TrackReferenceAtomType);
		e = trak_AddAtom(trak, (Atom *)tref);
		if (e) return e;
	}
	tref = trak->References;

	e = Track_FindRef(trak, M4_IPI_Ref, &dpnd);
	if (e) return e;
	if (!dpnd) {
		tmpRef = 0;
		dpnd = (TrackReferenceTypeAtom *) CreateAtom(IPIReferenceAtomType);
		e = tref_AddAtom(tref, (Atom *) dpnd);
		if (e) return e;
		e = reftype_AddRefTrack(dpnd, ipiD->IPI_ES_Id, &tmpRef);
		if (e) return e;
		//and replace the tag and value...
		ipiD->IPI_ES_Id = tmpRef;
		ipiD->tag = IPI_DescPtr_Tag;
	} else {
		//Watch out! ONLY ONE IPI dependancy is allowed per stream
		dpnd->trackIDCount = 1;
		dpnd->trackIDs[0] = ipiD->IPI_ES_Id;
		//and replace the tag and value...
		ipiD->IPI_ES_Id = 1;
		ipiD->tag = IPI_DescPtr_Tag;
	}
	//and add the desc to the esd...
	return OD_AddDescToDesc((Descriptor *)esd, desc);
}


//use carefully. Very usefull when you made a lot of changes (IPMP, IPI, OCI, ...)
//THIS WILL REPLACE THE WHOLE DESCRIPTOR ...
M4Err M4_ChangeStreamDescriptor(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, ESDescriptor *newESD)
{
	M4Err e;
	ESDescriptor *esd;
	TrackAtom *trak;
	SampleEntryAtom *entry;
	SampleDescriptionAtom *stsd;
	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stsd = trak->Media->information->sampleTable->SampleDescription;
	if (!stsd) return movie->LastError = M4InvalidMP4File;

	if (!StreamDescriptionIndex || StreamDescriptionIndex > ChainGetCount(stsd->atomList)) {
		return movie->LastError = M4BadParam;
	}
	entry = (SampleEntryAtom *)ChainGetEntry(stsd->atomList, StreamDescriptionIndex - 1);
	//no support for generic sample entries (eg, no MPEG4 descriptor)
	if (entry == NULL) return M4BadParam;

	trak->Media->mediaHeader->modificationTime = GetMP4Time();
	//duplicate our desc
	e = OD_DuplicateDescriptor((Descriptor *)newESD, (Descriptor **)&esd);
	if (e) return e;
	return Track_SetStreamDescriptor(trak, StreamDescriptionIndex, entry->dataReferenceIndex, esd, NULL);
}

M4Err M4_SetVisualEntrySize(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 Width, u32 Height)
{
	M4Err e;
	TrackAtom *trak;
	SampleEntryAtom *entry;
	SampleDescriptionAtom *stsd;
	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stsd = trak->Media->information->sampleTable->SampleDescription;
	if (!stsd) {
		return movie->LastError = M4InvalidMP4File;
	}
	if (!StreamDescriptionIndex || StreamDescriptionIndex > ChainGetCount(stsd->atomList)) {
		return movie->LastError = M4BadParam;
	}
	entry = ChainGetEntry(stsd->atomList, StreamDescriptionIndex - 1);
	//no support for generic sample entries (eg, no MPEG4 descriptor)
	if (entry == NULL) return M4BadParam;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	//valid for MPEG visual, JPG and 3GPP H263
	switch (entry->type) {
	case MPEGVisualSampleEntryAtomType:
	case H263SampleEntryAtomType:
		((VisualSampleEntryAtom*)entry)->Width = Width;
		((VisualSampleEntryAtom*)entry)->Height = Height;
		trak->Header->width = Width<<16;
		trak->Header->height = Height<<16;
		return M4OK;
	/*check BIFS*/
	default:
		if (trak->Media->handler->handlerType==M4_BIFSMediaType) {
			trak->Header->width = Width<<16;
			trak->Header->height = Height<<16;
			return M4OK;
		}
		return M4BadParam;
	}
}

M4Err M4_SetAudioDescriptionInfo(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 sampleRate, u32 nbChannels, u8 bitsPerSample)
{
	M4Err e;
	TrackAtom *trak;
	SampleEntryAtom *entry;
	SampleDescriptionAtom *stsd;
	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stsd = trak->Media->information->sampleTable->SampleDescription;
	if (!stsd) {
		return movie->LastError = M4InvalidMP4File;
	}
	if (!StreamDescriptionIndex || StreamDescriptionIndex > ChainGetCount(stsd->atomList)) {
		return movie->LastError = M4BadParam;
	}
	entry = ChainGetEntry(stsd->atomList, StreamDescriptionIndex - 1);
	//no support for generic sample entries (eg, no MPEG4 descriptor)
	if (entry == NULL) return M4BadParam;
	trak->Media->mediaHeader->modificationTime = GetMP4Time();

	switch (entry->type) {
	case MPEGAudioSampleEntryAtomType:
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		((AudioSampleEntryAtom*)entry)->samplerate_hi = sampleRate;
		((AudioSampleEntryAtom*)entry)->samplerate_lo = 0;
		((AudioSampleEntryAtom*)entry)->channel_count = nbChannels;
		((AudioSampleEntryAtom*)entry)->bitspersample = bitsPerSample;
		return M4OK;
	/*check BIFS*/
	default:
		return M4BadParam;
	}
}

//set the storage mode of a file (FLAT, STREAMABLE, INTERLEAVED)
M4Err M4_SetStorageMode(M4File *the_file, u8 storageMode)
{
	M4Err e;
	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	switch (storageMode) {
	case M4_FLAT:
	case M4_STREAMABLE:
	case M4_INTERLEAVED:
		movie->storageMode = storageMode;
		return M4OK;
	case M4_FULL_INTERLEAVED:
		movie->storageMode = storageMode;
		return M4OK;
	default:
		return M4BadParam;
	}
}


//update or insert a new edit segment in the track time line. Edits are used to modify
//the media normal timing. EditTime and EditDuration are expressed in Movie TimeScale
//If a segment with EditTime already exists, IT IS ERASED
M4Err M4_SetEditSegment(M4File *the_file, u32 trackNumber, u32 EditTime, u32 EditDuration, u32 MediaTime, u8 EditMode)
{
	TrackAtom *trak;
	EditAtom *edts;
	EditListAtom *elst;
	edtsEntry *ent, *newEnt;
	u32 i;
	M4Err e;
	u64 startTime;
	M4Err edts_AddAtom(EditAtom *ptr, Atom *a);

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	edts = trak->EditAtom;
	if (! edts) {
		edts = (EditAtom *) CreateAtom(EditAtomType);
		if (!edts) return M4OutOfMem;
		trak_AddAtom(trak, (Atom *)edts);
	}
	elst = edts->editList;
	if (!elst) {
		elst = (EditListAtom *) CreateAtom(EditListAtomType);
		if (!elst) return M4OutOfMem;
		edts_AddAtom(edts, (Atom *)elst);
	}

	startTime = 0;
	ent = NULL;
	//get the prev entry to this startTime if any
	for (i = 0; i < ChainGetCount(elst->entryList); i++) {
		ent = (edtsEntry*)ChainGetEntry(elst->entryList, i);
		if ( (startTime <= EditTime) && (startTime + ent->segmentDuration > EditTime) ) 
			goto found;
		startTime += ent->segmentDuration;
	}

	//not found, add a new entry and adjust the prev one if any
	if (!ent) {
		newEnt = CreateEditEntry(EditDuration, MediaTime, EditMode);
		if (!newEnt) return M4OutOfMem;
		ChainAddEntry(elst->entryList, newEnt);
		return SetTrackDuration(trak);
	}
	
	startTime -= ent->segmentDuration;

found:
	
	//if same time, we erase the current one...
	if (startTime == EditTime) {
		ent->segmentDuration = EditDuration;
		switch (EditMode) {
		case M4_EDIT_EMPTY:
			ent->mediaRate = 1;
			ent->mediaTime = -1;
			break;
		case M4_EDIT_DWELL:
			ent->mediaRate = 0;
			ent->mediaTime = MediaTime;
			break;
		default:
			ent->mediaRate = 1;
			ent->mediaTime = MediaTime;
			break;
		}
		return SetTrackDuration(trak);
	}

	//adjust so that the prev ent leads to EntryTime
	//Note: we don't change the next one as it is unknown to us in
	//a lot of case (the author's changes)
	ent->segmentDuration = EditTime - startTime;
	newEnt = CreateEditEntry(EditDuration, MediaTime, EditMode);
	if (!newEnt) return M4OutOfMem;
	//is it the last entry ???
	if (i >= ChainGetCount(elst->entryList) - 1) {
		//add the new entry at the end
		ChainAddEntry(elst->entryList, newEnt);
		return SetTrackDuration(trak);
	} else {
		//insert after the current entry (which is i)
		ChainInsertEntry(elst->entryList, newEnt, i+1);
		return SetTrackDuration(trak);
	}
}

//remove the edit segments for the whole track
M4Err M4_RemoveEditSegments(M4File *the_file, u32 trackNumber)
{
	M4Err e;
	TrackAtom *trak;
	edtsEntry *ent;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	if (!trak->EditAtom || !trak->EditAtom->editList) return M4OK;

	while (ChainGetCount(trak->EditAtom->editList->entryList)) {
		ent = (edtsEntry*)ChainGetEntry(trak->EditAtom->editList->entryList, 0);
		free(ent);
		e = ChainDeleteEntry(trak->EditAtom->editList->entryList, 0);
		if (e) return e;
	}
	//then delete the EditAtom...
	DelAtom((Atom *)trak->EditAtom);
	trak->EditAtom = NULL;
	return SetTrackDuration(trak);
}


//remove the edit segments for the whole track
M4Err M4_RemoveEditSegment(M4File *the_file, u32 trackNumber, u32 seg_index)
{
	M4Err e;
	TrackAtom *trak;
	edtsEntry *ent, *next_ent;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !seg_index) return M4BadParam;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	if (!trak->EditAtom || !trak->EditAtom->editList) return M4OK;
	if (ChainGetCount(trak->EditAtom->editList->entryList)<=1) return M4_RemoveEditSegments(the_file, trackNumber);

	ent = (edtsEntry*) ChainGetEntry(trak->EditAtom->editList->entryList, seg_index-1);
	ChainDeleteEntry(trak->EditAtom->editList->entryList, seg_index-1);
	next_ent = ChainGetEntry(trak->EditAtom->editList->entryList, seg_index-1);
	if (next_ent) next_ent->segmentDuration += ent->segmentDuration;
	free(ent);
	return SetTrackDuration(trak);
}


M4Err M4_AppendEditSegment(M4File *the_file, u32 trackNumber, u32 EditDuration, u32 MediaTime, u8 EditMode)
{
	M4Err e;
	TrackAtom *trak;
	edtsEntry *ent;
	M4Err edts_AddAtom(EditAtom *ptr, Atom *a);
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	if (!trak->EditAtom) {
		EditAtom *edts = (EditAtom *) CreateAtom(EditAtomType);
		if (!edts) return M4OutOfMem;
		trak_AddAtom(trak, (Atom *)edts);
	}
	if (!trak->EditAtom->editList) {
		EditListAtom *elst = (EditListAtom *) CreateAtom(EditListAtomType);
		if (!elst) return M4OutOfMem;
		edts_AddAtom(trak->EditAtom, (Atom *)elst);
	}
	ent = malloc(sizeof(edtsEntry));
	if (!ent) return M4OutOfMem;

	ent->segmentDuration = EditDuration;
	switch (EditMode) {
	case M4_EDIT_EMPTY:
		ent->mediaRate = 1;
		ent->mediaTime = -1;
		break;
	case M4_EDIT_DWELL:
		ent->mediaRate = 0;
		ent->mediaTime = MediaTime;
		break;
	default:
		ent->mediaRate = 1;
		ent->mediaTime = MediaTime;
		break;
	}
	ChainAddEntry(trak->EditAtom->editList->entryList, ent);
	return SetTrackDuration(trak);
}

M4Err M4_ModifyEditSegment(M4File *the_file, u32 trackNumber, u32 seg_index, u32 EditDuration, u32 MediaTime, u8 EditMode)
{
	M4Err e;
	TrackAtom *trak;
	edtsEntry *ent;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !seg_index) return M4BadParam;
	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	if (!trak->EditAtom || !trak->EditAtom->editList) return M4OK;
	if (ChainGetCount(trak->EditAtom->editList->entryList)<seg_index) return M4BadParam;
	ent = (edtsEntry*) ChainGetEntry(trak->EditAtom->editList->entryList, seg_index-1);

	ent->segmentDuration = EditDuration;
	switch (EditMode) {
	case M4_EDIT_EMPTY:
		ent->mediaRate = 1;
		ent->mediaTime = -1;
		break;
	case M4_EDIT_DWELL:
		ent->mediaRate = 0;
		ent->mediaTime = MediaTime;
		break;
	default:
		ent->mediaRate = 1;
		ent->mediaTime = MediaTime;
		break;
	}
	return SetTrackDuration(trak);
}

//removes the desired track
M4Err M4_RemoveTrack(M4File *the_file, u32 trackNumber)
{
	M4Err e;
	TrackAtom *the_trak, *trak;
	M4Movie *movie;
	TrackReferenceTypeAtom *tref;
	u32 i, j, k, *newRefs, descIndex;
	u8 found;
	M4Sample *samp;
	the_trak = GetTrackFromFile(the_file, trackNumber);
	if (!the_trak) return M4BadParam;

	movie = (M4Movie *)the_file;
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	if (movie->moov->iods && movie->moov->iods->descriptor) {
		Descriptor *desc;
		ES_ID_Inc *inc;
		Chain *ESDs;
		desc = movie->moov->iods->descriptor;
		if (desc->tag == MP4_IOD_Tag) {
			ESDs = ((M4F_InitialObjectDescriptor *)desc)->ES_ID_IncDescriptors;
		} else if (desc->tag == MP4_OD_Tag) {
			ESDs = ((M4F_ObjectDescriptor *)desc)->ES_ID_IncDescriptors;
		} else {
			return M4InvalidMP4File;
		}

		//remove the track ref from the root OD if any
		for (i=0; i<ChainGetCount(ESDs); i++) {
			inc = (ES_ID_Inc*)ChainGetEntry(ESDs, i);
			if (inc->trackID == the_trak->Header->trackID) {
				OD_DeleteDescriptor((Descriptor **)&inc);
				ChainDeleteEntry(ESDs, i);
				i--;
			}
		}
	}

	//remove the track from the movie
	ChainDeleteItem(movie->moov->trackList, the_trak);
	ChainDeleteItem(movie->moov->atomList, the_trak);

	//rewrite any OD tracks
	for (i=0; i<ChainGetCount(movie->moov->trackList); i++) {
		trak = (TrackAtom*)ChainGetEntry(movie->moov->trackList, i);
		if (trak->Media->handler->handlerType != M4_ODMediaType) continue;
		//this is an OD track...
		j = M4_GetSampleCount(the_file, i+1);
		for (k=0; k < j; k++) {
			//getting the sample will remove the references to the deleted track in the output OD frame
			samp = M4_GetSample(the_file, i+1, k+1, &descIndex);
			//so let's update with the new OD frame ! If the sample is empty, remove it
			if (!samp->dataLength) {
				e = M4_RemoveSample(the_file, i+1, k+1);
				if (e) return e;
			} else {
				e = M4_UpdateSample(the_file, i+1, k+1, samp);
				if (e) return e;
			}
			//and don't forget to delete the sample
			M4_DeleteSample(&samp);
		}
	}

	//remove the track ref from any "tref" atom in all tracks (except the one to delete ;)
	for (i=0; i<ChainGetCount(movie->moov->trackList); i++) {
		trak = (TrackAtom*)ChainGetEntry(movie->moov->trackList, i);
		if (trak == the_trak) continue;
		if (! trak->References || ! ChainGetCount(trak->References->atomList)) continue;
		for (j=0; j<ChainGetCount(trak->References->atomList); j++) {
			tref = (TrackReferenceTypeAtom*)ChainGetEntry(trak->References->atomList, j);
			found = 0;
			for (k=0; k<tref->trackIDCount; k++) {
				if (tref->trackIDs[k] == the_trak->Header->trackID) found++;
			}
			if (!found) continue;
			//no more refs, remove this ref_type
			if (found == tref->trackIDCount) {
				DelAtom((Atom *)tref);
				ChainDeleteEntry(trak->References->atomList, j);
				j--;
			} else {
				newRefs = (u32*)malloc(sizeof(u32) * (tref->trackIDCount - found));
				found = 0;
				for (k = 0; k < tref->trackIDCount; k++) {
					if (tref->trackIDs[k] != the_trak->Header->trackID) {
						newRefs[k-found] = tref->trackIDs[k];
					} else {
						found++;
					}
				}
				free(tref->trackIDs);
				tref->trackIDs = newRefs;
				tref->trackIDCount -= found;
			}
		}
		//a little opt: remove the ref atom if empty...
		if (! ChainGetCount(trak->References->atomList)) {
			DelAtom((Atom *)trak->References);
			trak->References = NULL;
		}
	}

	//delete the track
	DelAtom((Atom *)the_trak);


	return M4OK;
}


M4Err M4_SetCopyright(M4File *the_file, const char *threeCharCode, char *notice)
{
	M4Movie *mov;
	M4Err e;
	CopyrightAtom *ptr;
	UserDataMap *map;
	u32 count, i;
	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType);
	
	mov = (M4Movie *)the_file;
	e = CanAccessMovie(mov, M4_OPEN_WRITE);
	if (e) return e;

	if (!notice || !threeCharCode) return M4BadParam;

	if (!mov->moov->udta) {
		e = moov_AddAtom(mov->moov, CreateAtom(UserDataAtomType));
		if (e) return e;
	}
	map = udta_getEntry(mov->moov->udta, CopyrightAtomType);
	
	if (map) {
		//try to find one in our language...
		count = ChainGetCount(map->atomList);
		for (i=0; i<count; i++) {
			ptr = (CopyrightAtom*)ChainGetEntry(map->atomList, i);
			if (!strcmp(threeCharCode, (const char *) ptr->packedLanguageCode)) {
				free(ptr->notice);
				ptr->notice = (char*)malloc(sizeof(char) * (strlen(notice) + 1));
				strcpy(ptr->notice, notice);
				return M4OK;
			}
		}
	}
	//nope, create one
	ptr = (CopyrightAtom *)CreateAtom(CopyrightAtomType);

	memcpy(ptr->packedLanguageCode, threeCharCode, 4);
	ptr->notice = (char*)malloc(sizeof(char) * (strlen(notice)+1));
	strcpy(ptr->notice, notice);
	return udta_AddAtom(mov->moov->udta, (Atom *) ptr);
}

M4Err M4_DeleteCopyright(M4File *the_file, u32 index)
{
	M4Movie *mov;
	M4Err e;
	CopyrightAtom *ptr;
	UserDataMap *map;
	u32 count;
	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType);
	
	mov = (M4Movie *)the_file;
	e = CanAccessMovie(mov, M4_OPEN_WRITE);
	if (e) return e;

	if (!index) return M4BadParam;
	if (!mov->moov->udta) return M4OK;

	map = udta_getEntry(mov->moov->udta, CopyrightAtomType);
	if (!map) return M4OK;

	count = ChainGetCount(map->atomList);
	if (index>count) return M4BadParam;

	ptr = (CopyrightAtom*)ChainGetEntry(map->atomList, index-1);
	if (ptr) {
		ChainDeleteEntry(map->atomList, index-1);
		if (ptr->notice) free(ptr->notice);
		free(ptr);
	}
	/*last copyright, remove*/
	if (!ChainGetCount(map->atomList)) {
		ChainDeleteItem(mov->moov->udta->recordList, map);
		DeleteChain(map->atomList);
		free(map);
	}
	return M4OK;
}


M4Err M4_SetWatermark(M4File *the_file, bin128 UUID, u8* data, u32 length)
{
	M4Movie *mov;
	M4Err e;
	WatermarkAtom *ptr;
	UserDataMap *map;
	u32 count, i;
	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType);
	
	mov = (M4Movie *)the_file;
	e = CanAccessMovie(mov, M4_OPEN_WRITE);
	if (e) return e;

	if (!mov->moov->udta) {
		e = moov_AddAtom(mov->moov, CreateAtom(UserDataAtomType));
		if (e) return e;
	}
	
	map = udta_getEntry(mov->moov->udta, ExtendedAtomType);
	if (map) {
		count = ChainGetCount(map->atomList);
		for(i=0; i<count; i++){
			ptr = (WatermarkAtom *)ChainGetEntry(map->atomList, i);
			
			if(!memcmp(UUID, ptr->uuid, 16)){
				free(ptr->data);
				ptr->data = (char*)malloc(length);
				memcpy(ptr->data, data, length);
				ptr->dataSize = length;
				return M4OK;
			}
		}
	}
	//nope, create one
	ptr = (WatermarkAtom *)CreateAtom(ExtendedAtomType);

	memcpy(ptr->uuid, UUID, 16);
	ptr->data = (char*)malloc(length);
	memcpy(ptr->data, data, length);
	ptr->dataSize = length;
	return udta_AddAtom(mov->moov->udta, (Atom *) ptr);
}

//set the interleaving time of media data (INTERLEAVED mode only)
//InterleaveTime is in MovieTimeScale
M4Err M4_SetInterleavingTime(M4File *the_file, u32 InterleaveTime)
{
	M4Err e;
	M4Movie *mov = (M4Movie *)the_file;

	e = CanAccessMovie(mov, M4_OPEN_WRITE);
	if (e) return e;

	if (!InterleaveTime) return M4BadParam;
	mov->interleavingTime = InterleaveTime;
	return M4OK;
}

u32 M4_GetInterleavingTime(M4File *the_file)
{
	M4Movie *mov = (M4Movie *)the_file;
	return mov ? mov->interleavingTime : 0;
}

//set the storage mode of a file (FLAT, STREAMABLE, INTERLEAVED)
u8 M4_GetStorageMode(M4File *the_file)
{
	M4Movie *movie = (M4Movie *)the_file;
	return movie->storageMode;
}




//use a compact track version for sample size. This is not usually recommended 
//except for speech codecs where the track has a lot of small samples
//compaction is done automatically while writing based on the track's sample sizes
M4Err M4_UseCompactSize(M4File *the_file, u32 trackNumber, u8 CompactionOn)
{
	TrackAtom *trak;
	u32 i, size;
	SampleSizeAtom *stsz;
	M4Err e;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	
	if (!trak->Media || !trak->Media->information
		|| !trak->Media->information->sampleTable || !trak->Media->information->sampleTable->SampleSize)
		return M4InvalidMP4File;

	stsz = trak->Media->information->sampleTable->SampleSize;
	
	//switch to regular table
	if (!CompactionOn) {
		if (stsz->type == SampleSizeAtomType) return M4OK;
		stsz->type = SampleSizeAtomType;
		//invalidate the sampleSize and recompute it
		stsz->sampleSize = 0;
		if (!stsz->sampleCount) return M4OK;
		//if the table is empty we can only assume the track is empty (no size indication)
		if (!stsz->sizes) return M4OK;
		size = stsz->sizes[0];
		//check whether the sizes are all the same or not
		for (i=1; i<stsz->sampleCount; i++) {
			if (size != stsz->sizes[i]) {
				size = 0;
				break;
			}
		}
		if (size) {
			free(stsz->sizes);
			stsz->sizes = NULL;
			stsz->sampleSize = size;
		}
		return M4OK;
	}

	//switch to compact table
	if (stsz->type == CompactSampleSizeAtomType) return M4OK;
	//fill the table. Although it seems weird , this is needed in case of edition
	//after the function is called. NOte however than we force regular table
	//at write time if all samples are of same size
	if (stsz->sampleSize) {
		//this is a weird table indeed ;)
		if (stsz->sizes) free(stsz->sizes);
		stsz->sizes = malloc(sizeof(u32)*stsz->sampleCount);
		memset(stsz->sizes, stsz->sampleSize, sizeof(u32));
	}
	//set the SampleSize to 0 while the file is open
	stsz->sampleSize = 0;
	stsz->type = CompactSampleSizeAtomType;
	return M4OK;
}

//checks the file has no data for file type and signature modifs
static M4Err CheckNoData(M4Movie *movie)
{
	u32 size;

	//handle capture mode and deny if any sample has been written to the file
	if (movie->openMode != M4_OPEN_WRITE) return M4OK;
	size = 0;
	//TODO: add signature for J2K
	//add brand
	if (movie->brand) {
		SizeAtom((Atom *)movie->brand);
		size += (u32) movie->brand->size;
	}
	//add our 16 bytes of capture mdat (cf MovieCreate)
	size += 16;
	if (BS_GetPosition(movie->editFileMap->bs) != size) return M4BadParam;
	return M4OK;
}


//update file type and signature atoms in capture mode
static M4Err UpdateFirstAtoms(M4Movie *movie)
{
	M4Err e;
	if (movie->openMode != M4_OPEN_WRITE) return M4OK;

	//rewind
	BS_Seek(movie->editFileMap->bs, 0);
	movie->editFileMap->curPos = 0;

	//TODO: store signature for J2K files

	//store brand
	e = SizeAtom((Atom *)movie->brand);
	if (e) return e;
	e = WriteAtom((Atom *)movie->brand, movie->editFileMap->bs);
	if (e) return e;

	//then write 16 bytes blank data for mdat header
	BS_WriteInt(movie->editFileMap->bs, 0, 128);
	return M4OK;
}


M4Err M4_SetMovieVersionInfo(M4File *the_file, u32 MajorBrand, u32 MinorVersion)
{
	u32 i, *p;
	M4Movie *movie = (M4Movie *)the_file;
	M4Err e;

	if (!MajorBrand) return M4BadParam;
	
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;

	e = CheckNoData(movie);
	if (e) return e;


	if (!movie->brand) movie->brand = (FileTypeAtom *) CreateAtom(FileTypeAtomType);
	
	movie->brand->majorBrand = MajorBrand;
	movie->brand->minorVersion = MinorVersion;

	if (!movie->brand->altBrand) {
		movie->brand->altBrand = malloc(sizeof(u32));
		movie->brand->altBrand[0] = MajorBrand;
		movie->brand->altCount = 1;
		return UpdateFirstAtoms(movie);
	}

	//if brand already present don't change anything
	for (i=0; i<movie->brand->altCount; i++) {
		if (movie->brand->altBrand[i] == MajorBrand) return M4OK;
	}
	p = malloc(sizeof(u32)*(movie->brand->altCount + 1));
	if (!p) return M4OutOfMem;
	memcpy(p, movie->brand->altBrand, sizeof(u32)*movie->brand->altCount);
	p[movie->brand->altCount] = MajorBrand;
	movie->brand->altCount += 1;
	free(movie->brand->altBrand);
	movie->brand->altBrand = p;
	//rewrite begining of the file
	return UpdateFirstAtoms(movie);
}

M4Err M4_ModifyAlternateBrand(M4File *the_file, u32 Brand, u8 AddIt)
{
	u32 i, k, *p;
	M4Err e;
	M4Movie *movie = (M4Movie *)the_file;
	
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	if (!Brand || !movie->brand) return M4BadParam;

	e = CheckNoData(movie);
	if (e) return e;

	//do not mofify major one
	if (!AddIt && movie->brand->majorBrand == Brand) return M4OK;

	if (!AddIt && movie->brand->altCount == 1) {
		//fixes it in case
		movie->brand->altBrand[0] = movie->brand->majorBrand;
		return M4OK;
	}
	//check for the brand
	for (i=0; i<movie->brand->altCount; i++) {
		if (movie->brand->altBrand[i] == Brand) goto found;
	}
	//Not found
	if (!AddIt) return M4OK;
	//add it
	p = malloc(sizeof(u32)*(movie->brand->altCount + 1));
	if (!p) return M4OutOfMem;
	memcpy(p, movie->brand->altBrand, sizeof(u32)*movie->brand->altCount);
	p[movie->brand->altCount] = Brand;
	movie->brand->altCount += 1;
	free(movie->brand->altBrand);
	movie->brand->altBrand = p;
	//rewrite begining
	return UpdateFirstAtoms(movie);

found:

	//found
	if (AddIt) return M4OK;
	assert(movie->brand->altCount>1);

	//remove it
	p = malloc(sizeof(u32)*(movie->brand->altCount - 1));
	if (!p) return M4OutOfMem;
	k = 0;
	for (i=0; i<movie->brand->altCount; i++) {
		if (movie->brand->altBrand[i] == Brand) continue;
		else {
			p[k] = movie->brand->altBrand[i];
			k++;
		}
	}
	movie->brand->altCount -= 1;
	free(movie->brand->altBrand);
	movie->brand->altBrand = p;
	//rewrite begining
	return UpdateFirstAtoms(movie);
}


M4Err M4_SetSamplePaddingBits(M4File *the_file, u32 trackNumber, u32 sampleNumber, u8 NbBits)
{
	TrackAtom *trak;
	M4Err e;

	e = CanAccessMovie((M4Movie *) the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || NbBits > 7) return M4BadParam;
	
	//set Padding info
	return stbl_SetPaddingBits(trak->Media->information->sampleTable, sampleNumber, NbBits);
}


M4Err M4_RemoveUserDataItem(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex)
{
	UserDataMap *map;
	Atom *a;
	u32 i;
	bin128 t;
	M4Err e;
	TrackAtom *trak;
	UserDataAtom *udta;

	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	if (UserDataType == ExtendedAtomType) UserDataType = 0;
	memset(t, 1, 16);

	if (trackNumber) {
		trak = GetTrackFromFile(the_file, trackNumber);
		if (!trak) return M4BadParam;
		udta = trak->udta;
	} else {
		udta = movie->moov->udta;
	}
	if (!udta) return M4BadParam;
	if (!UserDataIndex) return M4BadParam;

	for (i=0; i<ChainGetCount(udta->recordList); i++) {
		map = ChainGetEntry(udta->recordList, i);
		if ((map->atomType == ExtendedAtomType)  && !memcmp(map->uuid, UUID, 16)) goto found;
		else if (map->atomType == UserDataType) goto found;
	}
	//not found
	return M4OK;

found:

	if (UserDataIndex > ChainGetCount(map->atomList) ) return M4BadParam;
	//delete the atom
	a = ChainGetEntry(map->atomList, UserDataIndex-1);
	
	ChainDeleteEntry(map->atomList, UserDataIndex-1);
	DelAtom(a);

	//remove the map if empty
	if (!ChainGetCount(map->atomList)) {
		ChainDeleteEntry(udta->recordList, i);
		DeleteAtomList(map->atomList);
		free(map);
	}
	//but we keep the UDTA no matter what
	return M4OK;
}

M4Err M4_RemoveUserData(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID)
{
	UserDataMap *map;
	u32 i;
	M4Err e;
	bin128 t;
	TrackAtom *trak;
	UserDataAtom *udta;

	M4Movie *movie = (M4Movie *)the_file;

	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	if (UserDataType == ExtendedAtomType) UserDataType = 0;
	memset(t, 1, 16);

	if (trackNumber) {
		trak = GetTrackFromFile(the_file, trackNumber);
		if (!trak) return M4BadParam;
		udta = trak->udta;
	} else {
		udta = movie->moov->udta;
	}
	if (!udta) return M4BadParam;

	for (i=0; i<ChainGetCount(udta->recordList); i++) {
		map = ChainGetEntry(udta->recordList, i);
		if ((map->atomType == ExtendedAtomType)  && !memcmp(map->uuid, UUID, 16)) goto found;
		else if (map->atomType == UserDataType) goto found;
	}
	//not found
	return M4OK;

found:

	ChainDeleteEntry(udta->recordList, i);
	DeleteAtomList(map->atomList);
	free(map);

	//but we keep the UDTA no matter what
	return M4OK;
}

M4Err M4_AddUserData(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, char *data, u32 DataLength)
{
	UnknownAtom *a;
	bin128 t;
	M4Err e;
	TrackAtom *trak;
	UserDataAtom *udta;

	M4Movie *movie = (M4Movie *)the_file;
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	
	if (UserDataType == ExtendedAtomType) UserDataType = 0;

	if (trackNumber) {
		trak = GetTrackFromFile(the_file, trackNumber);
		if (!trak) return M4BadParam;
		if (!trak->udta) trak_AddAtom(trak, CreateAtom(UserDataAtomType));
		udta = trak->udta;
	} else {
		if (!movie->moov->udta) moov_AddAtom(movie->moov, CreateAtom(UserDataAtomType));
		udta = movie->moov->udta;
	}
	if (!udta) return M4OutOfMem;

	//create a default atom
	if (UserDataType) {
		a = (UnknownAtom *) CreateAtom(UserDataType);
	} else {
		a = (UnknownAtom *) CreateAtom(ExtendedAtomType);
		memcpy(a->uuid, UUID, 16);
	}

	memset(t, 1, 16);
	if ( (a->type != ExtendedAtomType) && memcmp(a->uuid, t, 16)) {
		DelAtom((Atom *)a);
		return M4BadParam;
	}
	a->data = malloc(sizeof(char)*DataLength);
	memcpy(a->data, data, DataLength);
	a->dataSize = DataLength;

	return udta_AddAtom(udta, (Atom *) a);
}



void M4_DisableBrand(M4File *the_file)
{
	M4Movie *movie = (M4Movie *)the_file;
	if (CanAccessMovie((M4Movie *) the_file, M4_OPEN_WRITE)) return;

	if (movie->brand) DelAtom((Atom *) movie->brand);
	movie->brand = NULL;
}



M4Err M4_AddSampleFragment(M4File *the_file, u32 trackNumber, u32 sampleNumber, u16 FragmentSize)
{
	M4Err e;
	TrackAtom *trak;

	e = CanAccessMovie((M4Movie *) the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !sampleNumber || !FragmentSize) return M4BadParam;

	//set Padding info
	return stbl_AddSampleFragment(trak->Media->information->sampleTable, sampleNumber, FragmentSize);
}


M4Err M4_RemoveSampleFragment(M4File *the_file, u32 trackNumber, u32 sampleNumber)
{
	TrackAtom *trak;
	M4Err e;

	e = CanAccessMovie((M4Movie *) the_file, M4_OPEN_WRITE);
	if (e) return e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//set Padding info
	return stbl_RemoveSampleFragments(trak->Media->information->sampleTable, sampleNumber);
}





M4Err M4_NewUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 entry_type, bin128 entry_UUID, char *URLname, char *URNname, UnknownStreamDescription *udesc, u32 *outDescriptionIndex)
{
	TrackAtom *trak;
	M4Err e;
	u32 dataRefIndex;
	TrackReferenceTypeAtom *dpnd;
	TrackReferenceAtom *tref;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !udesc) return M4BadParam;

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

	if (trak->Media->handler->type==M4_VisualMediaType) {
		GenericVisualSampleEntryAtom *entry;
		//create a new entry
		entry = (GenericVisualSampleEntryAtom*) CreateAtom(GenericVisualSampleEntryAtomType);
		if (!entry) return M4OutOfMem;

		if (!entry_type) {
			entry->EntryType = ExtendedAtomType;
			memcpy(entry->uuid, entry_UUID, sizeof(bin128));
		} else {
			entry->EntryType = entry_type;
		}
		entry->dataReferenceIndex = dataRefIndex;
		entry->vendor = udesc->vendor_code;
		entry->version = udesc->version;
		entry->revision = udesc->revision;
		entry->temporal_quality = udesc->temporal_quality;
		entry->spacial_quality = udesc->spacial_quality;
		entry->Width = udesc->width;
		entry->Height = udesc->height;
		entry->bit_depth = udesc->bitsPerSample;
		strcpy(entry->compressor_name, udesc->szCompressorName);
		entry->color_table_index = -1;
		entry->frames_per_sample = 1;
		entry->horiz_res = udesc->h_res;
		entry->vert_res = udesc->v_res;
		if (udesc->extension_buf && udesc->extension_buf_size) {
			entry->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!entry->data) {
				DelAtom((Atom *) entry);
				return M4OutOfMem;
			}
			memcpy(entry->data, udesc->extension_buf, udesc->extension_buf_size);
			entry->data_size = udesc->extension_buf_size;
		}
		e = ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, entry);
	}
	else if (trak->Media->handler->type==M4_AudioMediaType) {
		GenericAudioSampleEntryAtom *gena;
		//create a new entry
		gena = (GenericAudioSampleEntryAtom*) CreateAtom(GenericAudioSampleEntryAtomType);
		if (!gena) return M4OutOfMem;

		if (!entry_type) {
			gena->EntryType = ExtendedAtomType;
			memcpy(gena->uuid, entry_UUID, sizeof(bin128));
		} else {
			gena->EntryType = entry_type;
		}
		gena->dataReferenceIndex = dataRefIndex;
		gena->vendor = udesc->vendor_code;
		gena->version = udesc->version;
		gena->revision = udesc->revision;
		gena->bitspersample = udesc->bitsPerSample;
		gena->channel_count = udesc->NumChannels;
		gena->samplerate_hi = udesc->SampleRate>>16;
		gena->samplerate_lo = udesc->SampleRate & 0xFF;

		if (udesc->extension_buf && udesc->extension_buf_size) {
			gena->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!gena->data) {
				DelAtom((Atom *) gena);
				return M4OutOfMem;
			}
			memcpy(gena->data, udesc->extension_buf, udesc->extension_buf_size);
			gena->data_size = udesc->extension_buf_size;
		}
		e = ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, gena);
	}
	else {
		GenericMediaSampleEntryAtom *genm;
		//create a new entry
		genm = (GenericMediaSampleEntryAtom*) CreateAtom(GenericMediaSampleEntryAtomType);
		if (!genm) return M4OutOfMem;

		if (!entry_type) {
			genm->EntryType = ExtendedAtomType;
			memcpy(genm->uuid, entry_UUID, sizeof(bin128));
		} else {
			genm->EntryType = entry_type;
		}
		genm->dataReferenceIndex = dataRefIndex;
		if (udesc->extension_buf && udesc->extension_buf_size) {
			genm->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!genm->data) {
				DelAtom((Atom *) genm);
				return M4OutOfMem;
			}
			memcpy(genm->data, udesc->extension_buf, udesc->extension_buf_size);
			genm->data_size = udesc->extension_buf_size;
		}
		e = ChainAddEntry(trak->Media->information->sampleTable->SampleDescription->atomList, genm);
	}
	*outDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
	return e;
}

//use carefully. Very usefull when you made a lot of changes (IPMP, IPI, OCI, ...)
//THIS WILL REPLACE THE WHOLE DESCRIPTOR ...
M4Err M4_ChangeUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, UnknownStreamDescription *udesc)
{
	TrackAtom *trak;
	M4Err e;
	GenericVisualSampleEntryAtom *entry;

	e = CanAccessMovie((M4Movie *)the_file, M4_OPEN_WRITE);
	if (e) return e;
	
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media || !StreamDescriptionIndex) return M4BadParam;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex-1);
	if (!entry) return M4BadParam;
	if (entry->type == GenericVisualSampleEntryAtomType) {
		entry->vendor = udesc->vendor_code;
		entry->version = udesc->version;
		entry->revision = udesc->revision;
		entry->temporal_quality = udesc->temporal_quality;
		entry->spacial_quality = udesc->spacial_quality;
		entry->Width = udesc->width;
		entry->Height = udesc->height;
		entry->bit_depth = udesc->bitsPerSample;
		strcpy(entry->compressor_name, udesc->szCompressorName);
		entry->color_table_index = -1;
		entry->frames_per_sample = 1;
		entry->horiz_res = udesc->h_res;
		entry->vert_res = udesc->v_res;
		if (entry->data) free(entry->data);
		entry->data = NULL;
		entry->data_size = 0;
		if (udesc->extension_buf && udesc->extension_buf_size) {
			entry->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!entry->data) {
				DelAtom((Atom *) entry);
				return M4OutOfMem;
			}
			memcpy(entry->data, udesc->extension_buf, udesc->extension_buf_size);
			entry->data_size = udesc->extension_buf_size;
		}
		return M4OK;
	} else if (entry->type == GenericAudioSampleEntryAtomType) {
		GenericAudioSampleEntryAtom *gena = (GenericAudioSampleEntryAtom *)entry;
		gena->vendor = udesc->vendor_code;
		gena->version = udesc->version;
		gena->revision = udesc->revision;
		gena->bitspersample = udesc->bitsPerSample;
		gena->channel_count = udesc->NumChannels;
		gena->samplerate_hi = udesc->SampleRate>>16;
		gena->samplerate_lo = udesc->SampleRate & 0xFF;
		if (gena->data) free(gena->data);
		gena->data = NULL;
		gena->data_size = 0;

		if (udesc->extension_buf && udesc->extension_buf_size) {
			gena->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!gena->data) {
				DelAtom((Atom *) gena);
				return M4OutOfMem;
			}
			memcpy(gena->data, udesc->extension_buf, udesc->extension_buf_size);
			gena->data_size = udesc->extension_buf_size;
		}
		return M4OK;
	} else if (entry->type == GenericMediaSampleEntryAtomType) {
		GenericMediaSampleEntryAtom *genm = (GenericMediaSampleEntryAtom *)entry;
		if (genm->data) free(genm->data);
		genm->data = NULL;
		genm->data_size = 0;

		if (udesc->extension_buf && udesc->extension_buf_size) {
			genm->data = malloc(sizeof(unsigned char) * udesc->extension_buf_size);
			if (!genm->data) {
				DelAtom((Atom *) genm);
				return M4OutOfMem;
			}
			memcpy(genm->data, udesc->extension_buf, udesc->extension_buf_size);
			genm->data_size = udesc->extension_buf_size;
		}
		return M4OK;
	}
	return M4BadParam;
}



//sets a track reference
M4Err M4_SetTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 ReferencedTrackID)
{
	M4Err e;
	TrackAtom *trak;
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *dpnd;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//no tref, create one
	tref = trak->References;
	if (!tref) {
		tref = (TrackReferenceAtom *) CreateAtom(TrackReferenceAtomType);
		e = trak_AddAtom(trak, (Atom *) tref);
		if (e) return e;
	}
	//find a ref of the given type
	e = Track_FindRef(trak, referenceType, &dpnd);
	if (!dpnd) {
		dpnd = (TrackReferenceTypeAtom *) CreateAtom(referenceType);
		e = tref_AddAtom(tref, (Atom *)dpnd);
		if (e) return e;
	}
	//add the ref
	return reftype_AddRefTrack(dpnd, ReferencedTrackID, NULL);
}

//removes a track reference
M4Err M4_RemoveTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 ReferenceIndex)
{
	M4Err e;
	TrackAtom *trak;
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *dpnd, *tmp;
	u32 i, k, *newIDs;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !ReferenceIndex) return M4BadParam;

	//no tref, nothing to remove
	tref = trak->References;
	if (!tref) return M4OK;
	//find a ref of the given type otherwise return
	e = Track_FindRef(trak, referenceType, &dpnd);
	if (e || !dpnd) return M4OK;
	//remove the ref
	if (ReferenceIndex > dpnd->trackIDCount) return M4BadParam;
	//last one
	if (dpnd->trackIDCount==1) {
		for (i=0; i<ChainGetCount(tref->atomList); i++) {
			tmp = ChainGetEntry(tref->atomList, i);
			if (tmp==dpnd) {
				ChainDeleteEntry(tref->atomList, i);
				DelAtom((Atom *) dpnd);
				return M4OK;
			}
		}
	}
	k = 0;
	newIDs = malloc(sizeof(u32)*(dpnd->trackIDCount-1));
	for (i=0; i<dpnd->trackIDCount; i++) {
		if (i+1 != ReferenceIndex) {
			newIDs[k] = dpnd->trackIDs[i];
			k++;
		}
	}
	free(dpnd->trackIDs);
	dpnd->trackIDCount -= 1;
	dpnd->trackIDs = newIDs;
	return M4OK;
}


//changes track ID
M4Err M4_ChangeTrackID(M4File *the_file, u32 trackNumber, u32 trackID)
{
	TrackReferenceTypeAtom *ref;
	M4Movie *mov;
	TrackAtom *trak, *a_trak;
	u32 i, j, k;

	trak = GetTrackFromFile(the_file, trackNumber);
	mov = GetMovie(the_file);
	if (trak && (trak->Header->trackID==trackID)) return M4OK;
	a_trak = GetTrackFromID(mov->moov, trackID);
	if (!mov || !trak || a_trak) return M4BadParam;

	if (mov->moov->mvhd->nextTrackID<=trackID)
		mov->moov->mvhd->nextTrackID = trackID;

	/*rewrite all dependencies*/
	for (i=0; i<ChainGetCount(mov->moov->trackList); i++) {
		a_trak = ChainGetEntry(mov->moov->trackList, i);
		if (!a_trak->References) continue;
		for (j=0; j<ChainGetCount(a_trak->References->atomList); j++) {
			ref = ChainGetEntry(a_trak->References->atomList, j);
			for (k=0; k<ref->trackIDCount; k++) {
				if (ref->trackIDs[k]==trak->Header->trackID) {
					ref->trackIDs[k] = trackID;
					break;
				}
			}
		}
	}
	
	/*and update IOD if any*/
	if (mov->moov->iods && mov->moov->iods->descriptor) {
		u32 i;
		M4F_ObjectDescriptor *od = (M4F_ObjectDescriptor *)mov->moov->iods->descriptor;
		for (i=0; i<ChainGetCount(od->ES_ID_IncDescriptors); i++) {
			ES_ID_Inc *inc = ChainGetEntry(od->ES_ID_IncDescriptors, i);
			if (inc->trackID==trak->Header->trackID) inc->trackID = trackID;
		}
	}

	trak->Header->trackID = trackID;
	return M4OK;
}


M4Err M4_SetSampleCTSOffset(M4File *the_file, u32 trackNumber, u32 sample_number, u32 offset)
{
	dttsEntry *ent;
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (!trak->Media->information->sampleTable->CompositionOffset) return M4BadParam;
	if (!trak->Media->information->sampleTable->CompositionOffset->unpack_mode) return M4BadParam;
	ent = ChainGetEntry(trak->Media->information->sampleTable->CompositionOffset->entryList, sample_number - 1);
	if (!ent) return M4BadParam;
	ent->decodingOffset = offset;
	return M4OK;
}

M4Err M4_DeleteCTSTable(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (!trak->Media->information->sampleTable->CompositionOffset) return M4OK;
	
	DelAtom((Atom *)trak->Media->information->sampleTable->CompositionOffset);
	trak->Media->information->sampleTable->CompositionOffset = NULL;
	return M4OK;
}

M4Err M4_SetCTSPackMode(M4File *the_file, u32 trackNumber, Bool unpack, Bool adjust_cts)
{
	M4Err stbl_repackCTS(CompositionOffsetAtom *ctts, Bool adjust_cts);

	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (unpack) {
		if (!trak->Media->information->sampleTable->CompositionOffset) trak->Media->information->sampleTable->CompositionOffset = (CompositionOffsetAtom *) CreateAtom(CompositionOffsetAtomType);
		if (ChainGetCount(trak->Media->information->sampleTable->CompositionOffset->entryList)) return M4BadParam;
		trak->Media->information->sampleTable->CompositionOffset->unpack_mode = 1;
		return M4OK;
	}
	if (!trak->Media->information->sampleTable->CompositionOffset) return M4OK;
	return stbl_repackCTS(trak->Media->information->sampleTable->CompositionOffset, adjust_cts);
}


M4Err M4_SetTrackVideoInfo(M4File *the_file, u32 trackNumber, u32 width, u32 height, s32 translation_x, s32 translation_y, s16 layer)
{
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Header) return M4BadParam;
	trak->Header->width = width;
	trak->Header->height = height;
	trak->Header->matrix[6] = translation_x;
	trak->Header->matrix[7] = translation_y;
	trak->Header->layer = layer;
	return M4OK;
}

M4Err M4_SetTrackName(M4File *the_file, u32 trackNumber, char *name)
{
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (trak->name) free (trak->name);
	trak->name = NULL;
	if (name) trak->name = strdup(name);
	return M4OK;
}
const char *M4_GetTrackName(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return NULL;
	return trak->name;
}


M4Err M4_MovieWrite(M4File *the_file)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;
	if (movie == NULL) return M4InvalidMP4File;

	e = M4OK;
	//write our movie to the file
	if (movie->openMode != M4_OPEN_READ) {
		M4_GetDuration(the_file);
#ifndef	M4_ISO_NO_FRAGMENTS
		//movie fragment mode, just store the fragment
		if ( (movie->openMode == M4_OPEN_WRITE) && (movie->FragmentsFlags & FRAG_WRITE_READY) ) {
			e = StoreFragment(movie);
		} else {
			e = WriteToFile(movie);
		}
#else 
		e = WriteToFile(movie);
#endif
	}
	return e;
}

M4Err M4_StoreMovieConfig(M4File *the_file, Bool remove_all)
{
	u32 i, count, len;
	char *data;
	BitStream *bs;
	bin128 binID;
	M4Movie *movie = (M4Movie *) the_file;
	if (movie == NULL) return M4BadParam;

	M4_RemoveUserData(the_file, 0, FOUR_CHAR_INT('G','P','A','C'), binID);
	count = M4_GetTrackCount(the_file);
	for (i=0; i<count; i++) M4_RemoveUserData(the_file, i+1, FOUR_CHAR_INT('G','P','A','C'), binID);

	if (remove_all) return M4OK;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	/*update movie: storage mode and interleaving type*/
	BS_WriteInt(bs, 0xFE, 8); /*marker*/
	BS_WriteInt(bs, movie->storageMode, 8);
	BS_WriteInt(bs, movie->interleavingTime, 32);
	BS_GetContent(bs, (unsigned char **) &data, &len);
	DeleteBitStream(bs);
	M4_AddUserData(the_file, 0, FOUR_CHAR_INT('G','P','A','C'), binID, data, len);
	free(data);
	/*update tracks: interleaving group/priority and track edit name*/
	for (i=0; i<count; i++) {
		u32 j;
		TrackAtom *trak = GetTrackFromFile(the_file, i+1);
		bs = NewBitStream(NULL, 0, BS_WRITE);
		BS_WriteInt(bs, 0xFE, 8);	/*marker*/
		BS_WriteInt(bs, trak->Media->information->sampleTable->groupID, 32);
		BS_WriteInt(bs, trak->Media->information->sampleTable->trackPriority, 32);
		len = trak->name ? strlen(trak->name) : 0;
		BS_WriteInt(bs, len, 32);
		for (j=0; j<len; j++) BS_WriteInt(bs, trak->name[j], 8);
		BS_GetContent(bs, (unsigned char **) &data, &len);
		DeleteBitStream(bs);
		M4_AddUserData(the_file, i+1, FOUR_CHAR_INT('G','P','A','C'), binID, data, len);
		free(data);
	}
	return M4OK;
}


M4Err M4_LoadMovieConfig(M4File *the_file)
{
	u32 i, count, len;
	char *data;
	BitStream *bs;
	Bool found_cfg;
	bin128 binID;
	M4Movie *movie = (M4Movie *) the_file;
	if (movie == NULL) return M4BadParam;

	found_cfg = 0;
	/*restore movie*/
	count = M4_GetUserDataItemCount(the_file, 0, FOUR_CHAR_INT('G','P','A','C'), binID);
	for (i=0; i<count; i++) {
		data = NULL;
		M4_GetUserDataItem(the_file, 0, FOUR_CHAR_INT('G','P','A','C'), binID, i+1, &data, &len);
		if (!data) continue;
		/*check marker*/
		if ((unsigned char) data[0] != 0xFE) {
			free(data);
			continue;
		}
		bs = NewBitStream(data, len, BS_READ);
		BS_ReadInt(bs, 8);	/*marker*/
		movie->storageMode = BS_ReadInt(bs, 8);
		movie->interleavingTime = BS_ReadInt(bs, 32);
		DeleteBitStream(bs);
		free(data);
		found_cfg = 1;
		break;
	}

	for (i=0; i<M4_GetTrackCount(the_file); i++) {
		u32 j;
		TrackAtom *trak = GetTrackFromFile(the_file, i+1);
		count = M4_GetUserDataItemCount(the_file, i+1, FOUR_CHAR_INT('G','P','A','C'), binID);
		for (j=0; j<count; j++) {
			data = NULL;
			M4_GetUserDataItem(the_file, i+1, FOUR_CHAR_INT('G','P','A','C'), binID, j+1, &data, &len);
			if (!data) continue;
			/*check marker*/
			if ((unsigned char) data[0] != 0xFE) {
				free(data);
				continue;
			}
			bs = NewBitStream(data, len, BS_READ);
			BS_ReadInt(bs, 8);	/*marker*/
			trak->Media->information->sampleTable->groupID = BS_ReadInt(bs, 32);
			trak->Media->information->sampleTable->trackPriority = BS_ReadInt(bs, 32);
			len = BS_ReadInt(bs, 32);
			if (len) {
				u32 k;
				trak->name = malloc(sizeof(char)*(len+1));
				for (k=0;k<len;k++) trak->name[k] = BS_ReadInt(bs, 8);
				trak->name[k] = 0;
			}
			DeleteBitStream(bs);
			free(data);
			found_cfg = 1;
			break;
		}
	}
	return found_cfg ? M4OK : M4NotSupported;
}

M4Err M4_SetTempDir(M4File *file, const char *tmpdir)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) file;
	if (!file) return M4BadParam;
	e = CanAccessMovie(movie, M4_OPEN_WRITE);
	if (e) return e;
	if (!movie->editFileMap || BS_GetPosition(movie->editFileMap->bs)) return M4BadParam;

	DataMap_Delete(movie->editFileMap);
	movie->editFileMap = NULL;
	return DataMap_New("mp4_tmp_edit", tmpdir, DM_MODE_W, & movie->editFileMap);
}


#endif	//M4_READ_ONLY

