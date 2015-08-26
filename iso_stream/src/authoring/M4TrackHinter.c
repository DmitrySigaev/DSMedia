/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#include <intern/m4_author_dev.h>

#ifndef M4_READ_ONLY

#include <m4_isomedia.h>

/*out-of-band sample desc (128 and 255 reserved in RFC)*/
#define SIDX_OFFSET_3GPP		129

u32 M4RTP_RSLHSize(M4RTPBuilder *, SLHeader *slh);

void GetAvgSampleInfos(M4File *file, u32 Track, u32 *avgSize, u32 *MaxSize, u32 *TimeDelta, u32 *maxCTSDelta, u32 *const_duration)
{
	u32 i, dI, prevTS, count, DTS;
	M4Sample *samp;

	*avgSize = *MaxSize = 0;
	*TimeDelta = 0;
	*maxCTSDelta = 0;
	prevTS = 0;
	DTS = 0;

	count = M4_GetSampleCount(file, Track);
	*const_duration = 0;

	for (i=0; i<count; i++) {
		samp = M4_GetSampleInfo(file, Track, i+1, &dI, NULL);
		//get the size
		*avgSize += samp->dataLength;
		if (*MaxSize < samp->dataLength) *MaxSize = samp->dataLength;
		//get the time
		*TimeDelta += samp->DTS+samp->CTS_Offset - prevTS;

		if (i==1) {
			*const_duration = samp->DTS+samp->CTS_Offset - prevTS;
		} else if ( (i<count-1) && (*const_duration != samp->DTS+samp->CTS_Offset - prevTS)) {
			*const_duration = 0;
		}

		prevTS = samp->DTS+samp->CTS_Offset;
		
		//get the CTS delta
		if (samp->CTS_Offset > *maxCTSDelta) *maxCTSDelta = samp->CTS_Offset;
		M4_DeleteSample(&samp);
	}
	if (count>1) *TimeDelta /= (count-1);
	*avgSize /= count;
	//delta is NOT an average, we need to know exactly how many bits are
	//needed to encode CTS-DTS for ANY samples
}

void InitSL_RTP(SLConfigDescriptor *slc)
{
	memset(slc, 0, sizeof(SLConfigDescriptor));
	slc->tag = SLConfigDescriptor_Tag;
	slc->useTimestampsFlag = 1;
	slc->timestampLength = 32;
}

void InitSL_NULL(SLConfigDescriptor *slc)
{
	memset(slc, 0, sizeof(SLConfigDescriptor));
	slc->tag = SLConfigDescriptor_Tag;
	slc->predefined = 0x01;
}



void MP4T_OnPacketDone(void *cbk, RTPHeader *header, 
						char *sl_rtp, u32 sl_rtp_size, 
						char *payload, u32 payload_size)
{
	u8 BFrame;
	MP4_RTPHinter *tkHint = (MP4_RTPHinter *)cbk;
	if (!tkHint || !tkHint->HintSample) return;
	assert(header->TimeStamp == tkHint->RTPTime);
	BFrame = 0;
	if (tkHint->hasBFrames && (tkHint->hinter->sl_header.compositionTimeStamp==tkHint->hinter->sl_header.decodingTimeStamp)) 
		BFrame = 1;

	M4H_RTP_SetPacketFlags(tkHint->file, tkHint->HintTrack, 0, 0, header->Marker, BFrame, 0);

	if (sl_rtp_size) {
		if (sl_rtp_size <= 14) {
			M4H_AddDirectData(tkHint->file, tkHint->HintTrack, sl_rtp, sl_rtp_size, 1);
		} else {
			M4H_AddSampleData(tkHint->file, tkHint->HintTrack, tkHint->HintID, 0, (u16) sl_rtp_size, 0, sl_rtp, 1);
		}
	}
	if (!tkHint->copy_media || !payload_size) return;

	/*do NOT specify offset as this is automatically added by the lib (to avoid keeping track of added data 
	size in the RTP HintSample)*/
	M4H_AddSampleData(tkHint->file, tkHint->HintTrack, tkHint->HintID, 0, (u16) payload_size, 0, payload, 0);
}


void MP4T_OnDataRef(void *cbk, u32 payload_size, u32 offset_from_orig)
{
	MP4_RTPHinter *tkHint = (MP4_RTPHinter *)cbk;
	if (!tkHint || tkHint->copy_media) return;
	/*add reference*/
	M4H_AddSampleData(tkHint->file, tkHint->HintTrack, tkHint->TrackID,
			tkHint->CurrentSample, (u16) payload_size, offset_from_orig + tkHint->base_offset_in_sample, 
			NULL, 0);
}

void MP4T_OnData(void *cbk, char *data, u32 data_size)
{
	MP4_RTPHinter *tkHint = (MP4_RTPHinter *)cbk;

	if (data_size <= 14) {
		M4H_AddDirectData(tkHint->file, tkHint->HintTrack, data, data_size, 0);
	} else {
		M4H_AddSampleData(tkHint->file, tkHint->HintTrack, tkHint->HintID, 0, (u16) data_size, 0, data, 0);
	}
}


void MP4T_OnNewPacket(void *cbk, RTPHeader *header)
{
	s32 res = 0;
	MP4_RTPHinter *tkHint = (MP4_RTPHinter *)cbk;
	if (!tkHint) return;
	if (tkHint->Last_RTP_SeqNum == header->SequenceNumber) return;

	/*do we need a new sample*/
	if (!tkHint->HintSample || (tkHint->RTPTime != header->TimeStamp)) {
		res = (u32) (tkHint->hinter->sl_header.compositionTimeStamp - tkHint->hinter->sl_header.decodingTimeStamp);
		if (res && !tkHint->hasBFrames) tkHint->hasBFrames = 1;
		/*close current sample*/
		if (tkHint->HintSample) M4H_EndHintSample(tkHint->file, tkHint->HintTrack, tkHint->SampleIsRAP);

		/*start new sample: We use DTS as the sampling instant (RTP TS) to make sure
		all packets are sent in order*/
		M4H_BeginHintSample(tkHint->file, tkHint->HintTrack, 1, header->TimeStamp-res);
		tkHint->HintSample ++;
		tkHint->RTPTime = header->TimeStamp;
		tkHint->Last_RTP_SeqNum = 0;
		tkHint->SampleIsRAP = tkHint->hinter->RAP_Packet;

	}
	/*create an RTP Packet with the appropriated marker flag - note: the flags are temp ones, 
	they are set when the full packet is signaled (to handle multi AUs per RTP)*/
	M4H_RTP_NewPacket(tkHint->file, tkHint->HintTrack, 0, 0, 0, header->Marker, header->PayloadType, 0, 0, header->SequenceNumber);
	tkHint->Last_RTP_SeqNum = header->SequenceNumber;
	/*Add the delta TS to make sure RTP TS is indeed the CTS (sampling time)*/
	if (res) M4H_RTP_SetPacketTimeOffset(tkHint->file, tkHint->HintTrack, res);
}

MP4_RTPHinter *NewTrackHinter(M4File *file, u32 TrackNum, 
							u32 Path_MTU, u32 flags, 
							u8 PayloadType, Bool copy_media,
							void (*OnProgress)(void *cbk_obj, u32 done, u32 total),
							void *cbk_obj,
							Bool hint_interleave,
							SLConfigDescriptor *sl, 
							u32 InterleaveGroupID,
							u8 InterleaveGroupPriority,
							M4Err *e)
{

	SLConfigDescriptor my_sl;
	u32 descIndex, MinSize, MaxSize, avgTS, maxDTS, streamType, oti, const_dur;
	u32 TrackMediaSubType, TrackMediaType, hintType, nbEdts, required_rate;
	const char *url, *urn;
	char *mpeg4mode;
	MP4_RTPHinter *tmp;
	ESDescriptor *esd;
	u32 GetNumBitsForMaxValue(u32 MaxValue);

	*e = M4BadParam;
	if (!file || !TrackNum || !M4_GetTrackID(file, TrackNum)) return NULL;

	if (!M4_GetSampleCount(file, TrackNum)) {
		*e = M4OK;
		return NULL;
	}
	*e = M4NotSupported;
	nbEdts = M4_GetEditSegmentCount(file, TrackNum);
	if (nbEdts>1) {
		u64 et, sd, mt;
		u8 em;
		M4_GetEditSegment(file, TrackNum, 1, &et, &sd, &mt, &em);
		if ((nbEdts>2) || (em!=M4_EDIT_EMPTY)) {
			fprintf(stdout, "Cannot hint track whith EditList\n");
			return NULL;
		}
	}
	if (nbEdts) M4_RemoveEditSegments(file, TrackNum);

	if (!M4_IsTrackEnabled(file, TrackNum)) return NULL;

	streamType = oti = 0;
	mpeg4mode = NULL;

	required_rate = 0;
	
	TrackMediaType = M4_GetMediaType(file, TrackNum);
	TrackMediaSubType = M4_GetMediaSubType(file, TrackNum, 1);

	/*timed-text is a bit special, we support multiple stream descriptions & co*/
	if (TrackMediaType==M4_TimedTextMediaType) {
		hintType = RTP_PAYT_3GPP_TEXT;
	} else {
		if (M4_GetStreamDescriptionCount(file, TrackNum) > 1) return NULL;

		TrackMediaSubType = M4_GetMediaSubType(file, TrackNum, 1);
		switch (TrackMediaSubType) {
		case M4_MPEG4_SubType:
			esd = M4_GetStreamDescriptor(file, TrackNum, 1);
			hintType = RTP_PAYT_MPEG4;
			if (esd) {
				streamType = esd->decoderConfig->streamType;
				oti = esd->decoderConfig->objectTypeIndication;
				if (esd->URLString) hintType = 0;
				/*autoconf, detect payloads*/
				if (flags & M4HF_AutoConf) {
					/*AAC*/
					if ((streamType==M4ST_AUDIO) && esd->decoderConfig->decoderSpecificInfo
						/*(nb: we use mpeg4 for MPEG-2 AAC)*/
						&& ((oti==0x40) || (oti==0x40) || (oti==0x66) || (oti==0x67) || (oti==0x68)) ) {

						u32 nbChan, sample_rate;
						M4ADecoderSpecificInfo a_cfg;
						M4A_GetConfig(esd->decoderConfig->decoderSpecificInfo->data, esd->decoderConfig->decoderSpecificInfo->dataLength, &a_cfg);
						nbChan = a_cfg.nb_chan;
						sample_rate = a_cfg.base_sr;
						switch (a_cfg.base_object_type) {
						case M4A_AAC_Main:
						case M4A_AAC_LC:
						case M4A_AAC_SBR:
						case M4A_AAC_LTP:
						case M4A_AAC_Scalable:
						case M4A_ER_AAC_LC:
						case M4A_ER_AAC_LTP:
						case M4A_ER_AAC_scalable:
							mpeg4mode = "AAC";
							break;
						case M4A_CELP:
						case M4A_ER_CELP:
							mpeg4mode = "CELP";
							break;
						}
						required_rate = sample_rate;
					}
					/*MPEG1/2 audio*/
					else if ((streamType==M4ST_AUDIO) && ((oti==0x69) || (oti==0x6B))) {
						u32 nbChan, sample_rate;
						M4Sample *samp = M4_GetSample(file, TrackNum, 1, NULL);
						u32 hdr = FOUR_CHAR_INT((u8)samp->data[0], (u8)samp->data[1], (u8)samp->data[2], (u8)samp->data[3]);
						nbChan = MP3_GetNumChannels(hdr);
						sample_rate = MP3_GetSamplingRate(hdr);
						M4_DeleteSample(&samp);
						hintType = RTP_PAYT_MPEG12;
						/*use official RTP/AVP payload type*/
						PayloadType = 14;
						/*for QT*/
						required_rate = 90000;
					}
					/*MPEG1/2 video*/
					else if ((streamType==M4ST_VISUAL) && ( ((oti>=0x60) && (oti<=0x65)) || (oti==0x6A)) ) {
						hintType = RTP_PAYT_MPEG12;
						/*use official RTP/AVP payload type*/
						PayloadType = 32;
						required_rate = 90000;
					}
				}
				OD_DeleteDescriptor((Descriptor**)&esd);
			}
			break;
		case M4_H263_SubType:
			hintType = RTP_PAYT_H263;
			required_rate = 90000;
			streamType = M4ST_VISUAL;
			break;
		case M4_AMR_SubType:
			required_rate = 8000;
			hintType = RTP_PAYT_AMR;
			streamType = M4ST_AUDIO;
			break;
		case M4_AMR_WB_SubType:
			required_rate = 16000;
			hintType = RTP_PAYT_AMR_WB;
			streamType = M4ST_AUDIO;
			break;
		default:
			/*ERROR*/
			hintType = 0;
			break;
		}
	}

	/*not hintable*/
	if (!hintType) return NULL;
	/*we only support self-contained files for hinting*/
	M4_GetStreamDataReference(file, TrackNum, 1, &url, &urn);
	if (url || urn) return NULL;
	
	*e = M4OutOfMem;
	tmp = malloc(sizeof(MP4_RTPHinter));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(MP4_RTPHinter));

	tmp->file = file;
	tmp->TrackNum = TrackNum;
	tmp->Path_MTU = Path_MTU;
	tmp->OnProgress = OnProgress;
	tmp->cbk_obj = cbk_obj;
	tmp->PayloadType = PayloadType;
	tmp->copy_media = copy_media;

	/*spatial scalability check*/
	if (M4_TrackHasTimeOffsets(file, TrackNum)) tmp->hasBFrames = 1;

	/*set storage modes*/
	tmp->Group = InterleaveGroupID;
	M4_SetTrackGroup(file, TrackNum, tmp->Group);
	tmp->Priority = 2*InterleaveGroupPriority;
	M4_SetTrackPriorityInGroup(file, TrackNum, tmp->Priority+1);

	/*get sample info*/
	GetAvgSampleInfos(file, TrackNum, &MinSize, &MaxSize, &avgTS, &maxDTS, &const_dur);

	/*update flags in MultiSL*/
	if (flags & M4HF_UseMulti) {
		if (MinSize != MaxSize) flags |= M4HF_SignalSize;
		if (!const_dur) flags |= M4HF_SignalTS;
	}
	if (tmp->hasBFrames) flags |= M4HF_SignalTS;
	tmp->HinterFlags = flags;

	/*setup SL with forced config - this should not be used!!*/
	if (sl) {
		memcpy(&my_sl, sl, sizeof(SLConfigDescriptor));
	} else {
		/*default SL for RTP */
		InitSL_RTP(&my_sl);

		my_sl.timestampResolution = M4_GetMediaTimeScale(file, TrackNum);
		/*override clockrate if set*/
		if (required_rate) {
			Double sc = required_rate;
			sc /= my_sl.timestampResolution;
			maxDTS = (u32) (maxDTS*sc);
			my_sl.timestampResolution = required_rate;
		}

		my_sl.AUSeqNumLength = GetNumBitsForMaxValue(M4_GetSampleCount(file, TrackNum));
		my_sl.CUDuration = const_dur;

		if (M4_MediaHasSyncPoints(file, TrackNum)) {
			my_sl.useRandomAccessPointFlag = 1;
		} else {
			my_sl.useRandomAccessPointFlag = 0;
			my_sl.useRandomAccessUnitsOnlyFlag = 1;
		}
	}

	// in case a different timescale was provided
	tmp->OrigTimeScale = M4_GetMediaTimeScale(file, TrackNum);
	tmp->hinter = NewRTPBuilder(hintType, &my_sl, flags, tmp, MP4T_OnNewPacket, MP4T_OnPacketDone, tmp->copy_media ? NULL : MP4T_OnDataRef, MP4T_OnData );

	//init the builder
	M4RTP_InitBuilder(tmp->hinter, PayloadType, Path_MTU, 
					   streamType, 1, MinSize, MaxSize, avgTS, maxDTS, hint_interleave, mpeg4mode);

	
	/*		Hint Track Setup	*/
	tmp->TrackID = M4_GetTrackID(file, TrackNum);
	tmp->HintID = tmp->TrackID + 65535;
	tmp->HintTrack = M4_NewTrack(file, tmp->HintID, M4_HintMediaType, my_sl.timestampResolution);
	M4H_SetupHintTrack(file, tmp->HintTrack, M4_Hint_RTP);
	/*create a hint description*/
	M4H_NewHintDescription(file, tmp->HintTrack, -1, -1, 0, &descIndex);
	M4H_RTP_SetTimeScale(file, tmp->HintTrack, descIndex, my_sl.timestampResolution);

	if (hintType==RTP_PAYT_MPEG4) {
		tmp->hinter->slMap.ObjectTypeIndication = oti;
		/*set this SL for extraction.*/
		M4_SetExtractionSLConfig(file, TrackNum, 1, &my_sl);
	}

#if 0
	/*QT FF: not setting these flags = server uses a random offset*/
	M4H_RTP_SetTimeOffset(file, tmp->HintTrack, 1, 0);
	/*we don't use seq offset for maintainance pruposes*/
	M4H_RTP_SetSequenceNumberOffset(file, tmp->HintTrack, 1, 0);
#endif
	*e = M4OK;
	return tmp;
}


void MP4T_DeleteHinter(MP4_RTPHinter *tkHinter)
{
	if (!tkHinter) return;

	if (tkHinter->hinter) M4RTP_DeleteBuilder(tkHinter->hinter);
	free(tkHinter);
}

M4Err MP4T_ProcessTrack(MP4_RTPHinter *tkHint)
{
	u32 i, j, descIndex, FragmentCount, ts, duration;
	u16 FragmentSize;
	u8 PadBits, AUEnd;
	Float ft;
	M4Sample *samp;

	tkHint->HintSample = tkHint->RTPTime = 0;

	tkHint->TotalSample = M4_GetSampleCount(tkHint->file, tkHint->TrackNum);
	ft = (Float) tkHint->hinter->sl_config.timestampResolution;
	ft /= tkHint->OrigTimeScale;

	for (i=0; i<tkHint->TotalSample; i++) {
		samp = M4_GetSample(tkHint->file, tkHint->TrackNum, i+1, &descIndex);
		if (!samp) return M4IOErr;

		//setup SL
		tkHint->CurrentSample = i + 1;

		ts = (u32) ((samp->DTS+samp->CTS_Offset)*ft);
		tkHint->hinter->sl_header.compositionTimeStamp = ts;
		ts = (u32) (samp->DTS*ft);
		tkHint->hinter->sl_header.decodingTimeStamp = ts;
		tkHint->hinter->sl_header.randomAccessPointFlag = samp->IsRAP;

		tkHint->base_offset_in_sample = 0;

		FragmentCount = M4_GetSampleFragmentCount(tkHint->file, tkHint->TrackNum, i+1);
		
		if (!FragmentCount) {
			if (tkHint->hinter->sl_config.usePaddingFlag) {
				M4_GetSamplePaddingBits(tkHint->file, tkHint->TrackNum, i+1, &PadBits);
				tkHint->hinter->sl_header.paddingBits = PadBits;
			} else {
				tkHint->hinter->sl_header.paddingBits = 0;
			}
			
			duration = M4_GetSampleDuration(tkHint->file, tkHint->TrackNum, i+1);
			M4RTP_ProcessData(tkHint->hinter, samp->data, samp->dataLength, 1, samp->dataLength, duration, (u8) (descIndex + SIDX_OFFSET_3GPP) );
			tkHint->hinter->sl_header.packetSequenceNumber += 1;
		}
		//we do have fragments
		else {
			AUEnd = 0;
			for (j=0; j<FragmentCount; j++) {
				
				tkHint->hinter->sl_header.paddingBits = 0;
				FragmentSize = M4_GetSampleFragmentSize(tkHint->file, tkHint->TrackNum, i+1, j+1);
				
				//padding bits is only for the LAST packet of the AU
				if (j==FragmentCount-1) {
					if (tkHint->hinter->sl_config.usePaddingFlag) {
						M4_GetSamplePaddingBits(tkHint->file, tkHint->TrackNum, i+1, &PadBits);
						tkHint->hinter->sl_header.paddingBits = PadBits;
					}
					AUEnd = (tkHint->base_offset_in_sample + FragmentSize == samp->dataLength) ? 1 : 0;
				}

				M4RTP_ProcessData(tkHint->hinter, samp->data + tkHint->base_offset_in_sample, FragmentSize, AUEnd, samp->dataLength, 0, 0);
				tkHint->base_offset_in_sample += FragmentSize;

				tkHint->hinter->sl_header.packetSequenceNumber += 1;
			}
			//looks like the last fragment info is missing ...
			if (tkHint->base_offset_in_sample < samp->dataLength) {
				AUEnd = 1;
				FragmentSize = samp->dataLength - tkHint->base_offset_in_sample;
				M4RTP_ProcessData(tkHint->hinter, samp->data + tkHint->base_offset_in_sample, FragmentSize, AUEnd, samp->dataLength, 0, 0);
				tkHint->hinter->sl_header.packetSequenceNumber += 1;
			}
		}

		M4_DeleteSample(&samp);

		//signal some progress
		if(tkHint->OnProgress != NULL){
			tkHint->OnProgress(tkHint->cbk_obj, tkHint->CurrentSample, tkHint->TotalSample);
		}

		tkHint->hinter->sl_header.AU_sequenceNumber += 1;
	}

	//flush
	M4RTP_ProcessData(tkHint->hinter, NULL, 0, 1, 0, 0, 0);

	M4H_EndHintSample(tkHint->file, tkHint->HintTrack, (u8) tkHint->hinter->RAP_Packet);
	return M4OK;
}

static void M4T_FormatTextSDP(M4RTPBuilder *builder, char *payload_name, char *sdpLine, M4File *file, u32 track)
{
	char buffer[2000];
	u32 w, h, i, m_w, m_h;
	s32 tx, ty;
	s16 l;
	sprintf(sdpLine, "a=fmtp:%d sver=60; ", builder->PayloadType);
	M4_GetTrackVideoInfo(file, track, &w, &h, &tx, &ty, &l);
	sprintf(buffer, "width=%d; height=%d; tx=%d; ty=%d; layer=%d; ", w>>16, h>>16, tx>>16, ty>>16, l);
	strcat(sdpLine, buffer);
	m_w = w;
	m_h = h;
	for (i=0; i<M4_GetTrackCount(file); i++) {
		switch (M4_GetMediaType(file, i+1)) {
		case M4_BIFSMediaType:
		case M4_VisualMediaType:
			M4_GetTrackVideoInfo(file, i+1, &w, &h, &tx, &ty, &l);
			if (w>m_w) m_w = w;
			if (h>m_h) m_h = h;
			break;
		default:
			break;
		}
	}
	sprintf(buffer, "max-w=%d; max-h=%d", m_w>>16, m_h>>16);
	strcat(sdpLine, buffer);

	strcat(sdpLine, "; tx3g=");
	for (i=0; i<M4_GetStreamDescriptionCount(file, track); i++) {
		char *tx3g;
		u32 tx3g_len, len;
		M4_GetEncodedTX3G(file, track, i+1, SIDX_OFFSET_3GPP, &tx3g, &tx3g_len);
		len = Base64_enc(tx3g, tx3g_len, buffer, 2000);
		free(tx3g);
		buffer[len] = 0;
		if (i) strcat(sdpLine, ", ");
		strcat(sdpLine, buffer);
	}
}

M4Err MP4T_FinalizeHintTrack(MP4_RTPHinter *tkHint, Bool AddSystemInfo)
{
	u32 TrackMediaType;
	u32 Width, Height;
	DecoderConfigDescriptor *dcd;
	char sdpLine[20000];
	char *mediaName, *payloadName;
	
	M4H_SDP_CleanTrack(tkHint->file, tkHint->TrackNum);
	TrackMediaType = M4_GetMediaType(tkHint->file, tkHint->TrackNum);
	mediaName = payloadName = NULL;
	switch (TrackMediaType) {
	case M4_VisualMediaType:
		M4_GetVisualEntrySize(tkHint->file, tkHint->TrackNum, 1, &Width, &Height);
		mediaName = "video";
		switch (tkHint->hinter->rtp_payt) {
		case RTP_PAYT_MPEG4:
			if (tkHint->hinter->slMap.ObjectTypeIndication==0x20) {
				if ( (tkHint->HinterFlags & M4HF_SignalRAP)
					|| (tkHint->HinterFlags & M4HF_SignalIDX)
					|| (tkHint->HinterFlags & M4HF_SignalSize)
					|| (tkHint->HinterFlags & M4HF_SignalTS)
					|| (tkHint->HinterFlags & M4HF_UseMulti)) {
					payloadName = "mpeg4-generic";
				} else {
					payloadName = "MP4V-ES";
				}
			} else {
				/*TODO: other visual streams are only carried as generic*/
				payloadName = "mpeg4-generic";
			}
			break;
		case RTP_PAYT_MPEG12:
			payloadName = "MPV";
			break;
		case RTP_PAYT_H263:
			payloadName = "H263-1998";
			break;
		}
		break;
	case M4_AudioMediaType:
		mediaName = "audio";
		switch (tkHint->hinter->rtp_payt) {
		case RTP_PAYT_MPEG4:
			payloadName = "mpeg4-generic";
			break;
		case RTP_PAYT_MPEG12:
			payloadName = "MPA";
			break;
		case RTP_PAYT_AMR:
			payloadName = "AMR";
			break;
		case RTP_PAYT_AMR_WB:
			payloadName = "AMR-WB";
			break;
		}
		break;
	case M4_TimedTextMediaType:
		mediaName = "text";
		payloadName = "3gpp-tt";
		break;
	default:
		if (tkHint->hinter->rtp_payt == RTP_PAYT_MPEG4) {
			mediaName = (TrackMediaType==M4_MPEGJMediaType) ? "application" : "video";
			payloadName = "mpeg4-generic";
		}
		break;
	}
	fprintf(stdout, "Finalizing hint track for payload \"%s\"\n", payloadName);

	/*for fine interleaving*/
	if (! tkHint->copy_media) {
		/*if we don't copy data set hint track and media track in the same group*/
		M4_SetTrackGroup(tkHint->file, tkHint->HintTrack, tkHint->Group);
	} else {
		M4_SetTrackGroup(tkHint->file, tkHint->HintTrack, tkHint->Group + OFFSET_HINT_GROUP_ID);
	}
	/*use user-secified priority*/
	M4_SetTrackPriorityInGroup(tkHint->file, tkHint->HintTrack, tkHint->Priority);

	/*TODO- extract out of hinter for future live tools*/
	sprintf(sdpLine, "m=%s 0 RTP/AVP %d", mediaName, tkHint->PayloadType);
	M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	sprintf(sdpLine, "a=rtpmap:%d %s/%d", tkHint->PayloadType, payloadName, tkHint->hinter->sl_config.timestampResolution);
	M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	/*control for MPEG-4*/
	if (AddSystemInfo) {
		sprintf(sdpLine, "a=mpeg4-esid:%d", M4_GetTrackID(tkHint->file, tkHint->TrackNum));
		M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	}
	/*control for QTSS/DSS*/
	sprintf(sdpLine, "a=control:trackID=%d", M4_GetTrackID(tkHint->file, tkHint->HintTrack));
	M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);

	/*H263 extensions*/
	if (tkHint->hinter->rtp_payt == RTP_PAYT_H263) {
		sprintf(sdpLine, "a=cliprect:0,0,%d,%d", Height, Width);
		M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	}
	/*AMR*/
	else if ((tkHint->hinter->rtp_payt == RTP_PAYT_AMR) || (tkHint->hinter->rtp_payt == RTP_PAYT_AMR_WB)) {
		M4RTP_FormatSDP(tkHint->hinter, payloadName, sdpLine, NULL, 0);
		M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	}
	/*Text*/
	else if (tkHint->hinter->rtp_payt == RTP_PAYT_3GPP_TEXT) {
		M4T_FormatTextSDP(tkHint->hinter, payloadName, sdpLine, tkHint->file, tkHint->TrackNum);
		M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	}
	/*MPEG-4 decoder config*/
	else if (tkHint->hinter->rtp_payt==RTP_PAYT_MPEG4) {
		dcd = M4_GetDecoderInformation(tkHint->file, tkHint->TrackNum, 1);

		if (dcd && dcd->decoderSpecificInfo && dcd->decoderSpecificInfo->data) {
			M4RTP_FormatSDP(tkHint->hinter, payloadName, sdpLine, dcd->decoderSpecificInfo->data, dcd->decoderSpecificInfo->dataLength);
		} else {
			M4RTP_FormatSDP(tkHint->hinter, payloadName, sdpLine, NULL, 0);
		}
		if (dcd) OD_DeleteDescriptor((Descriptor **)&dcd);
		M4H_SDP_TrackAddLine(tkHint->file, tkHint->HintTrack, sdpLine);
	}
	M4_SetTrackEnabled(tkHint->file, tkHint->HintTrack, 1);
	return M4OK;
}

M4Err MP4T_FinalizeHintMovie(M4File *file, u32 IOD_Profile, char *copyright, char *description)
{
	u32 i, bifsT, odT, descIndex, size, size64;
	InitialObjectDescriptor *iod;
	SLConfigDescriptor slc;
	ESDescriptor *esd;
	M4Sample *samp;
	Bool remove_ocr;
	char *buffer;
	char buf64[5000], sdpLine[2300];


	M4H_SDP_CleanMovie(file);

	//session info
	sprintf(buf64, "i=%s", description ? description : "IsoMedia File published with GPAC " M4_VERSION);
	M4H_SDP_MovieAddLine(file, buf64);
	//xtended attribute for copyright
	sprintf(buf64, "a=x-copyright: %s", copyright ? copyright : "(C) 2000-2004 GPAC Project");
	M4H_SDP_MovieAddLine(file, buf64);

	if (IOD_Profile == SDP_IOD_NONE) return M4OK;

	odT = bifsT = 0;
	for (i=0; i<M4_GetTrackCount(file); i++) {
		if (!M4_IsTrackInRootOD(file, i+1)) continue;
		switch (M4_GetMediaType(file,i+1)) {
		case M4_ODMediaType:
			odT = i+1;
			break;
		case M4_BIFSMediaType:
			bifsT = i+1;
			break;
		}
	}
	remove_ocr = 0;
	if (IOD_Profile == SDP_IOD_ISMA_STRICT) {
		IOD_Profile = SDP_IOD_ISMA;
		remove_ocr = 1;
	}

	/*if we want ISMA like iods, we need at least BIFS */
	if ( (IOD_Profile == SDP_IOD_ISMA) && !bifsT ) return M4BadParam;

	/*do NOT change PLs, we assume they are correct*/
	iod = (InitialObjectDescriptor *) M4_GetRootOD(file);
	if (!iod) return M4NotSupported;

	/*rewrite an IOD with good SL config - embbed data if possible*/
	if (IOD_Profile == SDP_IOD_ISMA) {
		while (ChainGetCount(iod->ESDescriptors)) {
			esd = ChainGetEntry(iod->ESDescriptors, 0);
			OD_DeleteDescriptor((Descriptor **) &esd);
			ChainDeleteEntry(iod->ESDescriptors, 0);
		}


		/*get OD esd, and embbed stream data if possible*/
		if (odT) {
			esd = M4_GetStreamDescriptor(file, odT, 1);
			if (M4_GetSampleCount(file, odT)==1) {
				samp = M4_GetSample(file, odT, 1, &descIndex);
				if (MP4T_CanEmbbedData(samp->data, samp->dataLength, M4ST_OD)) {
					InitSL_NULL(&slc);
					slc.predefined = 0;
					slc.useRandomAccessUnitsOnlyFlag = 1;
					slc.timeScale = slc.timestampResolution = M4_GetMediaTimeScale(file, odT);	
					slc.OCRResolution = 1000;
					slc.startCTS = samp->DTS+samp->CTS_Offset;
					slc.startDTS = samp->DTS;
					//set the SL for future extraction
					M4_SetExtractionSLConfig(file, odT, 1, &slc);

					size64 = Base64_enc(samp->data, samp->dataLength, buf64, 2000);
					buf64[size64] = 0;
					sprintf(sdpLine, "data:application/mpeg4-od-au;base64,%s", buf64);

					esd->decoderConfig->avgBitrate = 0;
					esd->decoderConfig->bufferSizeDB = samp->dataLength;
					esd->decoderConfig->maxBitrate = 0;
					size64 = strlen(sdpLine)+1;
					esd->URLString = malloc(sizeof(char) * size64);
					strcpy(esd->URLString, sdpLine);
				} else {
					fprintf(stdout, "Warning: OD sample too large to be embedded in IOD - ISAM disabled\n");
				}
				M4_DeleteSample(&samp);
			}
			if (remove_ocr) esd->OCRESID = 0;
			else if (esd->OCRESID == esd->ESID) esd->OCRESID = 0;
			
			//OK, add this to our IOD
			ChainAddEntry(iod->ESDescriptors, esd);
		}

		esd = M4_GetStreamDescriptor(file, bifsT, 1);
		if (M4_GetSampleCount(file, bifsT)==1) {
			samp = M4_GetSample(file, bifsT, 1, &descIndex);
			if (MP4T_CanEmbbedData(samp->data, samp->dataLength, M4ST_SCENE)) {

				slc.timeScale = slc.timestampResolution = M4_GetMediaTimeScale(file, bifsT);	
				slc.OCRResolution = 1000;
				slc.startCTS = samp->DTS+samp->CTS_Offset;
				slc.startDTS = samp->DTS;
				//set the SL for future extraction
				M4_SetExtractionSLConfig(file, bifsT, 1, &slc);
				//encode in Base64 the sample
				size64 = Base64_enc(samp->data, samp->dataLength, buf64, 2000);
				buf64[size64] = 0;
				sprintf(sdpLine, "data:application/mpeg4-bifs-au;base64,%s", buf64);

				esd->decoderConfig->avgBitrate = 0;
				esd->decoderConfig->bufferSizeDB = samp->dataLength;
				esd->decoderConfig->maxBitrate = 0;
				esd->URLString = malloc(sizeof(char) * (strlen(sdpLine)+1));
				strcpy(esd->URLString, sdpLine);
			} else {
				fprintf(stdout, "Warning: BIFS sample too large to be embedded in IOD - ISMA disabled\n");
			}
			M4_DeleteSample(&samp);
		}
		if (remove_ocr) esd->OCRESID = 0;
		else if (esd->OCRESID == esd->ESID) esd->OCRESID = 0;

		ChainAddEntry(iod->ESDescriptors, esd);
	}

	//encode the IOD
	buffer = NULL;
	size = 0;
	OD_EncDesc((Descriptor *) iod, &buffer, &size);
	OD_DeleteDescriptor((Descriptor **)&iod);

	//encode in Base64 the iod
	size64 = Base64_enc(buffer, size, buf64, 2000);
	buf64[size64] = 0;
	free(buffer);

	//format our SDP !
	sprintf(sdpLine, "a=mpeg4-iod:\"data:application/mpeg4-iod;base64,%s\"", buf64);
	//add the IOD to the SDP info.
	M4H_SDP_MovieAddLine(file, sdpLine);

	return M4OK;
}


#endif //M4_READ_ONLY

