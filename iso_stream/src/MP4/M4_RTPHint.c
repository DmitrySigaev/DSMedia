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

UserDataMap *udta_getEntry(UserDataAtom *ptr, u32 atomType);

#ifndef M4_READ_ONLY

M4Err M4H_RTP_SetPacketFlags(M4File *the_file, u32 trackNumber,
						   u8 PackingBit, 
						   u8 eXtensionBit, 
						   u8 MarkerBit, 
						   u8 B_frame, 
						   u8 IsRepeatedPacket)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	RTPPacket *pck;
	u32 dataRefIndex, ind;
	M4Err e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &dataRefIndex);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;

	ind = ChainGetCount(entry->w_sample->packetTable);
	if (!ind) return M4BadParam;
	pck = ChainGetEntry(entry->w_sample->packetTable, ind-1);

	pck->P_bit = PackingBit ? 1 : 0;
	pck->X_bit = eXtensionBit ? 1 : 0;
	pck->M_bit = MarkerBit ? 1 : 0;
	pck->B_bit = B_frame ? 1 : 0;
	pck->R_bit = IsRepeatedPacket ? 1 : 0;
	return M4OK;
}

M4Err M4H_RTP_NewPacket(M4File *the_file, u32 trackNumber, 
						   s32 relativeTime, 
						   u8 PackingBit, 
						   u8 eXtensionBit, 
						   u8 MarkerBit, 
						   u8 PayloadType, 
						   u8 B_frame, 
						   u8 IsRepeatedPacket, 
						   u16 SequenceNumber)
{
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	RTPPacket *pck;
	u32 dataRefIndex;
	M4Err e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &dataRefIndex);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;

	pck = (RTPPacket *) New_HintPacket(entry->w_sample->HintType);

	pck->P_bit = PackingBit ? 1 : 0;
	pck->X_bit = eXtensionBit ? 1 : 0;
	pck->M_bit = MarkerBit ? 1 : 0;
	pck->payloadType = PayloadType;
	pck->SequenceNumber = SequenceNumber;
	pck->B_bit = B_frame ? 1 : 0;
	pck->R_bit = IsRepeatedPacket ? 1 : 0;
	pck->relativeTransTime = relativeTime;
	return ChainAddEntry(entry->w_sample->packetTable, pck);
}


//set the time offset of this packet. This enables packets to be placed in the hint track 
//in decoding order, but have their presentation time-stamp in the transmitted 
//packet be in a different order. Typically used for MPEG video with B-frames
M4Err M4H_RTP_SetPacketTimeOffset(M4File *the_file, u32 trackNumber, s32 timeOffset)
{
	RtpoAtom *rtpo;
	TrackAtom *trak;
	HintSampleEntryAtom *entry;
	RTPPacket *pck;
	u32 dataRefIndex, i;
	M4Err e;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak || !CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	e = Media_GetSampleDesc(trak->Media, trak->Media->information->sampleTable->currentEntryIndex, (SampleEntryAtom **) &entry, &dataRefIndex);
	if (e) return e;
	if (!entry->w_sample) return M4BadParam;

	pck = ChainGetEntry(entry->w_sample->packetTable, ChainGetCount(entry->w_sample->packetTable) - 1);
	if (!pck) return M4BadParam;

	//look in the TLV
	for (i=0; i<ChainGetCount(pck->TLV); i++) {
		rtpo = ChainGetEntry(pck->TLV, i);
		if (rtpo->type == rtpoAtomType) {
			rtpo->timeOffset = timeOffset;
			return M4OK;
		}
	}
	//not found, add it
	rtpo = (RtpoAtom *) CreateAtom(rtpoAtomType);
	rtpo->timeOffset = timeOffset;

	return ChainAddEntry(pck->TLV, rtpo);
}

//add an SDP line to the SDP container at the track level (media-specific SDP info)
M4Err M4H_SDP_TrackAddLine(M4File *the_file, u32 trackNumber, const char *text)
{
	TrackAtom *trak;
	UserDataMap *map;
	HintTrackInfoAtom *hnti;
	SDPAtom *sdp;
	M4Err e;
	char *buf;
	M4Err hnti_AddAtom(HintTrackInfoAtom *hnti, Atom *a);

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//currently, only RTP hinting supports SDP
	if (!CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	map = udta_getEntry(trak->udta, HintTrackInfoAtomType);
	if (!map) return M4InvalidMP4File;

	//we should have only one HNTI in the UDTA
	if (ChainGetCount(map->atomList) != 1) return M4InvalidMP4File;

	hnti = ChainGetEntry(map->atomList, 0);
	if (!hnti->SDP) {
		e = hnti_AddAtom(hnti, CreateAtom(SDPAtomType));
		if (e) return e;
	}
	sdp = (SDPAtom *) hnti->SDP;

	if (!sdp->sdpText) {
		sdp->sdpText = malloc(sizeof(char) * (strlen(text) + 3));
		strcpy(sdp->sdpText, text);
		strcat(sdp->sdpText, "\r\n");
		return M4OK;
	}
	buf = malloc(sizeof(char) * (strlen(sdp->sdpText) + strlen(text) + 3));
	strcpy(buf, sdp->sdpText);
	strcat(buf, text);
	strcat(buf, "\r\n");
	free(sdp->sdpText);
	sdp->sdpText = buf;	
	return M4OK;
}

//remove all SDP info at the track level
M4Err M4H_SDP_CleanTrack(M4File *the_file, u32 trackNumber)
{
	TrackAtom *trak;
	UserDataMap *map;
	HintTrackInfoAtom *hnti;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;

	//currently, only RTP hinting supports SDP
	if (!CheckHintFormat(trak, M4_Hint_RTP)) return M4BadParam;

	map = udta_getEntry(trak->udta, HintTrackInfoAtomType);
	if (!map) return M4InvalidMP4File;

	//we should have only one HNTI in the UDTA
	if (ChainGetCount(map->atomList) != 1) return M4InvalidMP4File;

	hnti = ChainGetEntry(map->atomList, 0);
	if (!hnti->SDP) return M4OK;
	//and free the SDP
	free(((SDPAtom *)hnti->SDP)->sdpText);
	((SDPAtom *)hnti->SDP)->sdpText = NULL;
	return M4OK;
}


//add an SDP line to the SDP container at the movie level (presentation SDP info)
//NOTE: the \r\n end of line for SDP is automatically inserted
M4Err M4H_SDP_MovieAddLine(M4File *the_file, const char *text)
{
	M4Movie *mov;
	UserDataMap *map;
	RTPAtom *rtp;
	M4Err e;
	HintTrackInfoAtom *hnti;
	char *buf;

	M4Err udta_AddAtom(UserDataAtom *ptr, Atom *a);
	M4Err hnti_AddAtom(HintTrackInfoAtom *hnti, Atom *a);
	M4Err moov_AddAtom(MovieAtom *moov, Atom *a);

	mov = (M4Movie *)the_file;
	//check if we have a udta ...
	if (!mov->moov->udta) {
		e = moov_AddAtom(mov->moov, CreateAtom(UserDataAtomType));
		if (e) return e;
	}
	//find a hnti in the udta
	map = udta_getEntry(mov->moov->udta, HintTrackInfoAtomType);
	if (!map) {
		e = udta_AddAtom(mov->moov->udta, CreateAtom(HintTrackInfoAtomType));
		if (e) return e;
		map = udta_getEntry(mov->moov->udta, HintTrackInfoAtomType);
	}

	//there should be one and only one hnti
	if (!ChainGetCount(map->atomList) ) {
		e = udta_AddAtom(mov->moov->udta, CreateAtom(HintTrackInfoAtomType));
		if (e) return e;
	}
	else if (ChainGetCount(map->atomList) < 1) return M4InvalidMP4File;

	hnti = ChainGetEntry(map->atomList, 0);

	if (!hnti->SDP) {
		//we have to create it by hand, as we have a duplication of atom type 
		//(RTPSampleEntryAtom and RTPAtom have the same type...)
		rtp = malloc(sizeof(RTPAtom));
		rtp->subType = SDPAtomType;
		rtp->type = RTPAtomType;
		rtp->sdpText = NULL;
		hnti_AddAtom(hnti, (Atom *)rtp);
	}
	rtp = (RTPAtom *) hnti->SDP;

	if (!rtp->sdpText) {
		rtp->sdpText = malloc(sizeof(char) * (strlen(text) + 3));
		strcpy(rtp->sdpText, text);
		strcat(rtp->sdpText, "\r\n");
		return M4OK;
	}
	buf = malloc(sizeof(char) * (strlen(rtp->sdpText) + strlen(text) + 3));
	strcpy(buf, rtp->sdpText);
	strcat(buf, text);
	strcat(buf, "\r\n");
	free(rtp->sdpText);
	rtp->sdpText = buf;	
	return M4OK;
}


//remove all SDP info at the movie level
M4Err M4H_SDP_CleanMovie(M4File *the_file)
{
	M4Movie *mov;
	UserDataMap *map;
	HintTrackInfoAtom *hnti;

	mov = (M4Movie *)the_file;
	//check if we have a udta ...
	if (!mov->moov->udta) return M4OK;

	//find a hnti in the udta
	map = udta_getEntry(mov->moov->udta, HintTrackInfoAtomType);
	if (!map) return M4OK;

	//there should be one and only one hnti
	if (ChainGetCount(map->atomList) != 1) return M4InvalidMP4File;
	hnti = ChainGetEntry(map->atomList, 0);

	//remove and destroy the entry
	ChainDeleteEntry(map->atomList, 0);
	DelAtom((Atom *)hnti);
	return M4OK;
}

#endif //M4_READ_ONLY

M4Err M4H_SDP_GetSDP(M4File *the_file, const char **sdp, u32 *length)
{
	M4Movie *mov;
	UserDataMap *map;
	HintTrackInfoAtom *hnti;
	RTPAtom *rtp;
	mov = (M4Movie *)the_file;
	*length = 0;
	*sdp = NULL;
	//check if we have a udta ...
	if (!mov->moov->udta) return M4OK;

	//find a hnti in the udta
	map = udta_getEntry(mov->moov->udta, HintTrackInfoAtomType);
	if (!map) return M4OK;

	//there should be one and only one hnti
	if (ChainGetCount(map->atomList) != 1) return M4InvalidMP4File;
	hnti = ChainGetEntry(map->atomList, 0);

	if (!hnti->SDP) return M4OK;
	rtp = (RTPAtom *) hnti->SDP;

	*length = strlen(rtp->sdpText);
	*sdp = rtp->sdpText;
	return M4OK;
}

M4Err M4H_SDP_GetTrackSDP(M4File *the_file, u32 trackNumber, const char **sdp, u32 *length)
{
	TrackAtom *trak;
	UserDataMap *map;
	HintTrackInfoAtom *hnti;
	SDPAtom *sdpa;

	*sdp = NULL;
	*length = 0;

	trak = GetTrackFromFile(the_file, trackNumber);
	if (!trak) return M4BadParam;
	if (!trak->udta) return M4OK;

	map = udta_getEntry(trak->udta, HintTrackInfoAtomType);
	if (!map) return M4InvalidMP4File;

	//we should have only one HNTI in the UDTA
	if (ChainGetCount(map->atomList) != 1) return M4InvalidMP4File;

	hnti = ChainGetEntry(map->atomList, 0);
	if (!hnti->SDP) return M4OK;
	sdpa = (SDPAtom *) hnti->SDP;

	*length = strlen(sdpa->sdpText);
	*sdp = sdpa->sdpText;
	return M4OK;
}


