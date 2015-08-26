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
/*MPEG4 & X3D tags (for node tables & script handling)*/
#include <gpac/m4_mpeg4_nodes.h>
#include <gpac/m4_x3d_nodes.h>


LPROUTE SG_NewRoute(LPSCENEGRAPH sg, SFNode *fromNode, u32 fromField, SFNode *toNode, u32 toField)
{
	Route *r;
	FieldInfo info;
	if (!sg || !toNode || !fromNode) return NULL;

	SAFEALLOC(r, sizeof(Route));
	if (!r) return NULL;
	r->FromNode = fromNode;
	r->FromFieldIndex = fromField;
	r->ToNode = toNode;
	r->ToFieldIndex = toField;
	r->graph = sg;

	//remember the name of the event out
	Node_GetField(fromNode, fromField, &info);
	r->fromFieldName = info.name;
	//and bind eventOut
	ChainAddEntry(fromNode->sgprivate->outRoutes, r);

	ChainAddEntry(sg->Routes, r);
	return r;
}

void Route_Destroy(LPROUTE r)
{
	if (r->name) free(r->name);
	free(r);
}

void SG_DeleteRoute(LPROUTE r)
{
	LPSCENEGRAPH sg;
	s32 ind;

	RemoveActivatedRoute(r->graph, r);

	/*remove declared routes*/
	ind = ChainDeleteItem(r->graph->Routes, r);
	/*remove route from node*/
	if (r->FromNode) ChainDeleteItem(r->FromNode->sgprivate->outRoutes, r);
	r->is_setup = 0;
	sg = r->graph;
	while (sg->parent_scene) sg = sg->parent_scene;
	ChainAddEntry(sg->routes_to_destroy, r);
}


M4Err SG_DeleteRouteByID(LPSCENEGRAPH sg,u32 routeID)
{
	LPROUTE r;
	if(!sg) return M4BadParam;
	r = SG_FindRoute(sg, routeID);
	if (!r) return M4BadParam;
	SG_DeleteRoute(r);
	return M4OK;
}

LPROUTE SG_FindRoute(LPSCENEGRAPH sg, u32 RouteID)
{
	Route *r;
	u32 i;
	for (i=0; i<ChainGetCount(sg->Routes); i++) {
		r = ChainGetEntry(sg->Routes, i);
		if (r->ID == RouteID) return r;
	}
	return NULL;
}

LPROUTE SG_FindRouteByName(LPSCENEGRAPH sg, char *name)
{
	Route *r;
	u32 i;
	if (!sg || !name) return NULL;

	for (i=0; i<ChainGetCount(sg->Routes); i++) {
		r = ChainGetEntry(sg->Routes, i);
		if (r->name && !strcmp(r->name, name)) return r;
	}
	return NULL;
}


M4Err SG_SetRouteID(LPROUTE route, u32 ID)
{
	Route *ptr;
	if (!route || !ID) return M4BadParam;

	ptr = SG_FindRoute(route->graph, ID);
	if (ptr) return M4BadParam;
	route->ID = ID;
	return M4OK;
}
u32 SG_GetRouteID(LPROUTE route) 
{
	return route->ID;
}

M4Err SG_SetRouteName(LPROUTE route, char *name)
{
	Route *ptr;
	if (!name || !route) return M4BadParam;
	ptr = SG_FindRouteByName(route->graph, name);
	if (ptr) return M4BadParam;
	if (route->name) free(route->name);
	route->name = strdup(name);
	return M4OK;
}
char *SG_GetRouteName(LPROUTE route)
{
	return route->name;
}


Bool ActivateRoute(Route *r)
{
	Bool ret;
	/*URL/String conversion clone*/
	void VRML_FieldCopyCast(void *dest, u32 dst_field_type, void *orig, u32 ori_field_type);

	if (!r->is_setup) {
		Node_GetField(r->FromNode, r->FromFieldIndex, &r->FromField);
		Node_GetField(r->ToNode, r->ToFieldIndex, &r->ToField);
		r->is_setup = 1;
		/*special case when initing ISed routes on eventOuts: skip*/
		if (r->IS_route && (r->FromField.eventType == ET_EventOut)) return 0;
	}

	ret = 1;
	switch (r->FromField.fieldType) {
	case FT_SFNode:
		if (* (SFNode **) r->ToField.far_ptr != * (SFNode **) r->FromField.far_ptr) {
			SFNode *n = * (SFNode **) r->ToField.far_ptr;
			/*delete instance*/
			if (n) Node_Unregister(n, r->ToNode);
			/*and use the node*/
			* (SFNode **) r->ToField.far_ptr = * (SFNode **) r->FromField.far_ptr;
			n = * (SFNode **) r->FromField.far_ptr;
			Node_Register(n, r->ToNode);
		}
		break;

	/*move all pointers to dest*/
	case FT_MFNode:
	{
		u32 i;
		SFNode *p;
		Chain *orig = *(Chain**)r->FromField.far_ptr;
		Chain *dest = *(Chain**)r->ToField.far_ptr;

		/*empty list*/
		while (ChainGetCount(dest)){
			p = ChainGetEntry(dest, 0);
			ChainDeleteEntry(dest, 0);
			Node_Unregister(p, r->ToNode);
		}

		for (i=0; i<ChainGetCount(orig); i++) {
			p = ChainGetEntry(orig, i);
			ChainAddEntry(dest, p);
			Node_Register(p, r->ToNode);
		}
	}
		break;

	default:
		if (r->ToField.fieldType==r->FromField.fieldType) {
			/*if unchanged don't invalidate dst node*/
			if (VRML_FieldsEqual(r->ToField.far_ptr, r->FromField.far_ptr, r->FromField.fieldType)) {
				ret = 0;
			} else {
				VRML_FieldCopy(r->ToField.far_ptr, r->FromField.far_ptr, r->FromField.fieldType);
			}
		} 
		/*typecast URL <-> string if needed*/
		else {
			VRML_FieldCopyCast(r->ToField.far_ptr, r->ToField.fieldType, r->FromField.far_ptr, r->FromField.fieldType);
		}
		break;
	}

	//if this is a supported eventIn call watcher
	if (r->ToField.on_event_in) {
		r->ToField.on_event_in(r->ToNode);
	}
	//if this is a script eventIn call directly script
	else if (((r->ToNode->sgprivate->tag==TAG_MPEG4_Script) || (r->ToNode->sgprivate->tag==TAG_X3D_Script) ) 
		&& ((r->ToField.eventType==ET_EventIn) /*|| (r->ToField.eventType==ET_Field)*/) ) {
		Script_EventIn(r->ToNode, &r->ToField);
	}
	//check if ISed or not - this will notify the node of any changes
	else {
		Proto_CheckFieldChanged(r->ToNode, r->ToFieldIndex);
		/*only happen on proto, an eventOut may route to an eventOut*/
		if (r->IS_route && r->ToField.eventType==ET_EventOut) 
			Node_OnEventOut(r->ToNode, r->ToFieldIndex);
	}
	/*and signal routes on exposed fields if field changed*/
	if (r->ToField.eventType == ET_ExposedField)
		Node_OnEventOut(r->ToNode, r->ToFieldIndex);

	return ret;
}


void Node_OnEventOut(SFNode *node, u32 FieldIndex)
{
	u32 i;
	Route *r;
	if (!node) return;
	
	//this is not an ISed
	if (!node->sgprivate->NodeID && !node->sgprivate->scenegraph->pOwningProto) return;
	
	//search for routes to activate in the order they where declared
	for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(node->sgprivate->outRoutes, i);
		if (r->FromNode != node) continue;
		if (r->FromFieldIndex != FieldIndex) continue;

		/*no postpone for IS routes*/
		if (r->IS_route ) {
			if (ActivateRoute(r)) SG_NodeChanged(r->ToNode, &r->ToField);
		}
		//queue
		else {
			QueueRoute(node->sgprivate->scenegraph, r);
		}
	}
}

void Node_OnEventOutSTR(SFNode *node, const char *eventName)
{
	u32 i;
	LPROUTE r;

	/*node is being deleted ignore event*/
	if (!node->sgprivate->outRoutes) return;

	//this is not an ISed
	if (!node->sgprivate->NodeID && !node->sgprivate->scenegraph->pOwningProto) return;
	
	//search for routes to activate in the order they where declared
	for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(node->sgprivate->outRoutes, i);
		if (stricmp(r->fromFieldName, eventName)) continue;

		//no postpone
		if (r->IS_route) {
			ActivateRoute(r);
		}
		//queue
		else {
			QueueRoute(node->sgprivate->scenegraph, r);
		}
	}
}

