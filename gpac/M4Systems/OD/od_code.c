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

#define DATE_CODING_BIT_LEN	40

/*use to parse strings read the length as well - Warning : the alloc is done here !!*/
M4Err OD_ReadString(BitStream *bs, char **string, u32 isUTF8)
{
	u32 length;

	/*if the string is not NULL, return an error...*/
	if (*string != NULL) return M4BadParam;

	/*the lenth is always on 8 bits*/
	length = BS_ReadInt(bs, 8) * (isUTF8 ? 1 : 2);

	/*we want to use strlen to get rid of "stringLength" => we need an extra 0*/
	(*string) = (char *) malloc(length + 1);
	if (! string) return M4OutOfMem;
	BS_ReadData(bs, (*string), length);
	(*string)[length] = 0;
	return M4OK;
}

/*writes string*/
M4Err OD_WriteString(BitStream *bs, char *string, char isUTF8)
{
	u32 len = 0, nbBytes;
	/*we accept NULL strings now*/
	if (!string) {
		BS_WriteInt(bs, 0, 8);
		return M4OK;
	}		
	nbBytes = strlen(string) * (isUTF8 ? 1 : 2);
	if (nbBytes > 255) return M4IOErr;

	BS_WriteInt(bs, strlen(string), 8);
	while (len < nbBytes) {
		BS_WriteInt(bs, (s32) string[len], 8);
		len ++;
	}
	return M4OK;
}


Descriptor *NewContentClassificationDescriptor()
{
	ContentClassificationDescriptor *newDesc = (ContentClassificationDescriptor *) malloc(sizeof(ContentClassificationDescriptor));
	if (!newDesc) return NULL;

	newDesc->contentClassificationData = NULL;
	newDesc->dataLength = 0;
	newDesc->classificationEntity = 0;
	newDesc->classificationTable = 0;
	newDesc->tag = ContentClassification_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelCC(ContentClassificationDescriptor *ccd)
{
	if (!ccd) return M4BadParam;
	if (ccd->contentClassificationData) free(ccd->contentClassificationData);
	free(ccd);
	return M4OK;
}

M4Err ReadCC(BitStream *bs, ContentClassificationDescriptor *ccd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!ccd) return M4BadParam;

	ccd->classificationEntity = BS_ReadInt(bs, 32);
	ccd->classificationTable = BS_ReadInt(bs, 16);
	nbBytes += 6;
	ccd->dataLength = DescSize - 6;
	ccd->contentClassificationData = (unsigned char*)malloc(sizeof(char) * ccd->dataLength);
	if (!ccd->contentClassificationData) return M4OutOfMem;
	BS_ReadData(bs, ccd->contentClassificationData, ccd->dataLength);
	nbBytes += ccd->dataLength;

	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeCC(ContentClassificationDescriptor *ccd, u32 *outSize)
{
	if (!ccd) return M4BadParam;
	*outSize = 6 + ccd->dataLength;
	return M4OK;
}

M4Err WriteCC(BitStream *bs, ContentClassificationDescriptor *ccd)
{
	u32 size;
	M4Err e;
	if (!ccd) return M4BadParam;

	e = CalcSize((Descriptor *)ccd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ccd->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ccd->classificationEntity, 32);
	BS_WriteInt(bs, ccd->classificationTable, 16);
	BS_WriteData(bs, ccd->contentClassificationData, ccd->dataLength);
	return M4OK;
}

Descriptor *NewContentCreatorDateDescriptor()
{
	ContentCreationDateDescriptor *newDesc = (ContentCreationDateDescriptor *) malloc(sizeof(ContentCreationDateDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc->contentCreationDate, 0, 5);
	newDesc->tag = ContentCreationDate_Tag;
	return (Descriptor *) newDesc;
}


M4Err DelCCD(ContentCreationDateDescriptor *cdd)
{
	if (!cdd) return M4BadParam;
	free(cdd);
	return M4OK;
}

M4Err ReadCCD(BitStream *bs, ContentCreationDateDescriptor *cdd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!cdd) return M4BadParam;

	BS_ReadData(bs, cdd->contentCreationDate, DATE_CODING_BIT_LEN);
	nbBytes += DATE_CODING_BIT_LEN / 8;
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeCCD(ContentCreationDateDescriptor *cdd, u32 *outSize)
{
	if (!cdd) return M4BadParam;
	*outSize = (DATE_CODING_BIT_LEN / 8);
	return M4OK;
}

M4Err WriteCCD(BitStream *bs, ContentCreationDateDescriptor *cdd)
{
	u32 size;
	M4Err e;
	if (!cdd) return M4BadParam;

	e = CalcSize((Descriptor *)cdd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, cdd->tag, size);
	if (e) return e;

	BS_WriteData(bs, cdd->contentCreationDate , DATE_CODING_BIT_LEN);
	return M4OK;
}

Descriptor *NewContentCreatorNameDescriptor()
{
	ContentCreatorNameDescriptor *newDesc = (ContentCreatorNameDescriptor *) malloc(sizeof(ContentCreatorNameDescriptor));
	if (!newDesc) return NULL;

	newDesc->ContentCreators = NewChain();
	if (! newDesc->ContentCreators) {
		free(newDesc);
		return NULL;
	}
	newDesc->tag = ContentCreatorName_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelCCN(ContentCreatorNameDescriptor *cnd)
{
	u32 i;
	if (!cnd) return M4BadParam;

	for (i = 0; i < ChainGetCount(cnd->ContentCreators); i++) {
		contentCreatorInfo *tmp = (contentCreatorInfo*)ChainGetEntry(cnd->ContentCreators, i);
		if (tmp->contentCreatorName) free(tmp->contentCreatorName);
		free(tmp);
	}
	DeleteChain(cnd->ContentCreators);
	free(cnd);
	return M4OK;
}

M4Err ReadCCN(BitStream *bs, ContentCreatorNameDescriptor *cnd, u32 DescSize)
{
	M4Err e;
	u32 i, aligned, count, nbBytes = 0;
	if (!cnd) return M4BadParam;

	count = BS_ReadInt(bs, 8);
	nbBytes += 1;
	for (i = 0; i< count; i++) {
		contentCreatorInfo *tmp = (contentCreatorInfo*)malloc(sizeof(contentCreatorInfo));
		if (! tmp) return M4OutOfMem;
		memset(tmp , 0, sizeof(contentCreatorInfo));
		tmp->langCode = BS_ReadInt(bs, 24);
		tmp->isUTF8 = BS_ReadInt(bs, 1);
		aligned = BS_ReadInt(bs, 7);
		nbBytes += 4;

		e = OD_ReadString(bs, & tmp->contentCreatorName, tmp->isUTF8);
		if (e) return e;
		e = ChainAddEntry(cnd->ContentCreators, tmp);
		if (e) return e;
		nbBytes += strlen(tmp->contentCreatorName) * (tmp->isUTF8 ? 1 : 2) + 1;
	}
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeCCN(ContentCreatorNameDescriptor *cnd, u32 *outSize)
{
	u32 i;
	if (!cnd) return M4BadParam;

	*outSize = 1;
	for (i = 0; i< ChainGetCount(cnd->ContentCreators) ; i++) {
		contentCreatorInfo *tmp = (contentCreatorInfo*)ChainGetEntry(cnd->ContentCreators, i);
		if (! tmp) return M4InvalidDescriptor;
		*outSize += 5 + strlen(tmp->contentCreatorName) * (tmp->isUTF8 ? 1 : 2);
	}
	return M4OK;
}

M4Err WriteCCN(BitStream *bs, ContentCreatorNameDescriptor *cnd)
{
	M4Err e;
	u32 i, size;
	if (!cnd) return M4BadParam;

	e = CalcSize((Descriptor *)cnd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, cnd->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ChainGetCount(cnd->ContentCreators), 8);

	for (i = 0; i < ChainGetCount(cnd->ContentCreators) ; i++) {
		contentCreatorInfo *tmp = (contentCreatorInfo*)ChainGetEntry(cnd->ContentCreators, i);
		if (! tmp) return M4InvalidDescriptor;
		BS_WriteInt(bs, tmp->langCode, 24);
		BS_WriteInt(bs, tmp->isUTF8, 1);
		BS_WriteInt(bs, 0, 7);		//aligned
		OD_WriteString(bs, tmp->contentCreatorName, tmp->isUTF8);
	}
	return M4OK;
}
	

Descriptor *NewContentIdentificationDescriptor()
{
	ContentIdentificationDescriptor *newDesc = (ContentIdentificationDescriptor *) malloc(sizeof(ContentIdentificationDescriptor));
	if (!newDesc) return NULL;

	newDesc->compatibility = 0;
	newDesc->contentIdentifier = NULL;
	newDesc->tag = ContentIdentification_Tag;
	newDesc->contentIdentifierFlag = 0;
	newDesc->contentIdentifierType = 0;
	newDesc->contentType = 0;
	newDesc->contentTypeFlag = 0;
	newDesc->protectedContent = 0;
	return (Descriptor *) newDesc;
}


M4Err DelCI(ContentIdentificationDescriptor *cid)
{
	if (!cid) return M4BadParam;

	if (cid->contentIdentifier) free(cid->contentIdentifier);
	free(cid);
	return M4OK;
}


M4Err ReadCI(BitStream *bs, ContentIdentificationDescriptor *cid, u32 DescSize)
{
	u32 reserved, nbBytes = 0;
	if (! cid) return M4BadParam;

	cid->compatibility = BS_ReadInt(bs, 2);	//MUST BE NULL
	if (cid->compatibility) return M4InvalidDescriptor;

	cid->contentTypeFlag = BS_ReadInt(bs, 1);
	cid->contentIdentifierFlag = BS_ReadInt(bs, 1);
	cid->protectedContent = BS_ReadInt(bs, 1);
	reserved = BS_ReadInt(bs, 3);
	nbBytes += 1;

	if (cid->contentTypeFlag) {
		cid->contentType = BS_ReadInt(bs, 8);
		nbBytes += 1;
	}
	if (cid->contentIdentifierFlag) {
		cid->contentIdentifierType = BS_ReadInt(bs, 8);
		cid->contentIdentifier = (unsigned char*)malloc(DescSize - 2 - cid->contentTypeFlag);
		if (! cid->contentIdentifier) return M4OutOfMem;

		BS_ReadData(bs, cid->contentIdentifier, DescSize - 2 - cid->contentTypeFlag);
		nbBytes += DescSize - 1 - cid->contentTypeFlag;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeCI(ContentIdentificationDescriptor *cid, u32 *outSize)
{
	if (! cid) return M4BadParam;

	*outSize = 1;
	if (cid->contentTypeFlag) *outSize += 1;

	if (cid->contentIdentifierFlag) 
		*outSize += strlen((const char*)cid->contentIdentifier) - 1 - cid->contentTypeFlag;
	return M4OK;
}

M4Err WriteCI(BitStream *bs, ContentIdentificationDescriptor *cid)
{
	M4Err e;
	u32 size;
	if (! cid) return M4BadParam;

	e = CalcSize((Descriptor *)cid, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, cid->tag, size);
	if (e) return e;

	BS_WriteInt(bs, cid->compatibility, 2);
	BS_WriteInt(bs, cid->contentTypeFlag, 1);
	BS_WriteInt(bs, cid->contentIdentifierFlag, 1);
	BS_WriteInt(bs, cid->protectedContent, 1);
	BS_WriteInt(bs, 7, 3);		//reserved 0b111 = 7

	if (cid->contentTypeFlag) {
		BS_WriteInt(bs, cid->contentType, 8);
	}

	if (cid->contentIdentifierFlag) {
		BS_WriteInt(bs, cid->contentIdentifierType, 8);
		BS_WriteData(bs, cid->contentIdentifier, size - 2 - cid->contentTypeFlag);
	}
	return M4OK;
}

Descriptor *NewDecoderConfigDescriptor()
{
	DecoderConfigDescriptor *newDesc = (DecoderConfigDescriptor *) malloc(sizeof(DecoderConfigDescriptor));
	if (!newDesc) return NULL;

	newDesc->avgBitrate = 0;
	newDesc->bufferSizeDB = 0;
	newDesc->maxBitrate = 0;
	newDesc->objectTypeIndication = 0;
	newDesc->streamType = 0;
	newDesc->upstream = 0;
	newDesc->decoderSpecificInfo = NULL;
	newDesc->profileLevelIndicationIndexDescriptor = NewChain();
	newDesc->tag = DecoderConfigDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelDCD(DecoderConfigDescriptor *dcd)
{
	M4Err e;
	if (!dcd) return M4BadParam;

	if (dcd->decoderSpecificInfo) {
		e = DelDesc((Descriptor *) dcd->decoderSpecificInfo);
		if (e) return e;
	}
	e = DeleteDescriptorList(dcd->profileLevelIndicationIndexDescriptor);
	if (e) return e;
	free(dcd);
	return M4OK;
}

M4Err ReadDCD(BitStream *bs, DecoderConfigDescriptor *dcd, u32 DescSize)
{
	M4Err e;
	u32 reserved, tmp_size, nbBytes = 0;
	if (! dcd) return M4BadParam;

	dcd->objectTypeIndication = BS_ReadInt(bs, 8);
	dcd->streamType = BS_ReadInt(bs, 6);
	dcd->upstream = BS_ReadInt(bs, 1);
	reserved = BS_ReadInt(bs, 1);
	dcd->bufferSizeDB = BS_ReadInt(bs, 24);
	dcd->maxBitrate = BS_ReadInt(bs, 32);
	dcd->avgBitrate = BS_ReadInt(bs, 32);
	nbBytes += 13;

	while (nbBytes < DescSize) {
		Descriptor *tmp = NULL;
		e = ParseDescriptor(bs, &tmp, &tmp_size);
		if (e) return e;
		if (!tmp) return M4ReadDescriptorFailed;
		switch (tmp->tag) {
		case DecoderSpecificInfo_Tag:
			if (dcd->decoderSpecificInfo) {
				DelDesc(tmp);
				return M4InvalidDescriptor;
			}
			dcd->decoderSpecificInfo = (DefaultDescriptor *) tmp;
			break;

		case ExtensionPL_Tag:
			e = ChainAddEntry(dcd->profileLevelIndicationIndexDescriptor, tmp);
			if (e < M4OK) {
				DelDesc(tmp);
				return e;
			}
			break;

		/*iPod fix: delete and aborts, this will create an InvalidDescriptor at the ESD level with a loaded DSI,
		laoding will abort with a partially valid ESD which is all the matters*/
		case SLConfigDescriptor_Tag:
			DelDesc(tmp);
			return M4OK;

		//what the hell is this descriptor ?? Don't know, so delete it !
		default:
			DelDesc(tmp);
			break;
		}
		nbBytes += tmp_size + GetSizeFieldSize(tmp_size);
	}
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeDCD(DecoderConfigDescriptor *dcd, u32 *outSize)
{
	M4Err e;
	u32 tmpSize;
	if (! dcd) return M4BadParam;

	*outSize = 0;
	*outSize += 13;
	if (dcd->decoderSpecificInfo) {
		//warning: we don't know anything about the structure of a generic DecSpecInfo
		//we check the tag and size of the descriptor, but we most ofthe time can't parse it
		//the decSpecInfo is handle as a defaultDescriptor (opaque data, but same structure....)
		e = CalcSize((Descriptor *) dcd->decoderSpecificInfo , &tmpSize);
		if (e) return e;
		*outSize += tmpSize + GetSizeFieldSize(tmpSize);
	}
	e = calcDescListSize(dcd->profileLevelIndicationIndexDescriptor, outSize);
	if (e) return e;
	return M4OK;

}

M4Err WriteDCD(BitStream *bs, DecoderConfigDescriptor *dcd)
{
	M4Err e;
	u32 size;
	if (! dcd) return M4BadParam;

	e = CalcSize((Descriptor *)dcd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, dcd->tag, size);
	if (e) return e;

	BS_WriteInt(bs, dcd->objectTypeIndication, 8);
	BS_WriteInt(bs, dcd->streamType, 6);
	BS_WriteInt(bs, dcd->upstream, 1);
	BS_WriteInt(bs, 1, 1);	//reserved field...
	BS_WriteInt(bs, dcd->bufferSizeDB, 24);
	BS_WriteInt(bs, dcd->maxBitrate, 32);
	BS_WriteInt(bs, dcd->avgBitrate, 32);

	if (dcd->decoderSpecificInfo) {
		e = WriteDesc(bs, (Descriptor *) dcd->decoderSpecificInfo);
		if (e) return e;
	}
	e = writeDescList(bs, dcd->profileLevelIndicationIndexDescriptor);
	return e;
}


Descriptor *NewDefaultDescriptor()
{
	DefaultDescriptor *newDesc = (DefaultDescriptor *) malloc(sizeof(DefaultDescriptor));
	if (!newDesc) return NULL;

	newDesc->dataLength = 0;
	newDesc->data = NULL;
	//set it to the Max allowed
	newDesc->tag = USER_DESC_RANGE_END;
	return (Descriptor *) newDesc;
}

M4Err DelDD(DefaultDescriptor *dd)
{
	if (!dd) return M4BadParam;

	if (dd->data) free(dd->data);
	free(dd);
	return M4OK;
}

M4Err ReadDD(BitStream *bs, DefaultDescriptor *dd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (! dd) return M4BadParam;

	dd->dataLength = DescSize;
	dd->data = NULL;
	if (DescSize) {
		dd->data = (char*)malloc(dd->dataLength);
		if (! dd->data) return M4OutOfMem;
		BS_ReadData(bs, (unsigned char*)dd->data, dd->dataLength);
		nbBytes += dd->dataLength;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeDD(DefaultDescriptor *dd, u32 *outSize)
{
	if (! dd) return M4BadParam;
	*outSize  = dd->dataLength;
	return M4OK;
}

M4Err WriteDD(BitStream *bs, DefaultDescriptor *dd)
{
	M4Err e;
	u32 size;
	if (! dd) return M4BadParam;

	e = CalcSize((Descriptor *)dd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, dd->tag, size);
	if (e) return e;

	if (dd->data) {
		BS_WriteData(bs, (unsigned char*)dd->data, dd->dataLength);
	}
	return M4OK;
}

Descriptor *NewES_ID_Inc()
{
	ES_ID_Inc *newDesc = (ES_ID_Inc *) malloc(sizeof(ES_ID_Inc));
	if (!newDesc) return NULL;
	newDesc->tag = ES_ID_IncTag;
	newDesc->trackID = 0;
	return (Descriptor *) newDesc;
}

M4Err DelES_Inc(ES_ID_Inc *esd_inc)
{
	if (!esd_inc) return M4BadParam;
	free(esd_inc);
	return M4OK;
}
M4Err ReadES_Inc(BitStream *bs, ES_ID_Inc *esd_inc, u32 DescSize)
{
	u32 nbBytes = 0;
	if (! esd_inc) return M4BadParam;

	esd_inc->trackID = BS_ReadInt(bs, 32);
	nbBytes += 4;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}
M4Err SizeES_Inc(ES_ID_Inc *esd_inc, u32 *outSize)
{
	if (! esd_inc) return M4BadParam;
	*outSize = 4;
	return M4OK;
}
M4Err WriteES_Inc(BitStream *bs, ES_ID_Inc *esd_inc)
{
	M4Err e;
	u32 size;
	if (! esd_inc) return M4BadParam;

	e = CalcSize((Descriptor *)esd_inc, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, esd_inc->tag, size);
	if (e) return e;
	BS_WriteInt(bs, esd_inc->trackID, 32);
	return M4OK;
}

Descriptor *NewES_ID_Ref()
{
	ES_ID_Ref *newDesc = (ES_ID_Ref *) malloc(sizeof(ES_ID_Ref));
	if (!newDesc) return NULL;
	newDesc->tag = ES_ID_RefTag;
	newDesc->trackRef = 0;
	return (Descriptor *) newDesc;
}

M4Err DelES_Ref(ES_ID_Ref *esd_ref)
{
	if (!esd_ref) return M4BadParam;
	free(esd_ref);
	return M4OK;
}
M4Err ReadES_Ref(BitStream *bs, ES_ID_Ref *esd_ref, u32 DescSize)
{
	u32 nbBytes = 0;
	if (! esd_ref) return M4BadParam;

	esd_ref->trackRef = BS_ReadInt(bs, 16);
	nbBytes += 2;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeES_Ref(ES_ID_Ref *esd_ref, u32 *outSize)
{
	if (! esd_ref) return M4BadParam;
	*outSize = 2;
	return M4OK;
}
M4Err WriteES_Ref(BitStream *bs, ES_ID_Ref *esd_ref)
{
	M4Err e;
	u32 size;
	if (! esd_ref) return M4BadParam;

	e = CalcSize((Descriptor *)esd_ref, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, esd_ref->tag, size);
	if (e) return e;
	
	BS_WriteInt(bs, esd_ref->trackRef, 16);
	return M4OK;
}

Descriptor *NewExpandedTextualDescriptor()
{
	ExpandedTextualDescriptor *newDesc = (ExpandedTextualDescriptor *) malloc(sizeof(ExpandedTextualDescriptor));
	if (!newDesc) return NULL;

	newDesc->itemDescriptionList = NewChain();
	if (! newDesc->itemDescriptionList) {
		free(newDesc);
		return NULL;
	}
	newDesc->itemTextList = NewChain();
	if (! newDesc->itemTextList) {
		free(newDesc->itemDescriptionList);
		free(newDesc);
		return NULL;
	}
	newDesc->isUTF8 = 0;
	newDesc->langCode = 0;
	newDesc->NonItemText = NULL;
	newDesc->tag = ExpandedTextualDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelET(ExpandedTextualDescriptor *etd)
{
	if (!etd) return M4BadParam;

	while (ChainGetCount(etd->itemDescriptionList)) {
		ETD_ItemText *tmp = (ETD_ItemText*)ChainGetEntry(etd->itemDescriptionList, 0);
		if (tmp) {
			if (tmp->text) free(tmp->text);
			free(tmp);
		}
		ChainDeleteEntry(etd->itemDescriptionList, 0);
	}
	DeleteChain(etd->itemDescriptionList);

	while (ChainGetCount(etd->itemTextList)) {
		ETD_ItemText *tmp = (ETD_ItemText*)ChainGetEntry(etd->itemTextList, 0);
		if (tmp) {
			if (tmp->text) free(tmp->text);
			free(tmp);
		}
		ChainDeleteEntry(etd->itemTextList, 0);
	}
	DeleteChain(etd->itemTextList);

	if (etd->NonItemText) free(etd->NonItemText);
	free(etd);
	return M4OK;
}

M4Err ReadET(BitStream *bs, ExpandedTextualDescriptor *etd, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0;
	u32 i, aligned, len, nonLen, count;
	if (!etd) return M4BadParam;

	etd->langCode = BS_ReadInt(bs, 24);
	etd->isUTF8 = BS_ReadInt(bs, 1);
	aligned = BS_ReadInt(bs, 7);
	count = BS_ReadInt(bs, 8);
	nbBytes += 5;

	for (i = 0; i< count; i++) {
		//description
		ETD_ItemText *description, *Text;
		description = (ETD_ItemText*)malloc(sizeof(ETD_ItemText));
		if (! description) return M4OutOfMem;
		description->text = NULL;
		e = OD_ReadString(bs, & description->text, etd->isUTF8); 
		if (e) return e;
		e = ChainAddEntry(etd->itemDescriptionList, description);
		if (e) return e;
		nbBytes += 1 + strlen(description->text) * (etd->isUTF8 ? 1 : 2);

		//text
		Text = (ETD_ItemText*)malloc(sizeof(ETD_ItemText));
		if (! Text) return M4OutOfMem;
		Text->text = NULL;
		e = OD_ReadString(bs, & Text->text, etd->isUTF8);
		if (e) return e;
		e = ChainAddEntry(etd->itemTextList, Text);
		if (e) return e;
		nbBytes += 1 + strlen(Text->text) * (etd->isUTF8 ? 1 : 2);
	}
	len = BS_ReadInt(bs, 8);
	nbBytes += 1;
	nonLen = 0;
	while (len == 255) {
		nonLen += len;
		len = BS_ReadInt(bs, 8);
		nbBytes += 1;
	}
	nonLen += len;
	if (nonLen) {
		//here we have no choice but do the job ourselves
		//because the length is not encoded on 8 bits
		etd->NonItemText = (unsigned char*)malloc(nonLen * (etd->isUTF8 ? 1 : 2));
		if (! etd->NonItemText) return M4OutOfMem;
		BS_ReadData(bs, etd->NonItemText, nonLen * (etd->isUTF8 ? 1 : 2));
		nbBytes += nonLen * (etd->isUTF8 ? 1 : 2);
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}


M4Err SizeET(ExpandedTextualDescriptor *etd, u32 *outSize)
{
	u32 i, len, nonLen, lentmp;
	if (!etd) return M4BadParam;

	*outSize = 5;
	if (ChainGetCount(etd->itemDescriptionList) != ChainGetCount(etd->itemTextList)) return M4InvalidDescriptor;

	for (i = 0; i < ChainGetCount(etd->itemDescriptionList) ; i++) {
		ETD_ItemText *tmp;
		*outSize += 2;
		tmp = (ETD_ItemText*)ChainGetEntry(etd->itemDescriptionList, i);
		*outSize += strlen(tmp->text) * (etd->isUTF8 ? 1 : 0);
		tmp = (ETD_ItemText*)ChainGetEntry(etd->itemTextList, i);
		*outSize += strlen(tmp->text) * (etd->isUTF8 ? 1 : 0);
	}
	*outSize += 1;
	if (etd->NonItemText) {
		nonLen = (1 + strlen((const char*)etd->NonItemText) * (etd->isUTF8 ? 1 : 2));
	} else {
		nonLen = 0;
	}
	len = 255;
	lentmp = nonLen;
	if (lentmp < 255) { 
		len = lentmp;
	}
	while (len == 255) {
		*outSize += 1;
		lentmp -= 255;
		if (lentmp < 255) { 
			len = lentmp;
		}
	}
	*outSize += nonLen * (etd->isUTF8 ? 1 : 2);
	return M4OK;
}

M4Err WriteET(BitStream *bs, ExpandedTextualDescriptor *etd)
{
	M4Err e;
	u32 size, i, len, nonLen, lentmp;
	ETD_ItemText *tmp;
	if (!etd) return M4BadParam;

	if (ChainGetCount(etd->itemDescriptionList) != ChainGetCount(etd->itemTextList)) return M4InvalidDescriptor;

	e = CalcSize((Descriptor *)etd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, etd->tag, size);
	if (e) return e;

	BS_WriteInt(bs, etd->langCode, 24);
	BS_WriteInt(bs, etd->isUTF8, 1);
	BS_WriteInt(bs, 0, 7);		//aligned
	BS_WriteInt(bs, ChainGetCount(etd->itemDescriptionList), 8);

	for (i = 0; i< ChainGetCount(etd->itemDescriptionList); i++) {
		tmp = (ETD_ItemText*)ChainGetEntry(etd->itemDescriptionList, i);
		OD_WriteString(bs, tmp->text, etd->isUTF8);
		tmp = (ETD_ItemText*)ChainGetEntry(etd->itemTextList, i);
		OD_WriteString(bs, tmp->text, etd->isUTF8);
	}
	if (etd->NonItemText) {
		nonLen = strlen((const char*)etd->NonItemText) + 1;
	} else {
		nonLen = 0;
	}
	lentmp = nonLen;
	len = lentmp < 255 ? lentmp : 255;
	while (len == 255) {
		BS_WriteInt(bs, len, 8);
		lentmp -= len;
		len = lentmp < 255 ? lentmp : 255;
	}
	BS_WriteInt(bs, len, 8);
	BS_WriteData(bs, etd->NonItemText, nonLen * (etd->isUTF8 ? 1 : 2));
	return M4OK;
}

Descriptor *NewExtensionProfileLevelDescriptor()
{
	ExtensionProfileLevelDescriptor *newDesc = (ExtensionProfileLevelDescriptor *) malloc(sizeof(ExtensionProfileLevelDescriptor));
	if (!newDesc) return NULL;
	newDesc->AudioProfileLevelIndication = 0;
	newDesc->GraphicsProfileLevelIndication = 0;
	newDesc->MPEGJProfileLevelIndication = 0;
	newDesc->ODProfileLevelIndication = 0;
	newDesc->profileLevelIndicationIndex = 0;
	newDesc->SceneProfileLevelIndication = 0;
	newDesc->VisualProfileLevelIndication = 0;
	newDesc->tag = ExtensionPL_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelEPL(ExtensionProfileLevelDescriptor *pld)
{
	if (!pld) return M4BadParam;

	free(pld);
	return M4OK;
}

M4Err ReadEPL(BitStream *bs, ExtensionProfileLevelDescriptor *pld, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!pld) return M4BadParam;

	pld->profileLevelIndicationIndex = BS_ReadInt(bs, 8);
	pld->ODProfileLevelIndication = BS_ReadInt(bs, 8);
	pld->SceneProfileLevelIndication = BS_ReadInt(bs, 8);
	pld->AudioProfileLevelIndication = BS_ReadInt(bs, 8);
	pld->VisualProfileLevelIndication = BS_ReadInt(bs, 8);
	pld->GraphicsProfileLevelIndication = BS_ReadInt(bs, 8);
	pld->MPEGJProfileLevelIndication = BS_ReadInt(bs, 8);

	nbBytes += 7;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeEPL(ExtensionProfileLevelDescriptor *pld, u32 *outSize)
{
	if (!pld) return M4BadParam;

	*outSize = 7;
	return M4OK;
}

M4Err WriteEPL(BitStream *bs, ExtensionProfileLevelDescriptor *pld)
{
	M4Err e;
	u32 size;
	if (!pld) return M4BadParam;

	e = CalcSize((Descriptor *)pld, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, pld->tag, size);
	if (e) return e;

	BS_WriteInt(bs, pld->profileLevelIndicationIndex, 8);
	BS_WriteInt(bs, pld->ODProfileLevelIndication, 8);
	BS_WriteInt(bs, pld->SceneProfileLevelIndication, 8);
	BS_WriteInt(bs, pld->AudioProfileLevelIndication, 8);
	BS_WriteInt(bs, pld->VisualProfileLevelIndication, 8);
	BS_WriteInt(bs, pld->GraphicsProfileLevelIndication, 8);
	BS_WriteInt(bs, pld->MPEGJProfileLevelIndication, 8);
	return M4OK;
}

Descriptor *NewSegmentDescriptor()
{
	SegmentDescriptor *newDesc = (SegmentDescriptor *) malloc(sizeof(SegmentDescriptor));
	if (!newDesc) return NULL;

	memset(newDesc, 0, sizeof(SegmentDescriptor));
	newDesc->tag = SegmentDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelSegDesc(SegmentDescriptor *sd)
{
	if (!sd) return M4BadParam;

	if (sd->SegmentName) free(sd->SegmentName);
	free(sd);
	return M4OK;
}

M4Err ReadSegDesc(BitStream *bs, SegmentDescriptor *sd, u32 DescSize)
{
	u32 size, nbBytes = 0;
	if (!sd) return M4BadParam;

	sd->startTime = BS_ReadDouble(bs);
	sd->Duration = BS_ReadDouble(bs);
	nbBytes += 16;
	size = BS_ReadInt(bs, 8);
	nbBytes += 1;
	if (size) {
		sd->SegmentName = (unsigned char*) malloc(sizeof(char)*(size+1));
		if (!sd->SegmentName) return M4OutOfMem;
		BS_ReadData(bs, sd->SegmentName, size);
		sd->SegmentName[size] = 0;
		nbBytes += size;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeSegDesc(SegmentDescriptor *sd, u32 *outSize)
{
	if (!sd) return M4BadParam;
	*outSize = 17;
	if (sd->SegmentName) *outSize += strlen(sd->SegmentName);
	return M4OK;
}

M4Err WriteSegDesc(BitStream *bs, SegmentDescriptor *sd)
{
	M4Err e;
	u32 size;
	if (!sd) return M4BadParam;
	e = CalcSize((Descriptor *)sd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, sd->tag, size);
	if (e) return e;
	BS_WriteDouble(bs, sd->startTime);
	BS_WriteDouble(bs, sd->Duration);
	if (sd->SegmentName) {
		BS_WriteInt(bs, strlen(sd->SegmentName), 8);
		BS_WriteData(bs, sd->SegmentName, strlen(sd->SegmentName));
	} else {
		BS_WriteInt(bs, 0, 8);
	}
	return M4OK;
}
Descriptor *NewMediaTimeDescriptor()
{
	MediaTimeDescriptor *newDesc = (MediaTimeDescriptor *) malloc(sizeof(MediaTimeDescriptor));
	if (!newDesc) return NULL;

	memset(newDesc, 0, sizeof(MediaTimeDescriptor));
	newDesc->tag = MediaTimeDescriptor_Tag;
	return (Descriptor *) newDesc;
}
M4Err DelMediaTimeDesc(MediaTimeDescriptor *mt)
{
	if (!mt) return M4BadParam;
	free(mt);
	return M4OK;
}
M4Err ReadMediaTimeDesc(BitStream *bs, MediaTimeDescriptor *mt, u32 DescSize)
{
	if (!mt) return M4BadParam;
	mt->mediaTimeStamp = BS_ReadDouble(bs);
	return M4OK;
}
M4Err SizeMediaTimeDesc(MediaTimeDescriptor *mt, u32 *outSize)
{
	if (!mt) return M4BadParam;
	*outSize = 8;
	return M4OK;
}
M4Err WriteMediaTimeDesc(BitStream *bs, MediaTimeDescriptor *mt)
{
	M4Err e;
	u32 size;
	if (!mt) return M4BadParam;
	e = CalcSize((Descriptor *)mt, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, mt->tag, size);
	if (e) return e;
	BS_WriteDouble(bs, mt->mediaTimeStamp);
	return M4OK;
}

Descriptor *NewIPIDescriptorPointer()
{
	IPI_DescrPointer *newDesc = (IPI_DescrPointer *) malloc(sizeof(IPI_DescrPointer));
	if (!newDesc) return NULL;
	newDesc->IPI_ES_Id = 0;
	newDesc->tag = IPIPtr_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelIPIP(IPI_DescrPointer *ipid)
{
	if (!ipid) return M4BadParam;
	free(ipid);
	return M4OK;
}

M4Err ReadIPIP(BitStream *bs, IPI_DescrPointer *ipid, u32 DescSize)
{
	u32 nbBytes = 0;
	if (! ipid) return M4BadParam;

	ipid->IPI_ES_Id = BS_ReadInt(bs, 16);
	nbBytes += 2;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeIPIP(IPI_DescrPointer *ipid, u32 *outSize)
{
	if (! ipid) return M4BadParam;
	*outSize = 2;
	return M4OK;
}

M4Err WriteIPIP(BitStream *bs, IPI_DescrPointer *ipid)
{
	M4Err e;
	u32 size;
	if (! ipid) return M4BadParam;
	e = CalcSize((Descriptor *)ipid, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ipid->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ipid->IPI_ES_Id, 16);
	return M4OK;
}

Descriptor *NewIPMPDescriptor()
{
	IPMP_Descriptor *newDesc = (IPMP_Descriptor *) malloc(sizeof(IPMP_Descriptor));
	if (!newDesc) return NULL;

	newDesc->IPMP_DataLength = 0;
	newDesc->URLstring = NULL;
	newDesc->IPMP_Data = NULL;
	newDesc->IPMP_DataLength = 0;
	newDesc->IPMP_DescID = 0;
	newDesc->IPMPSType = 0;
	newDesc->URLstring = NULL;
	newDesc->tag = IPMP_Tag;
	return (Descriptor *) newDesc;
}
M4Err DelIPMP(IPMP_Descriptor *ipmp)
{
	if (!ipmp) return M4BadParam;
	if (ipmp->IPMP_Data) free(ipmp->IPMP_Data);
	if (ipmp->URLstring) free(ipmp->URLstring);
	free(ipmp);
	return M4OK;
}

M4Err ReadIPMP(BitStream *bs, IPMP_Descriptor *ipmp, u32 DescSize)
{
	u32 size, nbBytes = 0;
	if (!ipmp) return M4BadParam;

	ipmp->IPMP_DescID = BS_ReadInt(bs, 8);
	ipmp->IPMPSType  =BS_ReadInt(bs, 16);
	nbBytes += 3;
	size = DescSize - 3;

	if (! ipmp->IPMPSType) {
		ipmp->URLstring = (char*)malloc(size + 1);
		if (! ipmp->URLstring) return M4OutOfMem;
		BS_ReadData(bs, (unsigned char*)ipmp->URLstring, size);
		nbBytes += size;
		ipmp->URLstring[size] = 0;
	} else {
		ipmp->IPMP_DataLength = size;
		ipmp->IPMP_Data = (unsigned char*)malloc(size);
		if (! ipmp->IPMP_Data) return M4OutOfMem;
		BS_ReadData(bs, ipmp->IPMP_Data, size);
		nbBytes += size;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeIPMP(IPMP_Descriptor *ipmp, u32 *outSize)
{
	if (!ipmp) return M4BadParam;

	*outSize = 3;
	if (! ipmp->IPMPSType) {
		*outSize += strlen(ipmp->URLstring);
	} else {
		*outSize += ipmp->IPMP_DataLength;
	}
	return M4OK;
}

M4Err WriteIPMP(BitStream *bs, IPMP_Descriptor *ipmp)
{
	M4Err e;
	u32 size;
	if (!ipmp) return M4BadParam;

	e = CalcSize((Descriptor *)ipmp, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ipmp->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ipmp->IPMP_DescID, 8);
	BS_WriteInt(bs, ipmp->IPMPSType, 16);
	if (! ipmp->IPMPSType) {
		if (! strlen(ipmp->URLstring)) return M4InvalidDescriptor;
		BS_WriteData(bs, (unsigned char*)ipmp->URLstring, strlen(ipmp->URLstring));
	} else {
		BS_WriteData(bs, ipmp->IPMP_Data, ipmp->IPMP_DataLength);
	}
	return M4OK;
}

Descriptor *NewIPMPDescriptorPointer()
{
	IPMP_DescrPointer *newDesc = (IPMP_DescrPointer *) malloc(sizeof(IPMP_DescrPointer));
	if (!newDesc) return NULL;
	newDesc->IPMPDescrID = 0;
	newDesc->tag = IPMPPtr_Tag;
	return (Descriptor *) newDesc;
}
M4Err DelIPMPD(IPMP_DescrPointer *ipmpd)
{
	if (!ipmpd) return M4BadParam;
	free(ipmpd);
	return M4OK;
}

M4Err ReadIPMPD(BitStream *bs, IPMP_DescrPointer *ipmpd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (! ipmpd) return M4BadParam;
	ipmpd->IPMPDescrID = BS_ReadInt(bs, 8);
	nbBytes += 1;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}
M4Err SizeIPMPD(IPMP_DescrPointer *ipmpd, u32 *outSize)
{
	if (! ipmpd) return M4BadParam;
	*outSize = 1;
	return M4OK;
}
M4Err WriteIPMPD(BitStream *bs, IPMP_DescrPointer *ipmpd)
{
	M4Err e;
	u32 size;
	if (! ipmpd) return M4BadParam;

	e = CalcSize((Descriptor *)ipmpd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ipmpd->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ipmpd->IPMPDescrID, 8);
	return M4OK;
}

Descriptor *NewKeyWordDescriptor()
{
	KeyWordDescriptor *newDesc = (KeyWordDescriptor *) malloc(sizeof(KeyWordDescriptor));
	if (!newDesc) return NULL;

	newDesc->keyWordsList = NewChain();
	if (! newDesc->keyWordsList) {
		free(newDesc);
		return NULL;
	}
	newDesc->isUTF8 = 0;
	newDesc->languageCode = 0;
	newDesc->tag = KeyWordDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelKW(KeyWordDescriptor *kwd)
{
	if (!kwd) return M4BadParam;

	while (ChainGetCount(kwd->keyWordsList)) {
		KeyWordItem *tmp = (KeyWordItem*)ChainGetEntry(kwd->keyWordsList, 0);
		if (tmp) {
			if (tmp->keyWord) free(tmp->keyWord);
			free(tmp);
		}
	}
	DeleteChain(kwd->keyWordsList);
	free(kwd);
	return M4OK;
}

M4Err ReadKW(BitStream *bs, KeyWordDescriptor *kwd, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0, aligned, i, kwcount;
	if (!kwd) return M4BadParam;

	kwd->languageCode = BS_ReadInt(bs, 24);
	kwd->isUTF8 = BS_ReadInt(bs, 1);
	aligned = BS_ReadInt(bs, 7);
	kwcount = BS_ReadInt(bs, 8);
	nbBytes += 5;

	for (i = 0 ; i < kwcount; i++) {
		KeyWordItem *tmp = (KeyWordItem*)malloc(sizeof(KeyWordItem));
		if (! tmp) return M4OutOfMem;
		e = OD_ReadString(bs, & tmp->keyWord, kwd->isUTF8);
		if (e) return e;
		e = ChainAddEntry(kwd->keyWordsList, tmp);
		if (e) return e;
		nbBytes  += 1 + strlen(tmp->keyWord) * (kwd->isUTF8 ? 1 : 2);
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}


M4Err SizeKW(KeyWordDescriptor *kwd, u32 *outSize)
{
	u32 i;
	if (!kwd) return M4BadParam;

	*outSize = 5;
	for (i = 0 ; i < ChainGetCount(kwd->keyWordsList); i++) {
		KeyWordItem *tmp = (KeyWordItem*)ChainGetEntry(kwd->keyWordsList, i);
		*outSize += 1 + strlen(tmp->keyWord) * (kwd->isUTF8 ? 1 : 2);
	}
	return M4OK;
}
M4Err WriteKW(BitStream *bs, KeyWordDescriptor *kwd)
{
	M4Err e;
	u32 size, i;
	if (!kwd) return M4BadParam;

	e = CalcSize((Descriptor *)kwd, &size);
	e = writeBaseDescriptor(bs, kwd->tag, size);

	BS_WriteInt(bs, kwd->languageCode, 24);
	BS_WriteInt(bs, kwd->isUTF8, 1);
	BS_WriteInt(bs, 0, 7);		//aligned(8)
	BS_WriteInt(bs, ChainGetCount(kwd->keyWordsList), 8);

	for (i = 0; i< ChainGetCount(kwd->keyWordsList); i++) {
		KeyWordItem *tmp = (KeyWordItem*)ChainGetEntry(kwd->keyWordsList, i);
		if (tmp) {
			OD_WriteString(bs, tmp->keyWord, kwd->isUTF8);
		} 
	}
	return M4OK;
}

Descriptor *NewLanguageDescriptor()
{
	LanguageDescriptor *newDesc = (LanguageDescriptor *) malloc(sizeof(LanguageDescriptor));
	if (!newDesc) return NULL;
	newDesc->langCode = 0;
	newDesc->tag = LanguageDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelLang(LanguageDescriptor *ld)
{
	if (!ld) return M4BadParam;
	free(ld);
	return M4OK;
}

M4Err ReadLang(BitStream *bs, LanguageDescriptor *ld, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!ld) return M4BadParam;

	ld->langCode = BS_ReadInt(bs, 24);
	nbBytes += 3;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeLang(LanguageDescriptor *ld, u32 *outSize)
{
	if (!ld) return M4BadParam;
	*outSize = 3;
	return M4OK;
}

M4Err WriteLang(BitStream *bs, LanguageDescriptor *ld)
{
	M4Err e;
	u32 size;
	if (!ld) return M4BadParam;

	e = CalcSize((Descriptor *)ld, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ld->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ld->langCode, 24);
	return M4OK;
}


Descriptor *NewOCICreationDateDescriptor()
{
	OCICreationDateDescriptor *newDesc = (OCICreationDateDescriptor *) malloc(sizeof(OCICreationDateDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc->OCICreationDate, 0, 5);
	newDesc->tag = OCI_CreationDate_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelOCD(OCICreationDateDescriptor *ocd)
{
	if (!ocd) return M4BadParam;
	free(ocd);
	return M4OK;
}

M4Err ReadOCD(BitStream *bs, OCICreationDateDescriptor *ocd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!ocd) return M4BadParam;

	BS_ReadData(bs, ocd->OCICreationDate, DATE_CODING_BIT_LEN);
	nbBytes += DATE_CODING_BIT_LEN / 8;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeOCD(OCICreationDateDescriptor *ocd, u32 *outSize)
{
	if (!ocd) return M4BadParam;
	*outSize = (DATE_CODING_BIT_LEN / 8);
	return M4OK;
}

M4Err WriteOCD(BitStream *bs, OCICreationDateDescriptor *ocd)
{
	M4Err e;
	u32 size;
	if (!ocd) return M4BadParam;

	e = CalcSize((Descriptor *)ocd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ocd->tag, size);
	if (e) return e;
	BS_WriteData(bs, ocd->OCICreationDate , DATE_CODING_BIT_LEN);
	return M4OK;
}

Descriptor *NewOCICreatorNameDescriptor()
{
	OCICreatorNameDescriptor *newDesc = (OCICreatorNameDescriptor *) malloc(sizeof(OCICreatorNameDescriptor));
	if (!newDesc) return NULL;

	newDesc->OCICreators = NewChain();
	if (! newDesc->OCICreators) {
		free(newDesc);
		return NULL;
	}
	newDesc->tag = OCI_CreatorName_Tag;
	return (Descriptor *) newDesc;
}
M4Err DelOCN(OCICreatorNameDescriptor *ocn)
{
	u32 i;
	if (!ocn) return M4BadParam;
	
	for (i = 0; i < ChainGetCount(ocn->OCICreators); i++) {
		OCICreator *tmp = (OCICreator*)ChainGetEntry(ocn->OCICreators, i);
		if (tmp->OCICreatorName) free(tmp->OCICreatorName);
		free(tmp);
	}
	DeleteChain(ocn->OCICreators);
	free(ocn);
	return M4OK;
}

M4Err ReadOCN(BitStream *bs, OCICreatorNameDescriptor *ocn, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0;
	u32 i, aligned, count;
	if (!ocn) return M4BadParam;

	count = BS_ReadInt(bs, 8);
	nbBytes += 1;
	for (i = 0; i< count; i++) {
		OCICreator *tmp = (OCICreator*)malloc(sizeof(OCICreator));
		if (! tmp) return M4OutOfMem;
		tmp->langCode = BS_ReadInt(bs, 24);
		tmp->isUTF8 = BS_ReadInt(bs, 1);
		aligned = BS_ReadInt(bs, 7);
		nbBytes += 4;
		e = OD_ReadString(bs, & tmp->OCICreatorName, tmp->isUTF8);
		if (e) return e;
		nbBytes += strlen(tmp->OCICreatorName) + 1;
		e = ChainAddEntry(ocn->OCICreators, tmp);
		if (e) return e;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeOCN(OCICreatorNameDescriptor *ocn, u32 *outSize)
{
	u32 i;
	if (!ocn) return M4BadParam;

	*outSize = 1;
	for (i = 0; i< ChainGetCount(ocn->OCICreators) ; i++) {
		OCICreator *tmp = (OCICreator*)ChainGetEntry(ocn->OCICreators, i);
		if (! tmp) return M4InvalidDescriptor;
		*outSize += 5 + strlen(tmp->OCICreatorName) * (tmp->isUTF8 ? 1 : 2);
	}
	return M4OK;
}

M4Err WriteOCN(BitStream *bs, OCICreatorNameDescriptor *ocn)
{
	M4Err e;
	u32 size;
	u32 i;
	if (!ocn) return M4BadParam;

	e = CalcSize((Descriptor *)ocn, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, ocn->tag, size);
	if (e) return e;
	BS_WriteInt(bs, ChainGetCount(ocn->OCICreators), 8);

	for (i = 0; i < ChainGetCount(ocn->OCICreators) ; i++) {
		OCICreator *tmp = (OCICreator*)ChainGetEntry(ocn->OCICreators, i);
		if (! tmp) return M4InvalidDescriptor;
		BS_WriteInt(bs, tmp->langCode, 24);
		BS_WriteInt(bs, tmp->isUTF8, 1);
		BS_WriteInt(bs, 0, 7);		//aligned
		BS_WriteInt(bs, strlen(tmp->OCICreatorName) , 8);
		OD_WriteString(bs, tmp->OCICreatorName, tmp->isUTF8);
	}
	return M4OK;
}


Descriptor *NewProfileLevelIndicationIndexDescriptor()
{
	ProfileLevelIndicationIndexDescriptor *newDesc = (ProfileLevelIndicationIndexDescriptor *) malloc(sizeof(ProfileLevelIndicationIndexDescriptor));
	if (!newDesc) return NULL;
	newDesc->profileLevelIndicationIndex = 0;
	newDesc->tag = PL_IndicationIndex_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelPLII(ProfileLevelIndicationIndexDescriptor *plid)
{
	if (!plid) return M4BadParam;
	free(plid);
	return M4OK;
}

M4Err ReadPLII(BitStream *bs, ProfileLevelIndicationIndexDescriptor *plid, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!plid) return M4BadParam;

	plid->profileLevelIndicationIndex = BS_ReadInt(bs, 8);
	nbBytes += 1;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}
M4Err SizePLII(ProfileLevelIndicationIndexDescriptor *plid, u32 *outSize)
{
	if (!plid) return M4BadParam;
	*outSize = 1;
	return M4OK;
}
M4Err WritePLII(BitStream *bs, ProfileLevelIndicationIndexDescriptor *plid)
{
	M4Err e;
	u32 size;
	if (!plid) return M4BadParam;
	e = CalcSize((Descriptor *)plid, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, plid->tag, size);
	if (e) return e;
	BS_WriteInt(bs, plid->profileLevelIndicationIndex, 8);
	return M4OK;
}


Descriptor *NewRatingDescriptor()
{
	RatingDescriptor *newDesc = (RatingDescriptor *) malloc(sizeof(RatingDescriptor));
	if (!newDesc) return NULL;

	newDesc->infoLength = 0;
	newDesc->ratingInfo = NULL;
	newDesc->ratingCriteria = 0;
	newDesc->ratingEntity = 0;
	newDesc->tag = RatingDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelRat(RatingDescriptor *rd)
{
	if (!rd) return M4BadParam;

	if (rd->ratingInfo) free(rd->ratingInfo);
	free(rd);
	return M4OK;
}

M4Err ReadRat(BitStream *bs, RatingDescriptor *rd, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!rd) return M4BadParam;

	rd->ratingEntity = BS_ReadInt(bs, 32);
	rd->ratingCriteria = BS_ReadInt(bs, 16);
	rd->infoLength = DescSize - 6;
	nbBytes += 6;
	
	rd->ratingInfo = (unsigned char*)malloc(rd->infoLength);
	if (! rd->ratingInfo) return M4OutOfMem;
	BS_ReadData(bs, rd->ratingInfo, rd->infoLength);
	nbBytes += rd->infoLength;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeRat(RatingDescriptor *rd, u32 *outSize)
{
	if (!rd) return M4BadParam;

	*outSize = 6 + rd->infoLength;
	return M4OK;
}

M4Err WriteRat(BitStream *bs, RatingDescriptor *rd)
{
	M4Err e;
	u32 size;
	if (!rd) return M4BadParam;
	e = CalcSize((Descriptor *)rd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, rd->tag, size);
	if (e) return e;
	BS_WriteInt(bs, rd->ratingEntity, 32);
	BS_WriteInt(bs, rd->ratingCriteria, 16);
	BS_WriteData(bs, rd->ratingInfo, rd->infoLength);
	return M4OK;
}


Descriptor *NewRegistrationDescriptor()
{
	RegistrationDescriptor *newDesc = (RegistrationDescriptor *) malloc(sizeof(RegistrationDescriptor));
	if (!newDesc) return NULL;
	newDesc->additionalIdentificationInfo = NULL;
	newDesc->dataLength = 0;
	newDesc->formatIdentifier = 0;
	newDesc->tag = RegistrationDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelReg(RegistrationDescriptor *reg)
{
	if (!reg) return M4BadParam;

	if (reg->additionalIdentificationInfo) free(reg->additionalIdentificationInfo);
	free(reg);
	return M4OK;
}

M4Err ReadReg(BitStream *bs, RegistrationDescriptor *reg, u32 DescSize)
{
	u32 nbBytes = 0;
	if (!reg) return M4BadParam;

	reg->formatIdentifier = BS_ReadInt(bs, 32);
	reg->dataLength = DescSize - 4;
	reg->additionalIdentificationInfo = (unsigned char*)malloc(reg->dataLength);
	if (! reg->additionalIdentificationInfo) return M4OutOfMem;
	BS_ReadData(bs, reg->additionalIdentificationInfo, reg->dataLength);
	nbBytes += reg->dataLength + 4;
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}


M4Err SizeReg(RegistrationDescriptor *reg, u32 *outSize)
{
	if (!reg) return M4BadParam;

	*outSize = 4 + reg->dataLength;
	return M4OK;
}

M4Err WriteReg(BitStream *bs, RegistrationDescriptor *reg)
{
	M4Err e;
	u32 size;
	if (!reg) return M4BadParam;

	e = CalcSize((Descriptor *)reg, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, reg->tag, size);
	if (e) return e;

	BS_WriteInt(bs, reg->formatIdentifier, 32);
	BS_WriteData(bs, reg->additionalIdentificationInfo, reg->dataLength);
	return M4OK;
}

Descriptor *NewShortTextualDescriptor()
{
	ShortTextualDescriptor *newDesc = (ShortTextualDescriptor *) malloc(sizeof(ShortTextualDescriptor));
	if (!newDesc) return NULL;

	newDesc->eventName = NULL;
	newDesc->eventText = NULL;
	newDesc->isUTF8 = 0;
	newDesc->langCode = 0;
	newDesc->tag = ShortTextualDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelST(ShortTextualDescriptor *std)
{
	if (!std) return M4BadParam;

	if (std->eventName) free(std->eventName);
	if (std->eventText) free(std->eventText);
	free(std);
	return M4OK;
}

M4Err ReadST(BitStream *bs, ShortTextualDescriptor *std, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0;
	u8 aligned;
	if (!std) return M4BadParam;

	std->langCode = BS_ReadInt(bs, 24);
	std->isUTF8 = BS_ReadInt(bs, 1);
	aligned = BS_ReadInt(bs, 7);
	nbBytes += 4;

	e = OD_ReadString(bs, & std->eventName, std->isUTF8);
	if (e) return e;
	e = OD_ReadString(bs, & std->eventText, std->isUTF8);
	if (e) return e;
//	nbBytes += ( strlen(std->eventName) + strlen(std->eventText) ) * (std->isUTF8 ? 1 : 2) + 2;
//	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeST(ShortTextualDescriptor *std, u32 *outSize)
{
	if (!std) return M4BadParam;
	*outSize = 5;
	*outSize += (strlen(std->eventName) + strlen(std->eventText)) * (std->isUTF8 ? 1 : 2) + 1;
	return M4OK;
}

M4Err WriteST(BitStream *bs, ShortTextualDescriptor *std)
{
	M4Err e;
	u32 size;
	if (!std) return M4BadParam;

	e = CalcSize((Descriptor *)std, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, std->tag, size);
	if (e) return e;
	BS_WriteInt(bs, std->langCode, 24);
	BS_WriteInt(bs, std->isUTF8, 1);
	BS_WriteInt(bs, 0, 7);
	OD_WriteString(bs, std->eventName, std->isUTF8);
	OD_WriteString(bs, std->eventText, std->isUTF8);
	return M4OK;
}

Descriptor *NewSmpteCameraPositionDescriptor()
{
	SmpteCameraPositionDescriptor *newDesc = (SmpteCameraPositionDescriptor *) malloc(sizeof(SmpteCameraPositionDescriptor));
	if (!newDesc) return NULL;

	newDesc->ParamList = NewChain();
	if (! newDesc->ParamList) {
		free(newDesc);
		return NULL;
	}
	newDesc->cameraID = 0;
	newDesc->tag = SmpteCameraPosition_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelSCP(SmpteCameraPositionDescriptor *cpd)
{
	u32 i;
	if (!cpd) return M4BadParam;

	for (i = 0; i < ChainGetCount(cpd->ParamList); i++) {
		SmpteParam *tmp = (SmpteParam*)ChainGetEntry(cpd->ParamList, i);
		free(tmp);
	}
	DeleteChain(cpd->ParamList);
	free(cpd);
	return M4OK;
}
M4Err ReadSCP(BitStream *bs, SmpteCameraPositionDescriptor *cpd, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0, i, count;
	if (!cpd) return M4BadParam;

	cpd->cameraID = BS_ReadInt(bs, 8);
	count = BS_ReadInt(bs, 8);
	nbBytes += 2;

	for (i=0; i< count ; i++) {
		SmpteParam *tmp = (SmpteParam*)malloc(sizeof(SmpteParam));
		if (! tmp) return M4OutOfMem;
		tmp->paramID = BS_ReadInt(bs, 8);
		tmp->param = BS_ReadInt(bs, 32);
		nbBytes += 5;
		e = ChainAddEntry(cpd->ParamList, tmp);
		if (e) return e;
	}
	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}

M4Err SizeSCP(SmpteCameraPositionDescriptor *cpd, u32 *outSize)
{
	u32 i;
	if (!cpd) return M4BadParam;

	*outSize = 2;
	for (i = 0; i< ChainGetCount(cpd->ParamList) ; i++) {
		*outSize += 5 * ChainGetCount(cpd->ParamList);
	}
	return M4OK;
}

M4Err WriteSCP(BitStream *bs, SmpteCameraPositionDescriptor *cpd)
{
	M4Err e;
	u32 size, i;
	if (!cpd) return M4BadParam;

	e = CalcSize((Descriptor *)cpd, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, cpd->tag, size);
	if (e) return e;
	BS_WriteInt(bs, cpd->cameraID, 8);
	BS_WriteInt(bs, ChainGetCount(cpd->ParamList), 8);

	for (i = 0; i < ChainGetCount(cpd->ParamList) ; i++) {
		SmpteParam *tmp = (SmpteParam*)ChainGetEntry(cpd->ParamList, i);
		if (! tmp) return M4InvalidDescriptor;
		BS_WriteInt(bs, tmp->paramID, 8);
		BS_WriteInt(bs, tmp->param, 32);
	}
	return M4OK;
}

Descriptor *NewSupplementaryContentIdentificationDescriptor()
{
	SupplementaryContentIdentificationDescriptor *newDesc = (SupplementaryContentIdentificationDescriptor *) malloc(sizeof(SupplementaryContentIdentificationDescriptor));
	if (!newDesc) return NULL;
	newDesc->supplContentIdentifierTitle = NULL;
	newDesc->supplContentIdentifierValue  =NULL;
	newDesc->languageCode = 0;
	newDesc->tag = SuppContentIdentification_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelSCI(SupplementaryContentIdentificationDescriptor *scid)
{
	if (!scid) return M4BadParam;

	if (scid->supplContentIdentifierTitle) free(scid->supplContentIdentifierTitle);
	if (scid->supplContentIdentifierValue) free(scid->supplContentIdentifierValue);
	free(scid);
	return M4OK;
}

M4Err ReadSCI(BitStream *bs, SupplementaryContentIdentificationDescriptor *scid, u32 DescSize)
{
	M4Err e;
	u32 nbBytes = 0;
	if (! scid) return M4BadParam;

	scid->languageCode = BS_ReadInt(bs, 24);
	nbBytes += 3;

	e = OD_ReadString(bs, & scid->supplContentIdentifierTitle, 1);
	if (e) return e;
	nbBytes += 1 + strlen(scid->supplContentIdentifierTitle);

	e = OD_ReadString(bs, & scid->supplContentIdentifierValue, 1);
	if (e) return e;
	nbBytes += 1 + strlen(scid->supplContentIdentifierValue);

	if (nbBytes != DescSize) return M4ReadDescriptorFailed;
	return M4OK;
}


M4Err SizeSCI(SupplementaryContentIdentificationDescriptor *scid, u32 *outSize)
{
	if (! scid) return M4BadParam;
	*outSize = 5 + strlen(scid->supplContentIdentifierTitle) + strlen(scid->supplContentIdentifierValue);
	return M4OK;
}
M4Err WriteSCI(BitStream *bs, SupplementaryContentIdentificationDescriptor *scid)
{
	M4Err e;
	u32 size;
	if (! scid) return M4BadParam;
	e = CalcSize((Descriptor *)scid, &size);
	if (e) return e;
	e = writeBaseDescriptor(bs, scid->tag, size);
	if (e) return e;
	BS_WriteInt(bs, scid->languageCode, 24);
	OD_WriteString(bs, scid->supplContentIdentifierTitle, 1);
	OD_WriteString(bs, scid->supplContentIdentifierValue, 1);
	return M4OK;
}


Descriptor *NewMuxInfoDescriptor()
{
	MuxInfoDescriptor *newDesc = (MuxInfoDescriptor *) malloc(sizeof(MuxInfoDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(MuxInfoDescriptor));
	newDesc->tag = MuxInfoDescriptor_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelMuxInfo(MuxInfoDescriptor *mi)
{
	if (!mi) return M4BadParam;
	if (mi->file_name) free(mi->file_name);
	if (mi->streamFormat) free(mi->streamFormat);
	if (mi->textNode) free(mi->textNode);
	if (mi->fontNode) free(mi->fontNode);
	free(mi);
	return M4OK;
}

M4Err ReadMI(BitStream *bs, MuxInfoDescriptor *mi, u32 DescSize)
{
	return M4OK;
}
M4Err SizeMI(MuxInfoDescriptor *mi, u32 *outSize)
{
	*outSize = 0;
	return M4OK;
}
M4Err WriteMI(BitStream *bs, MuxInfoDescriptor *mi)
{
	return M4OK;
}

Descriptor *NewBIFSConfigDescriptor()
{
	BIFSConfigDescriptor *newDesc = (BIFSConfigDescriptor *) malloc(sizeof(BIFSConfigDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(BIFSConfigDescriptor));
	newDesc->tag = BIFSConfig_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelBIFSConfig(BIFSConfigDescriptor *desc)
{
	free(desc);
	return M4OK;
}

Descriptor *NewUIConfigDescriptor()
{
	UIConfigDescriptor *newDesc = (UIConfigDescriptor *) malloc(sizeof(UIConfigDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(UIConfigDescriptor));
	newDesc->tag = UIConfig_Tag;
	return (Descriptor *) newDesc;
}

M4Err DelUIConfig(UIConfigDescriptor *desc)
{
	if (desc->deviceName) free(desc->deviceName);
	if (desc->ui_data) free(desc->ui_data);
	free(desc);
	return M4OK;
}
