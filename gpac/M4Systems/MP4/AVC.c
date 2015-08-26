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

#include <gpac/intern/m4_isomedia_dev.h>


void DeleteAVCConfig(AVCDecoderConfigurationRecord *cfg)
{
	u32 i;
	if (cfg->sequenceParameterSets) {
		for (i=0; i<cfg->numSequenceParameterSets; i++) {
			if (cfg->sequenceParameterSets[i].data) free(cfg->sequenceParameterSets[i].data);
		}
		free(cfg->sequenceParameterSets);
	}
	if (cfg->pictureParameterSets) {
		for (i=0; i<cfg->numPictureParameterSets; i++) {
			if (cfg->pictureParameterSets[i].data) free(cfg->pictureParameterSets[i].data);
		}
		free(cfg->pictureParameterSets);
	}
	free(cfg);
}

M4Err AVC_WriteDSI(AVCDecoderConfigurationRecord *cfg, char **outData, u32 *outSize)
{
	u32 i;
	BitStream *bs = NewBitStream(NULL, 0, BS_WRITE);
	BS_WriteInt(bs, cfg->configurationVersion, 8);
	BS_WriteInt(bs, cfg->AVCProfileIndication , 8);
	BS_WriteInt(bs, cfg->profile_compatibility, 8);
	BS_WriteInt(bs, cfg->AVCProfileIndication, 8);
	BS_WriteInt(bs, 0x3F, 6);
	BS_WriteInt(bs, cfg->nal_unit_size - 1, 2);
	BS_WriteInt(bs, 0x7, 3);
	BS_WriteInt(bs, cfg->numSequenceParameterSets, 5);
	for (i=0; i<cfg->numSequenceParameterSets; i++) {
		BS_WriteInt(bs, cfg->sequenceParameterSets[i].size, 16);
		BS_WriteData(bs, cfg->sequenceParameterSets[i].data, cfg->sequenceParameterSets[i].size);
	}
	BS_WriteInt(bs, cfg->numPictureParameterSets, 8);
	for (i=0; i<cfg->numPictureParameterSets; i++) {
		BS_WriteInt(bs, cfg->pictureParameterSets[i].size, 16);
		BS_WriteData(bs, cfg->pictureParameterSets[i].data, cfg->pictureParameterSets[i].size);
	}
	*outSize = 0;
	*outData = NULL;
	BS_GetContent(bs, (unsigned char **) outData, outSize);
	DeleteBitStream(bs);
	return M4OK;
}

void AVC_RewriteESDescriptor(AVCSampleEntryAtom *avc)
{
	if (avc->esd) OD_DeleteDescriptor((Descriptor **)&avc->esd);
	avc->esd = OD_NewESDescriptor(2);
	avc->esd->decoderConfig->streamType = M4ST_VISUAL;
	/*AVC OTI is 0x21, AVC parameter set stream OTI (not supported in gpac) is 0x22*/
	avc->esd->decoderConfig->objectTypeIndication = 0x21;
	if (avc->bitrate) {
		avc->esd->decoderConfig->bufferSizeDB = avc->bitrate->bufferSizeDB;
		avc->esd->decoderConfig->avgBitrate = avc->bitrate->avgBitrate;
		avc->esd->decoderConfig->maxBitrate = avc->bitrate->maxBitrate;
	}
	if (avc->descr) {
		u32 i; 
		for (i=0; i<ChainGetCount(avc->descr->descriptors); i++) {
			Descriptor *clone;
			Descriptor *desc = ChainGetEntry(avc->descr->descriptors, i);
			clone = NULL;
			OD_DuplicateDescriptor(desc, &clone);
			ChainAddEntry(avc->esd->extensionDescriptors, clone);
		}
	}
	if (avc->avc_config) {
		AVC_WriteDSI(avc->avc_config->config, &avc->esd->decoderConfig->decoderSpecificInfo->data, &avc->esd->decoderConfig->decoderSpecificInfo->dataLength);
	}
}


void btrt_del(Atom *s)
{
	MPEG4BitRateAtom *ptr = (MPEG4BitRateAtom *)s;
	if (ptr) free(ptr);
}
M4Err btrt_Read(Atom *s, BitStream *bs, u64 *read)
{
	MPEG4BitRateAtom *ptr = (MPEG4BitRateAtom *)s;
	ptr->bufferSizeDB = BS_ReadInt(bs, 32);
	ptr->maxBitrate = BS_ReadInt(bs, 32);
	ptr->avgBitrate = BS_ReadInt(bs, 32);
	*read += 12;
	return (*read != ptr->size) ? M4ReadAtomFailed : M4OK;
}
Atom *btrt_New()
{
	MPEG4BitRateAtom *tmp = (MPEG4BitRateAtom *) malloc(sizeof(MPEG4BitRateAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MPEG4BitRateAtom));
	tmp->type = MPEG4BitRateAtomType;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY
M4Err btrt_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	MPEG4BitRateAtom *ptr = (MPEG4BitRateAtom *) s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	BS_WriteInt(bs, ptr->bufferSizeDB, 32);
	BS_WriteInt(bs, ptr->maxBitrate, 32);
	BS_WriteInt(bs, ptr->avgBitrate, 32);
	return M4OK;
}
M4Err btrt_Size(Atom *s)
{
	M4Err e;
	MPEG4BitRateAtom *ptr = (MPEG4BitRateAtom *)s;
	e = Atom_Size(s);
	ptr->size += 12;
	return e;
}
#endif



void m4ds_del(Atom *s)
{
	MPEG4ExtensionDescriptorsAtom *ptr = (MPEG4ExtensionDescriptorsAtom *)s;
	OD_DeleteDescList(ptr->descriptors);
	DeleteChain(ptr->descriptors);
	free(ptr);
}
M4Err m4ds_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	char *enc_od;
	MPEG4ExtensionDescriptorsAtom *ptr = (MPEG4ExtensionDescriptorsAtom *)s;
	u32 od_size = (u32) (ptr->size - *read);
	if (!od_size) return M4OK;
	enc_od = malloc(sizeof(char) * od_size);
	BS_ReadData(bs, enc_od, od_size);
	e = OD_ReadDescList(enc_od, od_size, ptr->descriptors);
	free(enc_od);
	*read = ptr->size;
	return e;
}
Atom *m4ds_New()
{
	MPEG4ExtensionDescriptorsAtom *tmp = (MPEG4ExtensionDescriptorsAtom *) malloc(sizeof(MPEG4ExtensionDescriptorsAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(MPEG4ExtensionDescriptorsAtom));
	tmp->type = MPEG4ExtensionDescriptorsAtomType;
	tmp->descriptors = NewChain();
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY
M4Err m4ds_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	char *enc_ods;
	u32 enc_od_size;
	MPEG4ExtensionDescriptorsAtom *ptr = (MPEG4ExtensionDescriptorsAtom *) s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	enc_ods = NULL;
	enc_od_size = 0;
	e = OD_EncDescList(ptr->descriptors, &enc_ods, &enc_od_size);
	if (e) return e;
	if (enc_od_size) {
		BS_WriteData(bs, enc_ods, enc_od_size);
		free(enc_ods);
	}
	return M4OK;
}
M4Err m4ds_Size(Atom *s)
{
	M4Err e;
	u32 descSize;
	MPEG4ExtensionDescriptorsAtom *ptr = (MPEG4ExtensionDescriptorsAtom *)s;
	e = Atom_Size(s);
	if (!e) e = OD_SizeDescList(ptr->descriptors, &descSize);
	ptr->size += descSize;
	return e;
}
#endif



void avcc_del(Atom *s)
{
	AVCConfigurationAtom *ptr = (AVCConfigurationAtom *)s;
	if (ptr->config) DeleteAVCConfig(ptr->config);
	free(ptr);
}
M4Err avcc_Read(Atom *s, BitStream *bs, u64 *read)
{
	u32 size;
	char *data;
	u32 i, count;
	AVCConfigurationAtom *ptr = (AVCConfigurationAtom *)s;

	if (ptr->config) DeleteAVCConfig(ptr->config);
	SAFEALLOC(ptr->config, sizeof(AVCDecoderConfigurationRecord));
	ptr->config->configurationVersion = BS_ReadInt(bs, 8);
	ptr->config->AVCProfileIndication = BS_ReadInt(bs, 8);
	ptr->config->profile_compatibility = BS_ReadInt(bs, 8);
	ptr->config->AVCLevelIndication = BS_ReadInt(bs, 8);
	BS_ReadInt(bs, 6);
	ptr->config->nal_unit_size = 1 + BS_ReadInt(bs, 2);
	BS_ReadInt(bs, 3);
	count = ptr->config->numSequenceParameterSets = BS_ReadInt(bs, 5);
	*read += 6;

	ptr->config->sequenceParameterSets = malloc(sizeof(AVCConfigSlot)*count);
	for (i=0; i<count; i++) {
		size = BS_ReadInt(bs, 16);
		data = malloc(sizeof(char) * size);
		BS_ReadData(bs, data, size);
		ptr->config->sequenceParameterSets[i].size = size;
		ptr->config->sequenceParameterSets[i].data = data;
		*read += 2+size;
	}

	count = ptr->config->numPictureParameterSets = BS_ReadInt(bs, 8);
	*read += 1;
	ptr->config->pictureParameterSets = malloc(sizeof(AVCConfigSlot)*count);
	for (i=0; i<count; i++) {
		size = BS_ReadInt(bs, 16);
		data = malloc(sizeof(char) * size);
		BS_ReadData(bs, data, size);
		ptr->config->pictureParameterSets[i].size = size;
		ptr->config->pictureParameterSets[i].data = data;
		*read += 2+size;
	}
	/*"Readers should be prepared to ignore unrecognised data beyond the definition of the data they understand"*/
	if (*read < ptr->size) {
		BS_ReadInt(bs, (u32) (8*(ptr->size - *read)) );
		*read = ptr->size;
	}
	return (*read != ptr->size) ? M4ReadAtomFailed : M4OK;
}
Atom *avcc_New()
{
	AVCConfigurationAtom *tmp = (AVCConfigurationAtom *) malloc(sizeof(MPEG4BitRateAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(AVCConfigurationAtom));
	tmp->type = AVCConfigurationAtomType;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY
M4Err avcc_Write(Atom *s, BitStream *bs)
{
	u32 i;
	M4Err e;
	AVCConfigurationAtom *ptr = (AVCConfigurationAtom *) s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;
	if (!ptr->config) return M4OK;

	BS_WriteInt(bs, ptr->config->configurationVersion, 8);
	BS_WriteInt(bs, ptr->config->AVCProfileIndication , 8);
	BS_WriteInt(bs, ptr->config->profile_compatibility, 8);
	BS_WriteInt(bs, ptr->config->AVCProfileIndication, 8);
	BS_WriteInt(bs, 0x3F, 6);
	BS_WriteInt(bs, ptr->config->nal_unit_size - 1, 2);
	BS_WriteInt(bs, 0x7, 3);
	BS_WriteInt(bs, ptr->config->numSequenceParameterSets, 5);
	for (i=0; i<ptr->config->numSequenceParameterSets; i++) {
		BS_WriteInt(bs, ptr->config->sequenceParameterSets[i].size, 16);
		BS_WriteData(bs, ptr->config->sequenceParameterSets[i].data, ptr->config->sequenceParameterSets[i].size);
	}

	BS_WriteInt(bs, ptr->config->numPictureParameterSets, 8);
	for (i=0; i<ptr->config->numPictureParameterSets; i++) {
		BS_WriteInt(bs, ptr->config->pictureParameterSets[i].size, 16);
		BS_WriteData(bs, ptr->config->pictureParameterSets[i].data, ptr->config->pictureParameterSets[i].size);
	}
	return M4OK;
}
M4Err avcc_Size(Atom *s)
{
	M4Err e;
	u32 i;
	AVCConfigurationAtom *ptr = (AVCConfigurationAtom *)s;
	e = Atom_Size(s);
	if (e || !ptr->config) return e;
	ptr->size += 7;
	for (i=0; i<ptr->config->numSequenceParameterSets; i++) ptr->size += 2 + ptr->config->sequenceParameterSets[i].size;
	for (i=0; i<ptr->config->numPictureParameterSets; i++) ptr->size += 2 + ptr->config->pictureParameterSets[i].size;
	return M4OK;
}
#endif



void avc1_del(Atom *s)
{
	AVCSampleEntryAtom *ptr = (AVCSampleEntryAtom *)s;
	if (ptr == NULL) return;
	if (ptr->avc_config) DelAtom((Atom *) ptr->avc_config);
	if (ptr->bitrate) DelAtom((Atom *) ptr->bitrate);
	if (ptr->descr) DelAtom((Atom *) ptr->descr);
	/*for publishing*/
	if (ptr->slc) OD_DeleteDescriptor((Descriptor **)&ptr->slc);
	if (ptr->esd) OD_DeleteDescriptor((Descriptor **)&ptr->esd);
	free(ptr);
}

M4Err avc1_AddAtom(AVCSampleEntryAtom *ptr, Atom *a)
{
	if (!a) return M4OK;
	switch (a->type) {
	case AVCConfigurationAtomType:
		if (ptr->avc_config) return M4InvalidAtom;
		ptr->avc_config = (AVCConfigurationAtom *)a;
		break;
	case MPEG4BitRateAtomType:
		if (ptr->bitrate) return M4InvalidAtom;
		ptr->bitrate = (MPEG4BitRateAtom *)a;
		break;
	case MPEG4ExtensionDescriptorsAtomType:
		if (ptr->descr) return M4InvalidAtom;
		ptr->descr = (MPEG4ExtensionDescriptorsAtom *)a;
		break;
	default:
		DelAtom(a);
	}
	return M4OK;
}
M4Err avc1_Read(Atom *s, BitStream *bs, u64 *read)
{
	M4Err e;
	u64 sr;
	Atom *a;
	AVCSampleEntryAtom *ptr = (AVCSampleEntryAtom *)s;

	ReadVideoSampleEntry((VisualSampleEntryAtom *)ptr, bs, read);
	
	while (*read < ptr->size) {
		e = ParseAtom(&a, bs, &sr);
		if (e) return e;
		*read += a->size;
		e = avc1_AddAtom(ptr, a);
		if (e) return e;
	}
	AVC_RewriteESDescriptor(ptr);
	return (*read != ptr->size) ? M4ReadAtomFailed : M4OK;
}
Atom *avc1_New()
{
	AVCSampleEntryAtom *tmp = (AVCSampleEntryAtom *) malloc(sizeof(AVCSampleEntryAtom));
	if (tmp == NULL) return NULL;
	memset(tmp, 0, sizeof(AVCSampleEntryAtom));

	InitVideoSampleEntry((VisualSampleEntryAtom *)tmp);
	tmp->type = AVCSampleEntryAtomType;
	return (Atom *)tmp;
}

#ifndef M4_READ_ONLY
M4Err avc1_Write(Atom *s, BitStream *bs)
{
	M4Err e;
	AVCSampleEntryAtom *ptr = (AVCSampleEntryAtom *)s;
	if (!s) return M4BadParam;
	e = Atom_Write(s, bs);
	if (e) return e;

	WriteVideoSampleEntry((VisualSampleEntryAtom *)s, bs);
	if (ptr->avc_config) {
		e = WriteAtom((Atom *) ptr->avc_config, bs);
		if (e) return e;
	}
	if (ptr->bitrate) {
		e = WriteAtom((Atom *) ptr->bitrate, bs);
		if (e) return e;
	}
	if (ptr->descr)	{
		e = WriteAtom((Atom *) ptr->descr, bs);
		if (e) return e;
	}
	return M4OK;
}

M4Err avc1_Size(Atom *s)
{
	M4Err e;
	AVCSampleEntryAtom *ptr = (AVCSampleEntryAtom *)s;
	e = Atom_Size(s);
	if (e) return e;

	SizeVideoSampleEntry((VisualSampleEntryAtom *)s);

	if (ptr->avc_config) {
		e = SizeAtom((Atom *) ptr->avc_config); 
		if (e) return e;
		ptr->size += ptr->avc_config->size;
	}
	if (ptr->bitrate) {
		e = SizeAtom((Atom *) ptr->bitrate);
		if (e) return e;
		ptr->size += ptr->bitrate->size;
	}
	if (ptr->descr) {
		e = SizeAtom((Atom *) ptr->descr);
		if (e) return e;
		ptr->size += ptr->descr->size;
	}
	return M4OK;
}
#endif 

