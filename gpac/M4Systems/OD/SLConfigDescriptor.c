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
Descriptor *NewSLConfigDescriptor(u8 predef)
{
	SLConfigDescriptor *newDesc = (SLConfigDescriptor *) malloc(sizeof(SLConfigDescriptor));
	if (!newDesc) return NULL;

	newDesc->predefined = predef;
	newDesc->AUDuration = 0;
	newDesc->AULength = 0;
	newDesc->AUSeqNumLength = 0;
	newDesc->CUDuration = 0;
	newDesc->degradationPriorityLength = 0;
	newDesc->durationFlag = 0;
	newDesc->instantBitrateLength = 0;
	newDesc->OCRLength = 0;
	newDesc->OCRResolution = 0;
	newDesc->packetSeqNumLength = 0;
	newDesc->startCTS = newDesc->startDTS = 0;
	newDesc->timeScale = 0;
	newDesc->timestampLength = 0;
	newDesc->timestampResolution = 0;
	newDesc->useAccessUnitEndFlag = 0;
	newDesc->useAccessUnitStartFlag = 0;
	newDesc->useIdleFlag = 0;
	newDesc->usePaddingFlag = 0;
	newDesc->useRandomAccessPointFlag = 0;
	newDesc->useRandomAccessUnitsOnlyFlag = 0;
	newDesc->useTimestampsFlag = 0;

	if (predef) SLSetPredefined(newDesc);
	newDesc->durationFlag = 0;
	newDesc->useTimestampsFlag = 1;
	newDesc->tag = SLConfigDescriptor_Tag;

	return (Descriptor *)newDesc;
}

//
//	Destructor
//
M4Err DelSL(SLConfigDescriptor *sl)
{
	if (!sl) return M4BadParam;
	free(sl);
	return M4OK;
}

//
//	Set the SL to the ISO predefined value
//
M4Err SLSetPredefined(SLConfigDescriptor *sl)
{
	if (! sl) return M4BadParam;

	switch (sl->predefined) {

	case SLPredef_MP4:
		sl->useAccessUnitStartFlag = 0;
		sl->useAccessUnitEndFlag = 0;
		//each packet is an AU, and we need RAP signaling
		sl->useRandomAccessPointFlag = 1;
		sl->useRandomAccessUnitsOnlyFlag = 0;
		sl->usePaddingFlag = 0;
		//in MP4 file, we use TimeStamps
		sl->useTimestampsFlag = 1;
		sl->useIdleFlag = 0;
		sl->durationFlag = 0;
		sl->timestampLength = 0;
		sl->OCRLength = 0;
		sl->AULength = 0;
		sl->instantBitrateLength = 0;
		sl->degradationPriorityLength = 0;
		sl->AUSeqNumLength = 0;
		sl->packetSeqNumLength = 0;
		break;

	case SLPredef_Null:
		sl->useAccessUnitStartFlag = 0;
		sl->useAccessUnitEndFlag = 0;
		sl->useRandomAccessPointFlag = 0;
		//should be 1 otherwise NULL SL is useless (cannot carry anything)
		sl->useRandomAccessUnitsOnlyFlag = 0;
		sl->usePaddingFlag = 0;
		sl->useTimestampsFlag = 0;
		sl->useIdleFlag = 0;
		sl->AULength = 0;
		sl->degradationPriorityLength = 0;
		sl->AUSeqNumLength = 0;
		sl->packetSeqNumLength = 0;

		//for MPEG4 IP
		sl->timestampResolution = 1000;
		sl->timestampLength = 32;
		break;
	case SLPredef_SkipSL:
		sl->predefined = SLPredef_SkipSL;
		break;
	/*handle all unknown predefined as predef-null*/
	default:
		sl->useAccessUnitStartFlag = 0;
		sl->useAccessUnitEndFlag = 0;
		sl->useRandomAccessPointFlag = 0;
		//should be 1 otherwise NULL SL is useless (cannot carry anything)
		sl->useRandomAccessUnitsOnlyFlag = 0;
		sl->usePaddingFlag = 0;
		sl->useTimestampsFlag = 1;
		sl->useIdleFlag = 0;
		sl->AULength = 0;
		sl->degradationPriorityLength = 0;
		sl->AUSeqNumLength = 0;
		sl->packetSeqNumLength = 0;

		sl->timestampResolution = 1000;
		sl->timestampLength = 32;
		break;
	}

	return M4OK;
}

u32 SLIsPredefined(SLConfigDescriptor *sl)
{
	if (!sl) return 0;
	if (sl->predefined) return sl->predefined;

	if (!sl->useAccessUnitStartFlag
		&&  !sl->useAccessUnitEndFlag
		&& !sl->usePaddingFlag
		&& sl->useTimestampsFlag
		&& !sl->useIdleFlag
		&& !sl->durationFlag
		&& !sl->timestampLength
		&& !sl->OCRLength
		&& !sl->AULength
		&& !sl->instantBitrateLength
		&& !sl->degradationPriorityLength
		&& !sl->AUSeqNumLength
		&& !sl->packetSeqNumLength)
		return SLPredef_MP4;

	return 0;
}

//this function gets the real amount of bytes needed to store the timeStamp
static u32 GetTSbytesLen(SLConfigDescriptor *sl)
{
	u32 TSlen, TSbytes;
	if (! sl) return 0;

	TSlen = sl->timestampLength * 2;
	TSbytes = TSlen / 8;
	TSlen = TSlen % 8;
	if (TSlen) TSbytes += 1;
	return TSbytes;
}

//
//		Reader
//
M4Err ReadSL(BitStream *bs, SLConfigDescriptor *sl, u32 DescSize)
{
	M4Err e;
	u32 reserved, nbBytes = 0;
	
	if (!sl) return M4BadParam;

	//APPLE fix
	if (!DescSize) {
		sl->predefined = SLPredef_MP4;
		return SLSetPredefined(sl);
	}

	sl->predefined = BS_ReadInt(bs, 8);
	nbBytes += 1;

	/*MPEG4 IP fix*/
	if (!sl->predefined && nbBytes==DescSize) {
		sl->predefined = SLPredef_Null;
		SLSetPredefined(sl);
		return M4OK;
	}

	if (sl->predefined) {
		//predefined configuration
		e = SLSetPredefined(sl);
		if (e) return e;
	} else {
		sl->useAccessUnitStartFlag = BS_ReadInt(bs, 1);
		sl->useAccessUnitEndFlag = BS_ReadInt(bs, 1);
		sl->useRandomAccessPointFlag = BS_ReadInt(bs, 1);
		sl->useRandomAccessUnitsOnlyFlag = BS_ReadInt(bs, 1);
		sl->usePaddingFlag = BS_ReadInt(bs, 1);
		sl->useTimestampsFlag = BS_ReadInt(bs, 1);
		sl->useIdleFlag = BS_ReadInt(bs, 1);
		sl->durationFlag = BS_ReadInt(bs, 1);
		sl->timestampResolution = BS_ReadInt(bs, 32);
		sl->OCRResolution = BS_ReadInt(bs, 32);

		sl->timestampLength = BS_ReadInt(bs, 8);
		if (sl->timestampLength > 64) return M4InvalidDescriptor;

		sl->OCRLength = BS_ReadInt(bs, 8);
		if (sl->OCRLength > 64) return M4InvalidDescriptor;
		
		sl->AULength = BS_ReadInt(bs, 8);
		if (sl->AULength > 32) return M4InvalidDescriptor;

		sl->instantBitrateLength = BS_ReadInt(bs, 8);
		sl->degradationPriorityLength = BS_ReadInt(bs, 4);
		sl->AUSeqNumLength = BS_ReadInt(bs, 5);
		if (sl->AUSeqNumLength > 16) return M4InvalidDescriptor;
		sl->packetSeqNumLength = BS_ReadInt(bs, 5);
		if (sl->packetSeqNumLength > 16) return M4InvalidDescriptor;
	
		reserved = BS_ReadInt(bs, 2);
		nbBytes += 15;
	}

	if (sl->durationFlag) {
		sl->timeScale = BS_ReadInt(bs, 32);
		sl->AUDuration = BS_ReadInt(bs, 16);
		sl->CUDuration = BS_ReadInt(bs, 16);
		nbBytes += 8;
	}
	if (! sl->useTimestampsFlag) {
		sl->startDTS = BS_ReadLongInt(bs, sl->timestampLength);
		sl->startCTS = BS_ReadLongInt(bs, sl->timestampLength);
		nbBytes += GetTSbytesLen(sl);
	}
	
	if (DescSize != nbBytes) return M4ReadDescriptorFailed;
	return M4OK;
}


//
//	Size
//
M4Err SizeSL(SLConfigDescriptor *sl, u32 *outSize)
{
	if (! sl) return M4BadParam;

	*outSize = 1;
	if (! sl->predefined)	*outSize += 15;
	if (sl->durationFlag)	*outSize += 8;
	if (! sl->useTimestampsFlag) *outSize += GetTSbytesLen(sl);
	return M4OK;
}

//
//	Writer
//
M4Err WriteSL(BitStream *bs, SLConfigDescriptor *sl)
{
	M4Err e;
	u32 size;
	if (! sl) return M4BadParam;

	e = CalcSize((Descriptor *)sl, &size);
	e = writeBaseDescriptor(bs, sl->tag, size);

	BS_WriteInt(bs, sl->predefined, 8);
	if (! sl->predefined) {
		BS_WriteInt(bs, sl->useAccessUnitStartFlag, 1);
		BS_WriteInt(bs, sl->useAccessUnitEndFlag, 1);
		BS_WriteInt(bs, sl->useRandomAccessPointFlag, 1);
		BS_WriteInt(bs, sl->useRandomAccessUnitsOnlyFlag, 1);
		BS_WriteInt(bs, sl->usePaddingFlag, 1);
		BS_WriteInt(bs, sl->useTimestampsFlag, 1);
		BS_WriteInt(bs, sl->useIdleFlag, 1);
		BS_WriteInt(bs, sl->durationFlag, 1);
		BS_WriteInt(bs, sl->timestampResolution, 32);
		BS_WriteInt(bs, sl->OCRResolution, 32);
		BS_WriteInt(bs, sl->timestampLength, 8);
		BS_WriteInt(bs, sl->OCRLength, 8);
		BS_WriteInt(bs, sl->AULength, 8);
		BS_WriteInt(bs, sl->instantBitrateLength, 8);
		BS_WriteInt(bs, sl->degradationPriorityLength, 4);
		BS_WriteInt(bs, sl->AUSeqNumLength, 5);
		BS_WriteInt(bs, sl->packetSeqNumLength, 5);
		BS_WriteInt(bs, 3, 2);	//reserved: 0b11 == 3
	}
	if (sl->durationFlag) {
		BS_WriteInt(bs, sl->timeScale, 32);
		BS_WriteInt(bs, sl->AUDuration, 16);
		BS_WriteInt(bs, sl->CUDuration, 16);
	}
	if (! sl->useTimestampsFlag) {
		BS_WriteLongInt(bs, sl->startDTS, sl->timestampLength);
		BS_WriteLongInt(bs, sl->startCTS, sl->timestampLength);
	}
	
	return M4OK;
}


