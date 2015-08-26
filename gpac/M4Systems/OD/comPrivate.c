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
//		CONSTRUCTORS
//
ODCommand *CreateCom(u8 tag)
{
	ODCommand *com;
	switch (tag) {
	case ODUpdate_Tag:
		return NewODUpdate();
	case ODRemove_Tag:
		return NewODRemove();
	case ESDUpdate_Tag:
		return NewESDUpdate();
	case ESDRemove_Tag:
		return NewESDRemove();
	//special case for ESDRemove in the file format...
	case ESDRemove_Ref_Tag:
		com = NewESDRemove();
		if (!com) return com;
		com->tag = ESDRemove_Ref_Tag;
		return com;

	case IPMPDUpdate_Tag:
		return NewIPMPDUpdate();
	case IPMPDRemove_Tag:
		return NewIPMPDRemove();

	case ODExecute_Tag:
		return NewODExecute();

	default:
		if ( (tag >= ISO_RESERVED_COMMANDS_BEGIN) && 
			( tag <= ISO_RESERVED_COMMANDS_END) ) {
			return NULL;
		}
		com = NewBaseCom();
		if (!com) return com;
		com->tag = tag;
		return com;
	}
}


//
//		DESTRUCTORS
//
M4Err DelCom(ODCommand *com)
{
	switch (com->tag) {
	case ODUpdate_Tag:
		return DelODUpdate((ObjectDescriptorUpdate *)com);
	case ODRemove_Tag:
		return DelODRemove((ObjectDescriptorRemove *)com);

	case ESDUpdate_Tag:
		return DelESDUpdate((ESDescriptorUpdate *)com);
	case ESDRemove_Tag:
	case ESDRemove_Ref_Tag:
		return DelESDRemove((ESDescriptorRemove *)com);
	case IPMPDUpdate_Tag:
		return DelIPMPDUpdate((IPMPDescriptorUpdate *)com);
	case IPMPDRemove_Tag:
		return DelIPMPDRemove((IPMPDescriptorRemove *)com);

	case ODExecute_Tag:
		return DelODExecute((ObjectDescriptorExecute *)com);

	default:
		return DelBaseCom((BaseODCommand *)com);
	}
}


//
//		READERS
//
M4Err ReadCom(BitStream *bs, ODCommand *com, u32 ComSize)
{
	switch (com->tag) {
	case ODUpdate_Tag:
		return ReadODUpdate(bs, (ObjectDescriptorUpdate *)com, ComSize);
	case ODRemove_Tag:
		return ReadODRemove(bs, (ObjectDescriptorRemove *)com, ComSize);
	case ESDUpdate_Tag:
		return ReadESDUpdate(bs, (ESDescriptorUpdate *)com, ComSize);
	case ESDRemove_Tag:
	case ESDRemove_Ref_Tag:
		return ReadESDRemove(bs, (ESDescriptorRemove *)com, ComSize);
	case IPMPDUpdate_Tag:
		return ReadIPMPDUpdate(bs, (IPMPDescriptorUpdate *)com, ComSize);
	case IPMPDRemove_Tag:
		return ReadIPMPDRemove(bs, (IPMPDescriptorRemove *)com, ComSize);
	case ODExecute_Tag:
		return ReadODExecute(bs, (ObjectDescriptorExecute *)com, ComSize);
	
	default:
		return ReadBaseCom(bs, (BaseODCommand *)com, ComSize);
	}
}



//
//		SIZE FUNCTION
//
M4Err ComSize(ODCommand *com, u32 *outSize)
{
	switch (com->tag) {
	case ODUpdate_Tag:
		return SizeODUpdate((ObjectDescriptorUpdate *)com, outSize);
	case ODRemove_Tag:
		return SizeODRemove((ObjectDescriptorRemove *)com, outSize);

	case ESDUpdate_Tag:
		return SizeESDUpdate((ESDescriptorUpdate *)com, outSize);
	case ESDRemove_Tag:
	case ESDRemove_Ref_Tag:
		return SizeESDRemove((ESDescriptorRemove *)com, outSize);
	case IPMPDUpdate_Tag:
		return SizeIPMPDUpdate((IPMPDescriptorUpdate *)com, outSize);
	case IPMPDRemove_Tag:
		return SizeIPMPDRemove((IPMPDescriptorRemove *)com, outSize);
	case ODExecute_Tag:
		return SizeODExecute((ObjectDescriptorExecute *)com, outSize);

	default:
		return SizeBaseCom((BaseODCommand *)com, outSize);
	}
}


//
//		WRITERS
//
M4Err WriteCom(BitStream *bs, ODCommand *com)
{
	switch (com->tag) {
	case ODUpdate_Tag:
		return WriteODUpdate(bs, (ObjectDescriptorUpdate *)com);
	case ODRemove_Tag:
		return WriteODRemove(bs, (ObjectDescriptorRemove *)com);

	case ESDUpdate_Tag:
		return WriteESDUpdate(bs, (ESDescriptorUpdate *)com);
	case ESDRemove_Tag:
	case ESDRemove_Ref_Tag:
		return WriteESDRemove(bs, (ESDescriptorRemove *)com);
	case IPMPDUpdate_Tag:
		return WriteIPMPDUpdate(bs, (IPMPDescriptorUpdate *)com);
	case IPMPDRemove_Tag:
		return WriteIPMPDRemove(bs, (IPMPDescriptorRemove *)com);

	case ODExecute_Tag:
		return WriteODExecute(bs, (ObjectDescriptorExecute *)com);
	
	default:
		return WriteBaseCom(bs, (BaseODCommand *)com);
	}
}

