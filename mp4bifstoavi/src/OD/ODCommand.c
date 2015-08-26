/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Object Descriptor sub-project
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


#include <intern/m4_od_dev.h>

M4Err ParseCommand(BitStream *bs, ODCommand **com, u32 *com_size)
{
	u32 val, size, sizeHeader;
	u8 tag;
	M4Err err;
	ODCommand *newCom;
	if (!bs) return M4BadParam;

	*com_size = 0;

	//tag
	tag = BS_ReadInt(bs, 8);
	sizeHeader = 1;
	
	//size
	size = 0;
	do {
		val = BS_ReadInt(bs, 8);
		sizeHeader++;
		size <<= 7;
		size |= val & 0x7F;
	} while ( val & 0x80 );
	*com_size = size;

	newCom = CreateCom(tag);
	if (! newCom) {
		*com = NULL;
		return M4OutOfMem;
	}

	newCom->tag = tag;

	err = ReadCom(bs, newCom, *com_size);
	//little trick to handle lazy bitstreams that encode 
	//SizeOfInstance on a fix number of bytes
	//This nb of bytes is added in Read methods
	*com_size += sizeHeader - GetSizeFieldSize(*com_size);
	*com = newCom;
	if (err) {
		DelCom(newCom);
		*com = NULL;
	}
	return err;
}


ODCommand *NewBaseCom()
{
	BaseODCommand *newCom = (BaseODCommand *) malloc(sizeof(BaseODCommand));
	if (!newCom) return NULL;
	newCom->dataSize = 0;
	newCom->data = NULL;
	return (ODCommand *)newCom;
}
M4Err DelBaseCom(BaseODCommand *bcRemove)
{
	if (! bcRemove) return M4BadParam;
	if (bcRemove->data) free(bcRemove->data);
	free(bcRemove);
	return M4OK;
}

M4Err ReadBaseCom(BitStream *bs, BaseODCommand *bcRem, u32 ComSize)
{
	if (! bcRem) return M4BadParam;

	bcRem->dataSize = ComSize;
	bcRem->data = (char *) malloc(sizeof(char) * bcRem->dataSize);
	if (! bcRem->data) return M4OutOfMem;
	BS_ReadData(bs, (unsigned char*)bcRem->data, bcRem->dataSize);
	return M4OK;
}
M4Err SizeBaseCom(BaseODCommand *bcRem, u32 *outSize)
{
	if (!bcRem) return M4BadParam;
	*outSize = bcRem->dataSize;
	return M4OK;
}
M4Err WriteBaseCom(BitStream *bs, BaseODCommand *bcRem)
{
	u32 size;
	M4Err e;
	if (! bcRem) return M4BadParam;

	e = SizeBaseCom(bcRem, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, bcRem->tag, size);
	if (e) return e;
	BS_WriteData(bs, (unsigned char*)bcRem->data, bcRem->dataSize);
	return M4OK;
}

ODCommand *NewODRemove()
{
	ObjectDescriptorRemove *newCom = (ObjectDescriptorRemove *) malloc(sizeof(ObjectDescriptorRemove));
	if (!newCom) return NULL;
	newCom->NbODs = 0;
	newCom->OD_ID = NULL;
	newCom->tag = ODRemove_Tag;
	return (ODCommand *)newCom;
}
M4Err DelODRemove(ObjectDescriptorRemove *ODRemove)
{
	if (! ODRemove) return M4BadParam;
	if (ODRemove->OD_ID) free(ODRemove->OD_ID);
	free(ODRemove);
	return M4OK;
}
M4Err ReadODRemove(BitStream *bs, ObjectDescriptorRemove *odRem, u32 ComSize)
{
	u32 i = 0, nbBits;
	if (! odRem) return M4BadParam;

	odRem->NbODs = (u32 ) (ComSize * 8) / 10;
	odRem->OD_ID = (u16 *) malloc(sizeof(u16) * odRem->NbODs);
	if (! odRem->OD_ID) return M4OutOfMem;

	for (i = 0; i < odRem->NbODs ; i++) {
		odRem->OD_ID[i] = BS_ReadInt(bs, 10);
	}
	nbBits = odRem->NbODs * 10;
	//now we need to align !!!
	nbBits += BS_Align(bs);
	if (nbBits != (ComSize * 8)) return M4ReadODCommandFailed;
	return M4OK;
}

M4Err SizeODRemove(ObjectDescriptorRemove *odRem, u32 *outSize)
{
	u32 size;
	if (!odRem) return M4BadParam;
	
	size = 10 * odRem->NbODs;
	*outSize = 0;
	*outSize = size/8;
	if (*outSize * 8 != size) *outSize += 1;
	return M4OK;
}

M4Err WriteODRemove(BitStream *bs, ObjectDescriptorRemove *odRem)
{
	M4Err e;
	u32 size, i;
	if (! odRem) return M4BadParam;

	e = SizeODRemove(odRem, &size);
	e = writeBaseDescriptor(bs, odRem->tag, size);

	for (i = 0; i < odRem->NbODs; i++) {
		BS_WriteInt(bs, odRem->OD_ID[i], 10);
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}


/*******************************************************
	OD Execute
*******************************************************/
ODCommand *NewODExecute()
{
	ObjectDescriptorExecute *newCom = (ObjectDescriptorExecute *) malloc(sizeof(ObjectDescriptorExecute));
	if (!newCom) return NULL;
	newCom->NbODs = 0;
	newCom->OD_ID = NULL;
	newCom->tag = ODExecute_Tag;
	return (ODCommand *)newCom;
}
M4Err DelODExecute(ObjectDescriptorExecute *ODExec)
{
	if (!ODExec) return M4BadParam;
	if (ODExec->OD_ID) free(ODExec->OD_ID);
	free(ODExec);
	return M4OK;
}

M4Err ReadODExecute(BitStream *bs, ObjectDescriptorExecute *ODExec, u32 ComSize)
{
	u32 i = 0, nbBits;
	if (!ODExec) return M4BadParam;

	ODExec->NbODs = (u32 ) (ComSize * 8) / 10;
	ODExec->OD_ID = (u16 *) malloc(sizeof(u16) * ODExec->NbODs);
	if (!ODExec->OD_ID) return M4OutOfMem;

	for (i = 0; i < ODExec->NbODs ; i++) {
		ODExec->OD_ID[i] = BS_ReadInt(bs, 10);
	}
	nbBits = ODExec->NbODs * 10;
	//now we need to align !!!
	nbBits += BS_Align(bs);
	if (nbBits != (ComSize * 8)) return M4ReadODCommandFailed;
	return M4OK;
}

M4Err SizeODExecute(ObjectDescriptorExecute *ODExec, u32 *outSize)
{
	u32 size;
	if (!ODExec) return M4BadParam;
	
	size = 10 * ODExec->NbODs;
	*outSize = 0;
	*outSize = size/8;
	if (*outSize * 8 != size) *outSize += 1;
	return M4OK;
}

M4Err WriteODExecute(BitStream *bs, ObjectDescriptorExecute *ODExec)
{
	M4Err e;
	u32 size, i;
	if (!ODExec) return M4BadParam;

	e = SizeODExecute(ODExec, &size);
	e = writeBaseDescriptor(bs, ODExec->tag, size);

	for (i = 0; i < ODExec->NbODs; i++) {
		BS_WriteInt(bs, ODExec->OD_ID[i], 10);
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}

ODCommand *NewODUpdate()
{
	ObjectDescriptorUpdate *newCom = (ObjectDescriptorUpdate *) malloc(sizeof(ObjectDescriptorUpdate));
	if (!newCom) return NULL;
	
	newCom->objectDescriptors = NewChain();
	if (! newCom->objectDescriptors) {
		free(newCom);
		return NULL;
	}
	newCom->tag = ODUpdate_Tag;
	return (ODCommand *)newCom;
}

M4Err DelODUpdate(ObjectDescriptorUpdate *ODUpdate)
{
	M4Err e;
	if (! ODUpdate) return M4BadParam;
	while (ChainGetCount(ODUpdate->objectDescriptors)) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(ODUpdate->objectDescriptors, 0);
		e = DelDesc(tmp);
		if (e) return e;
		e = ChainDeleteEntry(ODUpdate->objectDescriptors, 0);
		if (e) return e;
	}
	DeleteChain(ODUpdate->objectDescriptors);
	free(ODUpdate);
	return M4OK;
}



M4Err AddToODUpdate(ObjectDescriptorUpdate *odUp, Descriptor *desc)
{
	if (! odUp) return M4BadParam;
	if (! desc) return M4OK;

	switch (desc->tag) {
	case ObjectDescriptor_Tag:
	case InitialObjectDescriptor_Tag:
	case MP4_IOD_Tag:
	case MP4_OD_Tag:
		return ChainAddEntry(odUp->objectDescriptors, desc);

	default:
		DelDesc(desc);
		return M4OK;
	}
}

M4Err ReadODUpdate(BitStream *bs, ObjectDescriptorUpdate *odUp, u32 ComSize)
{
	Descriptor *tmp;
	M4Err e = M4OK;
	u32 tmpSize = 0, nbBytes = 0;
	if (! odUp) return M4BadParam;

	while (nbBytes < ComSize) {
		e = ParseDescriptor(bs, &tmp, &tmpSize);
		if (e) return e;
		e = AddToODUpdate(odUp, tmp);
		if (e) return e;
		nbBytes += tmpSize + GetSizeFieldSize(tmpSize);
	}
	//OD commands are aligned
	BS_Align(bs);
	if (nbBytes != ComSize) return M4ReadODCommandFailed;
	return e;
}
M4Err SizeODUpdate(ObjectDescriptorUpdate *odUp, u32 *outSize)
{
	Descriptor *tmp;
	u32 i, tmpSize;
	if (!odUp) return M4BadParam;

	*outSize = 0;
	for (i = 0; i < ChainGetCount(odUp->objectDescriptors); i++) {
		tmp = (Descriptor*)ChainGetEntry(odUp->objectDescriptors, i);
		CalcSize(tmp, &tmpSize);
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	return M4OK;
}
M4Err WriteODUpdate(BitStream *bs, ObjectDescriptorUpdate *odUp)
{
	M4Err e;
	u32 size, i;
	if (! odUp) return M4BadParam;

	e = SizeODUpdate(odUp, &size);
	if (e) return e;
	writeBaseDescriptor(bs, odUp->tag, size);
	if (e) return e;

	for (i = 0; i < ChainGetCount(odUp->objectDescriptors); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(odUp->objectDescriptors, i);
		e = WriteDesc(bs, tmp);
		if (e) return e;
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}


ODCommand *NewESDUpdate()
{
	ESDescriptorUpdate *newCom = (ESDescriptorUpdate *) malloc(sizeof(ESDescriptorUpdate));
	if (!newCom) return NULL;
	
	newCom->ESDescriptors = NewChain();
	if (! newCom->ESDescriptors) {
		free(newCom);
		return NULL;
	}
	newCom->tag = ESDUpdate_Tag;
	return (ODCommand *)newCom;
}

M4Err DelESDUpdate(ESDescriptorUpdate *ESDUpdate)
{
	M4Err e;
	if (! ESDUpdate) return M4BadParam;
	while (ChainGetCount(ESDUpdate->ESDescriptors)) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(ESDUpdate->ESDescriptors, 0);
		e = DelDesc(tmp);
		if (e) return e;
		e = ChainDeleteEntry(ESDUpdate->ESDescriptors, 0);
		if (e) return e;
	}
	DeleteChain(ESDUpdate->ESDescriptors);
	free(ESDUpdate);
	return M4OK;
}

M4Err AddToESDUpdate(ESDescriptorUpdate *esdUp, Descriptor *desc)
{
	if (! esdUp) return M4BadParam;
	if (!desc) return M4OK;

	switch (desc->tag) {
	case ESDescriptor_Tag:
	case ES_ID_RefTag:
		return ChainAddEntry(esdUp->ESDescriptors, desc);

	default:
		DelDesc(desc);
		return M4OK;
	}
}

M4Err ReadESDUpdate(BitStream *bs, ESDescriptorUpdate *esdUp, u32 ComSize)
{
	Descriptor *tmp;
	u32 tmpSize = 0, nbBits = 0;
	M4Err e = M4OK;
	if (! esdUp) return M4BadParam;

	esdUp->ODID = BS_ReadInt(bs, 10);
	nbBits += 10;
	//very tricky, we're at the bit level here...
	while (1) {
		e = ParseDescriptor(bs, &tmp, &tmpSize);
		if (e) return e;
		e = AddToESDUpdate(esdUp, tmp);
		if (e) return e;
		nbBits += ( tmpSize + GetSizeFieldSize(tmpSize) ) * 8;
		//our com is aligned, so nbBits is between (ComSize-1)*8 and ComSize*8
		if ( ( (nbBits >(ComSize-1)*8) && (nbBits <= ComSize * 8)) 
			|| (nbBits > ComSize*8) ) {	//this one is a security break
			break;
		}
	}
	if (nbBits > ComSize * 8) return M4ReadODCommandFailed;
	//Align our bitstream
	nbBits += BS_Align(bs);
	if (nbBits != ComSize *8) return M4ReadODCommandFailed;
	return e;
}



M4Err SizeESDUpdate(ESDescriptorUpdate *esdUp, u32 *outSize)
{
	u32 i, BitSize, tmpSize;
	if (!esdUp) return M4BadParam;

	*outSize = 0;
	BitSize = 10;
	for (i = 0; i < ChainGetCount(esdUp->ESDescriptors); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(esdUp->ESDescriptors, i);
		CalcSize(tmp, &tmpSize);
		BitSize += ( tmpSize + GetSizeFieldSize(tmpSize) ) * 8;
	}
	while ((s32) BitSize > 0) {
		BitSize -= 8;
		*outSize += 1;
	}
	return M4OK;
}
M4Err WriteESDUpdate(BitStream *bs, ESDescriptorUpdate *esdUp)
{
	M4Err e;
	u32 size, i;
	if (! esdUp) return M4BadParam;

	e = SizeESDUpdate(esdUp, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, esdUp->tag, size);
	if (e) return e;

	BS_WriteInt(bs, esdUp->ODID, 10);
	for (i = 0; i < ChainGetCount(esdUp->ESDescriptors); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(esdUp->ESDescriptors, i);
		e = WriteDesc(bs, tmp);
		if (e) return e;
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}


ODCommand *NewESDRemove()
{
	ESDescriptorRemove *newCom = (ESDescriptorRemove *) malloc(sizeof(ESDescriptorRemove));
	if (!newCom) return NULL;
	newCom->NbESDs = 0;
	newCom->ES_ID = NULL;
	newCom->tag = ESDRemove_Tag;
	return (ODCommand *)newCom;
}

M4Err DelESDRemove(ESDescriptorRemove *ESDRemove)
{
	if (! ESDRemove) return M4BadParam;
	if (ESDRemove->ES_ID) free(ESDRemove->ES_ID);
	free(ESDRemove);
	return M4OK;
}


M4Err ReadESDRemove(BitStream *bs, ESDescriptorRemove *esdRem, u32 ComSize)
{
	u32 i = 0, aligned, nbBits;
	if (! esdRem) return M4BadParam;

	esdRem->ODID = BS_ReadInt(bs, 10);
	aligned = BS_ReadInt(bs, 6);		//aligned

	//we have ComSize - 2 bytes left, and this is our ES_ID[1...255]
	//this works because OD commands are aligned
	if (ComSize < 2) return M4InvalidDescriptor;
	if (ComSize == 2) {
		esdRem->NbESDs = 0;
		esdRem->ES_ID = NULL;
		return M4OK;
	}
	esdRem->NbESDs = (ComSize - 2) / 2;
	esdRem->ES_ID = (u16 *) malloc(sizeof(u16) * esdRem->NbESDs);
	if (! esdRem->ES_ID) return M4OutOfMem;
	for (i = 0; i < esdRem->NbESDs ; i++) {
		esdRem->ES_ID[i] = BS_ReadInt(bs, 16);
	}
	//OD commands are aligned (but we should already be aligned....
	nbBits = BS_Align(bs);
	return M4OK;
}

M4Err SizeESDRemove(ESDescriptorRemove *esdRem, u32 *outSize)
{
	if (!esdRem) return M4BadParam;
	*outSize = 2 + 2 * esdRem->NbESDs;
	return M4OK;
}
M4Err WriteESDRemove(BitStream *bs, ESDescriptorRemove *esdRem)
{
	M4Err e;
	u32 size, i;
	if (! esdRem) return M4BadParam;

	e = SizeESDRemove(esdRem, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, esdRem->tag, size);
	if (e) return e;

	BS_WriteInt(bs, esdRem->ODID, 10);
	BS_WriteInt(bs, 0, 6);		//aligned
	for (i = 0; i < esdRem->NbESDs ; i++) {
		BS_WriteInt(bs, esdRem->ES_ID[i], 16);
	}
	//OD commands are aligned (but we are already aligned....
	BS_Align(bs);
	return M4OK;
}


ODCommand *NewIPMPDRemove()
{
	IPMPDescriptorRemove *newCom = (IPMPDescriptorRemove *) malloc(sizeof(IPMPDescriptorRemove));
	if (!newCom) return NULL;
	newCom->IPMPDescID  =NULL;
	newCom->NbIPMPDs = 0;
	newCom->tag = IPMPDRemove_Tag;
	return (ODCommand *)newCom;
}

M4Err DelIPMPDRemove(IPMPDescriptorRemove *IPMPDRemove)
{
	if (! IPMPDRemove) return M4BadParam;
	if (IPMPDRemove->IPMPDescID) free(IPMPDRemove->IPMPDescID);
	free(IPMPDRemove);
	return M4OK;
}

M4Err ReadIPMPDRemove(BitStream *bs, IPMPDescriptorRemove *ipmpRem, u32 ComSize)
{
	u32 i;
	if (! ipmpRem) return M4BadParam;

	//we have ComSize bytes left, and this is our IPMPD_ID[1...255]
	//this works because OD commands are aligned
	if (!ComSize) return M4OK;

	ipmpRem->NbIPMPDs = ComSize;
	ipmpRem->IPMPDescID = (u8 *) malloc(sizeof(u8) * ipmpRem->NbIPMPDs);
	if (! ipmpRem->IPMPDescID) return M4OutOfMem;

	for (i = 0; i < ipmpRem->NbIPMPDs; i++) {
		ipmpRem->IPMPDescID[i] = BS_ReadInt(bs, 8);
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}

M4Err SizeIPMPDRemove(IPMPDescriptorRemove *ipmpRem, u32 *outSize)
{
	if (!ipmpRem) return M4BadParam;

	*outSize = ipmpRem->NbIPMPDs;
	return M4OK;
}
M4Err WriteIPMPDRemove(BitStream *bs, IPMPDescriptorRemove *ipmpRem)
{
	M4Err e;
	u32 size, i;
	if (! ipmpRem) return M4BadParam;

	e = SizeIPMPDRemove(ipmpRem, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ipmpRem->tag, size);
	if (e) return e;

	for (i = 0; i < ipmpRem->NbIPMPDs; i++) {
		BS_WriteInt(bs, ipmpRem->IPMPDescID[i], 8);
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}

ODCommand *NewIPMPDUpdate()
{
	IPMPDescriptorUpdate *newCom = (IPMPDescriptorUpdate *) malloc(sizeof(IPMPDescriptorUpdate));
	if (!newCom) return NULL;
	newCom->IPMPDescList = NewChain();
	if (! newCom->IPMPDescList) {
		free(newCom);
		return NULL;
	}
	newCom->tag = IPMPDUpdate_Tag;
	return (ODCommand *)newCom;
}

M4Err DelIPMPDUpdate(IPMPDescriptorUpdate *IPMPDUpdate)
{
	M4Err e;
	if (! IPMPDUpdate) return M4BadParam;
	while (ChainGetCount(IPMPDUpdate->IPMPDescList)) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(IPMPDUpdate->IPMPDescList, 0);
		e = DelDesc(tmp);
		e = ChainDeleteEntry(IPMPDUpdate->IPMPDescList, 0);
	}
	DeleteChain(IPMPDUpdate->IPMPDescList);
	free(IPMPDUpdate);
	return M4OK;
}

M4Err AddToIPMPDUpdate(IPMPDescriptorUpdate *ipmpUp, Descriptor *desc)
{
	if (! ipmpUp) return M4BadParam;
	if (!desc) return M4OK;

	switch (desc->tag) {
	case IPMP_Tag:
		return ChainAddEntry(ipmpUp->IPMPDescList, desc);
	default:
		DelDesc(desc);
		return M4OK;
	}
}

M4Err ReadIPMPDUpdate(BitStream *bs, IPMPDescriptorUpdate *ipmpUp, u32 ComSize)
{
	Descriptor *tmp;
	u32 tmpSize = 0, nbBytes = 0;
	M4Err e = M4OK;
	if (! ipmpUp) return M4BadParam;

	while (nbBytes < ComSize) {
		e = ParseDescriptor(bs, &tmp, &tmpSize);
		if (e) return e;
		e = AddToIPMPDUpdate(ipmpUp, tmp);
		if (e) return e;
		nbBytes += tmpSize + GetSizeFieldSize(tmpSize);
	}
	//OD commands are aligned
	BS_Align(bs);
	if (nbBytes != ComSize) return M4ReadODCommandFailed;
	return e;
}


M4Err SizeIPMPDUpdate(IPMPDescriptorUpdate *ipmpUp, u32 *outSize)
{
	u32 i, tmpSize;
	if (!ipmpUp) return M4BadParam;

	*outSize = 0;
	for (i = 0; i < ChainGetCount(ipmpUp->IPMPDescList); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(ipmpUp->IPMPDescList, i);
		CalcSize(tmp, &tmpSize);
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	return M4OK;
}
M4Err WriteIPMPDUpdate(BitStream *bs, IPMPDescriptorUpdate *ipmpUp)
{
	M4Err e;
	u32 size, i;
	if (! ipmpUp) return M4BadParam;

	e = SizeIPMPDUpdate(ipmpUp, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ipmpUp->tag, size);
	if (e) return e;

	for (i = 0; i < ChainGetCount(ipmpUp->IPMPDescList); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(ipmpUp->IPMPDescList, i);
		e = WriteDesc(bs, tmp);
		if (e) return e;
	}
	//OD commands are aligned
	BS_Align(bs);
	return M4OK;
}
