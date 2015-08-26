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

HintPacket *New_HintPacket(u8 HintType)
{
	switch (HintType) {
	case PCK_RTP:
		return (HintPacket *) New_RTPPacket();

	default:
		return NULL;
	}
}

void Del_HintPacket(u8 HintType, HintPacket *ptr)
{
	switch (HintType) {
	case PCK_RTP:
		Del_RTPPacket((RTPPacket *)ptr);
		break;
	default:
		break;
	}
}

M4Err Read_HintPacket(u8 HintType, HintPacket *ptr, BitStream *bs)
{
	switch (HintType) {
	case PCK_RTP:
		return Read_RTPPacket((RTPPacket *)ptr, bs);
	default:
		return M4NotSupported;
	}
}

#ifndef M4_READ_ONLY

M4Err Write_HintPacket(u8 HintType, HintPacket *ptr, BitStream *bs)
{
	switch (HintType) {
	case PCK_RTP:
		return Write_RTPPacket((RTPPacket *)ptr, bs);
	default:
		return M4NotSupported;
	}
}

u32 Size_HintPacket(u8 HintType, HintPacket *ptr)
{
	switch (HintType) {
	case PCK_RTP:
		return Size_RTPPacket((RTPPacket *)ptr);
	default:
		return 0;
	}
}

M4Err Offset_HintPacket(u8 HintType, HintPacket *ptr, u32 offset, u32 HintSampleNumber)
{
	switch (HintType) {
	case PCK_RTP:
		return Offset_RTPPacket((RTPPacket *)ptr, offset, HintSampleNumber);
	default:
		return M4NotSupported;
	}
}

M4Err AddDTE_HintPacket(u8 HintType, HintPacket *ptr, GenericDTE *dte, u8 AtBegin)
{
	switch (HintType) {
	case PCK_RTP:
		if (AtBegin)
			return ChainInsertEntry( ((RTPPacket *)ptr)->DataTable, dte, 0);
		else
			return ChainAddEntry( ((RTPPacket *)ptr)->DataTable, dte);

	default:
		return M4NotSupported;
	}
}

u32 Length_HintPacket(u8 HintType, HintPacket *ptr)
{
	switch (HintType) {
	case PCK_RTP:
		return Length_RTPPacket((RTPPacket *)ptr);
	default:
		return 0;
	}
}


#endif

