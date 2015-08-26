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

void InitVideoSampleEntry(VisualSampleEntryAtom *ent)
{
	ent->horiz_res = ent->vert_res = 0x00480000;
	ent->frames_per_sample = 1;
	ent->bit_depth = 0x18;
	ent->color_table_index = -1;
}

void ReadVideoSampleEntry(VisualSampleEntryAtom *ptr, BitStream *bs, u64 *read)
{
	BS_ReadData(bs, (unsigned char*)ptr->reserved, 6);
	ptr->dataReferenceIndex = BS_ReadInt(bs, 16);
	ptr->version = BS_ReadInt(bs, 16);
	ptr->revision = BS_ReadInt(bs, 16);
	ptr->vendor = BS_ReadInt(bs, 32);
	ptr->temporal_quality  = BS_ReadInt(bs, 32);
	ptr->spacial_quality = BS_ReadInt(bs, 32);
	ptr->Width = BS_ReadInt(bs, 16);
	ptr->Height = BS_ReadInt(bs, 16);
	ptr->horiz_res = BS_ReadInt(bs, 32);
	ptr->vert_res = BS_ReadInt(bs, 32);
	ptr->entry_data_size = BS_ReadInt(bs, 32);
	ptr->frames_per_sample = BS_ReadInt(bs, 16);
	BS_ReadData(bs, ptr->compressor_name, 32);
	ptr->compressor_name[32] = 0;
	ptr->bit_depth = BS_ReadInt(bs, 16);
	ptr->color_table_index = BS_ReadInt(bs, 16);
	*read += 78;
}

#ifndef M4_READ_ONLY
void WriteVideoSampleEntry(VisualSampleEntryAtom *ptr, BitStream *bs)
{

	BS_WriteData(bs, (unsigned char*)ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);
	
	BS_WriteInt(bs, ptr->version, 16);
	BS_WriteInt(bs, ptr->revision, 16);
	BS_WriteInt(bs, ptr->vendor, 32);
	BS_WriteInt(bs, ptr->temporal_quality, 32);
	BS_WriteInt(bs, ptr->spacial_quality, 32);
	BS_WriteInt(bs, ptr->Width, 16);
	BS_WriteInt(bs, ptr->Height, 16);
	BS_WriteInt(bs, ptr->horiz_res, 32);
	BS_WriteInt(bs, ptr->vert_res, 32);
	BS_WriteInt(bs, ptr->entry_data_size, 32);
	BS_WriteInt(bs, ptr->frames_per_sample, 16);
	BS_WriteData(bs, ptr->compressor_name, 32);
	BS_WriteInt(bs, ptr->bit_depth, 16);
	BS_WriteInt(bs, ptr->color_table_index, 16);
}

void SizeVideoSampleEntry(VisualSampleEntryAtom *ent)
{
	ent->size += 78;
}


#endif



void InitAudioSampleEntry(AudioSampleEntryAtom *ptr)
{
	ptr->channel_count = 2;
	ptr->bitspersample = 16;
}

void ReadAudioSampleEntry(AudioSampleEntryAtom *ptr, BitStream *bs, u64 *read)
{
	BS_ReadData(bs, (unsigned char*)ptr->reserved, 6);
	ptr->dataReferenceIndex = BS_ReadInt(bs, 16);
	ptr->version = BS_ReadInt(bs, 16);
	ptr->revision = BS_ReadInt(bs, 16);
	ptr->vendor = BS_ReadInt(bs, 32);
	ptr->channel_count = BS_ReadInt(bs, 16);
	ptr->bitspersample = BS_ReadInt(bs, 16);
	ptr->compression_id = BS_ReadInt(bs, 16);
	ptr->packet_size = BS_ReadInt(bs, 16);
	ptr->samplerate_hi = BS_ReadInt(bs, 16);
	ptr->samplerate_lo = BS_ReadInt(bs, 16);
	*read += 28;
}

#ifndef M4_READ_ONLY

void WriteAudioSampleEntry(AudioSampleEntryAtom *ptr, BitStream *bs)
{
	BS_WriteData(bs, (unsigned char*) ptr->reserved, 6);
	BS_WriteInt(bs, ptr->dataReferenceIndex, 16);

	BS_WriteInt(bs, ptr->version, 16);
	BS_WriteInt(bs, ptr->revision, 16);
	BS_WriteInt(bs, ptr->vendor, 32);
	BS_WriteInt(bs, ptr->channel_count, 16);
	BS_WriteInt(bs, ptr->bitspersample, 16);
	BS_WriteInt(bs, ptr->compression_id, 16);
	BS_WriteInt(bs, ptr->packet_size, 16);
	BS_WriteInt(bs, ptr->samplerate_hi, 16);
	BS_WriteInt(bs, ptr->samplerate_lo, 16);
}

void SizeAudioSampleEntry(AudioSampleEntryAtom *ptr)
{
	ptr->size += 28;
}


#endif	//M4_READ_ONLY
