/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / MP4 reader plugin
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

#include "mp4_io.h"
#include <gpac/m4_decoder.h>


/*emulate a default IOD for all other files (3GP, weird MP4, QT )*/
M4Err M4R_EmulateIOD(M4Reader *read, char **raw_iod, u32 *raw_iod_size)
{
	LPODCODEC codec;
	ObjectDescriptorUpdate *odU;
	M4Err e;
	u32 i, OD_ID, ID;
	InitialObjectDescriptor *fake_iod;
	ObjectDescriptor *od;
	ESDescriptor *esd;

	//NM_OnMessage(read->service, M4OK, "IOD not found or broken - emulating");
	read->OD_ESID = 0;
	for (i=0; i<M4_GetTrackCount(read->mov); i++) {
		switch (M4_GetMediaType(read->mov, i+1)) {
		case M4_AudioMediaType:
		case M4_VisualMediaType:
		case M4_TimedTextMediaType:
			ID = M4_GetTrackID(read->mov, i+1);
			if ((ID<0xFFFF) && (read->OD_ESID < ID)) read->OD_ESID = ID;
			break;
		}
	}
	/*no usable tracks*/
	if (!read->OD_ESID) return M4InvalidMP4File;
	read->OD_ESID++;


	OD_ID = 2;

	/*make OD AU*/
	codec = OD_NewCodec(OD_WRITE);	
	odU = (ObjectDescriptorUpdate *) OD_NewCommand(ODUpdate_Tag);

	for (i=0; i<M4_GetTrackCount(read->mov); i++) {
		switch (M4_GetMediaType(read->mov, i+1)) {
		case M4_AudioMediaType:
		case M4_VisualMediaType:
		case M4_TimedTextMediaType:
			esd = MP4T_MapESDescriptor(read->mov, i+1);
			if (esd) {
				od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
				od->objectDescriptorID = OD_ID;
				esd->OCRESID = read->OD_ESID;
				ChainAddEntry(od->ESDescriptors, esd);
				ChainAddEntry(odU->objectDescriptors, od);
				OD_ID++;
			}
			break;
		default:
			break;
		}
	}

	OD_AddCommand(codec, (ODCommand *)odU);
	OD_EncodeAU(codec);
	OD_GetEncodedAU(codec, &read->od_au, &read->od_au_size);
	OD_DeleteCodec(codec);


	/*generate an IOD with our private dynamic OD stream*/
	fake_iod = (InitialObjectDescriptor *) OD_NewDescriptor(InitialObjectDescriptor_Tag);
	fake_iod->objectDescriptorID = 1;
	esd = OD_NewESDescriptor(0);
	esd->slConfig->timestampResolution = 1000;
	esd->slConfig->useRandomAccessPointFlag = 1;
	esd->slConfig->useTimestampsFlag = 1;
	esd->OCRESID = esd->ESID = read->OD_ESID;
	esd->decoderConfig->streamType = M4ST_OD;
	esd->decoderConfig->objectTypeIndication = GPAC_STATIC_OD_OTI;
	ChainAddEntry(fake_iod->ESDescriptors, esd);
	fake_iod->graphics_profileAndLevel = 1;
	fake_iod->OD_profileAndLevel = 1;
	fake_iod->scene_profileAndLevel = 1;
	fake_iod->audio_profileAndLevel = 0xFE;
	fake_iod->visual_profileAndLevel = 0xFE;
	e = OD_EncDesc((Descriptor *)fake_iod, raw_iod, raw_iod_size);
	OD_DeleteDescriptor((Descriptor **)&fake_iod);
	return e;
}

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return M4R_LoadPlugin();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4STREAMINGCLIENT:
		M4Read_Delete(ptr);
		break;
	}
}
