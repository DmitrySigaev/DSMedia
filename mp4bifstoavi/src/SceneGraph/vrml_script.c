/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Graph sub-project
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

#include <intern/m4_scenegraph_dev.h>
/*MPEG4 & X3D tags (for node tables & script handling)*/
#include <m4_mpeg4_nodes.h>
#include <m4_x3d_nodes.h>


static u32 script_get_nb_static_field(SFNode *node) 
{
	return (node->sgprivate->tag==TAG_MPEG4_Script) ? 3 : 4;
}

Bool SG_HasScripting()
{
#ifdef M4_USE_SPIDERMONKEY
	return 1;
#else
	return 0;
#endif
}

void Script_PreDestroy(SFNode *node)
{
	ScriptPriv *priv;
	ScriptField *field;
	priv = node->sgprivate->privateStack;
	
	if (priv->JS_PreDestroy) priv->JS_PreDestroy(node);

	//destroy extra fields
	while (ChainGetCount(priv->fields)) {
		field = ChainGetEntry(priv->fields, 0);
		ChainDeleteEntry(priv->fields, 0);
		if (field->pField) {
			//if Node unregister
			switch (field->fieldType) {
			//specific case for SFNode in script
			case FT_SFNode:
				Node_Unregister((SFNode *) field->pField, node);
				break;
			case FT_MFNode:
				Node_UnregisterChildren(node, (Chain*) field->pField);
				DeleteChain((Chain*)field->pField);
				break;
			default:
				VRML_DeleteFieldPointer(field->pField, field->fieldType);
				break;
			}
		}
		if (field->name) free(field->name);
		free(field);
	}
	DeleteChain(priv->fields);
	free(priv);
}

u32 Script_GetNumFields(SFNode *node, u8 IndexMode)
{
	u32 nb_static;
	ScriptPriv *priv = node->sgprivate->privateStack;
	switch (IndexMode) {
	case FCM_IN:
		return priv->numIn;
	case FCM_OUT:
		return priv->numOut;
	case FCM_DEF:
		return priv->numDef;
	case FCM_DYN:
		return 0;
	default:
		nb_static = script_get_nb_static_field(node);
		return priv ? ChainGetCount(priv->fields) + nb_static : nb_static;
	}
}

M4Err Script_GetFieldIndex(SFNode *node, u32 inField, u8 IndexMode, u32 *allField)
{
	u32 i;
	u32 nb_static = script_get_nb_static_field(node);

	ScriptPriv *priv = node->sgprivate->privateStack;
	for (i=0; i<ChainGetCount(priv->fields); i++) {
		ScriptField *sf = ChainGetEntry(priv->fields, i);
		*allField = i+nb_static;
		switch (IndexMode) {
		case FCM_IN:
			if ((u32)sf->IN_index==inField) return M4OK;
			break;
		case FCM_DEF:
			if ((u32)sf->DEF_index==inField) return M4OK;
			break;
		case FCM_OUT:
			if ((u32)sf->OUT_index==inField) return M4OK;
			break;
		case FCM_DYN:
			return M4BadParam;
		default:
			if (inField==i+nb_static) return M4OK;
			break;
		}
	}
	/*try with default*/
#ifdef NODE_USE_POINTERS
	return priv->Script_GetFieldIndex(node, inField, IndexMode, allField);
#else
	return MPEG4Node_GetFieldIndex(node, inField, IndexMode, allField);
#endif
}


M4Err Script_GetField(SFNode *node, FieldInfo *info)
{
	ScriptField *field;
	ScriptPriv *priv;
	u32 nb_static;

	if (!info || !node) return M4BadParam;

	priv = Node_GetPrivate(node);
	nb_static = script_get_nb_static_field(node);

	//static fields
	if (info->fieldIndex < nb_static) {
#ifdef NODE_USE_POINTERS
		return priv->Script_GetField(node, info);
#else
		if (nb_static==3) return MPEG4Node_GetField(node, info);
		return X3DNode_GetField(node, info);
#endif
	}

	//dyn fields
	field = ChainGetEntry(priv->fields, info->fieldIndex - nb_static);
	if (!field) return M4BadParam;

	info->eventType = field->eventType;
	info->fieldType = field->fieldType;
	info->name = field->name;
	//we need the eventIn name to activate the function...
	info->on_event_in = NULL;

	//setup pointer (special cases for nodes)
	switch (field->fieldType) {
	case FT_SFNode:
	case FT_MFNode:
		info->far_ptr = &field->pField;
		info->NDTtype = NDT_SFWorldNode;
		break;
	default:
		info->far_ptr = field->pField;
		break;
	}
	return M4OK;
}



void Script_Init(SFNode *node)
{
	ScriptPriv *priv;


	SAFEALLOC(priv, sizeof(ScriptPriv));
	priv->fields = NewChain();

	Node_SetPrivate(node, priv);
	node->sgprivate->PreDestroyNode = Script_PreDestroy;

#ifdef NODE_USE_POINTERS
	/*store original table and provide replacement */
	priv->Script_GetField = node->sgprivate->get_field;
	node->sgprivate->get_field = Script_GetField;
	node->sgprivate->get_field_count = Script_GetNumFields;
#endif

	//URL is exposedField (in, out Def)
	priv->numDef = priv->numIn = priv->numOut = script_get_nb_static_field(node) - 2;
	//directOutput and mustEvaluate are fields (def)
	priv->numDef += 2;
}


LPSCRIPTFIELD SG_NewScriptField(SFNode *node, u32 eventType, u32 fieldType, const char *name)
{
	ScriptPriv *priv;
	ScriptField *field;
	if (!name || ((node->sgprivate->tag != TAG_MPEG4_Script) && (node->sgprivate->tag != TAG_X3D_Script)))
		return NULL;

	if (eventType > SFET_EventOut) return NULL;
	priv = Node_GetPrivate(node);

	SAFEALLOC(field, sizeof(ScriptField));
	field->fieldType = fieldType;
	field->name = strdup(name);

	field->DEF_index = field->IN_index = field->OUT_index = -1;
	switch (eventType) {
	case SFET_Field:
		field->DEF_index = priv->numDef;
		priv->numDef++;
		field->eventType = ET_Field;
		break;
	case SFET_EventIn:
		field->IN_index = priv->numIn;
		priv->numIn++;
		field->eventType = ET_EventIn;
		break;
	case SFET_EventOut:
		field->OUT_index = priv->numOut;
		field->eventType = ET_EventOut;
		priv->numOut++;
		break;
	}
	//+ static fields
	field->ALL_index = script_get_nb_static_field(node) + ChainGetCount(priv->fields);
	ChainAddEntry(priv->fields, field);

	//create field entry
	if (fieldType != FT_SFNode) {
		field->pField = VRML_NewFieldPointer(fieldType);
	}
	
	return field;
}


M4Err Script_PrepareClone(SFNode *dest, SFNode *orig)
{
	u32 i, type;
	ScriptPriv *dest_priv, *orig_priv;
	orig_priv = orig->sgprivate->privateStack;
	dest_priv = dest->sgprivate->privateStack;
	if (!orig_priv || !dest_priv) return M4BadParam;

	for (i=0; i<ChainGetCount(orig_priv->fields); i++) {
		ScriptField *sf = ChainGetEntry(orig_priv->fields, i);
		switch (sf->eventType) {
		case ET_EventIn:
			type = SFET_EventIn;
			break;
		case ET_EventOut:
			type = SFET_EventOut;
			break;
		case ET_Field:
			type = SFET_Field;
			break;
		default:
			return M4BadParam;
		}
		SG_NewScriptField(dest, type, sf->fieldType, sf->name);
	}
	return M4OK;
}

M4Err ScriptField_GetInfo(LPSCRIPTFIELD field, FieldInfo *info)
{
	if (!field || !info) return M4BadParam;
	memset(info, 0, sizeof(FieldInfo));

	info->fieldIndex = field->ALL_index;
	info->eventType = field->eventType;
	info->fieldType = field->fieldType;
	info->name = field->name;

	//setup pointer (special cases for nodes)
	switch (field->fieldType) {
	case FT_SFNode:
	case FT_MFNode:
		info->far_ptr = &field->pField;
		info->NDTtype = NDT_SFWorldNode;
		break;
	default:
		info->far_ptr = field->pField;
		break;
	}
	return M4OK;
}

void Script_EventIn(SFNode *node, FieldInfo *in_field)
{
	ScriptPriv *priv = node->sgprivate->privateStack;
	if (priv->JS_EventIn) priv->JS_EventIn(node, in_field);
}

