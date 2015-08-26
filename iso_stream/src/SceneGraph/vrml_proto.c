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

/*proto rendering is just a bypass to call the rendering node*/
void RenderProtoInstance(SFNode *node, void *renderstack)
{
	ProtoInstance *inst;
	assert(node->sgprivate->tag == TAG_ProtoNode);
	inst = (ProtoInstance *) node;
	if (inst->RenderingNode) inst->RenderingNode->sgprivate->RenderNode(inst->RenderingNode, renderstack);
}

LPPROTO SG_NewProto(LPSCENEGRAPH inScene, u32 ProtoID, char *name, Bool unregistered)
{
	PrototypeNode *tmp;
	if (!inScene) return NULL;

	/*make sure we don't define a proto already defined in this scope*/
	if (!unregistered) {
		tmp = SG_FindProto(inScene, ProtoID, name);
		if (tmp) return NULL;
	}

	SAFEALLOC(tmp, sizeof(PrototypeNode));
	if (!tmp) return NULL;

	tmp->proto_fields = NewChain();
	tmp->node_code = NewChain();
	tmp->parent_graph = inScene;
	tmp->sub_graph = SG_NewSubScene(inScene);
	tmp->instances = NewChain();

	if (name) 
		tmp->Name = strdup(name);
	else
		tmp->Name = strdup("Unnamed Proto");
	tmp->ID = ProtoID;
	if (!unregistered) {
		ChainAddEntry(inScene->protos, tmp);
	} else {
		ChainAddEntry(inScene->unregistered_protos, tmp);
	}
	return tmp;
}


M4Err Proto_SetInGraph(LPPROTO proto, LPSCENEGRAPH inScene, Bool set_in)
{
	u32 i;
	LPPROTO tmp;
	Chain *removeFrom;
	Chain *insertIn;

	if (set_in) {
		removeFrom = proto->parent_graph->unregistered_protos;
		insertIn = proto->parent_graph->protos;
	} else {
		insertIn = proto->parent_graph->unregistered_protos;
		removeFrom = proto->parent_graph->protos;
	}

	ChainDeleteItem(removeFrom, proto);

	for (i=0; i<ChainGetCount(insertIn); i++) {
		tmp = ChainGetEntry(insertIn, i);
		if (tmp==proto) return M4OK;
		if (!set_in) continue;
		/*if registering, make sure no other proto has the same ID/name*/
		if (tmp->ID==proto->ID) return M4BadParam;
		if (!stricmp(tmp->Name, proto->Name)) return M4BadParam;
	}
	return ChainAddEntry(insertIn, proto);
}


M4Err SG_DeleteProto(LPPROTO proto)
{
	SFNode *node;
	ProtoFieldInterface *field;
	s32 i;

	i = ChainDeleteItem(proto->parent_graph->protos, proto);
	if (i<0) i = ChainDeleteItem(proto->parent_graph->unregistered_protos, proto);

	if (proto->userpriv && proto->OnDelete) proto->OnDelete(proto->userpriv);

	/*first destroy the code*/
	while (ChainGetCount(proto->node_code)) {
		node = ChainGetEntry(proto->node_code, 0);
		Node_Unregister(node, NULL);
		ChainDeleteEntry(proto->node_code, 0);
	}
	DeleteChain(proto->node_code);

	/*delete sub graph*/
	SG_Delete(proto->sub_graph);


	/*delete interface*/
	while (ChainGetCount(proto->proto_fields)) {
		field = ChainGetEntry(proto->proto_fields, 0);
		if (field->userpriv && field->OnDelete) field->OnDelete(field->userpriv);

		if (field->default_value) 
			VRML_DeleteFieldPointer(field->default_value, field->FieldType);
	
		if (field->FieldName) free(field->FieldName);

		/*QP fields are SF fields, we can safely free() them*/
		if (field->qp_max_value) free(field->qp_max_value);
		if (field->qp_min_value) free(field->qp_min_value);
		free(field);
		ChainDeleteEntry(proto->proto_fields, 0);
	}
	DeleteChain(proto->proto_fields);

	if (proto->Name) free(proto->Name);
	MFURL_Del(proto->ExternProto);
	while (ChainGetCount(proto->instances)) {
		ProtoInstance *p = ChainGetEntry(proto->instances, 0);
		ChainDeleteEntry(proto->instances, 0);
		p->proto_interface = NULL;
	}
	DeleteChain(proto->instances);	
	free(proto);
	return M4OK;
}

LPSCENEGRAPH Proto_GetSceneGraph(LPPROTO proto)
{
	return proto ? proto->sub_graph : NULL;
}

void Proto_SetPrivate(LPPROTO p, void *ptr, void (*OnDelete)(void *ptr) )
{
	if (p) {
		p->userpriv = ptr;
		p->OnDelete = OnDelete;
	}
}
void *Proto_GetPrivate(LPPROTO p)
{
	return p ? p->userpriv : NULL;
}

MFURL *Proto_GetExternURLFieldPointer(LPPROTO proto)
{
	return proto ? &proto->ExternProto : NULL;
}

M4Err Proto_AddNodeCode(LPPROTO proto, SFNode *pNode)
{
	if (!proto) return M4BadParam;
	return ChainAddEntry(proto->node_code, pNode);
}

LPPROTOFIELD Proto_FindFieldByName(LPPROTO proto, char *fieldName)
{
	LPPROTOFIELD ptr;
	u32 i;
	for (i=0; i<ChainGetCount(proto->proto_fields); i++) {
		ptr = ChainGetEntry(proto->proto_fields, i);
		if (ptr->FieldName && !stricmp(ptr->FieldName, fieldName)) return ptr;
	}
	return NULL;
}

LPPROTOFIELD Proto_NewField(LPPROTO proto, u32 fieldType, u32 eventType, char *fieldName)
{
	ProtoFieldInterface *tmp;

	if (fieldName) {
		tmp = Proto_FindFieldByName(proto, fieldName);
		if (tmp) return NULL;
	}
	SAFEALLOC(tmp, sizeof(ProtoFieldInterface));
	if (!tmp) return NULL;

	tmp->FieldType = fieldType;
	tmp->EventType = eventType;
	
	/*create container - can be NULL if SF node*/
	tmp->default_value = VRML_NewFieldPointer(fieldType);
	
	if (fieldName) tmp->FieldName = strdup(fieldName);
	
	tmp->ALL_index = ChainGetCount(proto->proto_fields);
	tmp->OUT_index = tmp->DEF_index = tmp->IN_index = -1;

	switch (eventType) {
	case ET_ExposedField:
		tmp->IN_index = proto->NumIn;
		proto->NumIn ++;
		tmp->OUT_index = proto->NumOut;
		proto->NumOut ++;
	case ET_Field:
		tmp->DEF_index = proto->NumDef;
		proto->NumDef ++;
		break;
	case ET_EventIn:
		tmp->IN_index = proto->NumIn;
		proto->NumIn ++;
		break;
	case ET_EventOut:
		tmp->OUT_index = proto->NumOut;
		proto->NumOut ++;
		break;
	}

	ChainAddEntry(proto->proto_fields, tmp);
	return tmp;
}

void ProtoField_SetPrivate(LPPROTOFIELD field, void *ptr, void (*OnDelete)(void *ptr))
{
	if (field) {
		field->userpriv = ptr;
		field->OnDelete = OnDelete;
	}
}

void *ProtoField_GetPrivate(LPPROTOFIELD field)
{
	return field ? field->userpriv : NULL;
}



M4Err ProtoField_GetField(LPPROTOFIELD field, FieldInfo *info)
{
	if (!field || !info) return M4BadParam;
	memset(info, 0, sizeof(FieldInfo));
	info->fieldIndex = field->ALL_index;
	info->fieldType = field->FieldType;
	info->eventType = field->EventType;
	switch (field->FieldType) {
	case FT_SFNode:
	case FT_MFNode:
		info->far_ptr = &field->default_value;
		break;
	default:
		info->far_ptr = field->default_value;
		break;
	}
	info->name = field->FieldName;
	return M4OK;
}

M4Err Proto_GetField(PrototypeNode *proto, SFNode *node, FieldInfo *info)
{
	ProtoFieldInterface *proto_field;
	ProtoInstance *inst;
	ProtoField *field;

	if (!proto && !node) return M4BadParam;

	if (proto) {
		proto_field = ChainGetEntry(proto->proto_fields, info->fieldIndex);
		if (!proto_field) return M4BadParam;

		info->fieldType = proto_field->FieldType;
		info->eventType = proto_field->EventType;
		info->fieldIndex = proto_field->ALL_index;
		info->NDTtype = NDT_SFWorldNode;

		if (VRML_IsSFField(proto_field->FieldType)) {
			if (proto_field->FieldType==FT_SFNode) {
				info->far_ptr = &proto_field->default_value;
			} else {
				info->far_ptr = proto_field->default_value;
			}
		} else {
			if (proto_field->FieldType==FT_MFNode) {
				info->far_ptr = &proto_field->default_value;
			} else {
				info->far_ptr = proto_field->default_value;
			}
		}
		info->name = proto_field->FieldName;
		return M4OK;
	}

	/*otherwise this is an instanciated proto*/
	if (node->sgprivate->tag!=TAG_ProtoNode) return M4BadParam;

	inst = (ProtoInstance *) node;
	field = ChainGetEntry(inst->fields, info->fieldIndex);
	if (!field) return M4BadParam;

	info->fieldType = field->FieldType;
	info->eventType = field->EventType;
	/*SF/MF nodes need pointers to field object - cf Proto_CreateNode*/
	if (VRML_GetSFType(field->FieldType) == FT_SFNode) {
		info->far_ptr = &field->field_pointer;
	} else {
		info->far_ptr = field->field_pointer;
	}
	/*set the name - watchout for deletion case*/
	if (inst->proto_interface) {
		proto_field = ChainGetEntry(inst->proto_interface->proto_fields, info->fieldIndex);
		info->name = proto_field->FieldName;
	} else {
		info->name = "ProtoFieldDeleted";
	}
	info->NDTtype = NDT_SFWorldNode;
		
	return M4OK;
}

SFNode *SG_CloneNode(LPSCENEGRAPH inScene, SFNode *orig, SFNode *cloned_parent)
{
	u32 i, j, count;
	Bool is_script;
	SFNode *node, *child, *tmp;
	Chain *list, *list2;
	Route *r1, *r2;
	void BIFS_SetupConditionalClone(SFNode *node, SFNode *orig);
	ProtoInstance *proto;
	PrototypeNode *proto_node;
	FieldInfo field_orig, field;

	/*this is not a mistake*/
	if (!orig) return NULL;

	/*check for DEF/USE*/
	if (orig->sgprivate->NodeID) {
		node = SG_FindNode(inScene, orig->sgprivate->NodeID);
		/*node already created, USE*/
		if (node) {
			Node_Register(node, cloned_parent);
			return node;
		}
	}
	/*create a node*/
	if (orig->sgprivate->tag == TAG_ProtoNode) {
		proto_node = ((ProtoInstance *)orig)->proto_interface;
		/*create the instance but don't load the code -c we MUST wait for ISed routes to be cloned before*/
		node = Proto_CreateNode(inScene, proto_node, (ProtoInstance *) orig);
	} else {
		node = SG_NewNode(inScene, orig->sgprivate->tag);
	}

	count = Node_GetFieldCount(orig);

	is_script = 0;
	if ((orig->sgprivate->tag==TAG_MPEG4_Script) || (orig->sgprivate->tag==TAG_X3D_Script)) is_script = 1;
	if (is_script) Script_PrepareClone(node, orig);

	/*copy each field*/
	for (i=0; i<count; i++) {
		Node_GetField(orig, i, &field_orig);

		/*get target ptr*/
		Node_GetField(node, i, &field);

		assert(field.eventType==field_orig.eventType);
		assert(field.fieldType==field_orig.fieldType);

		/*duplicate it*/
		switch (field.fieldType) {
		case FT_SFNode:
			child = SG_CloneNode(inScene, (void *) (* ((SFNode **) field_orig.far_ptr)), node);
			*((SFNode **) field.far_ptr) = child;
			break;
		case FT_MFNode:
			list = *( (Chain **) field_orig.far_ptr);
			list2 = *( (Chain **) field.far_ptr);

			for (j=0; j<ChainGetCount(list); j++) {
				tmp = ChainGetEntry(list, j);
				child = SG_CloneNode(inScene, tmp, node);
				ChainAddEntry(list2, child);
			}
			break;
		case FT_SFTime:
			VRML_FieldCopy(field.far_ptr, field_orig.far_ptr, field.fieldType);
			if (!inScene->GetSceneTime) break;
			/*update SFTime that must be updated when cloning the node*/
			if (orig->sgprivate->tag == TAG_ProtoNode) {
				if (Proto_FieldIsSFTimeOffset(orig, &field_orig))
					*((SFTime *)field.far_ptr) += inScene->GetSceneTime(inScene->SceneCallback);
			} else if (!stricmp(field_orig.name, "startTime") || !stricmp(field_orig.name, "startTime") ) {
				*((SFTime *)field.far_ptr) += inScene->GetSceneTime(inScene->SceneCallback);
			}
			break;
		default:
			VRML_FieldCopy(field.far_ptr, field_orig.far_ptr, field.fieldType);
			break;
		}
	}
	/*register node*/
	if (orig->sgprivate->NodeID) {
		Node_SetDEF(node, orig->sgprivate->NodeID, orig->sgprivate->NodeName);
	}
	Node_Register(node, cloned_parent);

	/*init node before creating ISed routes so the eventIn handler are in place*/
	if (node->sgprivate->tag == TAG_MPEG4_Conditional) BIFS_SetupConditionalClone(node, orig);
	else 
		if (node->sgprivate->tag != TAG_ProtoNode) Node_Init(node);

	if (!inScene->pOwningProto) return node;
	proto = inScene->pOwningProto;
	
	/*create Routes for ISed fields*/
	for (i=0; i<ChainGetCount(proto->proto_interface->sub_graph->Routes); i++) {
		r1 = ChainGetEntry(proto->proto_interface->sub_graph->Routes, i);
		r2 = NULL;
		/*locate only ISed routes*/
		if (!r1->IS_route) continue;
		
		/*eventOut*/
		if (r1->FromNode == orig) {
			r2 = SG_NewRoute(inScene, node, r1->FromFieldIndex, (SFNode *) proto, r1->ToFieldIndex);
			r2->IS_route = 1;
		}
		/*eventIn or exposedField*/
		else if (r1->ToNode == orig) {
			r2 = SG_NewRoute(inScene, (SFNode *) proto, r1->FromFieldIndex, node, r1->ToFieldIndex);
			r2->IS_route = 1;
		}
		/*activate the route now so that proto instanciation works properly, otherwise
		we may load scripts with wrong field values*/		
		if (r2) ActivateRoute(r2);
	}
	
	/*remember scripts*/
	if (is_script) ChainAddEntry(proto->scripts_to_load, node);

	/*this is a proto node, init our internal stuff*/
	if (node->sgprivate->tag == TAG_ProtoNode) {
		node->sgprivate->PreDestroyNode = NULL;
		node->sgprivate->privateStack = NULL;
		node->sgprivate->RenderNode = RenderProtoInstance;
		/*load code*/
		proto_instanciate((ProtoInstance *)node);
	}
	return node;
}


#ifdef NODE_USE_POINTERS
static M4Err protoinst_get_field(SFNode *node, FieldInfo *info)
{
	info->NDTtype = NDT_SFWorldNode;
	return Proto_GetField(NULL, node, info);
}
static void protoinst_del(SFNode *n)
{
	Proto_DeleteInstance((ProtoInstance *)n);
}
#endif

M4Err protoinst_get_field_ind(SFNode *Node, u32 inField, u8 IndexMode, u32 *allField)
{
	return Proto_GetFieldIndex((ProtoInstance *)Node, inField, IndexMode, allField);
}


static Bool is_same_proto(PrototypeNode *p1, PrototypeNode *p2)
{
	u32 i, count;
	if (ChainGetCount(p1->proto_fields) != ChainGetCount(p2->proto_fields)) return 0;
	count = ChainGetCount(p1->proto_fields);
	for (i=0; i<count; i++) {
		ProtoFieldInterface *pf1 = ChainGetEntry(p1->proto_fields, i);
		ProtoFieldInterface *pf2 = ChainGetEntry(p2->proto_fields, i);
		if (pf1->EventType != pf2->EventType) return 0;
		if (pf1->FieldType != pf2->FieldType) return 0;
		/*note we don't check names since we're not sure both protos use name coding (MPEG4 only)*/
	}
	return 1;
}

static PrototypeNode *SG_FindProtoByInterface(LPSCENEGRAPH sg, PrototypeNode *the_proto)
{
	PrototypeNode *proto;
	u32 i;

	assert(sg);

	/*browse all top-level */
	for (i=0; i<ChainGetCount(sg->protos); i++) {
		proto = ChainGetEntry(sg->protos, i);
		if (is_same_proto(proto, the_proto)) return proto;
	}
	/*browse all top-level unregistered in reverse order*/
	for (i=ChainGetCount(sg->unregistered_protos); i>0; i--) {
		proto = ChainGetEntry(sg->unregistered_protos, i-1);
		if (is_same_proto(proto, the_proto)) return proto;
	}
	return NULL;
}
/*performs common initialization of routes ISed fields and protos once everything is loaded*/
void proto_instanciate(ProtoInstance *proto_node)
{
	SFNode *node, *orig;
	Route *route, *r2;
	u32 i;
	PrototypeNode *proto = proto_node->proto_interface;
	PrototypeNode *owner = proto;

	if (!proto) return;

	if (owner->ExternProto.count) {
		LPSCENEGRAPH extern_lib;
		if (!owner->parent_graph->GetExternProtoLib) return;
		extern_lib = owner->parent_graph->GetExternProtoLib(proto->parent_graph->SceneCallback, &owner->ExternProto);
		if (!extern_lib) return;

		/*this is an hardcoded proto - all routes, node modifications and co are handled internally*/
		if (extern_lib == SG_INTERNAL_PROTO) {
			owner->parent_graph->UserNodeInit(owner->parent_graph->NodeInitCallback, (SFNode *) proto_node);
			return;
		}
		/*not loaded yet*/
		if (!ChainGetCount(extern_lib->protos)) return;

		/*overwrite this proto by external one*/
		proto = NULL;
		/*start with proto v2 addressing*/
		if (owner->ExternProto.vals[0].url) {
			u32 ID = -1;
			char *szName = strrchr(owner->ExternProto.vals[0].url, '#');
			if (szName) {
				szName++;
				if (sscanf(szName, "%d", &ID)) ID = -1;
			}
			proto = SG_FindProto(extern_lib, ID, szName);
		}
		if (!proto) proto = SG_FindProto(extern_lib, owner->ID, NULL);
		if (!proto) proto = SG_FindProtoByInterface(extern_lib, owner);
		/*couldn't find proto in the given lib, consider the proto as loaded (give up)*/
		if (!proto) {
			proto_node->is_loaded = 1;
			return;
		}

		/*unregister from prev and reg with real proto*/
		ChainDeleteItem(owner->instances, proto_node);
		ChainAddEntry(proto->instances, proto_node);
	}

	/*OVERRIDE the proto instance (eg don't instanciate an empty externproto...)*/
	proto_node->proto_interface = proto;

	/*clone all nodes*/
	for (i=0; i<ChainGetCount(proto->node_code); i++) {
		orig = ChainGetEntry(proto->node_code, i);
		/*node is cloned in the new scenegraph and its parent is NULL */
		node = SG_CloneNode(proto_node->sgprivate->scenegraph, orig, NULL);
		assert(node);

		/*assign first rendering node*/
		if (!i) proto_node->RenderingNode = node;
		ChainAddEntry(proto_node->node_code, node);
	}

	/*instantiate routes (not ISed ones)*/
	for (i=0; i<ChainGetCount(proto->sub_graph->Routes); i++) {
		route = ChainGetEntry(proto->sub_graph->Routes, i);
		if (route->IS_route) continue;

		r2 = SG_NewRoute(proto_node->sgprivate->scenegraph, 
				SG_FindNode(proto_node->sgprivate->scenegraph, route->FromNode->sgprivate->NodeID), 
				route->FromFieldIndex, 
				SG_FindNode(proto_node->sgprivate->scenegraph, route->ToNode->sgprivate->NodeID), 
				route->ToFieldIndex);

		if (route->ID) SG_SetRouteID(r2, route->ID);
		if (route->name) SG_SetRouteName(r2, route->name);
	}
	/*activate all ISed fields so that inits on events is properly done*/
	for (i=0; i<ChainGetCount(proto_node->sgprivate->scenegraph->Routes); i++) {
		route = ChainGetEntry(proto_node->sgprivate->scenegraph->Routes, i);
		if (route->IS_route) ActivateRoute(route);
	}
	/*and load all scripts (this must be done once all fields are routed for the "initialize" method)*/
	while (ChainGetCount(proto_node->scripts_to_load)) {
		node = ChainGetEntry(proto_node->scripts_to_load, 0);
		ChainDeleteEntry(proto_node->scripts_to_load, 0);
		Script_Load(node);
	}
	/*re-activate all ISed fields pointing to scripts once scripts are loaded (eventIns)*/
	for (i=0; i<ChainGetCount(proto_node->sgprivate->scenegraph->Routes); i++) {
		route = ChainGetEntry(proto_node->sgprivate->scenegraph->Routes, i);
		if (!route->IS_route || !route->ToNode) continue;
/*		assert(route->is_setup);
		if ((route->FromField.eventType == ET_EventOut) || (route->FromField.eventType == ET_EventIn) ) continue;
*/		if ((route->ToNode->sgprivate->tag==TAG_MPEG4_Script) || (route->ToNode->sgprivate->tag==TAG_X3D_Script) )
			ActivateRoute(route);
	}

	proto_node->is_loaded = 1;
}

SFNode *Proto_CreateNode(LPSCENEGRAPH scene, PrototypeNode *proto, ProtoInstance *from_inst)
{
	u32 i;
	ProtoField *inst, *from_field;
	ProtoFieldInterface *field;

	ProtoInstance *proto_node;
	SAFEALLOC(proto_node, sizeof(ProtoInstance));
	if (!proto_node) return NULL;

	Node_Setup((SFNode *)proto_node, TAG_ProtoNode);
	proto_node->node_code = NewChain();
	proto_node->fields = NewChain();
	proto_node->scripts_to_load = NewChain();

	proto_node->proto_interface = proto;
	ChainAddEntry(proto->instances, proto_node);

#ifdef NODE_USE_POINTERS
	proto_node->sgprivate->node_del = protoinst_del;
	proto_node->sgprivate->get_field = protoinst_get_field;
	proto_node->sgprivate->get_field_count = Proto_GetNumFields;
	proto_node->sgprivate->name = strdup(proto->Name);
#else
	proto_node->proto_name = strdup(proto->Name);
#endif


	/*create the namespace*/
	proto_node->sgprivate->scenegraph = SG_NewSubScene(scene);
	/*set this proto as owner of the new graph*/
	proto_node->sgprivate->scenegraph->pOwningProto = proto_node;

	/*instanciate fields*/
	for (i=0; i<ChainGetCount(proto->proto_fields); i++) {
		field = ChainGetEntry(proto->proto_fields, i);
		inst = malloc(sizeof(ProtoField));
		inst->EventType = field->EventType;
		inst->FieldType = field->FieldType;

		/*this is OK to call on SFNode (returns NULL) and MFNode (returns NewChain() )*/
		inst->field_pointer = VRML_NewFieldPointer(inst->FieldType);

		/*regular field, duplicate from default value or instanciated one if specified (since
		a proto may be partially instanciated when used in another proto)*/
		if (VRML_GetSFType(inst->FieldType) != FT_SFNode) {
			if (from_inst) {
				from_field = ChainGetEntry(from_inst->fields, i);
				VRML_FieldCopy(inst->field_pointer, from_field->field_pointer, inst->FieldType);
			} else {
				VRML_FieldCopy(inst->field_pointer, field->default_value, inst->FieldType);
			}
		}
		/*No default values for SFNodes as interfaces ...*/
		ChainAddEntry(proto_node->fields, inst);
	}
	return (SFNode *) proto_node;
}



SFNode *Proto_CreateInstance(LPSCENEGRAPH sg, LPPROTO proto)
{
	return Proto_CreateNode(sg, proto, NULL);
}

M4Err Proto_LoadCode(SFNode *node)
{
	ProtoInstance *inst;
	if (node->sgprivate->tag != TAG_ProtoNode) return M4BadParam;
	inst = (ProtoInstance *) node;
	if (!inst->proto_interface) return M4BadParam;
	if (inst->is_loaded) return M4OK;
	proto_instanciate(inst);
	return M4OK;
}


u32 Proto_GetNumFields(SFNode *node, u8 code_mode)
{
	ProtoInstance *proto;
	if (!node) return 0;

	proto = (ProtoInstance *)node;
	/*watchout for deletion case*/
	switch (code_mode) {
	case FCM_IN: return proto->proto_interface ? proto->proto_interface->NumIn : 0;
	case FCM_OUT: return proto->proto_interface ? proto->proto_interface->NumOut : 0;
	case FCM_DEF: return proto->proto_interface ? proto->proto_interface->NumDef : 0;
	case FCM_ALL: return ChainGetCount(proto->proto_interface ? proto->proto_interface->proto_fields : proto->fields);
	/*BIFS-ANIM not supported*/
	case FCM_DYN:
	default:
		return 0;
	}
}


void Proto_DeleteInstance(ProtoInstance *inst)
{
	ProtoField *field;
	SFNode *node;
	u32 index;

	index = 0;
	while (ChainGetCount(inst->fields)) {
		field = ChainGetEntry(inst->fields, 0);
		ChainDeleteEntry(inst->fields, 0);

		/*regular type*/
		if ( (field->FieldType!=FT_SFNode) && (field->FieldType!=FT_MFNode)) {
			VRML_DeleteFieldPointer(field->field_pointer, field->FieldType);
		}
		/*node types: delete instances*/
		else if (field->field_pointer) {
			if (field->FieldType == FT_SFNode) {
				Node_Unregister((SFNode *) field->field_pointer, (SFNode *) inst);
			} else {
				Chain *list = (Chain *)field->field_pointer;
				while (ChainGetCount(list)) {
					SFNode *child = ChainGetEntry(list, 0);
					ChainDeleteEntry(list, 0);
					Node_Unregister(child, (SFNode *) inst);
				}
				DeleteChain(list);
			}
		}

		free(field);
		index++;
	}
	DeleteChain(inst->fields);

	/*destroy the code*/
	while (ChainGetCount(inst->node_code)) {
		node = ChainGetEntry(inst->node_code, 0);
		Node_Unregister(node, (SFNode*) inst);
		ChainDeleteEntry(inst->node_code, 0);
	}
	DeleteChain(inst->node_code);

	assert(!ChainGetCount(inst->scripts_to_load));
	DeleteChain(inst->scripts_to_load);

	if (inst->proto_interface) ChainDeleteItem(inst->proto_interface->instances, inst);

	SG_Delete(inst->sgprivate->scenegraph);

#ifdef NODE_USE_POINTERS
	/*this is duplicated for proto since a proto declaration may be destroyed while instances are active*/
	free((char *) inst->sgprivate->name);
#else
	free((char *) inst->proto_name);
#endif

	SFNode_Delete((SFNode *)inst);
}

/*Note on ISed fields: we cannot support fan-in on proto, eg we assume only one eventIn field can recieve events
thus situations where a proto recieves eventIn from outside and the node with ISed eventIn recieves event 
from inside the proto are undefined*/
M4Err Proto_SetISField(LPPROTO proto, u32 protoFieldIndex, SFNode *node, u32 nodeFieldIndex)
{
	M4Err e;
	Route *r;
	FieldInfo field, nodeField;
	field.fieldIndex = protoFieldIndex;
	e = Proto_GetField(proto, NULL, &field);
	if (e) return e;
	e = Node_GetField(node, nodeFieldIndex, &nodeField);
	if (e) return e;
	if (field.fieldType != nodeField.fieldType) {
		if ((VRML_GetSFType(field.fieldType)==FT_SFString) && (VRML_GetSFType(nodeField.fieldType) == FT_SFURL)) {
			e = M4OK;
		} else if ((VRML_GetSFType(field.fieldType)==FT_SFURL) && (VRML_GetSFType(nodeField.fieldType) == FT_SFString)) {
			e = M4OK;
		} else {
//			printf("error in IS - node field %s.%s - inType %s - outType %s\n", Node_GetName(node) , nodeField.name, VRML_GetFieldTypeName(field.fieldType), VRML_GetFieldTypeName(nodeField.fieldType));
			return M4InvalidProto;
		}
	}

	SAFEALLOC(r, sizeof(Route));
	if (!r) return M4OutOfMem;
	r->IS_route = 1;

	if (nodeField.eventType==ET_EventOut) {
		r->FromFieldIndex = nodeFieldIndex;
		r->FromNode = node;
		r->ToFieldIndex = protoFieldIndex;
		r->ToNode = NULL;
		ChainAddEntry(node->sgprivate->outRoutes, r);
	} else {
		switch (field.eventType) {
		case ET_Field:
		case ET_ExposedField:
		case ET_EventIn:
			r->FromFieldIndex = protoFieldIndex;
			r->FromNode = NULL;
			r->ToFieldIndex = nodeFieldIndex;
			r->ToNode = node;
			break;
		case ET_EventOut:
			r->FromFieldIndex = nodeFieldIndex;
			r->FromNode = node;
			r->ToFieldIndex = protoFieldIndex;
			r->ToNode = NULL;
			ChainAddEntry(node->sgprivate->outRoutes, r);
			break;
		default:
			free(r);
			return M4BadParam;
		}
	}
	r->graph = proto->sub_graph;
	return ChainAddEntry(proto->sub_graph->Routes, r);
}

M4Err ProtoInstance_SetISField(SFNode *protoinst, u32 protoFieldIndex, SFNode *node, u32 nodeFieldIndex)
{
	M4Err e;
	Route *r;
	FieldInfo field, nodeField;
	if (protoinst->sgprivate->tag != TAG_ProtoNode) return M4BadParam;
	
	e = Node_GetField(protoinst, protoFieldIndex, &field);
	if (e) return e;
	e = Node_GetField(node, nodeFieldIndex, &nodeField);
	if (e) return e;
	if (field.fieldType != nodeField.fieldType) {
		if ((VRML_GetSFType(field.fieldType)==FT_SFString) && (VRML_GetSFType(nodeField.fieldType) == FT_SFURL)) {
			e = M4OK;
		} else if ((VRML_GetSFType(field.fieldType)==FT_SFURL) && (VRML_GetSFType(nodeField.fieldType) == FT_SFString)) {
			e = M4OK;
		} else {
//			printf("error in IS - node field %s.%s - inType %s - outType %s\n", Node_GetName(node) , nodeField.name, VRML_GetFieldTypeName(field.fieldType), VRML_GetFieldTypeName(nodeField.fieldType));
			return M4InvalidProto;
		}
	}

	SAFEALLOC(r, sizeof(Route));
	if (!r) return M4OutOfMem;
	r->IS_route = 1;

	if (nodeField.eventType==ET_EventOut) {
		r->FromFieldIndex = nodeFieldIndex;
		r->FromNode = node;
		r->ToFieldIndex = protoFieldIndex;
		r->ToNode = protoinst;
		ChainAddEntry(node->sgprivate->outRoutes, r);
	} else {
		switch (field.eventType) {
		case ET_Field:
		case ET_ExposedField:
		case ET_EventIn:
			r->FromFieldIndex = protoFieldIndex;
			r->FromNode = protoinst;
			r->ToFieldIndex = nodeFieldIndex;
			r->ToNode = node;
			break;
		case ET_EventOut:
			r->FromFieldIndex = nodeFieldIndex;
			r->FromNode = node;
			r->ToFieldIndex = protoFieldIndex;
			r->ToNode = protoinst;
			ChainAddEntry(node->sgprivate->outRoutes, r);
			break;
		default:
			free(r);
			return M4BadParam;
		}
	}
	r->graph = node->sgprivate->scenegraph;
	ActivateRoute(r);
	return ChainAddEntry(r->graph->Routes, r);
}


M4Err ProtoField_SetQuantizationInfo(LPPROTOFIELD field, 
										u32 QP_Type, 
										u32 hasMinMax, 
										u32 QPSFType, 
										void *qp_min_value, 
										void *qp_max_value, 
										u32 QP13_NumBits)
{

	if (!field) return M4BadParam;
	if (!QP_Type) return M4OK;
	if (!VRML_IsSFField(QPSFType)) return M4BadParam;

	field->QP_Type = QP_Type;
	field->hasMinMax = hasMinMax;
	if (hasMinMax) {
		if (qp_min_value) {
			field->qp_min_value = VRML_NewFieldPointer(QPSFType);
			VRML_FieldCopy(field->qp_min_value, qp_min_value, QPSFType);
		}
		if (qp_max_value) {
			field->qp_max_value = VRML_NewFieldPointer(QPSFType);
			VRML_FieldCopy(field->qp_max_value, qp_max_value, QPSFType);
		}
	}
	field->NumBits = QP13_NumBits;
	return M4OK;
}


M4Err Proto_GetFieldIndex(ProtoInstance *proto, u32 index, u32 code_mode, u32 *all_index)
{
	u32 i;
	ProtoFieldInterface *proto_field;

	for (i=0;i<ChainGetCount(proto->proto_interface->proto_fields); i++) {
		proto_field = ChainGetEntry(proto->proto_interface->proto_fields, i);
		assert(proto_field);
		switch (code_mode) {
		case FCM_IN:
			if (proto_field->IN_index == index) {
				*all_index = proto_field->ALL_index;
				return M4OK;
			}
			break;
		case FCM_OUT:
			if (proto_field->OUT_index == index) {
				*all_index = proto_field->ALL_index;
				return M4OK;
			}
			break;
		case FCM_DEF:
			if (proto_field->DEF_index == index) {
				*all_index = proto_field->ALL_index;
				return M4OK;
			}
			break;
		case FCM_ALL:
			if (proto_field->ALL_index == index) {
				*all_index = proto_field->ALL_index;
				return M4OK;
			}
			break;
		/*BIFS-ANIM not supported*/
		case FCM_DYN:
		default:
			return M4BadParam;
		}		
	}
	return M4BadParam;
}

u32 Proto_GetFieldCount(LPPROTO proto)
{
	if (!proto) return 0;
	return ChainGetCount(proto->proto_fields);
}

LPPROTOFIELD Proto_FindField(LPPROTO proto, u32 fieldIndex)
{
	if (!proto) return NULL;
	return ChainGetEntry(proto->proto_fields, fieldIndex);
}

void Proto_CheckFieldChanged(SFNode *node, u32 fieldIndex)
{
	u32 i;

	LPROUTE r;
	if (!node) return;

	if (node->sgprivate->tag == TAG_ProtoNode) {
		for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
			r = ChainGetEntry(node->sgprivate->outRoutes, i);
			if (!r->IS_route) continue;
			/*eventIn or exposedField*/
			if ((r->FromNode == node) && (r->FromFieldIndex == fieldIndex) ) {
				if (ActivateRoute(r)) 
					SG_NodeChanged(r->ToNode, &r->FromField);
			}
			/*eventOut*/
			if ((r->ToNode == node) && (r->ToFieldIndex == fieldIndex) ) {
				ActivateRoute(r);
			}
		}
		/*no notification for proto changes*/
		return;
	}
	/*the node has to belong to a proto graph*/
	if (! node->sgprivate->scenegraph->pOwningProto) return;

	/*search for IS routes in the node and activate them. Field can also be an eventOut !!*/
	for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(node->sgprivate->outRoutes, i);
		if (!r->IS_route) continue;
		/*activate eventOuts*/
		if ((r->FromNode == node) && (r->FromFieldIndex == fieldIndex)) {
			ActivateRoute(r);
		}
		/*or eventIns / (exposed)Fields*/
		else if ((r->ToNode == node) && (r->ToFieldIndex == fieldIndex)) {
			/*don't forget to notify the node it has changed*/
			if (ActivateRoute(r)) 
				SG_NodeChanged(node, &r->ToField);
		}
	}
}


Bool Proto_GetAQInfo(SFNode *Node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits)
{
	PrototypeNode *proto;
	u32 i;
	ProtoFieldInterface *proto_field;

	proto = ((ProtoInstance *)Node)->proto_interface;

	for (i=0; i<ChainGetCount(proto->proto_fields); i++) {
		proto_field = ChainGetEntry(proto->proto_fields, i);
		if (proto_field->ALL_index!=FieldIndex) continue;
		
		*QType = proto_field->QP_Type;
		*AType = proto_field->Anim_Type;
		*b_min = M4_MIN_FLOAT;
		*b_max = M4_MAX_FLOAT;

		if (proto_field->hasMinMax) {

			/*translate our bounds*/
			switch (VRML_GetSFType(proto_field->FieldType)) {
			case FT_SFInt32:
				*b_min = (SFFloat) * ( (SFInt32 *) proto_field->qp_min_value);
				*b_max = (SFFloat) * ( (SFInt32 *) proto_field->qp_max_value);
				break;
			/*TO DO EVERYWHERE: check on field translation from double to float
			during quant bounds*/
			case FT_SFTime:
				*b_min = (SFFloat) * ( (SFTime *) proto_field->qp_min_value);
				*b_max = (SFFloat) * ( (SFTime *) proto_field->qp_max_value);
				break;
			default:
				if (proto_field->qp_min_value)
					*b_min = (SFFloat) * ( (SFFloat *) proto_field->qp_min_value);
				if (proto_field->qp_max_value)
					*b_max = (SFFloat) * ( (SFFloat *) proto_field->qp_max_value);
				break;
			}

		}
		*QT13_bits = proto_field->NumBits;
		return 1;
	}
	return 0;
}


LPPROTO Node_GetProto(SFNode *node)
{
	ProtoInstance *inst;
	if (node->sgprivate->tag != TAG_ProtoNode) return NULL;
	inst = (ProtoInstance *) node;
	return inst->proto_interface;
}

/*returns the ID of the proto*/
u32 Proto_GetID(LPPROTO proto)
{
	return proto->ID;
}

const char *Proto_GetName(LPPROTO proto)
{
	return (const char *) proto->Name;
}

u32 Proto_GetRenderingTag(LPPROTO proto)
{
	SFNode *n;
	if (!proto) return TAG_UndefinedNode;
	n = ChainGetEntry(proto->node_code, 0);
	if (!n) return TAG_UndefinedNode;
	if (n->sgprivate->tag == TAG_ProtoNode) return Proto_GetRenderingTag(((ProtoInstance *)n)->proto_interface);
	return n->sgprivate->tag;
}

Bool Proto_FieldIsSFTimeOffset(SFNode *node, FieldInfo *field)
{
	u32 i;
	ProtoInstance *inst;
	FieldInfo inf;
	if (node->sgprivate->tag != TAG_ProtoNode) return 0;
	if (field->fieldType != FT_SFTime) return 0;

	inst = (ProtoInstance *) node;
	/*check in interface if this is ISed */
	for (i=0; i<ChainGetCount(inst->proto_interface->sub_graph->Routes); i++) {
		LPROUTE r = ChainGetEntry(inst->proto_interface->sub_graph->Routes, i);
		if (!r->IS_route) continue;
		/*only check eventIn/field/exposedField*/
		if (r->FromNode || (r->FromFieldIndex != field->fieldIndex)) continue;

		Node_GetField(r->ToNode, r->ToFieldIndex, &inf);
		/*IS to another proto*/
		if (r->ToNode->sgprivate->tag == TAG_ProtoNode) return Proto_FieldIsSFTimeOffset(r->ToNode, &inf);
		/*IS to a startTime/stopTime field*/
		if (!stricmp(inf.name, "startTime") || !stricmp(inf.name, "stopTime")) return 1;
	}
	return 0;
}

LPSCENEGRAPH Node_GetExternProtoScene(SFNode *node)
{
	LPSCENEGRAPH sg;
	sg = node->sgprivate->scenegraph;
	if (!sg->pOwningProto) return NULL;
	if (!sg->pOwningProto->proto_interface->ExternProto.count) return NULL;
	sg = sg->pOwningProto->proto_interface->parent_graph;
	while (sg->parent_scene) sg = sg->parent_scene;
	return sg;
}

