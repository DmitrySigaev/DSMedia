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

#include <gpac/intern/m4_od_dev.h>

//
//	Constructor
//
Descriptor *NewM4FObjectDescriptor()
{
	M4F_ObjectDescriptor *newDesc = (M4F_ObjectDescriptor *) malloc(sizeof(M4F_ObjectDescriptor));
	if (!newDesc) return NULL;

	newDesc->URLString = NULL;
	newDesc->ES_ID_IncDescriptors = NewChain();
	newDesc->ES_ID_RefDescriptors = NewChain();
	newDesc->OCIDescriptors = NewChain();
	newDesc->IPMPDescriptorPointers = NewChain();
	newDesc->extensionDescriptors = NewChain();
	newDesc->objectDescriptorID = 0;
	newDesc->tag = MP4_OD_Tag;
	return (Descriptor *) newDesc;
}

//
//	Destructor
//
M4Err DelM4_OD(M4F_ObjectDescriptor *od)
{
	M4Err e;
	if (!od) return M4BadParam;
	if (od->URLString)	free(od->URLString);
	e = DeleteDescriptorList(od->ES_ID_IncDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(od->ES_ID_RefDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(od->OCIDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(od->IPMPDescriptorPointers);
	if (e) return e;
	e = DeleteDescriptorList(od->extensionDescriptors);
	if (e) return e;
	free(od);
	return M4OK;
}

//
//	Add a desc to an OD in an MP4 file
//
M4Err AddDescriptorToM4_OD(M4F_ObjectDescriptor *od, Descriptor *desc)
{
	if (!od || !desc) return M4BadParam;

	//check if we can handle ContentClassif tags
	if ( (desc->tag >= ISO_OCI_RANGE_START) &&
		(desc->tag <= ISO_OCI_RANGE_END) ) {
		return ChainAddEntry(od->OCIDescriptors, desc);
	}

	//or extension ...
	if ( (desc->tag >= ISO_EXT_RANGE_START) &&
		(desc->tag <= ISO_EXT_RANGE_END) ) {
		return ChainAddEntry(od->extensionDescriptors, desc);
	}

	switch (desc->tag) {
	case ESDescriptor_Tag:
		return M4DescriptorNotAllowed;

	case ES_ID_IncTag:
		//there shouldn't be ref if inc
		if (ChainGetCount(od->ES_ID_RefDescriptors)) return M4DescriptorNotAllowed;
		return ChainAddEntry(od->ES_ID_IncDescriptors, desc);

	case ES_ID_RefTag:
		//there shouldn't be inc if ref
		if (ChainGetCount(od->ES_ID_IncDescriptors)) return M4DescriptorNotAllowed;
		return ChainAddEntry(od->ES_ID_RefDescriptors, desc);

	//we use the same struct for v1 and v2 IPMP DPs
	case IPMPPtr_Tag:
		return ChainAddEntry(od->IPMPDescriptorPointers, desc);

	default:
		return M4BadParam;
	}
}

//
//	Reader
//
M4Err ReadM4_OD(BitStream *bs, M4F_ObjectDescriptor *od, u32 DescSize)
{
	M4Err e;
	u32 reserved, urlflag;
	u32 tmpSize, nbBytes = 0;
	if (! od) return M4BadParam;

	od->objectDescriptorID = BS_ReadInt(bs, 10);
	urlflag = BS_ReadInt(bs, 1);
	reserved = BS_ReadInt(bs, 5);
	nbBytes += 2;
	
	if (urlflag) {
		e = OD_ReadString(bs, & od->URLString, 1);
		if (e) return e;
		nbBytes += strlen(od->URLString) + 1;
	}

	while (nbBytes < DescSize) {
		Descriptor *tmp = NULL;
		e = ParseDescriptor(bs, &tmp, &tmpSize);
		if (e) return e;
		if (!tmp) return M4ReadDescriptorFailed;
		e = AddDescriptorToM4_OD(od, tmp);
		if (e) return e;
		nbBytes += tmpSize + GetSizeFieldSize(tmpSize);
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}



//
//	Size
//
M4Err SizeM4_OD(M4F_ObjectDescriptor *od, u32 *outSize)
{
	M4Err e;
	if (! od) return M4BadParam;

	*outSize = 2;
	if (od->URLString) {
		*outSize += strlen(od->URLString) + 1;
	} else {
		e = calcDescListSize(od->ES_ID_IncDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(od->ES_ID_RefDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(od->OCIDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(od->IPMPDescriptorPointers, outSize);
		if (e) return e;
	}
	return calcDescListSize(od->extensionDescriptors, outSize);
}

//
//	Writer
//
M4Err WriteM4_OD(BitStream *bs, M4F_ObjectDescriptor *od)
{
	M4Err e;
	u32 size;
	if (! od) return M4BadParam;

	e = CalcSize((Descriptor *)od, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, od->tag, size);
	if (e) return e;
	
	BS_WriteInt(bs, od->objectDescriptorID, 10);
	BS_WriteInt(bs, od->URLString != NULL ? 1 : 0, 1);
	BS_WriteInt(bs, 31, 5);		//reserved: 0b1111.1 == 31

	if (od->URLString) {
		OD_WriteString(bs, od->URLString, 1);
	} else {
		e = writeDescList(bs, od->ES_ID_IncDescriptors);
		if (e) return e;
		e = writeDescList(bs, od->ES_ID_RefDescriptors);
		if (e) return e;
		e = writeDescList(bs, od->OCIDescriptors);
		if (e) return e;
		e = writeDescList(bs, od->IPMPDescriptorPointers);
		if (e) return e;
	}
	e = writeDescList(bs, od->extensionDescriptors);
	if (e) return e;
	return M4OK;
}

