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

Bool IsHintTrack(TrackAtom *trak)
{
	if (trak->Media->handler->handlerType != M4_HintMediaType) return 0;
	//QT doesn't specify any InfoHeader on HintTracks
	if (trak->Media->information->InfoHeader 
			&& trak->Media->information->InfoHeader->type != HintMediaHeaderAtomType) 
		return 0;

	return 1;
}

Bool CheckHintFormat(TrackAtom *trak, u32 HintType)
{
	HintMediaHeaderAtom *hmhd;
	if (!IsHintTrack(trak)) return 0;
	hmhd = (HintMediaHeaderAtom *)trak->Media->information->InfoHeader;
	if (hmhd->subType != HintType) return 0;
	return 1;
}

u32 GetHintFormat(TrackAtom *trak)
{
	return ((HintMediaHeaderAtom *)trak->Media->information->InfoHeader)->subType;
}


#ifndef M4_READ_ONLY

M4Err AdjustHintInfo(HintSampleEntryAtom *entry, u32 HintSampleNumber)
{
	u32 offset, count, i, size;
	HintPacket *pck;
	M4Err e;

	offset = Size_HintSample(entry->w_sample) - entry->w_sample->dataLength;
	count = ChainGetCount(entry->w_sample->packetTable);
	
	for (i=0; i<count; i++) {
		pck = ChainGetEntry(entry->w_sample->packetTable, i);
		if (offset && entry->w_sample->dataLength) {
			//adjust any offset in this packet (SampleDTE)
			e = Offset_HintPacket(entry->w_sample->HintType, pck, offset, HintSampleNumber);
			if (e) return e;
		}
		//adjust the max packet size for this sample entry...
		size = Length_HintPacket(entry->w_sample->HintType, pck);
		if (entry->MaxPacketSize < size) entry->MaxPacketSize = size;
	}
	return M4OK;
}


M4Err M4H_SetupHintTrack(M4File *the_file, u32 trackNumber, u32 HintType)
{
	M4Err e;
	TrackAtom *trak;
	TrackReferenceAtom *tref;
	TrackReferenceTypeAtom *dpnd;
	HintMediaHeaderAtom *hmhd;
	//UDTA related ...
	UserDataAtom *udta;

	M4Err tref_AddAtom(TrackReferenceAtom *ptr, Atom *a);
	M4Err trak_AddAtom(TrackAtom *ptr, Atom *a);
	M4Err moov_AddAtom(MovieAtom *ptr, Atom *a);
	M4Err udta_AddAtom(UserDataAtom *ptr, Atom *a);

	//what do we support
	switch (HintType) {
	case M4_Hint_RTP:
		break;
	default:
		return M4NotSupported;
	}

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4_GetLastError(the_file);
	if (((M4Movie *)the_file)->openMode != M4_OPEN_EDIT) return M4InvalidMP4Mode;

	//check we have a hint ...
	if ( !IsHintTrack(trak)) {
		return M4BadParam;
	}
	hmhd = (HintMediaHeaderAtom *)trak->Media->information->InfoHeader;
	//make sure the subtype was not already defined
	if (hmhd->subType) return M4HintPresent;
	//store the HintTrack format for later use...
	hmhd->subType = HintType;

	
	//hint tracks always have a tref and everything ...
	if (!trak->References) {
		if (!trak->References) {
			tref = (TrackReferenceAtom *) CreateAtom(TrackReferenceAtomType);
			e = trak_AddAtom(trak, (Atom *)tref);
			if (e) return e;
		}
	}
	tref = trak->References;

	//do we have a hint reference on this trak ???
	e = Track_FindRef(trak, HintTrackReferenceAtomType, &dpnd);
	if (e) return e;
	//if yes, return false (existing hint track...)
	if (dpnd) return M4HintPresent;

	//create our dep
	dpnd = (TrackReferenceTypeAtom *) CreateAtom(HintTrackReferenceAtomType);
	e = tref_AddAtom(tref, (Atom *) dpnd);
	if (e) return e;

	//for RTP, we need to do some UDTA-related stuff...
	if (HintType != M4_Hint_RTP) return M4OK;

	if (!trak->udta) {
		//create one
		udta = (UserDataAtom *) CreateAtom(UserDataAtomType);
		e = trak_AddAtom(trak, (Atom *) udta);
		if (e) return e;
	}
	udta = trak->udta;
	//end-of-udta marker for Darwin...
//	if (!udta->voidAtom) {
//		e = udta_AddAtom(udta, CreateAtom(VoidAtomType));
//		if (e) return e;
//	}

	//HNTI
	e = udta_AddAtom(udta, CreateAtom(HintTrackInfoAtomType));
	if (e) return e;

/*
	//NAME
	e = udta_AddAtom(udta, CreateAtom(nameAtomType));
	if (e) return e;
	//HINF
	return udta_AddAtom(udta, CreateAtom(HintInfoAtomType));
*/
	return M4OK;
}

//to use with internally supported protocols
M4Err M4H_NewHintDescription(M4File *the_file, u32 trackNumber, s32 HintTrackVersion, s32 LastCompatibleVersion, u8 Rely, u32 *HintDescriptionIndex)
{
	M4Err e;
	u32 drefIndex;
	TrackAtom *trak;
	HintSampleEntryAtom *hdesc;
	RelyHintEntry *relyA;

	M4Err stsd_AddAtom(SampleDescriptionAtom *ptr, Atom *a);

	trak = GetTrackFromFile(the_file, trackNumber);
	*HintDescriptionIndex = 0;
	if (!trak || !IsHintTrack(trak)) return M4BadParam;

	//OK, create a new HintSampleDesc
	hdesc = (HintSampleEntryAtom *) CreateAtom(GetHintFormat(trak));

	if (HintTrackVersion > 0) hdesc->HintTrackVersion = HintTrackVersion;
	if (LastCompatibleVersion > 0) hdesc->LastCompatibleVersion = LastCompatibleVersion;

	//create a data reference - WE ONLY DEAL WITH SELF-CONTAINED HINT TRACKS
	e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, NULL, NULL, &drefIndex);
	if (e) return e;
	hdesc->dataReferenceIndex = drefIndex;

	//add the entry to our table...
	e = stsd_AddAtom(trak->Media->information->sampleTable->SampleDescription, (Atom *) hdesc);
	if (e) return e;
	*HintDescriptionIndex = ChainGetCount(trak->Media->information->sampleTable->SampleDescription->atomList);

	//RTP needs a default timeScale... use the media one.
	if (CheckHintFormat(trak, M4_Hint_RTP)) {
		e = M4H_RTP_SetTimeScale(the_file, trackNumber, *HintDescriptionIndex, trak->Media->mediaHeader->timeScale);
		if (e) return e;
	}
	if (!Rely) return M4OK;

	//we need a rely atom (common to all protocols)
	relyA = (RelyHintEntry *) CreateAtom(relyHintEntryType);
	if (Rely == 1) {
		relyA->prefered = 1;
	} else {
		relyA->required = 1;
	}
	return ChainAddEntry(hdesc->HintDataTable, relyA);
}


/*******************************************************************
					RTP WRITING API
*******************************************************************/

//sets the RTP TimeScale
M4Err M4H_RTP_SetTimeScale(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeScale)
{
	TrackAtom *trak;
	HintSampleEntryAtom *hdesc;
	u32 i, count;
	TSHintEntry *ent;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	//OK, create a new HintSampleDesc
	hdesc = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, HintDescriptionIndex - 1);
	count = ChainGetCount(hdesc->HintDataTable);

	for (i=0; i< count; i++) {
		ent = ChainGetEntry(hdesc->HintDataTable, i);
		if (ent->type == timsHintEntryType) {			
			ent->timeScale = TimeScale;
			return M4OK;
		}
	}
	//we have to create a new entry...
	ent = (TSHintEntry *) CreateAtom(timsHintEntryType);
	ent->timeScale = TimeScale;
	return ChainAddEntry(hdesc->HintDataTable, ent);
}

//sets the RTP TimeOffset that the server will add to the packets
//if not set, the server adds a random offset
M4Err M4H_RTP_SetTimeOffset(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeOffset)
{
	TrackAtom *trak;
	HintSampleEntryAtom *hdesc;
	u32 i, count;
	TimeOffHintEntry *ent;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	//OK, create a new HintSampleDesc
	hdesc = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, HintDescriptionIndex - 1);
	count = ChainGetCount(hdesc->HintDataTable);

	for (i=0; i< count; i++) {
		ent = ChainGetEntry(hdesc->HintDataTable, i);
		if (ent->type == tsroHintEntryType) {			
			ent->TimeOffset = TimeOffset;
			return M4OK;
		}
	}
	//we have to create a new entry...
	ent = (TimeOffHintEntry *) CreateAtom(tsroHintEntryType);
	ent->TimeOffset = TimeOffset;
	return ChainAddEntry(hdesc->HintDataTable, ent);
}

//sets the RTP SequenceNumber Offset that the server will add to the packets
//if not set, the server adds a random offset
M4Err M4H_RTP_SetSequenceNumberOffset(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 SequenceNumberOffset)
{
	TrackAtom *trak;
	HintSampleEntryAtom *hdesc;
	u32 i, count;
	SeqOffHintEntry *ent;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	//OK, create a new HintSampleDesc
	hdesc = ChainGetEntry(trak->Media->information->sampleTable->SampleDescription->atomList, HintDescriptionIndex - 1);
	count = ChainGetCount(hdesc->HintDataTable);

	for (i=0; i< count; i++) {
		ent = ChainGetEntry(hdesc->HintDataTable, i);
		if (ent->type == snroHintEntryType) {			
			ent->SeqOffset = SequenceNumberOffset;
			return M4OK;
		}
	}
	//we have to create a new entry...
	ent = (SeqOffHintEntry *) CreateAtom(snroHintEntryType);
	ent->SeqOffset = SequenceNumberOffset;
	return ChainAddEntry(hdesc->HintDataTable, ent);
}

//Starts a new sample for the hint track. A sample is just a collection of packets
//the transmissionTime is indicated in the media timeScale of the hint track
M4Err M4H_BeginHintSample(M4File *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TransmissionTime)
{
	TrackAtom *trak;
	u32 descIndex, dataRefIndex;
	HintSample *samp;
	HintSampleEntryAtom *entry;
	M4Err e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;

	//assert we're increasing the timing...
	if (trak->Media->information->sampleTable->TimeToSample->w_LastDTS > TransmissionTime) return M4BadParam;

	//store the descIndex for this sample
	descIndex = HintDescriptionIndex;
	if (!HintDescriptionIndex) {
		descIndex = trak->Media->information->sampleTable->currentEntryIndex;
	}
	e = Media_GetSampleDesc(trak->Media, descIndex, (SampleEntryAtom **) &entry, &dataRefIndex);
	if (e) return e;
	if (!entry || !dataRefIndex) return M4BadParam;
	//set the current to this one if no packet is used
	if (entry->w_sample) return M4BadParam;
	trak->Media->information->sampleTable->currentEntryIndex = descIndex;

	//create a new sample based on the protocol type of the hint description entry
	samp = New_HintSample(entry->type);
	if (!samp) return M4NotSupported;

	//OK, let's store the time of this sample
	samp->TransmissionTime = TransmissionTime;
	//OK, set our sample in the entry...
	entry->w_sample = samp;
	return M4OK;
}

//stores the hint sample in the file
//set IsRandomAccessPoint if you want to indicate that this is a random access point 
//in the stream
M4Err M4H_EndHintSample(M4File *the_file, u32 trackNumber, u8 IsRandomAccessPoint)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	u32 dataRefIndex;
	M4Err e;
	BitStream *bs;
	M4Sample *samp;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &dataRefIndex);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;

	//first of all, we need to adjust the offset for data referenced IN THIS hint sample
	//and get some PckSize
	e = AdjustHintInfo(entry, trak->Media->information->sampleTable->SampleSize->sampleCount + 1);
	if (e) return e;	
	
	//ok, let's write the sample
	bs = NewBitStream(NULL, 0, BS_WRITE);
	e = Write_HintSample(entry->w_sample, bs);
	if (e) {
		DeleteBitStream(bs);
		return e;
	}
	BS_CutBuffer(bs);

	samp = M4_NewSample();
	samp->CTS_Offset = 0;
	samp->IsRAP = IsRandomAccessPoint;
	samp->DTS = entry->w_sample->TransmissionTime;
	//get the sample
	BS_GetContent(bs, (unsigned char **) &samp->data, &samp->dataLength);
	DeleteBitStream(bs);

	//finally add the sample 
	e = M4_AddSample(the_file, trackNumber, trak->Media->information->sampleTable->currentEntryIndex, samp);
	M4_DeleteSample(&samp);

	//and delete the sample in our entry ...
	Del_HintSample(entry->w_sample);
	entry->w_sample = NULL;
	return e;
}


//adds a blank chunk of data in the sample that is skipped while streaming
M4Err M4H_AddBlankData(M4File *the_file, u32 trackNumber, u8 AtBegin)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	u32 count;
	HintPacket *pck;
	EmptyDTE *dte;
	M4Err e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &count);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;
	count = ChainGetCount(entry->w_sample->packetTable);
	if (!count) return M4BadParam;
	pck = ChainGetEntry(entry->w_sample->packetTable, count - 1);

	dte = (EmptyDTE *) NewDTE(0);
	return AddDTE_HintPacket(entry->w_sample->HintType, pck, (GenericDTE *)dte, AtBegin);
}


//adds a chunk of data (max 14 bytes) in the packet that is directly copied 
//while streaming
M4Err M4H_AddDirectData(M4File *the_file, u32 trackNumber, char *data, u32 dataLength, u8 AtBegin)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	u32 count;
	HintPacket *pck;
	ImmediateDTE *dte;
	M4Err e;
	u32 offset = 0;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak) || (dataLength > 14)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &count);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;
	count = ChainGetCount(entry->w_sample->packetTable);
	if (!count) return M4BadParam;
	pck = ChainGetEntry(entry->w_sample->packetTable, count - 1);

	dte = (ImmediateDTE *) NewDTE(1);
	memcpy(dte->data, data + offset, dataLength);
	dte->dataLength = dataLength;
	return AddDTE_HintPacket(entry->w_sample->HintType, pck, (GenericDTE *)dte, AtBegin);
}

M4Err M4H_AddSampleData(M4File *the_file, u32 trackNumber, u32 SourceTrackID, u32 SampleNumber, u16 DataLength, u32 offsetInSample, char *extra_data, u8 AtBegin)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	u32 count;
	u16 refIndex;
	HintPacket *pck;
	SampleDTE *dte;
	M4Err e;
	TrackReferenceTypeAtom *hint;

	M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex);

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;


	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &count);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;
	count = ChainGetCount(entry->w_sample->packetTable);
	if (!count) return M4BadParam;
	pck = ChainGetEntry(entry->w_sample->packetTable, count - 1);

	dte = (SampleDTE *) NewDTE(2);

	dte->dataLength = DataLength;
	dte->sampleNumber = SampleNumber;
	dte->byteOffset = offsetInSample;

	//we're getting data from another track
	if (SourceTrackID != trak->Header->trackID) {
		//get (or set) the track reference index 
		e = Track_FindRef(trak, M4_HintTrack_Ref, &hint);
		if (e) return e;
		e = reftype_AddRefTrack(hint, SourceTrackID, &refIndex);
		if (e) return e;
		//WARNING: IN QT, MUST BE 0-based !!!
		dte->trackRefIndex = (u8) (refIndex - 1);
	} else {
		//we're in the hint track
		dte->trackRefIndex = -1;
		//basic check...
		if (SampleNumber > trak->Media->information->sampleTable->SampleSize->sampleCount + 1) {
			DelDTE((GenericDTE *)dte);
			return M4BadParam;
		}

		//are we in the current sample ??
		if (!SampleNumber || (SampleNumber == trak->Media->information->sampleTable->SampleSize->sampleCount + 1)) {
			//we adding some stuff in the current sample ...
			dte->byteOffset += entry->w_sample->dataLength;
			if (entry->w_sample->AdditionalData) {
				entry->w_sample->AdditionalData = realloc(entry->w_sample->AdditionalData, sizeof(char) * (entry->w_sample->dataLength + DataLength));
				memcpy(entry->w_sample->AdditionalData + entry->w_sample->dataLength, extra_data, DataLength);
				entry->w_sample->dataLength += DataLength;
			} else {
				entry->w_sample->AdditionalData = malloc(sizeof(char) * DataLength);
				memcpy(entry->w_sample->AdditionalData, extra_data, DataLength);
				entry->w_sample->dataLength = DataLength;
			}
			//and set the sample number ...
			dte->sampleNumber = trak->Media->information->sampleTable->SampleSize->sampleCount + 1;
		}
	}
	//OK, add the entry
	return AddDTE_HintPacket(entry->w_sample->HintType, pck, (GenericDTE *)dte, AtBegin);
}

M4Err M4H_AddStreamDescriptionData(M4File *the_file, u32 trackNumber, u32 SourceTrackID, u32 StreamDescriptionIndex, u16 DataLength, u32 offsetInDescription, u8 AtBegin)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	u32 count;
	u16 refIndex;
	HintPacket *pck;
	StreamDescDTE *dte;
	M4Err e;
	TrackReferenceTypeAtom *hint;

	M4Err reftype_AddRefTrack(TrackReferenceTypeAtom *ref, u32 trackID, u16 *outRefIndex);

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !IsHintTrack(trak)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &count);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;
	count = ChainGetCount(entry->w_sample->packetTable);
	if (!count) return M4BadParam;
	pck = ChainGetEntry(entry->w_sample->packetTable, count - 1);

	dte = (StreamDescDTE *) NewDTE(3);
	dte->byteOffset = offsetInDescription;
	dte->dataLength = DataLength;
	dte->streamDescIndex = StreamDescriptionIndex;
	if (SourceTrackID == trak->Header->trackID) {
		dte->trackRefIndex = -1;
	} else {
		//get (or set) the track reference index 
		e = Track_FindRef(trak, M4_HintTrack_Ref, &hint);
		if (e) return e;
		e = reftype_AddRefTrack(hint, SourceTrackID, &refIndex);
		if (e) return e;
		//WARNING: IN QT, MUST BE 0-based !!!
		dte->trackRefIndex = (u8) (refIndex - 1);
	}
	return AddDTE_HintPacket(entry->w_sample->HintType, pck, (GenericDTE *)dte, AtBegin);
}

#endif //M4_READ_ONLY
