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


#include <m4_descriptors.h>


//from OD
M4Err writeDescList(BitStream *bs, Chain *descList);
M4Err calcDescListSize(Chain *descList, u32 *outSize);
M4Err DelDesc(Descriptor *desc);
M4Err ParseDescriptor(BitStream *bs, Descriptor **desc, u32 *desc_size);
u32 GetSizeFieldSize(u32 size_desc);


//max size of an OCI event
#define OCI_MAX_EVENT_SIZE		1<<28 - 1
#define MAX_OCIEVENT_ID		0x7FFF

typedef struct tagOCIEvent
{
	u16 EventID;
	u8 AbsoluteTimeFlag;
	u8 StartingTime[4];
	u8 duration[4];
	Chain *OCIDescriptors;
} OCIEvent;

typedef struct tagOCICodec
{
	//events
	Chain *OCIEvents;
	//version, should always be one
	u8 Version;
	//encoder or decoder
	u8 Mode;
} OCICodec;

LPOCIEVENT NewOCIEvent(u16 EventID)
{
	OCIEvent *tmp;
	if (EventID > MAX_OCIEVENT_ID) return NULL;
	tmp = malloc(sizeof(OCIEvent));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(OCIEvent));
	tmp->EventID = EventID;
	tmp->OCIDescriptors = NewChain();
	return tmp;
}

void DeleteOCIEvent(LPOCIEVENT event)
{
	Descriptor *desc;
	if (!event) return;

	while (ChainGetCount(event->OCIDescriptors)) {
		desc = ChainGetEntry(event->OCIDescriptors, 0);
		ChainDeleteEntry(event->OCIDescriptors, 0);
		DelDesc(desc);
	}
	DeleteChain(event->OCIDescriptors);
	free(event);	
}

M4Err OCIEvent_SetStartTime(LPOCIEVENT event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds, u8 IsAbsoluteTime)
{
	if (!event || (Hours >= 100) || (Minutes >= 100) || (Seconds >= 100) || (HundredSeconds >= 100) )
		return M4BadParam;

	event->AbsoluteTimeFlag = IsAbsoluteTime;
	event->StartingTime[0] = Hours;
	event->StartingTime[1] = Minutes;
	event->StartingTime[2] = Seconds;
	event->StartingTime[3] = HundredSeconds;
	return M4OK;
}

M4Err OCIEvent_SetDuration(LPOCIEVENT event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds)
{
	if (!event || (Hours >= 100) || (Minutes >= 100) || (Seconds >= 100) || (HundredSeconds >= 100) )
		return M4BadParam;

	event->duration[0] = Hours;
	event->duration[1] = Minutes;
	event->duration[2] = Seconds;
	event->duration[3] = HundredSeconds;
	return M4OK;
}

u8 OCI_IsOCIDesc(Descriptor *oci_desc)
{
	if (oci_desc->tag < ISO_OCI_RANGE_START && oci_desc->tag > ISO_OCI_RANGE_END) return 0;
	return 1;
}

M4Err OCIEvent_AddDescriptor(LPOCIEVENT event, Descriptor *oci_desc)
{
	if (!event || !oci_desc) return M4BadParam;
	if (!OCI_IsOCIDesc(oci_desc)) return M4InvalidDescriptor;
	
	ChainAddEntry(event->OCIDescriptors, oci_desc);
	return M4OK;
}

M4Err OCIEvent_GetEventID(LPOCIEVENT event, u16 *ID)
{
	if (!event || !ID) return M4BadParam;
	*ID = event->EventID;
	return M4OK;
}

M4Err OCIEvent_GetStartTime(LPOCIEVENT event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds, u8 *IsAbsoluteTime)
{
	if (!event || !Hours || !Minutes || !Seconds || !HundredSeconds || !IsAbsoluteTime) 
		return M4BadParam;

	*IsAbsoluteTime = event->AbsoluteTimeFlag;
	*Hours = event->StartingTime[0];
	*Minutes = event->StartingTime[1];
	*Seconds = event->StartingTime[2];
	*HundredSeconds = event->StartingTime[3];
	return M4OK;
}

M4Err OCIEvent_GetDuration(LPOCIEVENT event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds)
{
	if (!event || !Hours || !Minutes || !Seconds || !HundredSeconds) 
		return M4BadParam;

	*Hours = event->duration[0];
	*Minutes = event->duration[1];
	*Seconds = event->duration[2];
	*HundredSeconds = event->duration[3];
	return M4OK;
}

u32 OCIEvent_GetDescriptorCount(LPOCIEVENT event)
{
	if (!event) return 0;
	return ChainGetCount(event->OCIDescriptors);
}

Descriptor *OCIEvent_GetDescriptor(LPOCIEVENT event, u32 DescIndex)
{
	if (!event || DescIndex >= ChainGetCount(event->OCIDescriptors) ) return NULL;
	return ChainGetEntry(event->OCIDescriptors, DescIndex);
}

M4Err OCIEvent_RemoveDescriptor(LPOCIEVENT event, u32 DescIndex)
{
	if (!event || DescIndex >= ChainGetCount(event->OCIDescriptors) ) return M4BadParam;
	return ChainDeleteEntry(event->OCIDescriptors, DescIndex);
}


//construction / destruction
LPOCICODEC OCI_NewCodec(u8 IsEncoder, u8 Version)
{
	OCICodec *tmp;
	if (Version != 0x01) return NULL;
	tmp = malloc(sizeof(OCICodec));
	if (!tmp) return NULL;
	tmp->Mode = IsEncoder ? 1 : 0;
	tmp->Version = 0x01;
	tmp->OCIEvents = NewChain();
	return tmp;
}

void OCI_DeleteCodec(LPOCICODEC codec)
{
	OCIEvent *ev;
	if (!codec) return;

	while (ChainGetCount(codec->OCIEvents)) {
		ev = ChainGetEntry(codec->OCIEvents, 0);
		DeleteOCIEvent(ev);
		ChainDeleteEntry(codec->OCIEvents, 0);
	}
	DeleteChain(codec->OCIEvents);
	free(codec);
}

M4Err OCI_AddEvent(LPOCICODEC codec, LPOCIEVENT event)
{
	if (!codec || !codec->Mode || !event) return M4BadParam;

	return ChainAddEntry(codec->OCIEvents, event);
}

M4Err WriteSevenBitLength(BitStream *bs, u32 size)
{
	u32 length;
	unsigned char vals[4];

	if (!bs || !size) return M4BadParam;
	
	length = size;
	vals[3] = (unsigned char) (length & 0x7f);
	length >>= 7;
	vals[2] = (unsigned char) ((length & 0x7f) | 0x80); 
	length >>= 7;
	vals[1] = (unsigned char) ((length & 0x7f) | 0x80); 
	length >>= 7;
	vals[0] = (unsigned char) ((length & 0x7f) | 0x80);
	
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

M4Err OCI_EncodeAU(LPOCICODEC codec, char **outAU, u32 *au_length)
{
	BitStream *bs;
	u32 i, size, desc_size;
	M4Err e;
	OCIEvent *ev;
	
	if (!codec || !codec->Mode || *outAU) return M4BadParam;

	bs = NULL;
	size = 0;

	//get the size of each event
	for (i=0; i<ChainGetCount(codec->OCIEvents); i++) {
		ev = ChainGetEntry(codec->OCIEvents, i);
		//fixed size header
		size += 10;
		e = calcDescListSize(codec->OCIEvents, &desc_size);
		if (e) goto err_exit;
		size += desc_size;
	}

	//encode
	bs = NewBitStream(NULL, 0, BS_WRITE);

	e = WriteSevenBitLength(bs, size);
	if (e) goto err_exit;

	//get one event, write it and delete it
	while (ChainGetCount(codec->OCIEvents)) {
		ev = ChainGetEntry(codec->OCIEvents, 0);
		ChainDeleteEntry(codec->OCIEvents, 0);

		BS_WriteInt(bs, ev->EventID, 15);
		BS_WriteInt(bs, ev->AbsoluteTimeFlag, 1);
		BS_WriteData(bs, ev->StartingTime, 4);
		BS_WriteData(bs, ev->duration, 4);
		
		e = writeDescList(bs, ev->OCIDescriptors);
		DeleteOCIEvent(ev);
		if (e) goto err_exit;
		//OCI Event is aligned
		BS_Align(bs);
	}
	BS_GetContent(bs, (unsigned char **) outAU, au_length);
	DeleteBitStream(bs);
	return M4OK;


err_exit:
	if (bs) DeleteBitStream(bs);
	//delete everything
	while (ChainGetCount(codec->OCIEvents)) {
		ev = ChainGetEntry(codec->OCIEvents, 0);
		ChainDeleteEntry(codec->OCIEvents, 0);
		DeleteOCIEvent(ev);
	}
	return e;
}


M4Err OCI_DecodeAU(LPOCICODEC codec, char *au, u32 au_length)
{
	OCIEvent *ev;
	BitStream *bs;
	u32 size, hdrS, desc_size, tot_size, tmp_size, val;
	Descriptor *tmp;	
	M4Err e;

	//must be decoder
	if (!codec || codec->Mode || !au) return M4BadParam;

	bs = NewBitStream(au, au_length, BS_READ);
	ev = 0;
	tot_size = 0;
	while (tot_size < au_length) {
		//create an event
		ev = NewOCIEvent(0);
		if (!ev) {
			e = M4OutOfMem;
			goto err_exit;
		}
		

		//FIX IM1
		BS_ReadInt(bs, 8);
		size = 0;
		//get its size
		hdrS = 0;
		do {
			val = BS_ReadInt(bs, 8);
			hdrS += 1;
			size <<= 7;
			size |= val & 0x7F;
		} while ( val & 0x80 );
		
		//parse event vars
		ev->EventID = BS_ReadInt(bs, 15);
		ev->AbsoluteTimeFlag = BS_ReadInt(bs, 1);
		BS_ReadData(bs, ev->StartingTime, 4);
		BS_ReadData(bs, ev->duration, 4);
		desc_size = 0;

		//parse descriptor list
		while (desc_size < size - 10) {
			e = ParseDescriptor(bs, &tmp, &tmp_size);
			//RE-FIX IM1
			if (e || !tmp) goto err_exit;
			if (!OCI_IsOCIDesc(tmp)) {
				DelDesc(tmp);
				e = M4InvalidDescriptor;
				goto err_exit;
			}
			ChainAddEntry(ev->OCIDescriptors, tmp);
			desc_size += tmp_size + GetSizeFieldSize(tmp_size);
		}
		
		if (desc_size != size - 10) {
			e = M4CorruptedData;
			goto err_exit;
		}

		ChainAddEntry(codec->OCIEvents, ev);
		//FIX IM1
		size += 1;
		tot_size += size + hdrS;
		ev = NULL;
	}

	if (tot_size != au_length) {
		e = M4CorruptedData;
		goto err_exit;
	}

	DeleteBitStream(bs);
	return M4OK;

err_exit:
	DeleteBitStream(bs);
	if (ev) DeleteOCIEvent(ev);
	//delete everything
	while (ChainGetCount(codec->OCIEvents)) {
		ev = ChainGetEntry(codec->OCIEvents, 0);
		ChainDeleteEntry(codec->OCIEvents, 0);
		DeleteOCIEvent(ev);
	}
	return e;
}


LPOCIEVENT OCI_GetEvent(LPOCICODEC codec)
{
	OCIEvent *ev;
	if (!codec ||codec->Mode) return NULL;
	ev = ChainGetEntry(codec->OCIEvents, 0);
	ChainDeleteEntry(codec->OCIEvents, 0);
	return ev;
}
