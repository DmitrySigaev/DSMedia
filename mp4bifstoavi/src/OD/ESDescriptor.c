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
Descriptor *NewESDescriptor()
{
	ESDescriptor *newDesc = (ESDescriptor *) malloc(sizeof(ESDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(ESDescriptor));
	newDesc->IPIDataSet = NewChain();
	newDesc->IPMPDescriptorPointers = NewChain();
	newDesc->langDesc = NewChain();
	newDesc->extensionDescriptors = NewChain();
	newDesc->tag = ESDescriptor_Tag;
	return (Descriptor *) newDesc;
}


//
//	Desctructor
//
M4Err DelESD(ESDescriptor *esd)
{
	M4Err e;
	if (!esd) return M4BadParam;
	if (esd->URLString)	free(esd->URLString);

	if (esd->decoderConfig)	{
		e = DelDesc((Descriptor *) esd->decoderConfig);
		if (e) return e;
	}
	if (esd->slConfig) {
		e = DelDesc((Descriptor *) esd->slConfig);
		if (e) return e;
	}
	if (esd->ipiPtr) {
		e = DelDesc((Descriptor *) esd->ipiPtr);
		if (e) return e;
	}
	if (esd->local_pl) {
		e = DelDesc((Descriptor *) esd->local_pl);
		if (e) return e;
	}
	if (esd->qos) {
		e = DelDesc((Descriptor *) esd->qos);
		if (e) return e;
	}
	if (esd->RegDescriptor)	{
		e = DelDesc((Descriptor *) esd->RegDescriptor);
		if (e) return e;
	}

	e = DeleteDescriptorList(esd->IPIDataSet);
	if (e) return e;
	e = DeleteDescriptorList(esd->langDesc);
	if (e) return e;
	e = DeleteDescriptorList(esd->IPMPDescriptorPointers);
	if (e) return e;
	e = DeleteDescriptorList(esd->extensionDescriptors);
	if (e) return e;
	free(esd);
	return M4OK;
}


M4Err AddDescriptorToESD(ESDescriptor *esd, Descriptor *desc)
{
	if (!esd || !desc) return M4BadParam;

	switch (desc->tag) {
	case DecoderConfigDescriptor_Tag:
		if (esd->decoderConfig) return M4InvalidDescriptor;
		esd->decoderConfig = (DecoderConfigDescriptor *) desc;
		break;

	case SLConfigDescriptor_Tag:
		if (esd->slConfig) return M4InvalidDescriptor;
		esd->slConfig = (SLConfigDescriptor *) desc;
		break;

	//the IPI_DescPtr_Tag is only used in the file format and replaces IPIPtr_Tag...
	case IPI_DescPtr_Tag:
	case IPIPtr_Tag:
		if (esd->ipiPtr) return M4InvalidDescriptor;
		esd->ipiPtr = (IPI_DescrPointer *) desc;
		break;

	case QoS_Tag:
		if (esd->qos) return M4InvalidDescriptor;
		esd->qos  =(QoS_Descriptor *) desc;
		break;

	case LanguageDescriptor_Tag:
		return ChainAddEntry(esd->langDesc, desc);

	case ContentIdentification_Tag:
	case SuppContentIdentification_Tag:
		return ChainAddEntry(esd->IPIDataSet, desc);

	//we use the same struct for v1 and v2 IPMP DPs
	case IPMPPtr_Tag:
		return ChainAddEntry(esd->IPMPDescriptorPointers, desc);

	case RegistrationDescriptor_Tag:
		if (esd->RegDescriptor) return M4InvalidDescriptor;
		esd->RegDescriptor =(RegistrationDescriptor *) desc;
		break;

	case MuxInfoDescriptor_Tag:
		ChainAddEntry(esd->extensionDescriptors, desc);
		break;

	default:
		if ( (desc->tag >= ISO_EXT_RANGE_START) &&
			(desc->tag <= ISO_EXT_RANGE_END) ) {
			return ChainAddEntry(esd->extensionDescriptors, desc);
		}
		DelDesc(desc);
		return M4OK;
	}

	return M4OK;
}

//
//	Reader
//
M4Err ReadESD(BitStream *bs, ESDescriptor *esd, u32 DescSize)
{
	M4Err e = M4OK;
	u32 ocrflag, urlflag, streamdependflag, tmp_size, nbBytes;

	if (! esd) return M4BadParam;

	nbBytes = 0;

	esd->ESID = BS_ReadInt(bs, 16);
	streamdependflag = BS_ReadInt(bs, 1);
	urlflag = BS_ReadInt(bs, 1);
	ocrflag = BS_ReadInt(bs, 1);
	esd->streamPriority = BS_ReadInt(bs, 5);
	nbBytes += 3;
	
	if (streamdependflag) {
		esd->dependsOnESID = BS_ReadInt(bs, 16);
		nbBytes += 2;
	}

	if (urlflag) {
		e = OD_ReadString(bs, & esd->URLString, 1);
		if (e) return e;
		nbBytes += strlen(esd->URLString) + 1;
	}
	if (ocrflag) {
		esd->OCRESID = BS_ReadInt(bs, 16);
		nbBytes += 2;
	}
	/*fix broken sync*/
//	if (esd->OCRESID == esd->ESID) esd->OCRESID = 0;

	while (nbBytes < DescSize) {
		Descriptor *tmp = NULL;
		e = ParseDescriptor(bs, &tmp, &tmp_size);
		/*fix for iPod files*/
		if (e==M4InvalidDescriptor) {
			nbBytes += tmp_size;
			if (nbBytes>DescSize) return e;
			BS_ReadInt(bs, DescSize-nbBytes);
			return M4OK;
		}
		if (e) return e;
		if (!tmp) return M4ReadDescriptorFailed;
		e = AddDescriptorToESD(esd, tmp);
		if (e) return e;
		nbBytes += tmp_size + GetSizeFieldSize(tmp_size);
		
		//apple fix
		if (!tmp_size) nbBytes = DescSize;

	}
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return e;

}





//
//	Size
//
M4Err SizeESD(ESDescriptor *esd, u32 *outSize)
{
	M4Err e;
	u32 tmpSize;
	if (! esd) return M4BadParam;

	*outSize = 0;
	*outSize += 3;

	/*this helps keeping proper sync: some people argue that OCR_ES_ID == ES_ID is a circular reference
	of streams. Since this is equivalent to no OCR_ES_ID, keep it that way*/
//	if (esd->OCRESID == esd->ESID) esd->OCRESID = 0;

	if (esd->dependsOnESID) *outSize += 2;
	if (esd->URLString) *outSize += strlen(esd->URLString) + 1;
	if (esd->OCRESID) *outSize += 2;

	if (esd->decoderConfig) {
		e = CalcSize((Descriptor *) esd->decoderConfig, &tmpSize);
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	if (esd->slConfig) {
		e = CalcSize((Descriptor *) esd->slConfig, &tmpSize);
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}

	if (esd->local_pl) {
		e = CalcSize((Descriptor *) esd->local_pl, &tmpSize);
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}

	if (esd->ipiPtr) {
		e = CalcSize((Descriptor *) esd->ipiPtr, &tmpSize);	
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	e = calcDescListSize(esd->IPIDataSet, outSize);
	if (e) return e;
	e = calcDescListSize(esd->IPMPDescriptorPointers, outSize);
	if (e) return e;
	e = calcDescListSize(esd->langDesc, outSize);
	if (e) return e;
	if (esd->qos) {
		e = CalcSize((Descriptor *) esd->qos, &tmpSize);	
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	if (esd->RegDescriptor) {
		e = CalcSize((Descriptor *) esd->RegDescriptor, &tmpSize);	
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	return calcDescListSize(esd->extensionDescriptors, outSize);
}

//
//		Writer
//
M4Err WriteESD(BitStream *bs, ESDescriptor *esd)
{
	M4Err e;
	u32 size;
	if (! esd) return M4BadParam;

	//THIS REMOVES THE PRECOR1 FLAG IF ANY - cf SizeESD()
	e = CalcSize((Descriptor *)esd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, esd->tag, size);
	if (e) return e;

	BS_WriteInt(bs, esd->ESID, 16);
	BS_WriteInt(bs, esd->dependsOnESID ? 1 : 0, 1);
	BS_WriteInt(bs, esd->URLString != NULL ? 1 : 0, 1);
	BS_WriteInt(bs, esd->OCRESID ? 1 : 0, 1);
	BS_WriteInt(bs, esd->streamPriority, 5);

	if (esd->dependsOnESID) {
		BS_WriteInt(bs, esd->dependsOnESID, 16);
	}
	if (esd->URLString) {
		e = OD_WriteString(bs, esd->URLString, 1);
		if (e) return e;
	}


	if (esd->OCRESID) {
		BS_WriteInt(bs, esd->OCRESID, 16);
	}
	if (esd->decoderConfig) {
		e = WriteDesc(bs, (Descriptor *) esd->decoderConfig);
		if (e) return e;
	}
	if (esd->slConfig) {
		e = WriteDesc(bs, (Descriptor *) esd->slConfig);
		if (e) return e;
	}
	if (esd->ipiPtr) {
		e = WriteDesc(bs, (Descriptor *) esd->ipiPtr);
		if (e) return e;
	}
	e = writeDescList(bs, esd->IPIDataSet);
	if (e) return e;
	e = writeDescList(bs, esd->IPMPDescriptorPointers);
	if (e) return e;
	e = writeDescList(bs, esd->langDesc);
	if (e) return e;
	if (esd->qos) {
		e = WriteDesc(bs, (Descriptor *) esd->qos);
		if (e) return e;
	}
	if (esd->RegDescriptor) {
		e = WriteDesc(bs, (Descriptor *) esd->RegDescriptor);
		if (e) return e;
	}
	if (esd->local_pl) {
		e = WriteDesc(bs, (Descriptor *) esd->local_pl);
		if (e) return e;
	}
	return writeDescList(bs, esd->extensionDescriptors);
}


