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

s32 GetSizeFieldSize(u32 size_desc)
{
	if (size_desc < 0x00000080) {
		return 1 + 1;
	} else if (size_desc < 0x00004000) {
		return 2 + 1;
	} else if (size_desc < 0x00200000) {
		return 3 + 1;
	} else if (size_desc < 0x10000000) {
		return 4 + 1;
	} else {
		return -1;
	}

}


M4Err ParseDescriptor(BitStream *bs, Descriptor **desc, u32 *desc_size)
{
	u32 val, size, sizeHeader;
	u8 tag;
	M4Err err;
	Descriptor *newDesc;
	if (!bs) return M4BadParam;

	*desc_size = 0;

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
	*desc_size = size;

	newDesc = CreateDescriptor(tag);
	if (! newDesc) {
		*desc = NULL;
		*desc_size = sizeHeader;
		if ( (tag >= ISO_RESERVED_RANGE_BEGIN) &&
			(tag <= ISO_RESERVED_RANGE_END) ) {
			return M4ISOForbiddenDescriptor;
		}
		else if (tag == ForbiddenFF_Tag || tag == ForbiddenZero_Tag) {
			return M4InvalidDescriptor;
		}
		return M4OutOfMem;
	}

	newDesc->tag = tag;

	err = ReadDesc(bs, newDesc, *desc_size);

	/*FFMPEG fix*/
	if ((tag==SLConfigDescriptor_Tag) && (((SLConfigDescriptor*)newDesc)->predefined==2)) {
		if (*desc_size==3) {
			*desc_size = 1;
			err = M4OK;
		}
	}

	//little trick to handle lazy bitstreams that encode 
	//SizeOfInstance on a fix number of bytes
	//This nb of bytes is added in Read methods
	*desc_size += sizeHeader - GetSizeFieldSize(*desc_size);
	*desc = newDesc;
	if (err) {
		DelDesc(newDesc);
		*desc = NULL;
	}
	return err;
}



M4Err DeleteDescriptorList(Chain *descList)
{
	M4Err e;
	u32 i;
	//no error if NULL chain...
	if (! descList) return M4OK;
	for (i = 0; i < ChainGetCount(descList); i++) {
		Descriptor *tmp = (Descriptor*)ChainGetEntry(descList, i);
		e = DelDesc(tmp);
		if (e) return e;
	}
	DeleteChain(descList);
	return M4OK;
}




M4Err calcDescListSize(Chain *descList, u32 *outSize)
{
	M4Err e;
	Descriptor *tmp;
	u32 tmpSize, count, i;
	if (! descList) return M4OK;

	count = ChainGetCount(descList);
	for ( i = 0; i < count; i++ ) {
		tmp = (Descriptor*)ChainGetEntry(descList, i);
		if (tmp) {
			e = CalcSize(tmp, &tmpSize);
			if (e) return e;
			if (tmpSize) *outSize += tmpSize + GetSizeFieldSize(tmpSize);
		}
	}
	return M4OK;
}



M4Err writeBaseDescriptor(BitStream *bs, u8 tag, u32 size)
{
	u32 length;
	unsigned char vals[4];

	if (!tag ) return M4BadParam;
	
	length = size;
	vals[3] = (unsigned char) (length & 0x7f);
	length >>= 7;
	vals[2] = (unsigned char) ((length & 0x7f) | 0x80); 
	length >>= 7;
	vals[1] = (unsigned char) ((length & 0x7f) | 0x80); 
	length >>= 7;
	vals[0] = (unsigned char) ((length & 0x7f) | 0x80);
	
	BS_WriteInt(bs, tag, 8);
	if (size < 0x00000080) {
		BS_WriteInt(bs, vals[3], 8);
	} else if (size < 0x00004000) {
		BS_WriteInt(bs, vals[2], 8);
		BS_WriteInt(bs, vals[3], 8);
	} else if (size < 0x00200000) {
		BS_WriteInt(bs, vals[1], 8);
		BS_WriteInt(bs, vals[2], 8);
		BS_WriteInt(bs, vals[3], 8);
	} else if (size < 0x10000000) {
		BS_WriteInt(bs, vals[0], 8);
		BS_WriteInt(bs, vals[1], 8);
		BS_WriteInt(bs, vals[2], 8);
		BS_WriteInt(bs, vals[3], 8);
	} else {
		return M4DescSizeOutOfRange;
	}
	return M4OK;
}


M4Err writeDescList(BitStream *bs, Chain *descList)
{
	M4Err e;
	u32 count, i;
	Descriptor *tmp;

	if (! descList) return M4OK;
	count = ChainGetCount(descList);
	for ( i = 0; i < count; i++ ) {
		tmp = (Descriptor*)ChainGetEntry(descList, i);
		if (tmp) {
			e = WriteDesc(bs, tmp);
			if (e) return e;
		} 
	}
	return M4OK;
}

const char *OD_GetStreamTypeName(u32 streamType)
{
	switch (streamType) {
	case M4ST_OD: return "ObjectDescriptor";
	case M4ST_OCR: return "ClockReference";
	case M4ST_SCENE: return "SceneDescription";
	case M4ST_VISUAL: return "Visual";
	case M4ST_AUDIO: return "Audio";
	case M4ST_MPEG7: return "MPEG7";
	case M4ST_IPMP: return "IPMP";
	case M4ST_OCI: return "OCI";
	case M4ST_MPEGJ: return "MPEGJ";
	case M4ST_INTERACT: return "Interaction";
	case M4ST_TEXT: return "Text";
	default: return NULL;
	}
}

u32 OD_GetStreamTypeByName(const char *streamType)
{
	if (!streamType) return 0;
	if (!stricmp(streamType, "ObjectDescriptor")) return M4ST_OD;
	if (!stricmp(streamType, "ClockReference")) return M4ST_OCR;
	if (!stricmp(streamType, "SceneDescription")) return M4ST_SCENE;
	if (!stricmp(streamType, "Visual")) return M4ST_VISUAL;
	if (!stricmp(streamType, "Audio")) return M4ST_AUDIO;
	if (!stricmp(streamType, "MPEG7")) return M4ST_MPEG7;
	if (!stricmp(streamType, "IPMP")) return M4ST_IPMP;
	if (!stricmp(streamType, "OCI")) return M4ST_OCI;
	if (!stricmp(streamType, "MPEGJ")) return M4ST_MPEGJ;
	if (!stricmp(streamType, "Interaction")) return M4ST_INTERACT;
	if (!stricmp(streamType, "Text")) return M4ST_TEXT;
	return 0;
}


/*special authoring functions*/
M4Err OD_GetBIFSConfig(DefaultDescriptor *dsi, u8 oti, BIFSConfigDescriptor *cfg)
{
	Bool hasSize;
	M4Err e;
	BitStream *bs;
	if (!dsi || !dsi->data || !dsi->dataLength || !cfg) return M4BadParam;
	bs = NewBitStream(dsi->data, dsi->dataLength, BS_READ);
	memset(cfg, 0, sizeof(BIFSConfigDescriptor));
	cfg->tag = BIFSConfig_Tag;	
	
	e = M4OK;
	if (oti==2) {
		/*3D Mesh Coding*/
		BS_ReadInt(bs, 1);
		/*PMF*/
		BS_ReadInt(bs, 1);
	}
	cfg->nodeIDbits = BS_ReadInt(bs, 5);
	cfg->routeIDbits = BS_ReadInt(bs, 5);
	if (oti==2) cfg->protoIDbits = BS_ReadInt(bs, 5);
	
	cfg->isCommandStream = BS_ReadInt(bs, 1);
	if (!cfg->isCommandStream) {
		e = M4NotSupported;
	} else {
		cfg->pixelMetrics = BS_ReadInt(bs, 1);
		hasSize = BS_ReadInt(bs, 1);
		if (hasSize) {
			cfg->pixelWidth = BS_ReadInt(bs, 16);
			cfg->pixelHeight = BS_ReadInt(bs, 16);
		}
		BS_Align(bs);
		if (BS_GetSize(bs) != BS_GetPosition(bs))  e = M4ReadDescriptorFailed;
	}
	DeleteBitStream(bs);
	return e;
}

M4Err OD_GetUIConfig(DefaultDescriptor *dsi, UIConfigDescriptor *cfg)
{
	u32 len, i;
	BitStream *bs;
	if (!dsi || !dsi->data || !dsi->dataLength || !cfg) return M4BadParam;
	memset(cfg, 0, sizeof(UIConfigDescriptor));
	cfg->tag = UIConfig_Tag;	
	bs = NewBitStream(dsi->data, dsi->dataLength, BS_READ);
	len = BS_ReadInt(bs, 8);
	cfg->deviceName = malloc(sizeof(char) * (len+1));
	for (i=0; i<len; i++) cfg->deviceName[i] = BS_ReadInt(bs, 8);
	cfg->deviceName[i] = 0;

	if (!stricmp(cfg->deviceName, "StringSensor") && BS_Available(bs)) {
		cfg->termChar = BS_ReadInt(bs, 8);
		cfg->delChar = BS_ReadInt(bs, 8);
	}
	DeleteBitStream(bs);
	return M4OK;
}

M4Err OD_EncodeUIConfig(UIConfigDescriptor *cfg, DefaultDescriptor **out_dsi)
{
	u32 i, len;
	BitStream *bs;
	DefaultDescriptor *dsi;
	if (!out_dsi || (cfg->tag != UIConfig_Tag)) return M4BadParam;

	*out_dsi = NULL;
	if (!cfg->deviceName) return M4OK;

	bs = NewBitStream(NULL, 0, BS_WRITE);
	len = strlen(cfg->deviceName);
	BS_WriteInt(bs, len, 8);
	for (i=0; i<len; i++) BS_WriteInt(bs, cfg->deviceName[i], 8);
	if (!stricmp(cfg->deviceName, "StringSensor")) {
		/*fixme - this should be UTF-8 chars*/
		if (cfg->delChar || cfg->termChar) {
			BS_WriteInt(bs, cfg->termChar, 8);
			BS_WriteInt(bs, cfg->delChar, 8);
		}
	}
	if (cfg->ui_data) BS_WriteData(bs, cfg->ui_data, cfg->ui_data_length);

	dsi = (DefaultDescriptor *) OD_NewDescriptor(DecoderSpecificInfo_Tag);
	BS_GetContent(bs, (unsigned char **) &dsi->data, &dsi->dataLength);
	DeleteBitStream(bs);
	*out_dsi = dsi;
	return M4OK;
}

Descriptor *NewTextSampleDescriptor()
{
	TextSampleDescriptor *newDesc = (TextSampleDescriptor*) malloc(sizeof(TextSampleDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(TextSampleDescriptor));
	newDesc->tag = TextSampleDescriptor_Tag;
	return (Descriptor *) newDesc;
}
M4Err DelTextSampleDescriptor(TextSampleDescriptor *sd)
{
	u32 i;
	for (i=0; i<sd->font_count; i++) 
		if (sd->fonts[i].fontName) free(sd->fonts[i].fontName);
	free(sd->fonts);
	free(sd);
	return M4OK;
}

/*TextConfig*/
Descriptor *NewTextConfigDescriptor()
{
	TextConfigDescriptor *newDesc = (TextConfigDescriptor*) malloc(sizeof(TextConfigDescriptor));
	if (!newDesc) return NULL;
	memset(newDesc, 0, sizeof(TextConfigDescriptor));
	newDesc->tag = TextConfig_Tag;
	newDesc->sample_descriptions = NewChain();
	newDesc->Base3GPPFormat = 0x10;
	newDesc->MPEGExtendedFormat = 0x10;
	newDesc->profileLevel = 0x10;
	newDesc->timescale = 1000;
	return (Descriptor *) newDesc;
}

void ResetTextConfig(TextConfigDescriptor *desc)
{
	Chain *bck;
	while (ChainGetCount(desc->sample_descriptions)) {
		TextSampleDescriptor *sd = ChainGetEntry(desc->sample_descriptions, 0);
		ChainDeleteEntry(desc->sample_descriptions, 0);
		DelTextSampleDescriptor(sd);
	}
	bck = desc->sample_descriptions;
	memset(desc, 0, sizeof(TextConfigDescriptor));
	desc->tag = TextConfig_Tag;	
	desc->sample_descriptions = bck;
}

M4Err DelTextConfig(TextConfigDescriptor *desc)
{
	ResetTextConfig(desc);
	DeleteChain(desc->sample_descriptions);
	free(desc);
	return M4OK;
}

/*we need atom parsing*/
#include <intern/m4_isomedia_dev.h>
M4Err OD_GetTextConfig(DefaultDescriptor *dsi, u8 oti, TextConfigDescriptor *cfg)
{
	u32 i, j;
	Bool has_alt_format, has_sd;
	M4Err e;
	BitStream *bs;
	if (!dsi || !dsi->data || !dsi->dataLength || !cfg) return M4BadParam;
	if (oti != 0x08) return M4NotSupported;

	/*reset*/
	ResetTextConfig(cfg);
	bs = NewBitStream(dsi->data, dsi->dataLength, BS_READ);

	e = M4OK;
	cfg->Base3GPPFormat = BS_ReadInt(bs, 8);
	cfg->MPEGExtendedFormat = BS_ReadInt(bs, 8);
	cfg->profileLevel = BS_ReadInt(bs, 8);
	cfg->timescale = BS_ReadInt(bs, 24);
	has_alt_format = BS_ReadInt(bs, 1);
	cfg->sampleDescriptionFlags = BS_ReadInt(bs, 2);
	has_sd = BS_ReadInt(bs, 1);
	cfg->has_vid_info = BS_ReadInt(bs, 1);
	BS_ReadInt(bs, 3);
	cfg->layer = BS_ReadInt(bs, 8);
	cfg->text_width = BS_ReadInt(bs, 16);
	cfg->text_height = BS_ReadInt(bs, 16);
	if (has_alt_format) {
		cfg->nb_compatible_formats = BS_ReadInt(bs, 8);
		for (i=0; i<cfg->nb_compatible_formats; i++) cfg->compatible_formats[i] = BS_ReadInt(bs, 8);
	}
	if (has_sd) {
		u8 sample_index;
		TextSampleDescriptor *txdesc;
		TextSampleEntryAtom *a;
		u64 read;
		s32 avail;
		u32 nb_desc = BS_ReadInt(bs, 8);

		/*parse TTU[5]s*/
		avail = (s32) BS_Available(bs);
		for (i=0; i<nb_desc; i++) {
			sample_index = BS_ReadInt(bs, 8);
			avail -= 1;
			e = ParseAtom((Atom **) &a, bs, &read);
			if (e) goto exit;
			avail -= (s32) a->size;

			if (avail<0) {
				e = M4NonCompliantBitStream;
				goto exit;
			}
			txdesc = malloc(sizeof(TextSampleDescriptor));
			txdesc->sample_index = sample_index;
			txdesc->displayFlags = a->displayFlags;
			txdesc->back_color = a->back_color;
			txdesc->default_pos = a->default_box;
			txdesc->default_style = a->default_style;
			txdesc->vert_justif = a->vertical_justification;
			txdesc->horiz_justif = a->horizontal_justification;
			txdesc->font_count = a->font_table ? a->font_table->entry_count : 0;
			if (txdesc->font_count) {
				txdesc->fonts = malloc(sizeof(FontRecord)*txdesc->font_count);
				for (j=0; j<txdesc->font_count; j++) {
					txdesc->fonts[j].fontID = a->font_table->fonts[j].fontID;
					txdesc->fonts[j].fontName = a->font_table->fonts[j].fontName ? strdup(a->font_table->fonts[j].fontName) : NULL;
				}
			}
			ChainAddEntry(cfg->sample_descriptions, txdesc);
			DelAtom((Atom *)a);
		}
	}
	if (cfg->has_vid_info) {
		cfg->video_width = BS_ReadInt(bs, 16);
		cfg->video_height = BS_ReadInt(bs, 16);
		cfg->horiz_offset = BS_ReadInt(bs, 16);
		cfg->vert_offset = BS_ReadInt(bs, 16);
	}
	
exit:
	DeleteBitStream(bs);
	if (e) ResetTextConfig(cfg);
	return e;
}

