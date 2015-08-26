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

//adds a DTS in the table and get the sample number of this new sample
//we could return an error if a sample with the same DTS already exists
//but this is not true for QT or MJ2K, only for MP4...
//we assume the authoring tool tries to create a compliant MP4 file.
M4Err stbl_AddDTS(SampleTableAtom *stbl, u32 DTS, u32 *sampleNumber, u32 LastAUDefDuration)
{
	u32 i, j, sampNum, curDTS;
	u32 *DTSs, *newDTSs;
	sttsEntry *ent;

	TimeToSampleAtom *stts = stbl->TimeToSample;

	//We don't update the reading cache when adding a sample

	*sampleNumber = 0;
	//if we don't have an entry, that's the first one...
	if (! ChainGetCount(stts->entryList)) {
		//assert the first DTS is 0. If not, that will break the whole file
		if (DTS) return M4BadParam;
		ent = (sttsEntry*)malloc(sizeof(sttsEntry));
		if (!ent) return M4OutOfMem;
		ent->sampleCount = 1;
		ent->sampleDelta = LastAUDefDuration;
		stts->w_currentEntry = ent;
		stts->w_currentSampleNum = (*sampleNumber) = 1;
		return ChainAddEntry(stts->entryList, ent);
	}

	//check the last DTS...
	if (DTS > stts->w_LastDTS) {
		//OK, we're adding at the end
		if (DTS == stts->w_LastDTS + stts->w_currentEntry->sampleDelta) {
			stts->w_currentEntry->sampleCount ++;
			stts->w_currentSampleNum ++;
			(*sampleNumber) = stts->w_currentSampleNum;
			stts->w_LastDTS = DTS;
			return M4OK;
		}
		//we need to split the entry
		if (stts->w_currentEntry->sampleCount == 1) {
			//use this one and adjust...
			stts->w_currentEntry->sampleDelta = DTS - stts->w_LastDTS;
			stts->w_currentEntry->sampleCount ++;
			stts->w_currentSampleNum ++;
			stts->w_LastDTS = DTS;
			(*sampleNumber) = stts->w_currentSampleNum;
			return M4OK;
		}
		//we definitely need to split the entry ;)
		stts->w_currentEntry->sampleCount --;
		ent = (sttsEntry*)malloc(sizeof(sttsEntry));
		ent->sampleCount = 2;
		ent->sampleDelta = DTS - stts->w_LastDTS;
		stts->w_LastDTS = DTS;
		stts->w_currentSampleNum ++;
		(*sampleNumber) = stts->w_currentSampleNum;
		stts->w_currentEntry = ent;
		return ChainAddEntry(stts->entryList, ent);
	}


	//unpack the DTSs...
	DTSs = (u32*)malloc(sizeof(u32) * stbl->SampleSize->sampleCount);
	curDTS = 0;
	sampNum = 0;
	ent = NULL;
	for (i=0; i < ChainGetCount(stts->entryList); i++) {
		ent = (sttsEntry*)ChainGetEntry(stts->entryList, i);
		for (j = 0; j<ent->sampleCount; j++) {
			DTSs[sampNum] = curDTS;
			curDTS += ent->sampleDelta;
			sampNum ++;
		}
	}
	//delete the table..
	while (ChainGetCount(stts->entryList)) {
		ent = (sttsEntry*)ChainGetEntry(stts->entryList, 0);
		free(ent);
		ChainDeleteEntry(stts->entryList, 0);
	}

	//create the new DTSs
	newDTSs = (u32*)malloc(sizeof(u32) * (stbl->SampleSize->sampleCount + 1));
	i = 0;
	while (i < stbl->SampleSize->sampleCount) {
		if (DTSs[i] > DTS) break;
		newDTSs[i] = DTSs[i];
		i++;
	}
	//if we add a sample with the same DTS as an existing one, it's added after.
	newDTSs[i] = DTS;
	*sampleNumber = i+1;
	for (; i<stbl->SampleSize->sampleCount; i++) {
		newDTSs[i+1] = DTSs[i];
	}
	free(DTSs);

	//rewrite the table
	ent = (sttsEntry*)malloc(sizeof(sttsEntry));
	ent->sampleCount = 0;
	ent->sampleDelta = newDTSs[1];
	i = 0;
	while (1) {
		if (i == stbl->SampleSize->sampleCount) {
			//and by default, our last sample has the same delta as the prev
			ent->sampleCount++;
			ChainAddEntry(stts->entryList, ent);
			break;
		}
		if (newDTSs[i+1] - newDTSs[i] == ent->sampleDelta) {
			ent->sampleCount += 1;
		} else {
			ChainAddEntry(stts->entryList, ent);
			ent = (sttsEntry*)malloc(sizeof(sttsEntry));
			ent->sampleCount = 1;
			ent->sampleDelta = newDTSs[i+1] - newDTSs[i];
		}
		i++;
	}
	free(newDTSs);
	//reset the cache to the end
	stts->w_currentEntry = ent;
	stts->w_currentSampleNum = stbl->SampleSize->sampleCount + 1;
	return M4OK;
}

M4Err AddCompositionOffset(CompositionOffsetAtom *ctts, u32 offset)
{
	dttsEntry *entry;
	if (!ctts) return M4BadParam;

	entry = ctts->w_currentEntry;
	if ( (entry == NULL) || (entry->decodingOffset != offset) ) {
		entry = (dttsEntry *) malloc(sizeof(dttsEntry));
		if (!entry) return M4OutOfMem;
		entry->sampleCount = 1;
		entry->decodingOffset = offset;
		ChainAddEntry(ctts->entryList, entry);
		ctts->w_currentEntry = entry;
	} else {
		entry->sampleCount++;
	}
	ctts->w_LastSampleNumber++;
	return M4OK;
}

//adds a CTS offset for a new sample
M4Err stbl_AddCTS(SampleTableAtom *stbl, u32 sampleNumber, u32 CTSoffset)
{
	dttsEntry *ent;
	u32 i, j, sampNum, *CTSs, *newCTSs;

	CompositionOffsetAtom *ctts = stbl->CompositionOffset;

	if (ctts->unpack_mode) {
		ent = (dttsEntry *) malloc(sizeof(dttsEntry));
		if (!ent) return M4OutOfMem;
		ent->sampleCount = 1;
		ent->decodingOffset = CTSoffset;
		return ChainAddEntry(ctts->entryList, ent);
	}

	//check if we're working in order...
	if (ctts->w_LastSampleNumber < sampleNumber) {
		//add some 0 till we get to the sample
		while (ctts->w_LastSampleNumber + 1 != sampleNumber) {
			AddCompositionOffset(ctts, 0);
		}
		return AddCompositionOffset(ctts, CTSoffset);
	}

	//NOPE we are inserting a sample...
	CTSs = (u32*)malloc(sizeof(u32) * stbl->SampleSize->sampleCount);
	sampNum = 0;
	for (i=0; i<ChainGetCount(ctts->entryList); i++) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, i);
		for (j = 0; j<ent->sampleCount; j++) {
			CTSs[sampNum] = ent->decodingOffset;
			sampNum ++;
		}
	}
	
	//delete the entries
	while (ChainGetCount(ctts->entryList)) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, 0);
		free(ent);
		ChainDeleteEntry(ctts->entryList, 0);
	}
	
	//create the new CTS
	newCTSs = (u32*)malloc(sizeof(u32) * (stbl->SampleSize->sampleCount + 1));
	j = 0;
	for (i = 0; i < stbl->SampleSize->sampleCount; i++) {
		if (i+1 == sampleNumber) {
			newCTSs[i] = CTSoffset;
			j = 1;
		}
		newCTSs[i+j] = CTSs[i];
	}
	free(CTSs);

	//rewrite the table
	ent = (dttsEntry*)malloc(sizeof(dttsEntry));
	ent->sampleCount = 1;
	ent->decodingOffset = newCTSs[0];
	i = 1;
	while (1) {
		if (i == stbl->SampleSize->sampleCount) {
			ChainAddEntry(ctts->entryList, ent);
			break;
		}
		if (newCTSs[i] == ent->decodingOffset) {
			ent->sampleCount += 1;
		} else {
			ChainAddEntry(ctts->entryList, ent);
			ent = (dttsEntry*)malloc(sizeof(dttsEntry));
			ent->sampleCount = 1;
			ent->decodingOffset = newCTSs[i];
		}
		i++;
	}
	free(newCTSs);
	//reset the cache to the end
	ctts->w_currentEntry = ent;
	//we've inserted a sample, therefore the last sample (n) has now number n+1
	//we cannot use SampleCount because we have probably skipped some samples
	//(we're calling AddCTS only if the sample gas a CTSOffset !!!)
	ctts->w_LastSampleNumber += 1;
	return M4OK;
}

M4Err stbl_repackCTS(CompositionOffsetAtom *ctts, Bool adjust_cts)
{
	dttsEntry *entry, *next, *ins;
	u32 i, count;
	ctts->unpack_mode = 0;

	count = ChainGetCount(ctts->entryList);
	if (!count) return M4OK;
	entry = ChainGetEntry(ctts->entryList, 0);
	i=1;
	while (i<count) {
		next = ChainGetEntry(ctts->entryList, i);
		if (entry->decodingOffset != next->decodingOffset) {
			/*these are B-frames. Adjust prev entry*/
			if (adjust_cts && !next->decodingOffset) {
				/*repack following B*/
				while (i+1<count) {
					ins = ChainGetEntry(ctts->entryList, i+1);
					if (ins->decodingOffset) break;
					next->sampleCount++;
					ChainDeleteEntry(ctts->entryList, i+1);
					free(ins);
					count--;
				}
				
				if (entry->sampleCount==1) {
					entry->decodingOffset = entry->decodingOffset * (1+next->sampleCount);
				} else {
					entry->sampleCount--;
					ins = malloc(sizeof(dttsEntry));
					ins->sampleCount = 1;
					ins->decodingOffset = entry->decodingOffset * (1+next->sampleCount);
					ChainInsertEntry(ctts->entryList, ins, i);
					i++;
					count++;
				}
			}
			entry = next;
			i++;
		} else {
			entry->sampleCount += next->sampleCount;
			ChainDeleteEntry(ctts->entryList, i);
			free(next);
			count--;
		}
	} 
	return M4OK;
}


//add size
M4Err stbl_AddSize(SampleSizeAtom *stsz, u32 sampleNumber, u32 size)
{
	u32 i, k;
	u32 *newSizes;
	if (!stsz || !size || !sampleNumber) return M4BadParam;

	if (sampleNumber > stsz->sampleCount + 1) return M4BadParam;

	//all samples have the same size
	if (stsz->sizes == NULL) {
		//1 first sample added in NON COMPACT MODE
		if (! stsz->sampleCount && (stsz->type != CompactSampleSizeAtomType) ) {
			stsz->sampleCount = 1;
			stsz->sampleSize = size;
			return M4OK;
		}
		//2- sample has the same size
		if (stsz->sampleSize == size) {
			stsz->sampleCount++;
			return M4OK;
		}
		//3- no, need to alloc a size table
		stsz->sizes = malloc(sizeof(u32) * (stsz->sampleCount + 1));
		if (!stsz->sizes) return M4OutOfMem;
		stsz->alloc_size = stsz->sampleCount + 1;

		k = 0;
		for (i = 0 ; i < stsz->sampleCount; i++) {
			if (i + 1 == sampleNumber) {
				stsz->sizes[i + k] = size;
				k = 1;
			}
			stsz->sizes[i+k] = stsz->sampleSize;
		}
		//this if we append a new sample
		if (stsz->sampleCount + 1 == sampleNumber) {
			stsz->sizes[stsz->sampleCount] = size;
		}
		stsz->sampleSize = 0;
		stsz->sampleCount++;
		return M4OK;
	}


	/*append*/
	if (stsz->sampleCount + 1 == sampleNumber) {
		if (!stsz->alloc_size) stsz->alloc_size = stsz->sampleCount;
		if (stsz->sampleCount == stsz->alloc_size) {
			stsz->alloc_size += 50;
			newSizes = (u32*)malloc(sizeof(u32)*(stsz->alloc_size) );
			if (!newSizes) return M4OutOfMem;
			memcpy(newSizes, stsz->sizes, sizeof(u32)*stsz->sampleCount);
			free(stsz->sizes);
			stsz->sizes = newSizes;
		}
		stsz->sizes[stsz->sampleCount] = size;
	} else {
		newSizes = (u32*)malloc(sizeof(u32)*(1 + stsz->sampleCount) );
		if (!newSizes) return M4OutOfMem;
		k = 0;
		for (i = 0; i < stsz->sampleCount; i++) {
			if (i + 1 == sampleNumber) {
				newSizes[i + k] = size;
				k = 1;
			}
			newSizes[i + k] = stsz->sizes[i];
		}
		free(stsz->sizes);
		stsz->sizes = newSizes;
	}
	stsz->sampleCount++;
	return M4OK;
}


M4Err stbl_AddRAP(SyncSampleAtom *stss, u32 sampleNumber)
{
	u32 i, k;
	u32 *newNumbers;

	if (!stss || !sampleNumber) return M4BadParam;

	if (stss->sampleNumbers == NULL) {
		stss->sampleNumbers = (u32*)malloc(sizeof(u32));
		if (!stss->sampleNumbers) return M4OutOfMem;
		stss->sampleNumbers[0] = sampleNumber;
		stss->entryCount = 1;
		return M4OK;
	}

	newNumbers = (u32*)malloc(sizeof(u32) * (stss->entryCount + 1));
	if (!newNumbers) return M4OutOfMem;
	if (stss->sampleNumbers[stss->entryCount-1] < sampleNumber) {
		memcpy(newNumbers, stss->sampleNumbers, sizeof(u32)*stss->entryCount);
		newNumbers[stss->entryCount] = sampleNumber;
	} else {
		//the table is in increasing order of sampleNumber
		k = 0;
		for (i = 0; i < stss->entryCount; i++) {
			if (stss->sampleNumbers[i] >= sampleNumber) {
				newNumbers[i + k] = sampleNumber;
				k = 1;
			}
			newNumbers[i + k] = stss->sampleNumbers[i] + k;
		}
	}
	free(stss->sampleNumbers);
	stss->sampleNumbers = newNumbers;
	//update our list
	stss->entryCount ++;
	return M4OK;
}

//this function is always called in INCREASING order of shadow sample numbers
M4Err stbl_AddShadow(ShadowSyncAtom *stsh, u32 sampleNumber, u32 shadowNumber)
{
	stshEntry *ent;
	u32 i;
	for (i=0; i<ChainGetCount(stsh->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(stsh->entries, i);
		if (ent->shadowedSampleNumber == shadowNumber) {
			ent->syncSampleNumber = sampleNumber;
			return M4OK;
		} else if (ent->shadowedSampleNumber > shadowNumber) break;
	}
	ent = (stshEntry*)malloc(sizeof(stshEntry));
	if (!ent) return M4OutOfMem;
	ent->shadowedSampleNumber = shadowNumber;
	ent->syncSampleNumber = sampleNumber;
	if (i == ChainGetCount(stsh->entries)) {
		return ChainAddEntry(stsh->entries, ent);
	} else {
		return ChainInsertEntry(stsh->entries, ent, i ? i-1 : 0);
	}
}

//used in edit/write, where sampleNumber == chunkNumber
M4Err stbl_AddChunkOffset(MediaAtom *mdia, u32 sampleNumber, u32 StreamDescIndex, u64 offset)
{
	SampleTableAtom *stbl;
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;
	stscEntry *ent, *tmp;
	u32 count, i, k, *newOff;
	u64 *newLarge;

	stbl = mdia->information->sampleTable;

	count = ChainGetCount(mdia->information->sampleTable->SampleToChunk->entryList);

	if (count + 1 < sampleNumber ) return M4BadParam;

	ent = (stscEntry*)malloc(sizeof(stscEntry));
	ent->isEdited = 0;
	if (Media_IsSelfContained(mdia, StreamDescIndex)) ent->isEdited = 1;
	ent->sampleDescriptionIndex = StreamDescIndex;
	ent->samplesPerChunk = 1;

	//we know 1 chunk == 1 sample, so easy...
	ent->firstChunk = sampleNumber;
	ent->nextChunk = sampleNumber + 1;

	//add the offset to the chunk...
	//and we change our offset
	if (stbl->ChunkOffset->type == ChunkOffsetAtomType) {
		stco = (ChunkOffsetAtom *)stbl->ChunkOffset;
		//if the new offset is a large one, we have to rewrite our table entry by entry (32->64 bit conv)...
		if (offset > 0xFFFFFFFF) {
			co64 = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
			co64->entryCount = stco->entryCount + 1;
			co64->offsets = (u64*)malloc(sizeof(u64) * co64->entryCount);
			k = 0;
			for (i=0;i<stco->entryCount; i++) {
				if (i + 1 == sampleNumber) {
					co64->offsets[i] = offset;
					k = 1;
				}
				co64->offsets[i+k] = (u64) stco->offsets[i];
			}
			if (!k) co64->offsets[co64->entryCount - 1] = offset;
			DelAtom(stbl->ChunkOffset);
			stbl->ChunkOffset = (Atom *) co64;
		} else {
			//no, we can use this one.
			if (sampleNumber > stco->entryCount) {
				if (!stco->alloc_size) stco->alloc_size = stco->entryCount;
				if (stco->entryCount == stco->alloc_size) {
					stco->alloc_size += 50;
					newOff = (u32*)malloc(sizeof(u32) * stco->alloc_size);
					memcpy(newOff, stco->offsets, sizeof(u32) * stco->entryCount);
					free(stco->offsets);
					stco->offsets = newOff;
				}
				stco->offsets[stco->entryCount] = (u32) offset;
				stco->entryCount += 1;
			} else {
				//nope. we're inserting
				newOff = (u32*)malloc(sizeof(u32) * (stco->entryCount + 1));
				k=0;
				for (i=0; i<stco->entryCount; i++) {
					if (i+1 == sampleNumber) {
						newOff[i] = (u32) offset;
						k=1;
					}
					newOff[i+k] = stco->offsets[i];
				}
				free(stco->offsets);
				stco->offsets = newOff;
				stco->entryCount ++;
				stco->alloc_size++;
			}
		}
	} else {
		//use large offset...
		co64 = (ChunkLargeOffsetAtom *)stbl->ChunkOffset;
		if (sampleNumber > co64->entryCount) {
			if (!co64->alloc_size) co64->alloc_size = co64->entryCount;
			if (co64->entryCount == co64->alloc_size) {
				co64->alloc_size += 50;
				newLarge = (u64*)malloc(sizeof(u64) * co64->alloc_size);
				memcpy(newLarge, co64->offsets, sizeof(u64) * co64->entryCount);
				free(co64->offsets);
				co64->offsets = newLarge;
			}
			co64->offsets[co64->entryCount] = (u32) offset;
			co64->entryCount += 1;
		} else {
			//nope. we're inserting
			newLarge = (u64*)malloc(sizeof(u64) * (co64->entryCount + 1));
			k=0;
			for (i=0; i<co64->entryCount; i++) {
				if (i+1 == sampleNumber) {
					newLarge[i] = offset;
					k=1;
				}
				newLarge[i+k] = co64->offsets[i];
			}
			free(co64->offsets);
			co64->offsets = newLarge;
			co64->entryCount++;
			co64->alloc_size++;
		}
	}

	//OK, now if we've inserted a chunk, update the sample to chunk info...
	if (sampleNumber == count + 1) {
		ent->nextChunk = count + 1;
		if (stbl->SampleToChunk->currentEntry)
			stbl->SampleToChunk->currentEntry->nextChunk = ent->firstChunk;
		stbl->SampleToChunk->currentEntry = ent;
		stbl->SampleToChunk->currentIndex = count;
		stbl->SampleToChunk->firstSampleInCurrentChunk = sampleNumber;
		//write - edit mode: sample number = chunk number
		stbl->SampleToChunk->currentChunk = sampleNumber;
		stbl->SampleToChunk->ghostNumber = 1;
		return ChainAddEntry(stbl->SampleToChunk->entryList, ent);
	}
	for (i = sampleNumber - 1; i<count; i++) {
		tmp = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, i);
		if (tmp) tmp->firstChunk +=1;
	}
	return ChainInsertEntry(stbl->SampleToChunk->entryList, ent, sampleNumber-1);
}




M4Err stbl_SetChunkOffset(MediaAtom *mdia, u32 sampleNumber, u64 offset)
{
	stscEntry *ent;
	u32 i;
	ChunkLargeOffsetAtom *co64;
	SampleTableAtom *stbl = mdia->information->sampleTable;

	if (!sampleNumber || !stbl) return M4BadParam;

	ent = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, sampleNumber - 1);

	//we edit our entry if self contained
	if (Media_IsSelfContained(mdia, ent->sampleDescriptionIndex))
		ent->isEdited = 1;

	//and we change our offset
	if (stbl->ChunkOffset->type == ChunkOffsetAtomType) {
		//if the new offset is a large one, we have to rewrite our table...
		if (offset > 0xFFFFFFFF) {
			co64 = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
			co64->entryCount = ((ChunkOffsetAtom *)stbl->ChunkOffset)->entryCount;
			co64->offsets = (u64*)malloc(sizeof(u64)*co64->entryCount);
			for (i=0;i<co64->entryCount; i++) {
				co64->offsets[i] = (u64) ((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets[i];
			}
			co64->offsets[ent->firstChunk - 1] = offset;
			DelAtom(stbl->ChunkOffset);
			stbl->ChunkOffset = (Atom *) co64;
			return M4OK;
		}
		((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets[ent->firstChunk - 1] = (u32) offset;
	} else {
		((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets[ent->firstChunk - 1] = offset;
	}
	return M4OK;
}


M4Err stbl_SetSampleCTS(SampleTableAtom *stbl, u32 sampleNumber, u32 offset)
{
	u32 i, j, sampNum, *CTSs;
	dttsEntry *ent;

	CompositionOffsetAtom *ctts = stbl->CompositionOffset;

	
	//if we're setting the CTS of a sample we've skipped...
	if (ctts->w_LastSampleNumber < sampleNumber) {
		//add some 0 till we get to the sample
		while (ctts->w_LastSampleNumber + 1 != sampleNumber) {
			AddCompositionOffset(ctts, 0);
		}
		return AddCompositionOffset(ctts, offset);
	}
	
	//NOPE we are inserting a sample...
	CTSs = (u32*)malloc(sizeof(u32) * ctts->w_LastSampleNumber);
	sampNum = 0;
	for (i=0; i<ChainGetCount(ctts->entryList); i++) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, i);
		for (j = 0; j<ent->sampleCount; j++) {
			if (sampNum + 1 == sampleNumber) {
				CTSs[sampNum] = offset;
			} else {
				CTSs[sampNum] = ent->decodingOffset;
			}
			sampNum ++;
		}
	}
	//delete the entries
	while (ChainGetCount(ctts->entryList)) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, 0);
		free(ent);
		ChainDeleteEntry(ctts->entryList, 0);
	}

	//rewrite the table
	ent = (dttsEntry*)malloc(sizeof(dttsEntry));
	ent->sampleCount = 1;
	ent->decodingOffset = CTSs[0];
	i = 1;
	//reset the read cache (entry insertion)
	ctts->r_currentEntryIndex = 1;
	ctts->r_FirstSampleInEntry = 1;
	while (1) {
		if (i == ctts->w_LastSampleNumber) {
			ChainAddEntry(ctts->entryList, ent);
			break;
		}
		if (CTSs[i] == ent->decodingOffset) {
			ent->sampleCount += 1;
		} else {
			ChainAddEntry(ctts->entryList, ent);
			ent = (dttsEntry*)malloc(sizeof(dttsEntry));
			ent->sampleCount = 1;
			ent->decodingOffset = CTSs[i];
			ctts->r_FirstSampleInEntry = i;
		}
		if (i==sampleNumber) ctts->r_currentEntryIndex = ChainGetCount(ctts->entryList) + 1;
		i++;
	}
	free(CTSs);
	return M4OK;
}

M4Err stbl_SetSampleSize(SampleSizeAtom *stsz, u32 SampleNumber, u32 size)
{
	u32 i;
	if (!SampleNumber || (stsz->sampleCount < SampleNumber)) return M4BadParam;

	if (stsz->sampleSize) {
		if (stsz->sampleSize == size) return M4OK;
		if (stsz->sampleCount == 1) {
			stsz->sampleSize = size;
			return M4OK;
		}
		//nope, we have to rewrite a table
		stsz->sizes = (u32*)malloc(sizeof(u32)*stsz->sampleCount);
		for (i=0; i<stsz->sampleCount; i++) stsz->sizes[i] = stsz->sampleSize;
		stsz->sampleSize = 0;
	}
	stsz->sizes[SampleNumber - 1] = size;
	return M4OK;
}


M4Err stbl_SetSampleRAP(SyncSampleAtom *stss, u32 SampleNumber, u8 isRAP)
{
	u32 i, j, k, *newNum, nextSamp;

	nextSamp = 0;
	//check if we have already a sync sample
	for (i = 0; i < stss->entryCount; i++) {
		if (stss->sampleNumbers[i] == SampleNumber) {
			if (isRAP) return M4OK;
			//remove it...
			newNum = (u32*)malloc(sizeof(u32) * (stss->entryCount-1));
			k = 0;
			for (j=0; j<stss->entryCount; j++) {
				if (stss->sampleNumbers[j] == SampleNumber) {
					k=1;
				} else {
					newNum[j-k] = stss->sampleNumbers[j];
				}
			}
			stss->entryCount -=1;
			free(stss->sampleNumbers);
			stss->sampleNumbers = newNum;
			return M4OK;
		}
		if (stss->sampleNumbers[i] > SampleNumber) break;
	}
	//we need to insert a RAP somewhere if RAP ...
	if (!isRAP) return M4OK;
	newNum = (u32*)malloc(sizeof(u32) * (stss->entryCount + 1));
	k = 0;
	for (j = 0 ; j<stss->entryCount; j++) {
		if (j == i) {
			newNum[j] = SampleNumber;
			k = 1;
		}
		newNum[j+k] = stss->sampleNumbers[j];
	}
	if (!k) {
		newNum[stss->entryCount] = SampleNumber;
	}
	free(stss->sampleNumbers);
	stss->sampleNumbers = newNum;
	stss->entryCount ++;
	return M4OK;
}

M4Err stbl_SetSyncShadow(ShadowSyncAtom *stsh, u32 sampleNumber, u32 syncSample)
{
	u32 i;
	stshEntry *ent;

	for (i=0; i<ChainGetCount(stsh->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(stsh->entries, i);
		if (ent->shadowedSampleNumber == sampleNumber) {
			ent->syncSampleNumber = syncSample;
			return M4OK;
		}
		if (ent->shadowedSampleNumber > sampleNumber) break;
	}
	//we need a new one...
	ent = (stshEntry*)malloc(sizeof(stshEntry));
	ent->shadowedSampleNumber = sampleNumber;
	ent->syncSampleNumber = syncSample;
	//insert or append ?
	if (i == ChainGetCount(stsh->entries)) {
		//don't update the cache ...
		return ChainAddEntry(stsh->entries, ent);
	} else {
		//update the cache
		stsh->r_LastEntryIndex = i;
		stsh->r_LastFoundSample = sampleNumber;
		return ChainInsertEntry(stsh->entries, ent, i);
	}
}


//always called before removing the sample from SampleSize
M4Err stbl_RemoveDTS(SampleTableAtom *stbl, u32 sampleNumber, u32 LastAUDefDuration)
{
	u32 *DTSs;
	u32 i, j, k, curDTS, sampNum;
	sttsEntry *ent;
	TimeToSampleAtom *stts;

	stts = stbl->TimeToSample;

	//gasp, we're removing the only sample: empty the sample table 
	if (stbl->SampleSize->sampleCount == 1) {
		if (ChainGetCount(stts->entryList)) {
			ChainDeleteEntry(stts->entryList, 0);
		}
		//update the reading cache
		stts->r_FirstSampleInEntry = stts->r_CurrentDTS = stts->r_currentEntryIndex = 0;
		return M4OK;
	}
	//unpack the DTSs...
	DTSs = (u32*)malloc(sizeof(u32) * (stbl->SampleSize->sampleCount - 1));
	curDTS = 0;
	sampNum = 0;
	ent = NULL;
	k=0;
	for (i=0; i < ChainGetCount(stts->entryList); i++) {
		ent = (sttsEntry*)ChainGetEntry(stts->entryList, i);
		for (j = 0; j<ent->sampleCount; j++) {
			if (sampNum == sampleNumber - 1) {
				k=1;
			} else {
				DTSs[sampNum-k] = curDTS;
			}
			curDTS += ent->sampleDelta;
			sampNum ++;
		}
	}
	//delete the table..
	while (ChainGetCount(stts->entryList)) {
		ent = (sttsEntry*)ChainGetEntry(stts->entryList, 0);
		free(ent);
		ChainDeleteEntry(stts->entryList, 0);
	}

	//rewrite the table
	ent = (sttsEntry*)malloc(sizeof(sttsEntry));
	ent->sampleCount = 0;
	if (stbl->SampleSize->sampleCount == 2) {
		ent->sampleDelta = LastAUDefDuration;
		ChainAddEntry(stts->entryList, ent);
	} else {
		ent->sampleDelta = DTSs[1];
		DTSs[0] = 0;
	}
	i = 0;
	while (1) {
		if (i+2 == stbl->SampleSize->sampleCount) {
			//and by default, our last sample has the same delta as the prev
			ent->sampleCount++;
			ChainAddEntry(stts->entryList, ent);
			break;
		}
		if (DTSs[i+1] - DTSs[i] == ent->sampleDelta) {
			ent->sampleCount += 1;
		} else {
			ChainAddEntry(stts->entryList, ent);
			ent = (sttsEntry*)malloc(sizeof(sttsEntry));
			ent->sampleCount = 1;
			ent->sampleDelta = DTSs[i+1] - DTSs[i];
		}
		i++;
	}
	stts->w_LastDTS = DTSs[stbl->SampleSize->sampleCount - 2];
	free(DTSs);
	//reset write the cache to the end
	stts->w_currentEntry = ent;
	stts->w_currentSampleNum = stbl->SampleSize->sampleCount - 1;
	//reset read the cache to the begining
	stts->r_FirstSampleInEntry = stts->r_CurrentDTS = stts->r_currentEntryIndex = 0;
	return M4OK;
}


//always called before removing the sample from SampleSize
M4Err stbl_RemoveCTS(SampleTableAtom *stbl, u32 sampleNumber)
{
	u32 *CTSs;
	u32 sampNum, i, j, k;
	dttsEntry *ent;
	CompositionOffsetAtom *ctts;

	ctts = stbl->CompositionOffset;

	//last one...
	if (stbl->SampleSize->sampleCount == 1) {
		DelAtom((Atom *) ctts);
		stbl->CompositionOffset = NULL;
		return M4OK;
	}

	//the number of entries is NOT ALWAYS the number of samples !
	//instead, use the cache
	//first case, we're removing a sample that was not added yet
	if (sampleNumber > ctts->w_LastSampleNumber) return M4OK;
	//No, the sample was here...
	//this is the only one we have.
	if (ctts->w_LastSampleNumber == 1) {
		DelAtom((Atom *) ctts);
		stbl->CompositionOffset = NULL;
		return M4OK;
	}
	CTSs = (u32*)malloc(sizeof(u32) * (ctts->w_LastSampleNumber - 1));
	sampNum = 0;
	k = 0;
	for (i=0; i<ChainGetCount(ctts->entryList); i++) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, i);
		for (j = 0; j<ent->sampleCount; j++) {
			if (sampNum + 1 == sampleNumber) {
				k = 1;
			} else {
				CTSs[sampNum-k] = ent->decodingOffset;
			}
			sampNum ++;
		}
	}
	
	//delete the entries
	while (ChainGetCount(ctts->entryList)) {
		ent = (dttsEntry*)ChainGetEntry(ctts->entryList, 0);
		free(ent);
		ChainDeleteEntry(ctts->entryList, 0);
	}
	

	//rewrite the table
	ent = (dttsEntry*)malloc(sizeof(dttsEntry));
	ent->sampleCount = 1;
	ent->decodingOffset = CTSs[0];
	i = 1;
	while (1) {
		if (i+1 == ctts->w_LastSampleNumber) {
			ChainAddEntry(ctts->entryList, ent);
			break;
		}
		if (CTSs[i] == ent->decodingOffset) {
			ent->sampleCount += 1;
		} else {
			ChainAddEntry(ctts->entryList, ent);
			ent = (dttsEntry*)malloc(sizeof(dttsEntry));
			ent->sampleCount = 1;
			ent->decodingOffset = CTSs[i];
		}
		i++;
	}
	free(CTSs);
	//reset the cache to the end
	ctts->w_currentEntry = ent;
	//we've removed a sample, therefore the last sample (n) has now number n-1
	//we cannot use SampleCount because we have probably skipped some samples
	//(we're calling AddCTS only if the sample gas a CTSOffset !!!)
	ctts->w_LastSampleNumber -= 1;
	return M4OK;
}

M4Err stbl_RemoveSize(SampleSizeAtom *stsz, u32 sampleNumber)
{
	u32 *newSizes;
	u32 i, k, oneSize, prevSize;

	//last sample
	if (stsz->sampleCount == 1) {
		if (stsz->sizes) free(stsz->sizes);
		stsz->sizes = NULL;
		stsz->sampleCount = 0;
		return M4OK;
	}
	//one single size
	if (stsz->sampleSize) {
		stsz->sampleCount -= 1;
		return M4OK;
	}

	//reallocate and check size by the way...
	newSizes = (u32*)malloc(sizeof(u32) * (stsz->sampleCount - 1));
	if (sampleNumber == 1) {
		prevSize = stsz->sizes[1];
	} else {
		prevSize = stsz->sizes[0];
	}
	k=0;
	oneSize = 0;
	for (i=0; i<stsz->sampleCount; i++) {
		if (i+1 == sampleNumber) {
			k=1;
		} else {
			newSizes[i-k] = stsz->sizes[i];
			oneSize = (prevSize == newSizes[i-k]) ? 1 : 0;
		}
	}
	//only in non-compact mode
	if (oneSize && (stsz->type != CompactSampleSizeAtomType) ) {
		free(newSizes);
		free(stsz->sizes);
		stsz->sampleSize = prevSize;
	} else {
		free(stsz->sizes);
		stsz->sizes = newSizes;
		stsz->sampleSize = 0;
	}
	stsz->sampleCount -= 1;
	return M4OK;
}

//always called after removing the sample from SampleSize
M4Err stbl_RemoveChunk(SampleTableAtom *stbl, u32 sampleNumber)
{
	u32 i, k;
	u32 *offsets;
	u64 *Loffsets;
	stscEntry *ent;

	//remove the entry in SampleToChunk (1 <-> 1 in edit mode)
	ChainDeleteEntry(stbl->SampleToChunk->entryList, sampleNumber - 1);
	//update the firstchunk info
	for (i = sampleNumber - 1; i < ChainGetCount(stbl->SampleToChunk->entryList); i++) {
		ent = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, i);
		ent->firstChunk -= 1;
		ent->nextChunk -= 1;
	}
	//update the cache
	stbl->SampleToChunk->firstSampleInCurrentChunk = 1;
	stbl->SampleToChunk->currentEntry = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, 0);
	stbl->SampleToChunk->currentIndex = 0;
	stbl->SampleToChunk->currentChunk = 1;
	stbl->SampleToChunk->ghostNumber = 1;

	//realloc the chunk offset
	if (stbl->ChunkOffset->type == ChunkOffsetAtomType) {
		if (!stbl->SampleSize->sampleCount) {
			free(((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets);
			((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets = NULL;
			((ChunkOffsetAtom *)stbl->ChunkOffset)->entryCount = 0;
			return M4OK;
		}
		offsets = (u32*)malloc(sizeof(u32) * (stbl->SampleSize->sampleCount));
		k=0;
		for (i=0; i<stbl->SampleSize->sampleCount+1; i++) {
			if (i+1 == sampleNumber) {
				k=1;
			} else {
				offsets[i-k] = ((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets[i];
			}
		}
		free(((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets);
		((ChunkOffsetAtom *)stbl->ChunkOffset)->offsets = offsets;
		((ChunkOffsetAtom *)stbl->ChunkOffset)->entryCount -= 1;
	} else {
		if (!stbl->SampleSize->sampleCount) {
			free(((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets);
			((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets = NULL;
			((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->entryCount = 0;
			return M4OK;
		}

		Loffsets = (u64*)malloc(sizeof(u64) * (stbl->SampleSize->sampleCount));
		k=0;
		for (i=0; i<stbl->SampleSize->sampleCount+1; i++) {
			if (i+1 == sampleNumber) {
				k=1;
			} else {
				Loffsets[i-k] = ((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets[i];
			}
		}
		free(((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets);
		((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->offsets = Loffsets;
		((ChunkLargeOffsetAtom *)stbl->ChunkOffset)->entryCount -= 1;
	}
	return M4OK;
}


M4Err stbl_RemoveRAP(SampleTableAtom *stbl, u32 sampleNumber)
{
	u32 i;

	SyncSampleAtom *stss = stbl->SyncSample;
	//we remove the only one around...
	if (stss->entryCount == 1) {
		if (stss->sampleNumbers[0] != sampleNumber) return M4OK;
		//free our numbers but don't delete (all samples are NON-sync
		free(stss->sampleNumbers);
		stss->sampleNumbers = NULL;
		stss->r_LastSampleIndex = stss->r_LastSyncSample = 0;
		stss->entryCount = 0;
		return M4OK;
	}
	//the real pain is that we may actually not have to change anything..
	for (i=0; i<stss->entryCount; i++) {
		if (sampleNumber == stss->sampleNumbers[i]) goto found;
	}
	//nothing to do
	return M4OK;

found:
	//a small opt: the sample numbers are in order...
	i++;
	for (;i<stss->entryCount; i++) {
		stss->sampleNumbers[i-1] = stss->sampleNumbers[i];
	}
	//and just realloc
	stss->sampleNumbers = (u32*)realloc(stss->sampleNumbers, sizeof(u32) * (stss->entryCount-1));
	stss->entryCount -= 1;
	return M4OK;
}

M4Err stbl_RemoveShadow(ShadowSyncAtom *stsh, u32 sampleNumber)
{
	u32 i;
	stshEntry *ent;

	//we loop for the whole chain cause the spec doesn't say if we can have several
	//shadows for 1 sample...
	for (i=0; i<ChainGetCount(stsh->entries); i++) {
		ent = (stshEntry*)ChainGetEntry(stsh->entries, i);
		if (ent->shadowedSampleNumber == sampleNumber) {
			ChainDeleteEntry(stsh->entries, i);
			i--;
		}
	}
	//reset the cache
	stsh->r_LastEntryIndex = 0;
	stsh->r_LastFoundSample = 0;
	return M4OK;
}


M4Err stbl_SetPaddingBits(SampleTableAtom *stbl, u32 SampleNumber, u8 bits)
{
	u8 *p;
	//make sure the sample is a good one
	if (SampleNumber > stbl->SampleSize->sampleCount) return M4BadParam;

	//create the table
	if (!stbl->PaddingBits) stbl->PaddingBits = (PaddingBitsAtom *) CreateAtom(PaddingBitsAtomType);

	//alloc
	if (!stbl->PaddingBits->padbits || !stbl->PaddingBits->SampleCount) {
		stbl->PaddingBits->SampleCount = stbl->SampleSize->sampleCount;
		stbl->PaddingBits->padbits = malloc(sizeof(u8)*stbl->PaddingBits->SampleCount);
		if (!stbl->PaddingBits->padbits) return M4OutOfMem;
		memset(stbl->PaddingBits->padbits, 0, sizeof(u8)*stbl->PaddingBits->SampleCount);
	}
	//realloc (this is needed in case n out of k samples get padding added)
	if (stbl->PaddingBits->SampleCount < stbl->SampleSize->sampleCount) {
		p = malloc(sizeof(u8) * stbl->SampleSize->sampleCount);
		if (!p) return M4OutOfMem;
		//set everything to 0
		memset(p, 0, stbl->SampleSize->sampleCount);
		//copy our previous table
		memcpy(p, stbl->PaddingBits->padbits, stbl->PaddingBits->SampleCount);
		free(stbl->PaddingBits->padbits);
		stbl->PaddingBits->padbits = p;
		stbl->PaddingBits->SampleCount = stbl->SampleSize->sampleCount;
	}
	stbl->PaddingBits->padbits[SampleNumber-1] = bits;
	return M4OK;
}

M4Err stbl_RemovePaddingBits(SampleTableAtom *stbl, u32 SampleNumber)
{
	u8 *p;
	u32 i, k;

	if (!stbl->PaddingBits) return M4OK;
	if (stbl->PaddingBits->SampleCount < SampleNumber) return M4BadParam;

	//last sample - remove the table
	if (stbl->PaddingBits->SampleCount == 1) {
		DelAtom((Atom *) stbl->PaddingBits);
		stbl->PaddingBits = NULL;
		return M4OK;
	}

	//reallocate and check size by the way...
	p = (u8 *)malloc(sizeof(u8) * (stbl->PaddingBits->SampleCount - 1));
	if (!p) return M4OutOfMem;

	k=0;
	for (i=0; i<stbl->PaddingBits->SampleCount; i++) {
		if (i+1 != SampleNumber) {
			p[k] = stbl->PaddingBits->padbits[i];
			k++;
		}
	}
	
	stbl->PaddingBits->SampleCount -= 1;
	free(stbl->PaddingBits->padbits);
	stbl->PaddingBits->padbits = p;
	return M4OK;
}

M4Err stbl_AddSampleFragment(SampleTableAtom *stbl, u32 sampleNumber, u16 size)
{
	M4Err e;
	u32 i;
	stsfEntry *ent;
	u16 *newSizes;
	SampleFragmentAtom *stsf;	
	M4Err stbl_AddAtom(SampleTableAtom *ptr, Atom *a);
	
	stsf = stbl->Fragments;

	if (!stsf) {
		//create table if any
		stsf = (SampleFragmentAtom *) CreateAtom(SampleFragmentAtomType);
		if (!stsf) return M4OutOfMem;
		e = stbl_AddAtom(stbl, (Atom *) stsf);
		if (e) return e;
	}

	//check cache
	if (!stsf->w_currentEntry || (stsf->w_currentEntry->SampleNumber < sampleNumber)) {
		stsf->w_currentEntry = NULL;
		stsf->w_currentEntryIndex = 0;
	}
	i = stsf->w_currentEntryIndex;

	for (; i<ChainGetCount(stsf->entryList); i++) {
		ent = ChainGetEntry(stsf->entryList, i);
		if (ent->SampleNumber > sampleNumber) {
			ent = malloc(sizeof(stsfEntry));
			memset(ent, 0, sizeof(stsfEntry));
			ent->SampleNumber = sampleNumber;
			ChainInsertEntry(stsf->entryList, ent, i);
			stsf->w_currentEntry = ent;
			stsf->w_currentEntryIndex = i;
			goto ent_found;
		}
		else if (ent->SampleNumber == sampleNumber) {
			stsf->w_currentEntry = ent;
			stsf->w_currentEntryIndex = i;
			goto ent_found;
		}
	}
	//if we get here add a new entry
	ent = malloc(sizeof(stsfEntry));
	memset(ent, 0, sizeof(stsfEntry));
	ent->SampleNumber = sampleNumber;
	ChainAddEntry(stsf->entryList, ent);
	stsf->w_currentEntry = ent;
	stsf->w_currentEntryIndex = ChainGetCount(stsf->entryList)-1;

ent_found:
	if (!ent->fragmentCount) {
		ent->fragmentCount = 1;
		ent->fragmentSizes = malloc(sizeof(u16));
		ent->fragmentSizes[0] = size;
		return M4OK;
	}
	newSizes = malloc(sizeof(u16) * (ent->fragmentCount+1));
	memcpy(newSizes, ent->fragmentSizes, sizeof(u16) * ent->fragmentCount);
	newSizes[ent->fragmentCount] = size;
	free(ent->fragmentSizes);
	ent->fragmentSizes = newSizes;
	ent->fragmentCount += 1;

	return M4OK;
}

M4Err stbl_RemoveSampleFragments(SampleTableAtom *stbl, u32 sampleNumber)
{
	u32 i;
	stsfEntry *ent;
	SampleFragmentAtom *stsf = stbl->Fragments;

	for (i=0; i<ChainGetCount(stsf->entryList); i++) {
		ent = ChainGetEntry(stsf->entryList, i);
		if (ent->SampleNumber == sampleNumber) {
			ChainDeleteEntry(stsf->entryList, i);
			if (ent->fragmentSizes) free(ent->fragmentSizes);
			free(ent);
			goto exit;
		}
	}
exit:
	//empty table, remove it
	if (!ChainGetCount(stsf->entryList)) {
		stbl->Fragments = NULL;
		DelAtom((Atom *)stsf);
	}
	return M4OK;
}

M4Err stbl_SampleSizeAppend(SampleSizeAtom *stsz, u32 data_size)
{
	u32 i;
	if (!stsz || !stsz->sampleCount) return M4BadParam;
	
	//we must realloc our table
	if (stsz->sampleSize) {
		stsz->sizes = malloc(sizeof(u32)*stsz->sampleCount);
		for (i=0; i<stsz->sampleCount; i++) stsz->sizes[i] = stsz->sampleSize;
		stsz->sampleSize = 0;
	}
	stsz->sizes[stsz->sampleCount-1] += data_size;
	return M4OK;
}

#endif	//M4_READ_ONLY



void stbl_AppendTime(SampleTableAtom *stbl, u32 duration)
{
	sttsEntry *ent;
	u32 count;
	count = ChainGetCount(stbl->TimeToSample->entryList);
	if (count) {
		ent = ChainGetEntry(stbl->TimeToSample->entryList, count-1);
		if (ent->sampleDelta == duration) {
			ent->sampleCount += 1;
			return;
		}
	}
	//nope need a new entry
	ent = malloc(sizeof(sttsEntry));
	ent->sampleCount = 1;
	ent->sampleDelta = duration;
	ChainAddEntry(stbl->TimeToSample->entryList, ent);
}

void stbl_AppendSize(SampleTableAtom *stbl, u32 size)
{
	u32 *new_sizes, i;

	if (!stbl->SampleSize->sampleCount) {
		stbl->SampleSize->sampleSize = size;
		stbl->SampleSize->sampleCount = 1;
		return;
	}
	if (stbl->SampleSize->sampleSize && (stbl->SampleSize->sampleSize==size)) {
		stbl->SampleSize->sampleCount += 1;
		return;
	}
	//realloc
	new_sizes = malloc(sizeof(u32)*(stbl->SampleSize->sampleCount+1));
	if (stbl->SampleSize->sizes) {
		memcpy(new_sizes, stbl->SampleSize->sizes, sizeof(u32)*stbl->SampleSize->sampleCount);
		free(stbl->SampleSize->sizes);
	} else {
		for (i=0; i<stbl->SampleSize->sampleCount;i++) new_sizes[i] = stbl->SampleSize->sampleSize;
	}
	stbl->SampleSize->sampleSize = 0;
	new_sizes[stbl->SampleSize->sampleCount] = size;
	stbl->SampleSize->sampleCount += 1;
	stbl->SampleSize->sizes = new_sizes;
}

void stbl_AppendChunk(SampleTableAtom *stbl, u64 offset)
{
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;
	u32 *new_offsets, i;
	u64 *off_64;

	//we may have to convert the table...
	if (stbl->ChunkOffset->type==ChunkOffsetAtomType) {
		stco = (ChunkOffsetAtom *)stbl->ChunkOffset;

		if (offset>0xFFFFFFFF) {
			co64 = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
			co64->entryCount = stco->entryCount + 1;
			co64->offsets = (u64*)malloc(sizeof(u64) * co64->entryCount);
			for (i=0; i<stco->entryCount; i++) co64->offsets[i] = stco->offsets[i];
			co64->offsets[i] = offset;
			DelAtom(stbl->ChunkOffset);
			stbl->ChunkOffset = (Atom *) co64;
			return;
		}
		//we're fine
		new_offsets = malloc(sizeof(u32)*(stco->entryCount+1));
		for (i=0; i<stco->entryCount; i++) new_offsets[i] = stco->offsets[i];
		new_offsets[i] = (u32) offset;
		if (stco->offsets) free(stco->offsets);
		stco->offsets = new_offsets;
		stco->entryCount += 1;
	}
	//large offsets
	else {
		co64 = (ChunkLargeOffsetAtom *)stbl->ChunkOffset;
		off_64 = malloc(sizeof(u32)*(co64->entryCount+1));
		for (i=0; i<co64->entryCount; i++) off_64[i] = co64->offsets[i];
		off_64[i] = offset;
		if (co64->offsets) free(co64->offsets);
		co64->offsets = off_64;
		co64->entryCount += 1;
	}
}

void stbl_AppendSampleToChunk(SampleTableAtom *stbl, u32 DescIndex, u32 samplesInChunk)
{
	u32 count, nextChunk;
	stscEntry *ent;

	count = ChainGetCount(stbl->SampleToChunk->entryList);
	nextChunk = ((ChunkOffsetAtom *) stbl->ChunkOffset)->entryCount;

	if (count) {
		ent = ChainGetEntry(stbl->SampleToChunk->entryList, count-1);
		//good we can use this one
		if ( (ent->sampleDescriptionIndex == DescIndex) && (ent->samplesPerChunk==samplesInChunk)) 
			return;

		//set the next chunk btw ...
		ent->nextChunk = nextChunk;
	}
	//ok we need a new entry - this assumes this function is called AFTER AppendChunk
	ent = malloc(sizeof(stscEntry));
	ent->firstChunk = nextChunk;
	ent->nextChunk = 0;
	ent->isEdited = 0;
	ent->sampleDescriptionIndex = DescIndex;
	ent->samplesPerChunk = samplesInChunk;
	ChainAddEntry(stbl->SampleToChunk->entryList, ent);
}

//called AFTER AddSize
void stbl_AppendRAP(SampleTableAtom *stbl, u8 isRap)
{
	u32 *new_raps, i;

	//no sync table
	if (!stbl->SyncSample) {
		//all samples RAP - no table
		if (isRap) return;

		//nope, create one
		stbl->SyncSample = (SyncSampleAtom *) CreateAtom(SyncSampleAtomType);
		if (stbl->SampleSize->sampleCount > 1) {
			stbl->SyncSample->sampleNumbers = malloc(sizeof(u32) * (stbl->SampleSize->sampleCount-1));
			for (i=0; i<stbl->SampleSize->sampleCount-1; i++) 
				stbl->SyncSample->sampleNumbers[i] = i+1;

		}
		stbl->SyncSample->entryCount = stbl->SampleSize->sampleCount-1;
		return;
	}
	if (!isRap) return;

	new_raps = malloc(sizeof(u32) * (stbl->SyncSample->entryCount + 1));
	for (i=0; i<stbl->SyncSample->entryCount; i++) new_raps[i] = stbl->SyncSample->sampleNumbers[i];
	new_raps[i] = stbl->SampleSize->sampleCount;
	if (stbl->SyncSample->sampleNumbers) free(stbl->SyncSample->sampleNumbers);
	stbl->SyncSample->sampleNumbers = new_raps;
	stbl->SyncSample->entryCount += 1;
}

void stbl_AppendPadding(SampleTableAtom *stbl, u8 padding)
{
	u32 i;
	u8 *pad_bits;
	if (!stbl->PaddingBits) stbl->PaddingBits = (PaddingBitsAtom *) CreateAtom(PaddingBitsAtomType);

	pad_bits = malloc(sizeof(u8) * stbl->SampleSize->sampleCount);
	memset(pad_bits, 0, sizeof(pad_bits));
//	for (i=0; i<stbl->SampleSize->sampleCount; i++) pad_bits[i] = 0;
	for (i=0; i<stbl->PaddingBits->SampleCount; i++) pad_bits[i] = stbl->PaddingBits->padbits[i];
	pad_bits[stbl->SampleSize->sampleCount-1] = padding;
	if (stbl->PaddingBits->padbits) free(stbl->PaddingBits->padbits);
	stbl->PaddingBits->padbits = pad_bits;
	stbl->PaddingBits->SampleCount = stbl->SampleSize->sampleCount;
}

void stbl_AppendCTSOffset(SampleTableAtom *stbl, u32 CTSOffset)
{
	u32 count;
	dttsEntry *ent;

	if (!stbl->CompositionOffset) stbl->CompositionOffset = (CompositionOffsetAtom *) CreateAtom(CompositionOffsetAtomType);

	count = ChainGetCount(stbl->CompositionOffset->entryList);
	if (count) {
		ent = ChainGetEntry(stbl->CompositionOffset->entryList, count-1);
		if (ent->decodingOffset == CTSOffset) {
			ent->sampleCount ++;
			return;
		}
	}
	ent = malloc(sizeof(dttsEntry));
	ent->sampleCount = 1;
	ent->decodingOffset = CTSOffset;
	ChainAddEntry(stbl->CompositionOffset->entryList, ent);
}

void stbl_AppendDegradation(SampleTableAtom *stbl, u16 DegradationPriority)
{
	u32 i;
	u16 *prio;
	if (!stbl->DegradationPriority) stbl->DegradationPriority = (DegradationPriorityAtom *) CreateAtom(DegradationPriorityAtomType);

	prio = malloc(sizeof(u16) * stbl->SampleSize->sampleCount);
	memset(prio, 0, sizeof(prio));
	for (i=0; i<stbl->DegradationPriority->entryCount; i++) prio[i] = stbl->DegradationPriority->priorities[i];
	prio[stbl->SampleSize->sampleCount-1] = DegradationPriority;
	if (stbl->DegradationPriority->priorities) free(stbl->DegradationPriority->priorities);
	stbl->DegradationPriority->priorities = prio;
	stbl->DegradationPriority->entryCount = stbl->SampleSize->sampleCount;
}
