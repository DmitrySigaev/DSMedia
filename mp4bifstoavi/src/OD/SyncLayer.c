/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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

#include <m4_descriptors.h>

/*allocates and writes the SL-PDU (Header + PDU) given the SLConfig and the SLHeader
for this PDU. AUs must be split in PDUs by another process if needed (packetizer).*/
void SL_Packetize(SLConfigDescriptor* slConfig, 
				  SLHeader *Header, 
				  char *PDU, 
				  u32 size,
				  char **outPacket,
				  u32 *OutSize)
{
	BitStream *bs = NewBitStream(NULL, 0, BS_WRITE);
	*OutSize = 0;
	if (!bs) return;

	if (slConfig->useAccessUnitStartFlag) BS_WriteInt(bs, Header->accessUnitStartFlag, 1);
	if (slConfig->useAccessUnitEndFlag) BS_WriteInt(bs, Header->accessUnitEndFlag, 1);
	if (slConfig->OCRLength > 0) BS_WriteInt(bs, Header->OCRflag, 1);
	if (slConfig->useIdleFlag) BS_WriteInt(bs, Header->idleFlag, 1);
	if (slConfig->usePaddingFlag) {
		BS_WriteInt(bs, Header->paddingFlag, 1);
		if (Header->paddingFlag) BS_WriteInt(bs, Header->paddingBits, 3);
	}
	if (! Header->idleFlag && (! Header->paddingFlag || Header->paddingBits != 0)) {
		if (slConfig->packetSeqNumLength > 0) BS_WriteInt(bs, Header->packetSequenceNumber, slConfig->packetSeqNumLength);
		if (slConfig->degradationPriorityLength > 0) {
			BS_WriteInt(bs, Header->degradationPriorityFlag, 1);
			if (Header->degradationPriorityFlag) BS_WriteInt(bs, Header->degradationPriority, slConfig->degradationPriorityLength);
		}
		if (Header->OCRflag) BS_WriteLongInt(bs, Header->objectClockReference, slConfig->OCRLength);
		if (Header->accessUnitStartFlag) {
			if (slConfig->useRandomAccessPointFlag) BS_WriteInt(bs, Header->randomAccessPointFlag, 1);
			if (slConfig->AUSeqNumLength > 0) BS_WriteInt(bs, Header->AU_sequenceNumber, slConfig->AUSeqNumLength);
			if (slConfig->useTimestampsFlag) {
				BS_WriteInt(bs, Header->decodingTimeStampFlag, 1);
				BS_WriteInt(bs, Header->compositionTimeStampFlag, 1);
			}
			if (slConfig->instantBitrateLength > 0) BS_WriteInt(bs, Header->instantBitrateFlag, 1);
			if (Header->decodingTimeStampFlag) BS_WriteLongInt(bs, Header->decodingTimeStamp, slConfig->timestampLength);
			if (Header->compositionTimeStampFlag) BS_WriteLongInt(bs, Header->compositionTimeStamp, slConfig->timestampLength);
			if (slConfig->AULength > 0) BS_WriteInt(bs, Header->accessUnitLength, slConfig->AULength);
			if (Header->instantBitrateFlag) BS_WriteInt(bs, Header->instantBitrate, slConfig->instantBitrateLength);
		}
	}
	//done with the Header, Alin
	BS_Align(bs);
	//write the PDU - already byte aligned with stuffing (paddingBits in SL Header)
	BS_WriteData(bs, PDU, size);

	BS_Align(bs);
	BS_GetContent(bs, (unsigned char **) outPacket, OutSize);
	DeleteBitStream(bs);
}


void SL_Depacketize (SLConfigDescriptor *slConfig, SLHeader *Header, char *PDU, u32 PDULength, u32 *HeaderLen)
{
	BitStream *bs;
	*HeaderLen = 0;
	if (!Header) return;
	//reset the input header
	memset(Header, 0, sizeof(SLHeader));

	bs = NewBitStream(PDU, PDULength, BS_READ);
	if (!bs) return;

	if (slConfig->useAccessUnitStartFlag) Header->accessUnitStartFlag = BS_ReadInt(bs, 1);
	if (slConfig->useAccessUnitEndFlag) Header->accessUnitEndFlag = BS_ReadInt(bs, 1);
	if ( !slConfig->useAccessUnitStartFlag && !slConfig->useAccessUnitEndFlag) {
		Header->accessUnitStartFlag = 1;
		Header->accessUnitEndFlag = 1;
	}
	if (slConfig->OCRLength > 0) Header->OCRflag = BS_ReadInt(bs, 1);
	if (slConfig->useIdleFlag) Header->idleFlag = BS_ReadInt(bs, 1);
	if (slConfig->usePaddingFlag) {
		Header->paddingFlag = BS_ReadInt(bs, 1);
		if (Header->paddingFlag) Header->paddingBits = BS_ReadInt(bs, 3);
	}
	if (!Header->idleFlag && (!Header->paddingFlag || Header->paddingBits != 0)) {

		if (slConfig->packetSeqNumLength > 0) Header->packetSequenceNumber = BS_ReadInt(bs, slConfig->packetSeqNumLength);
		if (slConfig->degradationPriorityLength > 0) {
			Header->degradationPriorityFlag = BS_ReadInt(bs, 1);
			if (Header->degradationPriorityFlag) Header->degradationPriority = BS_ReadInt(bs, slConfig->degradationPriorityLength);
		}
		if (Header->OCRflag) Header->objectClockReference = BS_ReadInt(bs, slConfig->OCRLength);
		if (Header->accessUnitStartFlag) {
			if (slConfig->useRandomAccessPointFlag) Header->randomAccessPointFlag = BS_ReadInt(bs, 1);
			if (slConfig->AUSeqNumLength > 0) Header->AU_sequenceNumber = BS_ReadInt(bs, slConfig->AUSeqNumLength);
			if (slConfig->useTimestampsFlag) {
				Header->decodingTimeStampFlag = BS_ReadInt(bs, 1);
				Header->compositionTimeStampFlag = BS_ReadInt(bs, 1);
			}
			if (slConfig->instantBitrateLength > 0) Header->instantBitrateFlag = BS_ReadInt(bs, 1);
			if (Header->decodingTimeStampFlag) Header->decodingTimeStamp = BS_ReadLongInt(bs, slConfig->timestampLength); 
			if (Header->compositionTimeStampFlag) Header->compositionTimeStamp = BS_ReadLongInt(bs, slConfig->timestampLength); 
			if (slConfig->AULength > 0) Header->accessUnitLength = BS_ReadInt(bs, slConfig->AULength);
			if (Header->instantBitrateFlag) Header->instantBitrate = BS_ReadInt(bs, slConfig->instantBitrateLength);
		}
	}
	/*FIXME - is header aligned ??? */
	BS_Align(bs);

	*HeaderLen = (u32) BS_GetPosition(bs);
	DeleteBitStream(bs);
}
