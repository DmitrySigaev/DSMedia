/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / OD decoder plugin
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

#include <gpac/intern/m4_esm_dev.h>

typedef struct 
{
	u16 *ESIDs;
	u32 streamCount;
	InlineScene *scene;
	u32 PL;
} ODPriv;

static M4Err OD_GetCapabilities(BaseDecoder *plug, CapObject *cap)
{
	cap->cap.valueINT = 0;
	return M4NotSupported;
}

static M4Err OD_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
	return M4OK;
}


static M4Err OD_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_inline_scene)
{
	ODPriv *priv = plug->privateStack;
	if (priv->scene) return M4BadParam;
	priv->scene = scene;
	return M4OK;
}

static void ODS_SetupOD(InlineScene *is, ObjectDescriptor *od)
{
	ODManager *odm, *parent;
	odm = IS_FindODM(is, od->objectDescriptorID);
	/*remove the old OD*/
	if (odm) ODM_RemoveOD(odm);
	odm = NewODManager();
	odm->OD = od;
	odm->term = is->root_od->term;
	odm->parentscene = is;
	ChainAddEntry(is->ODlist, odm);

	/*locate service owner*/
	parent = is->root_od;
	while (parent->remote_OD) parent = parent->remote_OD;
	ODM_SetupOD(odm, parent->net_service);
}

static M4Err ODS_ODUpdate(ODPriv *priv, ObjectDescriptorUpdate *odU)
{
	ObjectDescriptor *od;
	u32 count;

	/*extract all our ODs and compare with what we already have...*/
	count = ChainGetCount(odU->objectDescriptors);
	if (count > 255) return M4InvalidDescriptor;

	while (count) {
		od = ChainGetEntry(odU->objectDescriptors, 0);
		ChainDeleteEntry(odU->objectDescriptors, 0);
		count--;
		ODS_SetupOD(priv->scene, od);
	}
	return M4OK;
}



static M4Err ODS_RemoveOD(ODPriv *priv, ObjectDescriptorRemove *odR)
{
	u32 i;
	for (i=0; i< odR->NbODs; i++) {
		ODManager *odm = IS_FindODM(priv->scene, odR->OD_ID[i]);
		if (odm) ODM_RemoveOD(odm);
	}
	return M4OK;
}

static M4Err ODS_UpdateESD(ODPriv *priv, ESDescriptorUpdate *ESDs)
{
	ESDescriptor *esd, *prev;
	ODManager *odm;
	u32 count, i;

	odm = IS_FindODM(priv->scene, ESDs->ODID);
	/*spec: "ignore"*/
	if (!odm) return M4OK;
	/*spec: "ES_DescriptorUpdate shall not be applied on object descriptors that have set URL_Flag to '1' (see 8.6.3)."*/
	if (odm->remote_OD) return M4NonCompliantBitStream;

	count = ChainGetCount(ESDs->ESDescriptors);

	while (count) {
		esd = ChainGetEntry(ESDs->ESDescriptors, 0);
		/*spec: "ES_Descriptors with ES_IDs that have already been received within the same name scope shall be ignored."*/
		prev = NULL;
		for (i=0; i<ChainGetCount(odm->OD->ESDescriptors); i++) {
			prev = ChainGetEntry(odm->OD->ESDescriptors, i);
			if (prev->ESID == esd->ESID) break;
			prev = NULL;
		}

		if (prev) {
			OD_DeleteDescriptor((Descriptor **)&esd);
		} else {
			/*and register new stream*/
			ChainAddEntry(odm->OD->ESDescriptors, esd);
			ODM_SetupStream(odm, esd, odm->net_service);
		}

		/*remove the desc from the AU*/
		ChainDeleteEntry(ESDs->ESDescriptors, 0);
		count--;
	}
	/*resetup object since a new ES has been inserted 
	(typically an empty object first sent, then a stream added - cf how ogg demuxer works)*/
	IS_SetupOD(priv->scene, odm);
	return M4OK;
}


static M4Err ODS_RemoveESD(ODPriv *priv, ESDescriptorRemove *ESDs)
{
	ODManager *odm;
	u32 i;
	odm = IS_FindODM(priv->scene, ESDs->ODID);
	/*spec: "ignore"*/
	if (!odm) return M4OK;
	/*spec: "ES_DescriptorRemove shall not be applied on object descriptors that have set URL_Flag to '1' (see 8.6.3)."*/
	if (odm->remote_OD) return M4NonCompliantBitStream;

	for (i=0; i<ESDs->NbESDs; i++) {
		/*blindly call remove*/
		ODM_RemoveStream(odm, ESDs->ES_ID[i]);
	}
	return M4OK;
}

static M4Err OD_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	ODPriv *priv = plug->privateStack;
	
	if (Upstream) return M4OK;
	//warning, we only support one stream ...
	if (priv->streamCount) return M4NotSupported;
	priv->ESIDs = malloc(sizeof(u16));
	priv->ESIDs[0] = ES_ID;
	priv->streamCount ++;
	//that's it, OD doesn't use decoderSpecificInfo
	return M4OK;
}


static M4Err OD_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	ODPriv *priv = plug->privateStack;

	//warning, we only support one stream ...
	if (!priv->streamCount) return M4BadParam;
	free(priv->ESIDs);
	priv->ESIDs = NULL;
	priv->streamCount = 0;

	return M4OK;
}


static M4Err OD_ProcessData(SceneDecoder *plug, unsigned char *inBuffer, u32 inBufferLength, 
							u16 ES_ID, u32 AU_time, u32 mmlevel)
{
	M4Err e;
	ODCommand *com;
	LPODCODEC oddec;
	ODPriv *priv = plug->privateStack;
	
	oddec = OD_NewCodec(OD_READ);

	e = OD_SetBuffer(oddec, inBuffer, inBufferLength);
	if (!e)  e = OD_DecodeAU(oddec);
	if (e) goto err_exit;

	//3- process all the commands in this AU, in order
	while (1) {
		com = OD_GetCommand(oddec);
		if (!com) break;

		//ok, we have a command
		switch (com->tag) {
		case ODUpdate_Tag:
			e = ODS_ODUpdate(priv, (ObjectDescriptorUpdate *) com);
			break;
		case ODRemove_Tag:
			e = ODS_RemoveOD(priv, (ObjectDescriptorRemove *) com);
			break;
		case ESDUpdate_Tag:
			e = ODS_UpdateESD(priv, (ESDescriptorUpdate *)com);
			break;
		case ESDRemove_Tag:
			e = ODS_RemoveESD(priv, (ESDescriptorRemove *)com);
			break;
		case IPMPDUpdate_Tag:
			e = M4NotSupported;
			break;
		case IPMPDRemove_Tag:
			e = M4NotSupported;
			break;
		/*should NEVER exist outside the file format*/
		case ESDRemove_Ref_Tag:
			e = M4NonCompliantBitStream;
			break;
		default:
			if (com->tag >= ISO_RESERVED_COMMANDS_BEGIN && com->tag <= ISO_RESERVED_COMMANDS_END) {
				e = M4ISOForbiddenDescriptor;
			} else {
				/*we don't process user commands*/
				e = M4OK;
			}
			break;
		}
		OD_DeleteCommand(&com);
		if (e) return e;
	}

err_exit:
	OD_DeleteCodec(oddec);
	return e;
}


Bool OD_CanHandleStream(BaseDecoder *ifce, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	ODPriv *priv = ifce->privateStack;
	if (StreamType==M4ST_OD) {
		priv->PL = PL;
		return 1;
	}
	return 0;
}


void DeleteODDec(BaseDecoder *plug)
{
	ODPriv *priv = plug->privateStack;
	if (priv->ESIDs) free(priv->ESIDs);
	free(priv);
	free(plug);
}

BaseDecoder *NewODDec()
{
	SceneDecoder *tmp;
	ODPriv *priv;
	
	SAFEALLOC(tmp, sizeof(SceneDecoder));
	if (!tmp) return NULL;
	SAFEALLOC(priv, sizeof(ODPriv));

	tmp->privateStack = priv;
	tmp->AttachStream = OD_AttachStream;
	tmp->DetachStream = OD_DetachStream;
	tmp->GetCapabilities = OD_GetCapabilities;
	tmp->SetCapabilities = OD_SetCapabilities;
	tmp->ProcessData = OD_ProcessData;
	tmp->AttachScene = OD_AttachScene;
	tmp->CanHandleStream = OD_CanHandleStream;

	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC OD Decoder", "gpac distribution", 0)
	return (BaseDecoder *) tmp;
}


Bool QueryInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		return 1;
	default:
		return 0;
	}
}

void *LoadInterface(u32 InterfaceType)
{
	switch (InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		return NewODDec();
	default:
		return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		DeleteODDec(ifcd);
		break;
	}
}

