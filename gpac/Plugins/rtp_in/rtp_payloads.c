/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / RTP input plugin
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

#include "rtp_in.h"
#include <gpac/m4_decoder.h>


u32 payt_get_type(RTPClient *rtp, SDP_RTPMap *map, SDPMedia *media)
{
	u32 i, j;

	if (!stricmp(map->payload_name, "MP4V-ES") ) return RTP_PAYT_MPEG4;
	if (!stricmp(map->payload_name, "mpeg4-generic")) return RTP_PAYT_MPEG4;

	/*LATM: only without multiplexing (not tested but should be straight AUs)*/
	if (!stricmp(map->payload_name, "MP4A-LATM")) {
		for (i=0; i<ChainGetCount(media->FMTP); i++) {
			SDP_FMTP *fmtp = ChainGetEntry(media->FMTP, i);
			if (fmtp->PayloadType != map->PayloadType) continue;
			//this is our payload. check cpresent is 0
			for (j=0; j<ChainGetCount(fmtp->Attributes); j++) {
				X_Attribute *att = ChainGetEntry(fmtp->Attributes, j);
				if (!stricmp(att->Name, "cpresent") && atoi(att->Value)) return 0;
			}
		}
		return RTP_PAYT_MPEG4;
	}
	if (!stricmp(map->payload_name, "MPA") || !stricmp(map->payload_name, "MPV")) return RTP_PAYT_MPEG12;
	if (!stricmp(map->payload_name, "H263-1998") || !stricmp(map->payload_name, "H263-2000")) return RTP_PAYT_H263;
	if (!stricmp(map->payload_name, "AMR")) return RTP_PAYT_AMR;
	if (!stricmp(map->payload_name, "AMR-WB")) return RTP_PAYT_AMR_WB;
	if (!stricmp(map->payload_name, "3gpp-tt")) return RTP_PAYT_3GPP_TEXT;
	return 0;
}


static M4Err payt_set_param(RTPStream *ch, char *param_name, char *param_val)
{
	u32 i, val;
	char valS[3];
	BitStream *bs;

	if (!ch || !param_name) return M4BadParam;

	/*1 - mpeg4-generic / RFC 3016 payload type items*/

	/*PL (not needed when IOD is here)*/
	if (!stricmp(param_name, "Profile-level-id")) ch->sl_map.PL_ID = atoi(param_val);

	/*decoder specific info (not needed when IOD is here)*/
	else if (!stricmp(param_name, "config")) {
		//decode the buffer - the string buffer is MSB hexadecimal
		bs = NewBitStream(NULL, 0, BS_WRITE);
		valS[2] = 0;
		for (i=0; i<strlen(param_val);i+=2) {
			valS[0] = param_val[i];
			valS[1] = param_val[i+1];
			sscanf(valS, "%x", &val);
			BS_WriteInt(bs, val, 8);
		}
		if (ch->sl_map.config) free(ch->sl_map.config);
		ch->sl_map.config = NULL;
		BS_GetContent(bs, (unsigned char **) &ch->sl_map.config, &ch->sl_map.configSize);
		DeleteBitStream(bs);
	}
	/*mpeg4-generic payload type items required*/
	
	/*constant size (size of all AUs) */
	else if (!stricmp(param_name, "ConstantSize")) {
		ch->sl_map.ConstantSize = atoi(param_val);
	}
	/*constant size (size of all AUs) */
	else if (!stricmp(param_name, "ConstantDuration")) {
		ch->sl_map.ConstantDuration = atoi(param_val);
	}
	/*object type indication (not needed when IOD is here)*/
	else if (!stricmp(param_name, "ObjectType")) {
		ch->sl_map.ObjectTypeIndication = atoi(param_val);
	}
	else if (!stricmp(param_name, "StreamType")) ch->sl_map.StreamType = atoi(param_val);
	else if (!stricmp(param_name, "mode")) {
		strcpy(ch->sl_map.mode, param_val);
		/*in case no IOD and no streamType/OTI in the file*/
		if (!stricmp(param_val, "AAC-hbr") || !stricmp(param_val, "AAC-lbr") || !stricmp(param_val, "CELP-vbr") || !stricmp(param_val, "CELP-cbr")) {
			ch->sl_map.StreamType = M4ST_AUDIO;
			ch->sl_map.ObjectTypeIndication = 0x40;
		}

	}

	else if (!stricmp(param_name, "DTSDeltaLength")) ch->sl_map.DTSDeltaLength = atoi(param_val);
	else if (!stricmp(param_name, "CTSDeltaLength")) ch->sl_map.CTSDeltaLength = atoi(param_val);
	else if (!stricmp(param_name, "SizeLength")) ch->sl_map.SizeLength = atoi(param_val);
	else if (!stricmp(param_name, "IndexLength")) ch->sl_map.IndexLength = atoi(param_val);
	else if (!stricmp(param_name, "IndexDeltaLength")) ch->sl_map.IndexDeltaLength = atoi(param_val);
	else if (!stricmp(param_name, "RandomAccessIndication")) ch->sl_map.RandomAccessIndication = atoi(param_val);
	else if (!stricmp(param_name, "StreamStateIndication")) ch->sl_map.StreamStateIndication = atoi(param_val);
	else if (!stricmp(param_name, "AuxiliaryDataSizeLength")) ch->sl_map.AuxiliaryDataSizeLength = atoi(param_val);

	/*AMR config*/
	else if (!stricmp(param_name, "octet-align")) ch->octet_align = 1;

	return M4OK;
}

void payt_setup(RTPStream *ch, SDP_RTPMap *map, SDPMedia *media)
{
	u32 i, j;

	/*reset sl map*/
	memset(&ch->sl_map, 0, sizeof(RTPSLMap));
	
	/*setup channel*/
	RTP_SetupPayload(ch->rtp_ch, map);

	/*then process all FMTPs*/
	for (i=0; i<ChainGetCount(media->FMTP); i++) {
		SDP_FMTP *fmtp = ChainGetEntry(media->FMTP, i);
		//we work with only one PayloadType for now
		if (fmtp->PayloadType != map->PayloadType) continue;
		for (j=0; j<ChainGetCount(fmtp->Attributes); j++) {
			X_Attribute *att = ChainGetEntry(fmtp->Attributes, j);
			payt_set_param(ch, att->Name, att->Value);
		}
	}

	switch (ch->rtptype) {
	case RTP_PAYT_MPEG4:
		/*mark if AU header is present*/
		ch->sl_map.auh_first_min_len = ch->sl_map.CTSDeltaLength;
		ch->sl_map.auh_first_min_len += ch->sl_map.DTSDeltaLength;
		ch->sl_map.auh_first_min_len += ch->sl_map.SizeLength;
		ch->sl_map.auh_first_min_len += ch->sl_map.RandomAccessIndication;
		ch->sl_map.auh_first_min_len += ch->sl_map.StreamStateIndication;
		ch->sl_map.auh_min_len = ch->sl_map.auh_first_min_len;
		ch->sl_map.auh_first_min_len += ch->sl_map.IndexLength;
		ch->sl_map.auh_min_len += ch->sl_map.IndexDeltaLength;
		if (!stricmp(map->payload_name, "MP4V-ES")) {
			ch->sl_map.StreamType = M4ST_VISUAL;
			ch->sl_map.ObjectTypeIndication = 0x20;
		}
		else if (!stricmp(map->payload_name, "MP4A-LATM")) {
			ch->sl_map.StreamType = M4ST_AUDIO;
			ch->sl_map.ObjectTypeIndication = 0x40;
		}
		break;
	case RTP_PAYT_MPEG12:
		if (!stricmp(map->payload_name, "MPA")) {
			ch->sl_map.StreamType = M4ST_AUDIO;
			ch->sl_map.ObjectTypeIndication = 0x69;
		}
		else if (!stricmp(map->payload_name, "MPV")) {
			ch->sl_map.StreamType = M4ST_VISUAL;
			/*FIXME: how to differentiate MPEG1 from MPEG2 video before any frame is received??*/
			ch->sl_map.ObjectTypeIndication = 0x6A;
		}
		break;
	case RTP_PAYT_AMR:
	case RTP_PAYT_AMR_WB:
		{
			BitStream *bs;
			ch->sl_map.StreamType = M4ST_AUDIO;
			ch->sl_map.ObjectTypeIndication = GPAC_QT_CODECS_OTI;
			/*create DSI*/
			bs = NewBitStream(NULL, 0, BS_WRITE);
			if (ch->rtptype == RTP_PAYT_AMR) {
				BS_WriteInt(bs, FOUR_CHAR_INT('s', 'a', 'm', 'r'), 32);
			} else {
				BS_WriteInt(bs, FOUR_CHAR_INT('s', 'a', 'w', 'b'), 32);
			}
			BS_WriteInt(bs, 0, 4*32);
			BS_GetContent(bs, (unsigned char **) &ch->sl_map.config, &ch->sl_map.configSize);
			DeleteBitStream(bs);
		}
		break;
	case RTP_PAYT_H263:
		{
			u32 x, y, w, h;
			BitStream *bs;
			x = y = w = h = 0;
			for (j=0; j<ChainGetCount(media->Attributes); j++) {
				X_Attribute *att = ChainGetEntry(media->Attributes, j);
				if (stricmp(att->Name, "cliprect")) continue;
				/*only get the display area*/
				sscanf(att->Value, "%d,%d,%d,%d", &y, &x, &h, &w);
			}

			ch->sl_map.StreamType = M4ST_VISUAL;
			ch->sl_map.ObjectTypeIndication = GPAC_QT_CODECS_OTI;
			/*create DSI*/
			bs = NewBitStream(NULL, 0, BS_WRITE);
			BS_WriteInt(bs, FOUR_CHAR_INT('h', '2', '6', '3'), 32);
			BS_WriteInt(bs, w, 32);
			BS_WriteInt(bs, h, 32);
			BS_GetContent(bs, (unsigned char **) &ch->sl_map.config, &ch->sl_map.configSize);
			DeleteBitStream(bs);
		}
		break;
	case RTP_PAYT_3GPP_TEXT:
	{
		char *tx3g, *a_tx3g;
		BitStream *bs;
		u32 nb_desc;
		TextConfigDescriptor tcfg;
		memset(&tcfg, 0, sizeof(TextConfigDescriptor));
		tcfg.tag = TextConfig_Tag;
		tcfg.Base3GPPFormat = 0x10;
		tcfg.MPEGExtendedFormat = 0x10;
		tcfg.profileLevel = 0x10;
		tcfg.timescale = ch->clock_rate;
		tcfg.sampleDescriptionFlags = 1;
		tx3g = NULL;

		for (i=0; i<ChainGetCount(media->FMTP); i++) {
			SDP_FMTP *fmtp = ChainGetEntry(media->FMTP, i);
			if (fmtp->PayloadType != map->PayloadType) continue;
			for (j=0; j<ChainGetCount(fmtp->Attributes); j++) {
				X_Attribute *att = ChainGetEntry(fmtp->Attributes, j);

				if (!stricmp(att->Name, "width")) tcfg.text_width = atoi(att->Value);
				else if (!stricmp(att->Name, "height")) tcfg.text_height = atoi(att->Value);
				else if (!stricmp(att->Name, "tx")) tcfg.horiz_offset = atoi(att->Value);
				else if (!stricmp(att->Name, "ty")) tcfg.vert_offset = atoi(att->Value);
				else if (!stricmp(att->Name, "layer")) tcfg.layer = atoi(att->Value);
				else if (!stricmp(att->Name, "max-w")) tcfg.video_width = atoi(att->Value);
				else if (!stricmp(att->Name, "max-h")) tcfg.video_height = atoi(att->Value);
				else if (!stricmp(att->Name, "tx3g")) tx3g = att->Value;
			}
		}
		if (!tx3g) return;

		bs = NewBitStream(NULL, 0, BS_WRITE);
		BS_WriteInt(bs, tcfg.Base3GPPFormat, 8);
		BS_WriteInt(bs, tcfg.MPEGExtendedFormat, 8); /*MPEGExtendedFormat*/
		BS_WriteInt(bs, tcfg.profileLevel, 8); /*profileLevel*/
		BS_WriteInt(bs, tcfg.timescale, 24);
		BS_WriteInt(bs, 0, 1);	/*no alt formats*/
		BS_WriteInt(bs, tcfg.sampleDescriptionFlags, 2);
		BS_WriteInt(bs, 1, 1);	/*we will write sample desc*/
		BS_WriteInt(bs, 1, 1);	/*video info*/
		BS_WriteInt(bs, 0, 3);	/*reserved, spec doesn't say the values*/
		BS_WriteInt(bs, tcfg.layer, 8);
		BS_WriteInt(bs, tcfg.text_width, 16);
		BS_WriteInt(bs, tcfg.text_height, 16);
		/*get all tx3g (comma separated)*/
		nb_desc = 1;
		a_tx3g = tx3g;
		while ((a_tx3g = strchr(a_tx3g, ',')) ) {
			a_tx3g ++;
			nb_desc ++;
		}
		a_tx3g = tx3g;
		BS_WriteInt(bs, nb_desc, 8);
		while (1) {
			char *next_tx3g, szOut[1000];
			u32 len;
			strcpy(a_tx3g, tx3g);
			next_tx3g = strchr(a_tx3g, ',');
			if (next_tx3g) next_tx3g[0] = 0;
			len = Base64_dec(a_tx3g, strlen(a_tx3g), szOut, 1000);
			BS_WriteData(bs, szOut, len);
			tx3g = strchr(tx3g, ',');
			if (!tx3g) break;
			tx3g += 1;
			while (tx3g[0] == ' ') tx3g += 1;
		}

		/*write video cfg*/
		BS_WriteInt(bs, tcfg.video_width, 16);
		BS_WriteInt(bs, tcfg.video_height, 16);
		BS_WriteInt(bs, tcfg.horiz_offset, 16);
		BS_WriteInt(bs, tcfg.vert_offset, 16);
		BS_GetContent(bs, (unsigned char **)&ch->sl_map.config, &ch->sl_map.configSize);
		ch->sl_map.StreamType = M4ST_TEXT;
		ch->sl_map.ObjectTypeIndication = 0x08;
		DeleteBitStream(bs);
	}
		break;

	}

}


void RP_ParsePayloadMPEG4(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	u32 aux_size, au_size, first_idx, au_hdr_size, pay_start, num_au;
	s32 au_idx;
	BitStream *hdr_bs, *aux_bs;

	hdr_bs = NewBitStream(payload, size, BS_READ);
	aux_bs = NewBitStream(payload, size, BS_READ);

//	printf("parsing packet %d size %d ts %d M %d\n", hdr->SequenceNumber, size, hdr->TimeStamp, hdr->Marker);

	/*global AU header len*/
	au_hdr_size = 0;
	if (ch->sl_map.auh_first_min_len) {
		au_hdr_size = BS_ReadInt(hdr_bs, 16);
		BS_ReadInt(aux_bs, 16);
	}

	/*jump to aux section, skip it and get payload start*/
	BS_ReadInt(aux_bs, au_hdr_size);
	BS_Align(aux_bs);
	if (ch->sl_map.AuxiliaryDataSizeLength) {
		aux_size = BS_ReadInt(aux_bs, ch->sl_map.AuxiliaryDataSizeLength);
		BS_ReadInt(aux_bs, aux_size);
		BS_Align(aux_bs);
	}
	pay_start = (u32) BS_GetPosition(aux_bs);
	DeleteBitStream(aux_bs);

	first_idx = 0;
	au_idx = 0;

	ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
	ch->sl_hdr.decodingTimeStamp = hdr->TimeStamp;

	num_au = 0;

	ch->sl_hdr.accessUnitEndFlag = hdr->Marker;
	/*override some defaults for RFC 3016*/
	if (ch->new_au) {
		ch->sl_hdr.accessUnitStartFlag = 1;
	} else {
		ch->sl_hdr.accessUnitStartFlag = 0;
	}

	while (1) {
		/*get default AU size*/
		au_size = ch->sl_map.ConstantSize;
		/*not signaled, assume max one AU per packet*/
		if (!au_size) au_size = size - pay_start;
		
		if ((!num_au && ch->sl_map.auh_first_min_len) || (num_au && ch->sl_map.auh_min_len)) {
			/*AU size*/
			if (ch->sl_map.SizeLength) {
				au_size = BS_ReadInt(hdr_bs, ch->sl_map.SizeLength);
				if (au_size > size - pay_start) au_size = size - pay_start;
				au_hdr_size -= ch->sl_map.SizeLength;
			}
			/*AU index*/
			if (! num_au) {
				au_idx = first_idx = BS_ReadInt(hdr_bs, ch->sl_map.IndexLength);
				au_hdr_size -= ch->sl_map.IndexLength;
			} else {
				au_idx += 1 + (s32) BS_ReadInt(hdr_bs, ch->sl_map.IndexDeltaLength);
				au_hdr_size -= ch->sl_map.IndexDeltaLength;
			}
			/*CTS flag*/
			if (ch->sl_map.CTSDeltaLength) {
				ch->sl_hdr.compositionTimeStampFlag = BS_ReadInt(hdr_bs, 1);
				au_hdr_size -= 1;
			} else {
				/*get CTS from IDX*/
				if (ch->sl_map.ConstantDuration) {
					ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp + (au_idx - first_idx) * ch->sl_map.ConstantDuration;
				} else {
					ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp + (au_idx - first_idx) * ch->unit_duration;
				}
			}

			/*CTS in-band*/
			if (ch->sl_hdr.compositionTimeStampFlag) {
				ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp + (s32) BS_ReadInt(hdr_bs, ch->sl_map.CTSDeltaLength);
				au_hdr_size -= ch->sl_map.CTSDeltaLength;
			}
			/*DTS flag is always present (needed for reconstruction of TSs in case of packet loss)*/
			if (ch->sl_map.DTSDeltaLength) {
				ch->sl_hdr.decodingTimeStampFlag = BS_ReadInt(hdr_bs, 1);
				au_hdr_size -= 1;
			} else {
				/*NO DTS otherwise*/
				ch->sl_hdr.decodingTimeStampFlag = 0;
			}
			if (ch->sl_hdr.decodingTimeStampFlag) {
				s32 ts = hdr->TimeStamp - (s32) BS_ReadInt(hdr_bs, ch->sl_map.DTSDeltaLength);
				ch->sl_hdr.decodingTimeStamp = (ts>0) ? ts : 0;
				au_hdr_size -= ch->sl_map.DTSDeltaLength;
			}
			/*RAP flag*/
			if (ch->sl_map.RandomAccessIndication) {
				ch->sl_hdr.randomAccessPointFlag = BS_ReadInt(hdr_bs, 1);
				au_hdr_size -= 1;
			}
			/*stream state - map directly to seqNum*/
			if (ch->sl_map.StreamStateIndication) {
				ch->sl_hdr.AU_sequenceNumber = BS_ReadInt(hdr_bs, ch->sl_map.StreamStateIndication);
				au_hdr_size -= ch->sl_map.StreamStateIndication;
			}
		}
		/*no header present, update CTS/DTS - note we're sure there's no interleaving*/
		else {
			if (num_au) {
				ch->sl_hdr.compositionTimeStamp += ch->sl_map.ConstantDuration;
				ch->sl_hdr.decodingTimeStamp += ch->sl_map.ConstantDuration;
			}
		}
		/*we cannot map RTP SN to SL SN since an RTP packet may carry several SL ones - only inc by 1 seq nums*/
		ch->sl_hdr.packetSequenceNumber += 1;

		/*force indication of CTS whenever we have a new AU*/
		
		ch->sl_hdr.compositionTimeStampFlag = ch->new_au;

		if (ch->owner->first_packet_drop && (ch->sl_hdr.packetSequenceNumber >= ch->owner->first_packet_drop) ) {
			if ( (ch->sl_hdr.packetSequenceNumber - ch->owner->first_packet_drop) % ch->owner->frequency_drop)
				NM_OnSLPRecieved(ch->owner->service, ch->channel, payload + pay_start, au_size, &ch->sl_hdr, M4OK);
		} else {
			NM_OnSLPRecieved(ch->owner->service, ch->channel, payload + pay_start, au_size, &ch->sl_hdr, M4OK);
		}

		ch->sl_hdr.compositionTimeStampFlag = 0;

		if (au_hdr_size < ch->sl_map.auh_min_len) break;
		pay_start += au_size;
		if (pay_start >= size) break;
		num_au ++;
	}

	ch->new_au = hdr->Marker ? 1 : 0;

	DeleteBitStream(hdr_bs);
}


void RP_ParsePayloadMPEG12Audio(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	u16 offset;
	u32 mp3hdr, ts;
	BitStream *bs;

	ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
	ch->sl_hdr.decodingTimeStamp = hdr->TimeStamp;

	ch->sl_hdr.accessUnitStartFlag = ch->sl_hdr.accessUnitEndFlag ? 1 : 0;
	if (ch->new_au) ch->sl_hdr.accessUnitStartFlag = 1;

	/*get frag header*/
	bs = NewBitStream(payload, size, BS_READ);
	BS_ReadInt(bs, 16);
	offset = BS_ReadInt(bs, 16);
	DeleteBitStream(bs);
	payload += 4;
	size -= 4;
	mp3hdr = 0;
	while (1) {

		/*frame start if no offset*/
		ch->sl_hdr.accessUnitStartFlag = offset ? 0 : 1;

		/*new frame, store size*/
		ch->sl_hdr.compositionTimeStampFlag = 0;
		if (ch->sl_hdr.accessUnitStartFlag) {
			mp3hdr = FOUR_CHAR_INT((u8) payload[0], (u8) payload[1], (u8) payload[2], (u8) payload[3]);
			ch->sl_hdr.accessUnitLength = MP3_GetFrameSize(mp3hdr);
			ch->sl_hdr.compositionTimeStampFlag = 1;
		}
		if (!ch->sl_hdr.accessUnitLength) break;
		/*fragmented frame*/
		if (ch->sl_hdr.accessUnitLength>size) {
			NM_OnSLPRecieved(ch->owner->service, ch->channel, payload, ch->sl_hdr.accessUnitLength, &ch->sl_hdr, M4OK);
			ch->sl_hdr.accessUnitLength -= size;
			ch->sl_hdr.accessUnitStartFlag = ch->sl_hdr.accessUnitEndFlag = 0;
			return;
		}
		/*complete frame*/
		ch->sl_hdr.accessUnitEndFlag = 1;
		NM_OnSLPRecieved(ch->owner->service, ch->channel, payload, ch->sl_hdr.accessUnitLength, &ch->sl_hdr, M4OK);
		payload += ch->sl_hdr.accessUnitLength;
		size -= ch->sl_hdr.accessUnitLength;
		ch->sl_hdr.accessUnitLength = 0;
		
		/*if fragmented there shall not be other frames in the packet*/
		if (!ch->sl_hdr.accessUnitStartFlag) return;
		if (!size) break;
		offset = 0;
		/*get ts*/
		ts = MP3_GetSamplesPerFrame(mp3hdr);
		ch->sl_hdr.compositionTimeStamp += ts;
		ch->sl_hdr.decodingTimeStamp += ts;
	}
	ch->new_au = 1;
}

void RP_ParsePayloadMPEG12Video(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	u8 pic_type;

	ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
	ch->sl_hdr.decodingTimeStamp = hdr->TimeStamp;


	pic_type = payload[2] & 0x7;
	payload += 4;
	size -= 4;

	/*missed something*/
	if (ch->sl_hdr.compositionTimeStamp != hdr->TimeStamp) ch->new_au = 1;

	ch->sl_hdr.accessUnitStartFlag = ch->new_au ? 1 : 0;
	ch->sl_hdr.accessUnitEndFlag = hdr->Marker ? 1 : 0;
	ch->sl_hdr.randomAccessPointFlag = (pic_type==1) ? 1 : 0;

	if (ch->sl_hdr.accessUnitStartFlag) {
		ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
		ch->sl_hdr.compositionTimeStampFlag = 1;
	} else {
		ch->sl_hdr.compositionTimeStampFlag = 0;
	}
	NM_OnSLPRecieved(ch->owner->service, ch->channel, payload, size, &ch->sl_hdr, M4OK);
	ch->new_au = hdr->Marker ? 1 : 0;
}


void RP_ParsePayloadMPEG12(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	switch (ch->sl_map.StreamType) {
	case M4ST_VISUAL:
		RP_ParsePayloadMPEG12Video(ch, hdr, payload, size);
		break;
	case M4ST_AUDIO:
		RP_ParsePayloadMPEG12Audio(ch, hdr, payload, size);
		break;
	}
}

static const u32 amr_nb_frame_size_bytes[16] = 
{
	12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0
};
static const u32 amr_wb_frame_size_bytes[16] = 
{
	17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 
};

void RP_ParsePayloadAMR(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	unsigned char c, type;
	char *data;
	/*we support max 30 frames in one RTP packet...*/
	u32 nbFrame, i, frame_size;
	/*not supported yet*/
	if (!ch->octet_align) return;

	/*process toc and locate start of payload data*/
	nbFrame = 0;
	while (1) {
		c = payload[nbFrame + 1];
		nbFrame++;
		if (!(c & 0x80)) break;
	}
	data = payload + nbFrame + 1;
	ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
	/*then each frame*/
	for (i=0; i<nbFrame; i++) {
		c = payload[i + 1];
		type = ((c & 0x78) >> 3);
		if (ch->rtptype==RTP_PAYT_AMR) {
			frame_size = amr_nb_frame_size_bytes[type];
		} else {
			frame_size = amr_wb_frame_size_bytes[type];
		}

		ch->sl_hdr.compositionTimeStampFlag = 1;
		ch->sl_hdr.accessUnitStartFlag = 1;
		ch->sl_hdr.accessUnitEndFlag = 0;
		/*send TOC*/
		NM_OnSLPRecieved(ch->owner->service, ch->channel, &payload[i+1], 1, &ch->sl_hdr, M4OK);
		ch->sl_hdr.packetSequenceNumber ++;
		ch->sl_hdr.compositionTimeStampFlag = 0;
		ch->sl_hdr.accessUnitStartFlag = 0;
		ch->sl_hdr.accessUnitEndFlag = 1;
		/*send payload*/
		NM_OnSLPRecieved(ch->owner->service, ch->channel, data, frame_size, &ch->sl_hdr, M4OK);
		data += frame_size;
		ch->sl_hdr.compositionTimeStamp += 160;
	}
}


void RP_ParsePayloadH263(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	BitStream *bs;
	Bool P_bit, V_bit;
	u32 plen, plen_bits, offset;
	char blank[2];

	bs = NewBitStream(payload, size, BS_READ);
	/*reserved*/
	BS_ReadInt(bs, 5);
	P_bit = BS_ReadInt(bs, 1);
	V_bit = BS_ReadInt(bs, 1);
	plen = BS_ReadInt(bs, 6);
	plen_bits = BS_ReadInt(bs, 3);

	/*VRC not supported yet*/
	if (V_bit) {
		BS_ReadInt(bs, 8);
	}
	/*extra picture header not supported yet*/
	if (plen) {
		BS_SkipBytes(bs, plen);
	}
	offset = (u32) BS_GetPosition(bs);
	DeleteBitStream(bs);

	blank[0] = blank[1] = 0;
	/*start*/
	if (P_bit) {
		ch->sl_hdr.compositionTimeStamp = hdr->TimeStamp;
		ch->sl_hdr.compositionTimeStampFlag = 1;
		ch->sl_hdr.accessUnitStartFlag = 1;
		ch->sl_hdr.accessUnitEndFlag = 0;
		/*send missing start code*/
		NM_OnSLPRecieved(ch->owner->service, ch->channel, (char *) blank, 2, &ch->sl_hdr, M4OK);
		/*send payload*/
		ch->sl_hdr.compositionTimeStampFlag = 0;
		ch->sl_hdr.accessUnitStartFlag = 0;
		/*if M bit set, end of frame*/
		ch->sl_hdr.accessUnitEndFlag = hdr->Marker;
		NM_OnSLPRecieved(ch->owner->service, ch->channel, payload + offset, size - offset, &ch->sl_hdr, M4OK);
		return;
	}

	/*middle/end of frames - if M bit set, end of frame*/
	ch->sl_hdr.accessUnitEndFlag = hdr->Marker;
	NM_OnSLPRecieved(ch->owner->service, ch->channel, payload + offset, size - offset, &ch->sl_hdr, M4OK);
}

void rtp_ttxt_flush(RTPStream *ch, u32 ts)
{
	BitStream *bs;
	char *data;
	u32 data_size;
	if (!ch->ttu1) return;

	ch->sl_hdr.compositionTimeStamp = ts;
	ch->sl_hdr.compositionTimeStampFlag = 1;
	ch->sl_hdr.accessUnitStartFlag = 1;
	ch->sl_hdr.accessUnitEndFlag = 0;
	ch->sl_hdr.randomAccessPointFlag = 1;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, ch->sl_hdr.idleFlag, 1);
	ch->sl_hdr.idleFlag = 0;
	BS_WriteInt(bs, 0, 4);
	BS_WriteInt(bs, 1, 3);
	BS_WriteInt(bs, 8 + (u16) BS_GetPosition(ch->ttu1), 16);
	BS_WriteInt(bs, ch->sidx, 8);
	BS_WriteInt(bs, ch->sl_hdr.au_duration, 24);
	BS_WriteInt(bs, ch->txt_len, 16);
	BS_GetContent(bs, (unsigned char **)&data, &data_size);
	DeleteBitStream(bs);

	NM_OnSLPRecieved(ch->owner->service, ch->channel, data, data_size, &ch->sl_hdr, M4OK);
	free(data);
	ch->sl_hdr.accessUnitStartFlag = 0;
	ch->sl_hdr.accessUnitEndFlag = 1;
	BS_GetContent(ch->ttu1, (unsigned char **)&data, &data_size);
	NM_OnSLPRecieved(ch->owner->service, ch->channel, data, data_size, &ch->sl_hdr, M4OK);
	free(data);

	DeleteBitStream(ch->ttu1);
	ch->ttu1 = NULL;
	ch->nb_txt_frag = ch->cur_txt_frag = ch->sidx = ch->txt_len = ch->nb_mod_frag = 0;
}

void RP_ParsePayloadText(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size)
{
	Bool is_utf_16;
	u32 type, ttu_len, pay_start, duration, ts, sidx, txt_size;
	u32 nb_frag, cur_frag;
	BitStream *bs;

	ts = hdr->TimeStamp;

	bs = NewBitStream(payload, size, BS_READ);
	while (BS_Available(bs)) {
		pay_start = (u32) BS_GetPosition(bs);
		is_utf_16 = BS_ReadInt(bs, 1);
		BS_ReadInt(bs, 4);
		type = BS_ReadInt(bs, 3);
		ttu_len = BS_ReadInt(bs, 16);
		if (ttu_len<2) break;

		if (type==1) {
			/*flush any existing packet*/
			rtp_ttxt_flush(ch, (u32) ch->sl_hdr.compositionTimeStamp);
			
			/*bad ttu(1)*/
			if (ttu_len<8) break;
			ch->sl_hdr.compositionTimeStamp = ts;
			ch->sl_hdr.compositionTimeStampFlag = 1;
			ch->sl_hdr.accessUnitStartFlag = 1;
			ch->sl_hdr.accessUnitEndFlag = 1;
			ch->sl_hdr.randomAccessPointFlag = 1;
			BS_ReadInt(bs, 8);
			ch->sl_hdr.au_duration = BS_ReadInt(bs, 24);
			NM_OnSLPRecieved(ch->owner->service, ch->channel, payload + pay_start, ttu_len + 1, &ch->sl_hdr, M4OK);
			BS_SkipBytes(bs, ttu_len - 6);
			ts += ch->sl_hdr.au_duration;
		}
		/*text segment*/
		else if (type==2) {
			/*TS changed, flush packet*/
			if (ch->sl_hdr.compositionTimeStamp < ts) {
				rtp_ttxt_flush(ch, (u32) ch->sl_hdr.compositionTimeStamp);
			}
			if (ttu_len<9) break;
			ch->sl_hdr.compositionTimeStamp = ts;
			ch->sl_hdr.idleFlag = is_utf_16;
			nb_frag = BS_ReadInt(bs, 4);
			cur_frag = BS_ReadInt(bs, 4);
			duration = BS_ReadInt(bs, 24);
			sidx = BS_ReadInt(bs, 8);
			BS_ReadInt(bs, 16);/*complete text sample size, ignored*/
			txt_size = size - 10;
			
			/*init - 3GPP/MPEG-4 spliting is IMHO stupid: 
				- nb frag & cur frags are not needed: rtp reordering insures packet are in order, and 
			!!!we assume fragments are sent in order!!!
				- any other TTU suffices to indicate end of text string (modifiers or != RTP TS)
				- replacing these 8bits field with a 16 bit absolute character offset would add error recovery
			*/
			if (!ch->nb_txt_frag) {
				ch->nb_txt_frag = nb_frag;
				ch->cur_txt_frag = 0;
				ch->sidx = sidx;
			}
			/*flush prev if any mismatch*/
			if ((nb_frag != ch->nb_txt_frag) || (ch->cur_txt_frag>cur_frag)) {
				rtp_ttxt_flush(ch, (u32) ch->sl_hdr.compositionTimeStamp);
				ch->nb_txt_frag = nb_frag;
				ch->sidx = sidx;
			}
			if (!ch->ttu1) ch->ttu1 = NewBitStream(NULL, 0, BS_WRITE);

			/*we don't reorder - RTP reordering is done at lower level, if this is out of order too bad*/
			ch->cur_txt_frag = cur_frag;
			BS_WriteData(ch->ttu1, payload+10, txt_size);
			BS_SkipBytes(bs, txt_size);

			ch->sl_hdr.au_duration = duration;
			/*done*/
			if (hdr->Marker) {
				ch->txt_len = (u32) BS_GetPosition(ch->ttu1);
				rtp_ttxt_flush(ch, ts);
			}
		} else if ((type==3) || (type==4)) {
			if (!ch->ttu1) ch->ttu1 = NewBitStream(NULL, 0, BS_WRITE);
			/*first modifier, store effective written text*/
			if (type==3) ch->txt_len = (u32) BS_GetPosition(ch->ttu1);
			if (ttu_len<6) break;

			nb_frag = BS_ReadInt(bs, 4);
			if (!ch->nb_mod_frag) ch->nb_mod_frag = nb_frag;
			else if (ch->nb_mod_frag != nb_frag) {
				rtp_ttxt_flush(ch, (u32) ch->sl_hdr.compositionTimeStamp);
				ch->nb_mod_frag = nb_frag;
			}
			BS_ReadInt(bs, 4);  /*cur_frag, ignore*/
			ch->sl_hdr.au_duration = BS_ReadInt(bs, 24);
			BS_WriteData(ch->ttu1, payload+7, ttu_len-6);
			BS_SkipBytes(bs, ttu_len-6);

			/*done*/
			if (hdr->Marker) rtp_ttxt_flush(ch, ts);
		}
	}
	DeleteBitStream(bs);
}
