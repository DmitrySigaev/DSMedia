/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / GPAC Scene Context loader plugin
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
#include <gpac/m4_author.h>


typedef struct 
{
	InlineScene *inline_scene;
	M4Client *app;
	M4SceneManager *ctx;
	char *fileName;
	u32 load_flags;
	u32 nb_streams;
	u32 base_stream_id;
	/*mp3 import from flash*/
	Chain *files_to_delete;
	M4ContextLoader load;
} M4CTXPriv;


static M4Err M4CTX_GetCapabilities(BaseDecoder *plug, CapObject *cap)
{
	cap->cap.valueINT = 0;
	return M4NotSupported;
}

static M4Err M4CTX_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
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


static void M4CTX_Reset(M4CTXPriv *priv)
{
	if (priv->ctx) M4SM_Delete(priv->ctx);
	priv->ctx = NULL;
	SG_Reset(priv->inline_scene->graph);
	if (priv->load_flags != 3) priv->load_flags = 0;
	while (ChainGetCount(priv->files_to_delete)) {
		char *fileName = ChainGetEntry(priv->files_to_delete, 0);
		ChainDeleteEntry(priv->files_to_delete, 0);
		M4_DeleteFile(fileName);
		free(fileName);
	}
}

void M4CTX_OnActivate(SFNode *node)
{
	M4CTXPriv *priv = (M4CTXPriv *) Node_GetPrivate(node);
	M_Conditional*c = (M_Conditional*)node;
	/*always apply in parent graph to handle protos correctly*/
	if (c->activate) SG_ApplyCommandList(Node_GetParentGraph(node), c->buffer.commandList, IS_GetSceneTime(priv->inline_scene));
}
void M4CTX_OnReverseActivate(SFNode *node)
{
	M4CTXPriv *priv = (M4CTXPriv *) Node_GetPrivate(node);
	M_Conditional*c = (M_Conditional*)node;
	/*always apply in parent graph to handle protos correctly*/
	if (!c->reverseActivate) SG_ApplyCommandList(Node_GetParentGraph(node), c->buffer.commandList, IS_GetSceneTime(priv->inline_scene));
}

void M4CTX_NodeInit(void *cbk, SFNode *node)
{
	M4CTXPriv *priv = (M4CTXPriv *) cbk;
	if (Node_GetTag(node) == TAG_MPEG4_Conditional) {
		((M_Conditional*)node)->on_activate = M4CTX_OnActivate;
		((M_Conditional*)node)->on_reverseActivate = M4CTX_OnReverseActivate;
		Node_SetPrivate(node, priv);
	} else {
		Term_NodeInit(priv->inline_scene, node);
	}
}

static M4Err M4CTX_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_scene_decoder)
{
	M4CTXPriv *priv = plug->privateStack;
	if (priv->ctx) return M4BadParam;

	priv->inline_scene = scene;
	priv->app = scene->root_od->term;

	SG_SetInitCallback(scene->graph, M4CTX_NodeInit, priv);
	return M4OK;
}

static M4Err M4CTX_ReleaseScene(SceneDecoder *plug)
{
	M4CTX_Reset((M4CTXPriv *) plug->privateStack);
	return M4OK;
}

static M4Err M4CTX_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	M4CTXPriv *priv = plug->privateStack;
	if (Upstream) return M4NotSupported;

	/*animation stream like*/
	if (priv->ctx) {
		u32 i;
		for (i=0; i<ChainGetCount(priv->ctx->streams); i++) {
			M4StreamContext *sc = ChainGetEntry(priv->ctx->streams, i);
			if (ES_ID == sc->ESID) {
				priv->nb_streams++;
				return M4OK;
			}
		}
		return M4NonCompliantBitStream;
	}
	/*main dummy stream we need a dsi*/
	if (!decSpecInfo) return M4NonCompliantBitStream;

	priv->fileName = strdup(decSpecInfo);
	priv->nb_streams = 1;
	priv->load_flags = 0;
	priv->base_stream_id = ES_ID;
	return M4OK;
}

static M4Err M4CTX_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	M4CTXPriv *priv = plug->privateStack;
	priv->nb_streams --;
	return M4OK;
}

static Bool M4CTX_StreamInRootOD(ObjectDescriptor *od, u32 ESID)
{
	u32 i, count;
	/*no root, only one stream possible*/
	if (!od) return 1;
	count = ChainGetCount(od->ESDescriptors);
	/*idem*/
	if (!count) return 1;
	for (i=0; i<count; i++) {
		ESDescriptor *esd = ChainGetEntry(od->ESDescriptors, i);
		if (esd->ESID==ESID) return 1;
	}
	return 0;
}

static void M4CTX_OnMessage(void *cbk, char *szMsg, M4Err e)
{
	M4CTXPriv *priv = (M4CTXPriv *)cbk;
	M4_OnMessage(priv->inline_scene->root_od->term, priv->inline_scene->root_od->net_service->url, szMsg, e);
}

static void M4CTX_OnProgress(void *cbk, u32 done, u32 tot)
{
	char szMsg[1024];
	M4CTXPriv *priv = (M4CTXPriv *)cbk;
	sprintf(szMsg, "Importing %d / %d (%.2f %%)", done, tot, ((Float)100*done)/tot);
	M4_OnMessage(priv->inline_scene->root_od->term, 
		priv->inline_scene->root_od->net_service->url, szMsg, M4OK);
}

Double M4CTX_GetVRMLTime(void *cbk)
{
	u32 secs, msecs;
	Double res;
	M4_GetUTCTimeSince1970(&secs, &msecs);
	res = msecs;
	res /= 1000;
	res += secs;
	return res;
}


static M4Err M4CTX_ProcessData(SceneDecoder *plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 stream_time, u32 mmlevel)
{
	M4Err e = M4OK;
	u32 i, j, k, nb_updates, max_dur;
	M4AUContext *au;
	Bool can_delete_com;
	M4StreamContext *sc;
	M4CTXPriv *priv = plug->privateStack;

	/*something failed*/
	if (priv->load_flags==3) return M4EOF;

	/*this signals main scene deconnection, destroy the context if needed*/
	assert(ES_ID);

	if (priv->load_flags != 2) {
		/*load first frame only*/
		if (!priv->load_flags) {
			priv->load_flags = 1;
			priv->ctx = NewSceneManager(priv->inline_scene->graph);
			memset(&priv->load, 0, sizeof(M4ContextLoader));
			priv->load.ctx = priv->ctx;
			priv->load.cbk = priv;
			priv->load.scene_graph = priv->inline_scene->graph;
			priv->load.fileName = priv->fileName;
			priv->load.OnMessage = M4CTX_OnMessage;
			priv->load.OnProgress = M4CTX_OnProgress;
			priv->load.flags = M4CL_FOR_PLAYBACK;
			priv->load.swf_import_flags = M4SWF_StaticDictionary | M4SWF_UseXCurve | M4SWF_UseXLineProps;
			e = M4SM_LoaderInit(&priv->load);
			if (!e) {
				SG_SetSizeInfo(priv->inline_scene->graph, priv->ctx->scene_width, priv->ctx->scene_height, priv->ctx->is_pixel_metrics);
				/*VRML, override base clock*/
				if ((priv->load.type==M4CL_VRML) || (priv->load.type==M4CL_X3DV) || (priv->load.type==M4CL_X3D))
					SG_SetSceneTimeCallback(priv->inline_scene->graph, M4CTX_GetVRMLTime, priv);
			}
		} 
		/*load the rest*/
		else {
			priv->load_flags = 2;
			e = M4SM_LoaderRun(&priv->load);
			M4SM_LoaderDone(&priv->load);
		}

		if (e) {
			M4SM_Delete(priv->ctx);
			priv->ctx = NULL;
			priv->load_flags = 3;
			return e;
		}

		/*and figure out duration of root scene, and take care of XMT timing*/
		max_dur = 0;
		for (i=0; i<ChainGetCount(priv->ctx->streams); i++) {
			sc = ChainGetEntry(priv->ctx->streams, i);
			/*all streams in root OD are handled with ESID 0 to differentiate with any animation streams*/
			if (M4CTX_StreamInRootOD(priv->ctx->root_od, sc->ESID)) sc->ESID = 0;
			if (!sc->timeScale) sc->timeScale = 1000;

			au = NULL;
			for (j = 0; j<ChainGetCount(sc->AUs); j++) {
				au = ChainGetEntry(sc->AUs, j);
				if (!au->timing) au->timing = (u32) (sc->timeScale*au->timing_sec);
			}
			if (au && !sc->ESID && (au->timing>max_dur)) max_dur = au->timing * 1000 / sc->timeScale;
		}
		if (priv->load_flags==2) {
			if (max_dur) {
				priv->inline_scene->root_od->duration = max_dur;
				IS_SetSceneDuration(priv->inline_scene);
			}
		}
	}

	nb_updates = 0;

	for (i=0; i<ChainGetCount(priv->ctx->streams); i++) {
		M4StreamContext *sc = ChainGetEntry(priv->ctx->streams, i);
		/*not our stream*/
		if (sc->ESID && (sc->ESID != ES_ID)) continue;
		/*not the base stream*/
		if (!sc->ESID && (priv->base_stream_id != ES_ID)) continue;
		/*handle SWF media extraction*/
		if ((sc->streamType == M4ST_OD) && (priv->load_flags==1)) continue;

		/*check for seek*/
		if (sc->last_au_time > 1 + stream_time) {
			/*root streams shall not seek this way*/
			if (!sc->ESID) assert(0);
			/*on other streams, we assume the stream is 100% RAP wrt to the base stream*/
			else {
				sc->last_au_time = 0;
			}
		}

		can_delete_com = 0;
		if (!sc->ESID && (priv->load_flags==2)) can_delete_com = 1;

		/*we're in the right stream, apply update*/
		for (j=0; j<ChainGetCount(sc->AUs); j++) {
			M4AUContext *au = ChainGetEntry(sc->AUs, j);
			u32 au_time = au->timing*1000/sc->timeScale;
			if (au_time + 1 <= sc->last_au_time) {
				/*remove first replace command*/
				if (!sc->ESID && (sc->streamType==M4ST_SCENE)) {
					while (ChainGetCount(au->commands)) {
						SGCommand *com = ChainGetEntry(au->commands, 0);
						ChainDeleteEntry(au->commands, 0);
						SG_DeleteCommand(com);
					}
					ChainDeleteEntry(sc->AUs, j);
					DeleteChain(au->commands);
					free(au);
					j--;
				}
				continue;
			}
			if (au_time > stream_time) {
				nb_updates++;
				break;
			}

			if (sc->streamType == M4ST_SCENE) {
				/*apply the commands*/
				for (k=0; k<ChainGetCount(au->commands); k++) {
					SGCommand *com = ChainGetEntry(au->commands, k);
					e = SG_ApplyCommand(priv->inline_scene->graph, com, 0);
					if (e) break;
					/*remove commands on base layer*/
					if (can_delete_com) {
						ChainDeleteEntry(au->commands, k);
						SG_DeleteCommand(com);
						k--;
					}
				}
			} 
			else if (sc->streamType == M4ST_OD) {
				/*apply the commands*/
				while (ChainGetCount(au->commands)) {
					Bool keep_com = 0;
					ODCommand *com = ChainGetEntry(au->commands, 0);
					ChainDeleteEntry(au->commands, 0);
					switch (com->tag) {
					case ODUpdate_Tag:
					{
						ObjectDescriptorUpdate *odU = (ObjectDescriptorUpdate *)com;
						while (ChainGetCount(odU->objectDescriptors)) {
							ESDescriptor *esd;
							char *remote;
							MuxInfoDescriptor *mux = NULL;
							ObjectDescriptor *od = ChainGetEntry(odU->objectDescriptors, 0);
							ChainDeleteEntry(odU->objectDescriptors, 0);
							/*we can only work with single-stream ods*/
							esd = ChainGetEntry(od->ESDescriptors, 0);
							if (!esd) {
								if (od->URLString) {
									ODS_SetupOD(priv->inline_scene, od);
								} else {
									OD_DeleteDescriptor((Descriptor **) &od);
								}
								continue;
							}
							/*fix OCR dependencies*/
							if (M4CTX_StreamInRootOD(priv->ctx->root_od, esd->OCRESID)) esd->OCRESID = priv->base_stream_id;

							/*forbidden if ESD*/
							if (od->URLString) {
								OD_DeleteDescriptor((Descriptor **) &od);
								continue;
							}
							/*look for MUX info*/
							for (k=0; k<ChainGetCount(esd->extensionDescriptors); k++) {
								mux = ChainGetEntry(esd->extensionDescriptors, k);
								if (mux->tag == MuxInfoDescriptor_Tag) break;
								mux = NULL;
							}
							/*we need a mux if not animation stream*/
							if (!mux || !mux->file_name) {
								/*only animation streams are handled*/
								if (!esd->decoderConfig) {
									OD_DeleteDescriptor((Descriptor **) &od);
								} else if (esd->decoderConfig->streamType==M4ST_SCENE) {
									/*set ST to private scene to get sure the stream will be redirected to us*/
									esd->decoderConfig->streamType = M4ST_PRIVATE_SCENE;
									esd->dependsOnESID = priv->base_stream_id;
									ODS_SetupOD(priv->inline_scene, od);
								} else if (esd->decoderConfig->streamType==M4ST_INTERACT) {
									UIConfigDescriptor *cfg = (UIConfigDescriptor *) esd->decoderConfig->decoderSpecificInfo;
									OD_EncodeUIConfig(cfg, &esd->decoderConfig->decoderSpecificInfo);
									OD_DeleteDescriptor((Descriptor **) &cfg);
									ODS_SetupOD(priv->inline_scene, od);
								} else {
									OD_DeleteDescriptor((Descriptor **) &od);
								}
								continue;
							}
							/*text import*/
							if (mux->textNode) {
#ifdef M4_READ_ONLY
								OD_DeleteDescriptor((Descriptor **) &od);
								continue;
#else
								M4SM_ImportSRT_BIFS(priv->ctx, esd, mux);
								/*set ST to private scene and dependency to base to get sure the stream will be redirected to us*/
								esd->decoderConfig->streamType = M4ST_PRIVATE_SCENE;
								esd->dependsOnESID = priv->base_stream_id;
								ODS_SetupOD(priv->inline_scene, od);
								continue;
#endif
							}

							/*soundstreams are a bit of a pain, they may be declared before any data gets written*/
							if (mux->delete_file) {
								FILE *t = fopen(mux->file_name, "rb");
								if (!t) {
									keep_com = 1;
									ChainInsertEntry(odU->objectDescriptors, od, 0);
									break;
								}
								fclose(t);
							}
							/*remap to remote URL*/
							remote = strdup(mux->file_name);
							k = od->objectDescriptorID;
							/*if files were created we'll have to clean up (swf import)*/
							if (mux->delete_file) ChainAddEntry(priv->files_to_delete, strdup(remote));

							OD_DeleteDescriptor((Descriptor **) &od);
							od = (ObjectDescriptor *) OD_NewDescriptor(ObjectDescriptor_Tag);
							od->URLString = remote;
							od->objectDescriptorID = k;
							ODS_SetupOD(priv->inline_scene, od);
						}
						if (keep_com) break;
					}
						break;
					case ODRemove_Tag:
					{
						ObjectDescriptorRemove *odR = (ObjectDescriptorRemove*)com;
						for (k=0; k<odR->NbODs; k++) {
							ODManager *odm = IS_FindODM(priv->inline_scene, odR->OD_ID[k]);
							if (odm) ODM_RemoveOD(odm);
						}
					}
						break;
					default:
						break;
					}
					if (keep_com) {
						ChainInsertEntry(au->commands, com, 0);
						break;
					} else {
						OD_DeleteCommand(&com);
					}
					if (e) break;
				}

			}
			sc->last_au_time = au_time + 1;
			/*attach graph to renderer*/
			IS_AttachGraphToRender(priv->inline_scene);
			if (e) return e;

			/*for root streams remove completed AUs (no longer needed)*/
			if (!sc->ESID && !ChainGetCount(au->commands) ) {
				ChainDeleteEntry(sc->AUs, j);
				DeleteChain(au->commands);
				free(au);
				j--;
			}
		}
	}
	if (e) return e;
	if ((priv->load_flags==2) && !nb_updates) return M4EOF;
	return M4OK;
}

const char *M4CTX_GetName(struct _basedecoder *plug)
{
	M4CTXPriv *priv = plug->privateStack;

	switch (priv->load.type) {
	case M4CL_BT: return "MPEG-4 BT Parser";
	case M4CL_VRML: return "VRML 97 Parser";
	case M4CL_X3DV: return "X3D (VRML Syntax) Parser";
	case M4CL_XMTA: return "XMT-A Parser";
	case M4CL_X3D: return "X3D (XML Syntax) Parser";
	case M4CL_SWF: return "Flash (SWF) Emulator";
	case M4CL_MP4: return "MP4 Memory Loader";
	default: return "Undetermined";
	}
}

Bool M4CTX_CanHandleStream(BaseDecoder *ifce, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	if ((StreamType==M4ST_PRIVATE_SCENE) && (ObjectType==1)) return 1;
	return 0;
}

void DeleteContextLoader(BaseDecoder *plug)
{
	M4CTXPriv *priv = plug->privateStack;
	if (priv->fileName) free(priv->fileName);
	assert(!priv->ctx);
	DeleteChain(priv->files_to_delete);
	free(priv);
	free(plug);
}

BaseDecoder *NewContextLoader()
{
	M4CTXPriv *priv;
	SceneDecoder *tmp;
	
	SAFEALLOC(tmp, sizeof(SceneDecoder));
	SAFEALLOC(priv, sizeof(M4CTXPriv));
	priv->files_to_delete = NewChain();

	tmp->privateStack = priv;
	tmp->AttachStream = M4CTX_AttachStream;
	tmp->DetachStream = M4CTX_DetachStream;
	tmp->GetCapabilities = M4CTX_GetCapabilities;
	tmp->SetCapabilities = M4CTX_SetCapabilities;
	tmp->ProcessData = M4CTX_ProcessData;
	tmp->AttachScene = M4CTX_AttachScene;
	tmp->ReleaseScene = M4CTX_ReleaseScene;
	tmp->GetName = M4CTX_GetName;
	tmp->CanHandleStream = M4CTX_CanHandleStream;
	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC Context Loader", "gpac distribution", 0)
	return (BaseDecoder*)tmp;
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
	case M4SCENEDECODERINTERFACE: return NewContextLoader();
	default:
		return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		DeleteContextLoader(ifcd);
		break;
	}
}
