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

TrackAtom *GetTrackbyID(MovieAtom *moov, u32 TrackID)
{
	TrackAtom *trak;
	u32 i;
	if (!moov) return NULL;
	for (i = 0; i < ChainGetCount(moov->trackList); i++) {
		trak = (TrackAtom*)ChainGetEntry(moov->trackList, i);
		if (trak->Header->trackID == TrackID) return trak;
	}
	return NULL;
}

TrackAtom *GetTrack(MovieAtom *moov, u32 trackNumber)
{
	TrackAtom *trak;
	if (!moov || !trackNumber || (trackNumber > ChainGetCount(moov->trackList))) return NULL;
	trak = (TrackAtom*)ChainGetEntry(moov->trackList, trackNumber - 1);
	return trak;

}

//get the number of a track given its ID
//return 0 if not found error
u32 GetTrackNumberByID(MovieAtom *moov, u32 trackID)
{
	u32 i;
	TrackAtom *trak;
	for (i = 0; i < ChainGetCount(moov->trackList); i++) {
		trak = (TrackAtom*)ChainGetEntry(moov->trackList, i);
		if (trak->Header->trackID == trackID) return i+1;
	}
	return 0;
}

//extraction of the ESD from the track
M4Err GetESD(MovieAtom *moov, u32 trackID, u32 StreamDescIndex, ESDescriptor **outESD)
{
	M4Err e;
	ESDescriptor *esd;
	SampleTableAtom *stbl;
	TrackAtom *trak, *OCRTrack;
	TrackReferenceTypeAtom *dpnd;
	SLConfigDescriptor *slc;
	MPEGSampleEntryAtom *entry;

	dpnd = NULL;
	*outESD = NULL;

	trak = GetTrack(moov, GetTrackNumberByID(moov, trackID));
	if (!trak) return M4InvalidMP4File;

	e = Media_GetESD(trak->Media, StreamDescIndex, &esd, 0);
	if (e) return e;

	e = Media_GetSampleDesc(trak->Media, StreamDescIndex, (SampleEntryAtom **) &entry, NULL);
	if (e) return e;
	//set the ID
	esd->ESID = trackID;
	
	//find stream dependencies
	e = Track_FindRef(trak, StreamDependenceAtomType , &dpnd);
	if (e) return e;
	if (dpnd) {
		//ONLY ONE STREAM DEPENDENCY IS ALLOWED
		if (dpnd->trackIDCount != 1) return M4InvalidMP4Media;
		//fix the spec: where is the index located ??
		esd->dependsOnESID = dpnd->trackIDs[0];
	} else {
		esd->dependsOnESID = 0;
	}

	//OK, get the OCR (in a REAL MP4File, OCR is 0 in ESD and is specified through track reference
	dpnd = NULL;
	OCRTrack = NULL;
	//find OCR dependencies
	e = Track_FindRef(trak, OCRReferenceAtomType, &dpnd);
	if (e) return e;
	if (dpnd) {
		if (dpnd->trackIDCount != 1) return M4InvalidMP4Media;
		esd->OCRESID = dpnd->trackIDs[0];
		OCRTrack = GetTrackFromID(trak->moov, dpnd->trackIDs[0]);

		while (OCRTrack) {
			/*if we have a dependency on a track that doesn't have OCR dep, remove that dependency*/
			e = Track_FindRef(OCRTrack, OCRReferenceAtomType, &dpnd);
			if (e || !dpnd || !dpnd->trackIDCount) {
				OCRTrack = NULL;
				goto default_sync;
			}
			/*this is explicit desync*/
			if (dpnd && ((dpnd->trackIDs[0]==0) || (dpnd->trackIDs[0]==OCRTrack->Header->trackID))) break;
			/*loop in OCRs, break it*/
			if (esd->ESID == OCRTrack->Header->trackID) {
				OCRTrack = NULL;
				goto default_sync;
			}
			/*check next*/
			OCRTrack = GetTrackFromID(trak->moov, dpnd->trackIDs[0]);
		}
		if (!OCRTrack) goto default_sync;
	} else {
default_sync:
		/*all tracks are sync'ed by default*/
		if (trak->moov->mov->es_id_default_sync<0) {
			if (esd->OCRESID)
				trak->moov->mov->es_id_default_sync = esd->OCRESID;
			else
				trak->moov->mov->es_id_default_sync = esd->ESID;
		}
		if (trak->moov->mov->es_id_default_sync) esd->OCRESID = (u16) trak->moov->mov->es_id_default_sync;
		/*cf ESD writer*/
		if (esd->OCRESID == esd->ESID) esd->OCRESID = 0;
	}



	//update the IPI stuff if needed
	if (esd->ipiPtr != NULL) {
		dpnd = NULL;
		e = Track_FindRef(trak, IPIReferenceAtomType, &dpnd);
		if (e) return e;
		if (dpnd) {
			if (esd->ipiPtr->tag != IPI_DescPtr_Tag) return M4InvalidMP4File;
			//OK, retrieve the ID: the IPI_ES_Id is currently the ref track
			esd->ipiPtr->IPI_ES_Id = dpnd->trackIDs[esd->ipiPtr->IPI_ES_Id - 1];
			//and change the tag
			esd->ipiPtr->tag = IPIPtr_Tag;
		} else {
			return M4InvalidMP4File;
		}
	}

	/*normally all files shall be stored with predefined=SLPredef_MP4, but of course some are broken (philips)
	so we just check the ESD_URL. If set, use the given cfg, otherwise always rewrite it*/
	if (esd->URLString != NULL) {
		*outESD = esd;
		return M4OK;
	}

	//if we are in publishing mode and we have an SLConfig specified, use it as is
	switch (entry->type) {
	case MPEGVisualSampleEntryAtomType:
		slc = ((MPEGVisualSampleEntryAtom *)entry)->slc;
		break;
	case MPEGAudioSampleEntryAtomType:
		slc = ((MPEGAudioSampleEntryAtom *)entry)->slc;
		break;
	case MPEGSampleEntryAtomType:
		slc = entry->slc;
		break;
	default:
		slc = NULL;
		break;
	}
	if (slc) {
		OD_DeleteDescriptor((Descriptor **)&esd->slConfig);
		OD_DuplicateDescriptor((Descriptor *)slc, (Descriptor **)&esd->slConfig);
		*outESD = esd;
		return M4OK;
	}
	//otherwise use the regular mapping
	
	//this is a desc for a media in the file, let's rewrite some param
	esd->slConfig->timestampLength = 32;
	esd->slConfig->timestampResolution = trak->Media->mediaHeader->timeScale;
	//NO OCR from MP4File streams (eg, constant OC Res one)
	esd->slConfig->OCRLength = 0;
	if (OCRTrack) {
		esd->slConfig->OCRResolution = OCRTrack->Media->mediaHeader->timeScale;
	} else {
		esd->slConfig->OCRResolution = 0;
	}
	stbl = trak->Media->information->sampleTable;
	// a little optimization here: if all our samples are sync, 
	//set the RAPOnly to true... for external users...
	if (! stbl->SyncSample) {
		esd->slConfig->useRandomAccessUnitsOnlyFlag = 1;
		esd->slConfig->useRandomAccessPointFlag = 0;
	} else {
		esd->slConfig->useRandomAccessUnitsOnlyFlag = 0;
		//signal we are NOT using sync points if no info is present in the table
		esd->slConfig->useRandomAccessPointFlag = stbl->SyncSample->entryCount ? 1 : 0;
	}
	//do we have DegradationPriority ?
	if (stbl->DegradationPriority) {
		esd->slConfig->degradationPriorityLength = 15;
	} else {
		esd->slConfig->degradationPriorityLength = 0;
	}
	//paddingBits
	if (stbl->PaddingBits) {
		esd->slConfig->usePaddingFlag = 1;
	}
	//change to support reflecting OD streams
	esd->slConfig->useAccessUnitEndFlag = 1;
	esd->slConfig->useAccessUnitStartFlag = 1;

	//signal we do have padding flag (since we only use logical SL packet
	//the user can decide whether to use the info or not
	esd->slConfig->usePaddingFlag = stbl->PaddingBits ? 1 : 0;

	//same with degradation priority
	esd->slConfig->degradationPriorityLength = stbl->DegradationPriority ? 32 : 0;

	//this new SL will be OUT OF THE FILE. Let's set its predefined to 0
	esd->slConfig->predefined = 0;
	
	*outESD = esd;
	return M4OK;
}


//extraction of the ESD from the track for the given time
M4Err GetESDForTime(MovieAtom *moov, u32 trackID, u64 CTS, ESDescriptor **outESD)
{
	M4Err e;
	u32 sampleDescIndex;
	TrackAtom *trak;

	trak = GetTrack(moov, GetTrackNumberByID(moov, trackID));
	if (!trak) return M4InvalidMP4File;
	
	e = Media_GetSampleDescIndex(trak->Media, CTS, &sampleDescIndex );
	if (e) return e;
	return GetESD(moov, trackID, sampleDescIndex, outESD);
}


M4Err GetTrackId(MovieAtom *moov, u32 trackNumber, u32 *trackID)
{
	TrackAtom *trak;
	if (!moov) return M4InvalidMP4File;
	if (!trackNumber) return M4BadParam;
	if ( !trackNumber ||!trackID || (trackNumber - 1 >= ChainGetCount(moov->trackList)) ) return M4BadParam;

	trak = (TrackAtom*)ChainGetEntry(moov->trackList, trackNumber - 1);
	if (!trak) return M4BadParam;
	*trackID = trak->Header->trackID;
	return M4OK;
}


M4Err Track_FindRef(TrackAtom *trak, u32 ReferenceType, TrackReferenceTypeAtom **dpnd)
{
	TrackReferenceAtom *ref;
	Atom *a;
	u32 i;
	if (! trak) return M4BadParam;
	if (! trak->References) {
		*dpnd = NULL;
		return M4OK;
	}
	ref = trak->References;
	for (i = 0; i < ChainGetCount(ref->atomList); i++) {
		a = (Atom*)ChainGetEntry(ref->atomList, i);
		if (a->type == ReferenceType) {
			*dpnd = (TrackReferenceTypeAtom *)a;
			return M4OK;
		}
	}
	*dpnd = NULL;
	return M4OK;	
}

Bool Track_IsMPEG4Stream(u32 HandlerType)
{
	switch (HandlerType) {
	case M4_VisualMediaType:
	case M4_AudioMediaType:
	case M4_ODMediaType:
	case M4_OCRMediaType:
	case M4_BIFSMediaType:
	case M4_MPEG7MediaType:
	case M4_OCIMediaType:
	case M4_IPMPMediaType:
	case M4_MPEGJMediaType:
		return 1;
	/*Timedtext is NOT an MPEG-4 stream*/
	default:
		return 0;
	}
}


M4Err SetTrackDuration(TrackAtom *trak)
{
	u64 trackDuration;
	u32 i;
	edtsEntry *ent;
	EditListAtom *elst;
	M4Err e;

	//if we have an edit list, the duration is the sum of all the editList 
	//entries' duration (always expressed in MovieTimeScale)
	if (trak->EditAtom && trak->EditAtom->editList) {
		trackDuration = 0;
		elst = trak->EditAtom->editList;
		for (i = 0; i<ChainGetCount(elst->entryList); i++) {
			ent = (edtsEntry*)ChainGetEntry(elst->entryList, i);
			trackDuration += ent->segmentDuration;
		}
	} else {
		//the total duration is the media duration: adjust it in case...
		e = Media_SetDuration(trak);
		if (e) return e;
		//assert the timeScales are non-NULL
		if (!trak->moov->mvhd->timeScale && !trak->Media->mediaHeader->timeScale) return M4InvalidMP4File;
		trackDuration = (trak->Media->mediaHeader->duration * trak->moov->mvhd->timeScale) / trak->Media->mediaHeader->timeScale;
	}
	trak->Header->duration = trackDuration;
	trak->Header->modificationTime = GetMP4Time();
	return M4OK;
}


#ifndef	M4_ISO_NO_FRAGMENTS

M4Err MergeTrack(TrackAtom *trak, TrackFragmentAtom *traf, u64 *moof_offset)
{
	u32 i, j, chunk_size;
	u64 base_offset, data_offset;
	u32 def_duration, DescIndex, def_size, def_flags;
	u32 duration, size, flags, cts_offset;
	u8 pad, sync;
	u16 degr;
	TrackFragmentRunAtom *trun;
	TrunEntry *ent;

	void stbl_AppendTime(SampleTableAtom *stbl, u32 duration);
	void stbl_AppendSize(SampleTableAtom *stbl, u32 size);
	void stbl_AppendChunk(SampleTableAtom *stbl, u64 offset);
	void stbl_AppendSampleToChunk(SampleTableAtom *stbl, u32 DescIndex, u32 samplesInChunk);
	void stbl_AppendCTSOffset(SampleTableAtom *stbl, u32 CTSOffset);
	void stbl_AppendRAP(SampleTableAtom *stbl, u8 isRap);
	void stbl_AppendPadding(SampleTableAtom *stbl, u8 padding);
	void stbl_AppendDegradation(SampleTableAtom *stbl, u16 DegradationPriority);


	//setup all our defaults
	DescIndex = (traf->tfhd->flags & TF_SAMPLE_DESC) ? traf->tfhd->sample_desc_index : traf->trex->def_sample_desc_index;
	def_duration = (traf->tfhd->flags & TF_SAMPLE_DUR) ? traf->tfhd->def_sample_duration : traf->trex->def_sample_duration;
	def_size = (traf->tfhd->flags & TF_SAMPLE_SIZE) ? traf->tfhd->def_sample_size : traf->trex->def_sample_size;
	def_flags = (traf->tfhd->flags & TF_SAMPLE_FLAGS) ? traf->tfhd->def_sample_flags : traf->trex->def_sample_flags;

	//locate base offset
	base_offset = (traf->tfhd->flags & TF_BASE_OFFSET) ? traf->tfhd->base_data_offset : *moof_offset;

	chunk_size = 0;

	for (i=0; i<ChainGetCount(traf->TrackRuns); i++) {
		trun = ChainGetEntry(traf->TrackRuns, i);
		//merge the run
		for (j=0;j<trun->sample_count; j++) {
			ent = ChainGetEntry(trun->entries, j);
			size = def_size;
			duration = def_duration;
			flags = def_flags;
			
			if (ent) {
				if (trun->flags & TR_DURATION) duration = ent->Duration;
				if (trun->flags & TR_SIZE) size = ent->size;
				if (trun->flags & TR_FLAGS) {
					flags = ent->flags;
				} else if (!j && (trun->flags & TR_FIRST_FLAG)) {
					flags = trun->first_sample_flags;
				}
			}
			//add size first
			stbl_AppendSize(trak->Media->information->sampleTable, size);
			//then TS
			stbl_AppendTime(trak->Media->information->sampleTable, duration);
			//add chunk on first sample
			if (!j) {
				data_offset = base_offset;
				//aggregated offset
				if (!(traf->tfhd->flags & TF_BASE_OFFSET)) data_offset += chunk_size;

				if (trun->flags & TR_DATA_OFFSET) data_offset += trun->data_offset;

				stbl_AppendChunk(trak->Media->information->sampleTable, data_offset);
				//then sampleToChunk
				stbl_AppendSampleToChunk(trak->Media->information->sampleTable, 
					DescIndex, trun->sample_count);
			}
			chunk_size += size;
			

			//CTS
			cts_offset = (trun->flags & TR_CTS_OFFSET) ? ent->CTS_Offset : 0;
			if (cts_offset) stbl_AppendCTSOffset(trak->Media->information->sampleTable, cts_offset);
			
			//flags
			sync = GET_FRAG_SYNC(flags);
			stbl_AppendRAP(trak->Media->information->sampleTable, sync);
			pad = GET_FRAG_PAD(flags);
			if (pad) stbl_AppendPadding(trak->Media->information->sampleTable, pad);
			degr = GET_FRAG_DEG(flags);
			if (degr) stbl_AppendDegradation(trak->Media->information->sampleTable, degr);
		}
	}
	//end of the fragment, update offset
	*moof_offset += chunk_size;
	return M4OK;
}

#endif


#ifndef M4_READ_ONLY

//used to check if a TrackID is available
u8 RequestTrack(MovieAtom *moov, u32 TrackID)
{
	u32 i;
	TrackAtom *trak;

	for (i = 0; i < ChainGetCount(moov->trackList); i++) {
		trak = (TrackAtom*)ChainGetEntry(moov->trackList, i);
		if (trak->Header->trackID == TrackID) {
			M4SetLastError(moov->mov, M4InvalidTrackID);
			return 0;
		}
	}
	return 1;
}

M4Err Track_RemoveRef(TrackAtom *trak, u32 ReferenceType)
{
	TrackReferenceAtom *ref;
	Atom *a;
	u32 i;
	if (! trak) return M4BadParam;
	if (! trak->References) return M4OK;
	ref = trak->References;
	for (i = 0; i < ChainGetCount(ref->atomList); i++) {
		a = (Atom*)ChainGetEntry(ref->atomList, i);
		if (a->type == ReferenceType) {
			DelAtom(a);
			ChainDeleteEntry(ref->atomList, i);
			return M4OK;
		}
	}
	return M4OK;	
}

M4Err NewMedia(MediaAtom **mdia, u32 MediaType, u32 TimeScale)
{
	MediaHeaderAtom *mdhd;
	Atom *mediaInfo;
	HandlerAtom *hdlr;
	MediaInformationAtom *minf;
	DataInformationAtom *dinf;
	SampleTableAtom *stbl;
	SampleDescriptionAtom *stsd;
	DataReferenceAtom *dref;
	char *str;

	M4Err stbl_AddAtom(SampleTableAtom *ptr, Atom *a);
	M4Err mdia_AddAtom(MediaAtom *ptr, Atom *a);
	M4Err dinf_AddAtom(DataInformationAtom *ptr, Atom *a);
	M4Err minf_AddAtom(MediaInformationAtom *ptr, Atom *a);

	M4Err e;

	if (*mdia || !mdia) return M4BadParam;

	e = M4OK;
	minf = NULL;
	mdhd = NULL;
	hdlr = NULL;
	dinf = NULL;
	stbl = NULL;
	stsd = NULL;
	dref = NULL;
	mediaInfo = NULL;

	//first create the media
	*mdia = (MediaAtom *) CreateAtom(MediaAtomType);
	mdhd = (MediaHeaderAtom *) CreateAtom(MediaHeaderAtomType);

	//"handler name" is for debugging purposes. Let's stick our name here ;)
	switch (MediaType) {
	case M4_VisualMediaType:
		mediaInfo = CreateAtom(VideoMediaHeaderAtomType);
		str = "GPAC ISO Video Handler";
		break;
	case M4_AudioMediaType:
		mediaInfo = CreateAtom(SoundMediaHeaderAtomType);
		str = "GPAC ISO Audio Handler";
		break;
	case M4_HintMediaType:
		mediaInfo = CreateAtom(HintMediaHeaderAtomType);
		str = "GPAC ISO Hint Handler";
		break;
	case M4_ODMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 OD Handler";
		break;
	case M4_OCRMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 OCR Handler";
		break;
	case M4_BIFSMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 BIFS Handler";
		break;
	case M4_MPEG7MediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 MPEG-7 Handler";
		break;
	case M4_OCIMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 OCI Handler";
		break;
	case M4_IPMPMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 IPMP Handler";
		break;
	case M4_MPEGJMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC MPEG-4 MPEG-J Handler";
		break;
	case M4_TimedTextMediaType:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC Streaming Text Handler";
		break;
	default:
		mediaInfo = CreateAtom(MPEGMediaHeaderAtomType);
		str = "GPAC IsoMedia Handler";
		break;
	}
	hdlr = (HandlerAtom *) CreateAtom(HandlerAtomType);
	minf = (MediaInformationAtom *) CreateAtom(MediaInformationAtomType);

	mdhd->timeScale = TimeScale;
	hdlr->handlerType = MediaType;

	hdlr->nameLength = strlen(str) + 1;
	hdlr->nameUTF8 = (char*)malloc(sizeof(char) * hdlr->nameLength);
	if (!hdlr->nameUTF8) {
		e = M4OutOfMem;
		goto err_exit;
	}
	strcpy(hdlr->nameUTF8, str);

	//first set-up the sample table...
	stbl = (SampleTableAtom *) CreateAtom(SampleTableAtomType);
	dinf = (DataInformationAtom *) CreateAtom(DataInformationAtomType);
	//add all our sub-atoms in the sample table (only the mandatory ones)
	stbl->SampleDescription = (SampleDescriptionAtom *) CreateAtom(SampleDescriptionAtomType);
	stbl->ChunkOffset = CreateAtom(ChunkOffsetAtomType);
	//by default create a regular table
	stbl->SampleSize = (SampleSizeAtom *) CreateAtom(SampleSizeAtomType);
	stbl->SampleToChunk = (SampleToChunkAtom *) CreateAtom(SampleToChunkAtomType);
	stbl->TimeToSample = (TimeToSampleAtom *) CreateAtom(TimeToSampleAtomType);

	//Create a data reference WITHOUT DATA ENTRY (we don't know anything yet about the media Data)
	dref = (DataReferenceAtom *) CreateAtom(DataReferenceAtomType);
	e = dinf_AddAtom(dinf, (Atom *)dref); if (e) goto err_exit;

	e = minf_AddAtom(minf, (Atom *) mediaInfo); if (e) goto err_exit;
	e = minf_AddAtom(minf, (Atom *) stbl); if (e) goto err_exit;
	e = minf_AddAtom(minf, (Atom *) dinf); if (e) goto err_exit;

	e = mdia_AddAtom(*mdia, (Atom *) mdhd); if (e) goto err_exit;
	e = mdia_AddAtom(*mdia, (Atom *) minf); if (e) goto err_exit;
	e = mdia_AddAtom(*mdia, (Atom *) hdlr); if (e) goto err_exit;

	return M4OK;

err_exit:
	if (mdhd) DelAtom((Atom *)mdhd);
	if (minf) DelAtom((Atom *)minf);
	if (hdlr) {
		if (hdlr->nameUTF8) free(hdlr->nameUTF8);
		DelAtom((Atom *)hdlr);
	}
	return e;

}

M4Err Track_SetStreamDescriptor(TrackAtom *trak, u32 StreamDescriptionIndex, u32 DataReferenceIndex, ESDescriptor *esd, u32 *outStreamIndex)
{
	M4Err e;
	MPEGSampleEntryAtom *entry;
	MPEGVisualSampleEntryAtom *entry_v;
	MPEGAudioSampleEntryAtom *entry_a;
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *dpnd;
	u16 tmpRef;

	M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex);
	M4Err tref_AddAtom(TrackReferenceAtom *ptr, Atom *a);
	M4Err stsd_AddAtom(SampleDescriptionAtom *ptr, Atom *a);
	M4Err trak_AddAtom(TrackAtom *ptr, Atom *a);

	entry = NULL;
	tref = NULL;

	if (!trak || !esd || (!outStreamIndex && !DataReferenceIndex) ) return M4BadParam;
	if (!Track_IsMPEG4Stream(trak->Media->handler->handlerType)) return M4InvalidMP4Media;

	
	esd->ESID = 0;
	//set SL to predefined if no url
	if (esd->URLString == NULL) {
		esd->slConfig->predefined = SLPredef_MP4;
		esd->slConfig->durationFlag = 0;
		esd->slConfig->useTimestampsFlag = 1;
	}

	//get the REF atom if needed
	if (esd->dependsOnESID  || esd->OCRESID ) {
		if (!trak->References) {
			tref = (TrackReferenceAtom *) CreateAtom(TrackReferenceAtomType);
			e = trak_AddAtom(trak, (Atom *)tref);
			if (e) return e;
		}
		tref = trak->References;
	}

	//Update Stream dependancies
	e = Track_FindRef(trak, M4_StreamDependence_Ref, &dpnd);
	if (e) return e;
	if (!dpnd && esd->dependsOnESID) {
		dpnd = (TrackReferenceTypeAtom *) CreateAtom(StreamDependenceAtomType);
		e = tref_AddAtom(tref, (Atom *) dpnd);
		if (e) return e;
		e = reftype_AddRefTrack(dpnd, esd->dependsOnESID, NULL);
		if (e) return e;
	} else if (dpnd && !esd->dependsOnESID) {
		Track_RemoveRef(trak, StreamDependenceAtomType);
	}
	esd->dependsOnESID = 0;

	//Update Clock dependancies
	e = Track_FindRef(trak, M4_OCR_Ref, &dpnd);
	if (e) return e;
	if (!dpnd && esd->OCRESID) {
		dpnd = (TrackReferenceTypeAtom *) CreateAtom(OCRReferenceAtomType);
		e = tref_AddAtom(tref, (Atom *) dpnd);
		if (e) return e;
		e = reftype_AddRefTrack(dpnd, esd->OCRESID, NULL);
		if (e) return e;
	} else if (dpnd && !esd->OCRESID) {
		Track_RemoveRef(trak, OCRReferenceAtomType);
	} else if (dpnd && esd->OCRESID) {
		if (dpnd->trackIDCount != 1) return M4InvalidMP4Media;
		dpnd->trackIDs[0] = esd->OCRESID;
	}
	esd->OCRESID = 0;

	//brand new case: we have to change the IPI desc
	if (esd->ipiPtr) {
		e = Track_FindRef(trak, M4_IPI_Ref, &dpnd);
		if (e) return e;
		if (!dpnd) {
			tmpRef = 0;
			dpnd = (TrackReferenceTypeAtom *) CreateAtom(IPIReferenceAtomType);
			e = tref_AddAtom(tref, (Atom *) dpnd);
			if (e) return e;
			e = reftype_AddRefTrack(dpnd, esd->ipiPtr->IPI_ES_Id, &tmpRef);
			if (e) return e;
			//and replace the tag and value...
			esd->ipiPtr->IPI_ES_Id = tmpRef;
			esd->ipiPtr->tag = IPI_DescPtr_Tag;
		} else {
			//Watch out! ONLY ONE IPI dependancy is allowed per stream
			if (dpnd->trackIDCount != 1) return M4InvalidMP4Media;
			//if an existing one is there, what shall we do ???
			//donno, erase it
			dpnd->trackIDs[0] = esd->ipiPtr->IPI_ES_Id;
			//and replace the tag and value...
			esd->ipiPtr->IPI_ES_Id = 1;
			esd->ipiPtr->tag = IPI_DescPtr_Tag;
		}
	}

	//we have a streamDescritpionIndex, use it
	if (StreamDescriptionIndex) {
		entry = (MPEGSampleEntryAtom*)ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex - 1);
		if (!entry) return M4InvalidMP4File;

		switch (entry->type) {
		case MPEGSampleEntryAtomType:
			//OK, delete the previous ESD
			e = OD_DeleteDescriptor((Descriptor **) &entry->esd->desc);
			if (e) return e;
			entry->esd->desc = esd;
			break;
		case MPEGVisualSampleEntryAtomType:
			entry_v = (MPEGVisualSampleEntryAtom*) entry;
			//OK, delete the previous ESD
			e = OD_DeleteDescriptor((Descriptor **) &entry_v->esd->desc);
			if (e) return e;
			entry_v->esd->desc = esd;
			break;
		case MPEGAudioSampleEntryAtomType:
			entry_a = (MPEGAudioSampleEntryAtom*) entry;
			//OK, delete the previous ESD
			e = OD_DeleteDescriptor((Descriptor **) &entry_a->esd->desc);
			if (e) return e;
			entry_a->esd->desc = esd;
			break;
		}
	} else {
		//need to check we're not in URL mode where only ONE description is allowed...
		StreamDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
		if (StreamDescriptionIndex) {
			entry = (MPEGSampleEntryAtom*)ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, StreamDescriptionIndex - 1);
			if (!entry) return M4InvalidMP4File;
			if (entry->esd->desc->URLString) return M4BadParam;
		}

		//OK, check the handler and create the entry
		switch (trak->Media->handler->handlerType) {
		case M4_VisualMediaType:
			entry_v = (MPEGVisualSampleEntryAtom *) CreateAtom(MPEGVisualSampleEntryAtomType);
			if (!entry_v) return M4OutOfMem;
			//create an esdAtom
			entry_v->esd = (ESDAtom *) CreateAtom(ESDAtomType);
			entry_v->esd->desc = esd;

			//type cast possible now
			entry = (MPEGSampleEntryAtom*) entry_v;
			break;
		case M4_AudioMediaType:
			entry_a = (MPEGAudioSampleEntryAtom *) CreateAtom(MPEGAudioSampleEntryAtomType);
			entry_a->samplerate_hi = trak->Media->mediaHeader->timeScale;
			if (!entry_a) return M4OutOfMem;
			//create an esdAtom
			entry_a->esd = (ESDAtom *) CreateAtom(ESDAtomType);
			entry_a->esd->desc = esd;
			//type cast possible now
			entry = (MPEGSampleEntryAtom*) entry_a;
			break;
		case M4_ODMediaType:
		case M4_OCRMediaType:
		case M4_BIFSMediaType:
		case M4_MPEG7MediaType:
		case M4_OCIMediaType:
		case M4_IPMPMediaType:
		case M4_MPEGJMediaType:
			entry = (MPEGSampleEntryAtom *) CreateAtom(MPEGSampleEntryAtomType);
			//create an esdAtom
			entry->esd = (ESDAtom *) CreateAtom(ESDAtomType);
			entry->esd->desc = esd;
			break;
		}
		entry->dataReferenceIndex = DataReferenceIndex;

		//and add the entry to our table...
		e = stsd_AddAtom(trak->Media->information->sampleTable->SampleDescription, (Atom *) entry);
		if (e) return e;
		if(outStreamIndex) *outStreamIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);
	}
	return M4OK;
}

#endif	//M4_READ_ONLY

