/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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


#include <intern/m4_esm_dev.h>
#include "MediaMemory.h"
#include "MediaControl.h"

#include "InputSensor.h"

/*math.h is not included in main config (C++ clash on win32)*/
#include <math.h>


/*removes the channel ressources and destroy it*/
void ODM_DeleteChannel(ODManager *odm, struct _es_channel *ch);

ODManager *NewODManager()
{
	ODManager *tmp = malloc(sizeof(ODManager));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(ODManager));
	tmp->channels = NewChain();

	tmp->Audio_PL = -1;
	tmp->Graphics_PL = -1;
	tmp->OD_PL = -1;
	tmp->Scene_PL = -1;
	tmp->Visual_PL = -1;
	tmp->ProfileInlining = 0;
	tmp->ms_stack = NewChain();
	tmp->mc_stack = NewChain();
	return tmp;
}

void ODM_Delete(ODManager *odm)
{
	u32 i, count;
	count = ChainGetCount(odm->ms_stack);
	for (i = 0; i<count; i++) {
		MediaSensorStack *media_sens = ChainGetEntry(odm->ms_stack, i);
		MS_Stop(media_sens);
		/*and detach from stream object*/
		media_sens->is_init = 0;
	}
	if (odm->mo) odm->mo->odm = NULL;

	DeleteChain(odm->channels);
	DeleteChain(odm->ms_stack);
	DeleteChain(odm->mc_stack);
	OD_DeleteDescriptor((Descriptor **)&odm->OD);
	
	assert (!odm->net_service);
	free(odm);
}

void ODM_RemoveOD(ODManager *odm)
{
	ODManager *t;
	Channel *ch;

	ODM_Stop(odm, 1);

	/*disconnect sub-scene*/
	if (odm->subscene) IS_Disconnect(odm->subscene);

	/*remove remote OD if any - break links to avoid distroying twice the parent ODM*/
	if (odm->remote_OD) {
		t = odm->remote_OD;
		if (t->net_service && (t->net_service->owner != t)) t->net_service->nb_odm_users--;
		odm->remote_OD = NULL;
		t->parent_OD = NULL;
		ODM_RemoveOD(t);
	}

	/*then delete all the OD channels associated with this service*/
	while (ChainGetCount(odm->channels)) {
		ch = ChainGetEntry(odm->channels, 0);
		ODM_DeleteChannel(odm, ch);
	}

	if (odm->net_service) {
		if (odm->net_service->owner == odm) {
			if (odm->net_service->nb_odm_users) odm->net_service->nb_odm_users--;
			/*detach it!!*/
			odm->net_service->owner = NULL;
			/*try to assign a new root in case this is not scene shutdown*/
			if (odm->net_service->nb_odm_users && odm->parentscene) {
				u32 i;
				for (i=0; i<ChainGetCount(odm->parentscene->ODlist); i++) {
					ODManager *new_root = ChainGetEntry(odm->parentscene->ODlist, i);
					if (new_root == odm) continue;
					while (new_root->remote_OD) new_root = new_root->remote_OD;
					if (new_root->net_service != odm->net_service) continue;
					new_root->net_service->owner = new_root;
					break;
				}
			}
		}
		if (!odm->net_service->nb_odm_users) Term_CloseService(odm->term, odm->net_service);
		odm->net_service = NULL;
	}

	/*last thing to do, unload the decoders if no channels associated*/
	if (odm->codec) {
		assert(!ChainGetCount(odm->codec->inChannels));
		MM_RemoveCodec(odm->term->mediaman, odm->codec);
		DeleteCodec(odm->codec);
	}
	if (odm->ocr_codec) {
		assert(!ChainGetCount(odm->ocr_codec->inChannels));
		MM_RemoveCodec(odm->term->mediaman, odm->ocr_codec);
		DeleteCodec(odm->ocr_codec);
	}
	if (odm->oci_codec) {
		assert(!ChainGetCount(odm->oci_codec->inChannels));
		MM_RemoveCodec(odm->term->mediaman, odm->oci_codec);
		DeleteCodec(odm->oci_codec);
	}

	/*delete from the parent scene.*/
	if (odm->parentscene) {
		IS_RemoveOD(odm->parentscene, odm);
		if (odm->subscene) IS_Delete(odm->subscene);
		if (odm->parent_OD) odm->parent_OD->remote_OD = NULL;
		ODM_Delete(odm);
		return;
	}
	
	/*this is the scene root OD (may be a remote OD ..) */
	if (odm->term->root_scene) {
		M4Event evt;
		assert(odm->term->root_scene == odm->subscene);
		IS_Delete(odm->subscene);
		/*reset main pointer*/
		odm->term->root_scene = NULL;

		evt.type = M4E_CONNECT;
		evt.connect.is_connected = 0;
		M4USER_SENDEVENT(odm->term->user, &evt);
	}

	/*delete the ODMan*/
	ODM_Delete(odm);
}

static ObjectDescriptor *ODM_GetOD(ODManager *odm, const char *uuData, u32 uuDataLength)
{
	Descriptor *desc;
	InitialObjectDescriptor *iod;
	ObjectDescriptor *od;

	desc = NULL;
	if (OD_ReadDesc((char *) uuData, (u32) uuDataLength, &desc)) {
		return NULL;
	}

	switch (desc->tag) {
	case InitialObjectDescriptor_Tag:
		iod = (InitialObjectDescriptor *)desc;
		/*Check P&Ls of this IOD*/
		odm->Audio_PL = iod->audio_profileAndLevel;
		odm->Graphics_PL = iod->graphics_profileAndLevel;
		odm->OD_PL = iod->OD_profileAndLevel;
		odm->Scene_PL = iod->scene_profileAndLevel;
		odm->Visual_PL = iod->visual_profileAndLevel;
		odm->ProfileInlining = iod->inlineProfileFlag;

		/*then translate to an OD*/
		od = malloc(sizeof(ObjectDescriptor));
		od->ESDescriptors = iod->ESDescriptors;
		iod->ESDescriptors = NULL;
		od->extensionDescriptors = iod->extensionDescriptors;
		iod->extensionDescriptors = NULL;
		od->IPMPDescriptorPointers = iod->IPMPDescriptorPointers;
		iod->IPMPDescriptorPointers = NULL;
		od->objectDescriptorID = iod->objectDescriptorID;
		od->OCIDescriptors = iod->OCIDescriptors;
		iod->OCIDescriptors = NULL;
		od->tag = ObjectDescriptor_Tag;
		od->URLString = iod->URLString;
		iod->URLString = NULL;
		free(iod);
		return od;
	case ObjectDescriptor_Tag:
		odm->Audio_PL = odm->Graphics_PL = odm->OD_PL = odm->Scene_PL = odm->Visual_PL = -1;
		odm->ProfileInlining = 0;
		return (ObjectDescriptor *)desc;
	default:
		OD_DeleteDescriptor(&desc);
		return NULL;
	}
}

/*setup service for OD (extract IOD and go)*/
void ODM_SetupService(ODManager *odm, const char *sub_url)
{
	char *iod;
	u32 iod_len;
	u32 od_type;
	M4Err e;
	ODManager *par;
	M4Client *term;

	assert(odm->OD==NULL);

	odm->net_service->nb_odm_users++;
	od_type = NM_OD_UNDEF;

	/*for remote ODs, get expected OD type in case the service needs to generate the IOD on the fly*/
	par = odm;
	while (par->parent_OD) par = par->parent_OD;
	if (par->parentscene && par->OD && par->OD->URLString) {
		MediaObject *mo;
		mo = IS_FindObject(par->parentscene, par->OD->objectDescriptorID, par->OD->URLString);
		if (mo) od_type = mo->type;
	}
	iod = NULL;
	iod_len = 0;
	e = NM_Get_MPEG4_IOD(odm->net_service, od_type, sub_url, &iod, &iod_len);
	if (e) {
		M4_OnMessage(odm->term, odm->net_service->url, "MPEG4 Service Entry Point not found", e);
		goto err_exit;
	}

	odm->OD = ODM_GetOD(odm, iod, iod_len);
	free(iod);
	/*the OD was not given, shutdown the service*/
	if (!odm->OD) {
		M4_OnMessage(odm->term, odm->net_service->url, "MPEG4 Service Setup Failure", M4InvalidDescriptor);
		goto err_exit;
	}
	
	/*keep track of term since the setup may fail and the OD may be destroyed*/
	term = odm->term;
	Term_LockNet(term, 1);
	ODM_SetupOD(odm, odm->net_service);
	Term_LockNet(term, 0);

	return;

err_exit:
	if (!odm->parentscene) {
		M4Event evt;
		evt.type = M4E_CONNECT;
		evt.connect.is_connected = 0;
		M4USER_SENDEVENT(odm->term->user, &evt);
	}

}


/*locate ESD by ID*/
ESDescriptor *OD_GetStream(ObjectDescriptor *OD, u16 ESID)
{
	u32 i;
	ESDescriptor *esd;
	for (i=0; i<ChainGetCount(OD->ESDescriptors); i++) {
		esd = ChainGetEntry(OD->ESDescriptors, i);
		if (esd->ESID==ESID) return esd;
	}
	return NULL;
}

/*Validate the streams in this OD, and check if we have to setup an inline scene*/
M4Err ODM_ValidateOD(ODManager *odm, Bool *hasInline)
{
	u32 i;
	u16 es_id;
	ESDescriptor *esd, *base_scene;
	char *sOpt;
	LanguageDescriptor *ld;
	u32 defSt, lang;
	u32 Scene_st = 0, ODst = 0, OCRst = 0, IPMPst = 0, OCIst = 0, MPGJst = 0, MPG7st = 0, otherst = 0, nbStreamType = 0, prevST = 0, indepst = 0, uiST = 0, nbDep = 0;

	*hasInline = 0;
	base_scene = NULL;

	for (i = 0; i < ChainGetCount(odm->OD->ESDescriptors); i++) {
		esd = (ESDescriptor *)ChainGetEntry(odm->OD->ESDescriptors, i);

		switch (esd->decoderConfig->streamType) {
		case M4ST_OD:
			ODst++;
			if (esd->decoderConfig->objectTypeIndication == GPAC_STATIC_OD_OTI) Scene_st++;
			break;
		case M4ST_OCR:
			OCRst++;
			break;
		case M4ST_SCENE:
			Scene_st++;
			if (!base_scene) base_scene = esd;
			break;
		case M4ST_MPEG7:
			MPG7st++;
			break;
		case M4ST_IPMP:
			IPMPst++;
			break;
		case M4ST_OCI:
			OCIst++;
			break;
		case M4ST_MPEGJ:
			MPGJst++;
			break;
		case M4ST_INTERACT:
			uiST++;
			break;
		/*user-priv used for text loaders*/
		case M4ST_PRIVATE_SCENE:
			Scene_st++;
			if (!base_scene) base_scene = esd;
			break;
		default:
			otherst++;
			/*check that we have the same StreamType*/
			if (esd->decoderConfig->streamType != prevST) nbStreamType++;
			prevST = esd->decoderConfig->streamType;
			/*check for independant streams*/
			if (! esd->dependsOnESID) 
				indepst++;
			else
				nbDep++;
		}
	}
	if ((nbStreamType > 1) || (OCRst > 1) || (OCIst > 1) || (!Scene_st && ODst)) {
		return M4InvalidDescriptor;
	}

	/*language selection (just for fun)*/
	if (indepst > 1) {
		sOpt = IF_GetKey(odm->term->user->config, "General", "Language");
		if (!sOpt) {
			IF_SetKey(odm->term->user->config, "General", "Language", "und");
			sOpt = "und";
		}
		lang = sOpt[0]<<24 | sOpt[1] << 16 | sOpt[2];

		/*locate first base stream matching lang or first lang*/
		defSt = 0;
		for (i = 0; i < ChainGetCount(odm->OD->ESDescriptors); i++) {
			esd = (ESDescriptor *)ChainGetEntry(odm->OD->ESDescriptors, i);
			if (esd->decoderConfig->streamType != prevST) continue;
			if (!ChainGetCount(esd->langDesc)) {
				if (!defSt) defSt = esd->ESID;
				continue;
			}
			ld = ChainGetEntry(esd->langDesc, 0);
			if (ld->langCode==lang) {
				defSt = esd->ESID;
				break;
			}
		}
		assert(defSt);

		/*remove all other media streams*/
		for (i = 0; i < ChainGetCount(odm->OD->ESDescriptors); i++) {
			esd = (ESDescriptor *)ChainGetEntry(odm->OD->ESDescriptors, i);
			if (esd->decoderConfig->streamType != prevST) continue;
			/*not the right stream, remove*/
			if ((esd->ESID != defSt) && (esd->dependsOnESID!=defSt)) {
				ChainDeleteEntry(odm->OD->ESDescriptors, i);
				i--;
				OD_DeleteDescriptor((Descriptor**) &esd);
			}
		}
	
	}

	/* and we don't handle MPEGJ nor MPEG-7*/
	if (MPGJst || MPG7st) return M4NotSupported;

	if (!Scene_st) return M4OK;
	
	*hasInline = 1;
	/*we have a scene stream without dependancies, this is an inline*/
	if (!base_scene || !base_scene->dependsOnESID) return M4OK;

	/*if the stream the base scene depends on is in this OD, this is in inline*/
	es_id = base_scene->dependsOnESID;
	while (es_id) {
		esd = OD_GetStream(odm->OD, es_id);
		/*the stream this stream depends on is not in this OD, this is some anim stream*/
		if (!esd) {
			*hasInline = 0;
			return M4OK;
		}
		es_id = esd->dependsOnESID;
		/*should be forbidden (circular reference), we assume this describes inline (usually wrong BIFS->OD setup)*/
		if (es_id==base_scene->ESID) break;
	}
	/*no dependency to external stream, this is an inline*/
	return M4OK;
}



/*connection of OD and setup of streams. The streams are not requested if the OD
is in an unexecuted state
the ODM is created either because of IOD / remoteOD, or by the OD codec. In the later
case, the InlineScene pointer will be set by the OD codec.*/
void ODM_SetupOD(ODManager *odm, LPNETSERVICE serv)
{
	Bool hasInline;
	u32 i, numOK;
	M4Err e;

	if (!odm->net_service) odm->net_service = serv;
	
	/*if this is a remote OD, we need a new manager and a new service...*/
	if (odm->OD->URLString) {
		odm->remote_OD = NewODManager();
		odm->remote_OD->term = odm->term;
		/*assign parent OD*/
		odm->remote_OD->parent_OD = odm;
		/*assign parent scene (regular remote OD)*/
		if (odm->parentscene) {
			odm->remote_OD->parentscene = odm->parentscene;
		}
		/*assign subscene (root remote OD)*/
		else {
			odm->remote_OD->subscene = odm->subscene;
		}

		Term_ConnectODManager(odm->term, odm->remote_OD, odm->OD->URLString, odm->net_service);
		return;
	}


	e = ODM_ValidateOD(odm, &hasInline);
	if (e) {
		M4_OnMessage(odm->term, odm->net_service->url, "MPEG-4 Service Error", e);
		ODM_RemoveOD(odm);
		return;
	}

	/*if there is a BIFS stream in the OD, we need an InlineScene (except if we already 
	have one, which means this is the first IOD)*/
	if (hasInline && !odm->subscene) {
		odm->subscene = NewInlineScene(odm->parentscene);
		odm->subscene->root_od = odm;
		SG_SetJavaScriptAPI(odm->subscene->graph, &odm->term->js_ifce);
	}

	numOK = odm->pending_channels = 0;

	/*avoid channels PLAY request when confirming connection (sync network service)*/
	odm->is_open = 2;

	for (i=0; i<ChainGetCount(odm->OD->ESDescriptors); i++) {
		ESDescriptor *esd = ChainGetEntry(odm->OD->ESDescriptors, i);
		e = ODM_SetupStream(odm, esd, serv);
		/*notify error but still go on, all streams are not so usefull*/
		if (e==M4OK) 
			numOK++;
		else
			M4_OnMessage(odm->term, odm->net_service->url, "Stream Setup Failure", e);

	}
	odm->is_open = 0;

	/*special case for ODs only having OCRs: force a START since they're never refered to by media nodes*/
	if (odm->ocr_codec) ODM_Start(odm);

#if 0
	/*clean up - note that this will not be performed if one of the stream is using ESD URL*/
	if (!numOK) {
		ODM_RemoveOD(odm);
		return;
	}
#endif
	
	/*setup mediaobject info except for top-level OD*/
	if (odm->parentscene) {
		IS_SetupOD(odm->parentscene, odm);
	} else {
		/*othewise send a connect ack for top level*/
		M4Event evt;
		evt.type = M4E_CONNECT;
		evt.connect.is_connected = 1;
		M4USER_SENDEVENT(odm->term->user, &evt);
	}

	/* and connect ONLY if main scene - inlines are connected when attached to Inline nodes*/
	if (!odm->parentscene) {
		ODManager *root = odm->subscene->root_od;
		assert(odm->subscene == odm->term->root_scene);
		while (root->remote_OD) root = root->remote_OD;
		if (root == odm) ODM_Start(odm);
	}

	
	/*for objects inserted by user (subs & co), auto select*/
	if (odm->term->root_scene->is_dynamic_scene && odm->parent_OD) {
		ODManager *par = odm->parent_OD;
		while (par->parent_OD) par = par->remote_OD;
		if (par->OD->objectDescriptorID==DYNAMIC_OD_ID) {
			M4Event evt;
			if (par->OD_PL) {
				IS_SelectODM(odm->term->root_scene, odm);
				par->OD_PL = 0;
			}
			evt.type = M4E_STREAMLIST;
			M4USER_SENDEVENT(odm->term->user,&evt);
		}
	}
}

/*refresh all ODs when an non-interactive stream is found*/
void ODM_RefreshNonInteractives(ODManager *odm)
{
	u32 i, j;
	Channel *ch;
	ODManager *test_od;
	InlineScene *in_scene;

	/*check for inline*/
	in_scene = odm->parentscene;
	if (odm->subscene && odm->subscene->root_od == odm) {
		in_scene = odm->subscene;
		for (j=0; j<ChainGetCount(odm->channels); j++) {
			ch = ChainGetEntry(odm->channels, j);
			if (ch->clock->no_time_ctrl) {
				odm->no_time_ctrl = 1;
				break;
			}
		}
	}

	for (i=0; i<ChainGetCount(in_scene->ODlist); i++) {
		test_od = ChainGetEntry(in_scene->ODlist, i);
		if (odm==test_od) continue;
		for (j=0; j<ChainGetCount(test_od->channels); j++) {
			ch = ChainGetEntry(test_od->channels, j);
			if (ch->clock->no_time_ctrl) {
				test_od->no_time_ctrl = 1;
				break;
			}
		}
	}
}

void ODM_CheckChannelService(Channel *ch)
{
	if (ch->service == ch->odm->net_service) return;
	/*if the stream has created a service check if close is needed or not*/
	if (ch->esd->URLString && !ch->service->nb_ch_users) Term_CloseService(ch->odm->term, ch->service);
}

/*setup channel, clock and query caps*/
M4Err ODM_SetupStream(ODManager *odm, ESDescriptor *esd, LPNETSERVICE serv)
{
	CapObject cap;
	Channel *ch;
	Clock *ck;
	Chain *ck_namespace;
	GenericCodec *dec;
	s8 flag;
	u16 clockID;
	M4Err e;
	InlineScene *is;

	/*find the clock for this new channel*/
	ck = NULL;
	flag = -1;

	/*get clocks namespace (eg, parent scene)*/
	is = odm->subscene ? odm->subscene : odm->parentscene;

	ck_namespace = odm->net_service->Clocks;
	/*little trick for non-OD addressing: if object is a remote one, and service owner already has clocks, 
	override OCR. This will solve addressing like file.avi#audio and file.avi#video*/
	if (odm->parent_OD && (ChainGetCount(ck_namespace)==1) ) {
		ck = ChainGetEntry(ck_namespace, 0);
		esd->OCRESID = ck->clockID;
	}
	/*for dynamic scene, force all streams to be sync on main OD stream (one timeline, no need to reload ressources)*/
	else if (odm->term->root_scene->is_dynamic_scene) {
		ODManager *root_od = odm->term->root_scene->root_od;
		while (root_od->remote_OD) root_od = root_od->remote_OD;
		if (ChainGetCount(root_od->net_service->Clocks)==1) {
			ck = ChainGetEntry(root_od->net_service->Clocks, 0);
			esd->OCRESID = ck->clockID;
			goto clock_setup;
		}
	}

	/*do we have an OCR specified*/
	clockID = esd->OCRESID;
	/*if OCR stream force self-synchro !!*/
	if (esd->decoderConfig->streamType==M4ST_OCR) clockID = esd->ESID;
	if (!clockID) {
		/*if no clock ID but depandancy, force the clock to be the base layer for AV but not systems (animation streams, ..)*/
		if ((esd->decoderConfig->streamType==M4ST_VISUAL) || (esd->decoderConfig->streamType==M4ST_AUDIO)) clockID = esd->dependsOnESID;
		if (!clockID) clockID = esd->ESID;
	}

	/*override clock dependencies if specified*/
	if (odm->term->force_single_clock) {
		if (is->scene_codec) {
			clockID = is->scene_codec->ck->clockID;
		} else if (is->od_codec) {
			clockID = is->od_codec->ck->clockID;
		}
		ck_namespace = odm->term->root_scene->root_od->net_service->Clocks;
	}
	/*if the Clock is the stream, check if we have embedded OCR in the stream...*/
	if (clockID == esd->ESID) {
		flag = (esd->slConfig && esd->slConfig->OCRLength > 0);
	}

	if (!esd->slConfig) {
		esd->slConfig = (SLConfigDescriptor *)OD_NewDescriptor(SLConfigDescriptor_Tag);
		esd->slConfig->timestampResolution = 1000;
	}

	/*attach clock in namespace*/
	ck = CK_AttachClock(ck_namespace, is, clockID, esd->ESID, flag);
	if (!ck) return M4OutOfMem;
	esd->OCRESID = ck->clockID;

clock_setup:
	/*create a channel for this stream*/
	ch = NewChannel(esd);
	if (!ch) return M4OutOfMem;
	ch->clock = ck;
	ch->service = serv;

	/*setup the decoder for this stream or find the existing one.*/
	e = M4OK;
	dec = NULL;
	switch (esd->decoderConfig->streamType) {
	case M4ST_OD:
		//OD - MUST be in inline
		if (!odm->subscene) {
			e = M4NonCompliantBitStream;
			break;
		}

		/*OD codec acts as main scene codec when used to generate scene graph*/
		if (esd->decoderConfig->objectTypeIndication==GPAC_STATIC_OD_OTI) {
			dec = odm->subscene->scene_codec = NewDecoder(odm, esd, odm->OD_PL, &e);
			MM_AddCodec(odm->term->mediaman, odm->subscene->scene_codec);
			odm->subscene->is_dynamic_scene = 1;
			dec->flags |= COD_IS_STATIC_OD;
		} else if (! odm->subscene->od_codec) {
			dec = odm->subscene->od_codec = NewDecoder(odm, esd, odm->OD_PL, &e);
			MM_AddCodec(odm->term->mediaman, odm->subscene->od_codec);
		}
		break;
	case M4ST_OCR:
		/*OD codec acts as main scene codec when used to generate scene graph*/
		dec = odm->ocr_codec = NewDecoder(odm, esd, odm->OD_PL, &e);
		MM_AddCodec(odm->term->mediaman, odm->ocr_codec);
		break;
	case M4ST_SCENE:
		/*animationStream */
		if (!odm->subscene) {
			if (!odm->codec) {
				odm->codec = NewDecoder(odm, esd, odm->Scene_PL, &e);
				MM_AddCodec(odm->term->mediaman, odm->codec);
			}
			dec = odm->codec;
		}
		/*inline scene*/
		else {
			if (! odm->subscene->scene_codec) {
				odm->subscene->scene_codec = NewDecoder(odm, esd, odm->Scene_PL, &e);
				MM_AddCodec(odm->term->mediaman, odm->subscene->scene_codec);
			}
			dec = odm->subscene->scene_codec;
		}
		break;
	case M4ST_OCI:
		/*OCI - only one per OD */
		if (odm->oci_codec) {
			e = M4NonCompliantBitStream;
		} else {
			odm->oci_codec = NewDecoder(odm, esd, odm->OD_PL, &e);
			odm->oci_codec->odm = odm;
			MM_AddCodec(odm->term->mediaman, odm->oci_codec);
		}
		break;

	case M4ST_AUDIO:
	case M4ST_VISUAL:
		/*we have a media or user-specific codec...*/
		if (!odm->codec) {
			odm->codec = NewDecoder(odm, esd, (esd->decoderConfig->streamType==M4ST_VISUAL) ? odm->Visual_PL : odm->Audio_PL, &e);
			if (!e) MM_AddCodec(odm->term->mediaman, odm->codec);
		}
		dec = odm->codec;
		break;

	/*interaction stream*/
	case M4ST_INTERACT:
		if (!odm->codec) {
			odm->codec = NewDecoder(odm, esd, odm->OD_PL, &e);
			if (!e) {
				IS_Configure(odm->codec->decio, odm->parentscene, esd->URLString ? 1 : 0);
				MM_AddCodec(odm->term->mediaman, odm->codec);
				/*register it*/
				ChainAddEntry(odm->term->input_streams, odm->codec);
			}
		}
		dec = odm->codec;
		break;

	case M4ST_PRIVATE_SCENE:
		if (odm->subscene) {
			assert(!odm->subscene->scene_codec);
			odm->subscene->scene_codec = NewDecoder(odm, esd, odm->Scene_PL, &e);
			if (odm->subscene->scene_codec) {
				MM_AddCodec(odm->term->mediaman, odm->subscene->scene_codec);
			}
			dec = odm->subscene->scene_codec;
		} else {
			/*this is a bit tricky: the scene decoder needs to ba called with the dummy streams of this 
			object, so we associate the main decoder to this object*/
			odm->codec = dec = Codec_UseDecoder(odm->parentscene->scene_codec, odm);
			MM_AddCodec(odm->term->mediaman, odm->codec);
		}
		break;
	/*all other cases*/
	default:
		if (!odm->codec) {
			odm->codec = NewDecoder(odm, esd, odm->OD_PL, &e);
			if (!e) MM_AddCodec(odm->term->mediaman, odm->codec);

		}
		dec = odm->codec;
		break;
	}

	/*if we have a decoder, set up the channel and co.*/
	if (!dec) {
		if (e) {
			DeleteChannel(ch);
			return e;
		}
	}

	/*setup scene decoder*/
	if (dec->decio && (dec->decio->InterfaceType==M4SCENEDECODERINTERFACE) ) {
		SceneDecoder *sdec = (SceneDecoder *) dec->decio;
		is = odm->subscene ? odm->subscene : odm->parentscene;
		if (sdec->AttachScene) sdec->AttachScene(sdec, is, (is->scene_codec==dec) ? 1: 0);
	}

	ch->es_state = ES_Setup;
	ch->odm = odm;

	/*one more channel to wait for*/
	odm->pending_channels++;

	/*get media padding BEFORE channel setup, since we use it on channel connect ack*/
	if (dec) {
		cap.CapCode = CAP_PADDING_BYTES;
		Codec_GetCap(dec, &cap);
		ch->media_padding_bytes = cap.cap.valueINT;

		cap.CapCode = CAP_HASRESILIENCE;
		Codec_GetCap(dec, &cap);
		ch->codec_resilient = cap.cap.valueINT;
	}

	/*service redirection*/
	if (esd->URLString) {
		ChannelSetup *cs;
		/*here we have a pb with the MPEG4 model: streams are supposed to be attachable as soon as the OD 
		update is recieved, but this is not true with ESD URLs, where service setup may take some time (file
		downloading, authentification, etc...). We therefore need to wait for the service connect response before 
		setting up the channel...*/
		cs = malloc(sizeof(ChannelSetup));
		cs->ch = ch;
		cs->dec = dec;
		Term_LockNet(odm->term, 1);
		ChainAddEntry(odm->term->channels_pending, cs);
		e = Term_ConnectChannelURL(odm->term, ch, esd->URLString);
		if (e) {
			s32 i = ChainFindEntry(odm->term->channels_pending, cs);
			if (i>=0) {
				ChainDeleteEntry(odm->term->channels_pending, (u32) i);
				free(cs);
				odm->pending_channels--;
				ODM_CheckChannelService(ch);
				DeleteChannel(ch);
			}
		}
		Term_LockNet(odm->term, 0);
		if (ch->service->owner) {
			ChainDeleteItem(odm->term->channels_pending, cs);
			free(cs);
			return ODM_SetupChannel(ch, dec, M4OK);
		}
		return e;
	}

	/*regular setup*/
	return ODM_SetupChannel(ch, dec, M4OK);
}

M4Err ODM_SetupChannel(Channel *ch, GenericCodec *dec, M4Err had_err)
{
	char szURL[2048];
	M4Err e;

	e = had_err;
	if (e) {
		ch->odm->pending_channels--;
		goto err_exit;
	}
	if (ch->esd->URLString) {
		strcpy(szURL, ch->esd->URLString);
	} else {
		sprintf(szURL, "ES_ID=%d", ch->esd->ESID);
	}


	/*insert channel*/
	if (dec) ChainInsertEntry(ch->odm->channels, ch, 0);

	ch->es_state = ES_WaitingForAck;

	/*connect before setup: this is needed in case the decoder cfg is wrong, we may need to get it from
	network config...*/
	e = NM_ConnectChannel(ch->service, ch, szURL, ch->esd->decoderConfig->upstream);

	if (e) {
		if (dec) ChainDeleteEntry(ch->odm->channels, 0);
		goto err_exit;
	}
	/*add to decoder*/
	if (dec) {
		e = Codec_AddChannel(dec, ch);
		if (e) {
			switch (ch->esd->decoderConfig->streamType) {
			case M4ST_VISUAL:
				M4_OnMessage(ch->odm->term, ch->service->url, "Video Setup failed", e);
				break;
			case M4ST_AUDIO:
				M4_OnMessage(ch->odm->term, ch->service->url, "Audio Setup failed", e);
				break;
			}
			ChainDeleteEntry(ch->odm->channels, 0);
			/*disconnect*/
			NM_DisconnectChannel(ch->service, ch);
			if (ch->esd->URLString) ch->service->nb_ch_users--;
			goto err_exit;
		}
	}

	/*in case a channel is inserted in a running OD, open and play if not in queue*/
	if (ch->odm->is_open==1) {
		Term_LockNet(ch->odm->term, 1);
		Channel_Start(ch);
		if (ChainFindEntry(ch->odm->term->od_pending, ch->odm)<0) {
			NetworkCommand com;
			com.command_type = CHAN_PLAY;
			com.base.on_channel = ch;
			com.play.speed = ch->clock->speed;
			com.play.start_range = CK_GetTime(ch->clock);
			com.play.start_range /= 1000;
			com.play.end_range = -1.0f;
			NM_ServiceCommand(ch->service, &com);
		}
		if (dec && (dec->Status!=CODEC_PLAY)) MM_StartCodec(dec);
		Term_LockNet(ch->odm->term, 0);
	}

	return M4OK;

err_exit:
	ODM_CheckChannelService(ch);
	DeleteChannel(ch);
	return e;
}

/*confirmation of channel delete from net*/
void ODM_DeleteChannel(ODManager *odm, Channel *ch)
{
	u32 i, count, ch_pos;
	Channel *ch2;
	Clock *ck;

	if (!ch) return;

	//find a clock with this stream ES_ID
	ck = CK_FindClock(odm->net_service->Clocks, ch->esd->ESID, 0);

	count = ChainGetCount(odm->channels);
	ch_pos = count+1;

	for (i=0; i<count; i++) {
		ch2 = ChainGetEntry(odm->channels, i);
		if (ch2 == ch) {
			ch_pos = i;	
			if (ck) continue;
			break;
		}
		//note that when a stream is added, we need to update clocks info ...
		if (ck && ch->clock && (ch2->clock->clockID == ck->clockID)) Channel_Stop(ch2);
	}
	/*remove channel*/
	if (ch_pos != count+1) ChainDeleteEntry(odm->channels, ch_pos);

	/*remove from the codec*/
	count = 0;
	if (!count && odm->codec) 
		count = Codec_RemoveChannel(odm->codec, ch);
	if (!count && odm->ocr_codec)
		count = Codec_RemoveChannel(odm->ocr_codec, ch);
	if (!count && odm->oci_codec)
		count = Codec_RemoveChannel(odm->oci_codec, ch);
	if (!count && odm->subscene) {
		if (odm->subscene->scene_codec) count = Codec_RemoveChannel(odm->subscene->scene_codec, ch);
		if (!count) count = Codec_RemoveChannel(odm->subscene->od_codec, ch);
	}
	assert(count);

	NM_DisconnectChannel(ch->service, ch);
	if (ch->esd->URLString) ch->service->nb_ch_users--;
	ODM_CheckChannelService(ch);

	//and delete
	DeleteChannel(ch);
}

void ODM_RemoveStream(ODManager *odm, u16 ES_ID)
{
	ESDescriptor *esd;
	Channel *ch;
	u32 i;
	for (i=0; i<ChainGetCount(odm->OD->ESDescriptors); i++) {
		esd = ChainGetEntry(odm->OD->ESDescriptors, i);
		if (esd->ESID==ES_ID) goto esd_found;
	}
	return;

esd_found:
	/*remove esd*/
	ChainDeleteEntry(odm->OD->ESDescriptors, i);
	/*locate channel*/
	ch = NULL;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i);
		if (ch->esd->ESID == ES_ID) break;
		ch = NULL;
	}
	/*destroy ESD*/
	OD_DeleteDescriptor((Descriptor **) &esd);
	/*remove channel*/
	if (ch) ODM_DeleteChannel(odm, ch);
}

/*this is the tricky part: make sure the net is locked before doing anything since an async service 
reply could destroy the object we're queuing for play*/
void ODM_Start(ODManager *odm)
{
	Term_LockNet(odm->term, 1);
	/*only if not open & ready (not waiting for ACK on channel setup)*/
	if (!odm->is_open && !odm->pending_channels) {
		u32 i;
		odm->is_open = 1;
		/*start all channels and postpone play - this assures that all channels of a multiplexed are setup
		before one starts playing*/
		for (i=0; i<ChainGetCount(odm->channels); i++) {
			Channel *ch = ChainGetEntry(odm->channels, i);
			Channel_Start(ch);
		}
		if (ChainFindEntry(odm->term->od_pending, odm)<0) ChainAddEntry(odm->term->od_pending, odm);
	}
	Term_LockNet(odm->term, 0);
}

void ODM_Play(ODManager *odm)
{
	u32 i;
	NetworkCommand com;
	MediaControlStack *ctrl;

	/*send play command*/
	com.command_type = CHAN_PLAY;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		Double ck_time;
		Channel *ch = ChainGetEntry(odm->channels, i);
		com.base.on_channel = ch;
		com.play.speed = 1.0;
		/*plays from current time*/
		ck_time = CK_GetTime(ch->clock);
		ck_time /= 1000;
		/*handle initial start - MPEG-4 is a bit annoying here, streams are not started through OD but through
		scene nodes. If the stream runs on the BIFS/OD clock, the clock is already started at this point and we're 
		sure to get at least a one-frame delay in PLAY, so just remove it - note we're generous (one second)
		but this shouldn't hurt*/
		if (ck_time<=1.0) ck_time = 0;
		com.play.start_range = ck_time;
		com.play.end_range = -1;
		/*override range and speed with MC - here we don't adjust since mediaControl is here to give us the exact
		media start time*/
		ctrl = ODM_GetMediaControl(odm);
		if (ctrl) {
			MC_GetRange(ctrl, &com.play.start_range, &com.play.end_range);
			com.play.speed = ctrl->control->mediaSpeed;
			/*if the channel doesn't control the clock, jump to current time in the controled range, not just the begining*/
			if ((ch->esd->ESID!=ch->clock->clockID) && (ck_time>com.play.start_range) && (com.play.end_range>com.play.start_range) && (ck_time<com.play.end_range)) {
				com.play.start_range = ck_time;
			}
			CK_SetSpeed(ch->clock, ctrl->control->mediaSpeed);
		}
		/*user-defined seek on top scene*/
		else if (odm->term->root_scene->root_od==odm) {
			com.play.start_range = odm->term->restart_time;
			com.play.start_range /= 1000.0;
		}
		/*full object playback*/
		if (com.play.end_range<=0) {
			odm->range_end = odm->duration;
		} else {
			/*segment playback - since our timing is in ms whereas segment ranges are double precision, 
			make sure we have a LARGER range in ms, otherwise media sensors won't deactivate properly*/
			odm->range_end = (u32) ceil(1000 * com.play.end_range);
		}
		NM_ServiceCommand(ch->service, &com);
	}
	/*if root OD reset the global seek time*/	
	if (odm->term->root_scene->root_od==odm) odm->term->restart_time = 0;


	/*start codecs last (otherwise we end up pulling data from channels not yet connected->pbs when seeking)*/
	if (odm->codec) {
		/*reset*/
		if (odm->codec->CB) {
			CB_SetStatus(odm->codec->CB, CB_STOP);
			odm->codec->CB->HasSeenEOS = 0;
		}
		MM_StartCodec(odm->codec);
	} else if (odm->subscene) {
		if (odm->subscene->scene_codec) MM_StartCodec(odm->subscene->scene_codec);
		if (odm->subscene->od_codec) MM_StartCodec(odm->subscene->od_codec);
	}
	if (odm->ocr_codec) MM_StartCodec(odm->ocr_codec);
	if (odm->oci_codec) MM_StartCodec(odm->oci_codec);
}


void ODM_Stop(ODManager *odm, Bool force_close)
{
	u32 i, count;
	MediaControlStack *ctrl;
	NetworkCommand com;
	
	ChainDeleteItem(odm->term->od_pending, odm);

	if (!odm->is_open) return;

	/*little opt for image codecs: don't actually stop the OD*/
	if (!force_close && odm->codec && odm->codec->CB) {
		if (odm->codec->CB->Capacity==1) return;
	}

	/*stop codecs*/
	if (odm->codec) {
		MM_StopCodec(odm->codec);
	} else if (odm->subscene) {
		if (odm->subscene->scene_codec) MM_StopCodec(odm->subscene->scene_codec);
		if (odm->subscene->od_codec) MM_StopCodec(odm->subscene->od_codec);
	}
	if (odm->ocr_codec) MM_StopCodec(odm->ocr_codec);
	if (odm->oci_codec) MM_StopCodec(odm->oci_codec);

	/*stop channels*/
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		Channel *ch = ChainGetEntry(odm->channels, i);
		Channel_Stop(ch);
	}

	/*send stop command*/
	com.command_type = CHAN_STOP;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		Channel *ch = ChainGetEntry(odm->channels, i);
		com.base.on_channel = ch;
		NM_ServiceCommand(ch->service, &com);
	}

	odm->is_open = 0;
	odm->current_time = 0;

	/*reset media sensor(s)*/
	count = ChainGetCount(odm->ms_stack);
	for (i = 0; i<count; i++) {
		MediaSensorStack *media_sens = ChainGetEntry(odm->ms_stack, i);
		MS_Stop(media_sens);
	}
	/*reset media control state*/
	ctrl = ODM_GetMediaControl(odm);
	if (ctrl) ctrl->current_seg = 0;

}

void ODM_EndOfStream(ODManager *odm, Channel *on_channel)
{
	if (ODM_CheckSegmentSwitch(odm)) return;
	
	if (odm->codec && (on_channel->esd->decoderConfig->streamType==odm->codec->type)) {
		Codec_SetStatus(odm->codec, CODEC_EOS);
		return;
	} 
	if (on_channel->esd->decoderConfig->streamType==M4ST_OCR) {
		Codec_SetStatus(odm->ocr_codec, CODEC_EOS);
		return;
	}
	if (on_channel->esd->decoderConfig->streamType==M4ST_OCI) {
		Codec_SetStatus(odm->oci_codec, CODEC_EOS);
		return;
	}
	if (!odm->subscene) return;

	if (odm->subscene->scene_codec && (ChainFindEntry(odm->subscene->scene_codec->inChannels, on_channel)>=0) ) {
		Codec_SetStatus(odm->subscene->scene_codec, CODEC_EOS);
		return;
	}

	if (on_channel->esd->decoderConfig->streamType==M4ST_OD) {
		Codec_SetStatus(odm->subscene->od_codec, CODEC_EOS);
		return;
	}
}

void ODM_SetDuration(ODManager *odm, Channel *ch, u32 stream_duration)
{
	if (odm->codec) {
		if (ch->esd->decoderConfig->streamType == odm->codec->type)
			if (odm->duration < stream_duration)
				odm->duration = stream_duration;
	} else if (odm->ocr_codec) {
		if (ch->esd->decoderConfig->streamType == odm->ocr_codec->type)
			if (odm->duration < stream_duration)
				odm->duration = stream_duration;
	} else if (odm->subscene && odm->subscene->scene_codec) {
		if (ChainFindEntry(odm->subscene->scene_codec->inChannels, ch) >= 0) {
			if (odm->duration < stream_duration) odm->duration = stream_duration;
		}
	}

	/*update scene duration*/
	IS_SetSceneDuration(odm->subscene ? odm->subscene : (odm->parentscene ? odm->parentscene : odm->term->root_scene));
}


Clock *ODM_GetMediaClock(ODManager *odm)
{
	if (odm->codec) return odm->codec->ck;
	if (odm->ocr_codec) return odm->ocr_codec->ck;
	if (odm->subscene && odm->subscene->scene_codec) return odm->subscene->scene_codec->ck;
	return NULL;
}


void ODM_SetMediaControl(ODManager *odm, MediaControlStack *ctrl)
{
	u32 i;
	Channel *ch;

	/*keep track of it*/
	if (ctrl && (ChainFindEntry(odm->mc_stack, ctrl) < 0)) ChainAddEntry(odm->mc_stack, ctrl);
	if (ctrl && !ctrl->control->enabled) return;

	/*for each clock in the controled OD*/
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i);
		if (ch->clock->mc != ctrl) {
			/*deactivate current control*/
			if (ch->clock->mc) {
				ch->clock->mc->control->enabled = 0;
				Node_OnEventOutSTR((SFNode *)ch->clock->mc->control, "enabled");
			}
			/*and attach this control to the clock*/
			ch->clock->mc = ctrl;
		}
	}
	/*store active control on media*/
	odm->media_ctrl = ODM_GetMediaControl(odm);
}

MediaControlStack *ODM_GetMediaControl(ODManager *odm)
{
	Clock *ck;
	ck = ODM_GetMediaClock(odm);
	if (!ck) return NULL;
	return ck->mc;
}

MediaControlStack *ODM_GetObjectMediaControl(ODManager *odm)
{
	MediaControlStack *ctrl;
	ctrl = ODM_GetMediaControl(odm);
	if (!ctrl) return NULL;
	/*inline scene control*/
	if (odm->subscene && (ctrl->stream->odm == odm->subscene->root_od) ) return ctrl;
	if (ctrl->stream->OD_ID != odm->OD->objectDescriptorID) return NULL;
	return ctrl;
}

void ODM_RemoveMediaControl(ODManager *odm, MediaControlStack *ctrl)
{
	ChainDeleteItem(odm->mc_stack, ctrl);
	/*removed. Note the spec doesn't say what to do in this case...*/
	if (odm->media_ctrl == ctrl) ODM_SetMediaControl(odm, NULL);
}

Bool ODM_SwitchMediaControl(ODManager *odm, MediaControlStack *ctrl)
{
	u32 i;
	if (!ctrl->control->enabled) return 0;

	/*for all media controls other than this one force enable to false*/
	for (i=0; i<ChainGetCount(odm->mc_stack); i++) {
		MediaControlStack *st2 = ChainGetEntry(odm->mc_stack, i);
		if (st2 == ctrl) continue;
		if (st2->control->enabled) {
			st2->control->enabled = 0;
			Node_OnEventOutSTR((SFNode *) st2->control, "enabled");
		}
		st2->enabled = 0;
	}
	if (ctrl == odm->media_ctrl) return 0;
	ODM_SetMediaControl(odm, ctrl);
	return 1;
}

Bool ODM_SharesClock(ODManager *odm, Clock *ck)
{
	u32 i;
	Channel *ch;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i);
		if (ch->clock == ck) return 1;
	}
	return 0;
}



void ODM_Pause(ODManager *odm)
{
	u32 i;
	NetworkCommand com;
	Channel *ch;

	if (odm->no_time_ctrl) return;


	/*stop codecs, and update status for media codecs*/
	if (odm->codec) {
		MM_StopCodec(odm->codec);
		Codec_SetStatus(odm->codec, CODEC_PAUSE);
	} else if (odm->subscene) {
		if (odm->subscene->scene_codec) {
			Codec_SetStatus(odm->subscene->scene_codec, CODEC_PAUSE);
			MM_StopCodec(odm->subscene->scene_codec);
		}
		if (odm->subscene->od_codec) MM_StopCodec(odm->subscene->od_codec);
	}
	if (odm->ocr_codec) MM_StopCodec(odm->ocr_codec);
	if (odm->oci_codec) MM_StopCodec(odm->oci_codec);

	com.command_type = CHAN_PAUSE;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i); 
		CK_Pause(ch->clock);
		com.base.on_channel = ch;
		NM_ServiceCommand(ch->service, &com);
	}

	/*mediaSensor  shall generate isActive false when paused*/
	for (i = 0; i<ChainGetCount(odm->ms_stack); i++) {
		MediaSensorStack *media_sens = ChainGetEntry(odm->ms_stack, i);
		if (media_sens && media_sens->sensor->isActive) {
			media_sens->sensor->isActive = 0;
			Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
		}
	}
}

void ODM_Resume(ODManager *odm)
{
	u32 i;
	NetworkCommand com;
	Channel *ch;

	if (odm->no_time_ctrl) return;


	/*start codecs, and update status for media codecs*/
	if (odm->codec) {
		MM_StartCodec(odm->codec);
		Codec_SetStatus(odm->codec, CODEC_PLAY);
	} else if (odm->subscene) {
		if (odm->subscene->scene_codec) {
			Codec_SetStatus(odm->subscene->scene_codec, CODEC_PLAY);
			MM_StartCodec(odm->subscene->scene_codec);
		}
		if (odm->subscene->od_codec) MM_StartCodec(odm->subscene->od_codec);
	}
	if (odm->ocr_codec) MM_StartCodec(odm->ocr_codec);
	if (odm->oci_codec) MM_StartCodec(odm->oci_codec);
	
	com.command_type = CHAN_RESUME;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i); 
		CK_Resume(ch->clock);
		com.base.on_channel = ch;
		NM_ServiceCommand(ch->service, &com);
	}

	/*mediaSensor shall generate isActive TRUE when resumed*/
	for (i = 0; i<ChainGetCount(odm->ms_stack); i++) {
		MediaSensorStack *media_sens = ChainGetEntry(odm->ms_stack, i);
		if (media_sens && !media_sens->sensor->isActive) {
			media_sens->sensor->isActive = 1;
			Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
		}
	}
}

void ODM_SetSpeed(ODManager *odm, Float speed)
{
	u32 i;
	NetworkCommand com;
	Channel *ch;

	if (odm->no_time_ctrl) return;

	com.command_type = CHAN_SET_SPEED;
	com.play.speed = speed;
	for (i=0; i<ChainGetCount(odm->channels); i++) {
		ch = ChainGetEntry(odm->channels, i); 
		CK_SetSpeed(ch->clock, speed);
		com.play.on_channel = ch;
		NM_ServiceCommand(ch->service, &com);
	}
}

SegmentDescriptor *ODM_GetSegment(ODManager *odm, char *descName)
{
	u32 i;
	for (i=0; i<ChainGetCount(odm->OD->OCIDescriptors); i++) {
		SegmentDescriptor *desc = ChainGetEntry(odm->OD->OCIDescriptors, i);
		if (desc->tag != SegmentDescriptor_Tag) continue;
		if (!stricmp(desc->SegmentName, descName)) return desc;
	}
	return NULL;
}

static void ODM_InsertSegment(ODManager *odm, SegmentDescriptor *seg, Chain *list)
{
	/*this reorders segments when inserting into list - I believe this is not compliant*/
#if 0
	u32 i;
	for (i=0; i<ChainGetCount(list); i++) {
		SegmentDescriptor *desc = ChainGetEntry(list, i);
		if (desc == seg) return;
		if (seg->startTime + seg->Duration <= desc->startTime) {
			ChainInsertEntry(list, seg, i);
			return;
		}
	}
#endif
	ChainAddEntry(list, seg);
}

/*add segment descriptor and sort them*/
void ODM_InitSegmentDescriptors(ODManager *odm, Chain *list, MFURL *url)
{
	char *str, *sep;
	char seg1[1024], seg2[1024], seg_url[4096];
	SegmentDescriptor *first_seg, *last_seg;
	u32 i, j;

	/*browse all URLs*/
	for (i=0; i<url->count; i++) {
		if (!url->vals[i].url) continue;
		str = strstr(url->vals[i].url, "#");
		if (!str) continue;
		str++;
		strcpy(seg_url, str);
		/*segment closed range*/
		if ((sep = strstr(seg_url, "-")) ) {
			strcpy(seg2, sep+1);
			sep[0] = 0;
			strcpy(seg1, seg_url);
			first_seg = ODM_GetSegment(odm, seg1);
			if (!first_seg) continue;
			last_seg = ODM_GetSegment(odm, seg2);
		} 
		/*segment open range*/
		else if ((sep = strstr(seg_url, "+")) ) {
			sep[0] = 0;
			strcpy(seg1, seg_url);
			first_seg = ODM_GetSegment(odm, seg_url);
			if (!first_seg) continue;
			last_seg = NULL;
		} 
		/*single segment*/
		else {
			first_seg = ODM_GetSegment(odm, seg_url);
			if (!first_seg) continue;
			ODM_InsertSegment(odm, first_seg, list);
			continue;
		}
		/*segment range process*/
		ODM_InsertSegment(odm, first_seg, list);
		for (j=0; j<ChainGetCount(odm->OD->OCIDescriptors); j++) {
			SegmentDescriptor *seg = ChainGetEntry(odm->OD->OCIDescriptors, j);
			if (seg->tag != SegmentDescriptor_Tag) continue;
			if (seg==first_seg) continue;
			if (seg->startTime + seg->Duration <= first_seg->startTime) continue;
			/*this also includes last_seg insertion !!*/
			if (last_seg && (seg->startTime + seg->Duration > last_seg->startTime + last_seg->Duration) ) continue;
			ODM_InsertSegment(odm, seg, list);
		}
	}
}

Bool ODM_CheckSegmentSwitch(ODManager *odm)
{
	u32 count, i;
	SegmentDescriptor *cur, *next;
	MediaControlStack *ctrl = ODM_GetMediaControl(odm);

	/*if no control or control not on this object ignore segment switch*/
	if (!ctrl || (ctrl->stream->odm != odm)) return 0;

	count = ChainGetCount(ctrl->seg);
	/*reached end of controled stream (no more segments)*/
	if (ctrl->current_seg>=count) return 0;

	/*synth media, trigger if end of segment run-time*/
	if (!odm->codec || ((odm->codec->type!=M4ST_VISUAL) && (odm->codec->type!=M4ST_AUDIO))) {
		Clock *ck = ODM_GetMediaClock(odm);
		u32 now = CK_GetTime(ck);
		u32 dur = odm->subscene ? odm->subscene->duration : odm->duration;
		cur = ChainGetEntry(ctrl->seg, ctrl->current_seg);
		if (odm->subscene && odm->subscene->needs_restart) return 0;
		if (cur) dur = (u32) ((cur->Duration+cur->startTime)*1000);
		if (now<=dur) return 0;
	} else {
		/*FIXME - for natural media with scalability, we should only process when all streams of the object are done*/
	}

	/*get current segment and move to next one*/
	cur = ChainGetEntry(ctrl->seg, ctrl->current_seg);
	ctrl->current_seg ++;

	/*resync in case we have been issuing a play range over several segments*/
	for (i=ctrl->current_seg; i<count; i++) {
		next = ChainGetEntry(ctrl->seg, i);
		if (
			/*if next seg start is after cur seg start*/
			(cur->startTime < next->startTime) 
			/*if next seg start is before cur seg end*/
			&& (cur->startTime + cur->Duration > next->startTime) 
			/*if next seg start is already passed*/
			&& (1000*next->startTime < odm->current_time)
			/*then next segment was taken into account when requesting play*/
			) {
			cur = next;
			ctrl->current_seg ++;
		}
	}
	/*if last segment in ctrl is done, end of stream*/
	if (ctrl->current_seg >= count) return 0;
	next = ChainGetEntry(ctrl->seg, ctrl->current_seg);

	/*if next seg start is not in current seg, media needs restart*/
	if ((next->startTime < cur->startTime) || (cur->startTime + cur->Duration < next->startTime))
		MC_Restart(odm);

	return 1;
}

