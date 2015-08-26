#include "LASeRCodec.h"

#ifdef M4_USE_LASeR

Double IS_GetSceneTime(void *is);

M4Err LASeR_AttachScene(SceneDecoder *plug, InlineScene *scene, Bool is_inline)
{
	LASeRPriv *priv = plug->privateStack;
	if (priv->codec) return M4BadParam;
	priv->pScene = scene;
	priv->app = scene->root_od->term;
	
	priv->codec = LASeR_NewDecoder(scene->graph);
	/*attach the clock*/
	LASeR_SetClock(priv->codec, IS_GetSceneTime, scene);
	return M4OK;
}

static M4Err LASeR_AttachStream(BaseDecoder *plug, 
									 u16 ES_ID, 
									 unsigned char *decSpecInfo, 
									 u32 decSpecInfoSize, 
									 u16 DependsOnES_ID,
									 u32 objectTypeIndication, 
									 Bool Upstream)
{
	LASeRPriv *priv = plug->privateStack;
	if (Upstream) return M4NotSupported;
	return LASeR_ConfigureStream(priv->codec, decSpecInfo, decSpecInfoSize);
}

static M4Err LASeR_DetachStream(BaseDecoder *plug, u16 ES_ID)
{
	return M4OK;
}

static M4Err LASeR_GetCapabilities(BaseDecoder*plug,
										CapObject *capability)
{
	switch (capability->CapCode) {
	default:
		capability->cap.valueINT = 0;
		break;
	}
	return M4OK;
}


static M4Err LASeR_SetCapabilities(BaseDecoder *plug, 
										const CapObject capability)
{
	return M4OK;
}

static M4Err LASeR_ProcessData(SceneDecoder *plug, unsigned char *inBuffer, u32 inBufferLength, 
								u16 ES_ID, u32 AU_time, u32 mmlevel)
{
	LASeRPriv *priv = plug->privateStack;
	
	LASeR_ParseSceneUnit(priv->codec, inBuffer, inBufferLength);

	/*if scene not attached do it*/
	IS_AttachGraphToRender(priv->pScene);
	return M4OK;
}

void LASeRDec_Delete(BaseDecoder *plug)
{
	LASeRPriv *priv = plug->privateStack;
	LASeR_DeleteDecoder(priv->codec);
	free(priv);
	free(plug);
}

BaseDecoder *NewLASeRCodec()
{
	LASeRPriv *priv;
	SceneDecoder *tmp;
	
	SAFEALLOC(tmp,sizeof(SceneDecoder));
	SAFEALLOC(priv, sizeof(LASeRPriv));
	priv->codec = NULL;
	tmp->privateStack = priv;
	tmp->AttachStream = LASeR_AttachStream;
	tmp->DetachStream = LASeR_DetachStream;
	tmp->GetCapabilities = LASeR_GetCapabilities;
	tmp->SetCapabilities = LASeR_SetCapabilities;
	tmp->ProcessData = LASeR_ProcessData;
	tmp->AttachScene = LASeR_AttachScene;
	M4_REG_PLUG(tmp, M4SCENEDECODERINTERFACE, "GPAC LASeR Decoder", "gpac distribution", 0)
	return (BaseDecoder *) tmp;
}

#endif	//M4_USE_LASeR


