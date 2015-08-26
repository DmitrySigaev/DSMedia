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


RTPPacket *New_RTPPacket()
{
	RTPPacket *tmp = malloc(sizeof(RTPPacket));
	tmp->TLV = NewChain();
	tmp->DataTable = NewChain();
	tmp->B_bit = tmp->M_bit = tmp->P_bit = tmp->payloadType = tmp->payloadType = tmp->R_bit = tmp->X_bit = 0;
	tmp->relativeTransTime = 0;
	tmp->SequenceNumber = 0;
	return tmp;
}

void Del_RTPPacket(RTPPacket *ptr)
{
	GenericDTE *p;
	//the DTE
	while (ChainGetCount(ptr->DataTable)) {
		p = ChainGetEntry(ptr->DataTable, 0);
		DelDTE(p);
		ChainDeleteEntry(ptr->DataTable, 0);
	}
	DeleteChain(ptr->DataTable);
	//the TLV
	DeleteAtomList(ptr->TLV);
	free(ptr);
}

M4Err Read_RTPPacket(RTPPacket *ptr, BitStream *bs)
{
	M4Err e;
	u8 hasTLV, type;
	u16 i, count;
	u32 TLVsize, tempSize;
	GenericDTE *dte;
	Atom *a;

	ptr->relativeTransTime = BS_ReadInt(bs, 32);
	//RTP Header
	//1- reserved fields
	BS_ReadInt(bs, 2);
	ptr->P_bit = BS_ReadInt(bs, 1);
	ptr->X_bit = BS_ReadInt(bs, 1);
	BS_ReadInt(bs, 4);
	ptr->M_bit = BS_ReadInt(bs, 1);
	ptr->payloadType = BS_ReadInt(bs, 7);

	ptr->SequenceNumber = BS_ReadInt(bs, 16);
	BS_ReadInt(bs, 13);
	hasTLV = BS_ReadInt(bs, 1);
	ptr->B_bit = BS_ReadInt(bs, 1);
	ptr->R_bit = BS_ReadInt(bs, 1);
	count = BS_ReadInt(bs, 16);
	
	//read the TLV
	if (hasTLV) {
		tempSize = 4;	//TLVsize includes its field length 
		TLVsize = BS_ReadInt(bs, 32);
		//and parse the atoms in here ...
		while (tempSize < TLVsize) {
			u64 sr;
			e = ParseAtom(&a, bs, &sr);
			if (e) return e;
			ChainAddEntry(ptr->TLV, a);
			tempSize += (u32) a->size;
		}
		if (tempSize != TLVsize) return M4ReadAtomFailed;
	}

	//read the DTEs
	for (i=0; i<count; i++) {
		type = BS_ReadInt(bs, 8);
		dte = NewDTE(type);
		e = ReadDTE(dte, bs);
		if (e) return e;
		ChainAddEntry(ptr->DataTable, dte);
	}
	return M4OK;
}

M4Err Offset_RTPPacket(RTPPacket *ptr, u32 offset, u32 HintSampleNumber)
{
	u32 count, i;
	GenericDTE *dte;
	M4Err e;
	
	count = ChainGetCount(ptr->DataTable);

	for (i=0; i<count; i++) {
		dte = ChainGetEntry(ptr->DataTable, i);
		e = OffsetDTE(dte, offset, HintSampleNumber);
		if (e) return e;
	}
	return M4OK;
}

//Gets the REAL size of the packet once rebuild, but without CSRC fields in the
//header
u32 Length_RTPPacket(RTPPacket *ptr)
{
	u32 size, count, i;
	GenericDTE *dte;

	//64 bit header
	size = 8;
	//32 bit SSRC
	size += 4;
	count = ChainGetCount(ptr->DataTable);
	for (i=0; i<count; i++) {
		dte = ChainGetEntry(ptr->DataTable, i);
		switch (dte->source) {
		case 0:
			break;
		case 1:
			size += ((ImmediateDTE *)dte)->dataLength;
			break;
		case 2:
			size += ((SampleDTE *)dte)->dataLength;
			break;
		case 3:
			size += ((StreamDescDTE *)dte)->dataLength;
			break;
		}
	}
	return size;
}


#ifndef M4_READ_ONLY

u32 Size_RTPPacket(RTPPacket *ptr)
{
	Atom none;
	u32 size, count;
	//the RTP Header size and co
	size = 12;
	//the extra table size
	count = ChainGetCount(ptr->TLV);
	if (count) {
		//get the size of the table (it's just an atom list)
		none.size = 4;	//WE INCLUDE THE SIZE FIELD LENGTH
		none.type = 0;
		//REMEMBER THAT TLV ENTRIES ARE 4-BYTES ALIGNED !!!
		SizeAtomList(&none, ptr->TLV);
		size += (u32) none.size;
	}
	//the DTE (each entry is 16 bytes)
	count = ChainGetCount(ptr->DataTable);
	size += count * 16;
	return size;
}

M4Err Write_RTPPacket(RTPPacket *ptr, BitStream *bs)
{
	M4Err e;
	u32 TLVcount, DTEcount, i;
	Atom none;
	GenericDTE *dte;

	BS_WriteInt(bs, ptr->relativeTransTime, 32);
	//RTP Header
//	BS_WriteInt(bs, 2, 2);
	//version is 2
	BS_WriteInt(bs, 2, 2);
	BS_WriteInt(bs, ptr->P_bit, 1);
	BS_WriteInt(bs, ptr->X_bit, 1);
	BS_WriteInt(bs, 0, 4);
	BS_WriteInt(bs, ptr->M_bit, 1);
	BS_WriteInt(bs, ptr->payloadType, 7);

	BS_WriteInt(bs, ptr->SequenceNumber, 16);
	BS_WriteInt(bs, 0, 13);
	TLVcount = ChainGetCount(ptr->TLV);
	DTEcount = ChainGetCount(ptr->DataTable);
	BS_WriteInt(bs, TLVcount ? 1 : 0, 1);
	BS_WriteInt(bs, ptr->B_bit, 1);
	BS_WriteInt(bs, ptr->R_bit, 1);

	BS_WriteInt(bs, DTEcount, 16);

	if (TLVcount) {
		//first write the size of the table ...
		none.size = 4;	//WE INCLUDE THE SIZE FIELD LENGTH
		none.type = 0;
		SizeAtomList(&none, ptr->TLV);
		BS_WriteInt(bs, (u32) none.size, 32);
		//then write the atom list
		e = WriteAtomList(&none, ptr->TLV, bs);
		if (e) return e;
	}
	//write the DTE...
	for (i = 0; i < DTEcount; i++) {
		dte = ChainGetEntry(ptr->DataTable, i);
		e = WriteDTE(dte, bs);
		if (e) return e;
	}
	return M4OK;
}

#endif	//M4_READ_ONLY
