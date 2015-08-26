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

HintSample *New_HintSample(u32 ProtocolType)
{
	HintSample *tmp;
	u8 type;
	//check we can support the protocol desired
	switch (ProtocolType) {
	case RTPHintSampleEntryAtomType:
		type = PCK_RTP;
		break;
	default:
		return NULL;
	}
	tmp = malloc(sizeof(HintSample));
	tmp->packetTable = NewChain();
	tmp->AdditionalData = NULL;
	tmp->dataLength = 0;
	tmp->HintType = type;
	tmp->TransmissionTime = 0;
	tmp->reserved = 0;
	return tmp;
}

void Del_HintSample(HintSample *ptr)
{
	HintPacket *pck;

	while (ChainGetCount(ptr->packetTable)) {
		pck = ChainGetEntry(ptr->packetTable, 0);
		Del_HintPacket(ptr->HintType, pck);
		ChainDeleteEntry(ptr->packetTable, 0);
	}
	DeleteChain(ptr->packetTable);
	if (ptr->AdditionalData) free(ptr->AdditionalData);
	free(ptr);
}

M4Err Read_HintSample(HintSample *ptr, BitStream *bs, u32 sampleSize)
{
	u16 entryCount, i;
	HintPacket *pck;
	M4Err e;
	u64 sizeIn, sizeOut;

	sizeIn = BS_Available(bs);

	entryCount = BS_ReadInt(bs, 16);
	ptr->reserved = BS_ReadInt(bs, 16);

	for (i = 0; i < entryCount; i++) {
		pck = New_HintPacket(ptr->HintType);
		e = Read_HintPacket(ptr->HintType, pck, bs);
		if (e) return e;
		ChainAddEntry(ptr->packetTable, pck);
	}

	sizeOut = BS_Available(bs) - sizeIn;

	//do we have some more data after the packets ??
	if ((u32)sizeOut < sampleSize) {
		ptr->dataLength = sampleSize - (u32)sizeOut;
		ptr->AdditionalData = malloc(sizeof(char) * ptr->dataLength);
		BS_ReadData(bs, ptr->AdditionalData, ptr->dataLength);
	}
	return M4OK;
}


#ifndef M4_READ_ONLY

u32 Write_HintSample(HintSample *ptr, BitStream *bs)
{
	u32 count, i;
	HintPacket *pck;
	M4Err e;

	count = ChainGetCount(ptr->packetTable);
	BS_WriteInt(bs, count, 16);
	BS_WriteInt(bs, ptr->reserved, 16);
	//write the packet table
	for (i=0; i<count; i++) {
		pck = ChainGetEntry(ptr->packetTable, i);
		e = Write_HintPacket(ptr->HintType, pck, bs);
		if (e) return e;
	}
	//write additional data
	if (ptr->AdditionalData) {
		BS_WriteData(bs, ptr->AdditionalData, ptr->dataLength);
	}
	return M4OK;
}


u32 Size_HintSample(HintSample *ptr)
{
	u32 size, count, i;
	HintPacket *pck;

	size = 4;
	count = ChainGetCount(ptr->packetTable);
	for (i=0; i<count; i++) {
		pck = ChainGetEntry(ptr->packetTable, i);
		size += Size_HintPacket(ptr->HintType, pck);
	}
	size += ptr->dataLength;
	return size;
}

#endif

