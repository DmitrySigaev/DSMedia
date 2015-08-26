/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004
 *					All rights reserved
 *
 *  This file is part of GPAC / X3D Scene Graph sub-project
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


/*
	DO NOT MOFIFY - File generated on GMT Wed Oct 20 11:07:37 2004

	BY X3DGen for GPAC Version 0.2.2 DEV
*/


#include <gpac/m4_x3d_nodes.h>

#include <gpac/intern/m4_scenegraph_dev.h>

/*for NDT tag definitions*/
#include <gpac/m4_mpeg4_nodes.h>

/*
	Anchor Node deletion
*/

static void Anchor_Del(SFNode *node)
{
	X_Anchor *p = (X_Anchor *) node;
	DestroyVRMLParent(node);	
	SFString_Del(p->description);
	MFString_Del(p->parameter);
	MFURL_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Anchor_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err Anchor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Anchor *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Anchor *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Anchor *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Anchor *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Anchor *)node)->children;
		return M4OK;
	case 3:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_Anchor *) node)->description;
		return M4OK;
	case 4:
		info->name = "parameter";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_Anchor *) node)->parameter;
		return M4OK;
	case 5:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Anchor *) node)->url;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Anchor *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Anchor_Create()
{
	X_Anchor *p;
	SAFEALLOC(p, sizeof(X_Anchor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Anchor);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Anchor";
	((SFNode *)p)->sgprivate->node_del = Anchor_Del;
	((SFNode *)p)->sgprivate->get_field_count = Anchor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Anchor_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Appearance Node deletion
*/

static void Appearance_Del(SFNode *node)
{
	X_Appearance *p = (X_Appearance *) node;
	Node_Unregister((SFNode *) p->material, node);	
	Node_Unregister((SFNode *) p->texture, node);	
	Node_Unregister((SFNode *) p->textureTransform, node);	
	Node_Unregister((SFNode *) p->fillProperties, node);	
	Node_Unregister((SFNode *) p->lineProperties, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Appearance_get_field_count(SFNode *node)
{
	return 6;
}

static M4Err Appearance_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "material";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMaterialNode;
		info->far_ptr = & ((X_Appearance *)node)->material;
		return M4OK;
	case 1:
		info->name = "texture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_Appearance *)node)->texture;
		return M4OK;
	case 2:
		info->name = "textureTransform";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureTransformNode;
		info->far_ptr = & ((X_Appearance *)node)->textureTransform;
		return M4OK;
	case 3:
		info->name = "fillProperties";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFFillPropertiesNode;
		info->far_ptr = & ((X_Appearance *)node)->fillProperties;
		return M4OK;
	case 4:
		info->name = "lineProperties";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFX3DLinePropertiesNode;
		info->far_ptr = & ((X_Appearance *)node)->lineProperties;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Appearance *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Appearance_Create()
{
	X_Appearance *p;
	SAFEALLOC(p, sizeof(X_Appearance));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Appearance);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Appearance";
	((SFNode *)p)->sgprivate->node_del = Appearance_Del;
	((SFNode *)p)->sgprivate->get_field_count = Appearance_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Appearance_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Arc2D Node deletion
*/

static void Arc2D_Del(SFNode *node)
{
	X_Arc2D *p = (X_Arc2D *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Arc2D_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err Arc2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "endAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Arc2D *) node)->endAngle;
		return M4OK;
	case 1:
		info->name = "radius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Arc2D *) node)->radius;
		return M4OK;
	case 2:
		info->name = "startAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Arc2D *) node)->startAngle;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Arc2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Arc2D_Create()
{
	X_Arc2D *p;
	SAFEALLOC(p, sizeof(X_Arc2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Arc2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Arc2D";
	((SFNode *)p)->sgprivate->node_del = Arc2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Arc2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Arc2D_get_field;

#endif


	/*default field values*/
	p->endAngle = (SFFloat) 1.5707963;
	p->radius = (SFFloat) 1;
	p->startAngle = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	ArcClose2D Node deletion
*/

static void ArcClose2D_Del(SFNode *node)
{
	X_ArcClose2D *p = (X_ArcClose2D *) node;
	SFString_Del(p->closureType);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ArcClose2D_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err ArcClose2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "closureType";
		info->eventType = ET_Field;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_ArcClose2D *) node)->closureType;
		return M4OK;
	case 1:
		info->name = "endAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ArcClose2D *) node)->endAngle;
		return M4OK;
	case 2:
		info->name = "radius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ArcClose2D *) node)->radius;
		return M4OK;
	case 3:
		info->name = "startAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ArcClose2D *) node)->startAngle;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ArcClose2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ArcClose2D_Create()
{
	X_ArcClose2D *p;
	SAFEALLOC(p, sizeof(X_ArcClose2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ArcClose2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ArcClose2D";
	((SFNode *)p)->sgprivate->node_del = ArcClose2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = ArcClose2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ArcClose2D_get_field;

#endif


	/*default field values*/
	p->closureType.buffer = malloc(sizeof(char) * 4);
	strcpy(p->closureType.buffer, "PIE");
	p->endAngle = (SFFloat) 1.5707963;
	p->radius = (SFFloat) 1;
	p->startAngle = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	AudioClip Node deletion
*/

static void AudioClip_Del(SFNode *node)
{
	X_AudioClip *p = (X_AudioClip *) node;
	SFString_Del(p->description);
	MFURL_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 AudioClip_get_field_count(SFNode *node)
{
	return 13;
}

static M4Err AudioClip_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_AudioClip *) node)->description;
		return M4OK;
	case 1:
		info->name = "loop";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_AudioClip *) node)->loop;
		return M4OK;
	case 2:
		info->name = "pitch";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_AudioClip *) node)->pitch;
		return M4OK;
	case 3:
		info->name = "startTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->startTime;
		return M4OK;
	case 4:
		info->name = "stopTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->stopTime;
		return M4OK;
	case 5:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_AudioClip *) node)->url;
		return M4OK;
	case 6:
		info->name = "duration_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->duration_changed;
		return M4OK;
	case 7:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_AudioClip *) node)->isActive;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_AudioClip *)node)->metadata;
		return M4OK;
	case 9:
		info->name = "pauseTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->pauseTime;
		return M4OK;
	case 10:
		info->name = "resumeTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->resumeTime;
		return M4OK;
	case 11:
		info->name = "elapsedTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_AudioClip *) node)->elapsedTime;
		return M4OK;
	case 12:
		info->name = "isPaused";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_AudioClip *) node)->isPaused;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *AudioClip_Create()
{
	X_AudioClip *p;
	SAFEALLOC(p, sizeof(X_AudioClip));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_AudioClip);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "AudioClip";
	((SFNode *)p)->sgprivate->node_del = AudioClip_Del;
	((SFNode *)p)->sgprivate->get_field_count = AudioClip_get_field_count;
	((SFNode *)p)->sgprivate->get_field = AudioClip_get_field;

#endif


	/*default field values*/
	p->pitch = (SFFloat) 1.0;
	p->startTime = 0;
	p->stopTime = 0;
	p->pauseTime = 0;
	p->resumeTime = 0;
	return (SFNode *)p;
}


/*
	Background Node deletion
*/

static void Background_Del(SFNode *node)
{
	X_Background *p = (X_Background *) node;
	MFFloat_Del(p->groundAngle);
	MFColor_Del(p->groundColor);
	MFURL_Del(p->backUrl);
	MFURL_Del(p->bottomUrl);
	MFURL_Del(p->frontUrl);
	MFURL_Del(p->leftUrl);
	MFURL_Del(p->rightUrl);
	MFURL_Del(p->topUrl);
	MFFloat_Del(p->skyAngle);
	MFColor_Del(p->skyColor);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Background_get_field_count(SFNode *node)
{
	return 14;
}

static M4Err Background_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_bind";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Background *)node)->on_set_bind;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Background *) node)->set_bind;
		return M4OK;
	case 1:
		info->name = "groundAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_Background *) node)->groundAngle;
		return M4OK;
	case 2:
		info->name = "groundColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_Background *) node)->groundColor;
		return M4OK;
	case 3:
		info->name = "backUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->backUrl;
		return M4OK;
	case 4:
		info->name = "bottomUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->bottomUrl;
		return M4OK;
	case 5:
		info->name = "frontUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->frontUrl;
		return M4OK;
	case 6:
		info->name = "leftUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->leftUrl;
		return M4OK;
	case 7:
		info->name = "rightUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->rightUrl;
		return M4OK;
	case 8:
		info->name = "topUrl";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Background *) node)->topUrl;
		return M4OK;
	case 9:
		info->name = "skyAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_Background *) node)->skyAngle;
		return M4OK;
	case 10:
		info->name = "skyColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_Background *) node)->skyColor;
		return M4OK;
	case 11:
		info->name = "isBound";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Background *) node)->isBound;
		return M4OK;
	case 12:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Background *)node)->metadata;
		return M4OK;
	case 13:
		info->name = "bindTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_Background *) node)->bindTime;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Background_Create()
{
	X_Background *p;
	SAFEALLOC(p, sizeof(X_Background));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Background);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Background";
	((SFNode *)p)->sgprivate->node_del = Background_Del;
	((SFNode *)p)->sgprivate->get_field_count = Background_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Background_get_field;

#endif


	/*default field values*/
	p->skyColor.vals = malloc(sizeof(SFColor)*1);
	p->skyColor.count = 1;
	p->skyColor.vals[0].red = (SFFloat) 0;
	p->skyColor.vals[0].green = (SFFloat) 0;
	p->skyColor.vals[0].blue = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	Billboard Node deletion
*/

static void Billboard_Del(SFNode *node)
{
	X_Billboard *p = (X_Billboard *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Billboard_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err Billboard_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Billboard *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Billboard *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Billboard *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Billboard *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Billboard *)node)->children;
		return M4OK;
	case 3:
		info->name = "axisOfRotation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Billboard *) node)->axisOfRotation;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Billboard *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Billboard_Create()
{
	X_Billboard *p;
	SAFEALLOC(p, sizeof(X_Billboard));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Billboard);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Billboard";
	((SFNode *)p)->sgprivate->node_del = Billboard_Del;
	((SFNode *)p)->sgprivate->get_field_count = Billboard_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Billboard_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	p->axisOfRotation.x = (SFFloat) 0;
	p->axisOfRotation.y = (SFFloat) 1;
	p->axisOfRotation.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	BooleanFilter Node deletion
*/

static void BooleanFilter_Del(SFNode *node)
{
	X_BooleanFilter *p = (X_BooleanFilter *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 BooleanFilter_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err BooleanFilter_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_boolean";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanFilter *)node)->on_set_boolean;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanFilter *) node)->set_boolean;
		return M4OK;
	case 1:
		info->name = "inputFalse";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanFilter *) node)->inputFalse;
		return M4OK;
	case 2:
		info->name = "inputNegate";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanFilter *) node)->inputNegate;
		return M4OK;
	case 3:
		info->name = "inputTrue";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanFilter *) node)->inputTrue;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_BooleanFilter *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *BooleanFilter_Create()
{
	X_BooleanFilter *p;
	SAFEALLOC(p, sizeof(X_BooleanFilter));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_BooleanFilter);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "BooleanFilter";
	((SFNode *)p)->sgprivate->node_del = BooleanFilter_Del;
	((SFNode *)p)->sgprivate->get_field_count = BooleanFilter_get_field_count;
	((SFNode *)p)->sgprivate->get_field = BooleanFilter_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	BooleanSequencer Node deletion
*/

static void BooleanSequencer_Del(SFNode *node)
{
	X_BooleanSequencer *p = (X_BooleanSequencer *) node;
	MFFloat_Del(p->key);
	MFBool_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 BooleanSequencer_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err BooleanSequencer_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "next";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanSequencer *)node)->on_next;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanSequencer *) node)->next;
		return M4OK;
	case 1:
		info->name = "previous";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanSequencer *)node)->on_previous;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanSequencer *) node)->previous;
		return M4OK;
	case 2:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanSequencer *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_BooleanSequencer *) node)->set_fraction;
		return M4OK;
	case 3:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_BooleanSequencer *) node)->key;
		return M4OK;
	case 4:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFBool;
		info->far_ptr = & ((X_BooleanSequencer *) node)->keyValue;
		return M4OK;
	case 5:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanSequencer *) node)->value_changed;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_BooleanSequencer *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *BooleanSequencer_Create()
{
	X_BooleanSequencer *p;
	SAFEALLOC(p, sizeof(X_BooleanSequencer));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_BooleanSequencer);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "BooleanSequencer";
	((SFNode *)p)->sgprivate->node_del = BooleanSequencer_Del;
	((SFNode *)p)->sgprivate->get_field_count = BooleanSequencer_get_field_count;
	((SFNode *)p)->sgprivate->get_field = BooleanSequencer_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	BooleanToggle Node deletion
*/

static void BooleanToggle_Del(SFNode *node)
{
	X_BooleanToggle *p = (X_BooleanToggle *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 BooleanToggle_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err BooleanToggle_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_boolean";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanToggle *)node)->on_set_boolean;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanToggle *) node)->set_boolean;
		return M4OK;
	case 1:
		info->name = "toggle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanToggle *) node)->toggle;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_BooleanToggle *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *BooleanToggle_Create()
{
	X_BooleanToggle *p;
	SAFEALLOC(p, sizeof(X_BooleanToggle));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_BooleanToggle);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "BooleanToggle";
	((SFNode *)p)->sgprivate->node_del = BooleanToggle_Del;
	((SFNode *)p)->sgprivate->get_field_count = BooleanToggle_get_field_count;
	((SFNode *)p)->sgprivate->get_field = BooleanToggle_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	BooleanTrigger Node deletion
*/

static void BooleanTrigger_Del(SFNode *node)
{
	X_BooleanTrigger *p = (X_BooleanTrigger *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 BooleanTrigger_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err BooleanTrigger_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_triggerTime";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_BooleanTrigger *)node)->on_set_triggerTime;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_BooleanTrigger *) node)->set_triggerTime;
		return M4OK;
	case 1:
		info->name = "triggerTrue";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_BooleanTrigger *) node)->triggerTrue;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_BooleanTrigger *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *BooleanTrigger_Create()
{
	X_BooleanTrigger *p;
	SAFEALLOC(p, sizeof(X_BooleanTrigger));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_BooleanTrigger);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "BooleanTrigger";
	((SFNode *)p)->sgprivate->node_del = BooleanTrigger_Del;
	((SFNode *)p)->sgprivate->get_field_count = BooleanTrigger_get_field_count;
	((SFNode *)p)->sgprivate->get_field = BooleanTrigger_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Box Node deletion
*/

static void Box_Del(SFNode *node)
{
	X_Box *p = (X_Box *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Box_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Box_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "size";
		info->eventType = ET_Field;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Box *) node)->size;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Box *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Box_Create()
{
	X_Box *p;
	SAFEALLOC(p, sizeof(X_Box));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Box);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Box";
	((SFNode *)p)->sgprivate->node_del = Box_Del;
	((SFNode *)p)->sgprivate->get_field_count = Box_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Box_get_field;

#endif


	/*default field values*/
	p->size.x = (SFFloat) 2;
	p->size.y = (SFFloat) 2;
	p->size.z = (SFFloat) 2;
	return (SFNode *)p;
}


/*
	Circle2D Node deletion
*/

static void Circle2D_Del(SFNode *node)
{
	X_Circle2D *p = (X_Circle2D *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Circle2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Circle2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "radius";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Circle2D *) node)->radius;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Circle2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Circle2D_Create()
{
	X_Circle2D *p;
	SAFEALLOC(p, sizeof(X_Circle2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Circle2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Circle2D";
	((SFNode *)p)->sgprivate->node_del = Circle2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Circle2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Circle2D_get_field;

#endif


	/*default field values*/
	p->radius = (SFFloat) 1;
	return (SFNode *)p;
}


/*
	Collision Node deletion
*/

static void Collision_Del(SFNode *node)
{
	X_Collision *p = (X_Collision *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->proxy, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Collision_get_field_count(SFNode *node)
{
	return 8;
}

static M4Err Collision_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Collision *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Collision *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Collision *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Collision *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Collision *)node)->children;
		return M4OK;
	case 3:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Collision *) node)->enabled;
		return M4OK;
	case 4:
		info->name = "proxy";
		info->eventType = ET_Field;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Collision *)node)->proxy;
		return M4OK;
	case 5:
		info->name = "collideTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_Collision *) node)->collideTime;
		return M4OK;
	case 6:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Collision *) node)->isActive;
		return M4OK;
	case 7:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Collision *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Collision_Create()
{
	X_Collision *p;
	SAFEALLOC(p, sizeof(X_Collision));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Collision);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Collision";
	((SFNode *)p)->sgprivate->node_del = Collision_Del;
	((SFNode *)p)->sgprivate->get_field_count = Collision_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Collision_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	p->enabled = 1;
	return (SFNode *)p;
}


/*
	Color Node deletion
*/

static void Color_Del(SFNode *node)
{
	X_Color *p = (X_Color *) node;
	MFColor_Del(p->color);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Color_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Color_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_Color *) node)->color;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Color *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Color_Create()
{
	X_Color *p;
	SAFEALLOC(p, sizeof(X_Color));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Color);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Color";
	((SFNode *)p)->sgprivate->node_del = Color_Del;
	((SFNode *)p)->sgprivate->get_field_count = Color_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Color_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	ColorInterpolator Node deletion
*/

static void ColorInterpolator_Del(SFNode *node)
{
	X_ColorInterpolator *p = (X_ColorInterpolator *) node;
	MFFloat_Del(p->key);
	MFColor_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ColorInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err ColorInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_ColorInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ColorInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_ColorInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_ColorInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_ColorInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ColorInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ColorInterpolator_Create()
{
	X_ColorInterpolator *p;
	SAFEALLOC(p, sizeof(X_ColorInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ColorInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ColorInterpolator";
	((SFNode *)p)->sgprivate->node_del = ColorInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = ColorInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ColorInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	ColorRGBA Node deletion
*/

static void ColorRGBA_Del(SFNode *node)
{
	X_ColorRGBA *p = (X_ColorRGBA *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ColorRGBA_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err ColorRGBA_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColorRGBA;
		info->far_ptr = & ((X_ColorRGBA *) node)->color;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ColorRGBA *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ColorRGBA_Create()
{
	X_ColorRGBA *p;
	SAFEALLOC(p, sizeof(X_ColorRGBA));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ColorRGBA);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ColorRGBA";
	((SFNode *)p)->sgprivate->node_del = ColorRGBA_Del;
	((SFNode *)p)->sgprivate->get_field_count = ColorRGBA_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ColorRGBA_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Cone Node deletion
*/

static void Cone_Del(SFNode *node)
{
	X_Cone *p = (X_Cone *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Cone_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err Cone_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "bottomRadius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Cone *) node)->bottomRadius;
		return M4OK;
	case 1:
		info->name = "height";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Cone *) node)->height;
		return M4OK;
	case 2:
		info->name = "side";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Cone *) node)->side;
		return M4OK;
	case 3:
		info->name = "bottom";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Cone *) node)->bottom;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Cone *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Cone_Create()
{
	X_Cone *p;
	SAFEALLOC(p, sizeof(X_Cone));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Cone);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Cone";
	((SFNode *)p)->sgprivate->node_del = Cone_Del;
	((SFNode *)p)->sgprivate->get_field_count = Cone_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Cone_get_field;

#endif


	/*default field values*/
	p->bottomRadius = (SFFloat) 1;
	p->height = (SFFloat) 2;
	p->side = 1;
	p->bottom = 1;
	return (SFNode *)p;
}


/*
	Contour2D Node deletion
*/

static void Contour2D_Del(SFNode *node)
{
	X_Contour2D *p = (X_Contour2D *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Contour2D_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err Contour2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Contour2D *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFNurbsControlCurveNode;
		info->far_ptr = & ((X_Contour2D *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Contour2D *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFNurbsControlCurveNode;
		info->far_ptr = & ((X_Contour2D *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFNurbsControlCurveNode;
		info->far_ptr = & ((X_Contour2D *)node)->children;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Contour2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Contour2D_Create()
{
	X_Contour2D *p;
	SAFEALLOC(p, sizeof(X_Contour2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Contour2D);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Contour2D";
	((SFNode *)p)->sgprivate->node_del = Contour2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Contour2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Contour2D_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SFNurbsControlCurveNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	ContourPolyline2D Node deletion
*/

static void ContourPolyline2D_Del(SFNode *node)
{
	X_ContourPolyline2D *p = (X_ContourPolyline2D *) node;
	MFVec2f_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ContourPolyline2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err ContourPolyline2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_ContourPolyline2D *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ContourPolyline2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ContourPolyline2D_Create()
{
	X_ContourPolyline2D *p;
	SAFEALLOC(p, sizeof(X_ContourPolyline2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ContourPolyline2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ContourPolyline2D";
	((SFNode *)p)->sgprivate->node_del = ContourPolyline2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = ContourPolyline2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ContourPolyline2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Coordinate Node deletion
*/

static void Coordinate_Del(SFNode *node)
{
	X_Coordinate *p = (X_Coordinate *) node;
	MFVec3f_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Coordinate_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Coordinate_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_Coordinate *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Coordinate *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Coordinate_Create()
{
	X_Coordinate *p;
	SAFEALLOC(p, sizeof(X_Coordinate));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Coordinate);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Coordinate";
	((SFNode *)p)->sgprivate->node_del = Coordinate_Del;
	((SFNode *)p)->sgprivate->get_field_count = Coordinate_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Coordinate_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	CoordinateDouble Node deletion
*/

static void CoordinateDouble_Del(SFNode *node)
{
	X_CoordinateDouble *p = (X_CoordinateDouble *) node;
	MFVec3d_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 CoordinateDouble_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err CoordinateDouble_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3d;
		info->far_ptr = & ((X_CoordinateDouble *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_CoordinateDouble *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *CoordinateDouble_Create()
{
	X_CoordinateDouble *p;
	SAFEALLOC(p, sizeof(X_CoordinateDouble));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_CoordinateDouble);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "CoordinateDouble";
	((SFNode *)p)->sgprivate->node_del = CoordinateDouble_Del;
	((SFNode *)p)->sgprivate->get_field_count = CoordinateDouble_get_field_count;
	((SFNode *)p)->sgprivate->get_field = CoordinateDouble_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Coordinate2D Node deletion
*/

static void Coordinate2D_Del(SFNode *node)
{
	X_Coordinate2D *p = (X_Coordinate2D *) node;
	MFVec2f_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Coordinate2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Coordinate2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Coordinate2D *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Coordinate2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Coordinate2D_Create()
{
	X_Coordinate2D *p;
	SAFEALLOC(p, sizeof(X_Coordinate2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Coordinate2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Coordinate2D";
	((SFNode *)p)->sgprivate->node_del = Coordinate2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Coordinate2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Coordinate2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	CoordinateInterpolator Node deletion
*/

static void CoordinateInterpolator_Del(SFNode *node)
{
	X_CoordinateInterpolator *p = (X_CoordinateInterpolator *) node;
	MFFloat_Del(p->key);
	MFVec3f_Del(p->keyValue);
	MFVec3f_Del(p->value_changed);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 CoordinateInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err CoordinateInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_CoordinateInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CoordinateInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_CoordinateInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_CoordinateInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_CoordinateInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_CoordinateInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *CoordinateInterpolator_Create()
{
	X_CoordinateInterpolator *p;
	SAFEALLOC(p, sizeof(X_CoordinateInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_CoordinateInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "CoordinateInterpolator";
	((SFNode *)p)->sgprivate->node_del = CoordinateInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = CoordinateInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = CoordinateInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	CoordinateInterpolator2D Node deletion
*/

static void CoordinateInterpolator2D_Del(SFNode *node)
{
	X_CoordinateInterpolator2D *p = (X_CoordinateInterpolator2D *) node;
	MFFloat_Del(p->key);
	MFVec2f_Del(p->keyValue);
	MFVec2f_Del(p->value_changed);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 CoordinateInterpolator2D_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err CoordinateInterpolator2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_CoordinateInterpolator2D *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CoordinateInterpolator2D *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_CoordinateInterpolator2D *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_CoordinateInterpolator2D *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_CoordinateInterpolator2D *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_CoordinateInterpolator2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *CoordinateInterpolator2D_Create()
{
	X_CoordinateInterpolator2D *p;
	SAFEALLOC(p, sizeof(X_CoordinateInterpolator2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_CoordinateInterpolator2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "CoordinateInterpolator2D";
	((SFNode *)p)->sgprivate->node_del = CoordinateInterpolator2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = CoordinateInterpolator2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = CoordinateInterpolator2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Cylinder Node deletion
*/

static void Cylinder_Del(SFNode *node)
{
	X_Cylinder *p = (X_Cylinder *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Cylinder_get_field_count(SFNode *node)
{
	return 6;
}

static M4Err Cylinder_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "bottom";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Cylinder *) node)->bottom;
		return M4OK;
	case 1:
		info->name = "height";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Cylinder *) node)->height;
		return M4OK;
	case 2:
		info->name = "radius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Cylinder *) node)->radius;
		return M4OK;
	case 3:
		info->name = "side";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Cylinder *) node)->side;
		return M4OK;
	case 4:
		info->name = "top";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Cylinder *) node)->top;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Cylinder *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Cylinder_Create()
{
	X_Cylinder *p;
	SAFEALLOC(p, sizeof(X_Cylinder));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Cylinder);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Cylinder";
	((SFNode *)p)->sgprivate->node_del = Cylinder_Del;
	((SFNode *)p)->sgprivate->get_field_count = Cylinder_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Cylinder_get_field;

#endif


	/*default field values*/
	p->bottom = 1;
	p->height = (SFFloat) 2;
	p->radius = (SFFloat) 1;
	p->side = 1;
	p->top = 1;
	return (SFNode *)p;
}


/*
	CylinderSensor Node deletion
*/

static void CylinderSensor_Del(SFNode *node)
{
	X_CylinderSensor *p = (X_CylinderSensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFString_Del(p->description);
	SFNode_Delete((SFNode *)p);
}


static u32 CylinderSensor_get_field_count(SFNode *node)
{
	return 12;
}

static M4Err CylinderSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "autoOffset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_CylinderSensor *) node)->autoOffset;
		return M4OK;
	case 1:
		info->name = "diskAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CylinderSensor *) node)->diskAngle;
		return M4OK;
	case 2:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_CylinderSensor *) node)->enabled;
		return M4OK;
	case 3:
		info->name = "maxAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CylinderSensor *) node)->maxAngle;
		return M4OK;
	case 4:
		info->name = "minAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CylinderSensor *) node)->minAngle;
		return M4OK;
	case 5:
		info->name = "offset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_CylinderSensor *) node)->offset;
		return M4OK;
	case 6:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_CylinderSensor *) node)->isActive;
		return M4OK;
	case 7:
		info->name = "rotation_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_CylinderSensor *) node)->rotation_changed;
		return M4OK;
	case 8:
		info->name = "trackPoint_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_CylinderSensor *) node)->trackPoint_changed;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_CylinderSensor *)node)->metadata;
		return M4OK;
	case 10:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_CylinderSensor *) node)->description;
		return M4OK;
	case 11:
		info->name = "isOver";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_CylinderSensor *) node)->isOver;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *CylinderSensor_Create()
{
	X_CylinderSensor *p;
	SAFEALLOC(p, sizeof(X_CylinderSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_CylinderSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "CylinderSensor";
	((SFNode *)p)->sgprivate->node_del = CylinderSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = CylinderSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = CylinderSensor_get_field;

#endif


	/*default field values*/
	p->autoOffset = 1;
	p->diskAngle = (SFFloat) 0.2617;
	p->enabled = 1;
	p->maxAngle = (SFFloat) -1;
	p->minAngle = (SFFloat) 0;
	p->offset = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	DirectionalLight Node deletion
*/

static void DirectionalLight_Del(SFNode *node)
{
	X_DirectionalLight *p = (X_DirectionalLight *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 DirectionalLight_get_field_count(SFNode *node)
{
	return 6;
}

static M4Err DirectionalLight_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "ambientIntensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_DirectionalLight *) node)->ambientIntensity;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_DirectionalLight *) node)->color;
		return M4OK;
	case 2:
		info->name = "direction";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_DirectionalLight *) node)->direction;
		return M4OK;
	case 3:
		info->name = "intensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_DirectionalLight *) node)->intensity;
		return M4OK;
	case 4:
		info->name = "on";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_DirectionalLight *) node)->on;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_DirectionalLight *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *DirectionalLight_Create()
{
	X_DirectionalLight *p;
	SAFEALLOC(p, sizeof(X_DirectionalLight));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_DirectionalLight);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "DirectionalLight";
	((SFNode *)p)->sgprivate->node_del = DirectionalLight_Del;
	((SFNode *)p)->sgprivate->get_field_count = DirectionalLight_get_field_count;
	((SFNode *)p)->sgprivate->get_field = DirectionalLight_get_field;

#endif


	/*default field values*/
	p->ambientIntensity = (SFFloat) 0;
	p->color.red = (SFFloat) 1;
	p->color.green = (SFFloat) 1;
	p->color.blue = (SFFloat) 1;
	p->direction.x = (SFFloat) 0;
	p->direction.y = (SFFloat) 0;
	p->direction.z = (SFFloat) -1;
	p->intensity = (SFFloat) 1;
	p->on = 1;
	return (SFNode *)p;
}


/*
	Disk2D Node deletion
*/

static void Disk2D_Del(SFNode *node)
{
	X_Disk2D *p = (X_Disk2D *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Disk2D_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err Disk2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "innerRadius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Disk2D *) node)->innerRadius;
		return M4OK;
	case 1:
		info->name = "outerRadius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Disk2D *) node)->outerRadius;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Disk2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Disk2D_Create()
{
	X_Disk2D *p;
	SAFEALLOC(p, sizeof(X_Disk2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Disk2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Disk2D";
	((SFNode *)p)->sgprivate->node_del = Disk2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Disk2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Disk2D_get_field;

#endif


	/*default field values*/
	p->innerRadius = (SFFloat) 0;
	p->outerRadius = (SFFloat) 1;
	return (SFNode *)p;
}


/*
	ElevationGrid Node deletion
*/

static void ElevationGrid_Del(SFNode *node)
{
	X_ElevationGrid *p = (X_ElevationGrid *) node;
	MFFloat_Del(p->set_height);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	MFFloat_Del(p->height);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ElevationGrid_get_field_count(SFNode *node)
{
	return 15;
}

static M4Err ElevationGrid_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_height";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_ElevationGrid *)node)->on_set_height;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_ElevationGrid *) node)->set_height;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_ElevationGrid *)node)->color;
		return M4OK;
	case 2:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_ElevationGrid *)node)->normal;
		return M4OK;
	case 3:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_ElevationGrid *)node)->texCoord;
		return M4OK;
	case 4:
		info->name = "height";
		info->eventType = ET_Field;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_ElevationGrid *) node)->height;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ElevationGrid *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ElevationGrid *) node)->colorPerVertex;
		return M4OK;
	case 7:
		info->name = "creaseAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ElevationGrid *) node)->creaseAngle;
		return M4OK;
	case 8:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ElevationGrid *) node)->normalPerVertex;
		return M4OK;
	case 9:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ElevationGrid *) node)->solid;
		return M4OK;
	case 10:
		info->name = "xDimension";
		info->eventType = ET_Field;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_ElevationGrid *) node)->xDimension;
		return M4OK;
	case 11:
		info->name = "xSpacing";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ElevationGrid *) node)->xSpacing;
		return M4OK;
	case 12:
		info->name = "zDimension";
		info->eventType = ET_Field;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_ElevationGrid *) node)->zDimension;
		return M4OK;
	case 13:
		info->name = "zSpacing";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ElevationGrid *) node)->zSpacing;
		return M4OK;
	case 14:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ElevationGrid *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ElevationGrid_Create()
{
	X_ElevationGrid *p;
	SAFEALLOC(p, sizeof(X_ElevationGrid));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ElevationGrid);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ElevationGrid";
	((SFNode *)p)->sgprivate->node_del = ElevationGrid_Del;
	((SFNode *)p)->sgprivate->get_field_count = ElevationGrid_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ElevationGrid_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->creaseAngle = (SFFloat) 0.0;
	p->normalPerVertex = 1;
	p->solid = 1;
	p->xDimension = 0;
	p->xSpacing = (SFFloat) 1.0;
	p->zDimension = 0;
	p->zSpacing = (SFFloat) 1.0;
	return (SFNode *)p;
}


/*
	Extrusion Node deletion
*/

static void Extrusion_Del(SFNode *node)
{
	X_Extrusion *p = (X_Extrusion *) node;
	MFVec2f_Del(p->set_crossSection);
	MFRotation_Del(p->set_orientation);
	MFVec2f_Del(p->set_scale);
	MFVec3f_Del(p->set_spine);
	MFVec2f_Del(p->crossSection);
	MFRotation_Del(p->orientation);
	MFVec2f_Del(p->scale);
	MFVec3f_Del(p->spine);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Extrusion_get_field_count(SFNode *node)
{
	return 15;
}

static M4Err Extrusion_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_crossSection";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Extrusion *)node)->on_set_crossSection;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Extrusion *) node)->set_crossSection;
		return M4OK;
	case 1:
		info->name = "set_orientation";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Extrusion *)node)->on_set_orientation;
		info->fieldType = FT_MFRotation;
		info->far_ptr = & ((X_Extrusion *) node)->set_orientation;
		return M4OK;
	case 2:
		info->name = "set_scale";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Extrusion *)node)->on_set_scale;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Extrusion *) node)->set_scale;
		return M4OK;
	case 3:
		info->name = "set_spine";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Extrusion *)node)->on_set_spine;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_Extrusion *) node)->set_spine;
		return M4OK;
	case 4:
		info->name = "beginCap";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Extrusion *) node)->beginCap;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Extrusion *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "convex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Extrusion *) node)->convex;
		return M4OK;
	case 7:
		info->name = "creaseAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Extrusion *) node)->creaseAngle;
		return M4OK;
	case 8:
		info->name = "crossSection";
		info->eventType = ET_Field;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Extrusion *) node)->crossSection;
		return M4OK;
	case 9:
		info->name = "endCap";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Extrusion *) node)->endCap;
		return M4OK;
	case 10:
		info->name = "orientation";
		info->eventType = ET_Field;
		info->fieldType = FT_MFRotation;
		info->far_ptr = & ((X_Extrusion *) node)->orientation;
		return M4OK;
	case 11:
		info->name = "scale";
		info->eventType = ET_Field;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Extrusion *) node)->scale;
		return M4OK;
	case 12:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Extrusion *) node)->solid;
		return M4OK;
	case 13:
		info->name = "spine";
		info->eventType = ET_Field;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_Extrusion *) node)->spine;
		return M4OK;
	case 14:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Extrusion *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Extrusion_Create()
{
	X_Extrusion *p;
	SAFEALLOC(p, sizeof(X_Extrusion));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Extrusion);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Extrusion";
	((SFNode *)p)->sgprivate->node_del = Extrusion_Del;
	((SFNode *)p)->sgprivate->get_field_count = Extrusion_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Extrusion_get_field;

#endif


	/*default field values*/
	p->beginCap = 1;
	p->ccw = 1;
	p->convex = 1;
	p->creaseAngle = (SFFloat) 0.0;
	p->crossSection.vals = malloc(sizeof(SFVec2f)*5);
	p->crossSection.count = 5;
	p->crossSection.vals[0].x = (SFFloat) 1;
	p->crossSection.vals[0].y = (SFFloat) 1;
	p->crossSection.vals[1].x = (SFFloat) 1;
	p->crossSection.vals[1].y = (SFFloat) -1;
	p->crossSection.vals[2].x = (SFFloat) -1;
	p->crossSection.vals[2].y = (SFFloat) -1;
	p->crossSection.vals[3].x = (SFFloat) -1;
	p->crossSection.vals[3].y = (SFFloat) 1;
	p->crossSection.vals[4].x = (SFFloat) 1;
	p->crossSection.vals[4].y = (SFFloat) 1;
	p->endCap = 1;
	p->orientation.vals = malloc(sizeof(SFRotation)*1);
	p->orientation.count = 1;
	p->orientation.vals[0].xAxis = (SFFloat) 0;
	p->orientation.vals[0].yAxis = (SFFloat) 0;
	p->orientation.vals[0].zAxis = (SFFloat) 1;
	p->orientation.vals[0].angle = (SFFloat) 0;
	p->scale.vals = malloc(sizeof(SFVec2f)*1);
	p->scale.count = 1;
	p->scale.vals[0].x = (SFFloat) 1;
	p->scale.vals[0].y = (SFFloat) 1;
	p->solid = 1;
	p->spine.vals = malloc(sizeof(SFVec3f)*2);
	p->spine.count = 2;
	p->spine.vals[0].x = (SFFloat) 0;
	p->spine.vals[0].y = (SFFloat) 0;
	p->spine.vals[0].z = (SFFloat) 0;
	p->spine.vals[1].x = (SFFloat) 0;
	p->spine.vals[1].y = (SFFloat) 1;
	p->spine.vals[1].z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	FillProperties Node deletion
*/

static void FillProperties_Del(SFNode *node)
{
	X_FillProperties *p = (X_FillProperties *) node;
	SFNode_Delete((SFNode *)p);
}


static u32 FillProperties_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err FillProperties_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "filled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_FillProperties *) node)->filled;
		return M4OK;
	case 1:
		info->name = "hatchColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_FillProperties *) node)->hatchColor;
		return M4OK;
	case 2:
		info->name = "hatched";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_FillProperties *) node)->hatched;
		return M4OK;
	case 3:
		info->name = "hatchStyle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_FillProperties *) node)->hatchStyle;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *FillProperties_Create()
{
	X_FillProperties *p;
	SAFEALLOC(p, sizeof(X_FillProperties));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_FillProperties);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "FillProperties";
	((SFNode *)p)->sgprivate->node_del = FillProperties_Del;
	((SFNode *)p)->sgprivate->get_field_count = FillProperties_get_field_count;
	((SFNode *)p)->sgprivate->get_field = FillProperties_get_field;

#endif


	/*default field values*/
	p->filled = 1;
	p->hatchColor.red = (SFFloat) 1;
	p->hatchColor.green = (SFFloat) 1;
	p->hatchColor.blue = (SFFloat) 1;
	p->hatched = 1;
	p->hatchStyle = 1;
	return (SFNode *)p;
}


/*
	Fog Node deletion
*/

static void Fog_Del(SFNode *node)
{
	X_Fog *p = (X_Fog *) node;
	SFString_Del(p->fogType);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Fog_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err Fog_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_Fog *) node)->color;
		return M4OK;
	case 1:
		info->name = "fogType";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_Fog *) node)->fogType;
		return M4OK;
	case 2:
		info->name = "visibilityRange";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Fog *) node)->visibilityRange;
		return M4OK;
	case 3:
		info->name = "set_bind";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Fog *)node)->on_set_bind;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Fog *) node)->set_bind;
		return M4OK;
	case 4:
		info->name = "isBound";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Fog *) node)->isBound;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Fog *)node)->metadata;
		return M4OK;
	case 6:
		info->name = "bindTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_Fog *) node)->bindTime;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Fog_Create()
{
	X_Fog *p;
	SAFEALLOC(p, sizeof(X_Fog));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Fog);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Fog";
	((SFNode *)p)->sgprivate->node_del = Fog_Del;
	((SFNode *)p)->sgprivate->get_field_count = Fog_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Fog_get_field;

#endif


	/*default field values*/
	p->color.red = (SFFloat) 1;
	p->color.green = (SFFloat) 1;
	p->color.blue = (SFFloat) 1;
	p->fogType.buffer = malloc(sizeof(char) * 7);
	strcpy(p->fogType.buffer, "LINEAR");
	p->visibilityRange = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	FontStyle Node deletion
*/

static void FontStyle_Del(SFNode *node)
{
	X_FontStyle *p = (X_FontStyle *) node;
	MFString_Del(p->family);
	MFString_Del(p->justify);
	SFString_Del(p->language);
	SFString_Del(p->style);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 FontStyle_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err FontStyle_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "family";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_FontStyle *) node)->family;
		return M4OK;
	case 1:
		info->name = "horizontal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_FontStyle *) node)->horizontal;
		return M4OK;
	case 2:
		info->name = "justify";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_FontStyle *) node)->justify;
		return M4OK;
	case 3:
		info->name = "language";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_FontStyle *) node)->language;
		return M4OK;
	case 4:
		info->name = "leftToRight";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_FontStyle *) node)->leftToRight;
		return M4OK;
	case 5:
		info->name = "size";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_FontStyle *) node)->size;
		return M4OK;
	case 6:
		info->name = "spacing";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_FontStyle *) node)->spacing;
		return M4OK;
	case 7:
		info->name = "style";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_FontStyle *) node)->style;
		return M4OK;
	case 8:
		info->name = "topToBottom";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_FontStyle *) node)->topToBottom;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_FontStyle *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *FontStyle_Create()
{
	X_FontStyle *p;
	SAFEALLOC(p, sizeof(X_FontStyle));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_FontStyle);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "FontStyle";
	((SFNode *)p)->sgprivate->node_del = FontStyle_Del;
	((SFNode *)p)->sgprivate->get_field_count = FontStyle_get_field_count;
	((SFNode *)p)->sgprivate->get_field = FontStyle_get_field;

#endif


	/*default field values*/
	p->family.vals = malloc(sizeof(SFString)*1);
	p->family.count = 1;
	p->family.vals[0] = malloc(sizeof(char) * 6);
	strcpy(p->family.vals[0], "SERIF");
	p->horizontal = 1;
	p->justify.vals = malloc(sizeof(SFString)*1);
	p->justify.count = 1;
	p->justify.vals[0] = malloc(sizeof(char) * 6);
	strcpy(p->justify.vals[0], "BEGIN");
	p->leftToRight = 1;
	p->size = (SFFloat) 1.0;
	p->spacing = (SFFloat) 1.0;
	p->style.buffer = malloc(sizeof(char) * 6);
	strcpy(p->style.buffer, "PLAIN");
	p->topToBottom = 1;
	return (SFNode *)p;
}


/*
	Group Node deletion
*/

static void Group_Del(SFNode *node)
{
	X_Group *p = (X_Group *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Group_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err Group_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Group *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Group *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Group *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Group *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Group *)node)->children;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Group *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Group_Create()
{
	X_Group *p;
	SAFEALLOC(p, sizeof(X_Group));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Group);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Group";
	((SFNode *)p)->sgprivate->node_del = Group_Del;
	((SFNode *)p)->sgprivate->get_field_count = Group_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Group_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	ImageTexture Node deletion
*/

static void ImageTexture_Del(SFNode *node)
{
	X_ImageTexture *p = (X_ImageTexture *) node;
	MFURL_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ImageTexture_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err ImageTexture_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_ImageTexture *) node)->url;
		return M4OK;
	case 1:
		info->name = "repeatS";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ImageTexture *) node)->repeatS;
		return M4OK;
	case 2:
		info->name = "repeatT";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ImageTexture *) node)->repeatT;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ImageTexture *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ImageTexture_Create()
{
	X_ImageTexture *p;
	SAFEALLOC(p, sizeof(X_ImageTexture));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ImageTexture);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ImageTexture";
	((SFNode *)p)->sgprivate->node_del = ImageTexture_Del;
	((SFNode *)p)->sgprivate->get_field_count = ImageTexture_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ImageTexture_get_field;

#endif


	/*default field values*/
	p->repeatS = 1;
	p->repeatT = 1;
	return (SFNode *)p;
}


/*
	IndexedFaceSet Node deletion
*/

static void IndexedFaceSet_Del(SFNode *node)
{
	X_IndexedFaceSet *p = (X_IndexedFaceSet *) node;
	MFInt32_Del(p->set_colorIndex);
	MFInt32_Del(p->set_coordIndex);
	MFInt32_Del(p->set_normalIndex);
	MFInt32_Del(p->set_texCoordIndex);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	MFInt32_Del(p->colorIndex);
	MFInt32_Del(p->coordIndex);
	MFInt32_Del(p->normalIndex);
	MFInt32_Del(p->texCoordIndex);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IndexedFaceSet_get_field_count(SFNode *node)
{
	return 19;
}

static M4Err IndexedFaceSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_colorIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedFaceSet *)node)->on_set_colorIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->set_colorIndex;
		return M4OK;
	case 1:
		info->name = "set_coordIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedFaceSet *)node)->on_set_coordIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->set_coordIndex;
		return M4OK;
	case 2:
		info->name = "set_normalIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedFaceSet *)node)->on_set_normalIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->set_normalIndex;
		return M4OK;
	case 3:
		info->name = "set_texCoordIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedFaceSet *)node)->on_set_texCoordIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->set_texCoordIndex;
		return M4OK;
	case 4:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_IndexedFaceSet *)node)->color;
		return M4OK;
	case 5:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_IndexedFaceSet *)node)->coord;
		return M4OK;
	case 6:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_IndexedFaceSet *)node)->normal;
		return M4OK;
	case 7:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_IndexedFaceSet *)node)->texCoord;
		return M4OK;
	case 8:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->ccw;
		return M4OK;
	case 9:
		info->name = "colorIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->colorIndex;
		return M4OK;
	case 10:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->colorPerVertex;
		return M4OK;
	case 11:
		info->name = "convex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->convex;
		return M4OK;
	case 12:
		info->name = "coordIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->coordIndex;
		return M4OK;
	case 13:
		info->name = "creaseAngle";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->creaseAngle;
		return M4OK;
	case 14:
		info->name = "normalIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->normalIndex;
		return M4OK;
	case 15:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->normalPerVertex;
		return M4OK;
	case 16:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->solid;
		return M4OK;
	case 17:
		info->name = "texCoordIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedFaceSet *) node)->texCoordIndex;
		return M4OK;
	case 18:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IndexedFaceSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IndexedFaceSet_Create()
{
	X_IndexedFaceSet *p;
	SAFEALLOC(p, sizeof(X_IndexedFaceSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IndexedFaceSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IndexedFaceSet";
	((SFNode *)p)->sgprivate->node_del = IndexedFaceSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = IndexedFaceSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IndexedFaceSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->convex = 1;
	p->creaseAngle = (SFFloat) 0.0;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	IndexedLineSet Node deletion
*/

static void IndexedLineSet_Del(SFNode *node)
{
	X_IndexedLineSet *p = (X_IndexedLineSet *) node;
	MFInt32_Del(p->set_colorIndex);
	MFInt32_Del(p->set_coordIndex);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	MFInt32_Del(p->colorIndex);
	MFInt32_Del(p->coordIndex);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IndexedLineSet_get_field_count(SFNode *node)
{
	return 8;
}

static M4Err IndexedLineSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_colorIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedLineSet *)node)->on_set_colorIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedLineSet *) node)->set_colorIndex;
		return M4OK;
	case 1:
		info->name = "set_coordIndex";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedLineSet *)node)->on_set_coordIndex;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedLineSet *) node)->set_coordIndex;
		return M4OK;
	case 2:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_IndexedLineSet *)node)->color;
		return M4OK;
	case 3:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_IndexedLineSet *)node)->coord;
		return M4OK;
	case 4:
		info->name = "colorIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedLineSet *) node)->colorIndex;
		return M4OK;
	case 5:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedLineSet *) node)->colorPerVertex;
		return M4OK;
	case 6:
		info->name = "coordIndex";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedLineSet *) node)->coordIndex;
		return M4OK;
	case 7:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IndexedLineSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IndexedLineSet_Create()
{
	X_IndexedLineSet *p;
	SAFEALLOC(p, sizeof(X_IndexedLineSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IndexedLineSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IndexedLineSet";
	((SFNode *)p)->sgprivate->node_del = IndexedLineSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = IndexedLineSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IndexedLineSet_get_field;

#endif


	/*default field values*/
	p->colorPerVertex = 1;
	return (SFNode *)p;
}


/*
	IndexedTriangleFanSet Node deletion
*/

static void IndexedTriangleFanSet_Del(SFNode *node)
{
	X_IndexedTriangleFanSet *p = (X_IndexedTriangleFanSet *) node;
	MFInt32_Del(p->set_index);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	MFInt32_Del(p->index);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IndexedTriangleFanSet_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err IndexedTriangleFanSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_index";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedTriangleFanSet *)node)->on_set_index;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->set_index;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_IndexedTriangleFanSet *)node)->color;
		return M4OK;
	case 2:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleFanSet *)node)->coord;
		return M4OK;
	case 3:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_IndexedTriangleFanSet *)node)->normal;
		return M4OK;
	case 4:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleFanSet *)node)->texCoord;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->colorPerVertex;
		return M4OK;
	case 7:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->normalPerVertex;
		return M4OK;
	case 8:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->solid;
		return M4OK;
	case 9:
		info->name = "index";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleFanSet *) node)->index;
		return M4OK;
	case 10:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IndexedTriangleFanSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IndexedTriangleFanSet_Create()
{
	X_IndexedTriangleFanSet *p;
	SAFEALLOC(p, sizeof(X_IndexedTriangleFanSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IndexedTriangleFanSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IndexedTriangleFanSet";
	((SFNode *)p)->sgprivate->node_del = IndexedTriangleFanSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = IndexedTriangleFanSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IndexedTriangleFanSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	IndexedTriangleSet Node deletion
*/

static void IndexedTriangleSet_Del(SFNode *node)
{
	X_IndexedTriangleSet *p = (X_IndexedTriangleSet *) node;
	MFInt32_Del(p->set_index);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	MFInt32_Del(p->index);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IndexedTriangleSet_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err IndexedTriangleSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_index";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedTriangleSet *)node)->on_set_index;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->set_index;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_IndexedTriangleSet *)node)->color;
		return M4OK;
	case 2:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleSet *)node)->coord;
		return M4OK;
	case 3:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_IndexedTriangleSet *)node)->normal;
		return M4OK;
	case 4:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleSet *)node)->texCoord;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->colorPerVertex;
		return M4OK;
	case 7:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->normalPerVertex;
		return M4OK;
	case 8:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->solid;
		return M4OK;
	case 9:
		info->name = "index";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleSet *) node)->index;
		return M4OK;
	case 10:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IndexedTriangleSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IndexedTriangleSet_Create()
{
	X_IndexedTriangleSet *p;
	SAFEALLOC(p, sizeof(X_IndexedTriangleSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IndexedTriangleSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IndexedTriangleSet";
	((SFNode *)p)->sgprivate->node_del = IndexedTriangleSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = IndexedTriangleSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IndexedTriangleSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	IndexedTriangleStripSet Node deletion
*/

static void IndexedTriangleStripSet_Del(SFNode *node)
{
	X_IndexedTriangleStripSet *p = (X_IndexedTriangleStripSet *) node;
	MFInt32_Del(p->set_index);
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	MFInt32_Del(p->index);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IndexedTriangleStripSet_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err IndexedTriangleStripSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_index";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IndexedTriangleStripSet *)node)->on_set_index;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->set_index;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_IndexedTriangleStripSet *)node)->color;
		return M4OK;
	case 2:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleStripSet *)node)->coord;
		return M4OK;
	case 3:
		info->name = "creaseAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->creaseAngle;
		return M4OK;
	case 4:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_IndexedTriangleStripSet *)node)->normal;
		return M4OK;
	case 5:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_IndexedTriangleStripSet *)node)->texCoord;
		return M4OK;
	case 6:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->ccw;
		return M4OK;
	case 7:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->normalPerVertex;
		return M4OK;
	case 8:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->solid;
		return M4OK;
	case 9:
		info->name = "index";
		info->eventType = ET_Field;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IndexedTriangleStripSet *) node)->index;
		return M4OK;
	case 10:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IndexedTriangleStripSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IndexedTriangleStripSet_Create()
{
	X_IndexedTriangleStripSet *p;
	SAFEALLOC(p, sizeof(X_IndexedTriangleStripSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IndexedTriangleStripSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IndexedTriangleStripSet";
	((SFNode *)p)->sgprivate->node_del = IndexedTriangleStripSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = IndexedTriangleStripSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IndexedTriangleStripSet_get_field;

#endif


	/*default field values*/
	p->creaseAngle = (SFFloat) 0;
	p->ccw = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	Inline Node deletion
*/

static void Inline_Del(SFNode *node)
{
	X_Inline *p = (X_Inline *) node;
	MFURL_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Inline_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err Inline_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_Inline *) node)->url;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Inline *)node)->metadata;
		return M4OK;
	case 2:
		info->name = "load";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Inline *) node)->load;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Inline_Create()
{
	X_Inline *p;
	SAFEALLOC(p, sizeof(X_Inline));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Inline);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Inline";
	((SFNode *)p)->sgprivate->node_del = Inline_Del;
	((SFNode *)p)->sgprivate->get_field_count = Inline_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Inline_get_field;

#endif


	/*default field values*/
	p->load = 1;
	return (SFNode *)p;
}


/*
	IntegerSequencer Node deletion
*/

static void IntegerSequencer_Del(SFNode *node)
{
	X_IntegerSequencer *p = (X_IntegerSequencer *) node;
	MFFloat_Del(p->key);
	MFInt32_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IntegerSequencer_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err IntegerSequencer_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "next";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IntegerSequencer *)node)->on_next;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IntegerSequencer *) node)->next;
		return M4OK;
	case 1:
		info->name = "previous";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IntegerSequencer *)node)->on_previous;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IntegerSequencer *) node)->previous;
		return M4OK;
	case 2:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IntegerSequencer *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_IntegerSequencer *) node)->set_fraction;
		return M4OK;
	case 3:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_IntegerSequencer *) node)->key;
		return M4OK;
	case 4:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_IntegerSequencer *) node)->keyValue;
		return M4OK;
	case 5:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_IntegerSequencer *) node)->value_changed;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IntegerSequencer *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IntegerSequencer_Create()
{
	X_IntegerSequencer *p;
	SAFEALLOC(p, sizeof(X_IntegerSequencer));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IntegerSequencer);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IntegerSequencer";
	((SFNode *)p)->sgprivate->node_del = IntegerSequencer_Del;
	((SFNode *)p)->sgprivate->get_field_count = IntegerSequencer_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IntegerSequencer_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	IntegerTrigger Node deletion
*/

static void IntegerTrigger_Del(SFNode *node)
{
	X_IntegerTrigger *p = (X_IntegerTrigger *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 IntegerTrigger_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err IntegerTrigger_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_boolean";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_IntegerTrigger *)node)->on_set_boolean;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_IntegerTrigger *) node)->set_boolean;
		return M4OK;
	case 1:
		info->name = "integerKey";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_IntegerTrigger *) node)->integerKey;
		return M4OK;
	case 2:
		info->name = "triggerValue";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_IntegerTrigger *) node)->triggerValue;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_IntegerTrigger *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *IntegerTrigger_Create()
{
	X_IntegerTrigger *p;
	SAFEALLOC(p, sizeof(X_IntegerTrigger));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_IntegerTrigger);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "IntegerTrigger";
	((SFNode *)p)->sgprivate->node_del = IntegerTrigger_Del;
	((SFNode *)p)->sgprivate->get_field_count = IntegerTrigger_get_field_count;
	((SFNode *)p)->sgprivate->get_field = IntegerTrigger_get_field;

#endif


	/*default field values*/
	p->integerKey = -1;
	return (SFNode *)p;
}


/*
	KeySensor Node deletion
*/

static void KeySensor_Del(SFNode *node)
{
	X_KeySensor *p = (X_KeySensor *) node;
	SFString_Del(p->keyPress);
	SFString_Del(p->keyRelease);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 KeySensor_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err KeySensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_KeySensor *) node)->enabled;
		return M4OK;
	case 1:
		info->name = "actionKeyPress";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_KeySensor *) node)->actionKeyPress;
		return M4OK;
	case 2:
		info->name = "actionKeyRelease";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_KeySensor *) node)->actionKeyRelease;
		return M4OK;
	case 3:
		info->name = "altKey";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_KeySensor *) node)->altKey;
		return M4OK;
	case 4:
		info->name = "controlKey";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_KeySensor *) node)->controlKey;
		return M4OK;
	case 5:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_KeySensor *) node)->isActive;
		return M4OK;
	case 6:
		info->name = "keyPress";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_KeySensor *) node)->keyPress;
		return M4OK;
	case 7:
		info->name = "keyRelease";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_KeySensor *) node)->keyRelease;
		return M4OK;
	case 8:
		info->name = "shiftKey";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_KeySensor *) node)->shiftKey;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_KeySensor *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *KeySensor_Create()
{
	X_KeySensor *p;
	SAFEALLOC(p, sizeof(X_KeySensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_KeySensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "KeySensor";
	((SFNode *)p)->sgprivate->node_del = KeySensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = KeySensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = KeySensor_get_field;

#endif


	/*default field values*/
	p->enabled = 1;
	return (SFNode *)p;
}


/*
	LineProperties Node deletion
*/

static void LineProperties_Del(SFNode *node)
{
	X_LineProperties *p = (X_LineProperties *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 LineProperties_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err LineProperties_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "applied";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_LineProperties *) node)->applied;
		return M4OK;
	case 1:
		info->name = "linetype";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_LineProperties *) node)->linetype;
		return M4OK;
	case 2:
		info->name = "linewidthScaleFactor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_LineProperties *) node)->linewidthScaleFactor;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_LineProperties *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *LineProperties_Create()
{
	X_LineProperties *p;
	SAFEALLOC(p, sizeof(X_LineProperties));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_LineProperties);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "LineProperties";
	((SFNode *)p)->sgprivate->node_del = LineProperties_Del;
	((SFNode *)p)->sgprivate->get_field_count = LineProperties_get_field_count;
	((SFNode *)p)->sgprivate->get_field = LineProperties_get_field;

#endif


	/*default field values*/
	p->applied = 1;
	p->linetype = 1;
	p->linewidthScaleFactor = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	LineSet Node deletion
*/

static void LineSet_Del(SFNode *node)
{
	X_LineSet *p = (X_LineSet *) node;
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	MFInt32_Del(p->vertexCount);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 LineSet_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err LineSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_LineSet *)node)->color;
		return M4OK;
	case 1:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_LineSet *)node)->coord;
		return M4OK;
	case 2:
		info->name = "vertexCount";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_LineSet *) node)->vertexCount;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_LineSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *LineSet_Create()
{
	X_LineSet *p;
	SAFEALLOC(p, sizeof(X_LineSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_LineSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "LineSet";
	((SFNode *)p)->sgprivate->node_del = LineSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = LineSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = LineSet_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	LOD Node deletion
*/

static void LOD_Del(SFNode *node)
{
	X_LOD *p = (X_LOD *) node;
	DestroyVRMLParent(node);	
	MFFloat_Del(p->range);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 LOD_get_field_count(SFNode *node)
{
	return 6;
}

static M4Err LOD_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_LOD *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_LOD *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_LOD *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_LOD *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_LOD *)node)->children;
		return M4OK;
	case 3:
		info->name = "center";
		info->eventType = ET_Field;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_LOD *) node)->center;
		return M4OK;
	case 4:
		info->name = "range";
		info->eventType = ET_Field;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_LOD *) node)->range;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_LOD *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *LOD_Create()
{
	X_LOD *p;
	SAFEALLOC(p, sizeof(X_LOD));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_LOD);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "LOD";
	((SFNode *)p)->sgprivate->node_del = LOD_Del;
	((SFNode *)p)->sgprivate->get_field_count = LOD_get_field_count;
	((SFNode *)p)->sgprivate->get_field = LOD_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	p->center.x = (SFFloat) 0;
	p->center.y = (SFFloat) 0;
	p->center.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	Material Node deletion
*/

static void Material_Del(SFNode *node)
{
	X_Material *p = (X_Material *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Material_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err Material_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "ambientIntensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Material *) node)->ambientIntensity;
		return M4OK;
	case 1:
		info->name = "diffuseColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_Material *) node)->diffuseColor;
		return M4OK;
	case 2:
		info->name = "emissiveColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_Material *) node)->emissiveColor;
		return M4OK;
	case 3:
		info->name = "shininess";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Material *) node)->shininess;
		return M4OK;
	case 4:
		info->name = "specularColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_Material *) node)->specularColor;
		return M4OK;
	case 5:
		info->name = "transparency";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Material *) node)->transparency;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Material *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Material_Create()
{
	X_Material *p;
	SAFEALLOC(p, sizeof(X_Material));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Material);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Material";
	((SFNode *)p)->sgprivate->node_del = Material_Del;
	((SFNode *)p)->sgprivate->get_field_count = Material_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Material_get_field;

#endif


	/*default field values*/
	p->ambientIntensity = (SFFloat) 0.2;
	p->diffuseColor.red = (SFFloat) 0.8;
	p->diffuseColor.green = (SFFloat) 0.8;
	p->diffuseColor.blue = (SFFloat) 0.8;
	p->emissiveColor.red = (SFFloat) 0;
	p->emissiveColor.green = (SFFloat) 0;
	p->emissiveColor.blue = (SFFloat) 0;
	p->shininess = (SFFloat) 0.2;
	p->specularColor.red = (SFFloat) 0;
	p->specularColor.green = (SFFloat) 0;
	p->specularColor.blue = (SFFloat) 0;
	p->transparency = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	MetadataDouble Node deletion
*/

static void MetadataDouble_Del(SFNode *node)
{
	X_MetadataDouble *p = (X_MetadataDouble *) node;
	SFString_Del(p->name);
	SFString_Del(p->reference);
	MFDouble_Del(p->value);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MetadataDouble_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err MetadataDouble_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "name";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataDouble *) node)->name;
		return M4OK;
	case 1:
		info->name = "reference";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataDouble *) node)->reference;
		return M4OK;
	case 2:
		info->name = "value";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFDouble;
		info->far_ptr = & ((X_MetadataDouble *) node)->value;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataDouble *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MetadataDouble_Create()
{
	X_MetadataDouble *p;
	SAFEALLOC(p, sizeof(X_MetadataDouble));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MetadataDouble);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MetadataDouble";
	((SFNode *)p)->sgprivate->node_del = MetadataDouble_Del;
	((SFNode *)p)->sgprivate->get_field_count = MetadataDouble_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MetadataDouble_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MetadataFloat Node deletion
*/

static void MetadataFloat_Del(SFNode *node)
{
	X_MetadataFloat *p = (X_MetadataFloat *) node;
	SFString_Del(p->name);
	SFString_Del(p->reference);
	MFFloat_Del(p->value);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MetadataFloat_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err MetadataFloat_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "name";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataFloat *) node)->name;
		return M4OK;
	case 1:
		info->name = "reference";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataFloat *) node)->reference;
		return M4OK;
	case 2:
		info->name = "value";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_MetadataFloat *) node)->value;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataFloat *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MetadataFloat_Create()
{
	X_MetadataFloat *p;
	SAFEALLOC(p, sizeof(X_MetadataFloat));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MetadataFloat);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MetadataFloat";
	((SFNode *)p)->sgprivate->node_del = MetadataFloat_Del;
	((SFNode *)p)->sgprivate->get_field_count = MetadataFloat_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MetadataFloat_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MetadataInteger Node deletion
*/

static void MetadataInteger_Del(SFNode *node)
{
	X_MetadataInteger *p = (X_MetadataInteger *) node;
	SFString_Del(p->name);
	SFString_Del(p->reference);
	MFInt32_Del(p->value);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MetadataInteger_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err MetadataInteger_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "name";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataInteger *) node)->name;
		return M4OK;
	case 1:
		info->name = "reference";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataInteger *) node)->reference;
		return M4OK;
	case 2:
		info->name = "value";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_MetadataInteger *) node)->value;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataInteger *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MetadataInteger_Create()
{
	X_MetadataInteger *p;
	SAFEALLOC(p, sizeof(X_MetadataInteger));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MetadataInteger);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MetadataInteger";
	((SFNode *)p)->sgprivate->node_del = MetadataInteger_Del;
	((SFNode *)p)->sgprivate->get_field_count = MetadataInteger_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MetadataInteger_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MetadataSet Node deletion
*/

static void MetadataSet_Del(SFNode *node)
{
	X_MetadataSet *p = (X_MetadataSet *) node;
	SFString_Del(p->name);
	SFString_Del(p->reference);
	NodeList_Delete((Chain *) p->value, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MetadataSet_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err MetadataSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "name";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataSet *) node)->name;
		return M4OK;
	case 1:
		info->name = "reference";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataSet *) node)->reference;
		return M4OK;
	case 2:
		info->name = "value";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataSet *)node)->value;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MetadataSet_Create()
{
	X_MetadataSet *p;
	SAFEALLOC(p, sizeof(X_MetadataSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MetadataSet);
	p->value = NewChain();	

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MetadataSet";
	((SFNode *)p)->sgprivate->node_del = MetadataSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = MetadataSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MetadataSet_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SFMetadataNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MetadataString Node deletion
*/

static void MetadataString_Del(SFNode *node)
{
	X_MetadataString *p = (X_MetadataString *) node;
	SFString_Del(p->name);
	SFString_Del(p->reference);
	MFString_Del(p->value);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MetadataString_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err MetadataString_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "name";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataString *) node)->name;
		return M4OK;
	case 1:
		info->name = "reference";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_MetadataString *) node)->reference;
		return M4OK;
	case 2:
		info->name = "value";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_MetadataString *) node)->value;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MetadataString *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MetadataString_Create()
{
	X_MetadataString *p;
	SAFEALLOC(p, sizeof(X_MetadataString));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MetadataString);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MetadataString";
	((SFNode *)p)->sgprivate->node_del = MetadataString_Del;
	((SFNode *)p)->sgprivate->get_field_count = MetadataString_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MetadataString_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MovieTexture Node deletion
*/

static void MovieTexture_Del(SFNode *node)
{
	X_MovieTexture *p = (X_MovieTexture *) node;
	MFURL_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MovieTexture_get_field_count(SFNode *node)
{
	return 14;
}

static M4Err MovieTexture_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "loop";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_MovieTexture *) node)->loop;
		return M4OK;
	case 1:
		info->name = "speed";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_MovieTexture *) node)->speed;
		return M4OK;
	case 2:
		info->name = "startTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->startTime;
		return M4OK;
	case 3:
		info->name = "stopTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->stopTime;
		return M4OK;
	case 4:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFURL;
		info->far_ptr = & ((X_MovieTexture *) node)->url;
		return M4OK;
	case 5:
		info->name = "repeatS";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_MovieTexture *) node)->repeatS;
		return M4OK;
	case 6:
		info->name = "repeatT";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_MovieTexture *) node)->repeatT;
		return M4OK;
	case 7:
		info->name = "duration_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->duration_changed;
		return M4OK;
	case 8:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_MovieTexture *) node)->isActive;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MovieTexture *)node)->metadata;
		return M4OK;
	case 10:
		info->name = "resumeTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->resumeTime;
		return M4OK;
	case 11:
		info->name = "pauseTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->pauseTime;
		return M4OK;
	case 12:
		info->name = "elapsedTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_MovieTexture *) node)->elapsedTime;
		return M4OK;
	case 13:
		info->name = "isPaused";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_MovieTexture *) node)->isPaused;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MovieTexture_Create()
{
	X_MovieTexture *p;
	SAFEALLOC(p, sizeof(X_MovieTexture));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MovieTexture);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MovieTexture";
	((SFNode *)p)->sgprivate->node_del = MovieTexture_Del;
	((SFNode *)p)->sgprivate->get_field_count = MovieTexture_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MovieTexture_get_field;

#endif


	/*default field values*/
	p->speed = (SFFloat) 1.0;
	p->startTime = 0;
	p->stopTime = 0;
	p->repeatS = 1;
	p->repeatT = 1;
	p->resumeTime = 0;
	p->pauseTime = 0;
	return (SFNode *)p;
}


/*
	MultiTexture Node deletion
*/

static void MultiTexture_Del(SFNode *node)
{
	X_MultiTexture *p = (X_MultiTexture *) node;
	MFString_Del(p->function);
	MFString_Del(p->mode);
	MFString_Del(p->source);
	NodeList_Delete((Chain *) p->texture, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MultiTexture_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err MultiTexture_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "alpha";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_MultiTexture *) node)->alpha;
		return M4OK;
	case 1:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_MultiTexture *) node)->color;
		return M4OK;
	case 2:
		info->name = "function";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_MultiTexture *) node)->function;
		return M4OK;
	case 3:
		info->name = "mode";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_MultiTexture *) node)->mode;
		return M4OK;
	case 4:
		info->name = "source";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_MultiTexture *) node)->source;
		return M4OK;
	case 5:
		info->name = "texture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_MultiTexture *)node)->texture;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MultiTexture *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MultiTexture_Create()
{
	X_MultiTexture *p;
	SAFEALLOC(p, sizeof(X_MultiTexture));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MultiTexture);
	p->texture = NewChain();	

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MultiTexture";
	((SFNode *)p)->sgprivate->node_del = MultiTexture_Del;
	((SFNode *)p)->sgprivate->get_field_count = MultiTexture_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MultiTexture_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SFTextureNode;

#endif


	/*default field values*/
	p->alpha = (SFFloat) 1;
	p->color.red = (SFFloat) 1;
	p->color.green = (SFFloat) 1;
	p->color.blue = (SFFloat) 1;
	return (SFNode *)p;
}


/*
	MultiTextureCoordinate Node deletion
*/

static void MultiTextureCoordinate_Del(SFNode *node)
{
	X_MultiTextureCoordinate *p = (X_MultiTextureCoordinate *) node;
	NodeList_Delete((Chain *) p->texCoord, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MultiTextureCoordinate_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err MultiTextureCoordinate_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_MultiTextureCoordinate *)node)->texCoord;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MultiTextureCoordinate *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MultiTextureCoordinate_Create()
{
	X_MultiTextureCoordinate *p;
	SAFEALLOC(p, sizeof(X_MultiTextureCoordinate));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MultiTextureCoordinate);
	p->texCoord = NewChain();	

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MultiTextureCoordinate";
	((SFNode *)p)->sgprivate->node_del = MultiTextureCoordinate_Del;
	((SFNode *)p)->sgprivate->get_field_count = MultiTextureCoordinate_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MultiTextureCoordinate_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SFTextureCoordinateNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	MultiTextureTransform Node deletion
*/

static void MultiTextureTransform_Del(SFNode *node)
{
	X_MultiTextureTransform *p = (X_MultiTextureTransform *) node;
	NodeList_Delete((Chain *) p->textureTransform, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 MultiTextureTransform_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err MultiTextureTransform_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "textureTransform";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SFTextureTransformNode;
		info->far_ptr = & ((X_MultiTextureTransform *)node)->textureTransform;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_MultiTextureTransform *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *MultiTextureTransform_Create()
{
	X_MultiTextureTransform *p;
	SAFEALLOC(p, sizeof(X_MultiTextureTransform));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_MultiTextureTransform);
	p->textureTransform = NewChain();	

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "MultiTextureTransform";
	((SFNode *)p)->sgprivate->node_del = MultiTextureTransform_Del;
	((SFNode *)p)->sgprivate->get_field_count = MultiTextureTransform_get_field_count;
	((SFNode *)p)->sgprivate->get_field = MultiTextureTransform_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SFTextureTransformNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	NavigationInfo Node deletion
*/

static void NavigationInfo_Del(SFNode *node)
{
	X_NavigationInfo *p = (X_NavigationInfo *) node;
	MFFloat_Del(p->avatarSize);
	MFString_Del(p->type);
	Node_Unregister((SFNode *) p->metadata, node);	
	MFString_Del(p->transitionType);
	SFNode_Delete((SFNode *)p);
}


static u32 NavigationInfo_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err NavigationInfo_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_bind";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_NavigationInfo *)node)->on_set_bind;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_NavigationInfo *) node)->set_bind;
		return M4OK;
	case 1:
		info->name = "avatarSize";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_NavigationInfo *) node)->avatarSize;
		return M4OK;
	case 2:
		info->name = "headlight";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_NavigationInfo *) node)->headlight;
		return M4OK;
	case 3:
		info->name = "speed";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_NavigationInfo *) node)->speed;
		return M4OK;
	case 4:
		info->name = "type";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_NavigationInfo *) node)->type;
		return M4OK;
	case 5:
		info->name = "visibilityLimit";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_NavigationInfo *) node)->visibilityLimit;
		return M4OK;
	case 6:
		info->name = "isBound";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_NavigationInfo *) node)->isBound;
		return M4OK;
	case 7:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_NavigationInfo *)node)->metadata;
		return M4OK;
	case 8:
		info->name = "transitionType";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_NavigationInfo *) node)->transitionType;
		return M4OK;
	case 9:
		info->name = "bindTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_NavigationInfo *) node)->bindTime;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *NavigationInfo_Create()
{
	X_NavigationInfo *p;
	SAFEALLOC(p, sizeof(X_NavigationInfo));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_NavigationInfo);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "NavigationInfo";
	((SFNode *)p)->sgprivate->node_del = NavigationInfo_Del;
	((SFNode *)p)->sgprivate->get_field_count = NavigationInfo_get_field_count;
	((SFNode *)p)->sgprivate->get_field = NavigationInfo_get_field;

#endif


	/*default field values*/
	p->avatarSize.vals = malloc(sizeof(SFFloat)*3);
	p->avatarSize.count = 3;
	p->avatarSize.vals[0] = (SFFloat) 0.25;
	p->avatarSize.vals[1] = (SFFloat) 1.6;
	p->avatarSize.vals[2] = (SFFloat) 0.75;
	p->headlight = 1;
	p->speed = (SFFloat) 1.0;
	p->type.vals = malloc(sizeof(SFString)*2);
	p->type.count = 2;
	p->type.vals[0] = malloc(sizeof(char) * 5);
	strcpy(p->type.vals[0], "WALK");
	p->type.vals[1] = malloc(sizeof(char) * 4);
	strcpy(p->type.vals[1], "ANY");
	p->visibilityLimit = (SFFloat) 0.0;
	p->transitionType.vals = malloc(sizeof(SFString)*2);
	p->transitionType.count = 2;
	p->transitionType.vals[0] = malloc(sizeof(char) * 5);
	strcpy(p->transitionType.vals[0], "WALK");
	p->transitionType.vals[1] = malloc(sizeof(char) * 4);
	strcpy(p->transitionType.vals[1], "ANY");
	return (SFNode *)p;
}


/*
	Normal Node deletion
*/

static void Normal_Del(SFNode *node)
{
	X_Normal *p = (X_Normal *) node;
	MFVec3f_Del(p->vector);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Normal_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Normal_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "vector";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_Normal *) node)->vector;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Normal *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Normal_Create()
{
	X_Normal *p;
	SAFEALLOC(p, sizeof(X_Normal));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Normal);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Normal";
	((SFNode *)p)->sgprivate->node_del = Normal_Del;
	((SFNode *)p)->sgprivate->get_field_count = Normal_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Normal_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	NormalInterpolator Node deletion
*/

static void NormalInterpolator_Del(SFNode *node)
{
	X_NormalInterpolator *p = (X_NormalInterpolator *) node;
	MFFloat_Del(p->key);
	MFVec3f_Del(p->keyValue);
	MFVec3f_Del(p->value_changed);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 NormalInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err NormalInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_NormalInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_NormalInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_NormalInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_NormalInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_NormalInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_NormalInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *NormalInterpolator_Create()
{
	X_NormalInterpolator *p;
	SAFEALLOC(p, sizeof(X_NormalInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_NormalInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "NormalInterpolator";
	((SFNode *)p)->sgprivate->node_del = NormalInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = NormalInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = NormalInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	OrientationInterpolator Node deletion
*/

static void OrientationInterpolator_Del(SFNode *node)
{
	X_OrientationInterpolator *p = (X_OrientationInterpolator *) node;
	MFFloat_Del(p->key);
	MFRotation_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 OrientationInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err OrientationInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_OrientationInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_OrientationInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_OrientationInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFRotation;
		info->far_ptr = & ((X_OrientationInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_OrientationInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_OrientationInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *OrientationInterpolator_Create()
{
	X_OrientationInterpolator *p;
	SAFEALLOC(p, sizeof(X_OrientationInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_OrientationInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "OrientationInterpolator";
	((SFNode *)p)->sgprivate->node_del = OrientationInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = OrientationInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = OrientationInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	PixelTexture Node deletion
*/

static void PixelTexture_Del(SFNode *node)
{
	X_PixelTexture *p = (X_PixelTexture *) node;
	SFImage_Del(p->image);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 PixelTexture_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err PixelTexture_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "image";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFImage;
		info->far_ptr = & ((X_PixelTexture *) node)->image;
		return M4OK;
	case 1:
		info->name = "repeatS";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PixelTexture *) node)->repeatS;
		return M4OK;
	case 2:
		info->name = "repeatT";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PixelTexture *) node)->repeatT;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PixelTexture *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PixelTexture_Create()
{
	X_PixelTexture *p;
	SAFEALLOC(p, sizeof(X_PixelTexture));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PixelTexture);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PixelTexture";
	((SFNode *)p)->sgprivate->node_del = PixelTexture_Del;
	((SFNode *)p)->sgprivate->get_field_count = PixelTexture_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PixelTexture_get_field;

#endif


	/*default field values*/
	p->repeatS = 1;
	p->repeatT = 1;
	return (SFNode *)p;
}


/*
	PlaneSensor Node deletion
*/

static void PlaneSensor_Del(SFNode *node)
{
	X_PlaneSensor *p = (X_PlaneSensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFString_Del(p->description);
	SFNode_Delete((SFNode *)p);
}


static u32 PlaneSensor_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err PlaneSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "autoOffset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PlaneSensor *) node)->autoOffset;
		return M4OK;
	case 1:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PlaneSensor *) node)->enabled;
		return M4OK;
	case 2:
		info->name = "maxPosition";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_PlaneSensor *) node)->maxPosition;
		return M4OK;
	case 3:
		info->name = "minPosition";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_PlaneSensor *) node)->minPosition;
		return M4OK;
	case 4:
		info->name = "offset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PlaneSensor *) node)->offset;
		return M4OK;
	case 5:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PlaneSensor *) node)->isActive;
		return M4OK;
	case 6:
		info->name = "trackPoint_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PlaneSensor *) node)->trackPoint_changed;
		return M4OK;
	case 7:
		info->name = "translation_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PlaneSensor *) node)->translation_changed;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PlaneSensor *)node)->metadata;
		return M4OK;
	case 9:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_PlaneSensor *) node)->description;
		return M4OK;
	case 10:
		info->name = "isOver";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PlaneSensor *) node)->isOver;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PlaneSensor_Create()
{
	X_PlaneSensor *p;
	SAFEALLOC(p, sizeof(X_PlaneSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PlaneSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PlaneSensor";
	((SFNode *)p)->sgprivate->node_del = PlaneSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = PlaneSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PlaneSensor_get_field;

#endif


	/*default field values*/
	p->autoOffset = 1;
	p->enabled = 1;
	p->maxPosition.x = (SFFloat) -1;
	p->maxPosition.y = (SFFloat) -1;
	p->minPosition.x = (SFFloat) 0;
	p->minPosition.y = (SFFloat) 0;
	p->offset.x = (SFFloat) 0;
	p->offset.y = (SFFloat) 0;
	p->offset.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	PointLight Node deletion
*/

static void PointLight_Del(SFNode *node)
{
	X_PointLight *p = (X_PointLight *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 PointLight_get_field_count(SFNode *node)
{
	return 8;
}

static M4Err PointLight_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "ambientIntensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_PointLight *) node)->ambientIntensity;
		return M4OK;
	case 1:
		info->name = "attenuation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PointLight *) node)->attenuation;
		return M4OK;
	case 2:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_PointLight *) node)->color;
		return M4OK;
	case 3:
		info->name = "intensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_PointLight *) node)->intensity;
		return M4OK;
	case 4:
		info->name = "location";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PointLight *) node)->location;
		return M4OK;
	case 5:
		info->name = "on";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_PointLight *) node)->on;
		return M4OK;
	case 6:
		info->name = "radius";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_PointLight *) node)->radius;
		return M4OK;
	case 7:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PointLight *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PointLight_Create()
{
	X_PointLight *p;
	SAFEALLOC(p, sizeof(X_PointLight));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PointLight);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PointLight";
	((SFNode *)p)->sgprivate->node_del = PointLight_Del;
	((SFNode *)p)->sgprivate->get_field_count = PointLight_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PointLight_get_field;

#endif


	/*default field values*/
	p->ambientIntensity = (SFFloat) 0;
	p->attenuation.x = (SFFloat) 1;
	p->attenuation.y = (SFFloat) 0;
	p->attenuation.z = (SFFloat) 0;
	p->color.red = (SFFloat) 1;
	p->color.green = (SFFloat) 1;
	p->color.blue = (SFFloat) 1;
	p->intensity = (SFFloat) 1;
	p->location.x = (SFFloat) 0;
	p->location.y = (SFFloat) 0;
	p->location.z = (SFFloat) 0;
	p->on = 1;
	p->radius = (SFFloat) 100;
	return (SFNode *)p;
}


/*
	PointSet Node deletion
*/

static void PointSet_Del(SFNode *node)
{
	X_PointSet *p = (X_PointSet *) node;
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 PointSet_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err PointSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_PointSet *)node)->color;
		return M4OK;
	case 1:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_PointSet *)node)->coord;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PointSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PointSet_Create()
{
	X_PointSet *p;
	SAFEALLOC(p, sizeof(X_PointSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PointSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PointSet";
	((SFNode *)p)->sgprivate->node_del = PointSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = PointSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PointSet_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Polyline2D Node deletion
*/

static void Polyline2D_Del(SFNode *node)
{
	X_Polyline2D *p = (X_Polyline2D *) node;
	MFVec2f_Del(p->lineSegments);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Polyline2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Polyline2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "lineSegments";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Polyline2D *) node)->lineSegments;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Polyline2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Polyline2D_Create()
{
	X_Polyline2D *p;
	SAFEALLOC(p, sizeof(X_Polyline2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Polyline2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Polyline2D";
	((SFNode *)p)->sgprivate->node_del = Polyline2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Polyline2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Polyline2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Polypoint2D Node deletion
*/

static void Polypoint2D_Del(SFNode *node)
{
	X_Polypoint2D *p = (X_Polypoint2D *) node;
	MFVec2f_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Polypoint2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Polypoint2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_Polypoint2D *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Polypoint2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Polypoint2D_Create()
{
	X_Polypoint2D *p;
	SAFEALLOC(p, sizeof(X_Polypoint2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Polypoint2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Polypoint2D";
	((SFNode *)p)->sgprivate->node_del = Polypoint2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Polypoint2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Polypoint2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	PositionInterpolator Node deletion
*/

static void PositionInterpolator_Del(SFNode *node)
{
	X_PositionInterpolator *p = (X_PositionInterpolator *) node;
	MFFloat_Del(p->key);
	MFVec3f_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 PositionInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err PositionInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_PositionInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_PositionInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_PositionInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec3f;
		info->far_ptr = & ((X_PositionInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_PositionInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PositionInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PositionInterpolator_Create()
{
	X_PositionInterpolator *p;
	SAFEALLOC(p, sizeof(X_PositionInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PositionInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PositionInterpolator";
	((SFNode *)p)->sgprivate->node_del = PositionInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = PositionInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PositionInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	PositionInterpolator2D Node deletion
*/

static void PositionInterpolator2D_Del(SFNode *node)
{
	X_PositionInterpolator2D *p = (X_PositionInterpolator2D *) node;
	MFFloat_Del(p->key);
	MFVec2f_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 PositionInterpolator2D_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err PositionInterpolator2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_PositionInterpolator2D *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_PositionInterpolator2D *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_PositionInterpolator2D *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_PositionInterpolator2D *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_PositionInterpolator2D *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_PositionInterpolator2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *PositionInterpolator2D_Create()
{
	X_PositionInterpolator2D *p;
	SAFEALLOC(p, sizeof(X_PositionInterpolator2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_PositionInterpolator2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "PositionInterpolator2D";
	((SFNode *)p)->sgprivate->node_del = PositionInterpolator2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = PositionInterpolator2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = PositionInterpolator2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	ProximitySensor Node deletion
*/

static void ProximitySensor_Del(SFNode *node)
{
	X_ProximitySensor *p = (X_ProximitySensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ProximitySensor_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err ProximitySensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "center";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_ProximitySensor *) node)->center;
		return M4OK;
	case 1:
		info->name = "size";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_ProximitySensor *) node)->size;
		return M4OK;
	case 2:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ProximitySensor *) node)->enabled;
		return M4OK;
	case 3:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_ProximitySensor *) node)->isActive;
		return M4OK;
	case 4:
		info->name = "position_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_ProximitySensor *) node)->position_changed;
		return M4OK;
	case 5:
		info->name = "orientation_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_ProximitySensor *) node)->orientation_changed;
		return M4OK;
	case 6:
		info->name = "enterTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_ProximitySensor *) node)->enterTime;
		return M4OK;
	case 7:
		info->name = "exitTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_ProximitySensor *) node)->exitTime;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ProximitySensor *)node)->metadata;
		return M4OK;
	case 9:
		info->name = "centerOfRotation_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_ProximitySensor *) node)->centerOfRotation_changed;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ProximitySensor_Create()
{
	X_ProximitySensor *p;
	SAFEALLOC(p, sizeof(X_ProximitySensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ProximitySensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ProximitySensor";
	((SFNode *)p)->sgprivate->node_del = ProximitySensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = ProximitySensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ProximitySensor_get_field;

#endif


	/*default field values*/
	p->center.x = (SFFloat) 0;
	p->center.y = (SFFloat) 0;
	p->center.z = (SFFloat) 0;
	p->size.x = (SFFloat) 0;
	p->size.y = (SFFloat) 0;
	p->size.z = (SFFloat) 0;
	p->enabled = 1;
	return (SFNode *)p;
}


/*
	Rectangle2D Node deletion
*/

static void Rectangle2D_Del(SFNode *node)
{
	X_Rectangle2D *p = (X_Rectangle2D *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Rectangle2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Rectangle2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "size";
		info->eventType = ET_Field;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_Rectangle2D *) node)->size;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Rectangle2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Rectangle2D_Create()
{
	X_Rectangle2D *p;
	SAFEALLOC(p, sizeof(X_Rectangle2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Rectangle2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Rectangle2D";
	((SFNode *)p)->sgprivate->node_del = Rectangle2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = Rectangle2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Rectangle2D_get_field;

#endif


	/*default field values*/
	p->size.x = (SFFloat) 2;
	p->size.y = (SFFloat) 2;
	return (SFNode *)p;
}


/*
	ScalarInterpolator Node deletion
*/

static void ScalarInterpolator_Del(SFNode *node)
{
	X_ScalarInterpolator *p = (X_ScalarInterpolator *) node;
	MFFloat_Del(p->key);
	MFFloat_Del(p->keyValue);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 ScalarInterpolator_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err ScalarInterpolator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_fraction";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_ScalarInterpolator *)node)->on_set_fraction;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ScalarInterpolator *) node)->set_fraction;
		return M4OK;
	case 1:
		info->name = "key";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_ScalarInterpolator *) node)->key;
		return M4OK;
	case 2:
		info->name = "keyValue";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_ScalarInterpolator *) node)->keyValue;
		return M4OK;
	case 3:
		info->name = "value_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_ScalarInterpolator *) node)->value_changed;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_ScalarInterpolator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *ScalarInterpolator_Create()
{
	X_ScalarInterpolator *p;
	SAFEALLOC(p, sizeof(X_ScalarInterpolator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_ScalarInterpolator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "ScalarInterpolator";
	((SFNode *)p)->sgprivate->node_del = ScalarInterpolator_Del;
	((SFNode *)p)->sgprivate->get_field_count = ScalarInterpolator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = ScalarInterpolator_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Script Node deletion
*/

static void Script_Del(SFNode *node)
{
	X_Script *p = (X_Script *) node;
	MFScript_Del(p->url);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Script_get_field_count(SFNode *node)
{
	return 4;
}

static M4Err Script_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "url";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFScript;
		info->far_ptr = & ((X_Script *) node)->url;
		return M4OK;
	case 1:
		info->name = "directOutput";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Script *) node)->directOutput;
		return M4OK;
	case 2:
		info->name = "mustEvaluate";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Script *) node)->mustEvaluate;
		return M4OK;
	case 3:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Script *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Script_Create()
{
	X_Script *p;
	SAFEALLOC(p, sizeof(X_Script));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Script);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Script";
	((SFNode *)p)->sgprivate->node_del = Script_Del;
	((SFNode *)p)->sgprivate->get_field_count = Script_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Script_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Shape Node deletion
*/

static void Shape_Del(SFNode *node)
{
	X_Shape *p = (X_Shape *) node;
	Node_Unregister((SFNode *) p->appearance, node);	
	Node_Unregister((SFNode *) p->geometry, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Shape_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err Shape_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "appearance";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFAppearanceNode;
		info->far_ptr = & ((X_Shape *)node)->appearance;
		return M4OK;
	case 1:
		info->name = "geometry";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFGeometryNode;
		info->far_ptr = & ((X_Shape *)node)->geometry;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Shape *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Shape_Create()
{
	X_Shape *p;
	SAFEALLOC(p, sizeof(X_Shape));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Shape);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Shape";
	((SFNode *)p)->sgprivate->node_del = Shape_Del;
	((SFNode *)p)->sgprivate->get_field_count = Shape_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Shape_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	Sound Node deletion
*/

static void Sound_Del(SFNode *node)
{
	X_Sound *p = (X_Sound *) node;
	Node_Unregister((SFNode *) p->source, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Sound_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err Sound_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "direction";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Sound *) node)->direction;
		return M4OK;
	case 1:
		info->name = "intensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->intensity;
		return M4OK;
	case 2:
		info->name = "location";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Sound *) node)->location;
		return M4OK;
	case 3:
		info->name = "maxBack";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->maxBack;
		return M4OK;
	case 4:
		info->name = "maxFront";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->maxFront;
		return M4OK;
	case 5:
		info->name = "minBack";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->minBack;
		return M4OK;
	case 6:
		info->name = "minFront";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->minFront;
		return M4OK;
	case 7:
		info->name = "priority";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sound *) node)->priority;
		return M4OK;
	case 8:
		info->name = "source";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFAudioNode;
		info->far_ptr = & ((X_Sound *)node)->source;
		return M4OK;
	case 9:
		info->name = "spatialize";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Sound *) node)->spatialize;
		return M4OK;
	case 10:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Sound *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Sound_Create()
{
	X_Sound *p;
	SAFEALLOC(p, sizeof(X_Sound));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Sound);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Sound";
	((SFNode *)p)->sgprivate->node_del = Sound_Del;
	((SFNode *)p)->sgprivate->get_field_count = Sound_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Sound_get_field;

#endif


	/*default field values*/
	p->direction.x = (SFFloat) 0;
	p->direction.y = (SFFloat) 0;
	p->direction.z = (SFFloat) 1;
	p->intensity = (SFFloat) 1;
	p->location.x = (SFFloat) 0;
	p->location.y = (SFFloat) 0;
	p->location.z = (SFFloat) 0;
	p->maxBack = (SFFloat) 10;
	p->maxFront = (SFFloat) 10;
	p->minBack = (SFFloat) 1;
	p->minFront = (SFFloat) 1;
	p->priority = (SFFloat) 0;
	p->spatialize = 1;
	return (SFNode *)p;
}


/*
	Sphere Node deletion
*/

static void Sphere_Del(SFNode *node)
{
	X_Sphere *p = (X_Sphere *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Sphere_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err Sphere_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "radius";
		info->eventType = ET_Field;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Sphere *) node)->radius;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Sphere *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Sphere_Create()
{
	X_Sphere *p;
	SAFEALLOC(p, sizeof(X_Sphere));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Sphere);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Sphere";
	((SFNode *)p)->sgprivate->node_del = Sphere_Del;
	((SFNode *)p)->sgprivate->get_field_count = Sphere_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Sphere_get_field;

#endif


	/*default field values*/
	p->radius = (SFFloat) 1;
	return (SFNode *)p;
}


/*
	SphereSensor Node deletion
*/

static void SphereSensor_Del(SFNode *node)
{
	X_SphereSensor *p = (X_SphereSensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFString_Del(p->description);
	SFNode_Delete((SFNode *)p);
}


static u32 SphereSensor_get_field_count(SFNode *node)
{
	return 9;
}

static M4Err SphereSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "autoOffset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_SphereSensor *) node)->autoOffset;
		return M4OK;
	case 1:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_SphereSensor *) node)->enabled;
		return M4OK;
	case 2:
		info->name = "offset";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_SphereSensor *) node)->offset;
		return M4OK;
	case 3:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_SphereSensor *) node)->isActive;
		return M4OK;
	case 4:
		info->name = "rotation_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_SphereSensor *) node)->rotation_changed;
		return M4OK;
	case 5:
		info->name = "trackPoint_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_SphereSensor *) node)->trackPoint_changed;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_SphereSensor *)node)->metadata;
		return M4OK;
	case 7:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_SphereSensor *) node)->description;
		return M4OK;
	case 8:
		info->name = "isOver";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_SphereSensor *) node)->isOver;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *SphereSensor_Create()
{
	X_SphereSensor *p;
	SAFEALLOC(p, sizeof(X_SphereSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_SphereSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "SphereSensor";
	((SFNode *)p)->sgprivate->node_del = SphereSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = SphereSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = SphereSensor_get_field;

#endif


	/*default field values*/
	p->autoOffset = 1;
	p->enabled = 1;
	p->offset.xAxis = (SFFloat) 0;
	p->offset.yAxis = (SFFloat) 1;
	p->offset.zAxis = (SFFloat) 0;
	p->offset.angle = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	SpotLight Node deletion
*/

static void SpotLight_Del(SFNode *node)
{
	X_SpotLight *p = (X_SpotLight *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 SpotLight_get_field_count(SFNode *node)
{
	return 11;
}

static M4Err SpotLight_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "ambientIntensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_SpotLight *) node)->ambientIntensity;
		return M4OK;
	case 1:
		info->name = "attenuation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_SpotLight *) node)->attenuation;
		return M4OK;
	case 2:
		info->name = "beamWidth";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_SpotLight *) node)->beamWidth;
		return M4OK;
	case 3:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFColor;
		info->far_ptr = & ((X_SpotLight *) node)->color;
		return M4OK;
	case 4:
		info->name = "cutOffAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_SpotLight *) node)->cutOffAngle;
		return M4OK;
	case 5:
		info->name = "direction";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_SpotLight *) node)->direction;
		return M4OK;
	case 6:
		info->name = "intensity";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_SpotLight *) node)->intensity;
		return M4OK;
	case 7:
		info->name = "location";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_SpotLight *) node)->location;
		return M4OK;
	case 8:
		info->name = "on";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_SpotLight *) node)->on;
		return M4OK;
	case 9:
		info->name = "radius";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_SpotLight *) node)->radius;
		return M4OK;
	case 10:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_SpotLight *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *SpotLight_Create()
{
	X_SpotLight *p;
	SAFEALLOC(p, sizeof(X_SpotLight));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_SpotLight);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "SpotLight";
	((SFNode *)p)->sgprivate->node_del = SpotLight_Del;
	((SFNode *)p)->sgprivate->get_field_count = SpotLight_get_field_count;
	((SFNode *)p)->sgprivate->get_field = SpotLight_get_field;

#endif


	/*default field values*/
	p->ambientIntensity = (SFFloat) 0;
	p->attenuation.x = (SFFloat) 1;
	p->attenuation.y = (SFFloat) 0;
	p->attenuation.z = (SFFloat) 0;
	p->beamWidth = (SFFloat) 1.570796;
	p->color.red = (SFFloat) 1;
	p->color.green = (SFFloat) 1;
	p->color.blue = (SFFloat) 1;
	p->cutOffAngle = (SFFloat) 0.785398;
	p->direction.x = (SFFloat) 0;
	p->direction.y = (SFFloat) 0;
	p->direction.z = (SFFloat) -1;
	p->intensity = (SFFloat) 1;
	p->location.x = (SFFloat) 0;
	p->location.y = (SFFloat) 0;
	p->location.z = (SFFloat) 0;
	p->on = 1;
	p->radius = (SFFloat) 100;
	return (SFNode *)p;
}


/*
	StaticGroup Node deletion
*/

static void StaticGroup_Del(SFNode *node)
{
	X_StaticGroup *p = (X_StaticGroup *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 StaticGroup_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err StaticGroup_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "children";
		info->eventType = ET_Field;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_StaticGroup *)node)->children;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_StaticGroup *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *StaticGroup_Create()
{
	X_StaticGroup *p;
	SAFEALLOC(p, sizeof(X_StaticGroup));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_StaticGroup);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "StaticGroup";
	((SFNode *)p)->sgprivate->node_del = StaticGroup_Del;
	((SFNode *)p)->sgprivate->get_field_count = StaticGroup_get_field_count;
	((SFNode *)p)->sgprivate->get_field = StaticGroup_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	StringSensor Node deletion
*/

static void StringSensor_Del(SFNode *node)
{
	X_StringSensor *p = (X_StringSensor *) node;
	SFString_Del(p->enteredText);
	SFString_Del(p->finalText);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 StringSensor_get_field_count(SFNode *node)
{
	return 6;
}

static M4Err StringSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "deletionAllowed";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_StringSensor *) node)->deletionAllowed;
		return M4OK;
	case 1:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_StringSensor *) node)->enabled;
		return M4OK;
	case 2:
		info->name = "enteredText";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_StringSensor *) node)->enteredText;
		return M4OK;
	case 3:
		info->name = "finalText";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_StringSensor *) node)->finalText;
		return M4OK;
	case 4:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_StringSensor *) node)->isActive;
		return M4OK;
	case 5:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_StringSensor *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *StringSensor_Create()
{
	X_StringSensor *p;
	SAFEALLOC(p, sizeof(X_StringSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_StringSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "StringSensor";
	((SFNode *)p)->sgprivate->node_del = StringSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = StringSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = StringSensor_get_field;

#endif


	/*default field values*/
	p->deletionAllowed = 1;
	p->enabled = 1;
	return (SFNode *)p;
}


/*
	Switch Node deletion
*/

static void Switch_Del(SFNode *node)
{
	X_Switch *p = (X_Switch *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Switch_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err Switch_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Switch *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Switch *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Switch *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Switch *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Switch *)node)->children;
		return M4OK;
	case 3:
		info->name = "whichChoice";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFInt32;
		info->far_ptr = & ((X_Switch *) node)->whichChoice;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Switch *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Switch_Create()
{
	X_Switch *p;
	SAFEALLOC(p, sizeof(X_Switch));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Switch);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Switch";
	((SFNode *)p)->sgprivate->node_del = Switch_Del;
	((SFNode *)p)->sgprivate->get_field_count = Switch_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Switch_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	p->whichChoice = -1;
	return (SFNode *)p;
}


/*
	Text Node deletion
*/

static void Text_Del(SFNode *node)
{
	X_Text *p = (X_Text *) node;
	MFString_Del(p->string);
	MFFloat_Del(p->length);
	Node_Unregister((SFNode *) p->fontStyle, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Text_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err Text_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "string";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_Text *) node)->string;
		return M4OK;
	case 1:
		info->name = "length";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_Text *) node)->length;
		return M4OK;
	case 2:
		info->name = "fontStyle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFFontStyleNode;
		info->far_ptr = & ((X_Text *)node)->fontStyle;
		return M4OK;
	case 3:
		info->name = "maxExtent";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Text *) node)->maxExtent;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Text *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Text_Create()
{
	X_Text *p;
	SAFEALLOC(p, sizeof(X_Text));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Text);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Text";
	((SFNode *)p)->sgprivate->node_del = Text_Del;
	((SFNode *)p)->sgprivate->get_field_count = Text_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Text_get_field;

#endif


	/*default field values*/
	p->maxExtent = (SFFloat) 0.0;
	return (SFNode *)p;
}


/*
	TextureBackground Node deletion
*/

static void TextureBackground_Del(SFNode *node)
{
	X_TextureBackground *p = (X_TextureBackground *) node;
	MFFloat_Del(p->groundAngle);
	MFColor_Del(p->groundColor);
	Node_Unregister((SFNode *) p->backTexture, node);	
	Node_Unregister((SFNode *) p->bottomTexture, node);	
	Node_Unregister((SFNode *) p->frontTexture, node);	
	Node_Unregister((SFNode *) p->leftTexture, node);	
	Node_Unregister((SFNode *) p->rightTexture, node);	
	Node_Unregister((SFNode *) p->topTexture, node);	
	MFFloat_Del(p->skyAngle);
	MFColor_Del(p->skyColor);
	MFFloat_Del(p->transparency);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TextureBackground_get_field_count(SFNode *node)
{
	return 15;
}

static M4Err TextureBackground_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_bind";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_TextureBackground *)node)->on_set_bind;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TextureBackground *) node)->set_bind;
		return M4OK;
	case 1:
		info->name = "groundAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_TextureBackground *) node)->groundAngle;
		return M4OK;
	case 2:
		info->name = "groundColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_TextureBackground *) node)->groundColor;
		return M4OK;
	case 3:
		info->name = "backTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->backTexture;
		return M4OK;
	case 4:
		info->name = "bottomTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->bottomTexture;
		return M4OK;
	case 5:
		info->name = "frontTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->frontTexture;
		return M4OK;
	case 6:
		info->name = "leftTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->leftTexture;
		return M4OK;
	case 7:
		info->name = "rightTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->rightTexture;
		return M4OK;
	case 8:
		info->name = "topTexture";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureNode;
		info->far_ptr = & ((X_TextureBackground *)node)->topTexture;
		return M4OK;
	case 9:
		info->name = "skyAngle";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_TextureBackground *) node)->skyAngle;
		return M4OK;
	case 10:
		info->name = "skyColor";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFColor;
		info->far_ptr = & ((X_TextureBackground *) node)->skyColor;
		return M4OK;
	case 11:
		info->name = "transparency";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_TextureBackground *) node)->transparency;
		return M4OK;
	case 12:
		info->name = "bindTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TextureBackground *) node)->bindTime;
		return M4OK;
	case 13:
		info->name = "isBound";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TextureBackground *) node)->isBound;
		return M4OK;
	case 14:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TextureBackground *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TextureBackground_Create()
{
	X_TextureBackground *p;
	SAFEALLOC(p, sizeof(X_TextureBackground));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TextureBackground);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TextureBackground";
	((SFNode *)p)->sgprivate->node_del = TextureBackground_Del;
	((SFNode *)p)->sgprivate->get_field_count = TextureBackground_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TextureBackground_get_field;

#endif


	/*default field values*/
	p->skyColor.vals = malloc(sizeof(SFColor)*1);
	p->skyColor.count = 1;
	p->skyColor.vals[0].red = (SFFloat) 0;
	p->skyColor.vals[0].green = (SFFloat) 0;
	p->skyColor.vals[0].blue = (SFFloat) 0;
	p->transparency.vals = malloc(sizeof(SFFloat)*1);
	p->transparency.count = 1;
	p->transparency.vals[0] = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	TextureCoordinate Node deletion
*/

static void TextureCoordinate_Del(SFNode *node)
{
	X_TextureCoordinate *p = (X_TextureCoordinate *) node;
	MFVec2f_Del(p->point);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TextureCoordinate_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err TextureCoordinate_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "point";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_TextureCoordinate *) node)->point;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TextureCoordinate *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TextureCoordinate_Create()
{
	X_TextureCoordinate *p;
	SAFEALLOC(p, sizeof(X_TextureCoordinate));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TextureCoordinate);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TextureCoordinate";
	((SFNode *)p)->sgprivate->node_del = TextureCoordinate_Del;
	((SFNode *)p)->sgprivate->get_field_count = TextureCoordinate_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TextureCoordinate_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	TextureCoordinateGenerator Node deletion
*/

static void TextureCoordinateGenerator_Del(SFNode *node)
{
	X_TextureCoordinateGenerator *p = (X_TextureCoordinateGenerator *) node;
	SFString_Del(p->mode);
	MFFloat_Del(p->parameter);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TextureCoordinateGenerator_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err TextureCoordinateGenerator_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "mode";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_TextureCoordinateGenerator *) node)->mode;
		return M4OK;
	case 1:
		info->name = "parameter";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFFloat;
		info->far_ptr = & ((X_TextureCoordinateGenerator *) node)->parameter;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TextureCoordinateGenerator *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TextureCoordinateGenerator_Create()
{
	X_TextureCoordinateGenerator *p;
	SAFEALLOC(p, sizeof(X_TextureCoordinateGenerator));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TextureCoordinateGenerator);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TextureCoordinateGenerator";
	((SFNode *)p)->sgprivate->node_del = TextureCoordinateGenerator_Del;
	((SFNode *)p)->sgprivate->get_field_count = TextureCoordinateGenerator_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TextureCoordinateGenerator_get_field;

#endif


	/*default field values*/
	p->mode.buffer = malloc(sizeof(char) * 7);
	strcpy(p->mode.buffer, "SPHERE");
	return (SFNode *)p;
}


/*
	TextureTransform Node deletion
*/

static void TextureTransform_Del(SFNode *node)
{
	X_TextureTransform *p = (X_TextureTransform *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TextureTransform_get_field_count(SFNode *node)
{
	return 5;
}

static M4Err TextureTransform_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "center";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_TextureTransform *) node)->center;
		return M4OK;
	case 1:
		info->name = "rotation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_TextureTransform *) node)->rotation;
		return M4OK;
	case 2:
		info->name = "scale";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_TextureTransform *) node)->scale;
		return M4OK;
	case 3:
		info->name = "translation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_TextureTransform *) node)->translation;
		return M4OK;
	case 4:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TextureTransform *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TextureTransform_Create()
{
	X_TextureTransform *p;
	SAFEALLOC(p, sizeof(X_TextureTransform));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TextureTransform);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TextureTransform";
	((SFNode *)p)->sgprivate->node_del = TextureTransform_Del;
	((SFNode *)p)->sgprivate->get_field_count = TextureTransform_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TextureTransform_get_field;

#endif


	/*default field values*/
	p->center.x = (SFFloat) 0;
	p->center.y = (SFFloat) 0;
	p->rotation = (SFFloat) 0;
	p->scale.x = (SFFloat) 1;
	p->scale.y = (SFFloat) 1;
	p->translation.x = (SFFloat) 0;
	p->translation.y = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	TimeSensor Node deletion
*/

static void TimeSensor_Del(SFNode *node)
{
	X_TimeSensor *p = (X_TimeSensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TimeSensor_get_field_count(SFNode *node)
{
	return 14;
}

static M4Err TimeSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "cycleInterval";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->cycleInterval;
		return M4OK;
	case 1:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TimeSensor *) node)->enabled;
		return M4OK;
	case 2:
		info->name = "loop";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TimeSensor *) node)->loop;
		return M4OK;
	case 3:
		info->name = "startTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->startTime;
		return M4OK;
	case 4:
		info->name = "stopTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->stopTime;
		return M4OK;
	case 5:
		info->name = "cycleTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->cycleTime;
		return M4OK;
	case 6:
		info->name = "fraction_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_TimeSensor *) node)->fraction_changed;
		return M4OK;
	case 7:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TimeSensor *) node)->isActive;
		return M4OK;
	case 8:
		info->name = "time";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->time;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TimeSensor *)node)->metadata;
		return M4OK;
	case 10:
		info->name = "pauseTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->pauseTime;
		return M4OK;
	case 11:
		info->name = "resumeTime";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->resumeTime;
		return M4OK;
	case 12:
		info->name = "elapsedTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeSensor *) node)->elapsedTime;
		return M4OK;
	case 13:
		info->name = "isPaused";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TimeSensor *) node)->isPaused;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TimeSensor_Create()
{
	X_TimeSensor *p;
	SAFEALLOC(p, sizeof(X_TimeSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TimeSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TimeSensor";
	((SFNode *)p)->sgprivate->node_del = TimeSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = TimeSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TimeSensor_get_field;

#endif


	/*default field values*/
	p->cycleInterval = 1;
	p->enabled = 1;
	p->startTime = 0;
	p->stopTime = 0;
	p->pauseTime = 0;
	p->resumeTime = 0;
	return (SFNode *)p;
}


/*
	TimeTrigger Node deletion
*/

static void TimeTrigger_Del(SFNode *node)
{
	X_TimeTrigger *p = (X_TimeTrigger *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TimeTrigger_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err TimeTrigger_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_boolean";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_TimeTrigger *)node)->on_set_boolean;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TimeTrigger *) node)->set_boolean;
		return M4OK;
	case 1:
		info->name = "triggerTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TimeTrigger *) node)->triggerTime;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TimeTrigger *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TimeTrigger_Create()
{
	X_TimeTrigger *p;
	SAFEALLOC(p, sizeof(X_TimeTrigger));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TimeTrigger);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TimeTrigger";
	((SFNode *)p)->sgprivate->node_del = TimeTrigger_Del;
	((SFNode *)p)->sgprivate->get_field_count = TimeTrigger_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TimeTrigger_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	TouchSensor Node deletion
*/

static void TouchSensor_Del(SFNode *node)
{
	X_TouchSensor *p = (X_TouchSensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFString_Del(p->description);
	SFNode_Delete((SFNode *)p);
}


static u32 TouchSensor_get_field_count(SFNode *node)
{
	return 9;
}

static M4Err TouchSensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TouchSensor *) node)->enabled;
		return M4OK;
	case 1:
		info->name = "hitNormal_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_TouchSensor *) node)->hitNormal_changed;
		return M4OK;
	case 2:
		info->name = "hitPoint_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_TouchSensor *) node)->hitPoint_changed;
		return M4OK;
	case 3:
		info->name = "hitTexCoord_changed";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFVec2f;
		info->far_ptr = & ((X_TouchSensor *) node)->hitTexCoord_changed;
		return M4OK;
	case 4:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TouchSensor *) node)->isActive;
		return M4OK;
	case 5:
		info->name = "isOver";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TouchSensor *) node)->isOver;
		return M4OK;
	case 6:
		info->name = "touchTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_TouchSensor *) node)->touchTime;
		return M4OK;
	case 7:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TouchSensor *)node)->metadata;
		return M4OK;
	case 8:
		info->name = "description";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_TouchSensor *) node)->description;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TouchSensor_Create()
{
	X_TouchSensor *p;
	SAFEALLOC(p, sizeof(X_TouchSensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TouchSensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TouchSensor";
	((SFNode *)p)->sgprivate->node_del = TouchSensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = TouchSensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TouchSensor_get_field;

#endif


	/*default field values*/
	p->enabled = 1;
	return (SFNode *)p;
}


/*
	Transform Node deletion
*/

static void Transform_Del(SFNode *node)
{
	X_Transform *p = (X_Transform *) node;
	DestroyVRMLParent(node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Transform_get_field_count(SFNode *node)
{
	return 9;
}

static M4Err Transform_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "addChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Transform *)node)->on_addChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Transform *)node)->addChildren;
		return M4OK;
	case 1:
		info->name = "removeChildren";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Transform *)node)->on_removeChildren;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Transform *)node)->removeChildren;
		return M4OK;
	case 2:
		info->name = "center";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Transform *) node)->center;
		return M4OK;
	case 3:
		info->name = "children";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFNode;
		info->NDTtype = NDT_SF3DNode;
		info->far_ptr = & ((X_Transform *)node)->children;
		return M4OK;
	case 4:
		info->name = "rotation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_Transform *) node)->rotation;
		return M4OK;
	case 5:
		info->name = "scale";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Transform *) node)->scale;
		return M4OK;
	case 6:
		info->name = "scaleOrientation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_Transform *) node)->scaleOrientation;
		return M4OK;
	case 7:
		info->name = "translation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Transform *) node)->translation;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Transform *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Transform_Create()
{
	X_Transform *p;
	SAFEALLOC(p, sizeof(X_Transform));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Transform);
	SetupVRMLParent((SFNode *) p);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Transform";
	((SFNode *)p)->sgprivate->node_del = Transform_Del;
	((SFNode *)p)->sgprivate->get_field_count = Transform_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Transform_get_field;
	((SFNode *)p)->sgprivate->child_ndt = NDT_SF3DNode;

#endif


	/*default field values*/
	p->center.x = (SFFloat) 0;
	p->center.y = (SFFloat) 0;
	p->center.z = (SFFloat) 0;
	p->rotation.xAxis = (SFFloat) 0;
	p->rotation.yAxis = (SFFloat) 0;
	p->rotation.zAxis = (SFFloat) 1;
	p->rotation.angle = (SFFloat) 0;
	p->scale.x = (SFFloat) 1;
	p->scale.y = (SFFloat) 1;
	p->scale.z = (SFFloat) 1;
	p->scaleOrientation.xAxis = (SFFloat) 0;
	p->scaleOrientation.yAxis = (SFFloat) 0;
	p->scaleOrientation.zAxis = (SFFloat) 1;
	p->scaleOrientation.angle = (SFFloat) 0;
	p->translation.x = (SFFloat) 0;
	p->translation.y = (SFFloat) 0;
	p->translation.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	TriangleFanSet Node deletion
*/

static void TriangleFanSet_Del(SFNode *node)
{
	X_TriangleFanSet *p = (X_TriangleFanSet *) node;
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	MFInt32_Del(p->fanCount);
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TriangleFanSet_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err TriangleFanSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_TriangleFanSet *)node)->color;
		return M4OK;
	case 1:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_TriangleFanSet *)node)->coord;
		return M4OK;
	case 2:
		info->name = "fanCount";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_TriangleFanSet *) node)->fanCount;
		return M4OK;
	case 3:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_TriangleFanSet *)node)->normal;
		return M4OK;
	case 4:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_TriangleFanSet *)node)->texCoord;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleFanSet *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleFanSet *) node)->colorPerVertex;
		return M4OK;
	case 7:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleFanSet *) node)->normalPerVertex;
		return M4OK;
	case 8:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleFanSet *) node)->solid;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TriangleFanSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TriangleFanSet_Create()
{
	X_TriangleFanSet *p;
	SAFEALLOC(p, sizeof(X_TriangleFanSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TriangleFanSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TriangleFanSet";
	((SFNode *)p)->sgprivate->node_del = TriangleFanSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = TriangleFanSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TriangleFanSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	TriangleSet Node deletion
*/

static void TriangleSet_Del(SFNode *node)
{
	X_TriangleSet *p = (X_TriangleSet *) node;
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	Node_Unregister((SFNode *) p->texCoord, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TriangleSet_get_field_count(SFNode *node)
{
	return 9;
}

static M4Err TriangleSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_TriangleSet *)node)->color;
		return M4OK;
	case 1:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_TriangleSet *)node)->coord;
		return M4OK;
	case 2:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_TriangleSet *)node)->normal;
		return M4OK;
	case 3:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_TriangleSet *)node)->texCoord;
		return M4OK;
	case 4:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleSet *) node)->ccw;
		return M4OK;
	case 5:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleSet *) node)->colorPerVertex;
		return M4OK;
	case 6:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleSet *) node)->normalPerVertex;
		return M4OK;
	case 7:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleSet *) node)->solid;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TriangleSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TriangleSet_Create()
{
	X_TriangleSet *p;
	SAFEALLOC(p, sizeof(X_TriangleSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TriangleSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TriangleSet";
	((SFNode *)p)->sgprivate->node_del = TriangleSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = TriangleSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TriangleSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	TriangleSet2D Node deletion
*/

static void TriangleSet2D_Del(SFNode *node)
{
	X_TriangleSet2D *p = (X_TriangleSet2D *) node;
	MFVec2f_Del(p->vertices);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TriangleSet2D_get_field_count(SFNode *node)
{
	return 2;
}

static M4Err TriangleSet2D_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "vertices";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFVec2f;
		info->far_ptr = & ((X_TriangleSet2D *) node)->vertices;
		return M4OK;
	case 1:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TriangleSet2D *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TriangleSet2D_Create()
{
	X_TriangleSet2D *p;
	SAFEALLOC(p, sizeof(X_TriangleSet2D));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TriangleSet2D);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TriangleSet2D";
	((SFNode *)p)->sgprivate->node_del = TriangleSet2D_Del;
	((SFNode *)p)->sgprivate->get_field_count = TriangleSet2D_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TriangleSet2D_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}


/*
	TriangleStripSet Node deletion
*/

static void TriangleStripSet_Del(SFNode *node)
{
	X_TriangleStripSet *p = (X_TriangleStripSet *) node;
	Node_Unregister((SFNode *) p->color, node);	
	Node_Unregister((SFNode *) p->coord, node);	
	Node_Unregister((SFNode *) p->normal, node);	
	MFInt32_Del(p->stripCount);
	Node_Unregister((SFNode *) p->texCoord, node);	
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 TriangleStripSet_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err TriangleStripSet_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "color";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFColorNode;
		info->far_ptr = & ((X_TriangleStripSet *)node)->color;
		return M4OK;
	case 1:
		info->name = "coord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFCoordinateNode;
		info->far_ptr = & ((X_TriangleStripSet *)node)->coord;
		return M4OK;
	case 2:
		info->name = "normal";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFNormalNode;
		info->far_ptr = & ((X_TriangleStripSet *)node)->normal;
		return M4OK;
	case 3:
		info->name = "stripCount";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_MFInt32;
		info->far_ptr = & ((X_TriangleStripSet *) node)->stripCount;
		return M4OK;
	case 4:
		info->name = "texCoord";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFTextureCoordinateNode;
		info->far_ptr = & ((X_TriangleStripSet *)node)->texCoord;
		return M4OK;
	case 5:
		info->name = "ccw";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleStripSet *) node)->ccw;
		return M4OK;
	case 6:
		info->name = "colorPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleStripSet *) node)->colorPerVertex;
		return M4OK;
	case 7:
		info->name = "normalPerVertex";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleStripSet *) node)->normalPerVertex;
		return M4OK;
	case 8:
		info->name = "solid";
		info->eventType = ET_Field;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_TriangleStripSet *) node)->solid;
		return M4OK;
	case 9:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_TriangleStripSet *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *TriangleStripSet_Create()
{
	X_TriangleStripSet *p;
	SAFEALLOC(p, sizeof(X_TriangleStripSet));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_TriangleStripSet);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "TriangleStripSet";
	((SFNode *)p)->sgprivate->node_del = TriangleStripSet_Del;
	((SFNode *)p)->sgprivate->get_field_count = TriangleStripSet_get_field_count;
	((SFNode *)p)->sgprivate->get_field = TriangleStripSet_get_field;

#endif


	/*default field values*/
	p->ccw = 1;
	p->colorPerVertex = 1;
	p->normalPerVertex = 1;
	p->solid = 1;
	return (SFNode *)p;
}


/*
	Viewpoint Node deletion
*/

static void Viewpoint_Del(SFNode *node)
{
	X_Viewpoint *p = (X_Viewpoint *) node;
	SFString_Del(p->description);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 Viewpoint_get_field_count(SFNode *node)
{
	return 10;
}

static M4Err Viewpoint_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "set_bind";
		info->eventType = ET_EventIn;
		info->on_event_in = ((X_Viewpoint *)node)->on_set_bind;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Viewpoint *) node)->set_bind;
		return M4OK;
	case 1:
		info->name = "fieldOfView";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFFloat;
		info->far_ptr = & ((X_Viewpoint *) node)->fieldOfView;
		return M4OK;
	case 2:
		info->name = "jump";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Viewpoint *) node)->jump;
		return M4OK;
	case 3:
		info->name = "orientation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFRotation;
		info->far_ptr = & ((X_Viewpoint *) node)->orientation;
		return M4OK;
	case 4:
		info->name = "position";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Viewpoint *) node)->position;
		return M4OK;
	case 5:
		info->name = "description";
		info->eventType = ET_Field;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_Viewpoint *) node)->description;
		return M4OK;
	case 6:
		info->name = "bindTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_Viewpoint *) node)->bindTime;
		return M4OK;
	case 7:
		info->name = "isBound";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_Viewpoint *) node)->isBound;
		return M4OK;
	case 8:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_Viewpoint *)node)->metadata;
		return M4OK;
	case 9:
		info->name = "centerOfRotation";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_Viewpoint *) node)->centerOfRotation;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *Viewpoint_Create()
{
	X_Viewpoint *p;
	SAFEALLOC(p, sizeof(X_Viewpoint));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_Viewpoint);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "Viewpoint";
	((SFNode *)p)->sgprivate->node_del = Viewpoint_Del;
	((SFNode *)p)->sgprivate->get_field_count = Viewpoint_get_field_count;
	((SFNode *)p)->sgprivate->get_field = Viewpoint_get_field;

#endif


	/*default field values*/
	p->fieldOfView = (SFFloat) 0.785398;
	p->jump = 1;
	p->orientation.xAxis = (SFFloat) 0;
	p->orientation.yAxis = (SFFloat) 0;
	p->orientation.zAxis = (SFFloat) 1;
	p->orientation.angle = (SFFloat) 0;
	p->position.x = (SFFloat) 0;
	p->position.y = (SFFloat) 0;
	p->position.z = (SFFloat) 10;
	p->centerOfRotation.x = (SFFloat) 0;
	p->centerOfRotation.y = (SFFloat) 0;
	p->centerOfRotation.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	VisibilitySensor Node deletion
*/

static void VisibilitySensor_Del(SFNode *node)
{
	X_VisibilitySensor *p = (X_VisibilitySensor *) node;
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 VisibilitySensor_get_field_count(SFNode *node)
{
	return 7;
}

static M4Err VisibilitySensor_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "center";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_VisibilitySensor *) node)->center;
		return M4OK;
	case 1:
		info->name = "enabled";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_VisibilitySensor *) node)->enabled;
		return M4OK;
	case 2:
		info->name = "size";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFVec3f;
		info->far_ptr = & ((X_VisibilitySensor *) node)->size;
		return M4OK;
	case 3:
		info->name = "enterTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_VisibilitySensor *) node)->enterTime;
		return M4OK;
	case 4:
		info->name = "exitTime";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFTime;
		info->far_ptr = & ((X_VisibilitySensor *) node)->exitTime;
		return M4OK;
	case 5:
		info->name = "isActive";
		info->eventType = ET_EventOut;
		info->fieldType = FT_SFBool;
		info->far_ptr = & ((X_VisibilitySensor *) node)->isActive;
		return M4OK;
	case 6:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_VisibilitySensor *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *VisibilitySensor_Create()
{
	X_VisibilitySensor *p;
	SAFEALLOC(p, sizeof(X_VisibilitySensor));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_VisibilitySensor);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "VisibilitySensor";
	((SFNode *)p)->sgprivate->node_del = VisibilitySensor_Del;
	((SFNode *)p)->sgprivate->get_field_count = VisibilitySensor_get_field_count;
	((SFNode *)p)->sgprivate->get_field = VisibilitySensor_get_field;

#endif


	/*default field values*/
	p->center.x = (SFFloat) 0;
	p->center.y = (SFFloat) 0;
	p->center.z = (SFFloat) 0;
	p->enabled = 1;
	p->size.x = (SFFloat) 0;
	p->size.y = (SFFloat) 0;
	p->size.z = (SFFloat) 0;
	return (SFNode *)p;
}


/*
	WorldInfo Node deletion
*/

static void WorldInfo_Del(SFNode *node)
{
	X_WorldInfo *p = (X_WorldInfo *) node;
	MFString_Del(p->info);
	SFString_Del(p->title);
	Node_Unregister((SFNode *) p->metadata, node);	
	SFNode_Delete((SFNode *)p);
}


static u32 WorldInfo_get_field_count(SFNode *node)
{
	return 3;
}

static M4Err WorldInfo_get_field(SFNode *node, FieldInfo *info)
{
	switch (info->fieldIndex) {
	case 0:
		info->name = "info";
		info->eventType = ET_Field;
		info->fieldType = FT_MFString;
		info->far_ptr = & ((X_WorldInfo *) node)->info;
		return M4OK;
	case 1:
		info->name = "title";
		info->eventType = ET_Field;
		info->fieldType = FT_SFString;
		info->far_ptr = & ((X_WorldInfo *) node)->title;
		return M4OK;
	case 2:
		info->name = "metadata";
		info->eventType = ET_ExposedField;
		info->fieldType = FT_SFNode;
		info->NDTtype = NDT_SFMetadataNode;
		info->far_ptr = & ((X_WorldInfo *)node)->metadata;
		return M4OK;
	default:
		return M4BadParam;
	}
}



static SFNode *WorldInfo_Create()
{
	X_WorldInfo *p;
	SAFEALLOC(p, sizeof(X_WorldInfo));
	if(!p) return NULL;
	Node_Setup((SFNode *)p, TAG_X3D_WorldInfo);

#ifdef NODE_USE_POINTERS

	((SFNode *)p)->sgprivate->name = "WorldInfo";
	((SFNode *)p)->sgprivate->node_del = WorldInfo_Del;
	((SFNode *)p)->sgprivate->get_field_count = WorldInfo_get_field_count;
	((SFNode *)p)->sgprivate->get_field = WorldInfo_get_field;

#endif


	/*default field values*/
	return (SFNode *)p;
}




SFNode *X3D_CreateNode(u32 NodeTag)
{
	switch (NodeTag) {
	case TAG_X3D_Anchor:
		return Anchor_Create();
	case TAG_X3D_Appearance:
		return Appearance_Create();
	case TAG_X3D_Arc2D:
		return Arc2D_Create();
	case TAG_X3D_ArcClose2D:
		return ArcClose2D_Create();
	case TAG_X3D_AudioClip:
		return AudioClip_Create();
	case TAG_X3D_Background:
		return Background_Create();
	case TAG_X3D_Billboard:
		return Billboard_Create();
	case TAG_X3D_BooleanFilter:
		return BooleanFilter_Create();
	case TAG_X3D_BooleanSequencer:
		return BooleanSequencer_Create();
	case TAG_X3D_BooleanToggle:
		return BooleanToggle_Create();
	case TAG_X3D_BooleanTrigger:
		return BooleanTrigger_Create();
	case TAG_X3D_Box:
		return Box_Create();
	case TAG_X3D_Circle2D:
		return Circle2D_Create();
	case TAG_X3D_Collision:
		return Collision_Create();
	case TAG_X3D_Color:
		return Color_Create();
	case TAG_X3D_ColorInterpolator:
		return ColorInterpolator_Create();
	case TAG_X3D_ColorRGBA:
		return ColorRGBA_Create();
	case TAG_X3D_Cone:
		return Cone_Create();
	case TAG_X3D_Contour2D:
		return Contour2D_Create();
	case TAG_X3D_ContourPolyline2D:
		return ContourPolyline2D_Create();
	case TAG_X3D_Coordinate:
		return Coordinate_Create();
	case TAG_X3D_CoordinateDouble:
		return CoordinateDouble_Create();
	case TAG_X3D_Coordinate2D:
		return Coordinate2D_Create();
	case TAG_X3D_CoordinateInterpolator:
		return CoordinateInterpolator_Create();
	case TAG_X3D_CoordinateInterpolator2D:
		return CoordinateInterpolator2D_Create();
	case TAG_X3D_Cylinder:
		return Cylinder_Create();
	case TAG_X3D_CylinderSensor:
		return CylinderSensor_Create();
	case TAG_X3D_DirectionalLight:
		return DirectionalLight_Create();
	case TAG_X3D_Disk2D:
		return Disk2D_Create();
	case TAG_X3D_ElevationGrid:
		return ElevationGrid_Create();
	case TAG_X3D_Extrusion:
		return Extrusion_Create();
	case TAG_X3D_FillProperties:
		return FillProperties_Create();
	case TAG_X3D_Fog:
		return Fog_Create();
	case TAG_X3D_FontStyle:
		return FontStyle_Create();
	case TAG_X3D_Group:
		return Group_Create();
	case TAG_X3D_ImageTexture:
		return ImageTexture_Create();
	case TAG_X3D_IndexedFaceSet:
		return IndexedFaceSet_Create();
	case TAG_X3D_IndexedLineSet:
		return IndexedLineSet_Create();
	case TAG_X3D_IndexedTriangleFanSet:
		return IndexedTriangleFanSet_Create();
	case TAG_X3D_IndexedTriangleSet:
		return IndexedTriangleSet_Create();
	case TAG_X3D_IndexedTriangleStripSet:
		return IndexedTriangleStripSet_Create();
	case TAG_X3D_Inline:
		return Inline_Create();
	case TAG_X3D_IntegerSequencer:
		return IntegerSequencer_Create();
	case TAG_X3D_IntegerTrigger:
		return IntegerTrigger_Create();
	case TAG_X3D_KeySensor:
		return KeySensor_Create();
	case TAG_X3D_LineProperties:
		return LineProperties_Create();
	case TAG_X3D_LineSet:
		return LineSet_Create();
	case TAG_X3D_LOD:
		return LOD_Create();
	case TAG_X3D_Material:
		return Material_Create();
	case TAG_X3D_MetadataDouble:
		return MetadataDouble_Create();
	case TAG_X3D_MetadataFloat:
		return MetadataFloat_Create();
	case TAG_X3D_MetadataInteger:
		return MetadataInteger_Create();
	case TAG_X3D_MetadataSet:
		return MetadataSet_Create();
	case TAG_X3D_MetadataString:
		return MetadataString_Create();
	case TAG_X3D_MovieTexture:
		return MovieTexture_Create();
	case TAG_X3D_MultiTexture:
		return MultiTexture_Create();
	case TAG_X3D_MultiTextureCoordinate:
		return MultiTextureCoordinate_Create();
	case TAG_X3D_MultiTextureTransform:
		return MultiTextureTransform_Create();
	case TAG_X3D_NavigationInfo:
		return NavigationInfo_Create();
	case TAG_X3D_Normal:
		return Normal_Create();
	case TAG_X3D_NormalInterpolator:
		return NormalInterpolator_Create();
	case TAG_X3D_OrientationInterpolator:
		return OrientationInterpolator_Create();
	case TAG_X3D_PixelTexture:
		return PixelTexture_Create();
	case TAG_X3D_PlaneSensor:
		return PlaneSensor_Create();
	case TAG_X3D_PointLight:
		return PointLight_Create();
	case TAG_X3D_PointSet:
		return PointSet_Create();
	case TAG_X3D_Polyline2D:
		return Polyline2D_Create();
	case TAG_X3D_Polypoint2D:
		return Polypoint2D_Create();
	case TAG_X3D_PositionInterpolator:
		return PositionInterpolator_Create();
	case TAG_X3D_PositionInterpolator2D:
		return PositionInterpolator2D_Create();
	case TAG_X3D_ProximitySensor:
		return ProximitySensor_Create();
	case TAG_X3D_Rectangle2D:
		return Rectangle2D_Create();
	case TAG_X3D_ScalarInterpolator:
		return ScalarInterpolator_Create();
	case TAG_X3D_Script:
		return Script_Create();
	case TAG_X3D_Shape:
		return Shape_Create();
	case TAG_X3D_Sound:
		return Sound_Create();
	case TAG_X3D_Sphere:
		return Sphere_Create();
	case TAG_X3D_SphereSensor:
		return SphereSensor_Create();
	case TAG_X3D_SpotLight:
		return SpotLight_Create();
	case TAG_X3D_StaticGroup:
		return StaticGroup_Create();
	case TAG_X3D_StringSensor:
		return StringSensor_Create();
	case TAG_X3D_Switch:
		return Switch_Create();
	case TAG_X3D_Text:
		return Text_Create();
	case TAG_X3D_TextureBackground:
		return TextureBackground_Create();
	case TAG_X3D_TextureCoordinate:
		return TextureCoordinate_Create();
	case TAG_X3D_TextureCoordinateGenerator:
		return TextureCoordinateGenerator_Create();
	case TAG_X3D_TextureTransform:
		return TextureTransform_Create();
	case TAG_X3D_TimeSensor:
		return TimeSensor_Create();
	case TAG_X3D_TimeTrigger:
		return TimeTrigger_Create();
	case TAG_X3D_TouchSensor:
		return TouchSensor_Create();
	case TAG_X3D_Transform:
		return Transform_Create();
	case TAG_X3D_TriangleFanSet:
		return TriangleFanSet_Create();
	case TAG_X3D_TriangleSet:
		return TriangleSet_Create();
	case TAG_X3D_TriangleSet2D:
		return TriangleSet2D_Create();
	case TAG_X3D_TriangleStripSet:
		return TriangleStripSet_Create();
	case TAG_X3D_Viewpoint:
		return Viewpoint_Create();
	case TAG_X3D_VisibilitySensor:
		return VisibilitySensor_Create();
	case TAG_X3D_WorldInfo:
		return WorldInfo_Create();
	default:
		return NULL;
	}
}

#ifndef NODE_USE_POINTERS
const char *X3D_GetNodeName(u32 NodeTag)
{
	switch (NodeTag) {
	case TAG_X3D_Anchor:
		return "Anchor";
	case TAG_X3D_Appearance:
		return "Appearance";
	case TAG_X3D_Arc2D:
		return "Arc2D";
	case TAG_X3D_ArcClose2D:
		return "ArcClose2D";
	case TAG_X3D_AudioClip:
		return "AudioClip";
	case TAG_X3D_Background:
		return "Background";
	case TAG_X3D_Billboard:
		return "Billboard";
	case TAG_X3D_BooleanFilter:
		return "BooleanFilter";
	case TAG_X3D_BooleanSequencer:
		return "BooleanSequencer";
	case TAG_X3D_BooleanToggle:
		return "BooleanToggle";
	case TAG_X3D_BooleanTrigger:
		return "BooleanTrigger";
	case TAG_X3D_Box:
		return "Box";
	case TAG_X3D_Circle2D:
		return "Circle2D";
	case TAG_X3D_Collision:
		return "Collision";
	case TAG_X3D_Color:
		return "Color";
	case TAG_X3D_ColorInterpolator:
		return "ColorInterpolator";
	case TAG_X3D_ColorRGBA:
		return "ColorRGBA";
	case TAG_X3D_Cone:
		return "Cone";
	case TAG_X3D_Contour2D:
		return "Contour2D";
	case TAG_X3D_ContourPolyline2D:
		return "ContourPolyline2D";
	case TAG_X3D_Coordinate:
		return "Coordinate";
	case TAG_X3D_CoordinateDouble:
		return "CoordinateDouble";
	case TAG_X3D_Coordinate2D:
		return "Coordinate2D";
	case TAG_X3D_CoordinateInterpolator:
		return "CoordinateInterpolator";
	case TAG_X3D_CoordinateInterpolator2D:
		return "CoordinateInterpolator2D";
	case TAG_X3D_Cylinder:
		return "Cylinder";
	case TAG_X3D_CylinderSensor:
		return "CylinderSensor";
	case TAG_X3D_DirectionalLight:
		return "DirectionalLight";
	case TAG_X3D_Disk2D:
		return "Disk2D";
	case TAG_X3D_ElevationGrid:
		return "ElevationGrid";
	case TAG_X3D_Extrusion:
		return "Extrusion";
	case TAG_X3D_FillProperties:
		return "FillProperties";
	case TAG_X3D_Fog:
		return "Fog";
	case TAG_X3D_FontStyle:
		return "FontStyle";
	case TAG_X3D_Group:
		return "Group";
	case TAG_X3D_ImageTexture:
		return "ImageTexture";
	case TAG_X3D_IndexedFaceSet:
		return "IndexedFaceSet";
	case TAG_X3D_IndexedLineSet:
		return "IndexedLineSet";
	case TAG_X3D_IndexedTriangleFanSet:
		return "IndexedTriangleFanSet";
	case TAG_X3D_IndexedTriangleSet:
		return "IndexedTriangleSet";
	case TAG_X3D_IndexedTriangleStripSet:
		return "IndexedTriangleStripSet";
	case TAG_X3D_Inline:
		return "Inline";
	case TAG_X3D_IntegerSequencer:
		return "IntegerSequencer";
	case TAG_X3D_IntegerTrigger:
		return "IntegerTrigger";
	case TAG_X3D_KeySensor:
		return "KeySensor";
	case TAG_X3D_LineProperties:
		return "LineProperties";
	case TAG_X3D_LineSet:
		return "LineSet";
	case TAG_X3D_LOD:
		return "LOD";
	case TAG_X3D_Material:
		return "Material";
	case TAG_X3D_MetadataDouble:
		return "MetadataDouble";
	case TAG_X3D_MetadataFloat:
		return "MetadataFloat";
	case TAG_X3D_MetadataInteger:
		return "MetadataInteger";
	case TAG_X3D_MetadataSet:
		return "MetadataSet";
	case TAG_X3D_MetadataString:
		return "MetadataString";
	case TAG_X3D_MovieTexture:
		return "MovieTexture";
	case TAG_X3D_MultiTexture:
		return "MultiTexture";
	case TAG_X3D_MultiTextureCoordinate:
		return "MultiTextureCoordinate";
	case TAG_X3D_MultiTextureTransform:
		return "MultiTextureTransform";
	case TAG_X3D_NavigationInfo:
		return "NavigationInfo";
	case TAG_X3D_Normal:
		return "Normal";
	case TAG_X3D_NormalInterpolator:
		return "NormalInterpolator";
	case TAG_X3D_OrientationInterpolator:
		return "OrientationInterpolator";
	case TAG_X3D_PixelTexture:
		return "PixelTexture";
	case TAG_X3D_PlaneSensor:
		return "PlaneSensor";
	case TAG_X3D_PointLight:
		return "PointLight";
	case TAG_X3D_PointSet:
		return "PointSet";
	case TAG_X3D_Polyline2D:
		return "Polyline2D";
	case TAG_X3D_Polypoint2D:
		return "Polypoint2D";
	case TAG_X3D_PositionInterpolator:
		return "PositionInterpolator";
	case TAG_X3D_PositionInterpolator2D:
		return "PositionInterpolator2D";
	case TAG_X3D_ProximitySensor:
		return "ProximitySensor";
	case TAG_X3D_Rectangle2D:
		return "Rectangle2D";
	case TAG_X3D_ScalarInterpolator:
		return "ScalarInterpolator";
	case TAG_X3D_Script:
		return "Script";
	case TAG_X3D_Shape:
		return "Shape";
	case TAG_X3D_Sound:
		return "Sound";
	case TAG_X3D_Sphere:
		return "Sphere";
	case TAG_X3D_SphereSensor:
		return "SphereSensor";
	case TAG_X3D_SpotLight:
		return "SpotLight";
	case TAG_X3D_StaticGroup:
		return "StaticGroup";
	case TAG_X3D_StringSensor:
		return "StringSensor";
	case TAG_X3D_Switch:
		return "Switch";
	case TAG_X3D_Text:
		return "Text";
	case TAG_X3D_TextureBackground:
		return "TextureBackground";
	case TAG_X3D_TextureCoordinate:
		return "TextureCoordinate";
	case TAG_X3D_TextureCoordinateGenerator:
		return "TextureCoordinateGenerator";
	case TAG_X3D_TextureTransform:
		return "TextureTransform";
	case TAG_X3D_TimeSensor:
		return "TimeSensor";
	case TAG_X3D_TimeTrigger:
		return "TimeTrigger";
	case TAG_X3D_TouchSensor:
		return "TouchSensor";
	case TAG_X3D_Transform:
		return "Transform";
	case TAG_X3D_TriangleFanSet:
		return "TriangleFanSet";
	case TAG_X3D_TriangleSet:
		return "TriangleSet";
	case TAG_X3D_TriangleSet2D:
		return "TriangleSet2D";
	case TAG_X3D_TriangleStripSet:
		return "TriangleStripSet";
	case TAG_X3D_Viewpoint:
		return "Viewpoint";
	case TAG_X3D_VisibilitySensor:
		return "VisibilitySensor";
	case TAG_X3D_WorldInfo:
		return "WorldInfo";
	default:
		return "Unknown Node";
	}
}

void X3D_Node_Del(SFNode *node)
{
	switch (node->sgprivate->tag) {
	case TAG_X3D_Anchor:
		Anchor_Del(node); return;
	case TAG_X3D_Appearance:
		Appearance_Del(node); return;
	case TAG_X3D_Arc2D:
		Arc2D_Del(node); return;
	case TAG_X3D_ArcClose2D:
		ArcClose2D_Del(node); return;
	case TAG_X3D_AudioClip:
		AudioClip_Del(node); return;
	case TAG_X3D_Background:
		Background_Del(node); return;
	case TAG_X3D_Billboard:
		Billboard_Del(node); return;
	case TAG_X3D_BooleanFilter:
		BooleanFilter_Del(node); return;
	case TAG_X3D_BooleanSequencer:
		BooleanSequencer_Del(node); return;
	case TAG_X3D_BooleanToggle:
		BooleanToggle_Del(node); return;
	case TAG_X3D_BooleanTrigger:
		BooleanTrigger_Del(node); return;
	case TAG_X3D_Box:
		Box_Del(node); return;
	case TAG_X3D_Circle2D:
		Circle2D_Del(node); return;
	case TAG_X3D_Collision:
		Collision_Del(node); return;
	case TAG_X3D_Color:
		Color_Del(node); return;
	case TAG_X3D_ColorInterpolator:
		ColorInterpolator_Del(node); return;
	case TAG_X3D_ColorRGBA:
		ColorRGBA_Del(node); return;
	case TAG_X3D_Cone:
		Cone_Del(node); return;
	case TAG_X3D_Contour2D:
		Contour2D_Del(node); return;
	case TAG_X3D_ContourPolyline2D:
		ContourPolyline2D_Del(node); return;
	case TAG_X3D_Coordinate:
		Coordinate_Del(node); return;
	case TAG_X3D_CoordinateDouble:
		CoordinateDouble_Del(node); return;
	case TAG_X3D_Coordinate2D:
		Coordinate2D_Del(node); return;
	case TAG_X3D_CoordinateInterpolator:
		CoordinateInterpolator_Del(node); return;
	case TAG_X3D_CoordinateInterpolator2D:
		CoordinateInterpolator2D_Del(node); return;
	case TAG_X3D_Cylinder:
		Cylinder_Del(node); return;
	case TAG_X3D_CylinderSensor:
		CylinderSensor_Del(node); return;
	case TAG_X3D_DirectionalLight:
		DirectionalLight_Del(node); return;
	case TAG_X3D_Disk2D:
		Disk2D_Del(node); return;
	case TAG_X3D_ElevationGrid:
		ElevationGrid_Del(node); return;
	case TAG_X3D_Extrusion:
		Extrusion_Del(node); return;
	case TAG_X3D_FillProperties:
		FillProperties_Del(node); return;
	case TAG_X3D_Fog:
		Fog_Del(node); return;
	case TAG_X3D_FontStyle:
		FontStyle_Del(node); return;
	case TAG_X3D_Group:
		Group_Del(node); return;
	case TAG_X3D_ImageTexture:
		ImageTexture_Del(node); return;
	case TAG_X3D_IndexedFaceSet:
		IndexedFaceSet_Del(node); return;
	case TAG_X3D_IndexedLineSet:
		IndexedLineSet_Del(node); return;
	case TAG_X3D_IndexedTriangleFanSet:
		IndexedTriangleFanSet_Del(node); return;
	case TAG_X3D_IndexedTriangleSet:
		IndexedTriangleSet_Del(node); return;
	case TAG_X3D_IndexedTriangleStripSet:
		IndexedTriangleStripSet_Del(node); return;
	case TAG_X3D_Inline:
		Inline_Del(node); return;
	case TAG_X3D_IntegerSequencer:
		IntegerSequencer_Del(node); return;
	case TAG_X3D_IntegerTrigger:
		IntegerTrigger_Del(node); return;
	case TAG_X3D_KeySensor:
		KeySensor_Del(node); return;
	case TAG_X3D_LineProperties:
		LineProperties_Del(node); return;
	case TAG_X3D_LineSet:
		LineSet_Del(node); return;
	case TAG_X3D_LOD:
		LOD_Del(node); return;
	case TAG_X3D_Material:
		Material_Del(node); return;
	case TAG_X3D_MetadataDouble:
		MetadataDouble_Del(node); return;
	case TAG_X3D_MetadataFloat:
		MetadataFloat_Del(node); return;
	case TAG_X3D_MetadataInteger:
		MetadataInteger_Del(node); return;
	case TAG_X3D_MetadataSet:
		MetadataSet_Del(node); return;
	case TAG_X3D_MetadataString:
		MetadataString_Del(node); return;
	case TAG_X3D_MovieTexture:
		MovieTexture_Del(node); return;
	case TAG_X3D_MultiTexture:
		MultiTexture_Del(node); return;
	case TAG_X3D_MultiTextureCoordinate:
		MultiTextureCoordinate_Del(node); return;
	case TAG_X3D_MultiTextureTransform:
		MultiTextureTransform_Del(node); return;
	case TAG_X3D_NavigationInfo:
		NavigationInfo_Del(node); return;
	case TAG_X3D_Normal:
		Normal_Del(node); return;
	case TAG_X3D_NormalInterpolator:
		NormalInterpolator_Del(node); return;
	case TAG_X3D_OrientationInterpolator:
		OrientationInterpolator_Del(node); return;
	case TAG_X3D_PixelTexture:
		PixelTexture_Del(node); return;
	case TAG_X3D_PlaneSensor:
		PlaneSensor_Del(node); return;
	case TAG_X3D_PointLight:
		PointLight_Del(node); return;
	case TAG_X3D_PointSet:
		PointSet_Del(node); return;
	case TAG_X3D_Polyline2D:
		Polyline2D_Del(node); return;
	case TAG_X3D_Polypoint2D:
		Polypoint2D_Del(node); return;
	case TAG_X3D_PositionInterpolator:
		PositionInterpolator_Del(node); return;
	case TAG_X3D_PositionInterpolator2D:
		PositionInterpolator2D_Del(node); return;
	case TAG_X3D_ProximitySensor:
		ProximitySensor_Del(node); return;
	case TAG_X3D_Rectangle2D:
		Rectangle2D_Del(node); return;
	case TAG_X3D_ScalarInterpolator:
		ScalarInterpolator_Del(node); return;
	case TAG_X3D_Script:
		Script_Del(node); return;
	case TAG_X3D_Shape:
		Shape_Del(node); return;
	case TAG_X3D_Sound:
		Sound_Del(node); return;
	case TAG_X3D_Sphere:
		Sphere_Del(node); return;
	case TAG_X3D_SphereSensor:
		SphereSensor_Del(node); return;
	case TAG_X3D_SpotLight:
		SpotLight_Del(node); return;
	case TAG_X3D_StaticGroup:
		StaticGroup_Del(node); return;
	case TAG_X3D_StringSensor:
		StringSensor_Del(node); return;
	case TAG_X3D_Switch:
		Switch_Del(node); return;
	case TAG_X3D_Text:
		Text_Del(node); return;
	case TAG_X3D_TextureBackground:
		TextureBackground_Del(node); return;
	case TAG_X3D_TextureCoordinate:
		TextureCoordinate_Del(node); return;
	case TAG_X3D_TextureCoordinateGenerator:
		TextureCoordinateGenerator_Del(node); return;
	case TAG_X3D_TextureTransform:
		TextureTransform_Del(node); return;
	case TAG_X3D_TimeSensor:
		TimeSensor_Del(node); return;
	case TAG_X3D_TimeTrigger:
		TimeTrigger_Del(node); return;
	case TAG_X3D_TouchSensor:
		TouchSensor_Del(node); return;
	case TAG_X3D_Transform:
		Transform_Del(node); return;
	case TAG_X3D_TriangleFanSet:
		TriangleFanSet_Del(node); return;
	case TAG_X3D_TriangleSet:
		TriangleSet_Del(node); return;
	case TAG_X3D_TriangleSet2D:
		TriangleSet2D_Del(node); return;
	case TAG_X3D_TriangleStripSet:
		TriangleStripSet_Del(node); return;
	case TAG_X3D_Viewpoint:
		Viewpoint_Del(node); return;
	case TAG_X3D_VisibilitySensor:
		VisibilitySensor_Del(node); return;
	case TAG_X3D_WorldInfo:
		WorldInfo_Del(node); return;
	default:
		return;
	}
}

u32 X3DNode_GetFieldCount(SFNode *node)
{
	switch (node->sgprivate->tag) {
	case TAG_X3D_Anchor:return Anchor_get_field_count(node);
	case TAG_X3D_Appearance:return Appearance_get_field_count(node);
	case TAG_X3D_Arc2D:return Arc2D_get_field_count(node);
	case TAG_X3D_ArcClose2D:return ArcClose2D_get_field_count(node);
	case TAG_X3D_AudioClip:return AudioClip_get_field_count(node);
	case TAG_X3D_Background:return Background_get_field_count(node);
	case TAG_X3D_Billboard:return Billboard_get_field_count(node);
	case TAG_X3D_BooleanFilter:return BooleanFilter_get_field_count(node);
	case TAG_X3D_BooleanSequencer:return BooleanSequencer_get_field_count(node);
	case TAG_X3D_BooleanToggle:return BooleanToggle_get_field_count(node);
	case TAG_X3D_BooleanTrigger:return BooleanTrigger_get_field_count(node);
	case TAG_X3D_Box:return Box_get_field_count(node);
	case TAG_X3D_Circle2D:return Circle2D_get_field_count(node);
	case TAG_X3D_Collision:return Collision_get_field_count(node);
	case TAG_X3D_Color:return Color_get_field_count(node);
	case TAG_X3D_ColorInterpolator:return ColorInterpolator_get_field_count(node);
	case TAG_X3D_ColorRGBA:return ColorRGBA_get_field_count(node);
	case TAG_X3D_Cone:return Cone_get_field_count(node);
	case TAG_X3D_Contour2D:return Contour2D_get_field_count(node);
	case TAG_X3D_ContourPolyline2D:return ContourPolyline2D_get_field_count(node);
	case TAG_X3D_Coordinate:return Coordinate_get_field_count(node);
	case TAG_X3D_CoordinateDouble:return CoordinateDouble_get_field_count(node);
	case TAG_X3D_Coordinate2D:return Coordinate2D_get_field_count(node);
	case TAG_X3D_CoordinateInterpolator:return CoordinateInterpolator_get_field_count(node);
	case TAG_X3D_CoordinateInterpolator2D:return CoordinateInterpolator2D_get_field_count(node);
	case TAG_X3D_Cylinder:return Cylinder_get_field_count(node);
	case TAG_X3D_CylinderSensor:return CylinderSensor_get_field_count(node);
	case TAG_X3D_DirectionalLight:return DirectionalLight_get_field_count(node);
	case TAG_X3D_Disk2D:return Disk2D_get_field_count(node);
	case TAG_X3D_ElevationGrid:return ElevationGrid_get_field_count(node);
	case TAG_X3D_Extrusion:return Extrusion_get_field_count(node);
	case TAG_X3D_FillProperties:return FillProperties_get_field_count(node);
	case TAG_X3D_Fog:return Fog_get_field_count(node);
	case TAG_X3D_FontStyle:return FontStyle_get_field_count(node);
	case TAG_X3D_Group:return Group_get_field_count(node);
	case TAG_X3D_ImageTexture:return ImageTexture_get_field_count(node);
	case TAG_X3D_IndexedFaceSet:return IndexedFaceSet_get_field_count(node);
	case TAG_X3D_IndexedLineSet:return IndexedLineSet_get_field_count(node);
	case TAG_X3D_IndexedTriangleFanSet:return IndexedTriangleFanSet_get_field_count(node);
	case TAG_X3D_IndexedTriangleSet:return IndexedTriangleSet_get_field_count(node);
	case TAG_X3D_IndexedTriangleStripSet:return IndexedTriangleStripSet_get_field_count(node);
	case TAG_X3D_Inline:return Inline_get_field_count(node);
	case TAG_X3D_IntegerSequencer:return IntegerSequencer_get_field_count(node);
	case TAG_X3D_IntegerTrigger:return IntegerTrigger_get_field_count(node);
	case TAG_X3D_KeySensor:return KeySensor_get_field_count(node);
	case TAG_X3D_LineProperties:return LineProperties_get_field_count(node);
	case TAG_X3D_LineSet:return LineSet_get_field_count(node);
	case TAG_X3D_LOD:return LOD_get_field_count(node);
	case TAG_X3D_Material:return Material_get_field_count(node);
	case TAG_X3D_MetadataDouble:return MetadataDouble_get_field_count(node);
	case TAG_X3D_MetadataFloat:return MetadataFloat_get_field_count(node);
	case TAG_X3D_MetadataInteger:return MetadataInteger_get_field_count(node);
	case TAG_X3D_MetadataSet:return MetadataSet_get_field_count(node);
	case TAG_X3D_MetadataString:return MetadataString_get_field_count(node);
	case TAG_X3D_MovieTexture:return MovieTexture_get_field_count(node);
	case TAG_X3D_MultiTexture:return MultiTexture_get_field_count(node);
	case TAG_X3D_MultiTextureCoordinate:return MultiTextureCoordinate_get_field_count(node);
	case TAG_X3D_MultiTextureTransform:return MultiTextureTransform_get_field_count(node);
	case TAG_X3D_NavigationInfo:return NavigationInfo_get_field_count(node);
	case TAG_X3D_Normal:return Normal_get_field_count(node);
	case TAG_X3D_NormalInterpolator:return NormalInterpolator_get_field_count(node);
	case TAG_X3D_OrientationInterpolator:return OrientationInterpolator_get_field_count(node);
	case TAG_X3D_PixelTexture:return PixelTexture_get_field_count(node);
	case TAG_X3D_PlaneSensor:return PlaneSensor_get_field_count(node);
	case TAG_X3D_PointLight:return PointLight_get_field_count(node);
	case TAG_X3D_PointSet:return PointSet_get_field_count(node);
	case TAG_X3D_Polyline2D:return Polyline2D_get_field_count(node);
	case TAG_X3D_Polypoint2D:return Polypoint2D_get_field_count(node);
	case TAG_X3D_PositionInterpolator:return PositionInterpolator_get_field_count(node);
	case TAG_X3D_PositionInterpolator2D:return PositionInterpolator2D_get_field_count(node);
	case TAG_X3D_ProximitySensor:return ProximitySensor_get_field_count(node);
	case TAG_X3D_Rectangle2D:return Rectangle2D_get_field_count(node);
	case TAG_X3D_ScalarInterpolator:return ScalarInterpolator_get_field_count(node);
	case TAG_X3D_Script:return Script_get_field_count(node);
	case TAG_X3D_Shape:return Shape_get_field_count(node);
	case TAG_X3D_Sound:return Sound_get_field_count(node);
	case TAG_X3D_Sphere:return Sphere_get_field_count(node);
	case TAG_X3D_SphereSensor:return SphereSensor_get_field_count(node);
	case TAG_X3D_SpotLight:return SpotLight_get_field_count(node);
	case TAG_X3D_StaticGroup:return StaticGroup_get_field_count(node);
	case TAG_X3D_StringSensor:return StringSensor_get_field_count(node);
	case TAG_X3D_Switch:return Switch_get_field_count(node);
	case TAG_X3D_Text:return Text_get_field_count(node);
	case TAG_X3D_TextureBackground:return TextureBackground_get_field_count(node);
	case TAG_X3D_TextureCoordinate:return TextureCoordinate_get_field_count(node);
	case TAG_X3D_TextureCoordinateGenerator:return TextureCoordinateGenerator_get_field_count(node);
	case TAG_X3D_TextureTransform:return TextureTransform_get_field_count(node);
	case TAG_X3D_TimeSensor:return TimeSensor_get_field_count(node);
	case TAG_X3D_TimeTrigger:return TimeTrigger_get_field_count(node);
	case TAG_X3D_TouchSensor:return TouchSensor_get_field_count(node);
	case TAG_X3D_Transform:return Transform_get_field_count(node);
	case TAG_X3D_TriangleFanSet:return TriangleFanSet_get_field_count(node);
	case TAG_X3D_TriangleSet:return TriangleSet_get_field_count(node);
	case TAG_X3D_TriangleSet2D:return TriangleSet2D_get_field_count(node);
	case TAG_X3D_TriangleStripSet:return TriangleStripSet_get_field_count(node);
	case TAG_X3D_Viewpoint:return Viewpoint_get_field_count(node);
	case TAG_X3D_VisibilitySensor:return VisibilitySensor_get_field_count(node);
	case TAG_X3D_WorldInfo:return WorldInfo_get_field_count(node);
	default:
		return 0;
	}
}

M4Err X3DNode_GetField(SFNode *node, FieldInfo *field)
{
	switch (node->sgprivate->tag) {
	case TAG_X3D_Anchor: return Anchor_get_field(node, field);
	case TAG_X3D_Appearance: return Appearance_get_field(node, field);
	case TAG_X3D_Arc2D: return Arc2D_get_field(node, field);
	case TAG_X3D_ArcClose2D: return ArcClose2D_get_field(node, field);
	case TAG_X3D_AudioClip: return AudioClip_get_field(node, field);
	case TAG_X3D_Background: return Background_get_field(node, field);
	case TAG_X3D_Billboard: return Billboard_get_field(node, field);
	case TAG_X3D_BooleanFilter: return BooleanFilter_get_field(node, field);
	case TAG_X3D_BooleanSequencer: return BooleanSequencer_get_field(node, field);
	case TAG_X3D_BooleanToggle: return BooleanToggle_get_field(node, field);
	case TAG_X3D_BooleanTrigger: return BooleanTrigger_get_field(node, field);
	case TAG_X3D_Box: return Box_get_field(node, field);
	case TAG_X3D_Circle2D: return Circle2D_get_field(node, field);
	case TAG_X3D_Collision: return Collision_get_field(node, field);
	case TAG_X3D_Color: return Color_get_field(node, field);
	case TAG_X3D_ColorInterpolator: return ColorInterpolator_get_field(node, field);
	case TAG_X3D_ColorRGBA: return ColorRGBA_get_field(node, field);
	case TAG_X3D_Cone: return Cone_get_field(node, field);
	case TAG_X3D_Contour2D: return Contour2D_get_field(node, field);
	case TAG_X3D_ContourPolyline2D: return ContourPolyline2D_get_field(node, field);
	case TAG_X3D_Coordinate: return Coordinate_get_field(node, field);
	case TAG_X3D_CoordinateDouble: return CoordinateDouble_get_field(node, field);
	case TAG_X3D_Coordinate2D: return Coordinate2D_get_field(node, field);
	case TAG_X3D_CoordinateInterpolator: return CoordinateInterpolator_get_field(node, field);
	case TAG_X3D_CoordinateInterpolator2D: return CoordinateInterpolator2D_get_field(node, field);
	case TAG_X3D_Cylinder: return Cylinder_get_field(node, field);
	case TAG_X3D_CylinderSensor: return CylinderSensor_get_field(node, field);
	case TAG_X3D_DirectionalLight: return DirectionalLight_get_field(node, field);
	case TAG_X3D_Disk2D: return Disk2D_get_field(node, field);
	case TAG_X3D_ElevationGrid: return ElevationGrid_get_field(node, field);
	case TAG_X3D_Extrusion: return Extrusion_get_field(node, field);
	case TAG_X3D_FillProperties: return FillProperties_get_field(node, field);
	case TAG_X3D_Fog: return Fog_get_field(node, field);
	case TAG_X3D_FontStyle: return FontStyle_get_field(node, field);
	case TAG_X3D_Group: return Group_get_field(node, field);
	case TAG_X3D_ImageTexture: return ImageTexture_get_field(node, field);
	case TAG_X3D_IndexedFaceSet: return IndexedFaceSet_get_field(node, field);
	case TAG_X3D_IndexedLineSet: return IndexedLineSet_get_field(node, field);
	case TAG_X3D_IndexedTriangleFanSet: return IndexedTriangleFanSet_get_field(node, field);
	case TAG_X3D_IndexedTriangleSet: return IndexedTriangleSet_get_field(node, field);
	case TAG_X3D_IndexedTriangleStripSet: return IndexedTriangleStripSet_get_field(node, field);
	case TAG_X3D_Inline: return Inline_get_field(node, field);
	case TAG_X3D_IntegerSequencer: return IntegerSequencer_get_field(node, field);
	case TAG_X3D_IntegerTrigger: return IntegerTrigger_get_field(node, field);
	case TAG_X3D_KeySensor: return KeySensor_get_field(node, field);
	case TAG_X3D_LineProperties: return LineProperties_get_field(node, field);
	case TAG_X3D_LineSet: return LineSet_get_field(node, field);
	case TAG_X3D_LOD: return LOD_get_field(node, field);
	case TAG_X3D_Material: return Material_get_field(node, field);
	case TAG_X3D_MetadataDouble: return MetadataDouble_get_field(node, field);
	case TAG_X3D_MetadataFloat: return MetadataFloat_get_field(node, field);
	case TAG_X3D_MetadataInteger: return MetadataInteger_get_field(node, field);
	case TAG_X3D_MetadataSet: return MetadataSet_get_field(node, field);
	case TAG_X3D_MetadataString: return MetadataString_get_field(node, field);
	case TAG_X3D_MovieTexture: return MovieTexture_get_field(node, field);
	case TAG_X3D_MultiTexture: return MultiTexture_get_field(node, field);
	case TAG_X3D_MultiTextureCoordinate: return MultiTextureCoordinate_get_field(node, field);
	case TAG_X3D_MultiTextureTransform: return MultiTextureTransform_get_field(node, field);
	case TAG_X3D_NavigationInfo: return NavigationInfo_get_field(node, field);
	case TAG_X3D_Normal: return Normal_get_field(node, field);
	case TAG_X3D_NormalInterpolator: return NormalInterpolator_get_field(node, field);
	case TAG_X3D_OrientationInterpolator: return OrientationInterpolator_get_field(node, field);
	case TAG_X3D_PixelTexture: return PixelTexture_get_field(node, field);
	case TAG_X3D_PlaneSensor: return PlaneSensor_get_field(node, field);
	case TAG_X3D_PointLight: return PointLight_get_field(node, field);
	case TAG_X3D_PointSet: return PointSet_get_field(node, field);
	case TAG_X3D_Polyline2D: return Polyline2D_get_field(node, field);
	case TAG_X3D_Polypoint2D: return Polypoint2D_get_field(node, field);
	case TAG_X3D_PositionInterpolator: return PositionInterpolator_get_field(node, field);
	case TAG_X3D_PositionInterpolator2D: return PositionInterpolator2D_get_field(node, field);
	case TAG_X3D_ProximitySensor: return ProximitySensor_get_field(node, field);
	case TAG_X3D_Rectangle2D: return Rectangle2D_get_field(node, field);
	case TAG_X3D_ScalarInterpolator: return ScalarInterpolator_get_field(node, field);
	case TAG_X3D_Script: return Script_get_field(node, field);
	case TAG_X3D_Shape: return Shape_get_field(node, field);
	case TAG_X3D_Sound: return Sound_get_field(node, field);
	case TAG_X3D_Sphere: return Sphere_get_field(node, field);
	case TAG_X3D_SphereSensor: return SphereSensor_get_field(node, field);
	case TAG_X3D_SpotLight: return SpotLight_get_field(node, field);
	case TAG_X3D_StaticGroup: return StaticGroup_get_field(node, field);
	case TAG_X3D_StringSensor: return StringSensor_get_field(node, field);
	case TAG_X3D_Switch: return Switch_get_field(node, field);
	case TAG_X3D_Text: return Text_get_field(node, field);
	case TAG_X3D_TextureBackground: return TextureBackground_get_field(node, field);
	case TAG_X3D_TextureCoordinate: return TextureCoordinate_get_field(node, field);
	case TAG_X3D_TextureCoordinateGenerator: return TextureCoordinateGenerator_get_field(node, field);
	case TAG_X3D_TextureTransform: return TextureTransform_get_field(node, field);
	case TAG_X3D_TimeSensor: return TimeSensor_get_field(node, field);
	case TAG_X3D_TimeTrigger: return TimeTrigger_get_field(node, field);
	case TAG_X3D_TouchSensor: return TouchSensor_get_field(node, field);
	case TAG_X3D_Transform: return Transform_get_field(node, field);
	case TAG_X3D_TriangleFanSet: return TriangleFanSet_get_field(node, field);
	case TAG_X3D_TriangleSet: return TriangleSet_get_field(node, field);
	case TAG_X3D_TriangleSet2D: return TriangleSet2D_get_field(node, field);
	case TAG_X3D_TriangleStripSet: return TriangleStripSet_get_field(node, field);
	case TAG_X3D_Viewpoint: return Viewpoint_get_field(node, field);
	case TAG_X3D_VisibilitySensor: return VisibilitySensor_get_field(node, field);
	case TAG_X3D_WorldInfo: return WorldInfo_get_field(node, field);
	default:
		return M4BadParam;
	}
}


#endif


u32 X3D_GetTagByName(const char *node_name)
{
	if(!node_name) return 0;
	if (!stricmp(node_name, "Anchor")) return TAG_X3D_Anchor;
	if (!stricmp(node_name, "Appearance")) return TAG_X3D_Appearance;
	if (!stricmp(node_name, "Arc2D")) return TAG_X3D_Arc2D;
	if (!stricmp(node_name, "ArcClose2D")) return TAG_X3D_ArcClose2D;
	if (!stricmp(node_name, "AudioClip")) return TAG_X3D_AudioClip;
	if (!stricmp(node_name, "Background")) return TAG_X3D_Background;
	if (!stricmp(node_name, "Billboard")) return TAG_X3D_Billboard;
	if (!stricmp(node_name, "BooleanFilter")) return TAG_X3D_BooleanFilter;
	if (!stricmp(node_name, "BooleanSequencer")) return TAG_X3D_BooleanSequencer;
	if (!stricmp(node_name, "BooleanToggle")) return TAG_X3D_BooleanToggle;
	if (!stricmp(node_name, "BooleanTrigger")) return TAG_X3D_BooleanTrigger;
	if (!stricmp(node_name, "Box")) return TAG_X3D_Box;
	if (!stricmp(node_name, "Circle2D")) return TAG_X3D_Circle2D;
	if (!stricmp(node_name, "Collision")) return TAG_X3D_Collision;
	if (!stricmp(node_name, "Color")) return TAG_X3D_Color;
	if (!stricmp(node_name, "ColorInterpolator")) return TAG_X3D_ColorInterpolator;
	if (!stricmp(node_name, "ColorRGBA")) return TAG_X3D_ColorRGBA;
	if (!stricmp(node_name, "Cone")) return TAG_X3D_Cone;
	if (!stricmp(node_name, "Contour2D")) return TAG_X3D_Contour2D;
	if (!stricmp(node_name, "ContourPolyline2D")) return TAG_X3D_ContourPolyline2D;
	if (!stricmp(node_name, "Coordinate")) return TAG_X3D_Coordinate;
	if (!stricmp(node_name, "CoordinateDouble")) return TAG_X3D_CoordinateDouble;
	if (!stricmp(node_name, "Coordinate2D")) return TAG_X3D_Coordinate2D;
	if (!stricmp(node_name, "CoordinateInterpolator")) return TAG_X3D_CoordinateInterpolator;
	if (!stricmp(node_name, "CoordinateInterpolator2D")) return TAG_X3D_CoordinateInterpolator2D;
	if (!stricmp(node_name, "Cylinder")) return TAG_X3D_Cylinder;
	if (!stricmp(node_name, "CylinderSensor")) return TAG_X3D_CylinderSensor;
	if (!stricmp(node_name, "DirectionalLight")) return TAG_X3D_DirectionalLight;
	if (!stricmp(node_name, "Disk2D")) return TAG_X3D_Disk2D;
	if (!stricmp(node_name, "ElevationGrid")) return TAG_X3D_ElevationGrid;
	if (!stricmp(node_name, "Extrusion")) return TAG_X3D_Extrusion;
	if (!stricmp(node_name, "FillProperties")) return TAG_X3D_FillProperties;
	if (!stricmp(node_name, "Fog")) return TAG_X3D_Fog;
	if (!stricmp(node_name, "FontStyle")) return TAG_X3D_FontStyle;
	if (!stricmp(node_name, "Group")) return TAG_X3D_Group;
	if (!stricmp(node_name, "ImageTexture")) return TAG_X3D_ImageTexture;
	if (!stricmp(node_name, "IndexedFaceSet")) return TAG_X3D_IndexedFaceSet;
	if (!stricmp(node_name, "IndexedLineSet")) return TAG_X3D_IndexedLineSet;
	if (!stricmp(node_name, "IndexedTriangleFanSet")) return TAG_X3D_IndexedTriangleFanSet;
	if (!stricmp(node_name, "IndexedTriangleSet")) return TAG_X3D_IndexedTriangleSet;
	if (!stricmp(node_name, "IndexedTriangleStripSet")) return TAG_X3D_IndexedTriangleStripSet;
	if (!stricmp(node_name, "Inline")) return TAG_X3D_Inline;
	if (!stricmp(node_name, "IntegerSequencer")) return TAG_X3D_IntegerSequencer;
	if (!stricmp(node_name, "IntegerTrigger")) return TAG_X3D_IntegerTrigger;
	if (!stricmp(node_name, "KeySensor")) return TAG_X3D_KeySensor;
	if (!stricmp(node_name, "LineProperties")) return TAG_X3D_LineProperties;
	if (!stricmp(node_name, "LineSet")) return TAG_X3D_LineSet;
	if (!stricmp(node_name, "LOD")) return TAG_X3D_LOD;
	if (!stricmp(node_name, "Material")) return TAG_X3D_Material;
	if (!stricmp(node_name, "MetadataDouble")) return TAG_X3D_MetadataDouble;
	if (!stricmp(node_name, "MetadataFloat")) return TAG_X3D_MetadataFloat;
	if (!stricmp(node_name, "MetadataInteger")) return TAG_X3D_MetadataInteger;
	if (!stricmp(node_name, "MetadataSet")) return TAG_X3D_MetadataSet;
	if (!stricmp(node_name, "MetadataString")) return TAG_X3D_MetadataString;
	if (!stricmp(node_name, "MovieTexture")) return TAG_X3D_MovieTexture;
	if (!stricmp(node_name, "MultiTexture")) return TAG_X3D_MultiTexture;
	if (!stricmp(node_name, "MultiTextureCoordinate")) return TAG_X3D_MultiTextureCoordinate;
	if (!stricmp(node_name, "MultiTextureTransform")) return TAG_X3D_MultiTextureTransform;
	if (!stricmp(node_name, "NavigationInfo")) return TAG_X3D_NavigationInfo;
	if (!stricmp(node_name, "Normal")) return TAG_X3D_Normal;
	if (!stricmp(node_name, "NormalInterpolator")) return TAG_X3D_NormalInterpolator;
	if (!stricmp(node_name, "OrientationInterpolator")) return TAG_X3D_OrientationInterpolator;
	if (!stricmp(node_name, "PixelTexture")) return TAG_X3D_PixelTexture;
	if (!stricmp(node_name, "PlaneSensor")) return TAG_X3D_PlaneSensor;
	if (!stricmp(node_name, "PointLight")) return TAG_X3D_PointLight;
	if (!stricmp(node_name, "PointSet")) return TAG_X3D_PointSet;
	if (!stricmp(node_name, "Polyline2D")) return TAG_X3D_Polyline2D;
	if (!stricmp(node_name, "Polypoint2D")) return TAG_X3D_Polypoint2D;
	if (!stricmp(node_name, "PositionInterpolator")) return TAG_X3D_PositionInterpolator;
	if (!stricmp(node_name, "PositionInterpolator2D")) return TAG_X3D_PositionInterpolator2D;
	if (!stricmp(node_name, "ProximitySensor")) return TAG_X3D_ProximitySensor;
	if (!stricmp(node_name, "Rectangle2D")) return TAG_X3D_Rectangle2D;
	if (!stricmp(node_name, "ScalarInterpolator")) return TAG_X3D_ScalarInterpolator;
	if (!stricmp(node_name, "Script")) return TAG_X3D_Script;
	if (!stricmp(node_name, "Shape")) return TAG_X3D_Shape;
	if (!stricmp(node_name, "Sound")) return TAG_X3D_Sound;
	if (!stricmp(node_name, "Sphere")) return TAG_X3D_Sphere;
	if (!stricmp(node_name, "SphereSensor")) return TAG_X3D_SphereSensor;
	if (!stricmp(node_name, "SpotLight")) return TAG_X3D_SpotLight;
	if (!stricmp(node_name, "StaticGroup")) return TAG_X3D_StaticGroup;
	if (!stricmp(node_name, "StringSensor")) return TAG_X3D_StringSensor;
	if (!stricmp(node_name, "Switch")) return TAG_X3D_Switch;
	if (!stricmp(node_name, "Text")) return TAG_X3D_Text;
	if (!stricmp(node_name, "TextureBackground")) return TAG_X3D_TextureBackground;
	if (!stricmp(node_name, "TextureCoordinate")) return TAG_X3D_TextureCoordinate;
	if (!stricmp(node_name, "TextureCoordinateGenerator")) return TAG_X3D_TextureCoordinateGenerator;
	if (!stricmp(node_name, "TextureTransform")) return TAG_X3D_TextureTransform;
	if (!stricmp(node_name, "TimeSensor")) return TAG_X3D_TimeSensor;
	if (!stricmp(node_name, "TimeTrigger")) return TAG_X3D_TimeTrigger;
	if (!stricmp(node_name, "TouchSensor")) return TAG_X3D_TouchSensor;
	if (!stricmp(node_name, "Transform")) return TAG_X3D_Transform;
	if (!stricmp(node_name, "TriangleFanSet")) return TAG_X3D_TriangleFanSet;
	if (!stricmp(node_name, "TriangleSet")) return TAG_X3D_TriangleSet;
	if (!stricmp(node_name, "TriangleSet2D")) return TAG_X3D_TriangleSet2D;
	if (!stricmp(node_name, "TriangleStripSet")) return TAG_X3D_TriangleStripSet;
	if (!stricmp(node_name, "Viewpoint")) return TAG_X3D_Viewpoint;
	if (!stricmp(node_name, "VisibilitySensor")) return TAG_X3D_VisibilitySensor;
	if (!stricmp(node_name, "WorldInfo")) return TAG_X3D_WorldInfo;
	return 0;
}



/* NDT X3D */

#define SFWorldNode_X3D_Count	127
static const u32 SFWorldNode_X3D_TypeToTag[127] = {
 TAG_X3D_Anchor, TAG_X3D_Appearance, TAG_X3D_Arc2D, TAG_X3D_ArcClose2D, TAG_X3D_AudioClip, TAG_X3D_Background, TAG_X3D_Billboard, TAG_X3D_BooleanFilter, TAG_X3D_BooleanSequencer, TAG_X3D_BooleanToggle, TAG_X3D_BooleanTrigger, TAG_X3D_Box, TAG_X3D_Circle2D, TAG_X3D_Collision, TAG_X3D_Color, TAG_X3D_ColorInterpolator, TAG_X3D_ColorRGBA, TAG_X3D_Cone, TAG_X3D_Contour2D, TAG_X3D_ContourPolyline2D, TAG_X3D_Coordinate, TAG_X3D_CoordinateDouble, TAG_X3D_Coordinate2D, TAG_X3D_CoordinateInterpolator, TAG_X3D_CoordinateInterpolator2D, TAG_X3D_Cylinder, TAG_X3D_CylinderSensor, TAG_X3D_DirectionalLight, TAG_X3D_Disk2D, TAG_X3D_ElevationGrid, TAG_X3D_EspduTransform, TAG_X3D_Extrusion, TAG_X3D_FillProperties, TAG_X3D_Fog, TAG_X3D_FontStyle, TAG_X3D_GeoCoordinate, TAG_X3D_GeoElevationGrid, TAG_X3D_GeoLocation, TAG_X3D_GeoLOD, TAG_X3D_GeoMetadata, TAG_X3D_GeoPositionInterpolator, TAG_X3D_GeoTouchSensor, TAG_X3D_GeoViewpoint, TAG_X3D_Group, TAG_X3D_HAnimDisplacer, TAG_X3D_HAnimHumanoid, TAG_X3D_HAnimJoint, TAG_X3D_HAnimSegment, TAG_X3D_HAnimSite, TAG_X3D_ImageTexture, TAG_X3D_IndexedFaceSet, TAG_X3D_IndexedLineSet, TAG_X3D_IndexedTriangleFanSet, TAG_X3D_IndexedTriangleSet, TAG_X3D_IndexedTriangleStripSet, TAG_X3D_Inline, TAG_X3D_IntegerSequencer, TAG_X3D_IntegerTrigger, TAG_X3D_KeySensor, TAG_X3D_LineProperties, TAG_X3D_LineSet, TAG_X3D_LoadSensor, TAG_X3D_LOD, TAG_X3D_Material, TAG_X3D_MetadataDouble, TAG_X3D_MetadataFloat, TAG_X3D_MetadataInteger, TAG_X3D_MetadataSet, TAG_X3D_MetadataString, TAG_X3D_MovieTexture, TAG_X3D_MultiTexture, TAG_X3D_MultiTextureCoordinate, TAG_X3D_MultiTextureTransform, TAG_X3D_NavigationInfo, TAG_X3D_Normal, TAG_X3D_NormalInterpolator, TAG_X3D_NurbsCurve, TAG_X3D_NurbsCurve2D, TAG_X3D_NurbsOrientationInterpolator, TAG_X3D_NurbsPatchSurface, TAG_X3D_NurbsPositionInterpolator, TAG_X3D_NurbsSet, TAG_X3D_NurbsSurfaceInterpolator, TAG_X3D_NurbsSweptSurface, TAG_X3D_NurbsSwungSurface, TAG_X3D_NurbsTextureCoordinate, TAG_X3D_NurbsTrimmedSurface, TAG_X3D_OrientationInterpolator, TAG_X3D_PixelTexture, TAG_X3D_PlaneSensor, TAG_X3D_PointLight, TAG_X3D_PointSet, TAG_X3D_Polyline2D, TAG_X3D_Polypoint2D, TAG_X3D_PositionInterpolator, TAG_X3D_PositionInterpolator2D, TAG_X3D_ProximitySensor, TAG_X3D_ReceiverPdu, TAG_X3D_Rectangle2D, TAG_X3D_ScalarInterpolator, TAG_X3D_Script, TAG_X3D_Shape, TAG_X3D_SignalPdu, TAG_X3D_Sound, TAG_X3D_Sphere, TAG_X3D_SphereSensor, TAG_X3D_SpotLight, TAG_X3D_StaticGroup, TAG_X3D_StringSensor, TAG_X3D_Switch, TAG_X3D_Text, TAG_X3D_TextureBackground, TAG_X3D_TextureCoordinate, TAG_X3D_TextureCoordinateGenerator, TAG_X3D_TextureTransform, TAG_X3D_TimeSensor, TAG_X3D_TimeTrigger, TAG_X3D_TouchSensor, TAG_X3D_Transform, TAG_X3D_TransmitterPdu, TAG_X3D_TriangleFanSet, TAG_X3D_TriangleSet, TAG_X3D_TriangleSet2D, TAG_X3D_TriangleStripSet, TAG_X3D_Viewpoint, TAG_X3D_VisibilitySensor, TAG_X3D_WorldInfo
};

#define SF3DNode_X3D_Count	60
static const u32 SF3DNode_X3D_TypeToTag[60] = {
 TAG_X3D_Anchor, TAG_X3D_Background, TAG_X3D_Billboard, TAG_X3D_BooleanFilter, TAG_X3D_BooleanSequencer, TAG_X3D_BooleanToggle, TAG_X3D_BooleanTrigger, TAG_X3D_Collision, TAG_X3D_ColorInterpolator, TAG_X3D_CoordinateInterpolator, TAG_X3D_CoordinateInterpolator2D, TAG_X3D_CylinderSensor, TAG_X3D_DirectionalLight, TAG_X3D_EspduTransform, TAG_X3D_Fog, TAG_X3D_GeoLocation, TAG_X3D_GeoLOD, TAG_X3D_GeoMetadata, TAG_X3D_GeoPositionInterpolator, TAG_X3D_GeoTouchSensor, TAG_X3D_GeoViewpoint, TAG_X3D_Group, TAG_X3D_HAnimHumanoid, TAG_X3D_Inline, TAG_X3D_IntegerSequencer, TAG_X3D_IntegerTrigger, TAG_X3D_KeySensor, TAG_X3D_LOD, TAG_X3D_NavigationInfo, TAG_X3D_NormalInterpolator, TAG_X3D_NurbsOrientationInterpolator, TAG_X3D_NurbsPositionInterpolator, TAG_X3D_NurbsSet, TAG_X3D_NurbsSurfaceInterpolator, TAG_X3D_OrientationInterpolator, TAG_X3D_PlaneSensor, TAG_X3D_PointLight, TAG_X3D_PositionInterpolator, TAG_X3D_PositionInterpolator2D, TAG_X3D_ProximitySensor, TAG_X3D_ReceiverPdu, TAG_X3D_ScalarInterpolator, TAG_X3D_Script, TAG_X3D_Shape, TAG_X3D_SignalPdu, TAG_X3D_Sound, TAG_X3D_SphereSensor, TAG_X3D_SpotLight, TAG_X3D_StaticGroup, TAG_X3D_StringSensor, TAG_X3D_Switch, TAG_X3D_TextureBackground, TAG_X3D_TimeSensor, TAG_X3D_TimeTrigger, TAG_X3D_TouchSensor, TAG_X3D_Transform, TAG_X3D_TransmitterPdu, TAG_X3D_Viewpoint, TAG_X3D_VisibilitySensor, TAG_X3D_WorldInfo
};

#define SF2DNode_X3D_Count	34
static const u32 SF2DNode_X3D_TypeToTag[34] = {
 TAG_X3D_Anchor, TAG_X3D_BooleanFilter, TAG_X3D_BooleanSequencer, TAG_X3D_BooleanToggle, TAG_X3D_BooleanTrigger, TAG_X3D_ColorInterpolator, TAG_X3D_CoordinateInterpolator2D, TAG_X3D_EspduTransform, TAG_X3D_GeoMetadata, TAG_X3D_GeoTouchSensor, TAG_X3D_Group, TAG_X3D_Inline, TAG_X3D_IntegerSequencer, TAG_X3D_IntegerTrigger, TAG_X3D_KeySensor, TAG_X3D_LOD, TAG_X3D_NurbsOrientationInterpolator, TAG_X3D_NurbsPositionInterpolator, TAG_X3D_NurbsSet, TAG_X3D_NurbsSurfaceInterpolator, TAG_X3D_PositionInterpolator2D, TAG_X3D_ReceiverPdu, TAG_X3D_ScalarInterpolator, TAG_X3D_Script, TAG_X3D_Shape, TAG_X3D_SignalPdu, TAG_X3D_StaticGroup, TAG_X3D_StringSensor, TAG_X3D_Switch, TAG_X3D_TimeSensor, TAG_X3D_TimeTrigger, TAG_X3D_TouchSensor, TAG_X3D_TransmitterPdu, TAG_X3D_WorldInfo
};

#define SFAppearanceNode_X3D_Count	1
static const u32 SFAppearanceNode_X3D_TypeToTag[1] = {
 TAG_X3D_Appearance
};

#define SFGeometryNode_X3D_Count	31
static const u32 SFGeometryNode_X3D_TypeToTag[31] = {
 TAG_X3D_Arc2D, TAG_X3D_ArcClose2D, TAG_X3D_Box, TAG_X3D_Circle2D, TAG_X3D_Cone, TAG_X3D_Cylinder, TAG_X3D_Disk2D, TAG_X3D_ElevationGrid, TAG_X3D_Extrusion, TAG_X3D_GeoElevationGrid, TAG_X3D_IndexedFaceSet, TAG_X3D_IndexedLineSet, TAG_X3D_IndexedTriangleFanSet, TAG_X3D_IndexedTriangleSet, TAG_X3D_IndexedTriangleStripSet, TAG_X3D_LineSet, TAG_X3D_NurbsCurve, TAG_X3D_NurbsPatchSurface, TAG_X3D_NurbsSweptSurface, TAG_X3D_NurbsSwungSurface, TAG_X3D_NurbsTrimmedSurface, TAG_X3D_PointSet, TAG_X3D_Polyline2D, TAG_X3D_Polypoint2D, TAG_X3D_Rectangle2D, TAG_X3D_Sphere, TAG_X3D_Text, TAG_X3D_TriangleFanSet, TAG_X3D_TriangleSet, TAG_X3D_TriangleSet2D, TAG_X3D_TriangleStripSet
};

#define SFAudioNode_X3D_Count	1
static const u32 SFAudioNode_X3D_TypeToTag[1] = {
 TAG_X3D_AudioClip
};

#define SFStreamingNode_X3D_Count	4
static const u32 SFStreamingNode_X3D_TypeToTag[4] = {
 TAG_X3D_AudioClip, TAG_X3D_Inline, TAG_X3D_LoadSensor, TAG_X3D_MovieTexture
};

#define SFBackground3DNode_X3D_Count	2
static const u32 SFBackground3DNode_X3D_TypeToTag[2] = {
 TAG_X3D_Background, TAG_X3D_TextureBackground
};

#define SFColorNode_X3D_Count	2
static const u32 SFColorNode_X3D_TypeToTag[2] = {
 TAG_X3D_Color, TAG_X3D_ColorRGBA
};

#define SFNurbsControlCurveNode_X3D_Count	3
static const u32 SFNurbsControlCurveNode_X3D_TypeToTag[3] = {
 TAG_X3D_Contour2D, TAG_X3D_ContourPolyline2D, TAG_X3D_NurbsCurve2D
};

#define SFCoordinateNode_X3D_Count	3
static const u32 SFCoordinateNode_X3D_TypeToTag[3] = {
 TAG_X3D_Coordinate, TAG_X3D_CoordinateDouble, TAG_X3D_GeoCoordinate
};

#define SFCoordinate2DNode_X3D_Count	1
static const u32 SFCoordinate2DNode_X3D_TypeToTag[1] = {
 TAG_X3D_Coordinate2D
};

#define SFFillPropertiesNode_X3D_Count	1
static const u32 SFFillPropertiesNode_X3D_TypeToTag[1] = {
 TAG_X3D_FillProperties
};

#define SFFogNode_X3D_Count	1
static const u32 SFFogNode_X3D_TypeToTag[1] = {
 TAG_X3D_Fog
};

#define SFFontStyleNode_X3D_Count	1
static const u32 SFFontStyleNode_X3D_TypeToTag[1] = {
 TAG_X3D_FontStyle
};

#define SFGeoOriginNode_X3D_Count	1
static const u32 SFGeoOriginNode_X3D_TypeToTag[1] = {
 TAG_X3D_GeoOrigin
};

#define SFViewpointNode_X3D_Count	2
static const u32 SFViewpointNode_X3D_TypeToTag[2] = {
 TAG_X3D_GeoViewpoint, TAG_X3D_Viewpoint
};

#define SFTopNode_X3D_Count	1
static const u32 SFTopNode_X3D_TypeToTag[1] = {
 TAG_X3D_Group
};

#define SFHAnimDisplacerNode_X3D_Count	1
static const u32 SFHAnimDisplacerNode_X3D_TypeToTag[1] = {
 TAG_X3D_HAnimDisplacer
};

#define SFHAnimNode_X3D_Count	3
static const u32 SFHAnimNode_X3D_TypeToTag[3] = {
 TAG_X3D_HAnimJoint, TAG_X3D_HAnimSegment, TAG_X3D_HAnimSite
};

#define SFTextureNode_X3D_Count	4
static const u32 SFTextureNode_X3D_TypeToTag[4] = {
 TAG_X3D_ImageTexture, TAG_X3D_MovieTexture, TAG_X3D_MultiTexture, TAG_X3D_PixelTexture
};

#define SFX3DLinePropertiesNode_X3D_Count	1
static const u32 SFX3DLinePropertiesNode_X3D_TypeToTag[1] = {
 TAG_X3D_LineProperties
};

#define SFMaterialNode_X3D_Count	1
static const u32 SFMaterialNode_X3D_TypeToTag[1] = {
 TAG_X3D_Material
};

#define SFMetadataNode_X3D_Count	5
static const u32 SFMetadataNode_X3D_TypeToTag[5] = {
 TAG_X3D_MetadataDouble, TAG_X3D_MetadataFloat, TAG_X3D_MetadataInteger, TAG_X3D_MetadataSet, TAG_X3D_MetadataString
};

#define SFTextureCoordinateNode_X3D_Count	4
static const u32 SFTextureCoordinateNode_X3D_TypeToTag[4] = {
 TAG_X3D_MultiTextureCoordinate, TAG_X3D_NurbsTextureCoordinate, TAG_X3D_TextureCoordinate, TAG_X3D_TextureCoordinateGenerator
};

#define SFTextureTransformNode_X3D_Count	2
static const u32 SFTextureTransformNode_X3D_TypeToTag[2] = {
 TAG_X3D_MultiTextureTransform, TAG_X3D_TextureTransform
};

#define SFNavigationInfoNode_X3D_Count	1
static const u32 SFNavigationInfoNode_X3D_TypeToTag[1] = {
 TAG_X3D_NavigationInfo
};

#define SFNormalNode_X3D_Count	1
static const u32 SFNormalNode_X3D_TypeToTag[1] = {
 TAG_X3D_Normal
};

#define SFNurbsCurveNode_X3D_Count	1
static const u32 SFNurbsCurveNode_X3D_TypeToTag[1] = {
 TAG_X3D_NurbsCurve
};

#define SFNurbsSurfaceNode_X3D_Count	4
static const u32 SFNurbsSurfaceNode_X3D_TypeToTag[4] = {
 TAG_X3D_NurbsPatchSurface, TAG_X3D_NurbsSweptSurface, TAG_X3D_NurbsSwungSurface, TAG_X3D_NurbsTrimmedSurface
};




Bool X3D_IsNodeInTable(u32 NDT_Tag, u32 NodeTag)
{
	const u32 *types;
	u32 count, i;
	if (!NodeTag) return 0;
	types = NULL; count = 0;
	switch (NDT_Tag) {
	case NDT_SFWorldNode:
		types = SFWorldNode_X3D_TypeToTag; count = SFWorldNode_X3D_Count; break;
	case NDT_SF3DNode:
		types = SF3DNode_X3D_TypeToTag; count = SF3DNode_X3D_Count; break;
	case NDT_SF2DNode:
		types = SF2DNode_X3D_TypeToTag; count = SF2DNode_X3D_Count; break;
	case NDT_SFAppearanceNode:
		types = SFAppearanceNode_X3D_TypeToTag; count = SFAppearanceNode_X3D_Count; break;
	case NDT_SFGeometryNode:
		types = SFGeometryNode_X3D_TypeToTag; count = SFGeometryNode_X3D_Count; break;
	case NDT_SFAudioNode:
		types = SFAudioNode_X3D_TypeToTag; count = SFAudioNode_X3D_Count; break;
	case NDT_SFStreamingNode:
		types = SFStreamingNode_X3D_TypeToTag; count = SFStreamingNode_X3D_Count; break;
	case NDT_SFBackground3DNode:
		types = SFBackground3DNode_X3D_TypeToTag; count = SFBackground3DNode_X3D_Count; break;
	case NDT_SFColorNode:
		types = SFColorNode_X3D_TypeToTag; count = SFColorNode_X3D_Count; break;
	case NDT_SFNurbsControlCurveNode:
		types = SFNurbsControlCurveNode_X3D_TypeToTag; count = SFNurbsControlCurveNode_X3D_Count; break;
	case NDT_SFCoordinateNode:
		types = SFCoordinateNode_X3D_TypeToTag; count = SFCoordinateNode_X3D_Count; break;
	case NDT_SFCoordinate2DNode:
		types = SFCoordinate2DNode_X3D_TypeToTag; count = SFCoordinate2DNode_X3D_Count; break;
	case NDT_SFFillPropertiesNode:
		types = SFFillPropertiesNode_X3D_TypeToTag; count = SFFillPropertiesNode_X3D_Count; break;
	case NDT_SFFogNode:
		types = SFFogNode_X3D_TypeToTag; count = SFFogNode_X3D_Count; break;
	case NDT_SFFontStyleNode:
		types = SFFontStyleNode_X3D_TypeToTag; count = SFFontStyleNode_X3D_Count; break;
	case NDT_SFGeoOriginNode:
		types = SFGeoOriginNode_X3D_TypeToTag; count = SFGeoOriginNode_X3D_Count; break;
	case NDT_SFViewpointNode:
		types = SFViewpointNode_X3D_TypeToTag; count = SFViewpointNode_X3D_Count; break;
	case NDT_SFTopNode:
		types = SFTopNode_X3D_TypeToTag; count = SFTopNode_X3D_Count; break;
	case NDT_SFHAnimDisplacerNode:
		types = SFHAnimDisplacerNode_X3D_TypeToTag; count = SFHAnimDisplacerNode_X3D_Count; break;
	case NDT_SFHAnimNode:
		types = SFHAnimNode_X3D_TypeToTag; count = SFHAnimNode_X3D_Count; break;
	case NDT_SFTextureNode:
		types = SFTextureNode_X3D_TypeToTag; count = SFTextureNode_X3D_Count; break;
	case NDT_SFX3DLinePropertiesNode:
		types = SFX3DLinePropertiesNode_X3D_TypeToTag; count = SFX3DLinePropertiesNode_X3D_Count; break;
	case NDT_SFMaterialNode:
		types = SFMaterialNode_X3D_TypeToTag; count = SFMaterialNode_X3D_Count; break;
	case NDT_SFMetadataNode:
		types = SFMetadataNode_X3D_TypeToTag; count = SFMetadataNode_X3D_Count; break;
	case NDT_SFTextureCoordinateNode:
		types = SFTextureCoordinateNode_X3D_TypeToTag; count = SFTextureCoordinateNode_X3D_Count; break;
	case NDT_SFTextureTransformNode:
		types = SFTextureTransformNode_X3D_TypeToTag; count = SFTextureTransformNode_X3D_Count; break;
	case NDT_SFNavigationInfoNode:
		types = SFNavigationInfoNode_X3D_TypeToTag; count = SFNavigationInfoNode_X3D_Count; break;
	case NDT_SFNormalNode:
		types = SFNormalNode_X3D_TypeToTag; count = SFNormalNode_X3D_Count; break;
	case NDT_SFNurbsCurveNode:
		types = SFNurbsCurveNode_X3D_TypeToTag; count = SFNurbsCurveNode_X3D_Count; break;
	case NDT_SFNurbsSurfaceNode:
		types = SFNurbsSurfaceNode_X3D_TypeToTag; count = SFNurbsSurfaceNode_X3D_Count; break;
	default:
		return 0;
	}
	for(i=0; i<count; i++) { if (types[i]==NodeTag) return 1;}
	return 0;
}
