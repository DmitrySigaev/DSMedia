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

/**************************************************************
		Some Local functions for movie creation
**************************************************************/
M4Err ParseRootAtom(Atom **outAtom, BitStream *bs, u64 *bytesExpected);

#ifndef	M4_ISO_NO_FRAGMENTS

M4Err MergeFragment(MovieFragmentAtom *moof, M4Movie *mov)
{
	u32 i, j;
	u64 MaxDur;
	TrackFragmentAtom *traf;
	TrackAtom *trak;

	M4Err MergeTrack(TrackAtom *trak, TrackFragmentAtom *traf, u64 *moof_offset);
	
	MaxDur = 0;

	//we shall have a MOOV and its MVEX BEFORE any MOOF
	if (!mov->moov || !mov->moov->mvex) return M4InvalidMP4File;
	//and all fragments must be continous
	if (mov->NextMoofNumber + 1 != moof->mfhd->sequence_number) return M4InvalidMP4File;

	for (i=0; i<ChainGetCount(moof->TrackList); i++) {
		traf = ChainGetEntry(moof->TrackList, i);
		if (!traf->tfhd) {
			trak = NULL;
			traf->trex = NULL;
		} else {
			trak = GetTrackFromID(mov->moov, traf->tfhd->trackID);
			for (j=0; j<ChainGetCount(mov->moov->mvex->TrackExList); j++) {
				traf->trex = ChainGetEntry(mov->moov->mvex->TrackExList, j);
				if (traf->trex->trackID == traf->tfhd->trackID) break;
				traf->trex = NULL;
			}
		}
		if (!trak || !traf->trex) return M4InvalidMP4File;

		//NB we can modify the movie data-offset info since we are in the middle of
		//parsing an atom, so next atom readin will reset it...
		MergeTrack(trak, traf, &mov->current_top_atom_start);

		//update trak duration
		SetTrackDuration(trak);
		if (trak->Header->duration > MaxDur) 
			MaxDur = trak->Header->duration;
	}

	mov->NextMoofNumber += 1;
	//update movie duration
	if (mov->moov->mvhd->duration < MaxDur) mov->moov->mvhd->duration = MaxDur;
	return M4OK;
}

#endif


M4Movie *GetMovie(M4File *theFile)
{
	return (M4Movie *) theFile;
}

M4Err ParseMovieAtoms(M4Movie *mov, u64 *bytesMissing)
{
	Atom *a;
	u64 totSize;
	M4Err e = M4OK;

	totSize = 0;


#ifndef	M4_ISO_NO_FRAGMENTS
	//restart from where we stoped last
	totSize = mov->current_top_atom_start;
	BS_Seek(mov->movieFileMap->bs, mov->current_top_atom_start);
#endif


	//while we have some data, parse our atoms...
	while (BS_Available(mov->movieFileMap->bs)) {
		*bytesMissing = 0;
#ifndef	M4_ISO_NO_FRAGMENTS
		mov->current_top_atom_start = BS_GetPosition(mov->movieFileMap->bs);
#endif

		e = ParseRootAtom(&a, mov->movieFileMap->bs, bytesMissing);

		if (e >= 0) {
			e = M4OK;
		} else if (e == M4UncompleteFile) {
			//our mdat is uncomplete, only valid for READ ONLY files...
			if (mov->openMode != M4_OPEN_READ) {
				return M4InvalidMP4File;
			}
			return e;
		} else {
			return e;
		}

		switch (a->type) {
		//our MOOV atom
		case MovieAtomType:
			if (mov->moov) return M4InvalidMP4File;
			mov->moov = (MovieAtom *)a;
			//set our pointer to the movie
			mov->moov->mov = mov;
			//add it to our list
			e = ChainAddEntry(mov->TopAtoms, a);
			if (e) return e;
			totSize += a->size;
			break;
		//we only keep the MDAT in READ for dump purposes
		case MediaDataAtomType:
			totSize += a->size;
			if (!mov->mdat && mov->openMode == M4_OPEN_READ) {
				mov->mdat = (MediaDataAtom *) a;
				e = ChainAddEntry(mov->TopAtoms, mov->mdat);
				if (e) return e;
			}
			//if we don't have any MDAT yet, create one (edit-write mode)
			//We only work with one mdat, but we're puting it at the place
			//of the first mdat found when opening a file for editing
			else if (!mov->mdat && mov->openMode != M4_OPEN_READ) {
				DelAtom(a);
				mov->mdat = (MediaDataAtom *) CreateAtom(MediaDataAtomType);
				e = ChainAddEntry(mov->TopAtoms, mov->mdat);
				if (e) return e;
			} else {
				DelAtom(a);
			}
			break;
		case FileTypeAtomType:
			if (mov->brand) {
				//just ignore it
				DelAtom(a);
			} else {
				mov->brand = (FileTypeAtom *)a;
				totSize += a->size;
			}
			break;


#ifndef	M4_ISO_NO_FRAGMENTS
		case MovieFragmentAtomType:
			((MovieFragmentAtom *)a)->mov = mov;

			totSize += a->size;
			if (mov->FragmentsFlags & FRAG_READ_DEBUG) {
				//keep it for debug
				ChainAddEntry(mov->MovieFragments, a);
			} else {
				//merge all info
				e = MergeFragment((MovieFragmentAtom *)a, mov);
				//destroy atom
				DelAtom(a);
			}
			break;
#endif

		default:
			totSize += a->size;
			e = ChainAddEntry(mov->TopAtoms, a);
			break;
		}
	}
	
	if (!mov->moov || !mov->moov->mvhd) return M4InvalidMP4File;

	if (mov->openMode == M4_OPEN_READ) return M4OK;

#ifndef M4_READ_ONLY

	//set the default interleaving time
	mov->interleavingTime = mov->moov->mvhd->timeScale;

#ifndef	M4_ISO_NO_FRAGMENTS
	//and remove all fragmentation info
	while (ChainGetCount(mov->MovieFragments)) {
		a = ChainGetEntry(mov->MovieFragments, 0);
		DelAtom(a);
		ChainDeleteEntry(mov->MovieFragments, 0);
	}
	if (mov->moov->mvex) DelAtom((Atom *)mov->moov->mvex);
	mov->moov->mvex = NULL;
#endif

#endif //M4_READ_ONLY
	
	return M4OK;
}


M4Movie *M4NewMovie()
{
	M4Movie *mov = (M4Movie*)malloc(sizeof(M4Movie));
	if (mov == NULL) {
		M4SetLastError(NULL, M4OutOfMem);
		return NULL;
	}
	memset(mov, 0, sizeof(M4Movie));

	//init the atoms
	mov->TopAtoms = NewChain();
	if (!mov->TopAtoms) {
		M4SetLastError(NULL, M4OutOfMem);
		free(mov);
		return NULL;
	}

#ifndef	M4_ISO_NO_FRAGMENTS
	mov->MovieFragments = NewChain();
	if (!mov->MovieFragments) {
		M4SetLastError(NULL, M4OutOfMem);
		DeleteChain(mov->TopAtoms);
		free(mov);
		return NULL;
	}
#endif
	
	//BY DEFAULT, store in flat mode
	mov->storageMode = M4_FLAT;
	return mov;
}

//Create and parse the movie for READ - EDIT only
M4Movie *MovieOpen(const char *fileName, u8 OpenMode)
{
	M4Err e;
	u64 bytes;
	M4Movie *mov = M4NewMovie();
	if (! mov) return NULL;

	mov->fileName = strdup(fileName);
	mov->openMode = OpenMode;

	if ( (OpenMode == M4_OPEN_READ) || (OpenMode == M4_OPEN_READ_DUMP) ) {
		//always in read ...
		mov->openMode = M4_OPEN_READ;
		mov->es_id_default_sync = -1;
		//for open, we do it the regular way and let the DataMap assign the appropriate struct
		//this can be FILE (the only one supported...) as well as remote 
		//(HTTP, ...),not suported yet
		//the bitstream IS PART OF the DataMap
		//as this is read-only, use a FileMapping. this is the only place where 
		//we use file mapping
		e = DataMap_New(fileName, NULL, DM_MODE_RO, &mov->movieFileMap);
		if (e) {
			M4SetLastError(NULL, e);
			DelMovie(mov);
			return NULL;
		}

#ifndef	M4_ISO_NO_FRAGMENTS
		if (OpenMode == M4_OPEN_READ_DUMP) mov->FragmentsFlags |= FRAG_READ_DEBUG;
#endif

	} else {

#ifdef M4_READ_ONLY
		//not allowed for READ_ONLY lib
		DelMovie(mov);
		M4SetLastError(NULL, M4InvalidMP4Mode);
		return NULL;

#else

		//set a default output name for edited file
		mov->finalName = (char*)malloc(strlen(fileName) + 5);
		if (!mov->finalName) {
			M4SetLastError(NULL, M4OutOfMem);
			DelMovie(mov);
			return NULL;
		}
		strcpy(mov->finalName, "out_");
		strcat(mov->finalName, fileName);

		//open the original file with edit tag
		e = DataMap_New(fileName, NULL, DM_MODE_E, &mov->movieFileMap);
		//if the file doesn't exist, we assume it's wanted and create one from scratch
		if (e) {
			M4SetLastError(NULL, e);
			DelMovie(mov);
			return NULL;
		}
		//and create a temp fileName for the edit
		e = DataMap_New("mp4_tmp_edit", NULL, DM_MODE_W, & mov->editFileMap);
		if (e) {
			M4SetLastError(NULL, e);
			DelMovie(mov);
			return NULL;
		}
		mov->es_id_default_sync = -1;

#endif
	}

	//OK, let's parse the movie...
	mov->LastError = ParseMovieAtoms(mov, &bytes);
	if (mov->LastError) {
		M4SetLastError(NULL, mov->LastError);
		DelMovie(mov);
		return NULL;
	}
	return mov;
}


u64 GetMP4Time()
{
	u32 calctime, msec;
	u64 ret;
	M4_GetUTCTimeSince1970(&calctime, &msec);
	calctime += MAC_TIME_OFFSET;
	ret = calctime;
	return ret;
}

#ifndef M4_READ_ONLY


#endif

void DelMovie(M4Movie *mov)
{
	//these are our two main files
	if (mov->movieFileMap) DataMap_Delete(mov->movieFileMap);

#ifndef M4_READ_ONLY
	if (mov->editFileMap) {
		DataMap_Delete(mov->editFileMap);
	}
	if (mov->finalName) free(mov->finalName);
#endif	//M4_READ_ONLY

	if (mov->brand) DelAtom((Atom *)mov->brand);
	DeleteAtomList(mov->TopAtoms);

#ifndef	M4_ISO_NO_FRAGMENTS
	DeleteAtomList(mov->MovieFragments);
#endif
	if (mov->fileName) free(mov->fileName);
	free(mov);
}

TrackAtom *GetTrackFromID(MovieAtom *moov, u32 trackID) 
{
	u32 i, count;
	TrackAtom *trak;
	if (!moov || !trackID) return NULL;

	count = ChainGetCount(moov->trackList);
	for (i = 0; i<count; i++) {
		trak = (TrackAtom*)ChainGetEntry(moov->trackList, i);
		if (trak->Header->trackID == trackID) return trak;
	}
	return NULL;
}

TrackAtom *GetTrackFromFile(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie) return NULL;
	
	trak = GetTrack(movie->moov, trackNumber);
	if (!trak) movie->LastError = M4BadParam;
	return trak;
}


//WARNING: MOVIETIME IS EXPRESSED IN MEDIA TS
M4Err GetMediaTime(TrackAtom *trak, u32 movieTime, u64 *MediaTime, s64 *SegmentStartTime, s64 *MediaOffset, u8 *useEdit)
{
	M4Err e;
	u32 i;
	u64 time, lastSampleTime, m_time;
	s64 mtime;
	edtsEntry *ent;
	Double scale_ts;
//	u32 DTS, CTS;
	u32 sampleNumber, prevSampleNumber, firstDTS;
	SampleTableAtom *stbl = trak->Media->information->sampleTable;

	*useEdit = 1;
	*MediaTime = 0;
	//no segment yet...
	*SegmentStartTime = -1;
	*MediaOffset = -1;
	if (!trak->moov->mvhd->timeScale || !trak->Media->mediaHeader->timeScale) {
		return M4InvalidMP4File;
	}

	//no samples...
	if (!stbl->SampleSize->sampleCount) {
		lastSampleTime = 0;
	} else {
		lastSampleTime = trak->Media->mediaHeader->duration;
	}

	//No edits, 1 to 1 mapping
	if (! trak->EditAtom || !trak->EditAtom->editList) {
		*MediaTime = movieTime;
		//check this is in our media time line
		if (*MediaTime > lastSampleTime) *MediaTime = lastSampleTime;
		*useEdit = 0;
		return M4OK;
	}
	//browse the edit list and get the time
	scale_ts = trak->moov->mvhd->timeScale;
	scale_ts /= trak->Media->mediaHeader->timeScale;
	scale_ts *= (movieTime + 1);
	m_time = (u64) (scale_ts);

	time = 0;
	ent = NULL;
	for (i = 0; i < ChainGetCount(trak->EditAtom->editList->entryList); i++) {
		//get all the entries that are empty at the begining...
		ent = (edtsEntry*)ChainGetEntry(trak->EditAtom->editList->entryList, i);
		if (time + ent->segmentDuration > m_time) {
			goto ent_found;
		}
		time += ent->segmentDuration;
	}
	//we had nothing in the list (strange file but compliant...)
	//return the 1 to 1 mapped vale of the last media sample
	if (!ent) {
		*MediaTime = movieTime;
		//check this is in our media time line
		if (*MediaTime > lastSampleTime) *MediaTime = lastSampleTime;
		*useEdit = 0;
		return M4OK;
	}
	//request for a bigger time that what we can give: return the last sample (undefined behavior...)
	*MediaTime = lastSampleTime;
	return M4OK;

ent_found:
	//OK, we found our entry, set the SegmentTime
	*SegmentStartTime = time;

	//we request an empty list, there's no media here...
	if (ent->mediaTime < 0) {
		*MediaTime = 0;
		return M4OK;
	}
	//we request a dwell edit
	if (! ent->mediaRate) {
		*MediaTime = ent->mediaTime;
		//no media offset
		*MediaOffset = 0;
		return M4OK;
	}
	
	/*WARNING: this can be "-1" when doing searchForward mode (to prevent jumping to next entry)*/
	mtime = ent->mediaTime + movieTime - (time * trak->Media->mediaHeader->timeScale / trak->moov->mvhd->timeScale);
	if (mtime<0) mtime = 0;
	*MediaTime = (u64) mtime;

#if 0
	//
	//Sanity check: is the requested time valid ? This is to cope with wrong EditLists
	//we have the translated time, but we need to make sure we have a sample at this time ...
	//we have to find a COMPOSITION time
	e = findEntryForTime(stbl, (u32) *MediaTime, 1, &sampleNumber, &prevSampleNumber);
	if (e) return e;
	
	//first case: our time is after the last sample DTS (it's a broken editList somehow)
	//set the media time to the last sample
	if (!sampleNumber && !prevSampleNumber) {
		*MediaTime = lastSampleTime;
		return M4OK;
	}
	//get the appropriated sample
	if (!sampleNumber) sampleNumber = prevSampleNumber;

	stbl_GetSampleDTS(stbl->TimeToSample, sampleNumber, &DTS);
	CTS = 0;
	if (stbl->CompositionOffset) stbl_GetSampleCTS(stbl->CompositionOffset, sampleNumber, &CTS);
#endif

	//now get the entry sample (the entry time gives the CTS, and we need the DTS
	e = findEntryForTime(stbl, (u32) ent->mediaTime, 1, &sampleNumber, &prevSampleNumber);
	if (e) return e;

	//oops, the mediaTime indicates a sample that is not in our media !
	if (!sampleNumber && !prevSampleNumber) {
		*MediaTime = lastSampleTime;
		return M4InvalidMP4File;
	}
	if (!sampleNumber) sampleNumber = prevSampleNumber;

	stbl_GetSampleDTS(stbl->TimeToSample, sampleNumber, &firstDTS);

	//and store the "time offset" of the desired sample in this segment
	//this is weird, used to rebuild the timeStamp when reading from the track, not the
	//media ...
	*MediaOffset = firstDTS;
	return M4OK;
}

M4Err GetNextMediaTime(TrackAtom *trak, u32 movieTime, u64 *OutMovieTime)
{
	u32 i;
	u64 time;
	edtsEntry *ent;

	*OutMovieTime = 0;
	if (! trak->EditAtom || !trak->EditAtom->editList) return M4BadParam;

	time = 0;
	ent = NULL;
	for (i = 0; i < ChainGetCount(trak->EditAtom->editList->entryList); i++) {
		ent = (edtsEntry*)ChainGetEntry(trak->EditAtom->editList->entryList, i);
		if (time * trak->Media->mediaHeader->timeScale >= movieTime * trak->moov->mvhd->timeScale) {
			/*skip empty edits*/
			if (ent->mediaTime >= 0) {
				*OutMovieTime = time * trak->Media->mediaHeader->timeScale / trak->moov->mvhd->timeScale;
				if (*OutMovieTime>0) *OutMovieTime -= 1;
				return M4OK;
			}
		}
		time += ent->segmentDuration;
	}
	//request for a bigger time that what we can give: return the last sample (undefined behavior...)
	*OutMovieTime = trak->moov->mvhd->duration;
	return M4OK;
}

M4Err GetPrevMediaTime(TrackAtom *trak, u32 movieTime, u64 *OutMovieTime)
{
	u32 i;
	u64 time;
	edtsEntry *ent;

	*OutMovieTime = 0;
	if (! trak->EditAtom || !trak->EditAtom->editList) return M4BadParam;

	time = 0;
	ent = NULL;
	for (i = 0; i < ChainGetCount(trak->EditAtom->editList->entryList); i++) {
		ent = (edtsEntry*)ChainGetEntry(trak->EditAtom->editList->entryList, i);
		if (ent->mediaTime == -1) {
			if ( (time + ent->segmentDuration) * trak->Media->mediaHeader->timeScale >= movieTime * trak->moov->mvhd->timeScale) {
				*OutMovieTime = time * trak->Media->mediaHeader->timeScale / trak->moov->mvhd->timeScale;
				return M4OK;
			}
			continue;
		}
		/*get the first entry whose end is greater than or equal to the desired time*/
		time += ent->segmentDuration;
		if ( time * trak->Media->mediaHeader->timeScale >= movieTime * trak->moov->mvhd->timeScale) {
			*OutMovieTime = time * trak->Media->mediaHeader->timeScale / trak->moov->mvhd->timeScale;
			return M4OK;
		}
	}
	*OutMovieTime = 0;
	return M4OK;
}

#ifndef M4_READ_ONLY

//Create the movie for WRITE only
M4Movie *MovieCreate(const char *fileName, u8 OpenMode)
{
	M4Err e;
	u64 now;
	MovieHeaderAtom *mvhd;
	char blank[16];

	Atom *moov_New();
	Atom *mdat_New();
	Atom *mvhd_New();
	Atom *iods_New();
	M4Err moov_AddAtom(MovieAtom *ptr, Atom *a);

	M4Movie *mov = M4NewMovie();
	if (!mov) return NULL;
	mov->openMode = OpenMode;
	//then set up our movie

	//in WRITE, the input dataMap is ALWAYS NULL
	mov->movieFileMap = NULL;

	//but we have the edit one
	if (OpenMode == M4_OPEN_WRITE) {
		//THIS IS NOT A TEMP FILE, WRITE mode is used for "live capture"
		//this file will be the final file...
		mov->fileName = strdup(fileName);
		e = DataMap_New(fileName, NULL, DM_MODE_W, & mov->editFileMap);
		if (e) goto err_exit;
		
		//brand is set to ISOM by default
		M4_SetMovieVersionInfo( (M4File *) mov, ISO_Media_File, 1);

		//if we have a brand, write it.
		if (mov->brand) {
			e = SizeAtom((Atom *)mov->brand);
			if (e) goto err_exit;
			e = WriteAtom((Atom *)mov->brand, mov->editFileMap->bs);
			if (e) goto err_exit;
		}
		//we have a trick here: the data will be stored on the fly, so the first
		//thing in the file is the MDAT. As we don't know if we have a large file (>4 GB) or not
		//do as if we had one and write 16 bytes: 4 (type) + 4 (size) + 8 (largeSize)...
		memset(blank, 0, 16);
		e = DataMap_AddData(mov->editFileMap, blank, 16);
		if (e) goto err_exit;

	} else {
		//we are in EDIT mode but we are creating the file -> temp file
		mov->finalName = (char*)malloc(strlen(fileName) + 1);
		strcpy(mov->finalName, fileName);
		e = DataMap_New("mp4_tmp_edit", NULL, DM_MODE_W, & mov->editFileMap);
		if (e) {
			M4SetLastError(NULL, e);
			DelMovie(mov);
			return NULL;
		}
		//brand is set to ISOM by default
		M4_SetMovieVersionInfo( (M4File *) mov, ISO_Media_File, 1);
	}

	//OK, create our atoms (mvhd, iods, ...)
	mov->moov = (MovieAtom *) moov_New();
	mov->moov->mov = mov;
	//Header SetUp
	now = GetMP4Time();
	mvhd = (MovieHeaderAtom *) mvhd_New();
	mvhd->creationTime = now;
	mvhd->modificationTime = now;
	mvhd->nextTrackID = 1;
	//600 is our default movie TimeScale
	mvhd->timeScale = 600;
	//600 is our default interleaving time too
	mov->interleavingTime = 600;
	//OK, add to our movie...
	e = moov_AddAtom(mov->moov, (Atom *)mvhd);
	if (e) goto err_exit;

	//create an MDAT
	mov->mdat = (MediaDataAtom *) mdat_New();

	//insert our top atoms
	ChainAddEntry(mov->TopAtoms, mov->moov);
	ChainAddEntry(mov->TopAtoms, mov->mdat);
	//default behaviour is capture mode, no interleaving (eg, no rewrite of mdat)
	mov->storageMode = M4_FLAT;
	return mov;

err_exit:
	M4SetLastError(NULL, e);
	if (mov) DelMovie(mov);
	return NULL;
}

edtsEntry *CreateEditEntry(u32 EditDuration, u32 MediaTime, u8 EditMode)
{
	edtsEntry *ent;

	ent = (edtsEntry*)malloc(sizeof(edtsEntry));
	if (!ent) return NULL;

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
	ent->segmentDuration = EditDuration;
	return ent;
}

#endif	//M4_READ_ONLY

