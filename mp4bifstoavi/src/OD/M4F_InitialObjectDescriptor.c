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

//
//	Constructor
//
Descriptor *NewM4FInitialObjectDescriptor()
{
	M4F_InitialObjectDescriptor *newDesc = (M4F_InitialObjectDescriptor *) malloc(sizeof(M4F_InitialObjectDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(M4F_InitialObjectDescriptor));

	newDesc->ES_ID_IncDescriptors = NewChain();
	newDesc->ES_ID_RefDescriptors = NewChain();
	newDesc->OCIDescriptors = NewChain();
	newDesc->IPMPDescriptorPointers = NewChain();
	newDesc->extensionDescriptors = NewChain();
	newDesc->tag = MP4_IOD_Tag;

	//by default create an IOD with no inline and no capabilities
	newDesc->audio_profileAndLevel = 0xFF;
	newDesc->graphics_profileAndLevel = 0xFF;
	newDesc->scene_profileAndLevel = 0xFF;
	newDesc->OD_profileAndLevel = 0xFF;
	newDesc->visual_profileAndLevel = 0xFF;
	return (Descriptor *) newDesc;
}

//
//		Destructor
//
M4Err DelM4_IOD(M4F_InitialObjectDescriptor *iod)
{
	M4Err e;
	if (!iod) return M4BadParam;
	if (iod->URLString)	free(iod->URLString);
	e = DeleteDescriptorList(iod->ES_ID_IncDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(iod->ES_ID_RefDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(iod->OCIDescriptors);
	if (e) return e;
	e = DeleteDescriptorList(iod->IPMPDescriptorPointers);
	if (e) return e;
	e = DeleteDescriptorList(iod->extensionDescriptors);
	if (e) return e;
	free(iod);
	return M4OK;
}

//
//	Add a desc to the IOD for an MP4 file...
//
M4Err AddDescriptorToM4_IOD(M4F_InitialObjectDescriptor *iod, Descriptor *desc)
{
	if (!iod || !desc) return M4BadParam;

	//check if we can handle ContentClassif tags
	if ( (desc->tag >= ISO_OCI_RANGE_START) &&
		(desc->tag <= ISO_OCI_RANGE_END) ) {
		return ChainAddEntry(iod->OCIDescriptors, desc);
	}

	//or extensions
	if ( (desc->tag >= ISO_EXT_RANGE_START) &&
		(desc->tag <= ISO_EXT_RANGE_END) ) {
		return ChainAddEntry(iod->extensionDescriptors, desc);
	}

	switch (desc->tag) {
	case ESDescriptor_Tag:
		return M4DescriptorNotAllowed;

	case ES_ID_IncTag:
		//there shouldn't be ref if inc
		if (ChainGetCount(iod->ES_ID_RefDescriptors)) return M4DescriptorNotAllowed;
		return ChainAddEntry(iod->ES_ID_IncDescriptors, desc);

	case ES_ID_RefTag:
		//there shouldn't be inc if ref
		if (ChainGetCount(iod->ES_ID_IncDescriptors)) return M4DescriptorNotAllowed;
		return ChainAddEntry(iod->ES_ID_RefDescriptors, desc);

	//we use the same struct for v1 and v2 IPMP DPs
	case IPMPPtr_Tag:
		return ChainAddEntry(iod->IPMPDescriptorPointers, desc);

	default:
		return M4BadParam;
	}
}

//
//	Reader
//
M4Err ReadM4_IOD(BitStream *bs, M4F_InitialObjectDescriptor *iod, u32 DescSize)
{
	u32 nbBytes = 0, tmpSize;
	u32 reserved, urlflag;
	M4Err e;
	if (! iod) return M4BadParam;

	iod->objectDescriptorID = BS_ReadInt(bs, 10);
	urlflag = BS_ReadInt(bs, 1);
	iod->inlineProfileFlag = BS_ReadInt(bs, 1);
	reserved = BS_ReadInt(bs, 4);
	nbBytes += 2;
	
	if (urlflag) {
		e = OD_ReadString(bs, & iod->URLString, 1);
		if (e) return e;
		nbBytes += strlen(iod->URLString) + 1;
	} else {
		iod->OD_profileAndLevel = BS_ReadInt(bs, 8);
		iod->scene_profileAndLevel = BS_ReadInt(bs, 8);
		iod->audio_profileAndLevel = BS_ReadInt(bs, 8);
		iod->visual_profileAndLevel = BS_ReadInt(bs, 8);
		iod->graphics_profileAndLevel = BS_ReadInt(bs, 8);
		nbBytes += 5;
	}

	while (nbBytes < DescSize) {
		Descriptor *tmp = NULL;
		e = ParseDescriptor(bs, &tmp, &tmpSize);
		if (e) return e;
		if (!tmp) return M4ReadDescriptorFailed;
		e = AddDescriptorToM4_IOD(iod, tmp);
		if (e) return e;
		nbBytes += tmpSize + GetSizeFieldSize(tmpSize);
	}
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}



//
//	Size
//
M4Err SizeM4_IOD(M4F_InitialObjectDescriptor *iod, u32 *outSize)
{
	M4Err e;
	if (! iod) return M4BadParam;

	*outSize = 2;
	if (iod->URLString) {
		*outSize += strlen(iod->URLString) + 1;
	} else {
		*outSize += 5;
		e = calcDescListSize(iod->ES_ID_IncDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(iod->ES_ID_RefDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(iod->OCIDescriptors, outSize);
		if (e) return e;
		e = calcDescListSize(iod->IPMPDescriptorPointers, outSize);
		if (e) return e;
	}
	return calcDescListSize(iod->extensionDescriptors, outSize);
}

//
//	Writer
//
M4Err WriteM4_IOD(BitStream *bs, M4F_InitialObjectDescriptor *iod)
{
	M4Err e;
	u32 size;
	if (! iod) return M4BadParam;

	e = CalcSize((Descriptor *)iod, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, iod->tag, size);
	if (e) return e;
	
	BS_WriteInt(bs, iod->objectDescriptorID, 10);
	BS_WriteInt(bs, iod->URLString != NULL ? 1 : 0, 1);
	BS_WriteInt(bs, iod->inlineProfileFlag, 1);
	BS_WriteInt(bs, 15, 4);		//reserved: 0b1111 == 15

	if (iod->URLString) {
		OD_WriteString(bs, iod->URLString, 1);
	} else {
		BS_WriteInt(bs, iod->OD_profileAndLevel, 8);
		BS_WriteInt(bs, iod->scene_profileAndLevel, 8);
		BS_WriteInt(bs, iod->audio_profileAndLevel, 8);
		BS_WriteInt(bs, iod->visual_profileAndLevel, 8);
		BS_WriteInt(bs, iod->graphics_profileAndLevel, 8);
		e = writeDescList(bs, iod->ES_ID_IncDescriptors);
		if (e) return e;
		e = writeDescList(bs, iod->ES_ID_RefDescriptors);
		if (e) return e;
		e = writeDescList(bs, iod->OCIDescriptors);
		if (e) return e;
		e = writeDescList(bs, iod->IPMPDescriptorPointers);
		if (e) return e;
	}
	e = writeDescList(bs, iod->extensionDescriptors);
	if (e) return e;
	return M4OK;
}

