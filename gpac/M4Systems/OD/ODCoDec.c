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

/************************************************************
		Object Descriptor Codec Functions
************************************************************/
//our main CODEC object
//the ODCodec handles everything internally: 
typedef struct tagODCoDec
{
	//the bitstream where we put our encoded commands
	BitStream *bs;
	//the command list
	Chain *CommandList;
	//the mode (read / write)
	u32 od_mode;
} ODCoDec;


LPODCODEC OD_NewCodec(u32 mode)
{
	LPODCODEC codec;
	Chain *comList;


	if ( (mode != OD_READ) && (mode != OD_WRITE)) return NULL;
	comList = NewChain();
	if (!comList) return NULL;
	
	codec = (LPODCODEC) malloc(sizeof(ODCoDec));
	if (!codec) {
		DeleteChain(comList);
		return NULL;
	}
	//the bitstream is always NULL. It is created on the fly for access unit processing only
	codec->bs = NULL;
	codec->CommandList = comList;
	codec->od_mode = mode;
	return codec;
}

void OD_DeleteCodec(LPODCODEC codec)
{
	if (!codec) return;
	if (ChainGetCount(codec->CommandList)) {
		//empty our command list
	}
	DeleteChain(codec->CommandList);
	if (codec->bs) DeleteBitStream(codec->bs);
	free(codec);
}


/************************************************************
		Codec Encoder Functions
************************************************************/

M4Err OD_AddCommand(LPODCODEC codec, ODCommand *command)
{
	if (!codec || !command) return M4BadParam;
	return ChainAddEntry(codec->CommandList, command);
}

M4Err OD_EncodeAU(LPODCODEC codec)
{
	ODCommand *com;
	M4Err e;

	if (!codec || (codec->od_mode != OD_WRITE)) return M4BadParam;
	
	//check our bitstream: if existing, this means the previous encoded AU was not retrieved
	//we DON'T allow that
	if (codec->bs) return M4BadParam;
	codec->bs = NewBitStream(NULL, 0, BS_WRITE);
	if (!codec->bs) return M4OutOfMem;

	//encode each command and delete them from the list
	while (ChainGetCount(codec->CommandList)) {
		com = (ODCommand*)ChainGetEntry(codec->CommandList, 0);
		e = WriteCom(codec->bs, com); if (e) goto err_exit;
		//don't forget OD Commands are aligned...
		BS_Align(codec->bs);
		DelCom(com);
		ChainDeleteEntry(codec->CommandList, 0);
	}
	//resize our bitstream...
	BS_CutBuffer(codec->bs);
	return M4OK;

//if an error occurs, delete the BitStream and empty the codec
err_exit:
	DeleteBitStream(codec->bs);
	codec->bs = NULL;
	while (ChainGetCount(codec->CommandList)) {
		com = (ODCommand*)ChainGetEntry(codec->CommandList, 0);
		DelCom(com);
		ChainDeleteEntry(codec->CommandList, 0);
	}
	return e;
}

M4Err OD_GetEncodedAU(LPODCODEC codec, char **outAU, u32 *au_length)
{
	if (!codec || !codec->bs || (codec->od_mode != OD_WRITE) || !outAU || *outAU) return M4BadParam;
	BS_GetContent(codec->bs, (unsigned char**)outAU, au_length);
	DeleteBitStream(codec->bs);
	codec->bs = NULL;
	return M4OK;
}



/************************************************************
		Codec Decoder Functions
************************************************************/

M4Err OD_SetBuffer(LPODCODEC codec, char *au, u32 au_length)
{
	if (!codec || (codec->od_mode != OD_READ) ) return M4BadParam;
	if (!au || !au_length) return M4OK;

	//if the command list is not empty, this is an error
	if (ChainGetCount(codec->CommandList)) return M4BadParam;

	//the bitStream should not be here
	if (codec->bs) return M4BadParam;
	
	codec->bs = NewBitStream((unsigned char*)au, (u64) au_length, (unsigned char)BS_READ);
	if (!codec->bs) return M4OutOfMem;
	return M4OK;	
}


M4Err OD_DecodeAU(LPODCODEC codec)
{
	M4Err e = M4OK;
	u32 size = 0, comSize, bufSize;
	ODCommand *com;

	if (!codec || !codec->bs) return M4BadParam;

	bufSize = (u32) BS_Available(codec->bs);
	while (size < bufSize) {
		e =	ParseCommand(codec->bs, &com, &comSize); if (e) goto err_exit;
		ChainAddEntry(codec->CommandList, com);
		size += comSize + GetSizeFieldSize(comSize);
		//OD Commands are aligned
		BS_Align(codec->bs);
	}
	//then delete our bitstream
	DeleteBitStream(codec->bs);
	codec->bs = NULL;
	if (size != bufSize) {
		e = M4ReadODCommandFailed;
		goto err_exit;
	}
	return e;

err_exit:
	if (codec->bs) {
		DeleteBitStream(codec->bs);
		codec->bs = NULL;
	}
	while (ChainGetCount(codec->CommandList)) {
		com = (ODCommand*)ChainGetEntry(codec->CommandList, 0);
		DelCom(com);
		ChainDeleteEntry(codec->CommandList, 0);
	}
	return e;
}

//get the first command in the codec and remove the entry
ODCommand *OD_GetCommand(LPODCODEC codec)
{
	ODCommand *com;
	if (!codec || codec->bs) return NULL;
	com = (ODCommand*)ChainGetEntry(codec->CommandList, 0);
	if (com) ChainDeleteEntry(codec->CommandList, 0);
	return com;
}



/************************************************************
		OD Commands Functions
************************************************************/

//some easy way to get an OD ODCommand...
ODCommand *OD_NewCommand(u8 tag)
{
	ODCommand *newcom;

	newcom = CreateCom(tag);
	newcom->tag = tag;
	return (ODCommand *)newcom;
}

//	... and to delete it
M4Err OD_DeleteCommand(ODCommand **com)
{
	M4Err e;
	e = DelCom(*com);
	*com = NULL;
	return e;
}



/************************************************************
		Object Descriptors Functions
************************************************************/

//some easy way to get an mpeg4 descriptor ...
Descriptor *OD_NewDescriptor(u8 tag)
{
	Descriptor *newdesc;
	newdesc = CreateDescriptor(tag);
	newdesc->tag = tag;
	return (Descriptor *)newdesc;
}

//	... and to delete it
M4Err OD_DeleteDescriptor(Descriptor **desc)
{
	M4Err e = M4OK;
	if (*desc)	e = DelDesc(*desc);
	*desc = NULL;
	return e;
}

//use this function to decode a standalone descriptor
//the desc MUST be formatted with tag and size field!!!
M4Err OD_ReadDesc(char *raw_desc, u32 descSize, Descriptor * *outDesc)
{
	M4Err e;
	u32 size;
	BitStream *bs;

	if (!raw_desc || !descSize) return M4BadParam;

	bs = NewBitStream((unsigned char*)raw_desc, (u64) descSize, (unsigned char)BS_READ);
	if (!bs) return M4OutOfMem;

	size = 0;
	e = ParseDescriptor(bs, outDesc, &size);
	//the size dosn't have the header in it
	size += GetSizeFieldSize(size);
/*
	if (size != descSize) {
		if (*outDesc) DelDesc(*outDesc);
		*outDesc = NULL;
		e = M4ReadDescriptorFailed;
	}
*/

	DeleteBitStream(bs);
	return e;
}

//use this function to encode a standalone descriptor
//the desc will be formatted with tag and size field
M4Err OD_EncDesc(Descriptor *desc, char **outEncDesc, u32 *outSize)
{
	M4Err e;
	BitStream *bs;

	if (!desc || !outEncDesc || !outSize) return M4BadParam;

	*outEncDesc = NULL;
	*outSize = 0;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	if (!bs) return M4OutOfMem;
	//then encode our desc...
	e = WriteDesc(bs, desc);
	if (e) {
		DeleteBitStream(bs);
		return e;
	}
	//then get the content from our bitstream
	BS_GetContent(bs, (unsigned char**)outEncDesc, outSize);
	DeleteBitStream(bs);
	return M4OK;
}

//use this function to get the size of a standalone descriptor
u32 OD_GetDescSize(Descriptor *desc)
{
	u32 descSize;
	M4Err e;
	
	if (!desc) return M4BadParam;
	//get the descriptor length
	e = CalcSize(desc, &descSize);
	if (e) return 0;
	//add the header length
	descSize += GetSizeFieldSize(descSize);
	return descSize;

}

//this is usefull to duplicate on the fly a descriptor (mainly for authoring purposes) 
M4Err OD_DuplicateDescriptor(Descriptor *inDesc, Descriptor **outDesc)
{
	M4Err e;
	char *desc;
	u32 size;
	
	//warning: here we get some data allocated
	e = OD_EncDesc(inDesc, &desc, &size);
	if (e) return e;
	e = OD_ReadDesc(desc, size, outDesc);
	free(desc);
	return e;
}


/************************************************************
		Object Descriptors Edit Functions
************************************************************/

//This functions handles internally what desc can be added to another desc
//and adds it. NO DUPLICATION of the descriptor, so
//once a desc is added to its parent, destroying the parent WILL destroy this desc
M4Err OD_AddDescToDesc(Descriptor *parentDesc, Descriptor *newDesc)
{
	DecoderConfigDescriptor *dcd;

	//our ADD definition
	M4Err AddDescriptorToOD(ObjectDescriptor *od, Descriptor *desc);
	M4Err AddDescriptorToIOD(InitialObjectDescriptor *iod, Descriptor *desc);
	M4Err AddDescriptorToESD(ESDescriptor *esd, Descriptor *desc);
	M4Err AddDescriptorToM4_IOD(M4F_InitialObjectDescriptor *iod, Descriptor *desc);
	M4Err AddDescriptorToM4_OD(M4F_ObjectDescriptor *od, Descriptor *desc);
	
	if (!parentDesc || !newDesc) return M4BadParam;

	switch (parentDesc->tag) {
	//these are container descriptors
	case ObjectDescriptor_Tag:
		return AddDescriptorToOD((ObjectDescriptor *)parentDesc, newDesc);
	case InitialObjectDescriptor_Tag:
		return AddDescriptorToIOD((InitialObjectDescriptor *)parentDesc, newDesc);
	case ESDescriptor_Tag:
		return AddDescriptorToESD((ESDescriptor *)parentDesc, newDesc);
	case DecoderConfigDescriptor_Tag:
		dcd = (DecoderConfigDescriptor *)parentDesc;
		if ((newDesc->tag == DecoderSpecificInfo_Tag) 
			|| (newDesc->tag == BIFSConfig_Tag)
			|| (newDesc->tag == UIConfig_Tag)
			|| (newDesc->tag == TextConfig_Tag)
			) {
			if (dcd->decoderSpecificInfo) return M4DescriptorNotAllowed;
			dcd->decoderSpecificInfo = (DefaultDescriptor *) newDesc;
			return M4OK;
		} else if (newDesc->tag == ExtensionPL_Tag) {
			return ChainAddEntry(dcd->profileLevelIndicationIndexDescriptor, newDesc);
		}
		return M4DescriptorNotAllowed;

	case TextConfig_Tag:
		if (newDesc->tag != TextSampleDescriptor_Tag) return M4DescriptorNotAllowed;
		return ChainAddEntry(((TextConfigDescriptor *)parentDesc)->sample_descriptions, newDesc);

	case QoS_Tag:
		//tricky: the QoS doesnot accept a descriptor but a qualifier. 
		//We have another function for that...
		return M4BadParam;

	//MP4 File Format tags
	case MP4_IOD_Tag:
		return AddDescriptorToM4_IOD((M4F_InitialObjectDescriptor *)parentDesc, newDesc);
	case MP4_OD_Tag:
		return AddDescriptorToM4_OD((M4F_ObjectDescriptor *)parentDesc, newDesc);

	default:
		//signal a different error in case the parent is a non-standard descriptors (user private)
		if ( (parentDesc->tag >= USER_DESC_RANGE_BEGIN) && (parentDesc->tag <= USER_DESC_RANGE_END) ) {
			return M4PrivateDescriptorUnknown;
		}
		return M4DescriptorNotAllowed;
	}
}



/************************************************************
		QoSQualifiers Functions
************************************************************/

QoS_Default *OD_NewQoSQualifier(u8 tag)
{

	QoS_Default *NewQoS(u8 tag);

	QoS_Default *qos;

	qos = NewQoS(tag);
	return qos;
}

M4Err OD_DeleteQoSQualifier(QoS_Default **qos)
{
	if (*qos) DelQoS(*qos);
	*qos = NULL;
	return M4OK;
}


//same function, but for QoS, as a Qualifier IS NOT a descriptor
M4Err OD_AddQualifToQoS(QoS_Descriptor *desc, QoS_Default *qualif)
{
	u32 i;
	QoS_Default *def;

	if (desc->tag != QoS_Tag) return M4BadParam;
	if (desc->predefined) return M4DescriptorNotAllowed;

	for (i = 0; i<ChainGetCount(desc->QoS_Qualifiers); i++) {
		def = (QoS_Default*)ChainGetEntry(desc->QoS_Qualifiers, i);
		//if same Qualifier, not allowed...
		if (def->tag == qualif->tag) return M4DescriptorNotAllowed;
	}
	return ChainAddEntry(desc->QoS_Qualifiers, qualif);
}




/*****************************************************************************************
		Since IPMP V2, we introduce a new set of functions to read / write a list of 
	descriptors that have no containers (a bit like an OD command, but for descriptors)
		This is usefull for IPMPv2 DecoderSpecificInfo which contains a set of 
	IPMP_Declarators.
		As it could be used for other purposes we keep it generic
	You must create the list yourself, the functions just encode/decode from/to the list
*****************************************************************************************/

M4Err OD_ReadDescList(char *raw_list, u32 raw_size, Chain *descList)
{
	BitStream *bs;
	u32 size, desc_size;
	Descriptor *desc;
	M4Err e = M4OK;

	if (!descList || !raw_list || !raw_size) return M4BadParam;

	bs = NewBitStream(raw_list, raw_size, BS_READ);
	if (!bs) return M4OutOfMem;

	size = 0;
	while (size < raw_size) {
		e =	ParseDescriptor(bs, &desc, &desc_size); 
		if (e) goto exit;
		ChainAddEntry(descList, desc);
		size += desc_size + GetSizeFieldSize(desc_size);
	}

exit:
	//then delete our bitstream
	DeleteBitStream(bs);
	if (size != raw_size) e = M4ReadDescriptorFailed;
	return e;
}


M4Err OD_EncDescList(Chain *descList, char **outEncList, u32 *outSize)
{
	BitStream *bs;
	M4Err e;

	if (!descList || !outEncList || *outEncList || !outSize) return M4BadParam;

	*outSize = 0;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	if (!bs) return M4OutOfMem;

	e = writeDescList(bs, descList);
	if (e) {
		DeleteBitStream(bs);
		return e;
	}

	BS_GetContent(bs, (unsigned char**) outEncList, outSize);
	DeleteBitStream(bs);
	return M4OK;
}

M4Err OD_SizeDescList(Chain *descList, u32 *outSize)
{
	return calcDescListSize(descList, outSize);
}

//this functions will destroy the descriptors in a list but not the list
M4Err OD_DeleteDescList(Chain *descList)
{
	M4Err e;
	Descriptor *tmp;

	if (! descList) return M4BadParam;

	while (ChainGetCount(descList)) {
		tmp = (Descriptor*)ChainGetEntry(descList, 0);
		ChainDeleteEntry(descList, 0);
		e = DelDesc(tmp);
		if (e) return e;
	}
	return M4OK;
}



ESDescriptor *OD_NewESDescriptor(u32 sl_predefined)
{
	ESDescriptor *esd;
	esd = (ESDescriptor *) OD_NewDescriptor(ESDescriptor_Tag);
	esd->decoderConfig = (DecoderConfigDescriptor *) OD_NewDescriptor(DecoderConfigDescriptor_Tag);
	esd->decoderConfig->decoderSpecificInfo = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
	esd->slConfig = (SLConfigDescriptor *) NewSLConfigDescriptor((u8) sl_predefined);
	return esd;
}

