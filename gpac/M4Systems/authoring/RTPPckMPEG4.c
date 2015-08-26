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

//get the size of the RSLH section given the SLHeader and the SLMap
u32 M4RTP_AUHeaderSize(M4RTPBuilder *builder, SLHeader *slh)
{
	u32 nbBits = 0;
	
	/*no input header specified, compute the MAX size*/
	if (!slh) {
		/*size length*/
		if (!builder->slMap.ConstantSize) nbBits += builder->slMap.SizeLength;
		/*AU index length*/
		if (builder->first_sl_in_rtp) {
			if (builder->slMap.IndexLength) nbBits += builder->slMap.IndexLength;
		} else {
			if (builder->slMap.IndexDeltaLength) nbBits += builder->slMap.IndexDeltaLength;
		}
		/*CTS flag*/
		if (builder->slMap.CTSDeltaLength) {
			nbBits += 1;
			/*all non-first packets have the CTS written if asked*/
			if (!builder->first_sl_in_rtp) nbBits += builder->slMap.CTSDeltaLength;
		}
		if (builder->slMap.DTSDeltaLength) nbBits += 1 + builder->slMap.DTSDeltaLength;
		return nbBits;
	}

	/*size length*/
	if (!builder->slMap.ConstantSize) nbBits += builder->slMap.SizeLength;

	/*AU index*/
	if (builder->first_sl_in_rtp) {
		if (builder->slMap.IndexLength) nbBits += builder->slMap.IndexLength;
	} else {
		if (builder->slMap.IndexDeltaLength) nbBits += builder->slMap.IndexDeltaLength;
	}

	/*CTS Flag*/
	if (builder->slMap.CTSDeltaLength) {
		/*CTS not written if first SL*/
		if (builder->first_sl_in_rtp) slh->compositionTimeStampFlag = 0;
		/*but CTS flag is always written*/
		nbBits += 1;
	} else {
		slh->compositionTimeStampFlag = 0;
	}
	/*CTS*/
	if (slh->compositionTimeStampFlag) nbBits += builder->slMap.CTSDeltaLength;

	/*DTS Flag*/
	if (builder->slMap.DTSDeltaLength) {
		nbBits += 1;
	} else {
		slh->decodingTimeStampFlag = 0;
	}
	/*DTS*/
	if (slh->decodingTimeStampFlag) nbBits += builder->slMap.DTSDeltaLength;
	/*RAP indication*/
	if (builder->slMap.RandomAccessIndication) nbBits ++;
	/*streamState indication*/
	nbBits += builder->slMap.StreamStateIndication;

	return nbBits;
}


/*write the AU header section - return the nb of BITS written for AU header*/
u32 M4RTP_WriteAUHeader(M4RTPBuilder *builder, u32 PayloadSize, u32 RTP_TS)
{
	u32 nbBits = 0;
	s32 delta;
	
	/*size length*/
	if (builder->slMap.ConstantSize) {
		if (PayloadSize != builder->slMap.ConstantSize) return 0;
	} else {
		/*write the AU size - if not enough bytes (real-time cases) set size to 0*/
		if (builder->sl_header.accessUnitLength >= (1<<builder->slMap.SizeLength)) {
			BS_WriteInt(builder->auheader, 0, builder->slMap.SizeLength);
		} else {
			BS_WriteInt(builder->auheader, builder->sl_header.accessUnitLength, builder->slMap.SizeLength);
		}
		nbBits += builder->slMap.SizeLength;
	}
	/*AU index*/
	if (builder->first_sl_in_rtp) {
		if (builder->slMap.IndexLength) {
			BS_WriteInt(builder->auheader, builder->sl_header.AU_sequenceNumber, builder->slMap.IndexLength);
			nbBits += builder->slMap.IndexLength;
		}
	} else {
		if (builder->slMap.IndexDeltaLength) {
			//check interleaving, otherwise force default (which is currently always the case)
			delta = builder->sl_header.AU_sequenceNumber - builder->last_au_sn;
			delta -= 1;
			BS_WriteInt(builder->auheader, delta, builder->slMap.IndexDeltaLength);
			nbBits += builder->slMap.IndexDeltaLength;
		}
	}

	/*CTS Flag*/
	if (builder->slMap.CTSDeltaLength) {
		if (builder->first_sl_in_rtp) {
			builder->sl_header.compositionTimeStampFlag = 0;
			builder->sl_header.compositionTimeStamp = RTP_TS;
		}
		BS_WriteInt(builder->auheader, builder->sl_header.compositionTimeStampFlag, 1);
		nbBits += 1;
	}
	/*CTS*/
	if (builder->sl_header.compositionTimeStampFlag) {
		delta = (u32) builder->sl_header.compositionTimeStamp - RTP_TS;
		BS_WriteInt(builder->auheader, delta, builder->slMap.CTSDeltaLength);
		nbBits += builder->slMap.CTSDeltaLength;
	}
	/*DTS Flag*/
	if (builder->slMap.DTSDeltaLength) {
		BS_WriteInt(builder->auheader, builder->sl_header.decodingTimeStampFlag, 1);
		nbBits += 1;
	}
	/*DTS*/
	if (builder->sl_header.decodingTimeStampFlag) {
		delta = (u32) (builder->sl_header.compositionTimeStamp - builder->sl_header.decodingTimeStamp);
		BS_WriteInt(builder->auheader, delta, builder->slMap.DTSDeltaLength);
		nbBits += builder->slMap.DTSDeltaLength;
	}
	/*RAP indication*/
	if (builder->slMap.RandomAccessIndication) {
		BS_WriteInt(builder->auheader, builder->sl_header.randomAccessPointFlag, 1);
		nbBits ++;
	}
	return nbBits;
}


M4Err M4RTP_ProcessMPEG4(M4RTPBuilder *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize)
{
	char *sl_buffer, *payl_buffer;
	u32 sl_buffer_size, payl_buffer_size; 
	u32 auh_size_tmp, rslh_tmp, bytesLeftInPacket, infoSize, pckSize, pos;
	u8 flush_pck, no_split;

	flush_pck = 0;
	rslh_tmp = 0;

	bytesLeftInPacket = data_size;
	/*flush everything*/
	if (!data) {
		if (builder->payload) goto flush_packet;
		return M4OK;
	}

	//go till done
	while (bytesLeftInPacket) {
		no_split = 0;

		if (builder->sl_header.accessUnitStartFlag) {
			//init SL
			if (builder->sl_header.compositionTimeStamp != builder->sl_header.decodingTimeStamp) {
				builder->sl_header.decodingTimeStampFlag = 1;
			}
			builder->sl_header.compositionTimeStampFlag = 1;
			builder->sl_header.accessUnitLength = FullAUSize;

			//init some vars - based on the available size and the TS
			//we decide if we go with the same RTP TS serie or not
			if (builder->payload) {
				//don't store more than what we can (that is 2^slMap->CTSDelta - 1)
				if ( (builder->flags & M4HF_SignalTS) 
					&& (builder->sl_header.compositionTimeStamp - builder->rtp_header.TimeStamp >= ( 1 << builder->slMap.CTSDeltaLength) ) ) {
					goto flush_packet;
				}
				//don't split AU if # TS , start a new RTP pck 
				if (builder->sl_header.compositionTimeStamp != builder->rtp_header.TimeStamp)
					no_split = 1;
			}
			//adding a non RAP sample
			if (builder->payload && !builder->sl_header.randomAccessPointFlag) {
				builder->RAP_Packet = 0;
			}
		}

		/*new RTP Packet*/
		if (!builder->payload) {
			/*first SL in RTP*/
			builder->first_sl_in_rtp = 1;

			/*if this is the end of an AU we will set it to 0 as soon as an AU is splited*/
			builder->rtp_header.Marker = 1;
			builder->rtp_header.PayloadType = builder->PayloadType;
			builder->rtp_header.SequenceNumber += 1;

			builder->rtp_header.TimeStamp = (u32) builder->sl_header.compositionTimeStamp;
			/*RAP*/
			builder->RAP_Packet = builder->sl_header.randomAccessPointFlag;

			/*prepare the mapped headers*/
			builder->auheader = NewBitStream(NULL, 0, BS_WRITE);
			builder->payload = NewBitStream(NULL, 0, BS_WRITE);
			pckSize = infoSize = 0;
			builder->bytesInPacket = 0;

			/*in multiSL there is a MSLHSize structure on 2 bytes*/
			builder->auh_size = 0;
			if (builder->has_AU_header) {
				builder->auh_size = 16;
				BS_WriteInt(builder->auheader, 0, 16);
			}			
			flush_pck = 0;
			/*and create packet*/
			builder->OnNewPacket(builder->cbk_obj, &builder->rtp_header);
		}

		//make sure we are not interleaving too much - this should never happen actually
		if (builder->slMap.IndexDeltaLength 
			&& !builder->first_sl_in_rtp 
			&& (builder->sl_header.AU_sequenceNumber - builder->last_au_sn >= (u32) 1<<builder->slMap.IndexDeltaLength)) {
			//we cannot write this packet here
			goto flush_packet;
		} 

		auh_size_tmp = M4RTP_AUHeaderSize(builder, &builder->sl_header);

		infoSize = auh_size_tmp + builder->auh_size;
		infoSize /= 8;
		/*align*/
		if ( (builder->auh_size + auh_size_tmp) % 8) infoSize += 1;


		if (bytesLeftInPacket + infoSize + builder->bytesInPacket <= builder->Path_MTU) {
			//End of our data chunk
			pckSize = bytesLeftInPacket;
			builder->sl_header.accessUnitEndFlag = IsAUEnd;

			builder->auh_size += auh_size_tmp;
			
			builder->sl_header.paddingFlag = builder->sl_header.paddingBits ? 1 : 0;
		} else {

			//AU cannot fit in packet. If no split, start a new packet
			if (no_split) goto flush_packet;

			builder->auh_size += auh_size_tmp;

			pckSize = builder->Path_MTU - (infoSize + builder->bytesInPacket);
			//that's the end of the rtp packet
			flush_pck = 1;
			//but not of the AU -> marker is 0
			builder->rtp_header.Marker = 0;
		}

		M4RTP_WriteAUHeader(builder, pckSize, builder->rtp_header.TimeStamp);
				
		//copy the payload
		BS_WriteData(builder->payload, data + (data_size - bytesLeftInPacket), pckSize);
		
		//notify the user of our data structure
		if (builder->OnDataReference) 
			builder->OnDataReference(builder->cbk_obj, pckSize, data_size - bytesLeftInPacket);

		bytesLeftInPacket -= pckSize;
		builder->bytesInPacket += pckSize;
		builder->sl_header.paddingFlag = 0;
		builder->sl_header.accessUnitStartFlag = 0;

		//we are splitting a payload, auto increment SL seq num
		if (bytesLeftInPacket) {
			builder->sl_header.packetSequenceNumber += 1;
		} else if (! (builder->flags & M4HF_UseMulti) ) {
			builder->rtp_header.Marker = 1;
			flush_pck = 1;
		}

		//first SL in RTP is done
		builder->first_sl_in_rtp = 0;

		//store current sl
		builder->last_au_sn = builder->sl_header.AU_sequenceNumber;

		if (!flush_pck) continue;

		//done with the packet
flush_packet:

		BS_Align(builder->auheader);

		/*no aux data yet*/
		if (builder->slMap.AuxiliaryDataSizeLength)	{
			//write RSLH after the MSLH
			BS_WriteInt(builder->auheader, 0, builder->slMap.AuxiliaryDataSizeLength);			
		}				
		/*rewrite the size header*/
		if (builder->has_AU_header) {
			pos = (u32) BS_GetPosition(builder->auheader);		
			BS_Seek(builder->auheader, 0);
			builder->auh_size -= 16;
			BS_WriteInt(builder->auheader, builder->auh_size, 16);
			BS_Seek(builder->auheader, pos);
		}

		sl_buffer = NULL;
		BS_GetContent(builder->auheader, (unsigned char **) &sl_buffer, &sl_buffer_size);
		//delete our bitstream
		DeleteBitStream(builder->auheader);
		builder->auheader = NULL;

		payl_buffer = NULL;
		BS_GetContent(builder->payload, (unsigned char **) &payl_buffer, &payl_buffer_size);
		DeleteBitStream(builder->payload);
		builder->payload = NULL;

		//this is the FINAL RTP PACKET, notify
		builder->OnPacketDone(builder->cbk_obj, &builder->rtp_header, sl_buffer, sl_buffer_size, 
			payl_buffer, payl_buffer_size);

		free(payl_buffer);
		free(sl_buffer);
	}
	//packet is done, update AU markers
	if (IsAUEnd) {
		builder->sl_header.accessUnitStartFlag = 1;
		builder->sl_header.accessUnitEndFlag = 0;
	}
	return M4OK;
}

