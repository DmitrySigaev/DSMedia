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

//Get the sample number
M4Err findEntryForTime(SampleTableAtom *stbl, u32 DTS, u8 useCTS, u32 *sampleNumber, u32 *prevSampleNumber)
{
	u32 i, j, curDTS, curSampNum, CTSOffset;
	sttsEntry *ent;
	(*sampleNumber) = 0;
	(*prevSampleNumber) = 0;

	if (!stbl->CompositionOffset) useCTS = 0;

	//our cache
	if (stbl->TimeToSample->r_FirstSampleInEntry &&
		(DTS >= stbl->TimeToSample->r_CurrentDTS) ) {
		//if we're using CTS, we don't really know whether we're in the good entry or not
		//(eg, the real DTS of the sample could be in a previous entry
		i = stbl->TimeToSample->r_currentEntryIndex;
		curDTS = stbl->TimeToSample->r_CurrentDTS;
		curSampNum = stbl->TimeToSample->r_FirstSampleInEntry;
	} else {
		i = 0;
		curDTS = stbl->TimeToSample->r_CurrentDTS = 0;
		curSampNum = stbl->TimeToSample->r_FirstSampleInEntry = 1;
		stbl->TimeToSample->r_currentEntryIndex = 0;
	}

	//we need to validate our cache if we are using CTS because of B-frames and co...
	if (i && useCTS) {
		while (1) {
			stbl_GetSampleCTS(stbl->CompositionOffset, curSampNum, &CTSOffset);
			//we're too far, rewind
			if ( i && (curDTS + CTSOffset > DTS) ) {
				ent = (sttsEntry*)ChainGetEntry(stbl->TimeToSample->entryList, i);
				curSampNum -= ent->sampleCount;
				curDTS -= ent->sampleDelta * ent->sampleCount;
				i --;
			} else if (!i) {
				//begining of the table, no choice
				curDTS = stbl->TimeToSample->r_CurrentDTS = 0;
				curSampNum = stbl->TimeToSample->r_FirstSampleInEntry = 1;
				stbl->TimeToSample->r_currentEntryIndex = 0;
				break;
			} else {
				//OK now we're good
				break;
			}
		}
	}

	//look for the DTS from this entry
	for (; i<ChainGetCount(stbl->TimeToSample->entryList); i++) {
		ent = (sttsEntry*)ChainGetEntry(stbl->TimeToSample->entryList, i);
		if (useCTS) {
			stbl_GetSampleCTS(stbl->CompositionOffset, curSampNum, &CTSOffset);
		} else {
			CTSOffset = 0;
		}
		for (j=0; j<ent->sampleCount; j++) {
			if (curDTS + CTSOffset >= DTS) goto entry_found;
			curSampNum += 1;
			curDTS += ent->sampleDelta;
		}
		//we're switching to the next entry, update the cache!
		stbl->TimeToSample->r_CurrentDTS += ent->sampleCount * ent->sampleDelta;
		stbl->TimeToSample->r_currentEntryIndex += 1;
		stbl->TimeToSample->r_FirstSampleInEntry += ent->sampleCount;
	}
	//return as is
	return M4OK;

entry_found:
	//do we have the exact time ?
	if (curDTS + CTSOffset == DTS) {
		(*sampleNumber) = curSampNum;
	}
	//if we match the exact DTS also select this sample 
	else if (curDTS == DTS) {
		(*sampleNumber) = curSampNum;
	} else {
		//exception for the first sample (we need to "load" the playback)
		if (curSampNum != 1) {
			(*prevSampleNumber) = curSampNum - 1;
		} else {
			(*prevSampleNumber) = 1;
		}
	}
	return M4OK;
}

//Get the Size of a given sample
M4Err stbl_GetSampleSize(SampleSizeAtom *stsz, u32 SampleNumber, u32 *Size)
{
	if (!stsz || !SampleNumber || SampleNumber > stsz->sampleCount) return M4BadParam;

	(*Size) = 0;

	if (stsz->sampleSize && (stsz->type != CompactSampleSizeAtomType)) {
		(*Size) = stsz->sampleSize;
	} else {
		(*Size) = stsz->sizes[SampleNumber - 1];
	}
	return M4OK;
}



//Get the CTS offset of a given sample
M4Err stbl_GetSampleCTS(CompositionOffsetAtom *ctts, u32 SampleNumber, u32 *CTSoffset)
{
	u32 i, count;
	dttsEntry *ent;

	(*CTSoffset) = 0;
	ent = NULL;
	//test on SampleNumber is done before
	if (!ctts || !SampleNumber) return M4BadParam;

	if (ctts->r_FirstSampleInEntry && (ctts->r_FirstSampleInEntry < SampleNumber) ) {
		i = ctts->r_currentEntryIndex;
	} else {
		ctts->r_FirstSampleInEntry = 1;
		ctts->r_currentEntryIndex = 0;
		i = 0;
	}
	count = ChainGetCount(ctts->entryList);
	for (; i< count; i++) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, i);
		if (SampleNumber < ctts->r_FirstSampleInEntry + ent->sampleCount) break;
		//update our cache
		ctts->r_currentEntryIndex += 1;
		ctts->r_FirstSampleInEntry += ent->sampleCount;
	}
	//no ent, set everything to 0...
	if (!ent) return M4OK;
	/*asked for a sample not in table - this means CTTS is 0 (that's due to out internal packing construction of CTTS)*/
	if (SampleNumber >= ctts->r_FirstSampleInEntry + ent->sampleCount) return M4OK;
	(*CTSoffset) = ent->decodingOffset;
	return M4OK;
}

//Get the DTS of a sample
M4Err stbl_GetSampleDTS(TimeToSampleAtom *stts, u32 SampleNumber, u32 *DTS)
{
	u32 i, j, count;
	sttsEntry *ent;

	(*DTS) = 0;
	if (!stts || !SampleNumber) return M4BadParam;

	ent = NULL;
	//use our cache
	count = ChainGetCount(stts->entryList);
	if (stts->r_FirstSampleInEntry 
		&& (stts->r_FirstSampleInEntry <= SampleNumber)
		//this is for read/write access
		&& (stts->r_currentEntryIndex < count) ) {

		i = stts->r_currentEntryIndex;
	} else {
		i = stts->r_currentEntryIndex = 0;
		stts->r_FirstSampleInEntry = 1;
		stts->r_CurrentDTS = 0;
	}

	for (; i < count; i++) {
		ent = (sttsEntry*)ChainGetEntry(stts->entryList, i);

		//in our entry
		if (ent->sampleCount + stts->r_FirstSampleInEntry >= 1 + SampleNumber) {
			j = SampleNumber - stts->r_FirstSampleInEntry;
			goto found;
		}
			
		//update our cache
		stts->r_CurrentDTS += ent->sampleCount * ent->sampleDelta;
		stts->r_currentEntryIndex += 1;
		stts->r_FirstSampleInEntry += ent->sampleCount;
	}
//	if (SampleNumber >= stts->r_FirstSampleInEntry + ent->sampleCount) return M4BadParam;

	//no ent, this is really weird. Let's assume the DTS is then what is written in the table
	if (!ent || (i == count)) (*DTS) = stts->r_CurrentDTS;
	return M4OK;

found:
	(*DTS) = stts->r_CurrentDTS + j * ent->sampleDelta;

	if (stts->r_FirstSampleInEntry == 1)
		stts->r_FirstSampleInEntry = 1;

	
	
	return M4OK;
}

//Set the RAP flag of a sample
M4Err stbl_GetSampleRAP(SyncSampleAtom *stss, u32 SampleNumber, u8 *IsRAP, u32 *prevRAP, u32 *nextRAP)
{
	u32 i;

	if (prevRAP) *prevRAP = 0;
	if (nextRAP) *nextRAP = 0;

	(*IsRAP) = 0;
	if (!stss || !SampleNumber) return M4BadParam;

	if (stss->r_LastSyncSample && (stss->r_LastSyncSample < SampleNumber) ) {
		i = stss->r_LastSampleIndex;
	} else {
		i = 0;
	}
	for (; i < stss->entryCount; i++) {
		//get the entry
		if (stss->sampleNumbers[i] == SampleNumber) {
			//update the cache
			stss->r_LastSyncSample = SampleNumber;
			stss->r_LastSampleIndex = i;
			(*IsRAP) = 1;
		}
		else if (stss->sampleNumbers[i] > SampleNumber) {
			if (nextRAP) *nextRAP = stss->sampleNumbers[i];
			return M4OK;
		}
		if (prevRAP) *prevRAP = stss->sampleNumbers[i];
	}
	return M4OK;
}

//get the number of "ghost chunk" (implicit chunks described by an entry)
void GetGhostNum(stscEntry *ent, u32 EntryIndex, u32 count, SampleTableAtom *stbl)
{
	stscEntry *nextEnt;
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;
	u32 ghostNum = 1;

	if (!ent->nextChunk) {
		if (EntryIndex+1 == count) {
			//not specified in the spec, what if the last sample to chunk is no written?
			if (stbl->ChunkOffset->type == ChunkOffsetAtomType) {
				stco = (ChunkOffsetAtom *)stbl->ChunkOffset;
				ghostNum = (stco->entryCount > ent->firstChunk) ? (1 + stco->entryCount - ent->firstChunk) : 1;
			} else {
				co64 = (ChunkLargeOffsetAtom *)stbl->ChunkOffset;
				ghostNum = (co64->entryCount > ent->firstChunk) ? (1 + co64->entryCount - ent->firstChunk) : 1;
			}
		} else {
			//this is an unknown case due to edit mode...
			nextEnt = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, EntryIndex+1);
			ghostNum = nextEnt->firstChunk - ent->firstChunk;
		}
	} else {
		ghostNum = (ent->nextChunk > ent->firstChunk) ? (ent->nextChunk - ent->firstChunk) : 1;
	}
	stbl->SampleToChunk->ghostNumber = ghostNum;
}

//Get the offset, descIndex and chunkNumber of a sample...
M4Err stbl_GetSampleInfos(SampleTableAtom *stbl, u32 sampleNumber, u64 *offset, u32 *chunkNumber, u32 *descIndex, u8 *isEdited)
{
	M4Err e;
	u32 i, j, k, offsetInChunk, size, count;
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;
	stscEntry *ent;

	(*offset) = 0;
	(*chunkNumber) = (*descIndex) = 0;
	(*isEdited) = 0;
	if (!stbl || !sampleNumber) return M4BadParam;

	count = ChainGetCount(stbl->SampleToChunk->entryList);

	//check our cache
	if (stbl->SampleToChunk->firstSampleInCurrentChunk &&
		(stbl->SampleToChunk->firstSampleInCurrentChunk <= sampleNumber)) {
		
		i = stbl->SampleToChunk->currentIndex;
//		ent = ChainGetEntry(stbl->SampleToChunk->entryList, i);
		ent = stbl->SampleToChunk->currentEntry;
		GetGhostNum(ent, i, count, stbl);
		k = stbl->SampleToChunk->currentChunk;
	} else {
		i = 0;
		stbl->SampleToChunk->currentIndex = 0;
		stbl->SampleToChunk->currentChunk = 1;
		stbl->SampleToChunk->firstSampleInCurrentChunk = 1;
		ent = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, 0);
		stbl->SampleToChunk->currentEntry = ent;
		GetGhostNum(ent, 0, count, stbl);
		k = stbl->SampleToChunk->currentChunk;
	}

	//first get the chunk
	for (; i < count; i++) {
		//browse from the current chunk we're browsing from index 1
		for (; k <= stbl->SampleToChunk->ghostNumber; k++) {
			//browse all the samples in this chunk
			for (j = 0; j < ent->samplesPerChunk; j++) {
				//ok, this is our sample
				if (stbl->SampleToChunk->firstSampleInCurrentChunk + j == sampleNumber ) 
					goto sample_found;
			}
			//nope, get to next chunk
			stbl->SampleToChunk->firstSampleInCurrentChunk += ent->samplesPerChunk;
			stbl->SampleToChunk->currentChunk ++;
		}
		//not in this entry, get the next entry if not the last one
		if (i+1 != count) {
			ent = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, i+1);
			//update the GhostNumber
			GetGhostNum(ent, i+1, count, stbl);
			//update the entry in our cache
			stbl->SampleToChunk->currentEntry = ent;
			stbl->SampleToChunk->currentIndex = i+1;
			stbl->SampleToChunk->currentChunk = 1;
			k = 1;
		}
	}
	//if we get here, gasp, the sample was not found
	return M4InvalidMP4File;

sample_found:

	(*descIndex) = ent->sampleDescriptionIndex;
	(*chunkNumber) = ent->firstChunk + stbl->SampleToChunk->currentChunk - 1;
	(*isEdited) = ent->isEdited;

	//ok, get the size of all the previous sample
	offsetInChunk = 0;
	//warning, firstSampleInChunk is at least 1 - not 0
	for (i = stbl->SampleToChunk->firstSampleInCurrentChunk; i < sampleNumber; i++) {
		e = stbl_GetSampleSize(stbl->SampleSize, i, &size);
		if (e) return e;
		offsetInChunk += size;
	}
	//OK, that's the size of our offset in the chunk
	//now get the chunk
	if ( stbl->ChunkOffset->type == ChunkOffsetAtomType) {
		stco = (ChunkOffsetAtom *)stbl->ChunkOffset;
		if (stco->entryCount < (*chunkNumber) ) return M4InvalidMP4File;
		(*offset) = (u64) stco->offsets[(*chunkNumber) - 1] + (u64) offsetInChunk;
	} else {
		co64 = (ChunkLargeOffsetAtom *)stbl->ChunkOffset;
		if (co64->entryCount < (*chunkNumber) ) return M4InvalidMP4File;
		(*offset) = co64->offsets[(*chunkNumber) - 1] + (u64) offsetInChunk;
	}
	return M4OK;
}


M4Err stbl_GetSampleShadow(ShadowSyncAtom *stsh, u32 *sampleNumber, u32 *syncNum)
{
	u32 i;
	stshEntry *ent;

	if (stsh->r_LastFoundSample && (stsh->r_LastFoundSample <= *sampleNumber)) {
		i = stsh->r_LastEntryIndex;
	} else {
		i = 0;
		stsh->r_LastFoundSample = 1;
	}

	ent = NULL;
	(*syncNum) = 0;

	for (; i<ChainGetCount(stsh->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(stsh->entries, i);
		//we get the exact desired sample !
		if (ent->shadowedSampleNumber == *sampleNumber) {
			(*syncNum) = ent->syncSampleNumber;
			stsh->r_LastFoundSample = *sampleNumber;
			stsh->r_LastEntryIndex = i;
			return M4OK;
		} else if (ent->shadowedSampleNumber > *sampleNumber) {
			//do we have an entry before ? If not, there is no shadowing available
			//for this sample
			if (!i) return M4OK;
			//ok, indicate the previous ShadowedSample
			ent = (stshEntry*)ChainGetEntry(stsh->entries, i-1);
			(*syncNum) = ent->syncSampleNumber;
			//change the sample number
			(*sampleNumber) = ent->shadowedSampleNumber;
			//reset the cache to the last ShadowedSample
			stsh->r_LastEntryIndex = i-1;
			stsh->r_LastFoundSample = ent->shadowedSampleNumber;
		}
	}
	stsh->r_LastEntryIndex = i-1;
	stsh->r_LastFoundSample = ent ? ent->shadowedSampleNumber : 0;
	return M4OK;
}



M4Err stbl_GetPaddingBits(PaddingBitsAtom *padb, u32 SampleNumber, u8 *PadBits)
{
	if (!PadBits) return M4BadParam;
	*PadBits = 0;
	if (!padb || !padb->padbits) return M4OK;
	//the spec says "should" not shall. return 0 padding 
	if (padb->SampleCount < SampleNumber) return M4OK;
	*PadBits = padb->padbits[SampleNumber-1];
	return M4OK;
}


u32 stbl_GetSampleFragmentCount(SampleFragmentAtom *stsf, u32 sampleNumber)
{
	stsfEntry *ent;
	u32 i;
	if (!stsf) return 0;

	//check cache
	if (!stsf->r_currentEntry || (stsf->r_currentEntry->SampleNumber < sampleNumber)) {
		stsf->r_currentEntry = NULL;
		stsf->r_currentEntryIndex = 0;
	}
	i = stsf->r_currentEntryIndex;
	
	for (; i<ChainGetCount(stsf->entryList); i++) {
		ent = ChainGetEntry(stsf->entryList, i);
		if (ent->SampleNumber == sampleNumber) {
			stsf->r_currentEntry = ent;
			stsf->r_currentEntryIndex = i;
			return ent->fragmentCount;
		}
	}
	//not found
	return 0;
}

u32 stbl_GetSampleFragmentSize(SampleFragmentAtom *stsf, u32 sampleNumber, u32 FragmentIndex)
{
	stsfEntry *ent;
	u32 i;
	if (!stsf || !FragmentIndex) return 0;

	//check cache
	if (!stsf->r_currentEntry || (stsf->r_currentEntry->SampleNumber < sampleNumber)) {
		stsf->r_currentEntry = NULL;
		stsf->r_currentEntryIndex = 0;
	}
	i = stsf->r_currentEntryIndex;
	
	for (; i<ChainGetCount(stsf->entryList); i++) {
		ent = ChainGetEntry(stsf->entryList, i);
		if (ent->SampleNumber == sampleNumber) {
			stsf->r_currentEntry = ent;
			stsf->r_currentEntryIndex = i;
			if (FragmentIndex > ent->fragmentCount) return 0;
			return ent->fragmentSizes[FragmentIndex - 1];
		}
	}
	//not found
	return 0;
}

