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

#ifndef	M4_ISO_NO_FRAGMENTS

TrackExtendsAtom *GetTrex(MovieAtom *moov, u32 TrackID)
{
	u32 i;
	TrackExtendsAtom *trex;
	for (i=0; i<ChainGetCount(moov->mvex->TrackExList); i++) {
		trex = ChainGetEntry(moov->mvex->TrackExList, i);
		if (trex->trackID == TrackID) return trex;
	}
	return NULL;
}


#ifndef M4_READ_ONLY

TrackFragmentAtom *GetTraf(M4Movie *mov, u32 TrackID)
{
	u32 i;
	TrackFragmentAtom *traf;
	if (!mov->moof) return NULL;

	//reverse browse the TRAFs, as there may be more than one per track ...
	for (i=ChainGetCount(mov->moof->TrackList); i>0; i--) {
		traf = ChainGetEntry(mov->moof->TrackList, i-1);
		if (traf->tfhd->trackID == TrackID) return traf;
	}
	return NULL;
}

M4Err M4_FinalizeMovieForFragments(M4File *the_file)
{
	M4Err e;
	u32 i;
	TrackExtendsAtom *trex;
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !movie->moov) return M4BadParam;
	//this is only allowed in write mode
	if (movie->openMode != M4_OPEN_WRITE) return M4InvalidMP4Mode;

	if (movie->FragmentsFlags & FRAG_WRITE_READY) return M4OK;
	movie->FragmentsFlags = 0;

	//update durations
	M4_GetDuration(the_file);

	//write movie
	e = WriteToFile(movie);
	if (e) return e;

	//make sure we do have all we need. If not this is not an error, just consider 
	//the file closed
	if (!movie->moov->mvex || !ChainGetCount(movie->moov->mvex->TrackExList)) return M4OK;

	for (i=0; i<ChainGetCount(movie->moov->mvex->TrackExList); i++) {
		trex = ChainGetEntry(movie->moov->mvex->TrackExList, i++);
		if (!trex->trackID || !GetTrackFromID(movie->moov, trex->trackID)) return M4OK;
		//we could also check all our data refs are local but we'll do that at run time
		//in order to allow a mix of both (remote refs in MOOV and local in MVEX)

		//one thing that MUST be done is OD cross-dependancies. The movie fragment spec 
		//is broken here, since it cannot allow dynamic insertion of new ESD and their
		//dependancies
	}

	//ok we are fine - note the data map is created at the begining
	if (i) movie->FragmentsFlags |= FRAG_WRITE_READY;
	movie->NextMoofNumber = 1;
	return M4OK;
}


M4Err M4_SetupTrackFragment(M4File *the_file, u32 TrackID, 
							 u32 DefaultSampleDescriptionIndex,
							 u32 DefaultSampleDuration,
							 u32 DefaultSampleSize,
							 u8 DefaultSampleIsSync,
							 u8 DefaultSamplePadding,
							 u16 DefaultDegradationPriority)
{
	MovieExtendsAtom *mvex;
	TrackExtendsAtom *trex;
	TrackAtom *trak;
	M4Err moov_AddAtom(MovieAtom *ptr, Atom *a);
	M4Err mvex_AddAtom(MovieExtendsAtom *ptr, Atom *a);
	M4Movie *movie = (M4Movie *)the_file;

	if (!movie || !movie->moov) return M4BadParam;
	//this is only allowed in write mode
	if (movie->openMode != M4_OPEN_WRITE) return M4InvalidMP4Mode;
	//and only at setup
	if (movie->FragmentsFlags & FRAG_WRITE_READY) return M4BadParam;


	trak = GetTrackFromID(movie->moov, TrackID);
	if (!trak) return M4BadParam;

	//create MVEX if needed
	if (!movie->moov->mvex) {
		mvex = (MovieExtendsAtom *) CreateAtom(MovieExtendsAtomType);
		moov_AddAtom(movie->moov, (Atom *) mvex);
	} else {
		mvex = movie->moov->mvex;
	}

	trex = GetTrex(movie->moov, TrackID);
	if (!trex) {
		trex = (TrackExtendsAtom *) CreateAtom(TrackExtendsAtomType);
		trex->trackID = TrackID;
		mvex_AddAtom(mvex, (Atom *) trex);
	}
	trex->track = trak;
	trex->def_sample_desc_index = DefaultSampleDescriptionIndex;
	trex->def_sample_duration = DefaultSampleDuration;
	trex->def_sample_size = DefaultSampleSize;
	trex->def_sample_flags = FORMAT_FRAG_FLAGS(DefaultSamplePadding, DefaultSampleIsSync, DefaultDegradationPriority);

	return M4OK;
}


u32 GetNumUsedValues(TrackFragmentAtom *traf, u32 value, u32 index)
{
	u32 i, j, NumValue = 0;
	TrackFragmentRunAtom *trun;
	TrunEntry *ent;

	for (i=0; i<ChainGetCount(traf->TrackRuns); i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		for (j=0; j<ChainGetCount(trun->entries); j++) {
			ent = ChainGetEntry(trun->entries, j);
			switch (index) {
			case 1:
				if (value == ent->Duration) NumValue ++;
				break;
			case 2:
				if (value == ent->size) NumValue ++;
				break;
			case 3:
				if (value == ent->flags) NumValue ++;
				break;
			}
		}
	}
	return NumValue;
}


void ComputeFragmentDefaults(TrackFragmentAtom *traf)
{
	u32 i, j, count, MaxNum, DefValue, ret;
	TrackFragmentRunAtom *trun;
	TrunEntry *ent;

	count = ChainGetCount(traf->TrackRuns);

	//Duration default
	MaxNum = DefValue = 0;
	for (i=0; i<count; i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		for (j=0; j<ChainGetCount(trun->entries); j++) {
			ent = ChainGetEntry(trun->entries, j);
			ret = GetNumUsedValues(traf, ent->Duration, 1);
			if (ret>MaxNum) {
				//at least 2 duration, specify for all
				if (MaxNum) {
					DefValue = 0;
					goto escape_duration;
				}
				MaxNum = ret;
				DefValue = ent->Duration;
			}
		}
	}
escape_duration:
	//store if #
	if (DefValue && (DefValue != traf->trex->def_sample_duration)) {
		traf->tfhd->def_sample_duration = DefValue;
	}

	//Size default
	MaxNum = DefValue = 0;
	for (i=0; i<count; i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		for (j=0; j<ChainGetCount(trun->entries); j++) {
			ent = ChainGetEntry(trun->entries, j);
			ret = GetNumUsedValues(traf, ent->size, 2);
			if (ret>MaxNum || (ret==1)) {
				//at least 2 sizes so we must specify all sizes
				if (MaxNum) {
					DefValue = 0;
					goto escape_size;
				}
				MaxNum = ret;
				DefValue = ent->size;
			}
		}
	}

escape_size:
	//store if #
	if (DefValue && (DefValue != traf->trex->def_sample_size)) {
		traf->tfhd->def_sample_size = DefValue;
	}

	//Flags default
	MaxNum = DefValue = 0;
	for (i=0; i<count; i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		for (j=0; j<ChainGetCount(trun->entries); j++) {
			ent = ChainGetEntry(trun->entries, j);
			ret = GetNumUsedValues(traf, ent->flags, 3);
			if (ret>MaxNum) {
				MaxNum = ret;
				DefValue = ent->flags;
			}
		}
	}
	//store if #
	if (DefValue && (DefValue != traf->trex->def_sample_flags)) {
		traf->tfhd->def_sample_flags = DefValue;
	}
}


M4Err M4_TrackFragmentSetOptions(M4File *the_file, u32 TrackID, u32 Code, u32 Param)
{
	TrackFragmentAtom *traf;
	M4Movie *movie = (M4Movie *)the_file;
	if (!movie || !movie->moov) return M4BadParam;
	//this is only allowed in write mode
	if (movie->openMode != M4_OPEN_WRITE) return M4InvalidMP4Mode;

	traf = GetTraf(movie, TrackID);
	if (!traf) return M4BadParam;
	switch (Code) {
	case TFO_EMPTY:
		traf->tfhd->EmptyDuration = Param;
		break;
	case TFO_IFRAME:
		traf->tfhd->IFrameSwitching = Param;
		break;
	case TFO_DATACACHE:
		//don't cache only one sample ...
		traf->DataCache = Param > 1 ? Param : 0;
		break;
	}
	return M4OK;
}


u32 UpdateRuns(TrackFragmentAtom *traf)
{
	u32 sampleCount, i, j, RunSize, UseDefaultSize, RunDur, UseDefaultDur, RunFlags, NeedFlags, UseDefaultFlag, UseCTS, count;
	TrackFragmentRunAtom *trun;
	TrunEntry *ent, *first_ent;
	
	sampleCount = 0;


	//traf data offset - we ALWAYS use data offset indication when writting otherwise
	//we would need to have one TRUN max in a TRAF for offset reconstruction or store
	//all TRUN in memory before writting :( Anyway it is much safer to indicate the
	//base offset of each traf rather than using offset aggregation rules specified 
	//in the std
	traf->tfhd->flags = TF_BASE_OFFSET;

	//empty runs
	if (traf->tfhd->EmptyDuration) {
		while (ChainGetCount(traf->TrackRuns)) {
			trun = ChainGetEntry(traf->TrackRuns, 0);
			ChainDeleteEntry(traf->TrackRuns, 0);
			DelAtom((Atom *)trun);
		}
		traf->tfhd->flags = TF_DUR_EMPTY;
		if (traf->tfhd->EmptyDuration != traf->trex->def_sample_duration) {
			traf->tfhd->def_sample_duration = traf->tfhd->EmptyDuration;
			traf->tfhd->flags |= TF_SAMPLE_DUR;
		}
		return 0;
	}


	UseDefaultSize = 0;
	UseDefaultDur = 0;
	UseDefaultFlag = 0;

	for (i=0; i<ChainGetCount(traf->TrackRuns); i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		RunSize = 0;
		RunDur = 0;
		RunFlags = 0;
		UseCTS = 0;
		NeedFlags = 0;

		first_ent = NULL;
		//process all samples in run
		count = ChainGetCount(trun->entries);
		if (count==1)
			count = 1;
		for (j=0; j<count; j++) {
			ent = ChainGetEntry(trun->entries, j);
			if (!j) {
				first_ent = ent;
				RunSize = ent->size;
				RunDur = ent->Duration;
			} 
			//we may have one entry only ...
			if (j || (count==1)) {
				//flags are only after first entry
				if (j==1 || (count==1) ) RunFlags = ent->flags;

				if (ent->size != RunSize) RunSize = 0;
				if (ent->Duration != RunDur) RunDur = 0;
				if (j && (RunFlags != ent->flags)) NeedFlags = 1;
			}
			if (ent->CTS_Offset) UseCTS = 1;
		}
		//empty list
		if (!first_ent) {
			ChainDeleteEntry(traf->TrackRuns, i);
			i--;
			continue;
		}
		trun->sample_count = ChainGetCount(trun->entries);
		trun->flags = 0;

		//size checking
		//constant size, check if this is from current fragment default or global default
		if (RunSize && (traf->trex->def_sample_size == RunSize)) {
			if (!UseDefaultSize) UseDefaultSize = 2;
			else if (UseDefaultSize==1) RunSize = 0;
		} else if (RunSize && (traf->tfhd->def_sample_size == RunSize)) {
			if (!UseDefaultSize) UseDefaultSize = 1;
			else if (UseDefaultSize==2) RunSize = 0;
		}
		//we could check for single entry runs and set the default size in the tfhd but 
		//that's no bit saving...
		else {
			RunSize=0;
		}

		if (!RunSize) trun->flags |= TR_SIZE;
		
		//duration checking
		if (RunDur && (traf->trex->def_sample_duration == RunDur)) {
			if (!UseDefaultDur) UseDefaultDur = 2;
			else if (UseDefaultDur==1) RunDur = 0;
		} else if (RunDur && (traf->tfhd->def_sample_duration == RunDur)) {
			if (!UseDefaultDur) UseDefaultDur = 1;
			else if (UseDefaultDur==2) RunDur = 0;
		}
		if (!RunDur) trun->flags |= TR_DURATION;

		//flag checking
		if (!NeedFlags) {
			if (RunFlags == traf->trex->def_sample_flags) {
				if (!UseDefaultFlag) UseDefaultFlag = 2;
				else if (UseDefaultFlag==1) NeedFlags = 1;
			} else if (RunFlags == traf->tfhd->def_sample_flags) {
				if (!UseDefaultFlag) UseDefaultFlag = 1;
				else if(UseDefaultFlag==2) NeedFlags = 1;
			}
		}
		if (NeedFlags) {
			//one flags entry per sample only
			trun->flags |= TR_FLAGS;
		} else {
			//indicated in global setup
			if (first_ent->flags == traf->trex->def_sample_flags) {
				if (!UseDefaultFlag) UseDefaultFlag = 2;
				else if (UseDefaultFlag==1) trun->flags |= TR_FIRST_FLAG;
			}
			//indicated in local setup
			else if (first_ent->flags == traf->tfhd->def_sample_flags) {
				if (!UseDefaultFlag) UseDefaultFlag = 1;
				else if (UseDefaultFlag==2) trun->flags |= TR_FIRST_FLAG;
			}
			//explicit store
			else {
				trun->flags |= TR_FIRST_FLAG;
			}
		}

		//CTS flag
		if (UseCTS) trun->flags |= TR_CTS_OFFSET;

		//run data offset if the offset indicated is 0 (first sample in this MDAT) don't 
		//indicate it
		if (trun->data_offset) trun->flags |= TR_DATA_OFFSET;

		sampleCount += trun->sample_count;
	}

	//last update TRAF flags
	if (UseDefaultSize==1) traf->tfhd->flags |= TF_SAMPLE_SIZE;
	if (UseDefaultDur==1) traf->tfhd->flags |= TF_SAMPLE_DUR;
	if (UseDefaultFlag==1) traf->tfhd->flags |= TF_SAMPLE_FLAGS;
	if (traf->tfhd->sample_desc_index && traf->tfhd->sample_desc_index != traf->trex->def_sample_desc_index) traf->tfhd->flags |= TF_SAMPLE_DESC;


	return sampleCount;
}



M4Err StoreFragment(M4Movie *movie)
{
	M4Err e;
	u64 moof_start;
	u32 size, i, s_count;
	unsigned char *buffer;
	TrackFragmentAtom *traf;
	TrackFragmentRunAtom *trun;
	BitStream *bs;
	if (!movie->moof) return M4OK;

	bs = movie->editFileMap->bs;

	//1- flush all caches
	for (i=0; i<ChainGetCount(movie->moof->TrackList); i++) {
		traf = ChainGetEntry(movie->moof->TrackList, i);
		if (!traf->DataCache) continue;
		s_count = ChainGetCount(traf->TrackRuns);
		if (!s_count) continue;
		trun = ChainGetEntry(traf->TrackRuns, s_count-1);
		if (!trun->cache || !trun->sample_count) continue;
		
		//update offset
		trun->data_offset = (u32) (BS_GetPosition(movie->editFileMap->bs) - movie->current_top_atom_start - 8);
		//write cache
		BS_GetContent(trun->cache, &buffer, &size);
		BS_WriteData(movie->editFileMap->bs, buffer, size);
		DeleteBitStream(trun->cache);
		free(buffer);
		trun->cache = NULL;
	}
	//2- update MOOF MDAT header
	moof_start = BS_GetPosition(bs);

	//start of MDAT
	BS_Seek(bs, movie->current_top_atom_start);
	//we assume we never write large MDATs in fragment mode which should always be true
	size = (u32) (moof_start - movie->current_top_atom_start);
	BS_WriteInt(bs, (u32) size, 32);
	BS_WriteInt(bs, MediaDataAtomType, 32);
	BS_Seek(bs, moof_start);

	//3- clean our traf's
	for (i=0; i<ChainGetCount(movie->moof->TrackList); i++) {
		traf = ChainGetEntry(movie->moof->TrackList, i);
		//compute default settings for the TRAF
		ComputeFragmentDefaults(traf);
		//updates all trun and set all flags, INCLUDING TRAF FLAGS (durations, ...)
		s_count = UpdateRuns(traf);
		//empty fragment destroy it
		if (!traf->tfhd->EmptyDuration && !s_count) {
			ChainDeleteEntry(movie->moof->TrackList, i);
			i--;
			DelAtom((Atom *) traf);
			continue;
		}
	}

	//4- Write moof
	e = SizeAtom((Atom *) movie->moof);
	if (e) return e;
	e = WriteAtom((Atom *) movie->moof, bs);
	if (e) return e;
	
	//5- destroy our moof
	DelAtom((Atom *) movie->moof);
	movie->moof = NULL;
	movie->NextMoofNumber ++;
	return M4OK;
}


M4Err M4_StartFragment(M4File *the_file)
{
	u32 i, count;
	TrackExtendsAtom *trex;
	TrackFragmentAtom *traf;
	M4Err e;
	M4Movie *movie = (M4Movie *)the_file;
	//and only at setup
	if (!movie || !(movie->FragmentsFlags & FRAG_WRITE_READY) ) return M4BadParam;
	if (movie->openMode != M4_OPEN_WRITE) return M4InvalidMP4Mode;

	count = ChainGetCount(movie->moov->mvex->TrackExList);
	if (!count) return M4BadParam;

	//store fragment
	if (movie->moof) {
		e = StoreFragment(movie);
		if (e) return e;
	}
 	
	//format MDAT
	movie->current_top_atom_start = BS_GetPosition(movie->editFileMap->bs);
	BS_WriteInt(movie->editFileMap->bs, 0, 32);
	BS_WriteInt(movie->editFileMap->bs, MediaDataAtomType, 32);

	//create new fragment
	movie->moof = (MovieFragmentAtom *) CreateAtom(MovieFragmentAtomType);
	movie->moof->mfhd = (MovieFragmentHeaderAtom *) CreateAtom(MovieFragmentHeaderAtomType);
	movie->moof->mfhd->sequence_number = movie->NextMoofNumber;
	
	//we create a TRAF for each setup track, unused ones will be removed at store time
	for (i=0; i<count; i++) {
		trex = ChainGetEntry(movie->moov->mvex->TrackExList, i);
		traf = (TrackFragmentAtom *) CreateAtom(TrackFragmentAtomType);
		traf->trex = trex;
		traf->tfhd = (TrackFragmentHeaderAtom *) CreateAtom(TrackFragmentHeaderAtomType);
		traf->tfhd->trackID = trex->trackID;
		//add 8 bytes (MDAT size+type) to avoid the data_offset in the first trun
		traf->tfhd->base_data_offset = movie->current_top_atom_start + 8;
		ChainAddEntry(movie->moof->TrackList, traf);
	}
	return M4OK;
}

u32 GetRunSize(TrackFragmentRunAtom *trun)
{
	u32 i, size;
	TrunEntry *ent;
	size = 0;
	for (i=0; i<ChainGetCount(trun->entries); i++) {
		ent = ChainGetEntry(trun->entries, i);
		size += ent->size;
	}
	return size;
}


M4Err M4_TrackFragmentAddSample(M4File *the_file, u32 TrackID, M4Sample *sample, u32 DescIndex, 
								 u32 Duration,
								 u8 PaddingBits, u16 DegradationPriority)
{
	u32 count, buffer_size;
	unsigned char *buffer;
	u64 pos;
	TrunEntry *ent;
	TrackFragmentAtom *traf, *traf_2;
	TrackFragmentRunAtom *trun;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie->moof || !(movie->FragmentsFlags & FRAG_WRITE_READY) || !sample) return M4BadParam;

	traf = GetTraf(movie, TrackID);
	if (!traf) return M4BadParam;

	if (!traf->tfhd->sample_desc_index) traf->tfhd->sample_desc_index = DescIndex ? DescIndex : traf->trex->def_sample_desc_index;

	pos = BS_GetPosition(movie->editFileMap->bs);

	
	//WARNING: we change stream description, create a new TRAF
	if ( DescIndex && (traf->tfhd->sample_desc_index != DescIndex)) {
		//if we're caching flush the current run
		if (traf->DataCache) {
			count = ChainGetCount(traf->TrackRuns);
			if (count) {
				trun = ChainGetEntry(traf->TrackRuns, count-1);
				trun->data_offset = (u32) (pos - movie->current_top_atom_start - 8);
				BS_GetContent(trun->cache, &buffer, &buffer_size);
				BS_WriteData(movie->editFileMap->bs, buffer, buffer_size);
				DeleteBitStream(trun->cache);
				trun->cache = NULL;
				free(buffer);
			}
		}
		traf_2 = (TrackFragmentAtom *) CreateAtom(TrackFragmentAtomType);
		traf_2->trex = traf->trex;
		traf_2->tfhd = (TrackFragmentHeaderAtom *) CreateAtom(TrackFragmentHeaderAtomType);
		traf_2->tfhd->trackID = traf->tfhd->trackID;
		//keep the same offset
		traf_2->tfhd->base_data_offset = movie->current_top_atom_start + 8;
		ChainAddEntry(movie->moof->TrackList, traf_2);

		//duplicate infos
		traf_2->tfhd->IFrameSwitching = traf->tfhd->IFrameSwitching;
		traf_2->DataCache  = traf->DataCache;
		traf_2->tfhd->sample_desc_index  = DescIndex;

		//switch them ...
		traf = traf_2;
	}


	//add TRUN entry
	count = ChainGetCount(traf->TrackRuns);
	if (count) {
		trun = ChainGetEntry(traf->TrackRuns, count-1);
		//check data offset when no caching as trun entries shall ALWAYS be contiguous samples
		if (!traf->DataCache && (movie->current_top_atom_start + 8 + trun->data_offset + GetRunSize(trun) != pos) )
			count = 0;
		
		//check I-frame detection
		if (traf->tfhd->IFrameSwitching && sample->IsRAP) 
			count = 0;

		if (traf->DataCache && (traf->DataCache==trun->sample_count) )
			count = 0;

		//if data cache is on and we're changing TRUN, store the cache and update data offset
		if (!count && traf->DataCache) {
			trun->data_offset = (u32) (pos - movie->current_top_atom_start - 8);
			BS_GetContent(trun->cache, &buffer, &buffer_size);
			BS_WriteData(movie->editFileMap->bs, buffer, buffer_size);
			DeleteBitStream(trun->cache);
			trun->cache = NULL;
			free(buffer);
		}
	}

	//new run
	if (!count) {
		trun = (TrackFragmentRunAtom *) CreateAtom(TrackFragmentRunAtomType);
		//store data offset (we have the 8 btyes offset of the MDAT)
		trun->data_offset = (u32) (pos - movie->current_top_atom_start - 8);
		ChainAddEntry(traf->TrackRuns, trun);

		//if we use data caching, create a bitstream
		if (traf->DataCache) 
			trun->cache = NewBitStream(NULL, 0, BS_WRITE);
	}

	ent = malloc(sizeof(TrunEntry));
	memset(ent, 0, sizeof(TrunEntry));
	ent->CTS_Offset = sample->CTS_Offset;
	ent->Duration = Duration;
	ent->size = sample->dataLength;
	ent->flags = FORMAT_FRAG_FLAGS(PaddingBits, sample->IsRAP, DegradationPriority);
	ChainAddEntry(trun->entries, ent);
	
	trun->sample_count += 1;

	//rewrite OD frames
	if (traf->trex->track->Media->handler->handlerType == M4_ODMediaType) {
		//this may fail if depandancies are not well done ...
		Media_ParseODFrame(traf->trex->track->Media, sample);
	}

	//finally write the data
	if (!traf->DataCache) {
		BS_WriteData(movie->editFileMap->bs, sample->data, sample->dataLength);
	} else if (trun->cache) {
		BS_WriteData(trun->cache, sample->data, sample->dataLength);
	} else {
		return M4BadParam;
	}
	return M4OK;
}



M4Err M4_TrackFragmentAppendData(M4File *the_file, u32 TrackID, unsigned char *data, u32 data_size, u8 PaddingBits)
{
	u32 count;
	u8 rap;
	u16 degp;
	TrunEntry *ent;
	TrackFragmentAtom *traf;
	TrackFragmentRunAtom *trun;
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie->moof || !(movie->FragmentsFlags & FRAG_WRITE_READY) ) return M4BadParam;

	traf = GetTraf(movie, TrackID);
	if (!traf || !traf->tfhd->sample_desc_index) return M4BadParam;

	//add TRUN entry
	count = ChainGetCount(traf->TrackRuns);
	if (!count) return M4BadParam;

	trun = ChainGetEntry(traf->TrackRuns, count-1);
	count = ChainGetCount(trun->entries);
	if (!count) return M4BadParam;
	ent = ChainGetEntry(trun->entries, count-1);
	ent->size += data_size;

	rap = GET_FRAG_SYNC(ent->flags);
	degp = GET_FRAG_DEG(ent->flags);
	ent->flags = FORMAT_FRAG_FLAGS(PaddingBits, rap, degp);

	//finally write the data
	if (!traf->DataCache) {
		BS_WriteData(movie->editFileMap->bs, data, data_size);
	} else if (trun->cache) {
		BS_WriteData(trun->cache, data, data_size);
	} else {
		return M4BadParam;
	}
	return M4OK;
}


#endif	//M4_READ_ONLY

u32 M4_IsTrackFragmented(M4File *the_file, u32 TrackID)
{
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie || !movie->moov || !movie->moov->mvex) return 0;
	return (GetTrex(movie->moov, TrackID) != NULL) ? 1 : 0;
}

u32 M4_IsMovieFragmented(M4File *the_file)
{
	M4Movie *movie = (M4Movie *) the_file;
	if (!movie || !movie->moov || !movie->moov->mvex) return 0;
	//if no tracks are setup in MVEX consider this is not a fragmented movie
	//since only track data can be added to moof's
	return ChainGetCount(movie->moov->mvex->TrackExList);
}

#else

M4Err M4_FinalizeMovieForFragments(M4File *the_file)
{
	return M4NotSupported;
}

M4Err M4_SetupTrackFragment(M4File *the_file, u32 TrackID, 
							 u32 DefaultSampleDescriptionIndex,
							 u32 DefaultSampleDuration,
							 u32 DefaultSampleSize,
							 u8 DefaultSampleIsSync,
							 u8 DefaultSamplePadding,
							 u16 DefaultDegradationPriority)
{
	return M4NotSupported;
}

M4Err M4_TrackFragmentSetOptions(M4File *the_file, u32 TrackID, u32 Code, u32 Param)
{
	return M4NotSupported;
}

M4Err M4_StartFragment(M4File *the_file)
{
	return M4NotSupported;
}

M4Err M4_TrackFragmentAddSample(M4File *the_file, u32 TrackID, M4Sample *sample, u32 DescIndex, 
								 u32 Duration,
								 u8 PaddingBits, u16 DegradationPriority)
{
	return M4NotSupported;
}


u32 M4_IsTrackFragmented(M4File *the_file, u32 TrackID)
{
	return 0;
}

u32 M4_IsMovieFragmented(M4File *the_file)
{
	return 0;
}

#endif
