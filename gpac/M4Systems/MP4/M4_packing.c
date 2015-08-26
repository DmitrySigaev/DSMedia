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

//This functions unpack the offset for easy editing, eg each sample
//is contained in one chunk...
M4Err stbl_UnpackOffsets(SampleTableAtom *stbl)
{
	M4Err e;
	u8 isEdited;
	u32 i, chunkNumber, sampleDescIndex;
	u64 dataOffset;
	stscEntry *ent;
	ChunkOffsetAtom *stco_tmp;
	ChunkLargeOffsetAtom *co64_tmp;
	SampleToChunkAtom *stsc_tmp;

	if (!stbl) return M4InvalidMP4File;

	//we should have none of the mandatory atoms (allowed in the spec)
	if (!stbl->ChunkOffset && !stbl->SampleDescription && !stbl->SampleSize && !stbl->SampleToChunk && !stbl->TimeToSample)
		return M4OK;
	//or all the mandatory ones ...
	if (!stbl->ChunkOffset || !stbl->SampleDescription || !stbl->SampleSize || !stbl->SampleToChunk || !stbl->TimeToSample)
		return M4InvalidMP4File;

	//do we need to unpack? Not if we have only one sample per chunk.
	if (stbl->SampleSize->sampleCount == ChainGetCount(stbl->SampleToChunk->entryList)) return M4OK;

	//create a new SampleToChunk table
	stsc_tmp = (SampleToChunkAtom *) CreateAtom(SampleToChunkAtomType);

	//check the offset type and create a new table...
	if (stbl->ChunkOffset->type == ChunkOffsetAtomType) {
		co64_tmp = NULL;
		stco_tmp = (ChunkOffsetAtom *) CreateAtom(ChunkOffsetAtomType);
		stco_tmp->entryCount = stbl->SampleSize->sampleCount;
		stco_tmp->offsets = (u32*)malloc(stco_tmp->entryCount * sizeof(u32));
	} else if (stbl->ChunkOffset->type == ChunkLargeOffsetAtomType) {
		stco_tmp = NULL;
		co64_tmp = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
		co64_tmp->entryCount = stbl->SampleSize->sampleCount;
		co64_tmp->offsets = (u64*)malloc(co64_tmp->entryCount * sizeof(u64));
	} else {
		return M4InvalidMP4File;
	}

	ent = NULL;
	//OK write our two tables...
	for (i = 0; i < stbl->SampleSize->sampleCount; i++) {
		//get the data info for the sample
		e = stbl_GetSampleInfos(stbl, i+1, &dataOffset, &chunkNumber, &sampleDescIndex, &isEdited);
		if (e) goto err_exit;
		ent = (stscEntry*)malloc(sizeof(stscEntry));
		ent->isEdited = 0;
		ent->sampleDescriptionIndex = sampleDescIndex;
		//here's the trick: each sample is in ONE chunk
		ent->firstChunk = i+1;
		ent->nextChunk = i+2;
		ent->samplesPerChunk = 1;
		e = ChainAddEntry(stsc_tmp->entryList, ent);
		if (e) goto err_exit;
		if (stco_tmp) {
			stco_tmp->offsets[i] = (u32) dataOffset;
		} else {
			co64_tmp->offsets[i] = dataOffset;
		}
	}
	//close the list
	if (ent) ent->nextChunk = 0;
	

	//done, remove our previous tables
	DelAtom(stbl->ChunkOffset);
	DelAtom((Atom *)stbl->SampleToChunk);
	//and set these ones...
	if (stco_tmp) {
		stbl->ChunkOffset = (Atom *)stco_tmp;
	} else {
		stbl->ChunkOffset = (Atom *)co64_tmp;
	}
	stbl->SampleToChunk = stsc_tmp;
	stbl->SampleToChunk->currentEntry = (stscEntry*)ChainGetEntry(stbl->SampleToChunk->entryList, 0);
	stbl->SampleToChunk->currentIndex = 0;
	stbl->SampleToChunk->currentChunk = 0;
	stbl->SampleToChunk->firstSampleInCurrentChunk = 0;
	return M4OK;

err_exit:
	if (stco_tmp) DelAtom((Atom *) stco_tmp);
	if (co64_tmp) DelAtom((Atom *) co64_tmp);
	if (stsc_tmp) DelAtom((Atom *) stsc_tmp);
	return e;
}

#ifndef M4_READ_ONLY


//This function packs the offset after easy editing, eg samples
//are re-arranged in chunks according to the chunkOffsets
//NOTE: this has to be called once interleaving or whatever is done and 
//the final MDAT is written!!!
M4Err stbl_SetChunkAndOffset(SampleTableAtom *stbl, u32 sampleNumber, u32 StreamDescIndex, SampleToChunkAtom *the_stsc, Atom **the_stco, u64 data_offset, u8 forceNewChunk)
{
	M4Err e;
	u32 count;
	u8 newChunk;
	stscEntry *ent, *newEnt;

	if (!stbl) return M4InvalidMP4File;

	newChunk = 0;
	//do we need a new chunk ??? For that, we need
	//1 - make sure this sample data is contiguous to the prev one

	//force new chunk is set during writing (flat / interleaved)
	//it is set to 1 when data is not contiguous in the media (eg, interleaving)
	//when writing flat files, it is never used
	if (forceNewChunk) newChunk = 1;

	//2 - make sure we have the table inited (i=0)
	if (! the_stsc->currentEntry) {
		newChunk = 1;
	} else {
	//3 - make sure we do not exceed the MaxSamplesPerChunk and we have the same descIndex
		if (StreamDescIndex != the_stsc->currentEntry->sampleDescriptionIndex) 
			newChunk = 1;
		if (stbl->MaxSamplePerChunk && the_stsc->currentEntry->samplesPerChunk == stbl->MaxSamplePerChunk) 
			newChunk = 1;
	}

	//no need for a new chunk
	if (!newChunk) {
		the_stsc->currentEntry->samplesPerChunk += 1;
		return M4OK;
	}

	//OK, we have to create a new chunk...
	count = ChainGetCount(the_stsc->entryList);
	//check if we can remove the current sampleToChunk entry (same properties)
	if (count > 1) {
		ent = (stscEntry*)ChainGetEntry(the_stsc->entryList, count - 2);
		if ( (ent->sampleDescriptionIndex == the_stsc->currentEntry->sampleDescriptionIndex)
			&& (ent->samplesPerChunk == the_stsc->currentEntry->samplesPerChunk)
			) {
			//OK, it's the same SampleToChunk, so delete it
			ent->nextChunk = the_stsc->currentEntry->firstChunk;
			free(the_stsc->currentEntry);
			ChainDeleteEntry(the_stsc->entryList, count - 1);
			the_stsc->currentEntry = ent;
		}
	}

	//add our offset
	e = stbl_AddOffset(the_stco, data_offset);
	if (e) return e;

	//create a new entry (could be the first one, BTW)
	newEnt = (stscEntry*)malloc(sizeof(stscEntry));
	//get the first chunk value
	if ((*the_stco)->type == ChunkOffsetAtomType) {
		newEnt->firstChunk = ((ChunkOffsetAtom *) (*the_stco) )->entryCount;
	} else {
		newEnt->firstChunk = ((ChunkLargeOffsetAtom *) (*the_stco) )->entryCount;
	}
	newEnt->sampleDescriptionIndex = StreamDescIndex;
	newEnt->samplesPerChunk = 1;
	newEnt->nextChunk = 0;
	ChainAddEntry(the_stsc->entryList, newEnt);
	//if we already have an entry, adjust its next chunk to point to our new chunk
	if (the_stsc->currentEntry)
		the_stsc->currentEntry->nextChunk = newEnt->firstChunk;
	the_stsc->currentEntry = newEnt;
	return M4OK;
}



M4Err stbl_AddOffset(Atom **a, u64 offset)
{
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;
	u32 i;

	if ((*a)->type == ChunkOffsetAtomType) {
		stco = (ChunkOffsetAtom *) *a;
		//if dataOffset is bigger than 0xFFFFFFFF, move to LARGE offset
		if (offset > 0xFFFFFFFF) {
			co64 = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
			if (!co64) return M4OutOfMem;
			co64->entryCount = stco->entryCount + 1;
			co64->offsets = (u64*)malloc(co64->entryCount * sizeof(u64));
			if (!co64->offsets) {
				DelAtom((Atom *)co64);
				return M4OutOfMem;
			}
			for (i = 0; i< co64->entryCount - 1; i++) {
				co64->offsets[i] = (u64) stco->offsets[i];
			}
			co64->offsets[i] = offset;
			//delete the atom...
			DelAtom(*a);
			*a = (Atom *)co64;
			return M4OK;
		}
		//OK, stick with regular...
		stco->offsets = (u32*)realloc(stco->offsets, (stco->entryCount + 1) * sizeof(u32));
		if (!stco->offsets) return M4OutOfMem;
		stco->offsets[stco->entryCount] = (u32) offset;
		stco->entryCount += 1;
	} else {
		//this is a large offset
		co64 = (ChunkLargeOffsetAtom *) *a;
		co64->offsets = (u64*)realloc(co64->offsets, (co64->entryCount + 1) * sizeof(u64));
		if (!co64->offsets) return M4OutOfMem;
		co64->offsets[co64->entryCount] = offset;
		co64->entryCount += 1;
	}
	return M4OK;
}

#endif //M4_READ_ONLY
