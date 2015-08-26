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



/*for OD service types*/
#include <m4_render.h>
#include <intern/m4_esm_dev.h>
#include "MediaControl.h"

void MO_UpdateCaps(MediaObject *mo);


/*extern proto fetcher*/
LPSCENEGRAPH IS_GetProtoLib(void *SceneCallback, MFURL *lib_url);
typedef struct
{
	MFURL *url;
	MediaObject *mo;
} ProtoLink;

Double IS_GetSceneTime(void *_is)
{
	Double ret;
	InlineScene *is = (InlineScene *)_is;
	assert(is);
	if (!is->scene_codec) return 0.0;
	ret = CK_GetTime(is->scene_codec->ck);
	ret/=1000.0;
	return ret;
}

InlineScene *NewInlineScene(InlineScene *parentScene)
{
	InlineScene *tmp = malloc(sizeof(InlineScene));
	if (! tmp) return NULL;
	memset(tmp, 0, sizeof(InlineScene));

	tmp->ODlist = NewChain();
	tmp->media_objects = NewChain();
	tmp->extern_protos = NewChain();
	tmp->inline_nodes = NewChain();
	tmp->extra_scenes = NewChain();
	/*init inline scene*/
	if (parentScene) {
		tmp->graph = SG_NewSubScene(parentScene->graph);
	} else {
		tmp->graph = NewSceneGraph();
	}
	SG_SetInitCallback(tmp->graph, Term_NodeInit, tmp);
	SG_SetModifiedCallback(tmp->graph, Term_NodeModified, tmp);
	SG_SetPrivate(tmp->graph, tmp);
	SG_SetSceneTimeCallback(tmp->graph, IS_GetSceneTime, tmp);
	SG_SetProtoLoader(tmp->graph, IS_GetProtoLib);
	return tmp;
}

void IS_Delete(InlineScene *is)
{
	DeleteChain(is->ODlist);
	DeleteChain(is->inline_nodes);
	assert(!ChainGetCount(is->extra_scenes) );
	DeleteChain(is->extra_scenes);

	while (ChainGetCount(is->extern_protos)) {
		ProtoLink *pl = ChainGetEntry(is->extern_protos, 0);
		ChainDeleteEntry(is->extern_protos, 0);
		free(pl);
	}
	DeleteChain(is->extern_protos);

	/*delete scene decoder */
	if (is->scene_codec) {
		SceneDecoder *dec = (SceneDecoder *)is->scene_codec->decio;
		/*make sure the scene codec doesn't have anything left in the scene graph*/
		if (dec->ReleaseScene) dec->ReleaseScene(dec);

		MM_RemoveCodec(is->root_od->term->mediaman, is->scene_codec);
		DeleteCodec(is->scene_codec);
		/*reset pointer to NULL in case nodes try to access scene time*/
		is->scene_codec = NULL;
	}

	/*delete the scene graph*/
	SG_Delete(is->graph);

	if (is->od_codec) {
		MM_RemoveCodec(is->root_od->term->mediaman, is->od_codec);
		DeleteCodec(is->od_codec);
		is->od_codec = NULL;
	}
	/*don't touch the root_od, will be deleted by the parent scene*/

	/*clean all remaining associations*/
	while (ChainGetCount(is->media_objects)) {
		MediaObject *obj = ChainGetEntry(is->media_objects, 0);
		if (obj->odm) obj->odm->mo = NULL;
		ChainDeleteEntry(is->media_objects, 0);
		VRML_MF_Reset(&obj->URLs, FT_MFURL);
		free(obj);
	}
	DeleteChain(is->media_objects);

	if (is->audio_url.url) free(is->audio_url.url);
	if (is->visual_url.url) free(is->visual_url.url);
	if (is->text_url.url) free(is->text_url.url);
	free(is);
}

ODManager *IS_FindODM(InlineScene *is, u16 OD_ID)
{
	u32 i;
	ODManager *odm;
	//browse the OD List only
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		odm = ChainGetEntry(is->ODlist, i);
		if (odm->OD->objectDescriptorID == OD_ID) return odm;
	}
	return NULL;
}

void IS_Disconnect(InlineScene *is)
{
	SFNode *root_node;
	ODManager *odm;
	SceneDecoder *dec = NULL;
	if (is->scene_codec) dec = (SceneDecoder *)is->scene_codec->decio;

	if (is->graph_attached) {
		root_node = SG_GetRootNode(is->graph);
		while (ChainGetCount(is->inline_nodes)) {
			SFNode *n = ChainGetEntry(is->inline_nodes, 0);
			ChainDeleteEntry(is->inline_nodes, 0);
			Node_Unregister(root_node, n);
		}
	}
	/*release the scene*/
	if (dec && dec->ReleaseScene) dec->ReleaseScene(dec);
	SG_Reset(is->graph);
	is->graph_attached = 0;
	
	while (ChainGetCount(is->ODlist)) {
		odm = ChainGetEntry(is->ODlist, 0);
		ODM_RemoveOD(odm);
	}
	assert(!ChainGetCount(is->extra_scenes) );


	/*remove stream associations*/
	while (ChainGetCount(is->media_objects)) {
		MediaObject *obj = ChainGetEntry(is->media_objects, 0);
		ChainDeleteEntry(is->media_objects, 0);
		if (obj->odm) obj->odm->mo = NULL;
		VRML_MF_Reset(&obj->URLs, FT_MFURL);
		free(obj);
	}
}

static void IS_InsertObject(InlineScene *is, MediaObject *mo)
{
	ODManager *root_od;
	ODManager *odm;
	if (!mo || !is) return;

	odm = NewODManager();
	/*remember OD*/
	odm->mo = mo;
	mo->odm = odm;
	odm->parentscene = is;
	odm->OD = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
	odm->OD->objectDescriptorID = DYNAMIC_OD_ID;
	odm->OD->URLString = strdup(mo->URLs.vals[0].url);
	odm->parentscene = is;
	odm->term = is->root_od->term;
	root_od = is->root_od;
	ChainAddEntry(is->ODlist, odm);
	while (root_od->remote_OD) root_od = root_od->remote_OD;
	ODM_SetupOD(odm, root_od->net_service);
}

static void IS_ReinsertObject(InlineScene *is, MediaObject *mo)
{
	u32 i;
	free(mo->URLs.vals[0].url);
	mo->URLs.vals[0].url = NULL;
	for (i=0; i<mo->URLs.count-1; i++) mo->URLs.vals[i].url = mo->URLs.vals[i+1].url;
	mo->URLs.vals[mo->URLs.count-1].url = NULL;
	mo->URLs.count-=1;
	IS_InsertObject(is, mo);
}


void IS_RemoveOD(InlineScene *is, ODManager *odm)
{
	u32 i;
	ODManager *parent;

	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		ODManager *temp = ChainGetEntry(is->ODlist, i);
		if (temp == odm) {
			ChainDeleteEntry(is->ODlist, i);
			break;
		}
	}

	parent = odm;
	while (parent->parent_OD) parent = parent->parent_OD;

	for (i=0; i<ChainGetCount(is->media_objects); i++) {
		MediaObject *obj = ChainGetEntry(is->media_objects, i);
		if (
			/*assigned object*/
			(obj->odm==odm) || 
			/*remote OD*/
			((obj->OD_ID!=DYNAMIC_OD_ID) && parent->OD && (obj->OD_ID == parent->OD->objectDescriptorID) ) ||
			/*dynamic OD*/
			(obj->URLs.count && parent->OD && parent->OD->URLString && !stricmp(obj->URLs.vals[0].url, parent->OD->URLString)) 
		) {
			obj->mo_flags = 0;
			if (obj->odm) obj->odm->mo = NULL;
			obj->odm = NULL;
			obj->current_frame = NULL;
			obj->current_size = obj->current_ts = 0;

			/*if graph not attached we can remove the link (this is likely scene shutdown for some error)*/
			if (!is->graph_attached) {
				u32 j;
				for (j=0; j<ChainGetCount(is->extern_protos); j++) {
					ProtoLink *pl = ChainGetEntry(is->extern_protos, j);
					if (pl->mo==obj) {
						pl->mo = NULL;
						break;
					}
				}
				ChainDeleteEntry(is->media_objects, i);
				VRML_MF_Reset(&obj->URLs, FT_MFURL);
				free(obj);
			} else {
				/*if dynamic OD and more than 1 URLs resetup*/
				if ((obj->OD_ID==DYNAMIC_OD_ID) && (obj->URLs.count>1)) IS_ReinsertObject(is, obj);
			}
			/*unregister*/
			if (odm->parent_OD) odm->parent_OD->remote_OD = NULL;
			return;
		}
	}
}

u32 URL_GetODID(MFURL *url)
{
	u32 i, j, tmpid;
	char *str, *s_url;
	u32 id = 0;

	if (!url) return 0;
	
	for (i=0; i<url->count; i++) {
		if (url->vals[i].OD_ID) {
			/*works because OD ID 0 is forbidden in MPEG4*/
			if (!id) {
				id = url->vals[i].OD_ID;
			}
			/*bad url, only one object can be described in MPEG4 urls*/
			else if (id != url->vals[i].OD_ID) return 0;
		} else if (url->vals[i].url && strlen(url->vals[i].url)) {
			/*format: od:ID or od:ID#segment - also check for "ID" in case...*/
			str = url->vals[i].url;
			if (strstr(str, "od:")) str += 3;
			/*remove segment info*/
			s_url = strdup(str);
			j = 0;
			while (j<strlen(s_url)) {
				if (s_url[j]=='#') {
					s_url[j] = 0;
					break;
				}
				j++;
			}
			j = sscanf(s_url, "%d", &tmpid);
			/*be carefull, an url like "11-regression-test.mp4" will return 1 on sscanf :)*/
			if (j==1) {
				char szURL[20];
				sprintf(szURL, "%d", tmpid);
				if (stricmp(szURL, s_url)) j = 0;
			}
			free(s_url);

			if (j!= 1) {
				/*dynamic OD if only one URL specified*/
				if (!i) return DYNAMIC_OD_ID;
				/*otherwise ignore*/
				continue;
			}
			if (!id) {
				id = tmpid;
				continue;
			}
			/*bad url, only one object can be described in MPEG4 urls*/
			else if (id != tmpid) return 0;
		}
	}
	return id;
}


//browse all channels and update buffering info
void IS_UpdateBufferingInfo(InlineScene *is)
{
	u32 i, j, max_buffer, cur_buffer;
	Channel *ch;
	ODManager *odm;
	char message[1024];

	if (!is) return;

	max_buffer = cur_buffer = 0;

	/*get buffering on root OD*/
	for (j=0; j<ChainGetCount(is->root_od->channels); j++) {
		ch = ChainGetEntry(is->root_od->channels, j);
		/*count only re-buffering channels*/
		if (!ch->BufferOn) continue;

		max_buffer += ch->MaxBuffer;
		cur_buffer += (ch->BufferTime>0) ? ch->BufferTime : 1;
	}

	/*get buffering on all ODs*/
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		odm = ChainGetEntry(is->ODlist, i);

		if (!odm->codec) continue;
		for (j=0; j<ChainGetCount(odm->channels); j++) {
			ch = ChainGetEntry(odm->channels, j);
			/*count only re-buffering channels*/
			if (!ch->BufferOn) continue;

			max_buffer += ch->MaxBuffer;
			cur_buffer += (ch->BufferTime>0) ? ch->BufferTime : 1;
		}
	}

	if (!max_buffer || !cur_buffer || (max_buffer <= cur_buffer)) {
		sprintf(message, "Buffering 100 %c", '%');
	} else {
		Float ft = (Float) (100*cur_buffer);
		ft /= max_buffer;
		sprintf(message, "Buffering %.2f %c", ft, '%');
	}
	//note we signal it as the main service, even if some streams are running under another 
	//service (ESD URL, OD URL)
	M4_OnMessage(is->root_od->term, is->root_od->net_service->url, message, M4OK);
}



static Bool Inline_SetScene(M_Inline *root)
{
	ODManager *odm;
	MediaObject *mo;
	InlineScene *parent;
	LPSCENEGRAPH graph = Node_GetParentGraph((SFNode *) root);
	parent = SG_GetPrivate(graph);
	if (!parent) return 0;

	mo = IS_GetMediaObject(parent, &root->url, NM_OD_SCENE);
	if (!mo || !mo->odm) return 0;
	odm = mo->odm;
	
	/*we don't handle num_open as with regular ODs since an inline is never "started" by the scene renderer*/
	if (!mo->num_open && !odm->is_open) ODM_Start(odm);
	mo->num_open ++;

	/*handle remote*/
	while (odm->remote_OD) odm = odm->remote_OD;
	if (!odm->subscene) return 0;
	Node_SetPrivate((SFNode *)root, odm->subscene);
	return 1;
}

Bool Inline_IsSameURL(MFURL *obj_url, MFURL *inline_url)
{
	u32 i;
	char szURL1[M4_MAX_PATH], szURL2[M4_MAX_PATH], *ext;
	if (!obj_url->count) return 0;

	strcpy(szURL1, obj_url->vals[0].url);
	ext = strrchr(szURL1, '#');
	if (ext) ext[0] = 0;
	for (i=0; i<inline_url->count; i++) {
		strcpy(szURL2, inline_url->vals[i].url);
		ext = strrchr(szURL2, '#');
		if (ext) ext[0] = 0;
		if (!stricmp(szURL1, szURL2)) return 1;
	}
	return 0;
}

void Inline_Modified(SFNode *node)
{
	u32 ODID;
	MediaObject *mo;
	M_Inline *pInline = (M_Inline *) node;
	InlineScene *pIS = Node_GetPrivate(node);
	if (!pIS) return;

	mo = (pIS->root_od) ? pIS->root_od->mo : NULL;
	ODID = URL_GetODID(&pInline->url);

	/*disconnect current inline if we're the last one using it (same as regular OD session leave/join)*/
	if (mo) {
		Bool changed = 1;
		if (ODID != DYNAMIC_OD_ID) {
			if (ODID && (ODID==pIS->root_od->OD->objectDescriptorID)) changed = 0;
		} else {
			if (Inline_IsSameURL(&mo->URLs, &pInline->url) ) changed = 0;
		}
		if (mo->num_open) {
			if (!changed) return;
			mo->num_open --;
			if (!mo->num_open) {
				ODM_Stop(pIS->root_od, 1);
				IS_Disconnect(pIS);
			}
		}
	}
	
	if (ODID) Inline_SetScene(pInline);
}


static void IS_CheckMediaRestart(InlineScene *is)
{
	/*no ctrl if no duration*/
	if (!is->duration) return;
	if (!is->needs_restart) ODM_CheckSegmentSwitch(is->root_od);
	if (is->needs_restart) return;

	if (is->root_od->media_ctrl && is->root_od->media_ctrl->control->loop) {
		Clock *ck = ODM_GetMediaClock(is->root_od);
		if (ck->has_seen_eos) {
			u32 now = CK_GetTime(ck);
			u32 dur = is->duration;
			if (is->root_od->media_ctrl->current_seg) {
				/*only process when all segments are played*/
				if (ChainGetCount(is->root_od->media_ctrl->seg) <= is->root_od->media_ctrl->current_seg) {
					is->needs_restart = 1;
					is->root_od->media_ctrl->current_seg = 0;
				}
			}
			else {
				Double s, e;
				s = now; s/=1000;
				e = -1;
				MC_GetRange(is->root_od->media_ctrl, &s, &e);
				if ((e>=0) && (e<M4_MAX_FLOAT)) dur = (u32) (e*1000);
				if (dur<now) {
					is->needs_restart = 1;
					is->root_od->media_ctrl->current_seg = 0;
				}
			}
		} else {
			/*trigger render until to watch for restart...*/
			Term_InvalidateScene(is->root_od->term);
		}
	}
}


void IS_Render(SFNode *n, void *render_stack)
{
	SFNode *root;
	InlineScene *is;

	is = Node_GetPrivate(n);

	//if no private scene is associated	get the node parent graph, retrieve the IS and find the OD
	if (!is) {
		Inline_SetScene((M_Inline *) n);
		is = Node_GetPrivate(n);
		if (!is) {
			/*just like protos, we must invalidate parent graph until attached*/
			Node_SetDirty(n, 1);
			return;
		}
	}

	IS_CheckMediaRestart(is);

	/*if we need to restart, shutdown graph and do it*/
	if (is->needs_restart) {
		u32 current_seg = 0;
		if (is->root_od->media_ctrl) current_seg = is->root_od->media_ctrl->current_seg;
		is->needs_restart = 0;

		if (is->is_dynamic_scene) {
			if (is->root_od->media_ctrl) is->root_od->media_ctrl->current_seg = current_seg;
			IS_RestartDynamicScene(is, 0);
		} else {
			/*stop main object from playing but don't disconnect channels*/
			ODM_Stop(is->root_od, 1);
			/*this will close all ODs inside the scene and reset the graph*/
			IS_Disconnect(is);
			if (is->root_od->media_ctrl) is->root_od->media_ctrl->current_seg = current_seg;
			/*start*/
			ODM_Start(is->root_od);
		}
		Node_SetDirty(n, 1);
		return;
	} 
	
	/*if not attached return (attaching the graph cannot be done in render since render is not called while unattached :) */
	if (!is->graph_attached) {
		/*just like protos, we must invalidate parent graph until attached*/
		Node_SetDirty(n, 1);
		return;
	}
	/*clear dirty flags for any sub-inlines, bitmaps or protos*/
	Node_ClearDirty(n);
	
	root = SG_GetRootNode(is->graph);
	/*add Inline node as parent of new scene root (this enables correct subtree dirty state)*/
	if (ChainFindEntry(is->inline_nodes, n)<0) {
		ChainAddEntry(is->inline_nodes, n);
		Node_Register(root, n);
	}
	if (root) {
		SR_RenderInline(is->root_od->term->renderer, root, render_stack);
	}
}

void IS_AttachGraphToRender(InlineScene *is)
{
	if (is->graph_attached) return;
	if (SG_GetRootNode(is->graph)==NULL) return;
	is->graph_attached = 1;
	/*main display scene, setup renderer*/
	if (is->root_od->term->root_scene == is) {
		SR_SetSceneGraph(is->root_od->term->renderer, is->graph);
	}
	else {
		Term_InvalidateScene(is->root_od->term);
	}
}

static MediaObject *IS_CheckExistingObject(InlineScene *is, MFURL *urls)
{
	u32 i;
	for (i=0; i<ChainGetCount(is->media_objects); i++) {
		MediaObject *obj = ChainGetEntry(is->media_objects, i);
		if ((obj->OD_ID == DYNAMIC_OD_ID) && Inline_IsSameURL(&obj->URLs, urls)) return obj;
		else if ((obj->OD_ID != DYNAMIC_OD_ID) && (obj->OD_ID == urls->vals[0].OD_ID)) return obj;
	}
	return NULL;
}

static M4INLINE Bool is_match_obj_type(u32 type, u32 hint_type)
{
	if (!hint_type) return 1;
	if (type==hint_type) return 1;
	/*TEXT are used by animation stream*/
	if ((type==NM_OD_TEXT) && (hint_type==NM_OD_BIFS)) return 1;
	return 0;
}

MediaObject *IS_GetMediaObject(InlineScene *is, MFURL *url, u32 obj_type_hint)
{
	MediaObject *obj, *old_obj;
	u32 i, OD_ID;

	OD_ID = URL_GetODID(url);
	if (!OD_ID) return NULL;

	obj = NULL;
	for (i=0; i<ChainGetCount(is->media_objects); i++) {
		obj = ChainGetEntry(is->media_objects, i);
		/*regular OD scheme*/
		if (OD_ID != DYNAMIC_OD_ID && (obj->OD_ID==OD_ID)) return obj;

		/*dynamic OD scheme*/
		if ((OD_ID == DYNAMIC_OD_ID) && (obj->OD_ID==DYNAMIC_OD_ID)
			/*locate sub-url in given one (handles viewpoint/segments)*/
			&& Inline_IsSameURL(&obj->URLs, url) 
			/*if object type unknown (media control, media sensor), return first obj matching URL
			otherwise check types*/
			&& is_match_obj_type(obj->type, obj_type_hint)
			) return obj;
	}
	/*create a new object identification*/
	obj = NewMediaObject(is->root_od->term);
	obj->OD_ID = OD_ID;
	obj->type = obj_type_hint;
	ChainAddEntry(is->media_objects, obj);
	if (OD_ID == DYNAMIC_OD_ID) {
		char *szExt;
		VRML_FieldCopy(&obj->URLs, url, FT_MFURL);
		for (i=0; i<obj->URLs.count; i++) {
			/*remove proto addressing or viewpoint/viewport*/
			switch (obj_type_hint) {
			case NM_OD_SCENE:
				szExt = strrchr(obj->URLs.vals[i].url, '#');
				if (szExt) szExt[0] = 0;
				break;
			case NM_OD_AUDIO:
				/*little trick to avoid pbs when an audio and a visual node refer to the same service without 
				extensions (eg "file.avi")*/
				szExt = strrchr(obj->URLs.vals[i].url, '#');
				if (!szExt) {
					szExt = malloc(sizeof(char)* (strlen(obj->URLs.vals[i].url)+7));
					strcpy(szExt, obj->URLs.vals[i].url);
					strcat(szExt, "#audio");
					free(obj->URLs.vals[i].url);
					obj->URLs.vals[i].url = szExt;
				}
				break;
			}
		}
		if (obj_type_hint==NM_OD_AUDIO) {
			/*since we modify the URL the above check is not enough*/
			old_obj = IS_CheckExistingObject(is, &obj->URLs);
			if (old_obj != obj) {
				ChainDeleteItem(is->media_objects, obj);
				VRML_MF_Reset(&obj->URLs, FT_MFURL);
				free(obj);
				return old_obj;
			}
		}

		IS_InsertObject(is, obj);
		/*safety check!!!*/
		if (ChainFindEntry(is->media_objects, obj)<0) 
			return NULL;
	}
	return obj;
}

void IS_SetupOD(InlineScene *is, ODManager *odm)
{
	ODManager *parent;
	MediaObject *obj;
	u32 i;
	/*remote ODs shall NOT be setup*/
	assert(odm->remote_OD==NULL);

	/*get parent*/
	parent = odm;
	while (parent->parent_OD) parent = parent->parent_OD;

	/*an object may already be assigned (when using ESD URLs, setup is performed twice)*/
	if (odm->mo != NULL) goto existing;


	for (i=0; i<ChainGetCount(is->media_objects); i++) {
		obj = ChainGetEntry(is->media_objects, i);
		if (obj->OD_ID==DYNAMIC_OD_ID) {
			assert(obj->odm);
			if (obj->odm == parent) {
				/*assign FINAL OD, not parent*/
				obj->odm = odm;
				odm->mo = obj;
				goto existing;
			}
		}
		else if (obj->OD_ID == parent->OD->objectDescriptorID) {
			assert(obj->odm==NULL);
			obj->odm = odm;
			odm->mo = obj;
			goto existing;
		}
	}
	/*newly created OD*/
	odm->mo = NewMediaObject(odm->term);
	ChainAddEntry(is->media_objects, odm->mo);
	odm->mo->odm = odm;
	odm->mo->OD_ID = parent->OD->objectDescriptorID;

existing:
	/*setup object type*/
	if (!odm->codec) odm->mo->type = NM_OD_SCENE;
	else if (odm->codec->type == M4ST_VISUAL) odm->mo->type = NM_OD_VIDEO;
	else if (odm->codec->type == M4ST_AUDIO) odm->mo->type = NM_OD_AUDIO;
	else if (odm->codec->type == M4ST_TEXT) odm->mo->type = NM_OD_TEXT;
	else if (odm->codec->type == M4ST_SCENE) odm->mo->type = NM_OD_BIFS;
	
	/*update info*/
	MO_UpdateCaps(odm->mo);
	if (odm->mo->num_open && !odm->is_open) {
		ODM_Start(odm);
		if (odm->mo->speed != 1.0) ODM_SetSpeed(odm, odm->mo->speed);
	}
	/*invalidate scene for all nodes using the OD*/
	Term_InvalidateScene(odm->term);
}

void IS_Restart(InlineScene *is)
{
	is->needs_restart = 1;
	Term_InvalidateScene(is->root_od->term);
}


void IS_SetSceneDuration(InlineScene *is)
{
	Double dur;
	u32 i, max_dur;
	ODManager *odm;
	Clock *ck;

	/*this is not normative but works in so many cases... set the duration to the max duration
	of all streams sharing the clock*/
	ck = ODM_GetMediaClock(is->root_od);
	max_dur = is->root_od->duration;
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		odm = ChainGetEntry(is->ODlist, i);
		if (!odm->codec) continue;
		if (ck && ODM_SharesClock(odm, ck)) {
			if (odm->duration>max_dur) max_dur = odm->duration;
		}
	}
	if (is->duration == max_dur) return;

	is->duration = max_dur;
	dur = is->duration;
	dur /= 1000;
	
	for (i = 0; i < ChainGetCount(is->root_od->ms_stack); i++) {
		MediaSensorStack *media_sens = ChainGetEntry(is->root_od->ms_stack, i);
		if (media_sens->sensor->isActive) {
			media_sens->sensor->mediaDuration = dur;
			Node_OnEventOutSTR((SFNode *) media_sens->sensor, "mediaDuration");
		}
	}

	if ((is == is->root_od->term->root_scene) && is->root_od->term->user->EventProc) {
		M4Event evt;
		evt.type = M4E_DURATION;
		evt.duration.duration = dur;
		evt.duration.can_seek = !is->root_od->no_time_ctrl;
		if (dur<2.0) evt.duration.can_seek = 0;
		M4USER_SENDEVENT(is->root_od->term->user,&evt);
	}

}


static Bool IS_IsHardcodedProto(MFURL *url, LPINIFILE cfg)
{
	u32 i;
	char *sOpt = IF_GetKey(cfg, "Systems", "hardcoded_protos");
	for (i=0; i<url->count; i++) {
		if (!url->vals[i].url) continue;
		if (strstr(url->vals[i].url, "urn:inet:gpac:builtin")) return 1;
		if (sOpt && strstr(sOpt, url->vals[i].url)) return 1;
	}
	return 0;
}

void IS_LoadExternProto(InlineScene *is, MFURL *url)
{
	u32 i;
	ProtoLink *pl;
	if (!url || !url->count) return;

	/*internal, don't waste ressources*/
	if (IS_IsHardcodedProto(url, is->root_od->term->user->config)) return;
	
	for (i=0; i<ChainGetCount(is->extern_protos); i++) {
		pl = ChainGetEntry(is->extern_protos, i);
		if (pl->url == url) return;
		if (pl->url->vals[0].OD_ID == url->vals[0].OD_ID) return;
		if (pl->url->vals[0].url && url->vals[0].url && !stricmp(pl->url->vals[0].url, url->vals[0].url) ) return;
	}
	pl = malloc(sizeof(ProtoLink));
	pl->url = url;
	ChainAddEntry(is->extern_protos, pl);
	pl->mo = IS_GetMediaObject(is, url, NM_OD_SCENE);
	/*this may already be destroyed*/
	if (pl->mo) MO_Play(pl->mo);
}

LPSCENEGRAPH IS_GetProtoLib(void *_is, MFURL *lib_url)
{
	u32 i;
	InlineScene *is = (InlineScene *) _is;
	if (!is || !lib_url->count) return NULL;

	if (IS_IsHardcodedProto(lib_url, is->root_od->term->user->config)) return SG_INTERNAL_PROTO;

	for (i=0; i<ChainGetCount(is->extern_protos); i++) {
		ProtoLink *pl = ChainGetEntry(is->extern_protos, i);
		if (!pl->mo) continue;
		if (URL_GetODID(pl->url) != DYNAMIC_OD_ID) {
			if (URL_GetODID(pl->url) == URL_GetODID(lib_url)) {
				if (!pl->mo->odm || !pl->mo->odm->subscene) return NULL;
				return pl->mo->odm->subscene->graph;
			}
		} else if (lib_url->vals[0].url) {
			if (Inline_IsSameURL(&pl->mo->URLs, lib_url)) {
				if (!pl->mo->odm || !pl->mo->odm->subscene) return NULL;
				return pl->mo->odm->subscene->graph;
			}
		}
	}

	/*not found, create loader*/
	IS_LoadExternProto(is, lib_url);

	/*and return NULL*/
	return NULL;
}

ODManager *IS_GetProtoSceneByGraph(void *_is, LPSCENEGRAPH sg)
{
	u32 i;
	InlineScene *is = (InlineScene *) _is;
	if (!is) return NULL;
	for (i=0; i<ChainGetCount(is->extern_protos); i++) {
		ProtoLink *pl = ChainGetEntry(is->extern_protos, i);
		if (pl->mo->odm && pl->mo->odm->subscene && (pl->mo->odm->subscene->graph==sg)) return pl->mo->odm;
	}
	return NULL;
}


Bool IS_IsProtoLibObject(InlineScene *is, ODManager *odm)
{
	u32 i;
	for (i=0; i<ChainGetCount(is->extern_protos); i++) {
		ProtoLink *pl = ChainGetEntry(is->extern_protos, i);
		if (pl->mo->odm == odm) return 1;
	}
	return 0;
}


MediaObject *IS_FindObject(InlineScene *is, u16 ODID, char *url)
{
	u32 i;
	if (!url && !ODID) return NULL;
	for (i=0; i<ChainGetCount(is->media_objects); i++) {
		MediaObject *mo = ChainGetEntry(is->media_objects, i);
		if (ODID==DYNAMIC_OD_ID) {
			if (mo->URLs.count && !stricmp(mo->URLs.vals[0].url, url)) return mo;
		} else if (mo->OD_ID==ODID) return mo;
	}
	return NULL;
}


const char *IS_GetSceneViewName(InlineScene *is) 
{
	char *seg_name;
	SegmentDescriptor *ODM_GetSegment(ODManager *odm, char *descName);
	SegmentDescriptor *sdesc;
	/*check any viewpoint*/
	seg_name = strrchr(is->root_od->net_service->url, '#');
	if (!seg_name) return NULL;
	seg_name += 1;
	sdesc = ODM_GetSegment(is->root_od, seg_name);
	if (!sdesc && is->root_od->parent_OD) {
		ODManager *par = is->root_od->parent_OD;
		while (par->parent_OD) par = par->parent_OD;
		sdesc = ODM_GetSegment(par, seg_name);
	}
	if (sdesc) return NULL;
	return seg_name;
}

Bool IS_IsDefaultView(SFNode *node)
{
	const char *nname, *sname;
	LPSCENEGRAPH sg = Node_GetParentGraph(node);
	InlineScene *is = sg ? SG_GetPrivate(sg) : NULL;
	if (!is) return 0;

	nname = Node_GetDefName(node);
	if (!nname) return 0;
	sname = IS_GetSceneViewName(is);
	if (!sname) return 0;
	return (!strcmp(nname, sname));
}

void IS_RegisterExtraScene(InlineScene *is, LPSCENEGRAPH extra_scene, Bool do_remove)
{
	if (do_remove) {
		if (ChainFindEntry(is->extra_scenes, extra_scene)<0) return;
		ChainDeleteItem(is->extra_scenes, extra_scene);
		/*for root scene*/
		if (is->root_od->term->root_scene == is) {
			SR_RegisterExtraGraph(is->root_od->term->renderer, extra_scene, 1);
		}
	} else {
		if (ChainFindEntry(is->extra_scenes, extra_scene)>=0) return;
		ChainAddEntry(is->extra_scenes, extra_scene);
		/*for root scene*/
		if (is->root_od->term->root_scene == is) {
			SR_RegisterExtraGraph(is->root_od->term->renderer, extra_scene, 0);
		}
	}
}



static void IS_UpdateVideoPos(InlineScene *is)
{
	MFURL url;
	M_Transform2D *tr;
	MediaObject *mo;
	u32 w, h;
	if (!is->visual_url.OD_ID && !is->visual_url.url) return;

	url.count = 1;
	url.vals = &is->visual_url;
	mo = IS_CheckExistingObject(is, &url);
	if (!mo) return;
	tr = (M_Transform2D *) SG_FindNodeByName(is->graph, "DYN_TRANS");
	if (!tr) return;

	SG_GetSizeInfo(is->graph, &w, &h);
	if (!w || !h) return;

	tr->translation.x = (Float) (w - mo->width) / 2;
	tr->translation.y = (Float) (h - mo->height) / 2;
	Node_SetDirty((SFNode *)tr, 0);

	if (is->root_od->term->root_scene == is) {
		//if (is->graph_attached) SR_SetSceneGraph(is->root_od->term->renderer, NULL);
		SR_SetSceneGraph(is->root_od->term->renderer, is->graph);
	}
}

static SFNode *is_create_node(LPSCENEGRAPH sg, u32 tag, const char *def_name)
{
	SFNode *n = SG_NewNode(sg, tag);
	if (n) {
		if (def_name) Node_SetDEF(n, SG_GetNextAvailableNodeID(sg), def_name);
		Node_Init(n);
	}
	return n;
}

static Bool is_odm_url(SFURL *url, ODManager *odm)
{
	if (!url->OD_ID && !url->url) return 0;
	if (odm->OD->objectDescriptorID != DYNAMIC_OD_ID) return (url->OD_ID==odm->OD->objectDescriptorID) ? 1 : 0;
	if (!url->url || !odm->OD->URLString) return 0;
	return !stricmp(url->url, odm->OD->URLString);
}

/*regenerates the scene graph for dynamic scene.
This will also try to reload any previously presented streams. Note that in the usual case the scene is generated
just once when recieving the first OD AU (ressources are NOT destroyed when seeking), but since the network may need
to update the OD ressources, we still kake care of it*/
void IS_RegenerateScene(InlineScene *is)
{
	u32 i, nb_obj, w, h;
	SFNode *n1, *n2;
	SFURL *sfu;
	M4Event evt;
	ODManager *first_odm;
	M_AudioClip *ac;
	M_MovieTexture *mt;
	M_AnimationStream *as;

	if (!is->is_dynamic_scene) return;

	if (is->root_od->term->root_scene == is) 
		SR_SetSceneGraph(is->root_od->term->renderer, NULL);
	SG_Reset(is->graph);
	SG_GetSizeInfo(is->graph, &w, &h);
	SG_SetSizeInfo(is->graph, w, h, 1);
	n1 = is_create_node(is->graph, TAG_MPEG4_OrderedGroup, NULL);
	SG_SetRootNode(is->graph, n1);
	Node_Register(n1, NULL);

	n2 = is_create_node(is->graph, TAG_MPEG4_Sound2D, NULL);
	ChainAddEntry(((SFParent *)n1)->children, n2);
	Node_Register(n2, n1);

	ac = (M_AudioClip *) is_create_node(is->graph, TAG_MPEG4_AudioClip, "DYN_AUDIO");
	ac->startTime = IS_GetSceneTime(is);
	((M_Sound2D *)n2)->source = (SFNode *)ac;
	Node_Register((SFNode *)ac, n2);

	nb_obj = 0;
	first_odm = NULL;
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		ODManager *odm = ChainGetEntry(is->ODlist, i);
		if (!odm->codec || (odm->codec->type!=M4ST_AUDIO)) continue;

		if (is_odm_url(&is->audio_url, odm)) {
			VRML_MF_Append(&ac->url, FT_MFURL, (void **) &sfu);
			sfu->OD_ID = is->audio_url.OD_ID;
			if (is->audio_url.url) sfu->url = strdup(is->audio_url.url);
			first_odm = NULL;
			nb_obj++;
			break;
		}
		if (!first_odm) first_odm = odm;
	}
	if (first_odm) {
		if (is->audio_url.url) free(is->audio_url.url);
		is->audio_url.url = NULL;
		is->audio_url.OD_ID = first_odm->OD->objectDescriptorID;
		if (first_odm->OD->URLString) is->audio_url.url = strdup(first_odm->OD->URLString);
		VRML_MF_Append(&ac->url, FT_MFURL, (void **) &sfu);
		sfu->OD_ID = is->audio_url.OD_ID;
		if (is->audio_url.url) sfu->url = strdup(is->audio_url.url);
		nb_obj++;
	}

	/*transform for any translation due to scene resize (3GPP)*/
	n2 = is_create_node(is->graph, TAG_MPEG4_Transform2D, "DYN_TRANS");
	ChainAddEntry(((SFParent *)n1)->children, n2);
	Node_Register(n2, n1);
	n1 = n2;

	n2 = is_create_node(is->graph, TAG_MPEG4_Shape, NULL);
	ChainAddEntry(((SFParent *)n1)->children, n2);
	Node_Register(n2, n1);
	n1 = n2;
	n2 = is_create_node(is->graph, TAG_MPEG4_Appearance, NULL);
	((M_Shape *)n1)->appearance = n2;
	Node_Register(n2, n1);


	mt = (M_MovieTexture *) is_create_node(is->graph, TAG_MPEG4_MovieTexture, "DYN_VIDEO");
	mt->startTime = IS_GetSceneTime(is);
	((M_Appearance *)n2)->texture = (SFNode *)mt;
	Node_Register((SFNode *)mt, n2);

	first_odm = NULL;
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		ODManager *odm = ChainGetEntry(is->ODlist, i);
		if (!odm->codec || (odm->codec->type!=M4ST_VISUAL)) continue;

		if (is_odm_url(&is->audio_url, odm)) {
			VRML_MF_Append(&mt->url, FT_MFURL, (void **) &sfu);
			sfu->OD_ID = is->visual_url.OD_ID;
			if (is->visual_url.url) sfu->url = strdup(is->visual_url.url);
			if (first_odm->mo) SG_SetSizeInfo(is->graph, first_odm->mo->width, first_odm->mo->height, 1);
			first_odm = NULL;
			nb_obj++;
			break;
		}
		if (!first_odm) first_odm = odm;
	}
	if (first_odm) {
		if (is->visual_url.url) free(is->visual_url.url);
		is->visual_url.url = NULL;
		is->visual_url.OD_ID = first_odm->OD->objectDescriptorID;
		if (first_odm->OD->URLString) is->visual_url.url = strdup(first_odm->OD->URLString);
		VRML_MF_Append(&mt->url, FT_MFURL, (void **) &sfu);
		sfu->OD_ID = is->visual_url.OD_ID;
		if (is->visual_url.url) sfu->url = strdup(is->visual_url.url);
		if (first_odm->mo) SG_SetSizeInfo(is->graph, first_odm->mo->width, first_odm->mo->height, 1);
		nb_obj++;
	}

	n2 = is_create_node(is->graph, TAG_MPEG4_Bitmap, NULL);
	((M_Shape *)n1)->geometry = n2;
	Node_Register(n2, n1);


	/*text streams controlled through AnimationStream*/
	n1 = SG_GetRootNode(is->graph);
	as = (M_AnimationStream *) is_create_node(is->graph, TAG_MPEG4_AnimationStream, "DYN_TEXT");
	ChainAddEntry(((SFParent *)n1)->children, as);
	Node_Register((SFNode *)as, n1);

	first_odm = NULL;
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		ODManager *odm = ChainGetEntry(is->ODlist, i);
		if (!odm->codec || (odm->codec->type!=M4ST_TEXT)) continue;

		if (!nb_obj || is_odm_url(&is->text_url, odm)) {
			if (is->text_url.url) free(is->text_url.url);
			is->text_url.url = NULL;

			VRML_MF_Append(&as->url, FT_MFURL, (void **) &sfu);
			sfu->OD_ID = is->visual_url.OD_ID = odm->OD->objectDescriptorID;
			if (odm->OD->URLString) {
				sfu->url = strdup(odm->OD->URLString);
				is->text_url.url = strdup(odm->OD->URLString);
			}
			first_odm = NULL;
			break;
		}
		if (!first_odm) first_odm = odm;
	}
	if (first_odm) {
		if (is->text_url.url) free(is->text_url.url);
		is->text_url.url = NULL;
		VRML_MF_Append(&as->url, FT_MFURL, (void **) &sfu);
		sfu->OD_ID = is->text_url.OD_ID = first_odm->OD->objectDescriptorID;
		if (first_odm->OD->URLString) {
			sfu->url = strdup(first_odm->OD->URLString);
			is->text_url.url = strdup(first_odm->OD->URLString);
		}
	}

	/*disconnect to force resize*/
	if (is->root_od->term->root_scene == is) {
		if (is->graph_attached) SR_SetSceneGraph(is->root_od->term->renderer, NULL);
		SR_SetSceneGraph(is->root_od->term->renderer, is->graph);
		is->graph_attached = 1;
		evt.type = M4E_STREAMLIST;
		M4USER_SENDEVENT(is->root_od->term->user,&evt);

		IS_UpdateVideoPos(is);
	} else {
		is->graph_attached = 1;
		Term_InvalidateScene(is->root_od->term);
	}
}

static Bool check_odm_deactivate(SFURL *url, ODManager *odm, SFNode *n)
{
	FieldInfo info;
	if (!is_odm_url(url, odm) || !n) return 0;

	if (url->url) free(url->url);
	url->url = NULL;
	url->OD_ID = 0;

	Node_GetFieldByName(n, "url", &info);
	VRML_MF_Reset(info.far_ptr, FT_MFURL);
	Node_GetFieldByName(n, "stopTime", &info);
	*((SFTime *)info.far_ptr) = Node_GetSceneTime(n);
	SG_NodeChanged(n, NULL);
	return 1;
}

void IS_SelectODM(InlineScene *is, ODManager *odm)
{
	ODManager *real_odm;
	if (!is->is_dynamic_scene || !is->graph_attached || !odm) return;
	real_odm = odm;
	while (real_odm->remote_OD) real_odm = real_odm->remote_OD;
	while (odm->parent_OD) odm = odm->parent_OD;
	
	if (!real_odm->codec) return;

	if (real_odm->is_open) {
		if (check_odm_deactivate(&is->audio_url, odm, SG_FindNodeByName(is->graph, "DYN_AUDIO")) ) return;
		if (check_odm_deactivate(&is->visual_url, odm, SG_FindNodeByName(is->graph, "DYN_VIDEO") )) return;
		if (check_odm_deactivate(&is->text_url, odm, SG_FindNodeByName(is->graph, "DYN_TEXT") )) return;
	}

	if (real_odm->codec->type == M4ST_AUDIO) {
		M_AudioClip *ac = (M_AudioClip *) SG_FindNodeByName(is->graph, "DYN_AUDIO");
		if (!ac) return;
		if (is->audio_url.url) free(is->audio_url.url);
		is->audio_url.url = NULL;
		is->audio_url.OD_ID = odm->OD->objectDescriptorID;
		if (!ac->url.count) VRML_MF_Alloc(&ac->url, FT_MFURL, 1);
		ac->url.vals[0].OD_ID = odm->OD->objectDescriptorID;
		if (ac->url.vals[0].url) free(ac->url.vals[0].url);
		if (odm->OD->URLString) {
			is->audio_url.url = strdup(odm->OD->URLString);
			ac->url.vals[0].url = strdup(odm->OD->URLString);
		}
		ac->startTime = IS_GetSceneTime(is);
		SG_NodeChanged((SFNode *)ac, NULL);
		return;
	}

	if (real_odm->codec->type == M4ST_VISUAL) {
		M_MovieTexture *mt = (M_MovieTexture*) SG_FindNodeByName(is->graph, "DYN_VIDEO");
		if (!mt) return;
		if (is->visual_url.url) free(is->visual_url.url);
		is->visual_url.url = NULL;
		is->visual_url.OD_ID = odm->OD->objectDescriptorID;
		if (!mt->url.count) VRML_MF_Alloc(&mt->url, FT_MFURL, 1);
		mt->url.vals[0].OD_ID = odm->OD->objectDescriptorID;
		if (mt->url.vals[0].url) free(mt->url.vals[0].url);
		if (odm->OD->URLString) {
			is->visual_url.url = strdup(odm->OD->URLString);
			mt->url.vals[0].url = strdup(odm->OD->URLString);
		}
		mt->startTime = IS_GetSceneTime(is);
		SG_NodeChanged((SFNode *)mt, NULL);
		return;
	}


	if (real_odm->codec->type == M4ST_TEXT) {
		M_AnimationStream *as = (M_AnimationStream*) SG_FindNodeByName(is->graph, "DYN_TEXT");
		if (!as) return;
		if (is->text_url.url) free(is->text_url.url);
		is->text_url.url = NULL;
		is->text_url.OD_ID = odm->OD->objectDescriptorID;
		if (!as->url.count) VRML_MF_Alloc(&as->url, FT_MFURL, 1);
		as->url.vals[0].OD_ID = odm->OD->objectDescriptorID;
		if (as->url.vals[0].url) free(as->url.vals[0].url);
		if (odm->OD->URLString) {
			is->text_url.url = strdup(odm->OD->URLString);
			as->url.vals[0].url = strdup(odm->OD->URLString);
		}
		as->startTime = IS_GetSceneTime(is);
		SG_NodeChanged((SFNode *)as, NULL);
		return;
	}
}


void IS_ForceSceneSize(InlineScene *is, u32 width, u32 height)
{
	/*for now only allowed when no scene info*/
	if (!is->is_dynamic_scene) return;
	SG_SetSizeInfo(is->graph, width, height, SG_UsePixelMetrics(is->graph));
	if (is->root_od->term->root_scene != is) return;
	SR_SetSceneGraph(is->root_od->term->renderer, is->graph);

	IS_UpdateVideoPos(is);
}

void IS_RestartDynamicScene(InlineScene *is, u32 from_time)
{
	u32 i;
	Chain *to_restart;
	Clock *ck = is->scene_codec->ck;

	CK_Pause(ck);
	CK_Reset(ck);

	to_restart = NewChain();
	for (i=0; i<ChainGetCount(is->ODlist); i++) {
		ODManager *odm = ChainGetEntry(is->ODlist, i);
		while (odm->remote_OD) odm = odm->remote_OD;
		if (odm->is_open) {
			ChainAddEntry(to_restart, odm);
			ODM_Stop(odm, 1);
		}
	}
	if (is->root_od->media_ctrl) {
		Double start, end;
		start = from_time; start /= 1000;
		end = -1;
		MC_GetRange(is->root_od->media_ctrl, &start, &end);
		if (start>=0) from_time = (u32) (start*1000.0);
	}
	
	CK_SetTime(ck, from_time);

	for (i=0; i<ChainGetCount(to_restart); i++) {
		ODManager *odm = ChainGetEntry(to_restart, i);
		ODM_Start(odm);
	}
	DeleteChain(to_restart);
	/*also check nodes if no media control since they may be deactivated (end of stream)*/
	if (!is->root_od->media_ctrl) {
		M_AudioClip *ac = (M_AudioClip *) SG_FindNodeByName(is->graph, "DYN_AUDIO");
		M_MovieTexture *mt = (M_MovieTexture *) SG_FindNodeByName(is->graph, "DYN_VIDEO");
		M_AnimationStream *as = (M_AnimationStream *) SG_FindNodeByName(is->graph, "DYN_TEXT");
		if (ac) {
			ac->startTime = IS_GetSceneTime(is);
			SG_NodeChanged((SFNode *)ac, NULL);
		}
		if (mt) {
			mt->startTime = IS_GetSceneTime(is);
			SG_NodeChanged((SFNode *)mt, NULL);
		}
		if (as) {
			as->startTime = IS_GetSceneTime(is);
			SG_NodeChanged((SFNode *)as, NULL);
		}
	}

	CK_Resume(ck);
}
