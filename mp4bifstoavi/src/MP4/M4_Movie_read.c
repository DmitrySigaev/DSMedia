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

//the only static var. Used to store any error happening while opening a movie
static M4Err MP4_API_IO_Err;

void M4SetLastError(M4File *the_file, M4Err error)
{
	M4Movie *mov;
	if (!the_file) {
		MP4_API_IO_Err = error;
		return;
	}
	mov = GetMovie(the_file);
	if (mov) mov->LastError = error;
}


M4Err M4_GetLastError(M4File *the_file)
{
	if (!the_file) return MP4_API_IO_Err;
	return GetMovie(the_file)->LastError;
}

u8 M4_GetOpenMode(M4File *the_file)
{
	if (!the_file) return 0;
	return ((M4Movie *)the_file)->openMode;
}

/**************************************************************
					Sample Manip
**************************************************************/

//creates a new empty sample
M4Sample *M4_NewSample()
{
	M4Sample *tmp = (M4Sample *) malloc(sizeof(M4Sample));
	if (!tmp) return NULL;
	tmp->CTS_Offset = tmp->DTS = 0;
	tmp->dataLength = 0;
	tmp->IsRAP = 0;
	tmp->data = NULL;
	return tmp;
}

//creates a new formatted sample
M4Sample *M4_NewFullSample(char *data, u32 dataLength, u32 DTS,	u32 CTS_Offset, u8 IsRAP)
{
	M4Sample *samp = M4_NewSample();
	if (!samp) return NULL;
	samp->data = data;
	samp->dataLength = dataLength;
	samp->CTS_Offset = CTS_Offset;
	samp->DTS = DTS;
	samp->IsRAP = IsRAP;
	return samp;
}

//delete a sample
void M4_DeleteSample(M4Sample **samp)
{
	if (! *samp) return;
	if ((*samp)->data && (*samp)->dataLength) free((*samp)->data);
	free(*samp);
	*samp = NULL;
}


/**************************************************************
					File Opening in streaming mode
			the file map is regular (through FILE handles)
**************************************************************/
M4Err M4_MovieOpenEx(const char *fileName, M4File **the_file, u64 *BytesMissing)
{
	M4Movie *movie;
	M4Err e;

	*BytesMissing = 0;
	*the_file = NULL;

	movie = M4NewMovie();
	if (!movie) return M4OutOfMem;

	movie->fileName = strdup(fileName);
	movie->openMode = M4_OPEN_READ;
	//do NOT use FileMapping on incomplete files
	e = DataMap_New(fileName, NULL, DM_MODE_R, &movie->movieFileMap);
	if (e) {
		DelMovie(movie);
		return e;
	}

#ifndef M4_READ_ONLY
	movie->editFileMap = NULL;
	movie->finalName = NULL;
#endif	//M4_READ_ONLY

	e = ParseMovieAtoms(movie, BytesMissing);
	if (e == M4UncompleteFile) {
		//if we have a moov, we're fine
		if (movie->moov) {
			*the_file = (M4File *)movie;
			return M4OK;
		}
		//if not, delete the movie
		DelMovie(movie);
		return e;
	} else if (e) {
		//if not, delete the movie
		DelMovie(movie);
		return e;
	}
	//OK, let's return
	*the_file = (M4File *)movie;
	return M4OK;
}

/**************************************************************
					File Reading
**************************************************************/

M4File *M4_MovieOpen(const char *fileName, u8 OpenMode)
{
	M4Movie *movie;

	MP4_API_IO_Err = M4OK;

	
	switch (OpenMode) {
	case M4_OPEN_READ_DUMP:
	case M4_OPEN_READ:
		movie = MovieOpen(fileName, OpenMode);
		break;

#ifndef M4_READ_ONLY
	
	case M4_OPEN_WRITE:
		movie = MovieCreate(fileName, OpenMode);
		break;
	case M4_OPEN_EDIT:
		movie = MovieOpen(fileName, OpenMode);
		break;
	case M4_WRITE_EDIT:
		OpenMode = M4_OPEN_EDIT;
		movie = MovieCreate(fileName, OpenMode);
		break;

#endif	//M4_READ_ONLY
		
	default:
		return NULL;
	}
	return (M4File *) movie;
}

M4Err M4_MovieClose(M4File *the_file)
{
	M4Err e;
	M4Movie *movie = (M4Movie *) the_file;
	if (movie == NULL) return M4InvalidMP4File;

	e = M4OK;

#ifndef M4_READ_ONLY
	
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
	
#endif	//M4_READ_ONLY

	//free and return;
	DelMovie(movie);
	return e;
}

//this funct is used for exchange files, where the iods contains an OD
Descriptor *M4_GetRootOD(M4File *the_file)
{
	Descriptor *desc;
	ObjectDescriptor *od;
	InitialObjectDescriptor *iod;
	M4F_ObjectDescriptor *m4_od;
	M4F_InitialObjectDescriptor *m4_iod;
	ESDescriptor *esd;
	ES_ID_Inc *inc;
	u32 i;
	u8 useIOD;

	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) {
		MP4_API_IO_Err = M4InvalidMP4File;
		return NULL;
	}
	if (!movie->moov) {
		movie->LastError = M4InvalidMP4File;
		return NULL;
	}
	if (!movie->moov->iods) return NULL;

	od = NULL;
	iod = NULL;

	switch (movie->moov->iods->descriptor->tag) {
	case MP4_OD_Tag:
		od = (ObjectDescriptor*)malloc(sizeof(ObjectDescriptor));
		od->ESDescriptors = NewChain();
		useIOD = 0;
		break;
	case MP4_IOD_Tag:
		iod = (InitialObjectDescriptor*)malloc(sizeof(InitialObjectDescriptor));
		iod->ESDescriptors = NewChain();
		useIOD = 1;
		break;
	default:
		return NULL;
	}
	
	//duplicate our descriptor
	movie->LastError = OD_DuplicateDescriptor((Descriptor *) movie->moov->iods->descriptor, &desc);
	if (movie->LastError) return NULL;

	if (!useIOD) {
		m4_od = (M4F_ObjectDescriptor *)desc;
		od->objectDescriptorID = m4_od->objectDescriptorID;
		od->extensionDescriptors = m4_od->extensionDescriptors;
		m4_od->extensionDescriptors = NULL;
		od->IPMPDescriptorPointers = m4_od->IPMPDescriptorPointers;
		m4_od->IPMPDescriptorPointers = NULL;
		od->OCIDescriptors = m4_od->OCIDescriptors;
		m4_od->OCIDescriptors = NULL;
		od->URLString = m4_od->URLString;
		m4_od->URLString = NULL;
		od->tag = ObjectDescriptor_Tag;
		//then recreate the desc in Inc
		for (i = 0; i < ChainGetCount(m4_od->ES_ID_IncDescriptors); i++) {
			inc = (ES_ID_Inc*)ChainGetEntry(m4_od->ES_ID_IncDescriptors, i);
			movie->LastError = GetESDForTime(movie->moov, inc->trackID, 0, &esd);
			if (!movie->LastError) movie->LastError = ChainAddEntry(od->ESDescriptors, esd);
			if (movie->LastError) {
				OD_DeleteDescriptor(&desc);
				OD_DeleteDescriptor((Descriptor **) &od);
				return NULL;
			}
		}
		OD_DeleteDescriptor(&desc);
		return (Descriptor *)od;
	} else {
		m4_iod = (M4F_InitialObjectDescriptor *)desc;
		iod->objectDescriptorID = m4_iod->objectDescriptorID;
		iod->extensionDescriptors = m4_iod->extensionDescriptors;
		m4_iod->extensionDescriptors = NULL;
		iod->IPMPDescriptorPointers = m4_iod->IPMPDescriptorPointers;
		m4_iod->IPMPDescriptorPointers = NULL;
		iod->OCIDescriptors = m4_iod->OCIDescriptors;
		m4_iod->OCIDescriptors = NULL;
		iod->URLString = m4_iod->URLString;
		m4_iod->URLString = NULL;
		iod->tag = InitialObjectDescriptor_Tag;

		iod->audio_profileAndLevel = m4_iod->audio_profileAndLevel;
		iod->graphics_profileAndLevel = m4_iod->graphics_profileAndLevel;
		iod->inlineProfileFlag = m4_iod->inlineProfileFlag;
		iod->OD_profileAndLevel = m4_iod->OD_profileAndLevel;
		iod->scene_profileAndLevel = m4_iod->scene_profileAndLevel;
		iod->visual_profileAndLevel = m4_iod->visual_profileAndLevel;

		//then recreate the desc in Inc
		for (i = 0; i < ChainGetCount(m4_iod->ES_ID_IncDescriptors); i++) {
			inc = (ES_ID_Inc*)ChainGetEntry(m4_iod->ES_ID_IncDescriptors, i);
			movie->LastError = GetESDForTime(movie->moov, inc->trackID, 0, &esd);
			if (!movie->LastError) movie->LastError = ChainAddEntry(iod->ESDescriptors, esd);
			if (movie->LastError) {
				OD_DeleteDescriptor(&desc);
				OD_DeleteDescriptor((Descriptor **) &iod);
				return NULL;
			}
		}
		OD_DeleteDescriptor(&desc);
		return (Descriptor *)iod;
	}
}


u32 M4_GetTrackCount(M4File *the_file)
{
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) return M4BadParam;

	if (! movie->moov->trackList) {
		movie->LastError = M4InvalidMP4File;
		return 0;
	}
	return ChainGetCount(movie->moov->trackList);
}


u32 M4_GetTrackID(M4File *the_file, u32 trackNumber)
{
	u32 trackID;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) return 0;

	movie->LastError = GetTrackId(movie->moov, trackNumber, &trackID);
	if (movie->LastError) return 0;
	return trackID;
}


u32 M4_GetTrackByID(M4File *the_file, u32 trackID)
{
	TrackAtom *trak;
	u32 count;
	u32 i;
	if (the_file == NULL) return 0;
	
	count = M4_GetTrackCount(the_file);
	if (!count) return 0;
	for (i = 0; i < count; i++) {
		trak = GetTrackFromFile(the_file, i+1);
		if (!trak) return 0;
		if (trak->Header->trackID == trackID) return i+1;
	}
	return 0;
}

//return the timescale of the movie, 0 if error
u32 M4_GetTimeScale(M4File *the_file)
{
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) return 0;

	return movie->moov->mvhd->timeScale;
}

//return the duration of the movie, 0 if error
u64 M4_GetDuration(M4File *the_file)
{
#ifndef M4_READ_ONLY
	u32 i;
	u64 maxDur;
	TrackAtom *trak;
#endif

	M4Movie *movie = (M4Movie *) the_file;
	if (!movie ) return 0;

	//if file was open in Write or Edit mode, recompute the duration
	//the duration of a movie is the MaxDuration of all the tracks...

#ifndef M4_READ_ONLY

	if (movie->openMode != M4_OPEN_READ) {
		maxDur = 0;
		for (i = 0; i< ChainGetCount(movie->moov->trackList); i++) {
			trak = (TrackAtom*)ChainGetEntry(movie->moov->trackList, i);
			if( (movie->LastError = SetTrackDuration(trak))	) return 0;
			if (trak->Header->duration > maxDur)
				maxDur = trak->Header->duration;
		}
		movie->moov->mvhd->duration = maxDur;
	}

#endif	//M4_READ_ONLY

	return movie->moov->mvhd->duration;
}

//return the creation info of the movie
M4Err M4_GetCreationInfo(M4File *the_file, u64 *creationTime, u64 *modificationTime)
{
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) return M4BadParam;
	
	*creationTime = movie->moov->mvhd->creationTime;
	*modificationTime = movie->moov->mvhd->modificationTime;
	return M4OK;
}


//check the presence of a track in IOD. 0: NO, 1: YES, 2: ERROR
u8 M4_IsTrackInRootOD(M4File *the_file, u32 trackNumber)
{
	u32 i;
	u32 trackID;
	Descriptor *desc;
	ES_ID_Inc *inc;
	Chain *inc_list;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie || !movie->moov) return 2;

	if (!movie->moov->iods) return 0;

	desc = movie->moov->iods->descriptor;
	switch (desc->tag) {
	case MP4_IOD_Tag:
		inc_list = ((M4F_InitialObjectDescriptor *)desc)->ES_ID_IncDescriptors;
		break;
	case MP4_OD_Tag:
		inc_list = ((M4F_ObjectDescriptor *)desc)->ES_ID_IncDescriptors;
		break;
		//files without IOD are possible !
	default:
		return 0;
	}
	if ((movie->LastError = GetTrackId(movie->moov, trackNumber, &trackID))) return 2;
	for (i = 0; i < ChainGetCount(inc_list); i++) {
		inc = (ES_ID_Inc*)ChainGetEntry(inc_list, i);
		if (inc->trackID == trackID) return 1;
	}
	return 0;
}



//gets the enable flag of a track
//0: NO, 1: yes, 2: error
u8 M4_IsTrackEnabled(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);

	if (!trak) return 2;
	return (trak->Header->flags & 1) ? 1 : 0;
}


//get the track duration
//return 0 if bad param
u64 M4_GetTrackDuration(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

#ifndef M4_READ_ONLY

	/*in all modes except dump recompute duration in case headers are wrong*/
	if (((M4Movie *)the_file)->openMode != M4_OPEN_READ_DUMP) {
		SetTrackDuration(trak);
	}

#endif

	return trak->Header->duration;
}

//Return the number of track references of a track for a given ReferenceType
//return -1 if error
s32 M4_GetTrackReferenceCount(M4File *the_file, u32 trackNumber, u32 referenceType)
{
	TrackAtom *trak;
	TrackReferenceTypeAtom *dpnd;
	M4Movie *movie = (M4Movie *)the_file;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return -1;
	if (!trak->References) return 0;
	if (((M4Movie *)the_file)->openMode == M4_OPEN_WRITE) {
		movie->LastError = M4InvalidMP4Mode;
		return -1;
	}
	
	dpnd = NULL;
	if ( (movie->LastError = Track_FindRef(trak, referenceType, &dpnd)) ) return -1;
	if (!dpnd) return 0;
	return dpnd->trackIDCount;
}


//Return the referenced track number for a track and a given ReferenceType and Index
//return -1 if error, 0 if the reference is a NULL one, or the trackNumber
M4Err M4_GetTrackReference(M4File *the_file, u32 trackNumber, u32 referenceType, u32 referenceIndex, u32 *refTrack)
{
	M4Err e;
	TrackAtom *trak;
	TrackReferenceTypeAtom *dpnd;
	u32 refTrackNum;
	M4Movie *movie = (M4Movie *)the_file;
	trak = GetTrackFromFile(the_file, trackNumber);

	*refTrack = 0;
	if (!trak || !trak->References) return M4BadParam;
	
	dpnd = NULL;
	e = Track_FindRef(trak, referenceType, &dpnd);
	if (e) return e;
	if (!dpnd) return M4BadParam;

	if (referenceIndex > dpnd->trackIDCount) return M4BadParam;

	//the spec allows a NULL reference 
	//(ex, to force desync of a track, set a sync ref with ID = 0)
	if (dpnd->trackIDs[referenceIndex - 1] == 0) return M4OK;
	refTrackNum = GetTrackNumberByID(movie->moov, dpnd->trackIDs[referenceIndex-1]);

	//if the track was not found, this means the file is broken !!!
	if (! refTrackNum) return M4InvalidMP4File;
	*refTrack = refTrackNum;
	return M4OK;
}


//Return the media time given the absolute time in the Movie
M4Err M4_GetMediaTime(M4File *the_file, u32 trackNumber, u32 movieTime, u64 *MediaTime)
{
	TrackAtom *trak;
	u8 useEdit;
	s64 SegmentStartTime, mediaOffset;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !MediaTime) return M4BadParam;;

	SegmentStartTime = 0;
	return GetMediaTime(trak, movieTime, MediaTime, &SegmentStartTime, &mediaOffset, &useEdit);
}


//Get the stream description index (eg, the ESD) for a given time IN MEDIA TIMESCALE
//return 0 if error or if empty
u32 M4_GetStreamDescriptionIndex(M4File *the_file, u32 trackNumber, u64 for_time)
{
	u32 streamDescIndex;
	TrackAtom *trak;
	M4Movie *movie = (M4Movie *)the_file;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	if ( (movie->LastError = Media_GetSampleDescIndex(trak->Media, for_time, &streamDescIndex)) ) {
		return 0;
	}
	return streamDescIndex;
}

//Get the number of "streams" stored in the media - a media can have several stream descriptions...
u32 M4_GetStreamDescriptionCount(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	return ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
}


//Get the ESDescriptor given the StreamDescriptionIndex
//THE DESCRIPTOR IS DUPLICATED, SO HAS TO BE DELETED BY THE APP
ESDescriptor *M4_GetStreamDescriptor(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	ESDescriptor *outESD;
	M4Err e;
	M4Movie *movie = (M4Movie *)the_file;

	e = GetESD(movie->moov, M4_GetTrackID(the_file, trackNumber), StreamDescriptionIndex, &outESD);

	if (e && (e!= M4InvalidMP4Media)) {
		movie->LastError = e;
		return NULL;
	}
	return outESD;
}

//Get the decoderConfigDescriptor given the SampleDescriptionIndex
//THE DESCRIPTOR IS DUPLICATED, SO HAS TO BE DELETED BY THE APP
DecoderConfigDescriptor *M4_GetDecoderInformation(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	TrackAtom *trak;
	ESDescriptor *esd;
	Descriptor *decInfo;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return NULL;
	//get the ESD (possibly emulated)
	Media_GetESD(trak->Media, StreamDescriptionIndex, &esd, 1);
	if (!esd) return NULL;
	decInfo = (Descriptor *) esd->decoderConfig;
	esd->decoderConfig = NULL;
	OD_DeleteDescriptor((Descriptor **) &esd);
	return (DecoderConfigDescriptor *)decInfo;
}


//get the media duration (without edit)
//return 0 if bad param
u64 M4_GetMediaDuration(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	M4Movie *movie = (M4Movie *)the_file;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;


#ifndef M4_READ_ONLY

	/*except in dump mode always recompute the duration*/
	if (movie->openMode != M4_OPEN_READ_DUMP) {
		if ( (movie->LastError = Media_SetDuration(trak)) ) return 0;
	}

#endif

	return trak->Media->mediaHeader->duration;
}

//Get the timeScale of the media. All samples DTS/CTS are expressed in this timeScale
u32 M4_GetMediaTimeScale(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;
	return trak->Media->mediaHeader->timeScale;
}

#if 0

//not exported now...
M4Err M4_GetMediaLanguage(M4File *the_file, u32 trackNumber, char **outLanguageCode)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	if (*outLanguageCode) return M4BadParam;
	(*outLanguageCode) = malloc(sizeof(char) * 4);
	strcpy(*outLanguageCode, (const char *) trak->Media->mediaHeader->packedLanguage);
	return M4OK;
}

#endif


u32 M4_GetCopyrightCount(M4File *the_file)
{
	M4Movie *mov;
	UserDataMap *map;

	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType, bin128 UUID);

	mov = (M4Movie *)the_file;
	if (!mov->moov->udta) return 0;
	map = udta_getEntry(mov->moov->udta, CopyrightAtomType, NULL);
	if (!map) return 0;
	return ChainGetCount(map->atomList);
}

M4Err M4_GetCopyright(M4File *the_file, u32 Index, const char **threeCharCode, const char **notice)
{
	M4Movie *mov;
	UserDataMap *map;
	CopyrightAtom *cprt;


	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType, bin128 UUID);
	
	if (!Index) return M4BadParam;

	mov = (M4Movie *)the_file;
	if (!mov->moov->udta) return M4OK;
	map = udta_getEntry(mov->moov->udta, CopyrightAtomType, NULL);
	if (!map) return M4OK;

	if (Index > ChainGetCount(map->atomList)) return M4BadParam;

	cprt = (CopyrightAtom*)ChainGetEntry(map->atomList, Index-1);
	(*threeCharCode) = cprt->packedLanguageCode;
	(*notice) = cprt->notice;
	return M4OK;
}

M4Err M4_GetWatermark(M4File *the_file, bin128 UUID, u8** data, u32* length)
{
	M4Movie *mov;
	UserDataMap *map;
	WatermarkAtom *wm;
	UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType, bin128 UUID);

	mov = (M4Movie *)the_file;
	if (!mov->moov->udta) return M4NotSupported;
	
	map = udta_getEntry(mov->moov->udta, ExtendedAtomType, UUID);
	if (!map) return M4NotSupported;

	wm = (WatermarkAtom*)ChainGetEntry(map->atomList, 0);
	if (!wm) return M4NotSupported;

	*data = malloc(wm->dataSize);
	memcpy(*data, wm->data, wm->dataSize);
	*length = wm->dataSize;
	return M4OK;
}

u32 M4_GetMediaType(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	return trak->Media->handler->handlerType;
}

Bool IsMP4Description(u32 entryType)
{
	switch (entryType) {
	case MPEGSampleEntryAtomType:
	case MPEGAudioSampleEntryAtomType:
	case MPEGVisualSampleEntryAtomType:
		return 1;
	default:
		return 0;
	}
}

u32 M4_GetMediaSubType(M4File *the_file, u32 trackNumber, u32 DescriptionIndex)
{
	TrackAtom *trak;
	Atom *entry;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !DescriptionIndex) return 0;
	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, DescriptionIndex-1);
	if (!entry) return 0;

	//filter MPEG sub-types
	if (IsMP4Description(entry->type)) return M4_MPEG4_SubType;
	if (entry->type == GenericVisualSampleEntryAtomType) {
		return ((GenericVisualSampleEntryAtom *)entry)->EntryType;
	}
	else if (entry->type == GenericAudioSampleEntryAtomType) {
		return ((GenericAudioSampleEntryAtom *)entry)->EntryType;
	}
	else if (entry->type == GenericMediaSampleEntryAtomType) {
		return ((GenericMediaSampleEntryAtom *)entry)->EntryType;
	}
	return entry->type;
}


//Get the HandlerDescription name. 
M4Err M4_GetHandlerName(M4File *the_file, u32 trackNumber, const char **outName)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !outName) return M4BadParam;

	*outName = trak->Media->handler->nameUTF8;
	return M4OK;
}

//Check the DataReferences of this track
M4Err M4_CheckStreamDataReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	M4Err e;
	u32 drefIndex;
	TrackAtom *trak;

	if (!StreamDescriptionIndex || !trackNumber) return M4BadParam;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, StreamDescriptionIndex , NULL, &drefIndex);
	if (e) return e;
	if (!drefIndex) return M4BadParam;
	return Media_CheckDataEntry(trak->Media, drefIndex);
}

//get the location of the data. If URL && URN are NULL, the data is in this file
M4Err M4_GetStreamDataReference(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, const char **outURL, const char **outURN)
{
	TrackAtom *trak;
	DataEntryURLAtom *url;
	DataEntryURNAtom *urn;
	u32 drefIndex;
	M4Err e;

	if (!StreamDescriptionIndex || !trackNumber) return M4BadParam;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, StreamDescriptionIndex , NULL, &drefIndex);
	if (e) return e;
	if (!drefIndex) return M4BadParam;

	url = (DataEntryURLAtom*)ChainGetEntry(trak->Media->information->dataInformation->dref->atomList, drefIndex - 1);
	if (!url) return M4InvalidMP4File;

	*outURL = *outURN = NULL;
	if (url->type == DataEntryURLAtomType) {
		*outURL = url->location;
		*outURN = NULL;
	} else if (url->type == DataEntryURNAtomType) {
		urn = (DataEntryURNAtom *) url;
		*outURN = urn->nameURN;
		*outURL = urn->location;
	} else {
		*outURN = NULL;
		*outURL = NULL;
	}
	return M4OK;
}

//Get the number of samples
//return 0 if error or empty
u32 M4_GetSampleCount(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;
	return trak->Media->information->sampleTable->SampleSize->sampleCount;
}

u32 M4_TrackHasTimeOffsets(M4File *the_file, u32 trackNumber)
{
	u32 i;
	CompositionOffsetAtom *ctts;
	dttsEntry *pe;
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media->information->sampleTable->CompositionOffset) return 0;

	//return true at the first offset found
	ctts = trak->Media->information->sampleTable->CompositionOffset;
	for (i=0; i<ChainGetCount(ctts->entryList); i++) {
		pe = ChainGetEntry(ctts->entryList, i);
		if (pe->decodingOffset && pe->sampleCount) return 1;
	}
	return 0;
}

//return a sample give its number, and set the SampleDescIndex of this sample
//this index allows to retrieve the stream description if needed (2 media in 1 track)
//return NULL if error
M4Sample *M4_GetSample(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 *sampleDescriptionIndex)
{
	M4Err e;
	u32 descIndex;
	TrackAtom *trak;
	M4Sample *samp;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return NULL;

	if (!sampleNumber) return NULL;
	samp = M4_NewSample();
	if (!samp) return NULL;
	e = Media_GetSample(trak->Media, sampleNumber, &samp, &descIndex, 0, NULL);
	if (e) {
		M4SetLastError(the_file, e);
		M4_DeleteSample(&samp);
		return NULL;
	}
	if (sampleDescriptionIndex) *sampleDescriptionIndex = descIndex;
	
	return samp;
}

u32 M4_GetSampleDuration(M4File *the_file, u32 trackNumber, u32 sampleNumber)
{
	u32 dur;
	u32 dts;
	TrackAtom *trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !sampleNumber) return 0;

	stbl_GetSampleDTS(trak->Media->information->sampleTable->TimeToSample, sampleNumber, &dur);
	if (sampleNumber == trak->Media->information->sampleTable->SampleSize->sampleCount) {
		return (u32) trak->Media->mediaHeader->duration - dur;
	} 
	
	stbl_GetSampleDTS(trak->Media->information->sampleTable->TimeToSample, sampleNumber+1, &dts);
	return (dts - dur);
}

//same as M4_GetSample but doesn't fetch media data
M4Sample *M4_GetSampleInfo(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 *sampleDescriptionIndex, u64 *data_offset)
{
	M4Err e;
	u32 descIndex;
	TrackAtom *trak;
	M4Sample *samp;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return NULL;

	if (!sampleNumber) return NULL;
	samp = M4_NewSample();
	if (!samp) return NULL;
	e = Media_GetSample(trak->Media, sampleNumber, &samp, &descIndex, 1, data_offset);
	if (e) {
		M4SetLastError(the_file, e);
		M4_DeleteSample(&samp);
		return NULL;
	}
	if (sampleDescriptionIndex) *sampleDescriptionIndex = descIndex;
	return samp;
}

Bool M4_IsSelfContained(M4File *the_file, u32 trackNumber, u32 sampleDescriptionIndex)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;
	return Media_IsSelfContained(trak->Media, sampleDescriptionIndex);
}


//return a sample given a desired display time IN MEDIA TIME SCALE
//and set the StreamDescIndex of this sample
//this index allows to retrieve the stream description if needed (2 media in 1 track)
//return NULL if error
//WARNING: the sample may not be sync even though the sync was requested (depends on the media)
M4Err M4_GetSampleForTime(M4File *the_file, u32 trackNumber, u32 desiredTime, u32 *StreamDescriptionIndex, u8 SearchMode, M4Sample **sample, u32 *SampleNum)
{
	M4Err e;
	u32 sampleNumber, prevSampleNumber, syncNum, shadowSync;
	TrackAtom *trak;
	M4Sample *shadow;
	SampleTableAtom *stbl;
	u8 useShadow, IsSync;

	if (!sample || *sample) return M4BadParam;

	if (SampleNum) *SampleNum = 0;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stbl = trak->Media->information->sampleTable;
	
	e = findEntryForTime(stbl, desiredTime, 1, &sampleNumber, &prevSampleNumber);
	if (e) return e;

	//if no shadow table, reset to sync only
	useShadow = 0;
	if (!stbl->ShadowSync && (SearchMode == M4_SearchSyncShadow))
		SearchMode = M4_SearchSyncBackward;

	//if no syncTable, disable syncSearching, as all samples ARE sync
	if (! trak->Media->information->sampleTable->SyncSample) {
		if (SearchMode == M4_SearchSyncForward) SearchMode = M4_SearchForward;
		if (SearchMode == M4_SearchSyncBackward) SearchMode = M4_SearchBackward;
	}

	//not found, return EOF or browse backward
	if (!sampleNumber && !prevSampleNumber) {
		if (SearchMode == M4_SearchSyncBackward || SearchMode == M4_SearchBackward) {
			sampleNumber = trak->Media->information->sampleTable->SampleSize->sampleCount;
		}
		if (!sampleNumber) return M4EOF;
	}

	//check in case we have the perfect sample
	IsSync = 0;

	//according to the direction adjust the sampleNum value
	switch (SearchMode) {
	case M4_SearchSyncForward:
		IsSync = 1;
	case M4_SearchForward:
		//not the exact one
		if (!sampleNumber) {
			if (prevSampleNumber != stbl->SampleSize->sampleCount) {
				sampleNumber = prevSampleNumber + 1;
			} else {
				sampleNumber = prevSampleNumber;
			}
		}
		break;

	//if dummy mode, reset to default browsing
	case M4_SearchSyncBackward:
		IsSync = 1;
	case M4_SearchSyncShadow:
	case M4_SearchBackward:
	default:
		//first case, not found....
		if (!sampleNumber && !prevSampleNumber) {
			sampleNumber = stbl->SampleSize->sampleCount;
		} else if (!sampleNumber) {
			sampleNumber = prevSampleNumber;
		}
		break;
	}

	//get the sync sample num
	if (IsSync) {
		//get the SyncNumber
		e = Media_FindSyncSample(trak->Media->information->sampleTable,
						sampleNumber, &syncNum, SearchMode);
		if (e) return e;
		if (syncNum) sampleNumber = syncNum;
		syncNum = 0;
	}
	//if we are in shadow mode, get the previous sync sample
	//in case we can't find a good SyncShadow
	else if (SearchMode == M4_SearchSyncShadow) {
		//get the SyncNumber
		e = Media_FindSyncSample(trak->Media->information->sampleTable,
						sampleNumber, &syncNum, M4_SearchSyncBackward);
		if (e) return e;
	}


	//OK sampleNumber is exactly the sample we need (except for shadow)
	
	*sample = M4_NewSample();
	if (*sample == NULL) return M4OutOfMem;

	//we are in shadow mode, we need to browse both SyncSample and ShadowSyncSample to get
	//the desired sample...
	if (SearchMode == M4_SearchSyncShadow) {
		//get the shadowing number
		stbl_GetSampleShadow(stbl->ShadowSync, &sampleNumber, &shadowSync);
		//now sampleNumber is the closest previous shadowed sample.
		//1- If we have a closer sync sample, use it.
		//2- if the shadowSync is 0, we don't have any shadowing, use syncNum
		if ((sampleNumber < syncNum) || (!shadowSync)) {
			sampleNumber = syncNum;
		} else {
		//otherwise, we have a better alternate sample in the shadowSync for this sample
			useShadow = 1;
		}
	}

	e = Media_GetSample(trak->Media, sampleNumber, sample, StreamDescriptionIndex, 0, NULL);
	if (e) {
		M4_DeleteSample(sample);
		return e;
	}
	//optionally get the sample number
	if (SampleNum) *SampleNum = sampleNumber;

	//in shadow mode, we only get the data of the shadowing sample !
	if (useShadow) {
		//we have to use StreamDescriptionIndex in case the sample data is in another desc
		//though this is unlikely as non optimized...
		shadow = M4_GetSample(the_file, trackNumber, shadowSync, StreamDescriptionIndex);
		//if no sample, the shadowSync is broken, return the sample
		if (!shadow) return M4OK;
		(*sample)->IsRAP = 1;
		free((*sample)->data);
		(*sample)->dataLength = shadow->dataLength;
		(*sample)->data = shadow->data;
		//set data length to 0 to keep the buffer alive...
		shadow->dataLength = 0;
		M4_DeleteSample(&shadow);
	}
	return M4OK;
}

M4Err M4_GetSampleForMovieTime(M4File *the_file, u32 trackNumber, u32 movieTime, u32 *StreamDescriptionIndex, u8 SearchMode, M4Sample **sample, u32 *sampleNumber)
{
	M4Err e;
	TrackAtom *trak;
	u64 mediaTime;
	s64 segStartTime, mediaOffset;
	u32 sampNum;
	u8 useEdit;
	Float ft;
	M4Err GetNextMediaTime(TrackAtom *trak, u32 movieTime, u64 *OutMovieTime);
	M4Err GetPrevMediaTime(TrackAtom *trak, u32 movieTime, u64 *OutMovieTime);

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	if (*sample || !sample) return M4BadParam;
	//check 0-duration tracks (BIFS and co). Check we're not searching forward
	if (!trak->Header->duration) {
		if (movieTime && ( (SearchMode == M4_SearchSyncForward) || (SearchMode == M4_SearchForward)) ) {
			*sample = NULL;
			if (sampleNumber) *sampleNumber = 0;
			*StreamDescriptionIndex = 0;
			return M4EOF;
		}
	}
	else if (movieTime * trak->moov->mvhd->timeScale > trak->Header->duration * trak->Media->mediaHeader->timeScale) {
		*sample = NULL;
		if (sampleNumber) *sampleNumber = 0;
		*StreamDescriptionIndex = 0;
		return M4EOF;
	}

	//get the media time for this movie time...
	mediaTime = segStartTime = 0;
	*StreamDescriptionIndex = 0;

	e = GetMediaTime(trak, movieTime, &mediaTime, &segStartTime, &mediaOffset, &useEdit);
	if (e) return e;

	/*here we check if we were playing or not and return no sample in normal search modes*/
	if (useEdit && mediaOffset == -1) {
		if ((SearchMode==M4_SearchForward) || (SearchMode==M4_SearchBackward)) {
			/*get next sample time in MOVIE timescale*/
			if (SearchMode==M4_SearchForward) 
				e = GetNextMediaTime(trak, movieTime, &mediaTime);
			else
				e = GetPrevMediaTime(trak, movieTime, &mediaTime);
			if (e) return e;
			return M4_GetSampleForMovieTime(the_file, trackNumber, (u32) mediaTime, StreamDescriptionIndex, M4_SearchSyncForward, sample, sampleNumber);
		}
		if (sampleNumber) *sampleNumber = 0;
		*sample = M4_NewSample();
		(*sample)->DTS = movieTime;
		return M4OK;
	}

	//OK, we have a sample so fetch it
	e = M4_GetSampleForTime(the_file, trackNumber, (u32) mediaTime, StreamDescriptionIndex, SearchMode, sample, &sampNum);
	if (e) return e;

	//OK, now the trick: we have to rebuild the time stamps, according 
	//to the media time scale (used by SLConfig) - add the edit start time but stay in 
	//the track TS
	if (useEdit) {
		ft = (Float) segStartTime * trak->Media->mediaHeader->timeScale / trak->moov->mvhd->timeScale ;
		(*sample)->DTS += (u32) ft;
		/*watchout, the sample fetched mAY be before the first sample in the edit list (when seeking)*/
		if ( (*sample)->DTS > mediaOffset) {
			(*sample)->DTS -= (u32) mediaOffset;
		} else {
			(*sample)->DTS = 0;
		}
	}
	if (sampleNumber) *sampleNumber = sampNum;
	return M4OK;
}



u64 M4_GetBytesMissing(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	return trak->Media->BytesMissing;
}

M4Err M4_SetSamplesPadding(M4File *the_file, u32 trackNumber, u32 padding_bytes)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	trak->padding_bytes = padding_bytes;
	return M4OK;

}

//get the number of edited segment
u32 M4_GetEditSegmentCount(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	if (!trak->EditAtom || !trak->EditAtom->editList) return 0;
	return ChainGetCount(trak->EditAtom->editList->entryList);
}


//Get the desired segment information
M4Err M4_GetEditSegment(M4File *the_file, u32 trackNumber, u32 SegmentIndex, u64 *EditTime, u64 *SegmentDuration, u64 *MediaTime, u8 *EditMode)
{
	u32 i;
	u64 startTime;
	TrackAtom *trak;
	EditListAtom *elst;
	edtsEntry *ent;

	ent = NULL;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	if (!trak || !trak->EditAtom || 
		!trak->EditAtom->editList || 
		(SegmentIndex > ChainGetCount(trak->EditAtom->editList->entryList)) || 
		!SegmentIndex)
		return M4BadParam;

	elst = trak->EditAtom->editList;
	startTime = 0;

	for (i = 0; i < SegmentIndex; i++) {
		ent = (edtsEntry*)ChainGetEntry(elst->entryList, i);
		if (i < SegmentIndex-1) startTime += ent->segmentDuration;
	}
	*EditTime = startTime;
	*SegmentDuration = ent->segmentDuration;
	if (ent->mediaTime < 0) {
		*MediaTime = 0;
		*EditMode = M4_EDIT_EMPTY;
		return M4OK;
	}
	if (ent->mediaRate == 0) {
		*MediaTime = ent->mediaTime;
		*EditMode = M4_EDIT_DWELL;
		return M4OK;
	}
	*MediaTime = ent->mediaTime;
	*EditMode = M4_EDIT_NORMAL;
	return M4OK;
}

u8 M4_MediaHasSyncPoints(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;
	if (trak->Media->information->sampleTable->SyncSample) {
		if (!trak->Media->information->sampleTable->SyncSample->entryCount) return 2;	
		return 1;
	}
	return 0;
}

M4Err M4_GetMovieVersionInfo(M4File *the_file, u32 *brand, u32 *minorVersion, u32 *AlternateBrandsCount)
{
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !brand) return M4BadParam;
	if (!movie->brand) {
		*brand = 0;
		if (minorVersion) *minorVersion = 0;
		if (AlternateBrandsCount) *AlternateBrandsCount = 0;
		return M4OK;
	}

	*brand = movie->brand->majorBrand;
	if (minorVersion) *minorVersion = movie->brand->minorVersion;
	if (AlternateBrandsCount) *AlternateBrandsCount = movie->brand->altCount;
	return M4OK;
}

M4Err M4_GetMovieAlternateBrand(M4File *the_file, u32 BrandIndex, u32 *brand)
{
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !movie->brand || !brand) return M4BadParam;
	if (BrandIndex > movie->brand->altCount || !BrandIndex) return M4BadParam;
	*brand = movie->brand->altBrand[BrandIndex-1];
	return M4OK;
}

M4Err M4_GetSamplePaddingBits(M4File *the_file, u32 trackNumber, u32 sampleNumber, u8 *NbBits)
{
	TrackAtom *trak;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;


	//Padding info
	return stbl_GetPaddingBits(trak->Media->information->sampleTable->PaddingBits, 
		sampleNumber, NbBits);

}


Bool M4_HasPaddingBits(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	if (trak->Media->information->sampleTable->PaddingBits) return 1;
	return 0;
}


u32 M4_GetUserDataItemCount(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID)
{
	UserDataMap *map;
	TrackAtom *trak;
	UserDataAtom *udta;
	bin128 t;
	u32 i, count;
	M4Movie *movie = (M4Movie *)the_file;

	if (!movie || !movie->moov) return 0;
	
	if (UserDataType == ExtendedAtomType) UserDataType = 0;
	memset(t, 1, 16);
	
	if (trackNumber) {
		trak = GetTrackFromFile(the_file, trackNumber);
		if (!trak) return 0;
		udta = trak->udta;
	} else {
		udta = movie->moov->udta;
	}
	if (!udta) return 0;

	for (i=0; i<ChainGetCount(udta->recordList); i++) {
		map = ChainGetEntry(udta->recordList, i);
		count = ChainGetCount(map->atomList);

		if ((map->atomType == ExtendedAtomType) && !memcmp(map->uuid, UUID, 16)) return count;
		else if (map->atomType == UserDataType) return count;
	}
	return 0;
}

M4Err M4_GetUserDataItem(M4File *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex, char **userData, u32 *userDataSize)
{
	UserDataMap *map;
	UnknownAtom *ptr;
	u32 i;
	bin128 t;
	TrackAtom *trak;
	UserDataAtom *udta;

	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !movie->moov) return 0;
	
	if (trackNumber) {
		trak = GetTrackFromFile(the_file, trackNumber);
		if (!trak) return M4BadParam;
		udta = trak->udta;
	} else {
		udta = movie->moov->udta;
	}
	if (!udta) return M4BadParam;

	if (UserDataType == ExtendedAtomType) UserDataType = 0;
	memset(t, 1, 16);

	if (!UserDataIndex) return M4BadParam;
	if (!userData || !userDataSize || *userData) return M4BadParam;

	for (i=0; i<ChainGetCount(udta->recordList); i++) {
		map = ChainGetEntry(udta->recordList, i);
		if ((map->atomType == ExtendedAtomType) && !memcmp(map->uuid, UUID, 16)) goto found;
		else if (map->atomType == UserDataType) goto found;
	
	}
	return M4BadParam;

found:

	if (UserDataIndex > ChainGetCount(map->atomList) ) return M4BadParam;
	ptr = ChainGetEntry(map->atomList, UserDataIndex-1);

	//ok alloc the data
	*userData = malloc(sizeof(char)*ptr->dataSize);
	if (!*userData) return M4OutOfMem;
	memcpy(*userData, ptr->data, sizeof(char)*ptr->dataSize);
	*userDataSize = ptr->dataSize;	
	return M4OK;
}

void M4_MovieDelete(M4File *the_file)
{
	//free and return;
	DelMovie((M4Movie *)the_file);
}

u32 M4_GetTrackMaxChunkDuration(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	u32 i, sample_per_chunk, sample_dur;
	SampleToChunkAtom *stsc;
	TimeToSampleAtom *stts;
	stscEntry *sc;
	sttsEntry *ts;

	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !trackNumber || !movie->moov) return 0;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	stsc = trak->Media->information->sampleTable->SampleToChunk;
	stts = trak->Media->information->sampleTable->TimeToSample;

	sample_per_chunk = 0;
	for (i=0; i<ChainGetCount(stsc->entryList); i++) {
		sc = ChainGetEntry(stsc->entryList, i);
		if (sc->samplesPerChunk > sample_per_chunk) sample_per_chunk = sc->samplesPerChunk;
	}
	sample_dur = 0;
	for (i=0; i<ChainGetCount(stts->entryList); i++) {
		ts = ChainGetEntry(stts->entryList, i);
		if (ts->sampleDelta > sample_dur) sample_dur = ts->sampleDelta;
	}
	
	//rescale to ms
	i = 1000 * sample_dur * sample_per_chunk / trak->Media->mediaHeader->timeScale;
	return i;
}

u32 M4_GetSampleFragmentCount(M4File *the_file, u32 trackNumber, u32 sampleNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return 0;

	//Padding info
	return stbl_GetSampleFragmentCount(trak->Media->information->sampleTable->Fragments, sampleNumber);
}

u16 M4_GetSampleFragmentSize(M4File *the_file, u32 trackNumber, u32 sampleNumber, u32 FragmentIndex)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !FragmentIndex) return 0;

	//Padding info
	return stbl_GetSampleFragmentSize(trak->Media->information->sampleTable->Fragments, sampleNumber, FragmentIndex);
}

u32 M4_GetRandomAccessCount(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !trak->Media->information->sampleTable->SyncSample) return 0;
	return trak->Media->information->sampleTable->SyncSample->entryCount;
}


M4Err M4_GetTrackSampleDefaults(M4File *the_file, u32 trackNumber, 
							 u32 *defaultDuration, u32 *defaultSize, u32 *defaultDescriptionIndex,
							 u32 *defaultRandomAccess, u8 *defaultPadding, u16 *defaultDegradationPriority)
{
	TrackAtom *trak;
	sttsEntry *ts_ent;
	stscEntry *sc_ent;
	u32 i, j, maxValue, value;
	SampleTableAtom *stbl;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stbl = trak->Media->information->sampleTable;
	//duration
	if (defaultDuration) {
		maxValue = value = 0;
		for (i=0; i<ChainGetCount(stbl->TimeToSample->entryList); i++) {
			ts_ent = ChainGetEntry(stbl->TimeToSample->entryList, i);
			if (ts_ent->sampleCount>maxValue) {
				value = ts_ent->sampleDelta;
				maxValue = ts_ent->sampleCount;
			}
		}
		*defaultDuration = value;
	}
	//size
	if (defaultSize) {
		*defaultSize = stbl->SampleSize->sampleSize;
	}
	//descIndex
	if (defaultDescriptionIndex) {
		maxValue = value = 0;
		for (i=0; i<ChainGetCount(stbl->SampleToChunk->entryList); i++) {
			sc_ent = ChainGetEntry(stbl->SampleToChunk->entryList, i);
			if ((sc_ent->nextChunk - sc_ent->firstChunk) * sc_ent->samplesPerChunk > maxValue) {
				value = sc_ent->sampleDescriptionIndex;
				maxValue = (sc_ent->nextChunk - sc_ent->firstChunk) * sc_ent->samplesPerChunk;
			}
		}
		*defaultDescriptionIndex = value ? value : 1;
	}
	//RAP
	if (defaultRandomAccess) {
		//no sync table is ALL RAP
		*defaultRandomAccess = stbl->SyncSample ? 0 : 1;
		if (stbl->SyncSample 
			&& (stbl->SyncSample->entryCount >= stbl->SampleSize->sampleCount/2)) {
			*defaultRandomAccess = 1;
		}
	}
	//defaultPadding
	if (defaultPadding) {
		*defaultPadding = 0;
		if (stbl->PaddingBits) {
			maxValue = 0;
			for (i=0; i<stbl->PaddingBits->SampleCount; i++) {
				value = 0;
				for (j=0; j<stbl->PaddingBits->SampleCount; j++) {
					if (stbl->PaddingBits->padbits[i]==stbl->PaddingBits->padbits[j]) {
						value ++;
					}
				}
				if (value>maxValue) {
					maxValue = value;
					*defaultPadding = stbl->PaddingBits->padbits[i];
				}
			}
		}
	}
	//defaultDegradationPriority
	if (defaultDegradationPriority) {
		*defaultDegradationPriority = 0;
		if (stbl->DegradationPriority) {
			maxValue = 0;
			for (i=0; i<stbl->DegradationPriority->entryCount; i++) {
				value = 0;
				for (j=0; j<stbl->DegradationPriority->entryCount; j++) {
					if (stbl->DegradationPriority->priorities[i]==stbl->DegradationPriority->priorities[j]) {
						value ++;
					}
				}
				if (value>maxValue) {
					maxValue = value;
					*defaultDegradationPriority = stbl->DegradationPriority->priorities[i];
				}
			}
		}
	}
	return M4OK;
}


M4Err M4_RefreshFragmentedMovie(M4File *the_file, u64 *MissingBytes)
{
#ifndef	M4_ISO_NO_FRAGMENTS
	return M4NotSupported;
#else
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !movie->moov || !movie->moov->mvex) return M4BadParam;
	if (movie->openMode != M4_OPEN_READ) return M4BadParam;

	//ok parse root atoms
	return ParseMovieAtoms(movie, MissingBytes);
#endif
}


M4Err M4_SetStreamingTextMode(M4File *the_file, Bool do_convert)
{
	if (!the_file) return M4BadParam;
	((M4Movie *)the_file)->convert_streaming_text = do_convert;
	return M4OK;
}


UnknownStreamDescription *M4_GetUnknownStreamDescription(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	GenericVisualSampleEntryAtom *entry;
	GenericAudioSampleEntryAtom *gena;
	GenericMediaSampleEntryAtom *genm;
	TrackAtom *trak;
	UnknownStreamDescription *udesc;
	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !StreamDescriptionIndex) return NULL;

	entry = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex-1);
	//no entry or MPEG entry: 
	if (!entry || IsMP4Description(entry->type) ) return NULL;
	//if we handle the description return false
	switch (entry->type) {
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
	case H263SampleEntryAtomType:
		return NULL;
	case GenericVisualSampleEntryAtomType:
		udesc = malloc(sizeof(UnknownStreamDescription));
		memset(udesc, 0, sizeof(UnknownStreamDescription));
		if (entry->EntryType == ExtendedAtomType) {
			memcpy(udesc->UUID, entry->uuid, sizeof(bin128));
		} else {
			udesc->codec_tag = entry->EntryType;
		}
		udesc->version = entry->version;
		udesc->revision = entry->revision;
		udesc->vendor_code = entry->vendor;
		udesc->temporal_quality = entry->temporal_quality;
		udesc->spacial_quality = entry->spacial_quality;
		udesc->width = entry->Width;
		udesc->height = entry->Height;
		udesc->h_res = entry->horiz_res;
		udesc->v_res = entry->vert_res;
		strcpy(udesc->szCompressorName, entry->compressor_name);
		udesc->depth = entry->bit_depth;
		udesc->color_table_index = entry->color_table_index;
		if (entry->data_size) {
			udesc->extension_buf_size = entry->data_size;
			udesc->extension_buf = malloc(sizeof(char) * entry->data_size);
			memcpy(udesc->extension_buf, entry->data, entry->data_size);
		}
		return udesc;
	case GenericAudioSampleEntryAtomType:
		gena = (GenericAudioSampleEntryAtom *)entry;
		udesc = malloc(sizeof(UnknownStreamDescription));
		memset(udesc, 0, sizeof(UnknownStreamDescription));
		if (gena->EntryType == ExtendedAtomType) {
			memcpy(udesc->UUID, gena->uuid, sizeof(bin128));
		} else {
			udesc->codec_tag = gena->EntryType;
		}
		udesc->version = gena->version;
		udesc->revision = gena->revision;
		udesc->vendor_code = gena->vendor;
		udesc->SampleRate = gena->samplerate_hi;
		udesc->bitsPerSample = gena->bitspersample;
		udesc->NumChannels = gena->channel_count;
		if (gena->data_size) {
			udesc->extension_buf_size = gena->data_size;
			udesc->extension_buf = malloc(sizeof(char) * gena->data_size);
			memcpy(udesc->extension_buf, gena->data, gena->data_size);
		}
		return udesc;
	case GenericMediaSampleEntryAtomType:
		genm = (GenericMediaSampleEntryAtom *)entry;
		udesc = malloc(sizeof(UnknownStreamDescription));
		memset(udesc, 0, sizeof(UnknownStreamDescription));
		if (genm->EntryType == ExtendedAtomType) {
			memcpy(udesc->UUID, genm->uuid, sizeof(bin128));
		} else {
			udesc->codec_tag = genm->EntryType;
		}
		if (genm->data_size) {
			udesc->extension_buf_size = genm->data_size;
			udesc->extension_buf = malloc(sizeof(char) * genm->data_size);
			memcpy(udesc->extension_buf, genm->data, genm->data_size);
		}
		return udesc;
	}
	return NULL;
}

M4Err M4_GetVisualEntrySize(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *Width, u32 *Height)
{
	TrackAtom *trak;
	SampleEntryAtom *entry;
	SampleDescriptionAtom *stsd;
	M4Movie *movie = (M4Movie *)the_file;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stsd = trak->Media->information->sampleTable->SampleDescription;
	if (!stsd) return movie->LastError = M4InvalidMP4File;
	if (!StreamDescriptionIndex || StreamDescriptionIndex > ChainGetCount(stsd->atomList)) return movie->LastError = M4BadParam;

	entry = ChainGetEntry(stsd->atomList, StreamDescriptionIndex - 1);
	//no support for generic sample entries (eg, no MPEG4 descriptor)
	if (entry == NULL) return M4BadParam;
	
	//valid for MPEG visual, JPG and 3GPP H263
	switch (entry->type) {
	case MPEGVisualSampleEntryAtomType:
	case H263SampleEntryAtomType:
	case GenericVisualSampleEntryAtomType:
		*Width = ((VisualSampleEntryAtom*)entry)->Width;
		*Height = ((VisualSampleEntryAtom*)entry)->Height;
		return M4OK;
	default:
		return M4BadParam;
	}
}

M4Err M4_GetAudioEntryInfo(M4File *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *SampleRate, u32 *Channels, u8 *bitsPerSample)
{
	TrackAtom *trak;
	SampleEntryAtom *entry;
	SampleDescriptionAtom *stsd;
	M4Movie *movie = (M4Movie *)the_file;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	stsd = trak->Media->information->sampleTable->SampleDescription;
	if (!stsd) return movie->LastError = M4InvalidMP4File;
	if (!StreamDescriptionIndex || StreamDescriptionIndex > ChainGetCount(stsd->atomList)) return movie->LastError = M4BadParam;

	entry = ChainGetEntry(stsd->atomList, StreamDescriptionIndex - 1);
	//no support for generic sample entries (eg, no MPEG4 descriptor)
	if (entry == NULL) return M4BadParam;
	
	switch (entry->type) {
	case MPEGAudioSampleEntryAtomType:
	case AMRSampleEntryAtomType:
	case WB_AMRSampleEntryAtomType:
		(*SampleRate) = ((AudioSampleEntryAtom*)entry)->samplerate_hi;
		(*Channels) = ((AudioSampleEntryAtom*)entry)->channel_count;
		(*bitsPerSample) = (u8) ((AudioSampleEntryAtom*)entry)->bitspersample;
		return M4OK;
	default:
		return M4BadParam;
	}
}

const char *M4_GetFilename(M4File *the_file)
{
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie) return NULL;
#ifndef M4_READ_ONLY
	if (movie->finalName && !movie->fileName) return movie->finalName;
#endif
	return movie->fileName;
}


u8 M4_GetMoviePLIndication(M4File *the_file, u8 PL_Code)
{
	M4F_InitialObjectDescriptor *iod;
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie) return 0;
	if (!movie->moov->iods || !movie->moov->iods->descriptor) return 0xFF;
	if (movie->moov->iods->descriptor->tag != MP4_IOD_Tag) return 0xFF;

	iod = (M4F_InitialObjectDescriptor *)movie->moov->iods->descriptor;
	switch (PL_Code) {
	case M4_PL_AUDIO: return iod->audio_profileAndLevel;
	case M4_PL_VISUAL: return iod->visual_profileAndLevel;
	case M4_PL_GRAPHICS: return iod->graphics_profileAndLevel;
	case M4_PL_SCENE: return iod->scene_profileAndLevel;
	case M4_PL_OD: return iod->OD_profileAndLevel;
	case M4_PL_INLINE: return iod->inlineProfileFlag;
	case M4_PL_MPEGJ:
	default:
		return 0xFF;
	}
}


M4Err M4_GetTrackVideoInfo(M4File *the_file, u32 trackNumber, u32 *width, u32 *height, s32 *translation_x, s32 *translation_y, s16 *layer)
{
	TrackAtom *tk = GetTrackFromFile(the_file, trackNumber);
	if (!tk) return M4BadParam;
	*width = tk->Header->width;
	*height = tk->Header->height;
	*layer = tk->Header->layer;
	*translation_x = tk->Header->matrix[6];
	*translation_y = tk->Header->matrix[7];
	return M4OK;
}
