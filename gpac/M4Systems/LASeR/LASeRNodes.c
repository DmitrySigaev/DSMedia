#include "m4_laser_dev.h"

#ifdef M4_USE_LASeR

#include <gpac/intern/m4_scenegraph_dev.h>

void LASeRString_Del(LASeRString ls) 
{
	if (ls.string) free(ls.string);	
	ls.string = NULL;
	ls.length = 0;
}

static void LASeRPointSequence_Del(LASeRPointSequence ps) 
{
	if (ps.x) free(ps.x);	
	ps.x = NULL;
	if (ps.y) free(ps.y);	
	ps.y = NULL;
	ps.nbPoints = 0;
}

static void LASeRUrlParam_Del(LASeRUrlParam url)
{
	LASeRString_Del(url.name);
}

static void LASeRActionKey_Del(LASeRActionKey ak) 
{
	LASeRString_Del(ak.newText);
}

static void LASeRTransform_Del(SFNode *node)
{
	LASeRTransform *p = (LASeRTransform *) node;
	DestroyChildrenNode((SFNode *) p);	
	SFNode_Delete((SFNode *) p);
}

void *LASeRTransform_Create() 
{
	LASeRTransform *p;
	SAFEALLOC(p, sizeof(LASeRTransform));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRTransform);
	SetupChildrenNode((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Transform";
	((SFNode *)p)->sgprivate->node_del = LASeRTransform_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	p->hasChoice = 0;
	p->hasDelta = 0;
	p->m.xx = 1; p->m.yy = 1;
	p->m.xy = 0; p->m.yx = 0;
	p->m.xz = 0; p->m.yz = 0;

	return (SFNode *)p;
}

static void LASeRShape_Del(SFNode *node)
{
	LASeRShape *p = (LASeRShape *) node;

	LASeRPointSequence_Del(p->initpointseq);
	if (p->morepointseqs) {
		u32 i;
		for (i = 0; i < p->nbAdditionalSeqs; i++) {
			LASeRPointSequence_Del(p->morepointseqs[i]);
		}
		free(p->morepointseqs);
		p->morepointseqs = NULL;
	}

	SFNode_Delete((SFNode *) p);
}

void *LASeRShape_Create() 
{
	LASeRShape *p;
	SAFEALLOC(p, sizeof(LASeRShape));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRShape);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Shape";
	((SFNode *)p)->sgprivate->node_del = LASeRShape_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	p->width =1.0f;

	return (SFNode *)p;
}

static void LASeRAction_Del(SFNode *node)
{
	LASeRAction *p = (LASeRAction *) node;
	u32 i;
	
	for (i = 0; i <p->nbUrlParams; i++)
		LASeRUrlParam_Del(p->urlParams[i]);
	free(p->urlParams);
	p->urlParams = NULL;
	p->nbUrlParams = 0;

	for (i = 0; i <p->nbActionKeys; i++)
		LASeRActionKey_Del(p->actionKeys[i]);
	free(p->actionKeys);
	p->actionKeys = NULL;
	p->nbActionKeys = 0;

	SFNode_Delete((SFNode *) p);
}

void *LASeRAction_Create() 
{
	LASeRAction *p;
	SAFEALLOC(p, sizeof(LASeRAction));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRAction);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Action";
	((SFNode *)p)->sgprivate->node_del = LASeRAction_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRVideo_Del(SFNode *node)
{
	LASeRVideo *p = (LASeRVideo *) node;
	SFNode_Delete((SFNode *) p);
}

void *LASeRVideo_Create() 
{
	LASeRVideo *p;
	SAFEALLOC(p, sizeof(LASeRVideo));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRVideo);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Video";
	((SFNode *)p)->sgprivate->node_del = LASeRVideo_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRAudio_Del(SFNode *node)
{
	LASeRAudio *p = (LASeRAudio *) node;
	SFNode_Delete((SFNode *) p);
}

void *LASeRAudio_Create() 
{
	LASeRAudio *p;
	SAFEALLOC(p, sizeof(LASeRAudio));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRAudio);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Audio";
	((SFNode *)p)->sgprivate->node_del = LASeRAudio_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRBitmap_Del(SFNode *node)
{
	LASeRBitmap *p = (LASeRBitmap *) node;
	SFNode_Delete((SFNode *) p);
}

void *LASeRBitmap_Create() 
{
	LASeRBitmap *p;
	SAFEALLOC(p, sizeof(LASeRBitmap));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRBitmap);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Bitmap";
	((SFNode *)p)->sgprivate->node_del = LASeRBitmap_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRText_Del(SFNode *node)
{
	LASeRText *p = (LASeRText *) node;
	LASeRString_Del(p->string);
	SFNode_Delete((SFNode *) p);
}

void *LASeRText_Create() 
{
	LASeRText *p;
	SAFEALLOC(p, sizeof(LASeRText));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRText);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Text";
	((SFNode *)p)->sgprivate->node_del = LASeRText_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRAnimateTransform_Del(SFNode *node)
{
	LASeRAnimateTransform *p = (LASeRAnimateTransform *) node;
	if (p->keys) free(p->keys);
	p->keys = NULL;
	if (p->durations) free(p->durations);
	p->durations = NULL;
	p->nbKeys = 0;
	SFNode_Delete((SFNode *) p);
}

void *LASeRAnimateTransform_Create() 
{
	LASeRAnimateTransform *p;
	SAFEALLOC(p, sizeof(LASeRAnimateTransform));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRAnimateTransform);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "AnimateTransform";
	((SFNode *)p)->sgprivate->node_del = LASeRAnimateTransform_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRAnimateColor_Del(SFNode *node)
{
	LASeRAnimateColor *p = (LASeRAnimateColor *) node;
	if (p->keys) free(p->keys);
	p->keys = NULL;
	if (p->durations) free(p->durations);
	p->durations = NULL;
	p->nbKeys = 0;
	SFNode_Delete((SFNode *) p);
}

void *LASeRAnimateColor_Create() 
{
	LASeRAnimateColor *p;
	SAFEALLOC(p, sizeof(LASeRAnimateColor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRAnimateColor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "AnimateColor";
	((SFNode *)p)->sgprivate->node_del = LASeRAnimateColor_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRAnimateActivate_Del(SFNode *node)
{
	LASeRAnimateActivate *p = (LASeRAnimateActivate *) node;
	if (p->keys) free(p->keys);
	p->keys = NULL;
	if (p->durations) free(p->durations);
	p->durations = NULL;
	p->nbKeys = 0;
	SFNode_Delete((SFNode *) p);
}

void *LASeRAnimateActivate_Create() 
{
	LASeRAnimateActivate *p;
	SAFEALLOC(p, sizeof(LASeRAnimateActivate));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRAnimateActivate);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "AnimateActivate";
	((SFNode *)p)->sgprivate->node_del = LASeRAnimateActivate_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRUse_Del(SFNode *node)
{
	LASeRUse *p = (LASeRUse *) node;
	// Do not delete the used_node
	// it should be delete elsewhere
	SFNode_Delete((SFNode *) p);
}

void *LASeRUse_Create() 
{
	LASeRUse *p;
	SAFEALLOC(p, sizeof(LASeRUse));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRUse);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Use";
	((SFNode *)p)->sgprivate->node_del = LASeRUse_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRConditional_Del(SFNode *node)
{
	LASeRConditional *p = (LASeRConditional *) node;

	u32 i;
	for (i = 0; i<p->nbUpdates; i++) {
		//LASeRUpdate_Del(p->updates[i]);
	}
	free(p->updates);
	p->updates = NULL;
	p->nbUpdates = 0;

	SFNode_Delete((SFNode *) p);
}

void *LASeRConditional_Create() 
{
	LASeRConditional *p;
	SAFEALLOC(p, sizeof(LASeRConditional));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRConditional);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Conditional";
	((SFNode *)p)->sgprivate->node_del = LASeRConditional_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRTextInput_Del(SFNode *node)
{
	LASeRTextInput *p = (LASeRTextInput *) node;
	LASeRString_Del(p->title);
	SFNode_Delete((SFNode *) p);
}

void *LASeRTextInput_Create() 
{
	LASeRTextInput *p;
	SAFEALLOC(p, sizeof(LASeRTextInput));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRTextInput);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TextInput";
	((SFNode *)p)->sgprivate->node_del = LASeRTextInput_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRCursor_Del(SFNode *node)
{
	LASeRCursor *p = (LASeRCursor *) node;
	if (p->keys) free(p->keys);
	p->keys = NULL;
	p->nbKeys = 0;

	SFNode_Delete((SFNode *) p);
}

void *LASeRCursor_Create() 
{
	LASeRCursor *p;
	SAFEALLOC(p, sizeof(LASeRCursor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRCursor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Cursor";
	((SFNode *)p)->sgprivate->node_del = LASeRCursor_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

static void LASeRBackground_Del(SFNode *node)
{
	LASeRBackground *p = (LASeRBackground *) node;
	SFNode_Delete((SFNode *) p);
}

void *LASeRBackground_Create() 
{
	LASeRBackground *p;
	SAFEALLOC(p, sizeof(LASeRBackground));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_LASeRBackground);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Background";
	((SFNode *)p)->sgprivate->node_del = LASeRLASeRBackground_Del;
	((SFNode *)p)->sgprivate->get_field_count = NULL;
	((SFNode *)p)->sgprivate->get_field_index = NULL;
	((SFNode *)p)->sgprivate->get_field = NULL;
	((SFNode *)p)->sgprivate->get_aq_info = NULL;
	((SFNode *)p)->sgprivate->child_ndt = 0;

#endif

	/*default field values*/
	return (SFNode *)p;
}

SFNode *LASeR_CreateNode(u32 NodeTag)
{
	switch (NodeTag) {
	case TAG_LASeRTransform:
		return LASeRTransform_Create();
	case TAG_LASeRShape: 
		return LASeRShape_Create();
	case TAG_LASeRAction:
		return LASeRAction_Create();
	case TAG_LASeRVideo:
		return LASeRVideo_Create();
	case TAG_LASeRAudio:
		return LASeRAudio_Create();
	case TAG_LASeRBitmap:
		return LASeRBitmap_Create();
	case TAG_LASeRText:
		return LASeRText_Create();
	case TAG_LASeRAnimateTransform:
		return LASeRAnimateTransform_Create();
	case TAG_LASeRAnimateColor:
		return LASeRAnimateColor_Create();
	case TAG_LASeRAnimateActivate:
		return LASeRAnimateActivate_Create();
	case TAG_LASeRUse:
		return LASeRUse_Create();
	case TAG_LASeRConditional:
		return LASeRConditional_Create();
	case TAG_LASeRTextInput:
		return LASeRTextInput_Create();
	case TAG_LASeRCursor:
		return LASeRCursor_Create();
	case TAG_LASeRBackground:
		return LASeRBackground_Create();
	default:
		return NULL;
	}
}

#ifndef NODE_USE_POINTERS
const char *LASeR_GetNodeName(u32 NodeTag)
{
	switch (NodeTag) {
	case TAG_LASeRTransform:
		return "Transform";
	case TAG_LASeRShape: 
		return "Shape";
	case TAG_LASeRAction:
		return "Action";
	case TAG_LASeRVideo:
		return "Video";
	case TAG_LASeRAudio:
		return "Audio";
	case TAG_LASeRBitmap:
		return "Bitmap";
	case TAG_LASeRText:
		return "Text";
	case TAG_LASeRAnimateTransform:
		return "AnimateTransform";
	case TAG_LASeRAnimateColor:
		return "AnimateColor";
	case TAG_LASeRAnimateActivate:
		return "AnimateActivate";
	case TAG_LASeRUse:
		return "Use";
	case TAG_LASeRConditional:
		return "Conditional";
	case TAG_LASeRTextInput:
		return "TextInput";
	case TAG_LASeRCursor:
		return "Cursor";
	default:
		return "Unknown Node";
	}
}

void LASeRNode_Del(SFNode *node)
{
	switch (node->sgprivate->tag) {
	case TAG_LASeRTransform:
		LASeRTransform_Del(node); return;
	case TAG_LASeRShape: 
		LASeRShape_Del(node); return;
	case TAG_LASeRAction:
		LASeRAction_Del(node); return;
	case TAG_LASeRVideo:
		LASeRVideo_Del(node); return;
	case TAG_LASeRAudio:
		LASeRAudio_Del(node); return;
	case TAG_LASeRBitmap:
		LASeRBitmap_Del(node); return;
	case TAG_LASeRText:
		LASeRText_Del(node); return;
	case TAG_LASeRAnimateTransform:
		LASeRAnimateTransform_Del(node); return;
	case TAG_LASeRAnimateColor:
		LASeRAnimateColor_Del(node); return;
	case TAG_LASeRAnimateActivate:
		LASeRAnimateActivate_Del(node); return;
	case TAG_LASeRUse:
		LASeRUse_Del(node); return;
	case TAG_LASeRConditional:
		LASeRConditional_Del(node); return;
	case TAG_LASeRTextInput:
		LASeRTextInput_Del(node); return;
	case TAG_LASeRCursor:
		LASeRCursor_Del(node); return;
	case TAG_LASeRBackground:
		LASeRBackground_Del(node); return;
	default:
		return;
	}
}

u32 LASeRNode_GetFieldCount(SFNode *node, u8 code_mode)
{
	return 0;
}

M4Err LASeRNode_GetFieldIndex(SFNode *node, u32 inField, u8 code_mode, u32 *fieldIndex)
{
	return M4BadParam;
}

M4Err LASeRNode_GetField(SFNode *node, FieldInfo *field)
{
	return M4BadParam;
}
#endif


#endif	//M4_USE_LASeR
