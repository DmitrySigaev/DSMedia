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

#include <gpac/intern/m4_author_dev.h>

void InitSL_RTP(SLConfigDescriptor *slc);


u32 GetNumBitsForMaxValue(u32 MaxValue)
{
	u32 i = 0;
	//1<<i <=> 2^i for i<32 which is always our case
	while ( (u32) ( (1 << i) - 1)  < MaxValue) i++;
	return i;
}


void M4RTP_FormatSDP(M4RTPBuilder *builder, char *payload_name, char *sdpLine, char *dsi, u32 dsi_size)
{
	char buffer[20000], dsiString[20000];
	u32 i, k;
	sprintf(sdpLine, "a=fmtp:%d ", builder->PayloadType);

	if (builder->rtp_payt==RTP_PAYT_H263) return;
	if ((builder->rtp_payt==RTP_PAYT_AMR) || (builder->rtp_payt==RTP_PAYT_AMR_WB)) {
		strcat(sdpLine, "octet-align");
		return;
	}
	
	/*mandatory fields*/
	sprintf(buffer, "; profile-level-id=%d", builder->slMap.PL_ID);
	strcat(sdpLine, buffer);
	
	if (dsi && dsi_size) {
		k = 0;
		for (i=0; i<dsi_size; i++) {
			if ((unsigned char) dsi[i] < 0x10) {
				dsiString[k] = '0';
				k++;
				sprintf(&dsiString[k], "%x", (unsigned char) dsi[i]);
				k++;
			} else {
				sprintf(&dsiString[k], "%x", (unsigned char) dsi[i]);
				k+=2;
			}
		}
		dsiString[k] = 0;
		sprintf(buffer, "; config=%s", dsiString);
		strcat(sdpLine, buffer);
	}
	/*FIXME - LATM not tested ...*/
	if (!strcmp(payload_name, "MP4A-LATM")) {
		strcat(sdpLine, "; cpresent=0");
		return;
	}
	if (!strcmp(payload_name, "MP4V-ES")) return;

	sprintf(buffer, "; StreamType=%d", builder->slMap.StreamType);
	strcat(sdpLine, buffer);
	if (strcmp(builder->slMap.mode, "") && strcmp(builder->slMap.mode, "default")) {
		sprintf(buffer, "; mode=%s", builder->slMap.mode);
		strcat(sdpLine, buffer);
	} else {
		strcat(sdpLine, "; mode=generic");
	}

	/*optional fields*/
	if (builder->slMap.ObjectTypeIndication) {
		sprintf(buffer, "; objectType=%d", builder->slMap.ObjectTypeIndication);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.ConstantSize) {
		sprintf(buffer, "; ConstantSize=%d", builder->slMap.ConstantSize);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.ConstantDuration) {
		sprintf(buffer, "; ConstantDuration=%d", builder->slMap.ConstantDuration);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.maxDisplacement) {
		sprintf(buffer, "; maxDisplacement=%d", builder->slMap.maxDisplacement);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.deinterleaveBufferSize) {
		sprintf(buffer, "; de-interleaveBufferSize=%d", builder->slMap.deinterleaveBufferSize);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.SizeLength) {
		sprintf(buffer, "; SizeLength=%d", builder->slMap.SizeLength);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.IndexLength) {
		sprintf(buffer, "; IndexLength=%d", builder->slMap.IndexLength);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.IndexDeltaLength) {
		sprintf(buffer, "; IndexDeltaLength=%d", builder->slMap.IndexDeltaLength);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.CTSDeltaLength) {
		sprintf(buffer, "; CTSDeltaLength=%d", builder->slMap.CTSDeltaLength);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.DTSDeltaLength) {
		sprintf(buffer, "; DTSDeltaLength=%d", builder->slMap.DTSDeltaLength);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.RandomAccessIndication) {
		sprintf(buffer, "; RandomAccessIndication=%d", builder->slMap.RandomAccessIndication);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.StreamStateIndication) {
		sprintf(buffer, "; StreamStateIndication=%d", builder->slMap.StreamStateIndication);
		strcat(sdpLine, buffer);
	}
	if (builder->slMap.AuxiliaryDataSizeLength) {
		sprintf(buffer, "; AuxiliaryDataSizeLength=%d", builder->slMap.AuxiliaryDataSizeLength);
		strcat(sdpLine, buffer);
	}
}






M4RTPBuilder *NewRTPBuilder(u32 rtp_payt, SLConfigDescriptor *slc, u32 flags,
				void *cbk_obj, 
				void (*OnNewPacket)(void *cbk, RTPHeader *header),
				void (*OnPacketDone)(void *cbk, RTPHeader *header, 
						char *payl_hdr, u32 payl_hdr_size, 
						char *payload, u32 payload_size),
				void (*OnDataReference)(void *cbk, u32 payload_size, u32 offset_from_orig),
				void (*OnData)(void *cbk, char *data, u32 data_size)
				)
{
	M4RTPBuilder *tmp;
	if (!rtp_payt || !cbk_obj | !OnPacketDone) return NULL;
	
	tmp = malloc(sizeof(M4RTPBuilder));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(M4RTPBuilder));
	if (slc) {
		memcpy(&tmp->sl_config, slc, sizeof(SLConfigDescriptor));
	} else {
		//default
		InitSL_RTP(&tmp->sl_config);
	}
	tmp->OnNewPacket = OnNewPacket;
	tmp->OnDataReference = OnDataReference;
	tmp->OnData = OnData;
	tmp->cbk_obj = cbk_obj;
	tmp->OnPacketDone = OnPacketDone;
	tmp->rtp_payt = rtp_payt;
	tmp->flags = flags;
	//default init
	tmp->sl_header.AU_sequenceNumber = 1;
	tmp->sl_header.packetSequenceNumber = 1;

	//we assume we start on a new AU
	tmp->sl_header.accessUnitStartFlag = 1;	
	return tmp;
}

void M4RTP_DeleteBuilder(M4RTPBuilder *builder)
{
	if (!builder) return;

	if (builder->payload) DeleteBitStream(builder->payload);
	if (builder->auheader) DeleteBitStream(builder->auheader);
	free(builder);
}

M4Err M4RTP_ProcessData(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex)
{
	if (!data || !builder || !data_size) return M4BadParam;

	switch (builder->rtp_payt) {
	case RTP_PAYT_MPEG4:
		return M4RTP_ProcessMPEG4(builder, data, data_size, IsAUEnd, FullAUSize);
	case RTP_PAYT_MPEG12:
		return M4RTP_ProcessMPEG12(builder, data, data_size, IsAUEnd, FullAUSize);
	case RTP_PAYT_H263:
		return M4RTP_ProcessH263(builder, data, data_size, IsAUEnd, FullAUSize);
	case RTP_PAYT_AMR:
	case RTP_PAYT_AMR_WB:
		return M4RTP_ProcessAMR(builder, data, data_size, IsAUEnd, FullAUSize);
	case RTP_PAYT_3GPP_TEXT:
		return M4RTP_ProcessText(builder, data, data_size, IsAUEnd, FullAUSize, duration, descIndex);
	default:
		return M4BadParam;
	}
}

SLHeader *M4RTP_GetSLHeader(LPM4RTPBUILDER builder)
{
	if (!builder) return NULL;
	return &builder->sl_header;
}

//Compute the #params of the slMap
void M4RTP_InitBuilder(M4RTPBuilder *builder, u8 PayloadType, u32 PathMTU, 
					   u32 StreamType, u32 PL_ID,
					   u32 avgSize, u32 maxSize, 
					   u32 avgTS, u32 maxDTS,
					   Bool interleave, char *pref_mode) 
{
	u32 k, totDelta;

	memset(&builder->slMap, 0, sizeof(RTPSLMap));
	builder->Path_MTU = PathMTU;
	builder->PayloadType = PayloadType;
	builder->slMap.StreamType = StreamType;
	builder->slMap.PL_ID = PL_ID;
	if (pref_mode) strcpy(builder->slMap.mode, pref_mode);


	//some cst vars
	builder->rtp_header.Version = 2;
	builder->rtp_header.PayloadType = builder->PayloadType;

	/*our max config is with 1 packet only (SingleSL)*/
	builder->first_sl_in_rtp = 1;
	/*no AUX data*/
	builder->slMap.AuxiliaryDataSizeLength= 0;

	/*currently only MPEG-4 rtp payloads needs setup*/
	if (builder->rtp_payt != RTP_PAYT_MPEG4) return;


	/*mode setup*/
	if (!strnicmp(builder->slMap.mode, "AAC", 3)) {
		builder->flags = M4HF_UseMulti | M4HF_SignalSize | M4HF_SignalIDX;
		if (interleave) builder->slMap.ConstantDuration = avgTS;

		/*AAC LBR*/
		if (maxSize < 63) {
			builder->slMap.PL_ID = 14;
			strcpy(builder->slMap.mode, "AAC-lbr");
			builder->slMap.IndexLength = builder->slMap.IndexDeltaLength = 2;
			builder->slMap.SizeLength = 6;
		}
		/*AAC HBR*/
		else {
			builder->slMap.PL_ID = 16;
			strcpy(builder->slMap.mode, "AAC-hbr");
			builder->slMap.IndexLength = builder->slMap.IndexDeltaLength = 3;
			builder->slMap.SizeLength = 13;
		}
		goto check_header;
	}
	if (!strnicmp(builder->slMap.mode, "CELP", 4)) {
		builder->slMap.PL_ID = 14;
		/*CELP-cbr*/
		if (maxSize == avgSize) {
			/*reset flags (interleaving forbidden)*/
			builder->flags = M4HF_UseMulti;
			interleave = 0;
			strcpy(builder->slMap.mode, "CELP-cbr");
			builder->slMap.ConstantSize = avgSize;
			builder->slMap.ConstantDuration = avgTS;
		}
		/*CELP VBR*/
		else {
			strcpy(builder->slMap.mode, "CELP-vbr");
			builder->slMap.IndexLength = builder->slMap.IndexDeltaLength = 2;
			builder->slMap.SizeLength = 6;
			if (interleave) builder->slMap.ConstantDuration = avgTS;
			builder->flags = M4HF_UseMulti | M4HF_SignalSize | M4HF_SignalIDX;
		}
		goto check_header;
	}

	/*generic setup by flags*/
	
	/*size*/
	if (builder->flags & M4HF_SignalSize) {
		if (avgSize==maxSize) {
			builder->slMap.SizeLength = 0;
			builder->slMap.ConstantSize = maxSize;
		} else {
			builder->slMap.SizeLength = GetNumBitsForMaxValue(maxSize ? maxSize : PathMTU);
			builder->slMap.ConstantSize = 0;
		}
	} else {
		builder->slMap.SizeLength = 0;
		if (builder->flags & M4HF_UseMulti)
			builder->slMap.ConstantSize = (avgSize==maxSize) ? maxSize : 0;
		else
			builder->slMap.ConstantSize = 0;
	}

	/*single SL per RTP*/
	if (!(builder->flags & M4HF_UseMulti)) {
		if ( builder->sl_config.AUSeqNumLength && (builder->flags & M4HF_SignalIDX)) {
			builder->slMap.IndexLength = builder->sl_config.AUSeqNumLength;
		} else {
			builder->slMap.IndexLength = 0;
		}
		/*one packet per RTP so no delta*/
		builder->slMap.IndexDeltaLength = 0;

		/*CTS Delta is always 0 since we have one SL packet per RTP*/
		builder->slMap.CTSDeltaLength = 0;

		/*DTS Delta depends on the video type*/
		if ((builder->flags & M4HF_SignalTS) && maxDTS ) 
			builder->slMap.DTSDeltaLength = GetNumBitsForMaxValue(maxDTS);
		else
			builder->slMap.DTSDeltaLength = 0;

		/*RAP*/
		if (builder->sl_config.useRandomAccessPointFlag && (builder->flags & M4HF_SignalRAP)) {
			builder->slMap.RandomAccessIndication = 1;
		} else {
			builder->slMap.RandomAccessIndication = 0;
		}
		/*TODO: stream state*/
		goto check_header;
	}

	/*this is the avg samples we can store per RTP packet*/
	k = PathMTU / avgSize;
	if (k<=1) {
		builder->flags &= ~M4HF_UseMulti;
		builder->flags &= ~M4HF_SignalTS;
		builder->flags &= ~M4HF_SignalSize;
		builder->flags &= ~M4HF_SignalIDX;
		M4RTP_InitBuilder(builder, PayloadType, PathMTU, StreamType, PL_ID, avgSize, maxSize, avgTS, maxDTS, 0, pref_mode);
		return;
	}

	/*multiple SL per RTP - check if we have to send TS*/
	builder->slMap.ConstantDuration = builder->sl_config.CUDuration;
	if (!builder->slMap.ConstantDuration) {
		builder->flags |= M4HF_SignalTS;
	}
	/*if we have a constant duration and are not writting TSs, make sure we write AU IDX when interleaving*/
	else if (! (builder->flags & M4HF_SignalTS) && interleave) {
		builder->flags |= M4HF_SignalIDX;
	}

	if (builder->flags & M4HF_SignalTS) {
		/*compute CTS delta*/
		totDelta = k*avgTS;
		builder->slMap.CTSDeltaLength = GetNumBitsForMaxValue(k*avgTS);

		/*compute DTS delta. Delta is ALWAYS from the CTS of the same sample*/ 
		if (maxDTS) 
			builder->slMap.DTSDeltaLength = GetNumBitsForMaxValue(maxDTS);
		else
			builder->slMap.DTSDeltaLength = 0;
	}

	if ((builder->flags & M4HF_SignalIDX) && builder->sl_config.AUSeqNumLength) {
		builder->slMap.IndexLength = builder->sl_config.AUSeqNumLength;
		/*and k-1 AUs in Delta*/
		builder->slMap.IndexDeltaLength = interleave ? GetNumBitsForMaxValue(k-1) : 0;
	}

	/*RAP*/
	if (builder->sl_config.useRandomAccessPointFlag && (builder->flags & M4HF_SignalRAP)) {
		builder->slMap.RandomAccessIndication = 1;
	} else {
		builder->slMap.RandomAccessIndication = 0;
	}

check_header:
	/*check if we use AU header or not*/
	if (!builder->slMap.SizeLength 
		&& !builder->slMap.IndexLength 
		&& !builder->slMap.IndexDeltaLength
		&& !builder->slMap.DTSDeltaLength  
		&& !builder->slMap.CTSDeltaLength
		&& !builder->slMap.RandomAccessIndication
		&& !builder->slMap.StreamStateIndication) {
		builder->has_AU_header= 0;
	} else {
		builder->has_AU_header = 1;
	}
}
