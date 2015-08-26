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

#define MP4_CPRT_NOTICE "MP4 File Produced with GPAC"

M4Err M4_InsertCopyright(M4Movie *movie)
{
	u32 i;
	Atom *a;
	FreeSpaceAtom *free;

	if (!movie) return M4BadParam;
	//check if it's one of our demo files...
	for (i = 0; i < ChainGetCount(movie->TopAtoms); i++) {
		a = ChainGetEntry(movie->TopAtoms, i);
		if (a->type == FreeSpaceAtomType) {
			free = (FreeSpaceAtom *)a;
			if (free->dataSize)
				if (!strcmp(free->data, MP4_CPRT_NOTICE))	return M4OK;
		}
	}
	a = CreateAtom(FreeSpaceAtomType);
	if (!a) return M4OutOfMem;
	free = (FreeSpaceAtom *)a;
	free->dataSize = strlen(MP4_CPRT_NOTICE) + 1;
	free->data = malloc(free->dataSize);
	if (!free->data) return M4OutOfMem;
	strcpy(free->data, MP4_CPRT_NOTICE);
	return ChainAddEntry(movie->TopAtoms, free);
}

typedef struct {
	//the curent sample of this track
	u32 sampleNumber;
	//timeScale of the media (for interleaving)
	u32 timeScale;
	//this is for generic, time-based interleaving. Expressed in Media TimeScale
	u32 chunkDur;
	u32 DTSprev;
	u8 isDone;
	MediaAtom *mdia;
	//each writer has a sampleToChunck and ChunkOffset tables
	//these tables are filled during emulation mode and then will
	//replace the table in the SampleTableAtom
	SampleToChunkAtom *stsc;
	//we don't know if it's a large offset or not
	Atom *stco;
} TrackWriter;


//clean up our writers
void CleanWriters(Chain *writers)
{
	TrackWriter *writer;
	while (ChainGetCount(writers)) {
		writer = (TrackWriter*)ChainGetEntry(writers, 0);
		DelAtom(writer->stco);
		DelAtom((Atom *)writer->stsc);
		free(writer);
		ChainDeleteEntry(writers, 0);
	}
}

void ResetWriters(Chain *writers)
{
	u32 i;
	TrackWriter *writer;
	for (i=0; i<ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);
		writer->isDone = 0;
		writer->chunkDur = 0;
		writer->DTSprev = 0;
		writer->sampleNumber = 1;
		DelAtom((Atom *)writer->stsc);
		writer->stsc = (SampleToChunkAtom *) CreateAtom(SampleToChunkAtomType);
		if (writer->stco->type == ChunkOffsetAtomType) {
			free(((ChunkOffsetAtom *)writer->stco)->offsets);
			((ChunkOffsetAtom *)writer->stco)->offsets = NULL;
			((ChunkOffsetAtom *)writer->stco)->entryCount = 0;
		} else {
			free(((ChunkLargeOffsetAtom *)writer->stco)->offsets);
			((ChunkLargeOffsetAtom *)writer->stco)->offsets = NULL;
			((ChunkLargeOffsetAtom *)writer->stco)->entryCount = 0;
		}
	}
}

M4Err SetupWriters(M4Movie *movie, Chain *writers, u8 interleaving)
{
	u32 i, trackCount;
	TrackWriter *writer;
	TrackAtom *trak;

	
	trackCount = ChainGetCount(movie->moov->trackList);
	for (i = 0; i < trackCount; i++) {
		trak = GetTrack(movie->moov, i+1);
		
		writer = (TrackWriter*)malloc(sizeof(TrackWriter));
		if (!writer) goto exit;
		writer->sampleNumber = 1;
		writer->mdia = trak->Media;
		writer->timeScale = trak->Media->mediaHeader->timeScale;
		writer->isDone = 0;
		writer->DTSprev = 0;
		writer->chunkDur = 0;
		writer->stsc = (SampleToChunkAtom *) CreateAtom(SampleToChunkAtomType);
		if (trak->Media->information->sampleTable->ChunkOffset->type == ChunkOffsetAtomType) {
			writer->stco = CreateAtom(ChunkOffsetAtomType);
		} else {
			writer->stco = CreateAtom(ChunkLargeOffsetAtomType);
		}
		//stops from chunk escape
		if (interleaving) writer->mdia->information->sampleTable->MaxSamplePerChunk = 0;

		
		//optimization for interleaving: put audio last
		if (movie->storageMode != M4_INTERLEAVED) {
			ChainAddEntry(writers, writer);	
		} else {
			if (writer->mdia->information->InfoHeader->type == SoundMediaHeaderAtomType) {
				ChainAddEntry(writers, writer);	
			} else {
				ChainInsertEntry(writers, writer, 0);	
			}
		}
	}
	return M4OK;

exit:
	CleanWriters(writers);
	return M4OutOfMem;
}

M4Err ShiftOffset(Chain *writers, u64 offset)
{
	u32 i, j, k, l, last;
	TrackWriter *writer;
	stscEntry *ent;
	ChunkOffsetAtom *stco;
	ChunkLargeOffsetAtom *co64;

	for (i = 0; i < ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);

		//we have to proceed entry by entry in case a part of the media is not self-contained...
		for (j = 0; j<ChainGetCount(writer->stsc->entryList); j++) {
			ent = (stscEntry*)ChainGetEntry(writer->stsc->entryList, j);
			if (!Media_IsSelfContained(writer->mdia, ent->sampleDescriptionIndex)) continue;

			//OK, get the chunk(s) number(s) and "shift" its (their) offset(s).
			if (writer->stco->type == ChunkOffsetAtomType) {
				stco = (ChunkOffsetAtom *) writer->stco;
				//be carefull for the last entry, nextChunk is set to 0 in edit mode...
				last = ent->nextChunk ? ent->nextChunk : stco->entryCount + 1;
				for (k = ent->firstChunk; k < last; k++) {

					if (stco->offsets[k-1] + offset > 0xFFFFFFFF) {
						//too bad, rewrite the table....
						co64 = (ChunkLargeOffsetAtom *) CreateAtom(ChunkLargeOffsetAtomType);
						if (!co64) return M4OutOfMem;
						co64->entryCount = stco->entryCount;
						co64->offsets = (u64*)malloc(co64->entryCount * sizeof(u64));
						if (!co64) {
							DelAtom((Atom *)co64);
							return M4OutOfMem;
						}
						//duplicate the table 
						for (l = 0; l < co64->entryCount; l++) {
							co64->offsets[l] = (u64) stco->offsets[l];
							if (l + 1 == k) co64->offsets[l] += offset;
						}
						//and replace our atom
						DelAtom(writer->stco);
						writer->stco = (Atom *)co64;
					} else {
						stco->offsets[k-1] += (u32) offset;
					}
				}
			} else {
				co64 = (ChunkLargeOffsetAtom *) writer->stco;
				//be carefull for the last entry ...
				last = ent->nextChunk ? ent->nextChunk : co64->entryCount + 1;
				for (k = ent->firstChunk; k < last; k++) {
					co64->offsets[k-1] += offset;
				}
			}
		}
	}
	return M4OK;

}


//replace the chunk and offset tables...
M4Err WriteMoov(M4Movie *movie, Chain *writers, BitStream *bs)
{
	u32 i;
	TrackWriter *writer;
	M4Err e;
	Atom *stco;
	SampleToChunkAtom *stsc;

	//switch all our tables
	for (i = 0; i < ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);
		//don't delete them !!!
		stsc = writer->mdia->information->sampleTable->SampleToChunk;
		stco = writer->mdia->information->sampleTable->ChunkOffset;
		writer->mdia->information->sampleTable->SampleToChunk = writer->stsc;
		writer->mdia->information->sampleTable->ChunkOffset = writer->stco;
		writer->stco = stco;
		writer->stsc = stsc;
	}
	//write the moov atom...
	e = SizeAtom((Atom *)movie->moov);
	if (e) return e;
	e = WriteAtom((Atom *)movie->moov, bs);
	//and re-switch our table. We have to do it that way because it is 
	//needed when the moov is written first
	for (i = 0; i < ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);
		//don't delete them !!!
		stsc = writer->stsc;
		stco = writer->stco;
		writer->stsc = writer->mdia->information->sampleTable->SampleToChunk;
		writer->stco = writer->mdia->information->sampleTable->ChunkOffset;
		writer->mdia->information->sampleTable->SampleToChunk = stsc;
		writer->mdia->information->sampleTable->ChunkOffset = stco;
	}
	return e;
}

//compute the size of the moov as it will be written.
u64 GetMoovSize(M4Movie *movie, Chain *writers)
{
	u32 i;
	u64 size;
	TrackWriter *writer;

	SizeAtom((Atom *)movie->moov);
	size = movie->moov->size;
	if (size > 0xFFFFFFFF) size += 8;

	for (i=0; i < ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);
		size -= writer->mdia->information->sampleTable->ChunkOffset->size;
		size -= writer->mdia->information->sampleTable->SampleToChunk->size;
		SizeAtom((Atom *)writer->stsc);
		SizeAtom(writer->stco);
		size += writer->stsc->size;
		size += writer->stco->size;
	}
	return size;
}



//Write a sample to the file - this is only called for self-contained media
M4Err WriteSample(M4Movie *movie, u32 size, u64 offset, u8 isEdited, BitStream *bs)
{
	char *buf;
	DataMap *map;
	u32 bytes;

	buf = (char*)malloc(size);
	if (!buf) return M4OutOfMem;

	if (isEdited) {
		map = movie->editFileMap;
	} else {
		map = movie->movieFileMap;
	}
	//get the payload...
	if (size == 88) 
		size = 88;
	bytes = DataMap_GetData(map, buf, size, offset);
	if (bytes != size) {
		free(buf);
		return M4IOErr;
	}
	//write it to our stream...
	bytes = BS_WriteData(bs, (unsigned char*)buf, size);
	if (bytes != size) {
		free(buf);
		return M4IOErr;
	}
	free(buf);
	return M4OK;
}


//this function writes track by track in the order of tracks inside the moov...
M4Err DoWrite(M4Movie *movie, Chain *writers, BitStream *bs, u8 Emulation, u64 StartOffset)
{
	u32 i;
	M4Err e;
	TrackWriter *writer;
	u64 offset, sampOffset, predOffset;
	u32 chunkNumber, descIndex, sampSize;
	u8 isEdited, force;
	u64 mdatSize = 0;

	offset = (u64) StartOffset;
	predOffset = 0;
	for (i = 0; i < ChainGetCount(writers); i++) {
		writer = (TrackWriter*)ChainGetEntry(writers, i);
		while (!writer->isDone) {
			//To Check: are empty sample tables allowed ???
			if (writer->sampleNumber > writer->mdia->information->sampleTable->SampleSize->sampleCount) {
				writer->isDone = 1;
				continue;
			}
			e = stbl_GetSampleInfos(writer->mdia->information->sampleTable, writer->sampleNumber, &sampOffset, &chunkNumber, &descIndex, &isEdited);
			if (e) return e;
			e = stbl_GetSampleSize(writer->mdia->information->sampleTable->SampleSize, writer->sampleNumber, &sampSize);
			if (e) return e;

			//update our chunks. 
			force = 0;
			if (movie->openMode == M4_OPEN_WRITE) {
				offset = sampOffset;
				if (predOffset != offset) 
					force = 1;
			}
			//update our global offset...
			if (Media_IsSelfContained(writer->mdia, descIndex) ) {
				e = stbl_SetChunkAndOffset(writer->mdia->information->sampleTable, writer->sampleNumber, descIndex, writer->stsc, &writer->stco, offset, force);
				if (e) return e;
				if (movie->openMode == M4_OPEN_WRITE) {
					predOffset = sampOffset + sampSize;
				} else {
					offset += sampSize;
					mdatSize += sampSize;
				}
			} else {
				if (predOffset != offset) force = 1;
				predOffset = sampOffset + sampSize;
				//we have a DataRef, so use the offset idicated in sampleToChunk and ChunkOffset tables...
				e = stbl_SetChunkAndOffset(writer->mdia->information->sampleTable, writer->sampleNumber, descIndex, writer->stsc, &writer->stco, sampOffset, force);
				if (e) return e;
			}
			//we write the sample if not emulation
			if (!Emulation) {
				if (Media_IsSelfContained(writer->mdia, descIndex) ) {
					e = WriteSample(movie, sampSize, sampOffset, isEdited, bs);
					if (e) return e;
				}
			}
			//ok, the track is done
			if (writer->sampleNumber == writer->mdia->information->sampleTable->SampleSize->sampleCount) {
				writer->isDone = 1;
			} else {
				writer->sampleNumber ++;
			}
		}
	}
	//set the mdatSize...
	movie->mdat->dataSize = mdatSize;
	return M4OK;
}


//write the file track by track, with moov atom before or after the mdat
M4Err WriteFlat(M4Movie *movie, u8 moovFirst, BitStream *bs)
{
	M4Err e;
	u32 i;
	u64 offset, finalOffset, totSize, begin, firstSize, finalSize;
	Atom *a;
	Chain *writers = NewChain();

	begin = totSize = 0;

	//first setup the writers
	e = SetupWriters(movie, writers, 0);
	if (e) goto exit;

	if (!moovFirst) {
		//in capture mode, depending on the version, the mdat is at the begining
		//or right after the brand.
		if (movie->openMode == M4_OPEN_WRITE) {
			if (movie->brand) {
				SizeAtom((Atom *)movie->brand);
				begin = movie->brand->size;
			} else {
				begin = 0;
			}
			totSize = DataMap_GetTotalOffset(movie->editFileMap) - begin;
		} else {
			//the brand atom is always first no matter what
			if (movie->brand) {
				e = SizeAtom((Atom *)movie->brand);
				if (e) goto exit;
				e = WriteAtom((Atom *)movie->brand, bs);
				if (e) goto exit;
			}
		}

		//if the moov is at the end, write directly
		for (i=0; i<ChainGetCount(movie->TopAtoms); i++) {
			a = (Atom*)ChainGetEntry(movie->TopAtoms, i);
			switch (a->type) {
			case MovieAtomType:
				break;
			case MediaDataAtomType:
				//in case we're capturing
				if (movie->openMode == M4_OPEN_WRITE) {
					//emulate a write to recreate our tables (media data already written)
					e = DoWrite(movie, writers, bs, 1, begin);
					if (e) goto exit;
					continue;
				}
				//to avoid computing the size each time write always 4 + 4 + 8 bytes before
				begin = BS_GetPosition(bs);
				BS_WriteInt(bs, 0, 64);
				BS_WriteInt(bs, 0, 64);
				e = DoWrite(movie, writers, bs, 0, BS_GetPosition(bs));
				if (e) goto exit;
				totSize = BS_GetPosition(bs) - begin;
				break;
			default:
				e = SizeAtom(a);
				if (e) goto exit;
				e = WriteAtom(a, bs);
				if (e) goto exit;
				break;
			}
		}

		//OK, write the movie atom.
		e = WriteMoov(movie, writers, bs);
		if (e) goto exit;

		//set the mdat at the begining
		offset = BS_GetPosition(bs);
		e = BS_Seek(bs, begin);
		if (e) goto exit;
		//size
		if (totSize > 0xFFFFFFFF) {
			BS_WriteInt(bs, 1, 32);
		} else {
			BS_WriteInt(bs, (u32) totSize, 32);
		}
		//type
		BS_WriteInt(bs, MediaDataAtomType, 32);
		//extendedSize
		if (totSize > 0xFFFFFFFF) {
			BS_WriteLongInt(bs, totSize, 64);
		}
		e = BS_Seek(bs, offset);
		movie->mdat->size = totSize;
		goto exit;
	}

	//nope, we have to write the moov first. The pb is that 
	//1 - we don't know its size till the mdat is written
	//2 - we don't know the ofset at which the mdat will start...
	//3 - once the mdat is written, the chunkOffset table can have changed...
	
	//the brand atom is always first no matter what
	if (movie->brand) {
		e = SizeAtom((Atom *)movie->brand);
		if (e) goto exit;
		e = WriteAtom((Atom *)movie->brand, bs);
		if (e) goto exit;
	}
	//What we will do is first emulate the write from the begining...
	//note: this will set the size of the mdat
	e = DoWrite(movie, writers, bs, 1, BS_GetPosition(bs));
	if (e) goto exit;
	
	firstSize = GetMoovSize(movie, writers);
	offset = (firstSize > 0xFFFFFFFF ? firstSize + 8 : firstSize) + 8 + (movie->mdat->dataSize > 0xFFFFFFFF ? 8 : 0);
	e = ShiftOffset(writers, offset);
	if (e) goto exit;
	//get the size and see if it has changed (eg, we moved to 64 bit offsets)
	finalSize = GetMoovSize(movie, writers);
	if (firstSize != finalSize) {
		//we need to remove our offsets
		ResetWriters(writers);
		finalOffset = (finalSize > 0xFFFFFFFF ? finalSize + 8 : finalSize) + 8 + (movie->mdat->dataSize > 0xFFFFFFFF ? 8 : 0);
		//OK, now we're sure about the final size.
		//we don't need to re-emulate, as the only thing that changed is the offset
		//so just shift the offset
		e = ShiftOffset(writers, finalOffset - offset);
		if (e) goto exit;
	}
	//now write our stuff
	e = WriteMoov(movie, writers, bs);
	if (e) goto exit;
	e = SizeAtom((Atom *)movie->mdat);
	if (e) goto exit;
	e = WriteAtom((Atom *)movie->mdat, bs);
	if (e) goto exit;

	//we don't need the offset as the moov is already written writing...
	ResetWriters(writers);
	e = DoWrite(movie, writers, bs, 0, 0);
	if (e) goto exit;
	//then the rest
	for (i = 0; i < ChainGetCount(movie->TopAtoms); i++) {
		a = (Atom*)ChainGetEntry(movie->TopAtoms, i);
		switch (a->type) {
		case MovieAtomType:
		case MediaDataAtomType:
			break;
		default:
			e = SizeAtom(a);
			if (e) goto exit;
			e = WriteAtom(a, bs);
			if (e) goto exit;
		}
	}

exit:
	CleanWriters(writers);
	DeleteChain(writers);
	return e;
}

M4Err DoFullInterleave(M4Movie *movie, Chain *writers, BitStream *bs, u8 Emulation, u32 StartOffset)
{

	u32 i, tracksDone;
	TrackWriter *tmp, *curWriter, *prevWriter;
	M4Err e;
	u32 DTS, descIndex, sampSize, chunkNumber, DTStmp, TStmp;
	s32 res;
	u16 curGroupID, curTrackPriority;
	u8 forceNewChunk, writeGroup, isEdited;
	//this is used to emulate the write ...
	u64 offset, totSize, sampOffset;
	e = M4OK;

	totSize = 0;
	curGroupID = 1;

	prevWriter = NULL;
	//we emulate a write from this offset...
	offset = StartOffset;
	writeGroup = 1;
	tracksDone = 0;

	
	//browse each groups
	while (1) {
		writeGroup = 1;

		//proceed a group
		while (writeGroup) {
			//first get the appropriated sample for the min time in this group
			curWriter = NULL;
			DTStmp = -1;
			TStmp = 0;
			curTrackPriority = -1;

			for (i = 0; i < ChainGetCount(writers); i++) {
				tmp = ChainGetEntry(writers, i);

				//is it done writing ?
				//is it in our group ??
				if (tmp->isDone || tmp->mdia->information->sampleTable->groupID != curGroupID) continue;

				//OK, get the current sample in this track
				stbl_GetSampleDTS(tmp->mdia->information->sampleTable->TimeToSample, tmp->sampleNumber, &DTS);				
				res = TStmp ? DTStmp * tmp->timeScale - DTS * TStmp : 0;
				if (res < 0) continue;
				if ((!res) && curTrackPriority <= tmp->mdia->information->sampleTable->trackPriority) continue;
				curWriter = tmp;
				curTrackPriority = tmp->mdia->information->sampleTable->trackPriority;
				DTStmp = DTS;
				TStmp = tmp->timeScale;
			}
			//no sample found, we're done with this group
			if (!curWriter) {
				//we're done with the group
				curTrackPriority = 0;
				writeGroup = 0;
				continue;
			}
			//To Check: are empty sample tables allowed ???
			if (curWriter->sampleNumber > curWriter->mdia->information->sampleTable->SampleSize->sampleCount) {
				curWriter->isDone = 1;
				tracksDone ++;
				continue;
			}

			e = stbl_GetSampleInfos(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, &sampOffset, &chunkNumber, &descIndex, &isEdited);
			if (e) return e;
			e = stbl_GetSampleSize(curWriter->mdia->information->sampleTable->SampleSize, curWriter->sampleNumber, &sampSize);
			if (e) return e;
			
			//do we actually write, or do we emulate ?
			if (Emulation) {
				//are we in the same track ??? If not, force a new chunk when adding this sample
				if (curWriter != prevWriter) {
					forceNewChunk = 1;
				} else {
					forceNewChunk = 0;
				}
				//update our offsets...
				if (Media_IsSelfContained(curWriter->mdia, descIndex) ) {
					e = stbl_SetChunkAndOffset(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, descIndex, curWriter->stsc, &curWriter->stco, offset, forceNewChunk);
					if (e) return e;
					offset += sampSize;
					totSize += sampSize;
				} else {
					//we have a DataRef, so use the offset idicated in sampleToChunk 
					//and ChunkOffset tables...
					e = stbl_SetChunkAndOffset(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, descIndex, curWriter->stsc, &curWriter->stco, sampOffset, 0);
					if (e) return e;
				}
			} else {
				//this is no game, we're writing ....
				if (Media_IsSelfContained(curWriter->mdia, descIndex) ) {
					e = WriteSample(movie, sampSize, sampOffset, isEdited, bs);
					if (e) return e;
				}
			}
			//ok, the sample is done
			if (curWriter->sampleNumber == curWriter->mdia->information->sampleTable->SampleSize->sampleCount) {
				curWriter->isDone = 1;
				//one more track done...
				tracksDone ++;
			} else {
				curWriter->sampleNumber ++;
			}
			prevWriter = curWriter;
		}
		//if all our track are done, break
		if (tracksDone == ChainGetCount(writers)) break;
		//go to next group
		curGroupID ++;
	}
	movie->mdat->dataSize = totSize;
	return M4OK;
}


M4Err DoInterleave(M4Movie *movie, Chain *writers, BitStream *bs, u8 Emulation, u32 StartOffset)
{
	u32 i, tracksDone;
	TrackWriter *tmp, *curWriter;
	M4Err e;
	u32 DTS, descIndex, sampSize, chunkNumber;
	u16 curGroupID;
	u8 forceNewChunk, writeGroup, isEdited;
	//this is used to emulate the write ...
	u64 offset, totSize, sampOffset;
	u32 count;
	
	if (movie->storageMode == M4_FULL_INTERLEAVED) 
		return DoFullInterleave(movie, writers, bs, Emulation, StartOffset);

	e = M4OK;

	totSize = 0;
	curGroupID = 1;
	//we emulate a write from this offset...
	offset = StartOffset;
	writeGroup = 1;
	tracksDone = 0;

	count = ChainGetCount(writers);
	//browse each groups
	while (1) {
		writeGroup = 1;

		//proceed a group
		while (writeGroup) {
			curWriter = NULL;
			for (i=0 ; i < count; i++) {
				tmp = (TrackWriter*)ChainGetEntry(writers, i);
				
				//is it done writing ?
				if (tmp->isDone) continue;

				//is it in our group ??
				if (tmp->mdia->information->sampleTable->groupID != curGroupID) continue;

				//write till this chunk is full on this track...
				while (1) {
					//To Check: are empty sample tables allowed ???
					if (tmp->sampleNumber > tmp->mdia->information->sampleTable->SampleSize->sampleCount) {
						tmp->isDone = 1;
						tracksDone ++;
						break;
					}

					//OK, get the current sample in this track
					stbl_GetSampleDTS(tmp->mdia->information->sampleTable->TimeToSample, tmp->sampleNumber, &DTS);

					//can this samle fit in our chunk ?
					if ( ( (DTS - tmp->DTSprev) + tmp->chunkDur) *  movie->moov->mvhd->timeScale > movie->interleavingTime * tmp->timeScale) {
						//in case the sample is longer than InterleaveTime
						if (!tmp->chunkDur) {
							forceNewChunk = 1;
						} else {
							//this one is full. go to next one (exit the loop)
							tmp->chunkDur = 0;
							break;
						}
					} else {
						forceNewChunk = tmp->chunkDur ? 0 : 1;
					}
					//OK, we can write this track
					curWriter = tmp;
					
					//small check for first 2 samples (DTS = 0 :)
					if (tmp->sampleNumber == 2 && !tmp->chunkDur) forceNewChunk = 0;

					tmp->chunkDur += (DTS - tmp->DTSprev);
					tmp->DTSprev = DTS;

					e = stbl_GetSampleInfos(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, &sampOffset, &chunkNumber, &descIndex, &isEdited);
					if (e) return e;
					e = stbl_GetSampleSize(curWriter->mdia->information->sampleTable->SampleSize, curWriter->sampleNumber, &sampSize);
					if (e) return e;
					
					//do we actually write, or do we emulate ?
					if (Emulation) {
						//update our offsets...
						if (Media_IsSelfContained(curWriter->mdia, descIndex) ) {
							e = stbl_SetChunkAndOffset(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, descIndex, curWriter->stsc, &curWriter->stco, offset, forceNewChunk);
							if (e) return e;
							offset += sampSize;
							totSize += sampSize;
						} else {
							//we have a DataRef, so use the offset idicated in sampleToChunk 
							//and ChunkOffset tables...
							e = stbl_SetChunkAndOffset(curWriter->mdia->information->sampleTable, curWriter->sampleNumber, descIndex, curWriter->stsc, &curWriter->stco, sampOffset, 0);
							if (e) return e;
						}
					} else {
						//this is no game, we're writing ....
						if (Media_IsSelfContained(curWriter->mdia, descIndex) ) {
							e = WriteSample(movie, sampSize, sampOffset, isEdited, bs);
							if (e) return e;
						}
					}
					//ok, the sample is done
					if (curWriter->sampleNumber == curWriter->mdia->information->sampleTable->SampleSize->sampleCount) {
						curWriter->isDone = 1;
						//one more track done...
						tracksDone ++;
						break;
					} else {
						curWriter->sampleNumber ++;
					}
				}
			}
			//no sample found, we're done with this group
			if (!curWriter) {
				writeGroup = 0;
				continue;
			}
		}
		//if all our track are done, break
		if (tracksDone == ChainGetCount(writers)) break;
		//go to next group
		curGroupID ++;
	}
	movie->mdat->dataSize = totSize;
	return M4OK;
}


M4Err WriteInterleaved(M4Movie *movie, u8 moovFirst, BitStream *bs)
{
	M4Err e;
	u32 i;
	Atom *a;
	u64 firstSize, finalSize, offset, finalOffset;
	Chain *writers = NewChain();

	//first setup the writers
	e = SetupWriters(movie, writers, 1);
	if (e) goto exit;


	if (movie->brand) {
		e = SizeAtom((Atom *)movie->brand);
		if (e) goto exit;
		e = WriteAtom((Atom *)movie->brand, bs);
		if (e) goto exit;
	}

	e = DoInterleave(movie, writers, bs, 1, (u32) BS_GetPosition(bs));
	if (e) goto exit;

	firstSize = GetMoovSize(movie, writers);
	offset = (firstSize > 0xFFFFFFFF ? firstSize + 8 : firstSize) + 8 + (movie->mdat->dataSize > 0xFFFFFFFF ? 8 : 0);
	e = ShiftOffset(writers, offset);
	if (e) goto exit;
	//get the size and see if it has changed (eg, we moved to 64 bit offsets)
	finalSize = GetMoovSize(movie, writers);
	if (firstSize != finalSize) {
		//we need to remove our offsets
		ResetWriters(writers);
		finalOffset = (finalSize > 0xFFFFFFFF ? finalSize + 8 : finalSize) + 8 + (movie->mdat->dataSize > 0xFFFFFFFF ? 8 : 0);
		//OK, now we're sure about the final size -> shift the offsets
		//we don't need to re-emulate, as the only thing that changed is the offset
		//so just shift the offset
		e = ShiftOffset(writers, finalOffset - offset);
		if (e) goto exit;
	}
	//now write our stuff
	e = WriteMoov(movie, writers, bs);
	if (e) goto exit;
	e = SizeAtom((Atom *)movie->mdat);
	if (e) goto exit;
	e = WriteAtom((Atom *)movie->mdat, bs);
	if (e) goto exit;

	//we don't need the offset as we are writing...
	ResetWriters(writers);
	e = DoInterleave(movie, writers, bs, 0, 0);
	if (e) goto exit;
	//then the rest
	for (i = 0; i < ChainGetCount(movie->TopAtoms); i++) {
		a = (Atom*)ChainGetEntry(movie->TopAtoms, i);
		switch (a->type) {
		case MovieAtomType:
		case MediaDataAtomType:
			break;
		default:
			e = SizeAtom(a);
			if (e) goto exit;
			e = WriteAtom(a, bs);
			if (e) goto exit;
		}
	}

exit:
	CleanWriters(writers);
	DeleteChain(writers);
	return e;
}


M4Err WriteToFile(M4Movie *movie)
{
	FILE *stream;
	BitStream *bs;
	M4Err e = M4OK;
	if (!movie) return M4BadParam;

	if (movie->openMode == M4_OPEN_READ) return M4BadParam;

	e = M4_InsertCopyright(movie);
	if (e) return e;

	//capture mode: we don't need a new bitstream
	if (movie->openMode == M4_OPEN_WRITE) {
		return WriteFlat(movie, 0, movie->editFileMap->bs);
	}

	//OK, we need a new bitstream
	stream = fopen(movie->finalName, "wb");
	if (!stream) return M4IOErr;
	bs = NewBitStreamFromFile(stream, BS_FILE_WRITE_ONLY);
	if (!bs) {
		fclose(stream);
		return M4OutOfMem;
	}

	switch (movie->storageMode) {
	case M4_FULL_INTERLEAVED:
	case M4_INTERLEAVED:
		e = WriteInterleaved(movie, 0, bs);
		break;
	case M4_STREAMABLE:
		e = WriteFlat(movie, 1, bs);
		break;
	default:
		e = WriteFlat(movie, 0, bs);
		break;
	}
	
	DeleteBitStream(bs);
	fclose(stream);
	return e;
}



#endif	//M4_READ_ONLY

