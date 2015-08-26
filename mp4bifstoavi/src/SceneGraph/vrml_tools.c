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

/*this is not a NodeReplace, thus only the given container is updated - pos is 0-based*/
M4Err Node_ReplaceChild(SFNode *node, Chain *container, s32 pos, SFNode *newNode)
{
	u32 count;
	SFNode *n;
	
	count = ChainGetCount(container);
	if (!count) return M4OK;
	/*last item*/
	if ( (pos == -1) || ((u32) pos >= count) ) pos = count - 1;
	n = ChainGetEntry(container, (u32) pos);

	/*delete node*/

	if (n) Node_Unregister(n, node);

	/*delete entry*/
	ChainDeleteEntry(container, (u32) pos);

	if (newNode) ChainInsertEntry(container, newNode, pos);
	return M4OK;
}

void SG_DestroyRoutes(LPSCENEGRAPH sg)
{
	while (ChainGetCount(sg->routes_to_destroy) ) {
		Route *r = ChainGetEntry(sg->routes_to_destroy, 0);
		ChainDeleteEntry(sg->routes_to_destroy, 0);
		RemoveActivatedRoute(sg, r);
		Route_Destroy(r);
	}
}


void QueueRoute(LPSCENEGRAPH sg, Route *r)
{
	u32 now;
	if (!sg) return;

	/*get the top level scene (that's the only reliable one regarding simulatioin tick)*/
	while (sg->parent_scene) sg = sg->parent_scene;
	/*a single route may not be activated more than once in a simulation tick*/
	now = 1 + sg->simulation_tick;
	if (r->lastActivateTime >= now) return;
	r->lastActivateTime = now;
	ChainAddEntry(sg->routes_to_activate, r);
}

/*activate all routes in the order they where triggered*/
void SG_ActivateRoutes(LPSCENEGRAPH sg)
{
	Route *r;
	SFNode *targ;
	if (!sg) return;

	sg->simulation_tick++;

	while (ChainGetCount(sg->routes_to_activate)) {
		r = ChainGetEntry(sg->routes_to_activate, 0);
		ChainDeleteEntry(sg->routes_to_activate, 0);
		if (r) {
			targ = r->ToNode;
			if (ActivateRoute(r))
				if (r->is_setup) SG_NodeChanged(targ, &r->ToField);
		}
	}
	SG_DestroyRoutes(sg);
}

void RemoveActivatedRoute(LPSCENEGRAPH sg, Route *r)
{
	/*get the top level scene*/
	while (sg->parent_scene) sg = sg->parent_scene;
	/*remove route from queue list*/
	ChainDeleteItem(sg->routes_to_activate, r);
}

static void Node_on_add_children(SFNode *node)
{
	SFNode *child;
	s32 i;
	FieldInfo field;
	VRMLParent *n = (VRMLParent *)node;

	
	Node_GetField(node, 2, &field);

	/*for each node in input*/
	while (ChainGetCount(n->addChildren)) {
		child = ChainGetEntry(n->addChildren, 0);
		/*nothing in VRML stops from adding twice the same node but we don't allow that*/
		i = ChainFindEntry(n->children, child);
		if (i<0) {
			Node_Register(child, node);
			ChainAddEntry(n->children, child);
		}
		ChainDeleteEntry(n->addChildren, 0);
		Node_Unregister(child, node);
	}
	/*signal children field is modified*/
	SG_NodeChanged(node, &field);
}

static void Node_on_remove_children(SFNode *node)
{
	SFNode *child;
	FieldInfo field;
	s32 i;
	VRMLParent *n = (VRMLParent *)node;
	Node_GetField(node, 2, &field);

	/*for each node in input*/
	while (ChainGetCount(n->removeChildren)) {
		child = ChainGetEntry(n->removeChildren, 0);
		/*remove from children*/
		i = ChainFindEntry(n->children, child);
		if (i>=0) {
			ChainDeleteEntry(n->children, i);
			Node_Unregister(child, node);
		}

		ChainDeleteEntry(n->removeChildren, 0);
		Node_Unregister(child, node);
	}
	/*signal children field is modified*/
	SG_NodeChanged(node, &field);
}

void SetupVRMLParent(SFNode *pNode)
{
	VRMLParent *par = (VRMLParent *)pNode;
	par->children = NewChain();
	par->addChildren = NewChain();
	par->on_addChildren = Node_on_add_children;
	par->removeChildren = NewChain();
	par->on_removeChildren = Node_on_remove_children;
	pNode->sgprivate->is_dirty |= SG_CHILD_DIRTY;
}

void DestroyVRMLParent(SFNode *pNode)
{
	VRMLParent *par = (VRMLParent *)pNode;
	NodeList_Delete(par->children, pNode);
	NodeList_Delete(par->addChildren, pNode);
	NodeList_Delete(par->removeChildren, pNode);
}

M4Err SG_DeleteAllProtos(LPSCENEGRAPH scene)
{
	if (!scene) return M4BadParam;
	while (ChainGetCount(scene->protos)) {
		PrototypeNode *p = ChainGetEntry(scene->protos, 0);
		SG_DeleteProto(p);
	}
	return M4OK;
}

void SG_SetProtoLoader(LPSCENEGRAPH scene, LPSCENEGRAPH (*GetExternProtoLib)(void *SceneCallback, MFURL *lib_url))
{
	if (!scene) return;
	scene->GetExternProtoLib = GetExternProtoLib;
}


u32 SG_GetNextAvailableRouteID(LPSCENEGRAPH sg) 
{
	u32 i, count;
	u32 ID = 0;

	if (!sg->max_defined_route_id) {
		count = ChainGetCount(sg->Routes);
		/*routes are not sorted*/
		for (i=0; i<count; i++) {
			LPROUTE r = ChainGetEntry(sg->Routes, i);
			if (ID<=r->ID) ID = r->ID;
		}
		return ID+1;
	} else {
		sg->max_defined_route_id++;
		return sg->max_defined_route_id;
	}
}

void SG_SetMaxDefinedRouteID(LPSCENEGRAPH sg, u32 ID)
{
	sg->max_defined_route_id = ID;
}

u32 SG_GetNextAvailableProtoID(LPSCENEGRAPH sg) 
{
	u32 i, count;
	u32 ID = 0;
	count = ChainGetCount(sg->protos);
	/*protos are not sorted*/
	for (i=0; i<count; i++) {
		LPPROTO p = ChainGetEntry(sg->protos, i);
		if (ID<=p->ID) ID = p->ID;
	}
	count = ChainGetCount(sg->unregistered_protos);
	for (i=0; i<count; i++) {
		LPPROTO p = ChainGetEntry(sg->unregistered_protos, i);
		if (ID<=p->ID) ID = p->ID;
	}
	return ID+1;
}

//adds a child in the children list
M4Err Node_InsertChild(SFNode *parent, SFNode *new_child, s32 Position)
{
	SFParent *node = (SFParent *) parent;
	if (Position == -1) {
		return ChainAddEntry(node->children, new_child);
	} else {
		return ChainInsertEntry(node->children, new_child, Position);
	}
}

/*for V4Studio...*/
M4Err Node_RemoveChild(SFNode *parent, SFNode *toremove_child) 
{
	s32 ind;
	SFParent *node = (SFParent *) parent;

	ind = ChainDeleteItem(node->children, toremove_child);
	if (ind<0) return M4BadParam;
	/*V4Studio doesn't handle DEF/USE properly yet...*/
	/*Node_Unregister(toremove_child, parent);*/
	return M4OK;
}

void Script_Load(SFNode *n)
{
	if (n && n->sgprivate->scenegraph->Script_Load) n->sgprivate->scenegraph->Script_Load(n);
}

PrototypeNode *SG_FindProto(LPSCENEGRAPH sg, u32 ProtoID, char *name)
{
	PrototypeNode *proto;
	u32 i;

	assert(sg);

	/*browse all top-level */
	for (i=0; i<ChainGetCount(sg->protos); i++) {
		proto = ChainGetEntry(sg->protos, i);
		/*first check on name if given, since parsers use this with ID=0*/
		if (name) {
			if (proto->Name && !stricmp(name, proto->Name)) return proto;
		} else if (proto->ID == ProtoID) return proto;
	}
	/*browse all top-level unregistered in reverse order*/
	for (i=ChainGetCount(sg->unregistered_protos); i>0; i--) {
		proto = ChainGetEntry(sg->unregistered_protos, i-1);
		if (name) {
			if (proto->Name && !stricmp(name, proto->Name)) return proto;
		} else if (proto->ID == ProtoID) return proto;
	}
	return NULL;
}



u32 Node_GetChildTable(SFNode *Node)
{
	assert(Node);
	return MPEG4Node_GetChildNDT(Node);
}


M4Err Node_GetFieldIndex(SFNode *Node, u32 inField, u8 IndexMode, u32 *allField)
{
	assert(Node);
	switch (Node->sgprivate->tag) {
	case TAG_ProtoNode:
		return protoinst_get_field_ind(Node, inField, IndexMode, allField);
	case TAG_MPEG4_Script: 
	case TAG_X3D_Script: 
		return Script_GetFieldIndex(Node, inField, IndexMode, allField);
	default: return MPEG4Node_GetFieldIndex(Node, inField, IndexMode, allField);
	}
}


/* QUANTIZATION AND BIFS_Anim Info */
Bool Node_GetAQInfo(SFNode *Node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits)
{
	switch (Node->sgprivate->tag) {
	case TAG_ProtoNode: return Proto_GetAQInfo(Node, FieldIndex, QType, AType, b_min, b_max, QT13_bits);
	default: return MPEG4Node_GetAQInfo(Node, FieldIndex, QType, AType, b_min, b_max, QT13_bits);
	}
}

SFBool *NewSFBool()
{
	SFBool *tmp = malloc(sizeof(SFBool));
	memset(tmp, 0, sizeof(SFBool));
	return tmp;
}
SFFloat *NewSFFloat()
{
	SFFloat *tmp = malloc(sizeof(SFFloat));
	memset(tmp, 0, sizeof(SFFloat));
	return tmp;
}
SFDouble *NewSFDouble()
{
	SFDouble *tmp = malloc(sizeof(SFDouble));
	memset(tmp, 0, sizeof(SFDouble));
	return tmp;
}
SFTime *NewSFTime()
{
	SFTime *tmp = malloc(sizeof(SFTime));
	memset(tmp, 0, sizeof(SFTime));
	return tmp;
}
SFInt32 *NewSFInt32()
{
	SFInt32 *tmp = malloc(sizeof(SFInt32));
	memset(tmp, 0, sizeof(SFInt32));
	return tmp;
}
SFString *NewSFString()
{
	SFString *tmp = malloc(sizeof(SFString));
	memset(tmp, 0, sizeof(SFString));
	return tmp;
}
SFVec3f *NewSFVec3f()
{
	SFVec3f *tmp = malloc(sizeof(SFVec3f));
	memset(tmp, 0, sizeof(SFVec3f));
	return tmp;
}
SFVec3d *NewSFVec3d()
{
	SFVec3d *tmp = malloc(sizeof(SFVec3d));
	memset(tmp, 0, sizeof(SFVec3d));
	return tmp;
}
SFVec2f *NewSFVec2f()
{
	SFVec2f *tmp = malloc(sizeof(SFVec2f));
	memset(tmp, 0, sizeof(SFVec2f));
	return tmp;
}
SFVec2d *NewSFVec2d()
{
	SFVec2d *tmp = malloc(sizeof(SFVec2d));
	memset(tmp, 0, sizeof(SFVec2d));
	return tmp;
}
SFColor *NewSFColor()
{
	SFColor *tmp = malloc(sizeof(SFColor));
	memset(tmp, 0, sizeof(SFColor));
	return tmp;
}
SFColorRGBA *NewSFColorRGBA()
{
	SFColorRGBA *tmp = malloc(sizeof(SFColorRGBA));
	memset(tmp, 0, sizeof(SFColorRGBA));
	return tmp;
}
SFRotation *NewSFRotation()
{
	SFRotation *tmp = malloc(sizeof(SFRotation));
	memset(tmp, 0, sizeof(SFRotation));
	return tmp;
}
SFImage *NewSFImage()
{
	SFImage *tmp = malloc(sizeof(SFImage));
	memset(tmp, 0, sizeof(SFImage));
	return tmp;
}
SFURL *NewSFURL()
{
	SFURL *tmp = malloc(sizeof(SFURL));
	memset(tmp, 0, sizeof(SFURL));
	return tmp;
}
SFCommandBuffer *NewSFCommandBuffer()
{
	SFCommandBuffer *tmp = malloc(sizeof(SFCommandBuffer));
	memset(tmp, 0, sizeof(SFCommandBuffer));
	tmp->commandList = NewChain();
	return tmp;
}

MFBool *NewMFBool()
{
	MFBool *tmp = malloc(sizeof(MFBool));
	memset(tmp, 0, sizeof(MFBool));
	return tmp;
}
MFFloat *NewMFFloat()
{
	MFFloat *tmp = malloc(sizeof(MFFloat));
	memset(tmp, 0, sizeof(MFFloat));
	return tmp;
}
MFDouble *NewMFDouble()
{
	MFDouble *tmp = malloc(sizeof(MFDouble));
	memset(tmp, 0, sizeof(MFDouble));
	return tmp;
}
MFTime *NewMFTime()
{
	MFTime *tmp = malloc(sizeof(MFTime));
	memset(tmp, 0, sizeof(MFTime));
	return tmp;
}
MFInt32 *NewMFInt32()
{
	MFInt32 *tmp = malloc(sizeof(MFInt32));
	memset(tmp, 0, sizeof(MFInt32));
	return tmp;
}
MFString *NewMFString()
{
	MFString *tmp = malloc(sizeof(MFString));
	memset(tmp, 0, sizeof(MFString));
	return tmp;
}
MFVec3f *NewMFVec3f()
{
	MFVec3f *tmp = malloc(sizeof(MFVec3f));
	memset(tmp, 0, sizeof(MFVec3f));
	return tmp;
}
MFVec3d *NewMFVec3d()
{
	MFVec3d *tmp = malloc(sizeof(MFVec3d));
	memset(tmp, 0, sizeof(MFVec3d));
	return tmp;
}
MFVec2f *NewMFVec2f()
{
	MFVec2f *tmp = malloc(sizeof(MFVec2f));
	memset(tmp, 0, sizeof(MFVec2f));
	return tmp;
}
MFVec2d *NewMFVec2d()
{
	MFVec2d *tmp = malloc(sizeof(MFVec2d));
	memset(tmp, 0, sizeof(MFVec2d));
	return tmp;
}
MFColor *NewMFColor()
{
	MFColor *tmp = malloc(sizeof(MFColor));
	memset(tmp, 0, sizeof(MFColor));
	return tmp;
}
MFColorRGBA *NewMFColorRGBA()
{
	MFColorRGBA *tmp = malloc(sizeof(MFColorRGBA));
	memset(tmp, 0, sizeof(MFColorRGBA));
	return tmp;
}
MFRotation *NewMFRotation()
{
	MFRotation *tmp = malloc(sizeof(MFRotation));
	memset(tmp, 0, sizeof(MFRotation));
	return tmp;
}
MFURL *NewMFURL()
{
	MFURL *tmp = malloc(sizeof(MFURL));
	memset(tmp, 0, sizeof(MFURL));
	return tmp;
}

void *VRML_NewFieldPointer(u32 FieldType) 
{
	switch (FieldType) {
	case FT_SFBool: return NewSFBool();
	case FT_SFFloat: return NewSFFloat();
	case FT_SFDouble: return NewSFDouble();
	case FT_SFTime: return NewSFTime();
	case FT_SFInt32: return NewSFInt32();
	case FT_SFString: return NewSFString();
	case FT_SFVec3f: return NewSFVec3f();
	case FT_SFVec2f: return NewSFVec2f();
	case FT_SFVec3d: return NewSFVec3d();
	case FT_SFVec2d: return NewSFVec2d();
	case FT_SFColor: return NewSFColor();
	case FT_SFColorRGBA: return NewSFColorRGBA();
	case FT_SFRotation: return NewSFRotation();
	case FT_SFImage: return NewSFImage();
	case FT_MFBool: return NewMFBool();
	case FT_MFFloat: return NewMFFloat();
	case FT_MFTime: return NewMFTime();
	case FT_MFInt32: return NewMFInt32();
	case FT_MFString: return NewMFString();
	case FT_MFVec3f: return NewMFVec3f();
	case FT_MFVec2f: return NewMFVec2f();
	case FT_MFVec3d: return NewMFVec3d();
	case FT_MFVec2d: return NewMFVec2d();
	case FT_MFColor: return NewMFColor();
	case FT_MFColorRGBA: return NewMFColorRGBA();
	case FT_MFRotation: return NewMFRotation();

	//used in proto and script 
	case FT_MFNode: 
	{
		return NewChain();
	}
	//used in commands
	case FT_SFCommandBuffer:
		return NewSFCommandBuffer();

	case FT_SFURL: 
		return NewSFURL();
	case FT_MFURL:
		return NewMFURL();
	}
	return NULL;
}

void MFInt32_Del(MFInt32 par) { free(par.vals); }
void MFInt_Del(MFInt par) { free(par.vals); }
void MFFloat_Del(MFFloat par) { free(par.vals); }
void MFDouble_Del(MFDouble par) { free(par.vals); }
void MFBool_Del(MFBool par) { free(par.vals); }
void MFColor_Del(MFColor par) { free(par.vals); }
void MFColorRGBA_Del(MFColorRGBA par) { free(par.vals); }
void MFRotation_Del(MFRotation par) { free(par.vals); }
void MFTime_Del(MFTime par) { free(par.vals); }
void MFVec2f_Del(MFVec2f par) { free(par.vals); }
void MFVec2d_Del(MFVec2d par) { free(par.vals); }
void MFVec3f_Del(MFVec3f par) { free(par.vals); }
void MFVec3d_Del(MFVec3d par) { free(par.vals); }
void SFImage_Del(SFImage im) { free(im.pixels); }
void SFURL_Del(SFURL url) { if (url.url) free(url.url); }
void SFString_Del(SFString par) { if (par.buffer) free(par.buffer); }

void MFString_Del(MFString par)
{
	u32 i;
	for (i=0; i<par.count; i++) {
		if (par.vals[i]) free(par.vals[i]);
	}
	free(par.vals);
}


void SFScript_Del(SFScript sc)
{
	if (sc.script_text) free(sc.script_text);
}

void SFCommandBuffer_Del(SFCommandBuffer cb)
{
	u32 i;
	for (i=ChainGetCount(cb.commandList); i>0; i--) {
		SGCommand *com = ChainGetEntry(cb.commandList, i-1);
		SG_DeleteCommand(com);
	}
	DeleteChain(cb.commandList);
	if (cb.buffer) free(cb.buffer);
}

void MFURL_Del(MFURL url)
{
	u32 i;
	for (i=0; i<url.count; i++) {
		SFURL_Del(url.vals[i]);
	}
	free(url.vals);
}
void MFScript_Del(MFScript sc)
{
	u32 i;
	for (i=0; i<sc.count; i++) {
		if (sc.vals[i].script_text) free(sc.vals[i].script_text);
	}
	free(sc.vals);
}


void VRML_DeleteFieldPointer(void *field, u32 FieldType) 
{
	SFNode *node;

	switch (FieldType) {
	case FT_SFBool: 
	case FT_SFFloat:
	case FT_SFDouble:
	case FT_SFTime: 
	case FT_SFInt32:
	case FT_SFVec3f:
	case FT_SFVec3d:
	case FT_SFVec2f:
	case FT_SFVec2d:
	case FT_SFColor:
	case FT_SFColorRGBA:
	case FT_SFRotation:
		break;
	case FT_SFString:
		if ( ((SFString *)field)->buffer) free(((SFString *)field)->buffer);
		break;
	case FT_SFImage:
		SFImage_Del(* ((SFImage *)field));
		break;

	case FT_SFNode: 
		node = *(SFNode **) field;
		if (node) DestroyNode(node);
		return;
	case FT_SFCommandBuffer:
		SFCommandBuffer_Del(*(SFCommandBuffer *)field);
		break;
	
	case FT_MFBool:
		MFBool_Del( * ((MFBool *) field));
		break;
	case FT_MFFloat: 
		MFFloat_Del( * ((MFFloat *) field));
		break;
	case FT_MFDouble: 
		MFDouble_Del( * ((MFDouble *) field));
		break;
	case FT_MFTime: 
		MFTime_Del( * ((MFTime *)field));
		break;
	case FT_MFInt32:
		MFInt32_Del( * ((MFInt32 *)field));
		break;
	case FT_MFString:
		MFString_Del( *((MFString *)field));
		break;
	case FT_MFVec3f:
		MFVec3f_Del( * ((MFVec3f *)field));
		break;
	case FT_MFVec2f:
		MFVec2f_Del( * ((MFVec2f *)field));
		break;
	case FT_MFVec3d:
		MFVec3d_Del( * ((MFVec3d *)field));
		break;
	case FT_MFVec2d:
		MFVec2d_Del( * ((MFVec2d *)field));
		break;
	case FT_MFColor:
		MFColor_Del( * ((MFColor *)field));
		break;
	case FT_MFColorRGBA:
		MFColorRGBA_Del( * ((MFColorRGBA *)field));
		break;
	case FT_MFRotation:
		MFRotation_Del( * ((MFRotation *)field));
		break;
	case FT_MFURL:
		MFURL_Del( * ((MFURL *) field));
		break;		
	//used only in proto since this field is created by default for regular nodes
	case FT_MFNode: 
		while (ChainGetCount((Chain *)field)) {
			node = ChainGetEntry((Chain *)field, 0);
			DestroyNode(node);
			ChainDeleteEntry((Chain *)field, 0);
		}
		DeleteChain((Chain *)field);
		return;

	default:
		assert(0);
		return;
	}
	//free pointer
	free(field);
}


Bool VRML_IsSFField(u32 FieldType)
{
	switch (FieldType) {
	case FT_SFBool:
	case FT_SFFloat:
	case FT_SFDouble:
	case FT_SFTime:
	case FT_SFInt32:
	case FT_SFString:
	case FT_SFVec3f:
	case FT_SFVec3d:
	case FT_SFVec2f:
	case FT_SFVec2d:
	case FT_SFColor:
	case FT_SFColorRGBA:
	case FT_SFRotation:
	case FT_SFImage:
	case FT_SFNode:
	case FT_SFURL:
	case FT_SFCommandBuffer:
		return 1;
	default:
		return 0;
	}
}

/*********************************************************************
		MF Fields manipulation (alloc, realloc, GetAt)
*********************************************************************/

//return the size of fixed fields (eg no buffer in the field)
u32 GetSFFieldSize(u32 FieldType)
{
	switch (FieldType) {
	case FT_SFBool:
	case FT_MFBool:
		return sizeof(SFBool);
	case FT_SFFloat:
	case FT_MFFloat:
		return sizeof(SFFloat);
	case FT_SFTime:
	case FT_MFTime:
		return sizeof(SFTime);
	case FT_SFDouble:
	case FT_MFDouble:
		return sizeof(SFDouble);
	case FT_SFInt32:
	case FT_MFInt32:
		return sizeof(SFInt32);
	case FT_SFVec3f:
	case FT_MFVec3f:
		return 3*sizeof(SFFloat);
	case FT_SFVec2f:
	case FT_MFVec2f:
		return 2*sizeof(SFFloat);
	case FT_SFVec3d:
	case FT_MFVec3d:
		return 3*sizeof(SFDouble);
	case FT_SFColor:
	case FT_MFColor:
		return 3*sizeof(SFFloat);
	case FT_SFColorRGBA:
	case FT_MFColorRGBA:
		return 4*sizeof(SFFloat);
	case FT_SFRotation:
	case FT_MFRotation:
		return 4*sizeof(SFFloat);

	//check if that works!!
	case FT_SFString:
	case FT_MFString:
		//ptr to char
		return sizeof(SFString);
	case FT_SFScript:
	case FT_MFScript:
		return sizeof(SFScript);
	case FT_SFURL:
	case FT_MFURL:
		return sizeof(SFURL);
	default:
		return 0;
	}
}

u32 VRML_GetSFType(u32 FieldType)
{
	switch (FieldType) {
	case FT_SFBool:
	case FT_MFBool:
		return FT_SFBool;
	case FT_SFFloat:
	case FT_MFFloat:
		return FT_SFFloat;
	case FT_SFDouble:
	case FT_MFDouble:
		return FT_SFDouble;
	case FT_SFTime:
	case FT_MFTime:
		return FT_SFTime;
	case FT_SFInt32:
	case FT_MFInt32:
		return FT_SFInt32;
	case FT_SFVec3f:
	case FT_MFVec3f:
		return FT_SFVec3f;
	case FT_SFVec2f:
	case FT_MFVec2f:
		return FT_SFVec2f;
	case FT_SFVec3d:
	case FT_MFVec3d:
		return FT_SFVec3d;
	case FT_SFVec2d:
	case FT_MFVec2d:
		return FT_SFVec2d;
	case FT_SFColor:
	case FT_MFColor:
		return FT_SFColor;
	case FT_SFColorRGBA:
	case FT_MFColorRGBA:
		return FT_SFColorRGBA;
	case FT_SFRotation:
	case FT_MFRotation:
		return FT_SFRotation;

	//check if that works!!
	case FT_SFString:
	case FT_MFString:
		//ptr to char
		return FT_SFString;
	case FT_SFScript:
	case FT_MFScript:
		return FT_SFScript;
	case FT_SFURL:
	case FT_MFURL:
		return FT_SFURL;
	case FT_SFNode:
	case FT_MFNode:
		return FT_SFNode;
	default:
		return FT_Unknown;
	}
}

const char *VRML_GetEventTypeName(u32 EventType, Bool forX3D)
{
	switch (EventType) {
	case ET_EventIn: return forX3D ? "inputOnly" : "eventIn";
	case ET_Field: return forX3D ? "initializeOnly" : "field";
	case ET_ExposedField: return forX3D ? "inputOutput" : "exposedField";
	case ET_EventOut: return forX3D ? "outputOnly" : "eventOut";
	default: return "unknownEvent";
	}
}

const char *VRML_GetFieldTypeName(u32 FieldType)
{

	switch (FieldType) {
	case FT_SFBool: return "SFBool";
	case FT_SFFloat: return "SFFloat";
	case FT_SFDouble: return "SFDouble";
	case FT_SFTime: return "SFTime";
	case FT_SFInt32: return "SFInt32";
	case FT_SFString: return "SFString";
	case FT_SFVec3f: return "SFVec3f";
	case FT_SFVec2f: return "SFVec2f";
	case FT_SFVec3d: return "SFVec3d";
	case FT_SFVec2d: return "SFVec2d";
	case FT_SFColor: return "SFColor";
	case FT_SFColorRGBA: return "SFColorRGBA";
	case FT_SFRotation: return "SFRotation";
	case FT_SFImage: return "SFImage";
	case FT_SFNode: return "SFNode";
	case FT_SFVec4f: return "SFVec4f";
	case FT_MFBool: return "MFBool";
	case FT_MFFloat: return "MFFloat";
	case FT_MFDouble: return "MFDouble";
	case FT_MFTime: return "MFTime";
	case FT_MFInt32: return "MFInt32";
	case FT_MFString: return "MFString";
	case FT_MFVec3f: return "MFVec3f";
	case FT_MFVec2f: return "MFVec2f";
	case FT_MFVec3d: return "MFVec3d";
	case FT_MFVec2d: return "MFVec2d";
	case FT_MFColor: return "MFColor";
	case FT_MFColorRGBA: return "MFColorRGBA";
	case FT_MFRotation: return "MFRotation";
	case FT_MFImage: return "MFImage";
	case FT_MFNode: return "MFNode";
	case FT_MFVec4f: return "MFVec4f";
	case FT_SFURL: return "SFURL";
	case FT_MFURL: return "MFURL";
	case FT_SFCommandBuffer: return "SFCommandBuffer";
	case FT_SFScript: return "SFScript";
	case FT_MFScript: return "MFScript";
	default: return "UnknownType";
	}
}

u32 GetFieldTypeByName(char *fieldType)
{
	if (!stricmp(fieldType, "SFBool")) return FT_SFBool;
	else if (!stricmp(fieldType, "SFFloat")) return FT_SFFloat;
	else if (!stricmp(fieldType, "SFDouble")) return FT_SFDouble;
	else if (!stricmp(fieldType, "SFTime")) return FT_SFTime;
	else if (!stricmp(fieldType, "SFInt32")) return FT_SFInt32;
	else if (!stricmp(fieldType, "SFString")) return FT_SFString;
	else if (!stricmp(fieldType, "SFVec2f")) return FT_SFVec2f;
	else if (!stricmp(fieldType, "SFVec3f")) return FT_SFVec3f;
	else if (!stricmp(fieldType, "SFVec2d")) return FT_SFVec2d;
	else if (!stricmp(fieldType, "SFVec3d")) return FT_SFVec3d;
	else if (!stricmp(fieldType, "SFColor")) return FT_SFColor;
	else if (!stricmp(fieldType, "SFColorRGBA")) return FT_SFColorRGBA;
	else if (!stricmp(fieldType, "SFRotation")) return FT_SFRotation;
	else if (!stricmp(fieldType, "SFImage")) return FT_SFImage;
	else if (!stricmp(fieldType, "SFNode")) return FT_SFNode;

	else if (!stricmp(fieldType, "MFBool")) return FT_MFBool;
	else if (!stricmp(fieldType, "MFFloat")) return FT_MFFloat;
	else if (!stricmp(fieldType, "MFDouble")) return FT_MFDouble;
	else if (!stricmp(fieldType, "MFTime")) return FT_MFTime;
	else if (!stricmp(fieldType, "MFInt32")) return FT_MFInt32;
	else if (!stricmp(fieldType, "MFString")) return FT_MFString;
	else if (!stricmp(fieldType, "MFVec2f")) return FT_MFVec2f;
	else if (!stricmp(fieldType, "MFVec3f")) return FT_MFVec3f;
	else if (!stricmp(fieldType, "MFVec2d")) return FT_MFVec2d;
	else if (!stricmp(fieldType, "MFVec3d")) return FT_MFVec3d;
	else if (!stricmp(fieldType, "MFColor")) return FT_MFColor;
	else if (!stricmp(fieldType, "MFColorRGBA")) return FT_MFColorRGBA;
	else if (!stricmp(fieldType, "MFRotation")) return FT_MFRotation;
	else if (!stricmp(fieldType, "MFImage")) return FT_MFImage;
	else if (!stricmp(fieldType, "MFNode")) return FT_MFNode;

	return FT_Unknown;
}

//
//	Insert (+alloc) an MFField with a specified position for insertion and sets the ptr to the 
//	newly created slot
//	!! Doesnt work for MFNodes
//	InsertAt is the 0-based index for the new slot
M4Err VRML_MF_Insert(void *mf, u32 FieldType, void **new_ptr, u32 InsertAt)
{
	char *buffer;
	u32 FieldSize, i, k;
	GenMFField *mffield = (GenMFField *)mf;

	if (VRML_IsSFField(FieldType)) return M4BadParam;
	if (FieldType == FT_MFNode) return M4BadParam;

	FieldSize = GetSFFieldSize(FieldType);
	
	//field we can't copy
	if (!FieldSize) return M4BadParam;
	
	//first item ever
	if (!mffield->count || !mffield->array) {
		if (mffield->array) free(mffield->array);
		mffield->array = malloc(sizeof(char)*FieldSize);
		memset(mffield->array, 0, sizeof(char)*FieldSize);
		mffield->count = 1;
		if (new_ptr) *new_ptr = mffield->array;
		return M4OK;
	}

	//alloc 1+itemCount
	buffer = malloc(sizeof(char)*(1+mffield->count)*FieldSize);

	//append at the end
	if (InsertAt >= mffield->count) {
		memcpy(buffer, mffield->array, mffield->count * FieldSize);
		memset(buffer + mffield->count * FieldSize, 0, FieldSize);
		if (new_ptr) *new_ptr = buffer + mffield->count * FieldSize;
		free(mffield->array);
		mffield->array = buffer;
		mffield->count += 1;
		return M4OK;
	}
	//insert in the array
	k=0;
	for (i=0; i < mffield->count; i++) {
		if (InsertAt == i) {
			if (new_ptr) {
				*new_ptr = buffer + i*FieldSize;
				memset(*new_ptr, 0, sizeof(char)*FieldSize);
			}
			k = 1;
		}
		memcpy(buffer + (k+i) * FieldSize , mffield->array + i*FieldSize, FieldSize);
	}
	free(mffield->array);
	mffield->array = buffer;
	mffield->count += 1;
	return M4OK;
}

#define MAX_MFFIELD_ALLOC		5000000
M4Err VRML_MF_Alloc(void *mf, u32 FieldType, u32 NbItems)
{
	u32 FieldSize;
	GenMFField *mffield = (GenMFField *)mf;

	if (VRML_IsSFField(FieldType)) return M4BadParam;
	if (FieldType == FT_MFNode) return M4BadParam;

	FieldSize = GetSFFieldSize(FieldType);
	
	//field we can't copy
	if (!FieldSize) return M4BadParam;
	if (NbItems>MAX_MFFIELD_ALLOC) return M4IOErr;

	//erase everything
	if (mffield->array) free(mffield->array);
	mffield->array = NULL;
	if (NbItems) {
		mffield->array = malloc(sizeof(char)*FieldSize*NbItems);
		memset(mffield->array, 0, sizeof(char)*FieldSize*NbItems);
	}
	mffield->count = NbItems;
	return M4OK;
}

M4Err VRML_MF_GetItem(void *mf, u32 FieldType, void **new_ptr, u32 ItemPos)
{
	u32 FieldSize;
	GenMFField *mffield = (GenMFField *)mf;

	*new_ptr = NULL;
	if (VRML_IsSFField(FieldType)) return M4BadParam;
	if (FieldType == FT_MFNode) return M4BadParam;

	FieldSize = GetSFFieldSize(FieldType);
	
	//field we can't copy
	if (!FieldSize) return M4BadParam;
	if (ItemPos >= mffield->count) return M4BadParam;
	*new_ptr = mffield->array + ItemPos * FieldSize;
	return M4OK;
}


M4Err VRML_MF_Append(void *mf, u32 FieldType, void **new_ptr)
{
	GenMFField *mffield = (GenMFField *)mf;
	return VRML_MF_Insert(mf, FieldType, new_ptr, mffield->count+2);
}


//remove the specified item (0-based index)
M4Err VRML_MF_Remove(void *mf, u32 FieldType, u32 RemoveFrom)
{
	char *buffer;
	u32 FieldSize, i, k;
	GenMFField *mffield = (GenMFField *)mf;

	FieldSize = GetSFFieldSize(FieldType);
	
	//field we can't copy
	if (!FieldSize) return M4BadParam;

	if (!mffield->count || RemoveFrom >= mffield->count) return M4BadParam;

	if (mffield->count == 1) {
		free(mffield->array);
		mffield->array = NULL;
		mffield->count = 0;
		return M4OK;
	}
	k=0;
	buffer = malloc(sizeof(char)*(mffield->count-1)*FieldSize);
	for (i=0; i<mffield->count; i++) {
		if (RemoveFrom == i) {
			k = 1;
		} else {
			memcpy(buffer + (i-k)*FieldSize, mffield->array + i*FieldSize, FieldSize);
		}
	}
	free(mffield->array);
	mffield->array = buffer;
	mffield->count -= 1;
	return M4OK;
}

M4Err VRML_MF_Reset(void *mf, u32 FieldType)
{
	GenMFField *mffield = (GenMFField *)mf;

	//field we can't copy
	if (VRML_IsSFField(FieldType)) return M4BadParam;
	if (!GetSFFieldSize(FieldType)) return M4BadParam;

	switch (FieldType) {
	case FT_MFString:
		MFString_Del( * ((MFString *) mf));
		break;
	case FT_MFURL:
		MFURL_Del( * ((MFURL *) mf));
		break;
	case FT_MFScript:
		MFScript_Del( * ((MFScript *) mf));
		break;
	default:
		free(mffield->array);
		break;
	}

	mffield->array = NULL;
	mffield->count = 0;
	return M4OK;
}

/*special cloning with type-casting from SF/MF strings to URL conversion since proto URL doesn't exist
as a field type (it's just a stupid encoding trick) */
void VRML_FieldCopyCast(void *dest, u32 dst_field_type, void *orig, u32 ori_field_type)
{
	SFURL *url;
	char tmp[50];
	u32 size, i, sf_type_ori, sf_type_dst;
	void *dst_field, *orig_field;
	if (!dest || !orig) return;
	
	switch (dst_field_type) {
	case FT_SFString:
		if (ori_field_type == FT_SFURL) {
			url = ((SFURL *)orig);
			if (url->OD_ID>0) {
				sprintf(tmp, "%d", url->OD_ID);
				if ( ((SFString*)dest)->buffer) free(((SFString*)dest)->buffer);
				((SFString*)dest)->buffer = strdup(tmp);
			} else {
				if ( ((SFString*)dest)->buffer) free(((SFString*)dest)->buffer);
				((SFString*)dest)->buffer = strdup(url->url);
			}
		}
		/*for SFString to MFString cast*/
		else if (ori_field_type == FT_SFString) {
			if ( ((SFString*)dest)->buffer) free(((SFString*)dest)->buffer);
			((SFString*)dest)->buffer = strdup(((SFString*)orig)->buffer);
		}
		return;
	case FT_SFURL:
		if (ori_field_type != FT_SFString) return;
		url = ((SFURL *)dest);
		url->OD_ID = 0;
		if (url->url) free(url->url);
		if ( ((SFString*)orig)->buffer) 
			url->url = strdup(((SFString*)orig)->buffer);
		else 
			url->url = NULL;
		return;
	case FT_MFString:
	case FT_MFURL:
		break;
	default:
		return;
	}

	sf_type_dst = VRML_GetSFType(dst_field_type);

	if (VRML_IsSFField(ori_field_type)) {
		size = 1;
		VRML_MF_Alloc(dest, dst_field_type, size);
		VRML_MF_GetItem(dest, dst_field_type, &dst_field, 0);
		VRML_FieldCopyCast(dst_field, sf_type_dst, orig, ori_field_type);
		return;
	}

	size = ((GenMFField *)orig)->count;
	VRML_MF_Alloc(dest, dst_field_type, size);
	sf_type_ori = VRML_GetSFType(ori_field_type);
	//duplicate all items
	for (i=0; i<size; i++) {
		VRML_MF_GetItem(dest, dst_field_type, &dst_field, i);
		VRML_MF_GetItem(orig, ori_field_type, &orig_field, i);
		VRML_FieldCopyCast(dst_field, sf_type_dst, orig_field, sf_type_ori);
	}
	return;
}

void VRML_FieldCopy(void *dest, void *orig, u32 field_type)
{
	u32 size, i, sf_type;
	void *dst_field, *orig_field;

	if (!dest || !orig) return;

	switch (field_type) {
	case FT_SFBool:
		memcpy(dest, orig, sizeof(SFBool));
		break;
	case FT_SFColor:
		memcpy(dest, orig, sizeof(SFColor));
		break;
	case FT_SFFloat:
		memcpy(dest, orig, sizeof(SFFloat));
		break;
	case FT_SFInt32:
		memcpy(dest, orig, sizeof(SFInt32));
		break;
	case FT_SFRotation:
		memcpy(dest, orig, sizeof(SFRotation));
		break;
	case FT_SFTime:
		memcpy(dest, orig, sizeof(SFTime));
		break;
	case FT_SFVec2f:
		memcpy(dest, orig, sizeof(SFVec2f));
		break;
	case FT_SFVec3f:
		memcpy(dest, orig, sizeof(SFVec3f));
		break;
	case FT_SFString:
		if ( ((SFString*)dest)->buffer) free(((SFString*)dest)->buffer);
		if ( ((SFString*)orig)->buffer )
			((SFString*)dest)->buffer = strdup(((SFString*)orig)->buffer);
		else
			((SFString*)dest)->buffer = NULL;
		break;
	case FT_SFURL:
		if ( ((SFURL *)dest)->url ) free( ((SFURL *)dest)->url );
		((SFURL *)dest)->OD_ID = ((SFURL *)orig)->OD_ID;
		if (((SFURL *)orig)->url) 
			((SFURL *)dest)->url = strdup(((SFURL *)orig)->url);
		else
			((SFURL *)dest)->url = NULL;
		break;
	case FT_SFImage:
		if (((SFImage *)dest)->pixels) free(((SFImage *)dest)->pixels);
		((SFImage *)dest)->width = ((SFImage *)orig)->width;
		((SFImage *)dest)->height = ((SFImage *)orig)->height;
		((SFImage *)dest)->numComponents  = ((SFImage *)orig)->numComponents;
		size = ((SFImage *)dest)->width * ((SFImage *)dest)->height * ((SFImage *)dest)->numComponents;
		((SFImage *)dest)->pixels = malloc(sizeof(char)*size);
		memcpy(((SFImage *)dest)->pixels, ((SFImage *)orig)->pixels, sizeof(char)*size);
		break;
	case FT_SFCommandBuffer:
		SFCommandBuffer_Del( *(SFCommandBuffer *)dest);
		((SFCommandBuffer *)dest)->commandList = NewChain();
		((SFCommandBuffer *)dest)->bufferSize = ((SFCommandBuffer *)orig)->bufferSize;
		((SFCommandBuffer *)dest)->buffer = malloc(sizeof(char)*((SFCommandBuffer *)orig)->bufferSize);
		memcpy(((SFCommandBuffer *)dest)->buffer, 
			((SFCommandBuffer *)orig)->buffer,
			sizeof(char)*((SFCommandBuffer *)orig)->bufferSize);
		break;

	/*simply copy text string*/
	case FT_SFScript:
		if (((SFScript*)dest)->script_text) free(((SFScript*)dest)->script_text);		
		((SFScript*)dest)->script_text = NULL;
		if ( ((SFScript*)orig)->script_text)
			((SFScript *)dest)->script_text = strdup( ((SFScript*)orig)->script_text );
		break;


	//MFFields
	case FT_MFBool:
	case FT_MFFloat:
	case FT_MFTime:
	case FT_MFInt32:
	case FT_MFString:
	case FT_MFVec3f:
	case FT_MFVec2f:
	case FT_MFColor:
	case FT_MFRotation:
	case FT_MFImage:
	case FT_MFURL:
	case FT_MFScript:
		size = ((GenMFField *)orig)->count;
		VRML_MF_Reset(dest, field_type);
		VRML_MF_Alloc(dest, field_type, size);
		sf_type = VRML_GetSFType(field_type);
		//duplicate all items
		for (i=0; i<size; i++) {
			VRML_MF_GetItem(dest, field_type, &dst_field, i);
			VRML_MF_GetItem(orig, field_type, &orig_field, i);
			VRML_FieldCopy(dst_field, orig_field, sf_type);
		}
		break;
	}
}


Bool VRML_FieldsEqual(void *dest, void *orig, u32 field_type)
{
	u32 size, i, sf_type;
	void *dst_field, *orig_field;
	Bool changed = 0;

	if (!dest || !orig) return 0;

	switch (field_type) {
	case FT_SFBool:
		changed = memcmp(dest, orig, sizeof(SFBool));
		break;
	case FT_SFColor:
		if (((SFColor *)dest)->red != ((SFColor *)orig)->red) changed = 1;
		else if (((SFColor *)dest)->green != ((SFColor *)orig)->green) changed = 1;
		else if (((SFColor *)dest)->blue != ((SFColor *)orig)->blue) changed = 1;
		break;
	case FT_SFFloat:
		if ( (*(SFFloat *)dest) != (*(SFFloat *)orig) ) changed = 1;
		break;
	case FT_SFInt32:
		changed = memcmp(dest, orig, sizeof(SFInt32));
		break;
	case FT_SFRotation:
		if (((SFRotation *)dest)->xAxis != ((SFRotation *)orig)->xAxis) changed = 1;
		else if (((SFRotation *)dest)->yAxis != ((SFRotation *)orig)->yAxis) changed = 1;
		else if (((SFRotation *)dest)->zAxis != ((SFRotation *)orig)->zAxis) changed = 1;
		else if (((SFRotation *)dest)->angle != ((SFRotation *)orig)->angle) changed = 1;
		break;
	case FT_SFTime:
		if ( (*(SFTime *)dest) != (*(SFTime*)orig) ) changed = 1;
		break;
	case FT_SFVec2f:
		if (((SFVec2f *)dest)->x != ((SFVec2f *)orig)->x) changed = 1;
		else if (((SFVec2f *)dest)->y != ((SFVec2f *)orig)->y) changed = 1;
		break;
	case FT_SFVec3f:
		if (((SFVec3f *)dest)->x != ((SFVec3f *)orig)->x) changed = 1;
		else if (((SFVec3f *)dest)->y != ((SFVec3f *)orig)->y) changed = 1;
		else if (((SFVec3f *)dest)->z != ((SFVec3f *)orig)->z) changed = 1;
		break;
	case FT_SFString:
		if ( ((SFString*)dest)->buffer && ((SFString*)orig)->buffer) {
			changed = strcmp(((SFString*)dest)->buffer, ((SFString*)orig)->buffer);
		} else {
			changed = ( !((SFString*)dest)->buffer && !((SFString*)orig)->buffer) ? 0 : 1;
		}
		break;
	case FT_SFURL:
		if (((SFURL *)dest)->OD_ID > 0 || ((SFURL *)orig)->OD_ID > 0) {
			if ( ((SFURL *)orig)->OD_ID != ((SFURL *)dest)->OD_ID) changed = 1;
		} else {
			if ( ((SFURL *)orig)->url && ! ((SFURL *)dest)->url) changed = 1;
			else if ( ! ((SFURL *)orig)->url && ((SFURL *)dest)->url) changed = 1;
			else if ( strcmp( ((SFURL *)orig)->url , ((SFURL *)dest)->url) ) changed = 1;
		}
		break;
	case FT_SFImage:
	case FT_SFScript:
	case FT_SFCommandBuffer:
		changed = 1;
		break;

	//MFFields
	case FT_MFBool:
	case FT_MFFloat:
	case FT_MFTime:
	case FT_MFInt32:
	case FT_MFString:
	case FT_MFVec3f:
	case FT_MFVec2f:
	case FT_MFColor:
	case FT_MFRotation:
	case FT_MFImage:
	case FT_MFURL:
	case FT_MFScript:
		if ( ((GenMFField *)orig)->count != ((GenMFField *)dest)->count) changed = 1;
		else {
			size = ((GenMFField *)orig)->count;
			sf_type = VRML_GetSFType(field_type);
			for (i=0; i<size; i++) {
				VRML_MF_GetItem(dest, field_type, &dst_field, i);
				VRML_MF_GetItem(orig, field_type, &orig_field, i);
				if (! VRML_FieldsEqual(dst_field, orig_field, sf_type) ) {
					changed = 1;
					break;
				}
			}
		}
		break;
	}
	return changed ? 0 : 1;
}

#include <math.h>
void SFColor_fromHSV(SFColor *col)
{
    Float f, q, t, p, hue, sat, val;
    u32 i;  
	hue = col->red;
	sat = col->green;
	val = col->blue;
	if (sat==0.0f) {
		col->red = col->green = col->blue = val;
		return;
	}
    if (hue == 1.0f) hue = 0.0f;
    else hue *= 6.0f;
    i = (u32)(floor(hue));
    f = hue-i;
    p = val*(1.0f-sat);
    q = val*(1.0f- sat*f);
    t = val*(1.0f-(sat*(1.0f-f)));
    switch (i) {
	case 0: col->red = val; col->green = t; col->blue = p; break;
	case 1: col->red = q; col->green = val; col->blue = p; break;
	case 2: col->red = p; col->green = val; col->blue = t; break;
	case 3: col->red = p; col->green = q; col->blue = val; break;
	case 4: col->red = t; col->green = p; col->blue = val; break;
	case 5: col->red = val; col->green = p; col->blue = q; break;
    }
}

void SFColor_toHSV(SFColor *col)
{
	Float h, s;
	Float _max = MAX(col->red, MAX(col->green, col->blue));
	Float _min = MIN(col->red, MAX(col->green, col->blue));

	s = ((_max == 0) ? 0 : ((_max-_min)/_max));
	if (s != 0) {
		Float rl = (_max - col->red) / (_max-_min);
		Float gl = (_max - col->green) / (_max-_min);
		Float bl = (_max - col->blue) / (_max-_min);
		if (_max == col->red) {
			if (_min == col->green) h = 60*(5+bl);
			else h = 60*(1-gl);
		} else if (_max == col->green) {
			if (_min == col->blue) h = 60*(1+rl);
			else h = 60*(3-bl);
		} else {
			if (_min == col->red) h = 60*(3+gl);
			else h = 60*(5-rl);
		}
	} else {
		h = 0;
	}
	col->red = h;
	col->green = s;
	col->blue = _max;
}

SFColorRGBA SFColor_ToRGBA(SFColor val)
{
	SFColorRGBA res;
	res.alpha = 1;
	res.red = val.red;
	res.green = val.green;
	res.blue = val.blue;
	return res;
}



u32 Node_GetNumFields(SFNode *Node, u8 IndexMode)
{
	assert(Node);
#ifdef NODE_USE_POINTERS
	return Node->sgprivate->get_field_count(Node, IndexMode);
#else

	if (Node->sgprivate->tag == TAG_ProtoNode) return Proto_GetNumFields(Node, IndexMode);
	else if ((Node->sgprivate->tag == TAG_MPEG4_Script) || (Node->sgprivate->tag == TAG_X3D_Script) )
		return Script_GetNumFields(Node, IndexMode);
	else if (Node->sgprivate->tag <= TAG_RANGE_LAST_MPEG4) return MPEG4Node_GetFieldCount(Node, IndexMode);
	else if (Node->sgprivate->tag <= TAG_RANGE_LAST_X3D) return X3DNode_GetFieldCount(Node);
	else return 0;

#endif
}


Bool Node_IsInTable(SFNode *node, u32 NDTType)
{
	u32 tag = node ? node->sgprivate->tag : 0;
	if (!tag) return 0;
	if (tag==TAG_ProtoNode) tag = Proto_GetRenderingTag(((ProtoInstance *)node)->proto_interface);
	if (tag==TAG_UndefinedNode) return 1;
	else if (tag<=TAG_RANGE_LAST_MPEG4) {
		u32 i;
		u32 NDT_GetNodeType(u32 NDT_Tag, u32 NodeTag, u32 Version);

		for (i=0;i<LAST_BIFS_VERSION; i++) {
			if (NDT_GetNodeType(NDTType, tag, i+1)) return 1;
		}
		return 0;
	} else if (tag<=TAG_RANGE_LAST_X3D) {
		Bool X3D_IsNodeInTable(u32 NDT_Tag, u32 NodeTag);
		return X3D_IsNodeInTable(NDTType, tag);
	}
	return 0;
}

/*all our internally handled nodes*/
void ScalarInt_SetFraction(SFNode *node);
void PosInt2D_SetFraction(SFNode *node);
void ColorInt_SetFraction(SFNode *node);
void CI2D_SetFraction(SFNode *n);
void CoordInt_SetFraction(SFNode *n);
void PosInt_SetFraction(SFNode *node);
void OrientInt_SetFraction(SFNode *node);
void NormInt_SetFraction(SFNode *n);
void Valuator_SetInSFBool(SFNode *n);
void Valuator_SetInSFColor(SFNode *n);
void Valuator_SetInSFFloat(SFNode *n);
void Valuator_SetInSFInt32(SFNode *n);
void Valuator_SetInSFTime(SFNode *n);
void Valuator_SetInSFVec2f(SFNode *n);
void Valuator_SetInSFVec3f(SFNode *n);
void Valuator_SetInSFRotation(SFNode *n);
void Valuator_SetInSFString(SFNode *n);
void Valuator_SetInMFString(SFNode *n);
void Valuator_SetInMFColor(SFNode *n);
void Valuator_SetInMFFloat(SFNode *n);
void Valuator_SetInMFInt32(SFNode *n);
void Valuator_SetInMFVec2f(SFNode *n);
void Valuator_SetInMFVec3f(SFNode *n);
void Valuator_SetInMFRotation(SFNode *n);
void PA_Init(SFNode *n);
void PA_Modified(SFNode *n, FieldInfo *field);
void PA2D_Init(SFNode *n);
void PA2D_Modified(SFNode *n, FieldInfo *field);
void SA_Init(SFNode *n);
void SA_Modified(SFNode *n, FieldInfo *field);
void CI4D_SetFraction(SFNode *n);
void PI4D_SetFraction(SFNode *n);
/*X3D tools*/
void InitBooleanFilter(SFNode *n);
void InitBooleanSequencer(SFNode *n);
void InitBooleanToggle(SFNode *n);
void InitBooleanTrigger(SFNode *n);
void InitIntegerSequencer(SFNode *n);
void InitIntegerTrigger(SFNode *n);
void InitTimeTrigger(SFNode *n);


Bool VRML_NodeInit(SFNode *node)
{
	switch (node->sgprivate->tag) {
	case TAG_MPEG4_ColorInterpolator: 
	case TAG_X3D_ColorInterpolator:
		((M_ColorInterpolator *)node)->on_set_fraction = ColorInt_SetFraction; return 1;
	case TAG_MPEG4_CoordinateInterpolator: 
	case TAG_X3D_CoordinateInterpolator: 
		((M_CoordinateInterpolator *)node)->on_set_fraction = CoordInt_SetFraction; return 1;
	case TAG_MPEG4_CoordinateInterpolator2D: 
		((M_CoordinateInterpolator2D *)node)->on_set_fraction = CI2D_SetFraction; return 1;
	case TAG_MPEG4_NormalInterpolator: 
	case TAG_X3D_NormalInterpolator: 
		((M_NormalInterpolator*)node)->on_set_fraction = NormInt_SetFraction; return 1;
	case TAG_MPEG4_OrientationInterpolator: 
	case TAG_X3D_OrientationInterpolator: 
		((M_OrientationInterpolator*)node)->on_set_fraction = OrientInt_SetFraction; return 1;
	case TAG_MPEG4_PositionInterpolator: 
	case TAG_X3D_PositionInterpolator: 
		((M_PositionInterpolator *)node)->on_set_fraction = PosInt_SetFraction; return 1;
	case TAG_MPEG4_PositionInterpolator2D:
	case TAG_X3D_PositionInterpolator2D:
		((M_PositionInterpolator2D *)node)->on_set_fraction = PosInt2D_SetFraction; return 1;
	case TAG_MPEG4_ScalarInterpolator: 
	case TAG_X3D_ScalarInterpolator: 
		((M_ScalarInterpolator *)node)->on_set_fraction = ScalarInt_SetFraction; return 1;
	case TAG_MPEG4_Valuator:
		((M_Valuator *)node)->on_inSFTime = Valuator_SetInSFTime;
		((M_Valuator *)node)->on_inSFBool = Valuator_SetInSFBool;
		((M_Valuator *)node)->on_inSFColor = Valuator_SetInSFColor;
		((M_Valuator *)node)->on_inSFInt32 = Valuator_SetInSFInt32;
		((M_Valuator *)node)->on_inSFFloat = Valuator_SetInSFFloat;
		((M_Valuator *)node)->on_inSFVec2f = Valuator_SetInSFVec2f;
		((M_Valuator *)node)->on_inSFVec3f = Valuator_SetInSFVec3f;
		((M_Valuator *)node)->on_inSFRotation = Valuator_SetInSFRotation;
		((M_Valuator *)node)->on_inSFString = Valuator_SetInSFString;
		((M_Valuator *)node)->on_inMFColor = Valuator_SetInMFColor;
		((M_Valuator *)node)->on_inMFInt32 = Valuator_SetInMFInt32;
		((M_Valuator *)node)->on_inMFFloat = Valuator_SetInMFFloat;
		((M_Valuator *)node)->on_inMFVec2f = Valuator_SetInMFVec2f;
		((M_Valuator *)node)->on_inMFVec3f = Valuator_SetInMFVec3f;
		((M_Valuator *)node)->on_inMFRotation = Valuator_SetInMFRotation;
		((M_Valuator *)node)->on_inMFString = Valuator_SetInMFString;
		return 1;
	case TAG_MPEG4_PositionAnimator: PA_Init(node); return 1;
	case TAG_MPEG4_PositionAnimator2D: PA2D_Init(node); return 1;
	case TAG_MPEG4_ScalarAnimator: SA_Init(node); return 1;
	case TAG_MPEG4_PositionInterpolator4D: ((M_PositionInterpolator4D *)node)->on_set_fraction = PI4D_SetFraction; return 1;
	case TAG_MPEG4_CoordinateInterpolator4D: ((M_CoordinateInterpolator4D *)node)->on_set_fraction = CI4D_SetFraction; return 1;
	case TAG_MPEG4_Script: return 1;
	case TAG_X3D_Script: return 1;

	case TAG_X3D_BooleanFilter: InitBooleanFilter(node); return 1;
	case TAG_X3D_BooleanSequencer: InitBooleanSequencer(node); return 1;
	case TAG_X3D_BooleanToggle: InitBooleanToggle(node); return 1;
	case TAG_X3D_BooleanTrigger: InitBooleanTrigger(node); return 1;
	case TAG_X3D_IntegerSequencer: InitIntegerSequencer(node); return 1;
	case TAG_X3D_IntegerTrigger: InitIntegerTrigger(node); return 1;
	case TAG_X3D_TimeTrigger: InitTimeTrigger(node); return 1;
	}
	return 0;
}

Bool VRML_NodeChanged(SFNode *node, FieldInfo *field)
{
	switch (node->sgprivate->tag) {
	case TAG_ProtoNode:
		/*hardcoded protos need modification notifs*/
		if (node->sgprivate->RenderNode) return 0;
	case TAG_MPEG4_ColorInterpolator: 
	case TAG_X3D_ColorInterpolator:
	case TAG_MPEG4_CoordinateInterpolator: 
	case TAG_X3D_CoordinateInterpolator: 
	case TAG_MPEG4_CoordinateInterpolator2D: 
	case TAG_MPEG4_NormalInterpolator: 
	case TAG_X3D_NormalInterpolator: 
	case TAG_MPEG4_OrientationInterpolator: 
	case TAG_X3D_OrientationInterpolator: 
	case TAG_MPEG4_PositionInterpolator: 
	case TAG_X3D_PositionInterpolator: 
	case TAG_MPEG4_PositionInterpolator2D: 
	case TAG_MPEG4_ScalarInterpolator: 
	case TAG_X3D_ScalarInterpolator: 
	case TAG_MPEG4_Valuator:
	case TAG_MPEG4_PositionInterpolator4D:
	case TAG_MPEG4_CoordinateInterpolator4D:
	case TAG_MPEG4_Script:
	case TAG_X3D_Script:
	case TAG_X3D_BooleanFilter:
	case TAG_X3D_BooleanSequencer:
	case TAG_X3D_BooleanToggle:
	case TAG_X3D_BooleanTrigger:
	case TAG_X3D_IntegerSequencer:
	case TAG_X3D_IntegerTrigger:
	case TAG_X3D_TimeTrigger:
		return 1;
	case TAG_MPEG4_PositionAnimator: PA_Modified(node, field); return 1;
	case TAG_MPEG4_PositionAnimator2D: PA2D_Modified(node, field); return 1;
	case TAG_MPEG4_ScalarAnimator: SA_Modified(node, field); return 1;
	}
	return 0;
}


