/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS decoder plugin
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
#include <gpac/m4_bifs.h>

typedef struct 
{
	InlineScene *pScene;
	struct _m4_client *app;
	LPBIFSDEC codec;
	u32 PL, nb_streams;
} BIFSPriv;



static M4Err BIFS_GetCapabilities(BaseDecoder *plug, CapObject *cap)
{
	cap->cap.valueINT = 0;
	return M4NotSupported;
}

static M4Err BIFS_SetCapabilities(BaseDecoder *plug, const CapObject capability)
{
	return M4OK;
}

M4Err BIFS_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_scene_decoder)
{
	BIFSPriv *priv = plug->privateStack;
	if (priv->codec) return M4BadParam;
	priv->pScene = scene;
	priv->app = scene->root_od->term;
	
	priv->codec = BIFS_NewDecoder(scene->graph, 0);
	/*attach the clock*/
	BIFS_SetClock(priv->codec, IS_GetSceneTime, scene);
	/*ignore all size info on anim streams*/
	if (!is_scene_decoder) BIFS_IgnoreSizeInfo(priv->codec);
	return M4OK;
}

M4Err BIFS_ReleaseScene(SceneDecoder *plug)
{
	BIFSPriv *priv = plug->privateStack;
	if (!priv->codec || priv->nb_streams) return M4BadParam;
	BIFS_DeleteDecoder(priv->codec);
	priv->codec = NULL;
	return M4OK;
}

static M4Err BIFS_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	BIFSPriv *priv = plug->privateStack;
	M4Err e;
	if (Upstream) return M4NotSupported;
	e = BIFS_ConfigureStream(priv->codec, ES_ID, decSpecInfo, decSpecInfoSize, objectTypeIndication);
	if (!e) priv->nb_streams++;
	return e;
}

static M4Err BIFS_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	M4Err e;
	BIFSPriv *priv = plug->privateStack;
	e = BIFS_RemoveStream(priv->codec, ES_ID);
	if (e) return e;
	priv->nb_streams--;
	return M4OK;
}

static M4Err BIFS_ProcessData(SceneDecoder*plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 AU_time, u32 mmlevel)
{
	M4Err e = M4OK;
	BIFSPriv *priv = plug->privateStack;

	e = BIFS_DecodeAU(priv->codec, ES_ID, inBuffer, inBufferLength);

	/*if scene not attached do it*/
	IS_AttachGraphToRender(priv->pScene);
	return e;
}

Bool BIFS_CanHandleStream(BaseDecoder *ifce, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL)
{
	BIFSPriv *priv = ifce->privateStack;
	if (StreamType!=M4ST_SCENE) return 0;
	switch (ObjectType) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0xFE:
	case 0xFF:
		priv->PL = PL;
		return 1;
	default:
		return 0;
	}
}


void DeleteBIFSDec(BaseDecoder *plug)
{
	BIFSPriv *priv = plug->privateStack;
	/*in case something went wrong*/
	if (priv->codec) BIFS_DeleteDecoder(priv->codec);
	free(priv);
	free(plug);
}

BaseDecoder *NewBIFSDec()
{
	BIFSPriv *priv;
	SceneDecoder *tmp;
	
	SAFEALLOC(tmp, sizeof(SceneDecoder));
	if (!tmp) return NULL;
	SAFEALLOC(priv, sizeof(BIFSPriv));
	priv->codec = NULL;
	tmp->privateStack = priv;
	tmp->AttachStream = BIFS_AttachStream;
	tmp->DetachStream = BIFS_DetachStream;
	tmp->GetCapabilities = BIFS_GetCapabilities;
	tmp->SetCapabilities = BIFS_SetCapabilities;
	tmp->ProcessData = BIFS_ProcessData;
	tmp->AttachScene = BIFS_AttachScene;
	tmp->CanHandleStream = BIFS_CanHandleStream;
	tmp->ReleaseScene = BIFS_ReleaseScene;
	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC BIFS Decoder", "gpac distribution", 0)
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
		return NewBIFSDec();
	default:
		return NULL;
	}
}

void ShutdownInterface(void *ifce)
{
	BaseDecoder *ifcd = (BaseDecoder *)ifce;
	switch (ifcd->InterfaceType) {
	case M4SCENEDECODERINTERFACE:
		DeleteBIFSDec(ifcd);
		break;
	}
}
