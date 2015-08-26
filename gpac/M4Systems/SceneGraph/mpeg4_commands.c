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


#include <gpac/intern/m4_scenegraph_dev.h>
/*MPEG4 tags (for internal nodes)*/
#include <gpac/m4_mpeg4_nodes.h>



SGCommand *SG_NewCommand(LPSCENEGRAPH graph, u32 tag)
{
	SGCommand *ptr;
	SAFEALLOC(ptr, sizeof(SGCommand));
	if (!ptr) return NULL;
	ptr->tag = tag;
	ptr->in_scene = graph;
	ptr->new_proto_list = NewChain();
	ptr->command_fields = NewChain();
	return ptr;
}
static void SG_CheckNodeUnregister(SGCommand *com)
{
	if (com->tag==SG_SceneReplace) {
		Node_Unregister(com->node, NULL);
	} else {
		u32 i;
		for (i=0; i<com->in_scene->node_reg_size; i++) {
			if (com->in_scene->node_registry[i] == com->node) {
				Node_Unregister(com->node, NULL);
				return;
			}
		}
	}
}

void SG_DeleteCommand(SGCommand *com)
{
	u32 i;
	if (!com) return;

	while (ChainGetCount(com->command_fields)) {
		CommandFieldInfo *inf = ChainGetEntry(com->command_fields, 0);
		ChainDeleteEntry(com->command_fields, 0);

		switch (inf->fieldType) {
		case FT_SFNode:
			if (inf->field_ptr && *(SFNode **) inf->field_ptr) Node_Unregister(*(SFNode **) inf->field_ptr, com->node);
			break;
		case FT_MFNode:
			Node_UnregisterChildren(com->node, *(Chain**) inf->field_ptr);
			DeleteChain(*(Chain**) inf->field_ptr);
			break;
		default:
			VRML_DeleteFieldPointer(inf->field_ptr, inf->fieldType);
			break;
		}
		free(inf);
	}
	DeleteChain(com->command_fields);

	for (i=0; i<ChainGetCount(com->new_proto_list); i++) {
		LPPROTO proto = ChainGetEntry(com->new_proto_list, i);
		SG_DeleteProto(proto);
	}
	DeleteChain(com->new_proto_list);

	if (com->node) {
		if (!com->in_scene) Node_Unregister(com->node, NULL);
		else SG_CheckNodeUnregister(com);
	}

	if (com->del_proto_list) free(com->del_proto_list);
	if (com->def_name) free(com->def_name);
	if (com->scripts_to_load) DeleteChain(com->scripts_to_load);
	free(com);
}

static void SG_CheckFieldChange(SFNode *node, FieldInfo *field)
{
	/*and propagate eventIn if any*/
	if (field->on_event_in) {
		field->on_event_in(node);
	} else if ((field->eventType==ET_EventIn) && (Node_GetTag(node) == TAG_MPEG4_Script)) {
		Script_EventIn(node, field);
	} else {
		/*Notify eventOut in all cases to handle protos*/
		Node_OnEventOut(node, field->fieldIndex);
	}
	/*signal node modif*/
	SG_NodeChanged(node, field);
}

M4Err SG_ApplyCommand(LPSCENEGRAPH graph, SGCommand *com, Double time_offset)
{
	M4Err e;
	CommandFieldInfo *inf;
	FieldInfo field;
	SFNode *def, *node;
	void *slot_ptr;

	if (!com || !graph) return M4BadParam;

	e = M4OK;
	switch (com->tag) {
	case SG_SceneReplace:
		/*unregister root*/
		Node_Unregister(graph->RootNode, NULL);
		/*remove all protos and routes*/
		while (ChainGetCount(graph->routes_to_activate)) 
			ChainDeleteEntry(graph->routes_to_activate, 0);
		
		/*destroy all routes*/
		while (ChainGetCount(graph->Routes)) {
			Route *r = ChainGetEntry(graph->Routes, 0);
			/*this will unregister the route from the graph, so don't delete the chain entry*/
			SG_DeleteRoute(r);
		}
		/*destroy all proto*/
		while (ChainGetCount(graph->protos)) {
			PrototypeNode *p = ChainGetEntry(graph->protos, 0);
			/*this will unregister the proto from the graph, so don't delete the chain entry*/
			SG_DeleteProto(p);
		}
		/*DO NOT TOUCH node registry*/
		/*DO NOT TOUCH UNREGISTERED PROTOS*/

		/*move all protos in graph*/
		while (ChainGetCount(com->new_proto_list)) {
			PrototypeNode *p = ChainGetEntry(com->new_proto_list, 0);
			ChainDeleteEntry(com->new_proto_list, 0);
			ChainDeleteItem(graph->unregistered_protos, p);
			ChainAddEntry(graph->protos, p);
		}
		/*assign new root (no need to register/unregister)*/
		graph->RootNode = com->node;
		com->node = NULL;
		break;

	case SG_NodeReplace:
		if (!ChainGetCount(com->command_fields)) return M4OK;
		inf = ChainGetEntry(com->command_fields, 0);
		e = Node_ReplaceAllInstances(com->node, inf->new_node, 0);
		if (inf->new_node) Node_Register(inf->new_node, NULL);
		break;

	case SG_MultipleReplace:
	case SG_FieldReplace:
	{
		u32 i, j;
		Chain *container, *list;
		for (j=0; j<ChainGetCount(com->command_fields); j++) {
			inf = ChainGetEntry(com->command_fields, j);

			e = Node_GetField(com->node, inf->fieldIndex, &field);
			if (e) return e;

			switch (field.fieldType) {
			case FT_SFNode:
			{
				node = *((SFNode **) field.far_ptr);
				e = Node_Unregister(node, com->node);
				*((SFNode **) field.far_ptr) = inf->new_node;
				if (!e) Node_Register(inf->new_node, com->node);
				break;
			}
			case FT_MFNode:
				container = * ((Chain **) field.far_ptr);
				list = * ((Chain **) inf->field_ptr);
				Node_UnregisterChildren(com->node, container);

				for (i=0; i<ChainGetCount(list); i++) {
					node = ChainGetEntry(list, i);
					ChainAddEntry(container, node);
					if (!e) Node_Register(node, com->node);
				}
				break;
			default:
				/*this is a regular field, reset it and clone - we cannot switch pointers since the
				original fields are NOT pointers*/
				if (!VRML_IsSFField(field.fieldType)) {
					e = VRML_MF_Reset(field.far_ptr, field.fieldType);
				}
				if (e) return e;
				VRML_FieldCopy(field.far_ptr, inf->field_ptr, field.fieldType);
				if (field.fieldType==FT_SFTime) *(SFTime *)field.far_ptr = *(SFTime *)field.far_ptr + time_offset;
				break;
			}
			SG_CheckFieldChange(com->node, &field);
		}
		break;
	}

	case SG_MultipleIndexedReplace:
	case SG_IndexedReplace:
	{
		u32 sftype, i;
		for (i=0; i<ChainGetCount(com->command_fields); i++) {
			inf = ChainGetEntry(com->command_fields, i);

			e = Node_GetField(com->node, inf->fieldIndex, &field);
			if (e) return e;

			/*if MFNode remove the child and set new node*/
			if (field.fieldType == FT_MFNode) {
				/*we must remove the node before in case the new node uses the same ID (not forbidden) and this
				command removes the last instance of the node with the same ID*/
				Node_ReplaceChild(com->node, *((Chain**) field.far_ptr), inf->pos, inf->new_node);
				Node_Register(inf->new_node, com->node);
			}
			/*erase the field item*/
			else {
				if ((inf->pos < 0) || ((u32) inf->pos >= ((GenMFField *) field.far_ptr)->count) ) {
					inf->pos = ((GenMFField *)field.far_ptr)->count - 1;
					/*may happen with text and default value*/
					if (inf->pos < 0) {
						inf->pos = 0;
						VRML_MF_Alloc(field.far_ptr, field.fieldType, 1);
					}
				}
				e = VRML_MF_GetItem(field.far_ptr, field.fieldType, & slot_ptr, inf->pos);
				if (e) return e;
				sftype = VRML_GetSFType(field.fieldType);
				VRML_FieldCopy(slot_ptr, inf->field_ptr, sftype);
				/*note we don't add time offset, since there's no MFTime*/
			}
			SG_CheckFieldChange(com->node, &field);
		}
		break;
	}
	case SG_RouteReplace:
	{
		LPROUTE r;
		char *name;
		r = SG_FindRoute(graph, com->RouteID);
		def = SG_FindNode(graph, com->fromNodeID);
		node = SG_FindNode(graph, com->toNodeID);
		if (!node || !def) return M4InvalidNode;
		name = NULL;
		if (r) {
			name = r->name;
			r->name = NULL;
			SG_DeleteRoute(r);
		}
		r = SG_NewRoute(graph, def, com->fromFieldIndex, node, com->toFieldIndex);
		SG_SetRouteID(r, com->RouteID);
		if (name) {
			SG_SetRouteName(r, name);
			free(name);
		}
		break;
	}
	case SG_NodeDeleteEx:
	case SG_NodeDelete:
	{
		if (com->node) Node_ReplaceAllInstances(com->node, NULL, (com->tag==SG_NodeDeleteEx) ? 1 : 0);
		break;
	}
	case SG_RouteDelete:
	{
		return SG_DeleteRouteByID(graph, com->RouteID);
	}
	case SG_IndexedDelete:
	{
		if (!ChainGetCount(com->command_fields)) return M4OK;
		inf = ChainGetEntry(com->command_fields, 0);

		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;
		if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

		/*then we need special handling in case of a node*/
		if (VRML_GetSFType(field.fieldType) == FT_SFNode) {
			e = Node_ReplaceChild(com->node, * ((Chain **) field.far_ptr), inf->pos, NULL);
		} else {
			if ((inf->pos < 0) || ((u32) inf->pos >= ((GenMFField *) field.far_ptr)->count) ) {
				inf->pos = ((GenMFField *)field.far_ptr)->count - 1;
			}
			/*this is a regular MFField, just remove the item (realloc)*/
			e = VRML_MF_Remove(field.far_ptr, field.fieldType, inf->pos);
		}
		/*deletion -> node has changed*/
		if (!e) SG_CheckFieldChange(com->node, &field);
		break;
	}
	case SG_NodeInsert:
	{
		if (!ChainGetCount(com->command_fields)) return M4OK;
		inf = ChainGetEntry(com->command_fields, 0);

		e = Node_InsertChild(com->node, inf->new_node, inf->pos);
		if (!e) Node_Register(inf->new_node, com->node);
		/*notify (children is the 3rd field, so 2 0-based)*/
		if (!e) Node_OnEventOut(com->node, 2);
		break;
	}
	case SG_RouteInsert:
	{
		LPROUTE r;
		def = SG_FindNode(graph, com->fromNodeID);
		node = SG_FindNode(graph, com->toNodeID);
		if (!node || !def) return M4InvalidNode;
		r = SG_NewRoute(graph, def, com->fromFieldIndex, node, com->toFieldIndex);
		if (com->RouteID) SG_SetRouteID(r, com->RouteID);
		if (com->def_name) {
			SG_SetRouteName(r, com->def_name);
			free(com->def_name);
			com->def_name = NULL;
		}
		break;
	}
	case SG_IndexedInsert:
	{
		u32 sftype;
		if (!ChainGetCount(com->command_fields)) return M4OK;
		inf = ChainGetEntry(com->command_fields, 0);
		e = Node_GetField(com->node, inf->fieldIndex, &field);
		if (e) return e;

		/*rescale the MFField and parse the SFField*/
		if (field.fieldType != FT_MFNode) {
			if (inf->pos == -1) {
				e = VRML_MF_Append(field.far_ptr, field.fieldType, & slot_ptr);
			} else {
				e = VRML_MF_Insert(field.far_ptr, field.fieldType, & slot_ptr, inf->pos);
			}
			if (e) return e;
			sftype = VRML_GetSFType(field.fieldType);
			VRML_FieldCopy(slot_ptr, inf->field_ptr, sftype);
		} else {
			if (inf->new_node) {
				e = InsertSFNode(field.far_ptr, inf->new_node, inf->pos);
				if (e) return e;
				Node_Register(inf->new_node, com->node);
			}
		}
		if (!e) SG_CheckFieldChange(com->node, &field);
		break;
	}
	case SG_ProtoInsert:
		/*destroy all proto*/
		while (ChainGetCount(com->new_proto_list)) {
			PrototypeNode *p = ChainGetEntry(com->new_proto_list, 0);
			ChainDeleteEntry(com->new_proto_list, 0);
			ChainDeleteItem(graph->unregistered_protos, p);
			ChainAddEntry(graph->protos, p);
		}
		return M4OK;
	case SG_ProtoDelete:
		{
			u32 i;
			for (i=0; i<com->del_proto_list_size; i++) {
				/*note this will check for unregistered protos, but since IDs are unique we are sure we will 
				not destroy an unregistered one*/
				PrototypeNode *proto = SG_FindProto(graph, com->del_proto_list[i], NULL);
				if (proto) SG_DeleteProto(proto);
			}
		}
		return M4OK;
	case SG_ProtoDeleteAll:
		/*destroy all proto*/
		while (ChainGetCount(graph->protos)) {
			PrototypeNode *p = ChainGetEntry(graph->protos, 0);
			ChainDeleteEntry(graph->protos, 0);
			/*this will unregister the proto from the graph, so don't delete the chain entry*/
			SG_DeleteProto(p);
		}
		/*DO NOT TOUCH UNREGISTERED PROTOS*/
		return M4OK;
	/*only used by BIFS*/
	case SG_GlobalQuantizer:
		return M4OK;
	default:
		return M4NotSupported;
	}
	if (e) return e;

	if (com->scripts_to_load) {
		while (ChainGetCount(com->scripts_to_load)) {
			SFNode *script = ChainGetEntry(com->scripts_to_load, 0);
			ChainDeleteEntry(com->scripts_to_load, 0);
			Script_Load(script);
		}
		DeleteChain(com->scripts_to_load);
		com->scripts_to_load = NULL;
	}
	return M4OK;
}

CommandFieldInfo *SG_NewFieldCommand(SGCommand *com)
{
	CommandFieldInfo *ptr;
	SAFEALLOC(ptr, sizeof(CommandFieldInfo));
	ChainAddEntry(com->command_fields, ptr);
	return ptr;
}


M4Err SG_ApplyCommandList(LPSCENEGRAPH graph, Chain *comList, Double time_offset)
{
	M4Err e;
	u32 i;
	for (i=0; i<ChainGetCount(comList); i++) {
		SGCommand *com = ChainGetEntry(comList, i);
		e = SG_ApplyCommand(graph, com, time_offset);
		if (e) return e;
	}
	return M4OK;
}

SGCommand *SG_CloneCommand(SGCommand *com, LPSCENEGRAPH inGraph)
{
	u32 i;
	SGCommand *dest;
	
	if (com->tag==SG_SceneReplace) return NULL;
	/*FIXME - to do*/
	if (ChainGetCount(com->new_proto_list)) return NULL;
	dest = SG_NewCommand(inGraph, com->tag);

	/*node the command applies to - may be NULL*/
	dest->node = SG_CloneNode(inGraph, com->node, NULL);
	/*route insert, replace and delete*/
	dest->RouteID = com->RouteID;
	if (com->def_name) dest->def_name = strdup(com->def_name);
	dest->fromNodeID = com->fromNodeID;
	dest->fromFieldIndex = com->fromFieldIndex;
	dest->toNodeID = com->toNodeID;
	dest->toFieldIndex = com->toFieldIndex;
	dest->del_proto_list_size = com->del_proto_list_size;
	if (com->del_proto_list_size) {
		dest->del_proto_list = malloc(sizeof(u32) * com->del_proto_list_size);
		memcpy(dest->del_proto_list, com->del_proto_list, sizeof(u32) * com->del_proto_list_size);
	}

	for (i=0; i<ChainGetCount(com->command_fields); i++) {
		CommandFieldInfo *fo = ChainGetEntry(com->command_fields, i);
		CommandFieldInfo *fd = SG_NewFieldCommand(dest);

		fd->fieldIndex = fo->fieldIndex;
		fd->fieldType = fo->fieldType;
		fd->pos = fo->pos;
		if (fo->field_ptr) {
			fd->field_ptr = VRML_NewFieldPointer(fd->fieldType);
			VRML_FieldCopy(fd->field_ptr, fo->field_ptr, fo->fieldType);
		}

		if (fo->new_node) {
			fd->new_node = SG_CloneNode(inGraph, fo->new_node, dest->node);
			fd->field_ptr = &fd->new_node;
		}
		if (fo->node_list) {
			u32 j;
			fd->node_list = NewChain();
			for (j=0; j<ChainGetCount(fo->node_list); j++) {
				SFNode *co = ChainGetEntry(fo->node_list, j);
				SFNode *cd = SG_CloneNode(inGraph, co, dest->node);
				ChainAddEntry(fd->node_list, cd);
			}
			fd->field_ptr = &fd->node_list;
		}
	}
	return dest;
}

