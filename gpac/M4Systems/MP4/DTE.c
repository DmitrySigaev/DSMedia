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

/********************************************************************
		Creation of DataTable entries in the RTP sample
********************************************************************/

GenericDTE *New_EmptyDTE()
{
	EmptyDTE *dte = malloc(sizeof(EmptyDTE));
	dte->source = 0;
	return (GenericDTE *)dte;
}

GenericDTE *New_ImmediateDTE()
{
	ImmediateDTE *dte = malloc(sizeof(ImmediateDTE));
	dte->source = 1;
	memset(dte->data, 0, 14);
	dte->dataLength = 0;
	return (GenericDTE *)dte;
}

GenericDTE *New_SampleDTE()
{
	SampleDTE *dte = malloc(sizeof(SampleDTE));
	dte->source = 2;
	//can be -1 in QT , so init at -2
	dte->trackRefIndex = -2;
	dte->dataLength = 0;
	dte->sampleNumber = 0;
	dte->samplesPerComp = 1;
	dte->byteOffset = 0;
	dte->bytesPerComp = 1;
	return (GenericDTE *)dte;
}

GenericDTE *New_StreamDescDTE()
{
	StreamDescDTE *dte = malloc(sizeof(StreamDescDTE));
	dte->source = 3;
	dte->byteOffset = 0;
	dte->dataLength = 0;
	dte->reserved = 0;
	dte->streamDescIndex = 0;
	//can be -1 in QT , so init at -2
	dte->trackRefIndex = -2;
	return (GenericDTE *)dte;
}

//creation of DTEs
GenericDTE *NewDTE(u8 type)
{
	switch (type) {
	case 0:
		return New_EmptyDTE();
	case 1:
		return New_ImmediateDTE();
	case 2:
		return New_SampleDTE();
	case 3:
		return New_StreamDescDTE();
	default:
		return NULL;
	}
}

/********************************************************************
		Deletion of DataTable entries in the RTP sample
********************************************************************/
void Del_EmptyDTE(EmptyDTE *dte)
{ free(dte); }

void Del_ImmediateDTE(ImmediateDTE *dte)
{ free(dte); }

void Del_SampleDTE(SampleDTE *dte)
{ free(dte); }

void Del_StreamDescDTE(StreamDescDTE *dte)
{ free(dte); }

//deletion of DTEs
void DelDTE(GenericDTE *dte)
{
	switch (dte->source) {
	case 0:
		Del_EmptyDTE((EmptyDTE *)dte);
		break;
	case 1:
		Del_ImmediateDTE((ImmediateDTE *)dte);
		break;
	case 2:
		Del_SampleDTE((SampleDTE *)dte);
		break;
	case 3:
		Del_StreamDescDTE((StreamDescDTE *)dte);
		break;
	default:
		return;
	}
}



/********************************************************************
		Reading of DataTable entries in the RTP sample
********************************************************************/
M4Err Read_EmptyDTE(EmptyDTE *dte, BitStream *bs)
{
	char empty[15];
	//empty but always 15 bytes !!!
	BS_ReadData(bs, empty, 15);
	return M4OK;
}

M4Err Read_ImmediateDTE(ImmediateDTE *dte, BitStream *bs)
{
	dte->dataLength = BS_ReadInt(bs, 8);
	if (dte->dataLength > 14) return M4InvalidRTPHint;
	BS_ReadData(bs, dte->data, dte->dataLength);
	BS_ReadData(bs, dte->data, 14 - dte->dataLength);
	return M4OK;
}

M4Err Read_SampleDTE(SampleDTE *dte, BitStream *bs)
{
	dte->trackRefIndex = BS_ReadInt(bs, 8);
	dte->dataLength = BS_ReadInt(bs, 16);
	dte->sampleNumber = BS_ReadInt(bs, 32);
	dte->byteOffset = BS_ReadInt(bs, 32);
	dte->bytesPerComp = BS_ReadInt(bs, 16);
	dte->samplesPerComp = BS_ReadInt(bs, 16);
	return M4OK;
}

M4Err Read_StreamDescDTE(StreamDescDTE *dte, BitStream *bs)
{
	dte->trackRefIndex = BS_ReadInt(bs, 8);
	dte->dataLength = BS_ReadInt(bs, 16);
	dte->streamDescIndex = BS_ReadInt(bs, 32);
	dte->byteOffset = BS_ReadInt(bs, 32);
	dte->reserved = BS_ReadInt(bs, 32);
	return M4OK;
}

M4Err ReadDTE(GenericDTE *dte, BitStream *bs)
{
	switch (dte->source) {
	case 0:
		//nothing to o, it is an empty entry
		return Read_EmptyDTE((EmptyDTE *)dte, bs);
	case 1:
		return Read_ImmediateDTE((ImmediateDTE *)dte, bs);
	case 2:
		return Read_SampleDTE((SampleDTE *)dte, bs);
	case 3:
		return Read_StreamDescDTE((StreamDescDTE *)dte, bs);
	default:
		return M4InvalidRTPHint;
	}
}

/********************************************************************
		Writing of DataTable entries in the RTP sample
********************************************************************/
M4Err Write_EmptyDTE(EmptyDTE *dte, BitStream *bs)
{
	BS_WriteInt(bs, dte->source, 8);
	//empty but always 15 bytes !!!
	BS_WriteData(bs, "empty hint DTE", 15);
	return M4OK;
}

M4Err Write_ImmediateDTE(ImmediateDTE *dte, BitStream *bs)
{
	char data[14];
	BS_WriteInt(bs, dte->source, 8);
	BS_WriteInt(bs, dte->dataLength, 8);
	BS_WriteData(bs, dte->data, dte->dataLength);
	if (dte->dataLength < 14) {
		memset(data, 0, 14);
		BS_WriteData(bs, data, 14 - dte->dataLength);
	}
	return M4OK;
}

M4Err Write_SampleDTE(SampleDTE *dte, BitStream *bs)
{
	BS_WriteInt(bs, dte->source, 8);
	BS_WriteInt(bs, dte->trackRefIndex, 8);
	BS_WriteInt(bs, dte->dataLength, 16);
	BS_WriteInt(bs, dte->sampleNumber, 32);
	BS_WriteInt(bs, dte->byteOffset, 32);
	BS_WriteInt(bs, dte->bytesPerComp, 16);
	BS_WriteInt(bs, dte->samplesPerComp, 16);
	return M4OK;
}

M4Err Write_StreamDescDTE(StreamDescDTE *dte, BitStream *bs)
{
	BS_WriteInt(bs, dte->source, 8);

	BS_WriteInt(bs, dte->trackRefIndex, 8);
	BS_WriteInt(bs, dte->dataLength, 16);
	BS_WriteInt(bs, dte->streamDescIndex, 32);
	BS_WriteInt(bs, dte->byteOffset, 32);
	BS_WriteInt(bs, dte->reserved, 32);
	return M4OK;
}

M4Err WriteDTE(GenericDTE *dte, BitStream *bs)
{
	switch (dte->source) {
	case 0:
		//nothing to do, it is an empty entry
		return Write_EmptyDTE((EmptyDTE *)dte, bs);
	case 1:
		return Write_ImmediateDTE((ImmediateDTE *)dte, bs);
	case 2:
		return Write_SampleDTE((SampleDTE *)dte, bs);
	case 3:
		return Write_StreamDescDTE((StreamDescDTE *)dte, bs);
	default:
		return M4InvalidRTPHint;
	}
}

M4Err OffsetDTE(GenericDTE *dte, u32 offset, u32 HintSampleNumber)
{
	SampleDTE *sDTE;
	//offset shifting is only true for intra sample reference
	switch (dte->source) {
	case 2:
		break;
	default:
		return M4OK;
	}
	
	sDTE = (SampleDTE *)dte;
	//we only adjust for intra HintTrack reference
	if (sDTE->trackRefIndex != -1) return M4OK;
	//and in the same sample
	if (sDTE->sampleNumber != HintSampleNumber) return M4OK;
	sDTE->byteOffset += offset;
	return M4OK;
}

