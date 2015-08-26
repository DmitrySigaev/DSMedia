/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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
#include <intern/m4_bifs_dev.h>
#include "Quantize.h"

M4Err BD_DecMFFieldList(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);
M4Err BD_DecMFFieldVec(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);



void BD_GetName(BitStream *bs, char *name)
{
	u32 i = 0;
	while (1) {
		name[i] = BS_ReadInt(bs, 8);
		if (!name[i]) break;
		i++;
	}
}

static M4Err BD_DecProtoDelete(BifsDecoder * codec, BitStream *bs)
{
	u32 ID, flag, count;
	LPPROTO proto;

	flag = BS_ReadInt(bs, 1);
	if (flag) {
		flag = BS_ReadInt(bs, 1);
		while (flag) {
			ID = BS_ReadInt(bs, codec->info->config.ProtoIDBits);
			proto = SG_FindProto(codec->current_graph, ID, NULL);
			if (proto) SG_DeleteProto(proto);
			flag = BS_ReadInt(bs, 1);
		}
	} else {
		flag = BS_ReadInt(bs, 5);
		count = BS_ReadInt(bs, flag);
		while (count) {
			ID = BS_ReadInt(bs, codec->info->config.ProtoIDBits);
			proto = SG_FindProto(codec->current_graph, ID, NULL);
			if (proto) SG_DeleteProto(proto);
			count--;
		}
	}
	return M4OK;
}


static M4Err BD_DecMultipleIndexReplace(BifsDecoder * codec, BitStream *bs)
{
	u32 ID, ind, field_ind, NumBits, lenpos, lennum, count, pos;
	SFNode *node, *new_node;
	M4Err e;
	Chain *container;
	FieldInfo field, sffield;
	
	ID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = SG_FindNode(codec->current_graph, ID);
	if (!node) return M4NonCompliantBitStream;
	NumBits = GetNumBits(Node_GetNumFields(node, FCM_IN)-1);
	ind = BS_ReadInt(bs, NumBits);
	e = Node_GetFieldIndex(node, ind, FCM_IN, &field_ind);
	if (e) return e;
	e = Node_GetField(node, field_ind, &field);
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	lenpos = BS_ReadInt(bs, 5);
	lennum = BS_ReadInt(bs, 5);
	count = BS_ReadInt(bs, lennum);


	/*cf index value replace */
	if (field.fieldType == FT_MFNode) {
		while (count) {
			pos = BS_ReadInt(bs, lenpos);		
			container = *((Chain**) field.far_ptr);
			/*first decode*/
			new_node = BD_DecSFNode(codec, bs, field.NDTtype);
			if (!new_node) return codec->LastError;
			e = Node_Register(new_node, node);
			if (e) return e;
			/*then replace*/
			e = Node_ReplaceChild(node, container, pos, new_node);
			count--;
		}
		if (!e) BD_CheckFieldChanges(node, &field);
		return e;
	}
	/*Not a node list*/
	memcpy(&sffield, &field, sizeof(FieldInfo));
	sffield.fieldType = VRML_GetSFType(field.fieldType);

	while (count) {
		pos = BS_ReadInt(bs, lenpos);

		if (pos && pos >= ((GenMFField *)field.far_ptr)->count) {
			pos = ((GenMFField *)field.far_ptr)->count - 1;
		}

		e = VRML_MF_GetItem(field.far_ptr, field.fieldType, & sffield.far_ptr, pos);
		if (e) return e;
		e = BD_DecSFField(codec, bs, node, &sffield);
		if (e) break;
		count--;
	}
	if (!e) BD_CheckFieldChanges(node, &field);
	return e;
}

static M4Err BD_DecMultipleReplace(BifsDecoder * codec, BitStream *bs)
{
	M4Err e;
	u32 NodeID, flag;
	SFNode *node;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	
	flag = BS_ReadInt(bs, 1);
	if (flag) {
		e = BD_DecMaskNodeDesc(codec, bs, node);
	} else {
		e = BD_DecListNodeDesc(codec, bs, node);
	}
	return e;
}


static M4Err BD_DecGlobalQuantizer(BifsDecoder * codec, BitStream *bs)
{
	SFNode *node;
	node = BD_DecSFNode(codec, bs, NDT_SFWorldNode);

	/*reset global QP*/
	if (codec->GlobalQP) Node_Unregister((SFNode *) codec->GlobalQP, NULL);
	codec->GlobalQP = codec->ActiveQP = NULL;
	
	if (!node || (Node_GetTag(node) != TAG_MPEG4_QuantizationParameter)) {
		if (node) Node_Unregister(node, NULL);
		return codec->LastError;
	}

	/*register global QP*/
	codec->GlobalQP = codec->ActiveQP = (M_QuantizationParameter *) node;
	codec->GlobalQP->isLocal = 0;
	Node_Register(node, NULL);
	return M4OK;
}

static M4Err BD_DecNodeDeleteEx(BifsDecoder * codec, BitStream *bs)
{
	u32 NodeID;
	SFNode *node;
	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	return Node_ReplaceAllInstances(node, NULL, 1);
}

static M4Err BD_DecExtendedUpdate(BifsDecoder * codec, BitStream *bs)
{
	u32 type;

	type = BS_ReadInt(bs, 8);
	switch (type) {
	case 0:
		return BD_DecProtoList(codec, bs, NULL);
	case 1:
		return BD_DecProtoDelete(codec, bs);
	case 2:
		return SG_DeleteAllProtos(codec->current_graph);
	case 3:
		return BD_DecMultipleIndexReplace(codec, bs);
	case 4:
		return BD_DecMultipleReplace(codec, bs);
	case 5:
		return BD_DecGlobalQuantizer(codec, bs);
	case 6:
		return BD_DecNodeDeleteEx(codec, bs);
	default:
		return M4UnknowBIFSVersion;
	}
}

/*inserts a node in a container (node.children)*/
static M4Err BD_DecNodeInsert(BifsDecoder * codec, BitStream *bs)
{
	u32 NodeID, NDT;
	s32 type, pos;
	SFNode *node, *def;
	M4Err e;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	def = BIFS_FindNode(codec, NodeID);
	if (!def) return M4NonCompliantBitStream;

	NDT = Node_GetChildTable(def);
	if (!NDT) return M4NonCompliantBitStream;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		pos = BS_ReadInt(bs, 8);
		break;
	case 2:
		pos = 0;
		break;
	case 3:
		/*-1 means append*/
		pos = -1;
		break;
	default:
		return M4NonCompliantBitStream;
	}

	node = BD_DecSFNode(codec, bs, NDT);
	if (!node) return codec->LastError;

	e = Node_Register(node, def);
	if (e) return e;
	e = Node_InsertChild(def, node, pos);
	if (!e) {
		FieldInfo field;
		/*get it by name in case no add/removeChildren*/
		Node_GetFieldByName(def, "children", &field);
		BD_CheckFieldChanges(def, &field);
	}
	return e;
}

/*NB This can insert a node as well (but usually not in the .children field)*/
static M4Err BD_DecIndexInsert(BifsDecoder * codec, BitStream *bs)
{
	M4Err e;
	u32 NodeID;
	u32 NumBits, ind, field_ind;
	u8 type;
	s32 pos;
	SFNode *def, *node;
	FieldInfo field, sffield;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	def = BIFS_FindNode(codec, NodeID);
	if (!def) return M4NonCompliantBitStream;
	/*index insertion uses IN mode for field index*/
	NumBits = GetNumBits(Node_GetNumFields(def, FCM_IN)-1);
	ind = BS_ReadInt(bs, NumBits);

	e = Node_GetFieldIndex(def, ind, FCM_IN, &field_ind);
	if (e) return e;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		pos = BS_ReadInt(bs, 16);
		break;
	case 2:
		pos = 0;
		break;
	case 3:
		pos = -1;
		break;
	default:
		return M4NonCompliantBitStream;
	}

	e = Node_GetField(def, field_ind, &field);
	if (e) return e;
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	memcpy(&sffield, &field, sizeof(FieldInfo));
	sffield.fieldType = VRML_GetSFType(field.fieldType);

	/*rescale the MFField and parse the SFField*/
	if (field.fieldType==FT_MFNode) {
		node = BD_DecSFNode(codec, bs, field.NDTtype);
		if (!node) return codec->LastError;

		e = Node_Register(node, def);
		if (e) return e;
		/*this is generic MFNode container*/
		e = InsertSFNode(field.far_ptr, node, pos);
		if (!e) BD_CheckFieldChanges(def, &field);
	} else {
		if (pos == -1) {
			e = VRML_MF_Append(field.far_ptr, field.fieldType, & sffield.far_ptr);
		} else {
			/*insert is 0-based*/
			e = VRML_MF_Insert(field.far_ptr, field.fieldType, & sffield.far_ptr, pos);
		}
		if (e) return e;
		e = BD_DecSFField(codec, bs, def, &sffield);
		if (!e) BD_CheckFieldChanges(def, &field);
	}
	return e;
}


static M4Err BD_DecInsert(BifsDecoder * codec, BitStream *bs)
{
	u8 type;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		return BD_DecNodeInsert(codec, bs);
	/*Extended BIFS updates*/
	case 1:
		return BD_DecExtendedUpdate(codec, bs);
	case 2:
		return BD_DecIndexInsert(codec, bs);
	case 3:
		return BD_DecRoute(codec, bs, 1);
	default:
		return M4NonCompliantBitStream;
	}
}


static M4Err BD_DecIndexDelete(BifsDecoder * codec, BitStream *bs)
{
	u32 NodeID, NumBits, SF_type, ind, field_ind;
	s32 pos;
	u8 type;
	SFNode *node;
	M4Err e;
	FieldInfo field;
	
	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;

	NumBits = GetNumBits(Node_GetNumFields(node, FCM_IN) - 1);
	ind = BS_ReadInt(bs, NumBits);

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		pos = (u32) BS_ReadInt(bs, 16);
		break;
	case 2:
		pos = 0;
		break;
	case 3:
		pos = -1;
		break;
	default:
		return M4NonCompliantBitStream;
	}
	e = Node_GetFieldIndex(node, ind, FCM_IN, &field_ind);
	if (e) return e;
	e = Node_GetField(node, field_ind, &field);
	if (e) return e;
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	SF_type = VRML_GetSFType(field.fieldType);

	/*special handling in case of a node*/
	if (SF_type == FT_SFNode) {
		e = Node_ReplaceChild(node, * ((Chain **) field.far_ptr), pos, NULL);
	} else {
		e = VRML_MF_Remove(field.far_ptr, field.fieldType, pos);
	}
	/*deletion -> node has changed*/
	if (!e) BD_CheckFieldChanges(node, &field);

	return e;
}

static M4Err BD_DecDelete(BifsDecoder * codec, BitStream *bs)
{
	u8 type;
	u32 ID;
	SFNode *n;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		ID = 1+BS_ReadInt(bs, codec->info->config.NodeIDBits);
		n = BIFS_FindNode(codec, ID);
#ifdef MPEG4_STRICT
		if (!n) return M4NonCompliantBitStream;
#else
		if (!n) return M4OK;
#endif
		/*this is a delete of a DEF node, remove ALL INSTANCES*/
		return Node_ReplaceAllInstances(n, NULL, 0);
	case 2:
		return BD_DecIndexDelete(codec, bs);
	case 3:
		ID = 1+BS_ReadInt(bs, codec->info->config.RouteIDBits);
		/*don't complain if route can't be deleted (not present)*/
		SG_DeleteRouteByID(codec->current_graph, ID);
		return M4OK;
	default:
		return M4NonCompliantBitStream;
	}
	return M4OK;
}


static M4Err BD_DecNodeReplace(BifsDecoder * codec, BitStream *bs)
{
	u32 NodeID;
	SFNode *node, *new_node;
	M4Err e;
	
	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	/*this is delete / new on a DEF node: replace ALL instances*/
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	
	/*and just parse a new SFNode - it is encoded in SFWorldNode table */
	new_node = BD_DecSFNode(codec, bs, NDT_SFWorldNode);
	if (!new_node && codec->LastError) return codec->LastError;
	
	e = Node_ReplaceAllInstances(node, new_node, 0);
	return e;
}

static M4Err BD_DecFieldReplace(BifsDecoder * codec, BitStream *bs)
{
	M4Err e;
	u32 NodeID, ind, field_ind, NumBits;
	SFNode *node, *prev_node;
	Chain *prev_list;
	FieldInfo field;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	NumBits = GetNumBits(Node_GetNumFields(node, FCM_IN)-1);
	ind = BS_ReadInt(bs, NumBits);
	e = Node_GetFieldIndex(node, ind, FCM_IN, &field_ind);
	if (e) return e;

	e = Node_GetField(node, field_ind, &field);

	prev_node = NULL;
	prev_list = NULL;
	/*store prev SF node*/
	if (field.fieldType == FT_SFNode) {
		prev_node = *((SFNode **) field.far_ptr);
	}
	/*store prev MFNode content*/
	else if (field.fieldType == FT_MFNode) {
		Chain *list = * ((Chain **) field.far_ptr);
		prev_list = NewChain();
		while (ChainGetCount(list)) {
			SFNode *n = ChainGetEntry(list, 0);
			ChainDeleteEntry(list, 0);
			ChainAddEntry(prev_list, n);
		}
	}
	/*regular field*/
	else if (!VRML_IsSFField(field.fieldType)) {
		VRML_MF_Reset(field.far_ptr, field.fieldType);
	}

	/*parse the field*/
	e = BD_DecField(codec, bs, node, &field);
	/*remove prev nodes*/
	if (field.fieldType == FT_SFNode) {
		if (prev_node) e = Node_Unregister(prev_node, node);
	} else if (field.fieldType == FT_MFNode) {
		Node_UnregisterChildren(node, prev_list);
		DeleteChain(prev_list);
	}
	if (!e) BD_CheckFieldChanges(node, &field);
	return e;
}

static M4Err BD_DecIndexValueReplace(BifsDecoder * codec, BitStream *bs)
{
	SFNode *new_node;
	u32 NodeID, ind, field_ind, NumBits, pos;
	u8 type;
	SFNode *node;
	M4Err e;
	Chain *container;
	FieldInfo field, sffield;

	/*get the node*/
	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);

	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	NumBits = GetNumBits(Node_GetNumFields(node, FCM_IN)-1);
	ind = BS_ReadInt(bs, NumBits);
	e = Node_GetFieldIndex(node, ind, FCM_IN, &field_ind);
	if (e) return e;

	e = Node_GetField(node, field_ind, &field);
	if (VRML_IsSFField(field.fieldType)) return M4NonCompliantBitStream;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		pos = BS_ReadInt(bs, 16);
		break;
	case 2:
		pos = 0;
		break;
	case 3:
		pos = ((GenMFField *) field.far_ptr)->count - 1;
		break;
	default:
		return M4NonCompliantBitStream;
	}
	
	/*if MFNode remove the child and parse new node*/
	if (field.fieldType == FT_MFNode) {
		container = *((Chain**) field.far_ptr);
		/*get the new node*/
		new_node = BD_DecSFNode(codec, bs, field.NDTtype);
		if (codec->LastError) {
			e = codec->LastError;
			goto exit;
		}
		if (new_node) {
			e = Node_Register(new_node, node);
			if (e) return e;
		}
		/*replace prev node*/
		e = Node_ReplaceChild(node, container, pos, new_node);
		if (!e) BD_CheckFieldChanges(node, &field);
	}
	/*erase the field item*/
	else {
		memcpy(&sffield, &field, sizeof(FieldInfo));
		sffield.fieldType = VRML_GetSFType(field.fieldType);

		/*make sure this is consistent*/
		if (pos && pos >= ((GenMFField *)field.far_ptr)->count) {
			pos = ((GenMFField *)field.far_ptr)->count - 1;
		}

		e = VRML_MF_GetItem(field.far_ptr, field.fieldType, & sffield.far_ptr, pos);
		if (e) return e;
		e = BD_DecSFField(codec, bs, node, &sffield);
		if (!e) BD_CheckFieldChanges(node, &field);
	}

exit:
	return e;
}

static u32 BD_DecRouteReplace(BifsDecoder * codec, BitStream *bs)
{
	M4Err e;
	u32 RouteID, numBits, ind, node_id, fromID, toID;
	char name[1000], *ptr;
	LPROUTE r;
	SFNode *OutNode, *InNode;

	RouteID = 1+BS_ReadInt(bs, codec->info->config.RouteIDBits);
	
	r = SG_FindRoute(codec->current_graph, RouteID);
#ifdef MPEG4_STRICT
	if (!r) return M4NonCompliantBitStream;
	ptr = SG_GetRouteName(r);
	SG_DeleteRoute(r);
#else
	ptr = NULL; 
	if (r) {
		ptr = SG_GetRouteName(r);
		SG_DeleteRoute(r);
	}
#endif

	if (ptr) strcpy(name, ptr);

	/*origin*/
	node_id = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	OutNode = BIFS_FindNode(codec, node_id);
	if (!OutNode) return M4NonCompliantBitStream;
	numBits = GetNumBits(Node_GetNumFields(OutNode, FCM_OUT) - 1);
	ind = BS_ReadInt(bs, numBits);
	e = Node_GetFieldIndex(OutNode, ind, FCM_OUT, &fromID);
	if (e) return e;

	/*target*/
	node_id = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	InNode = BIFS_FindNode(codec, node_id);
	if (!InNode) return M4NonCompliantBitStream;
	numBits = GetNumBits(Node_GetNumFields(InNode, FCM_IN) - 1);
	ind = BS_ReadInt(bs, numBits);
	e = Node_GetFieldIndex(InNode, ind, FCM_IN, &toID);
	if (e) return e;

	r = SG_NewRoute(codec->current_graph, OutNode, fromID, InNode, toID);
	if (!r) return M4OutOfMem;
	SG_SetRouteID(r, RouteID);
	if (ptr) e = SG_SetRouteName(r, name);

	return e;
}


static M4Err BD_DecReplace(BifsDecoder * codec, BitStream *bs)
{
	u8 type;
	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		return BD_DecNodeReplace(codec, bs);
	case 1:
		return BD_DecFieldReplace(codec, bs);
	case 2:
		return BD_DecIndexValueReplace(codec, bs);
	case 3:
		return BD_DecRouteReplace(codec, bs);
	}
	return M4OK;
}

/*if parent is non-NULL, we are in a proto code parsing, otherwise this is a top-level proto*/
M4Err BD_DecProtoList(BifsDecoder * codec, BitStream *bs, Chain *proto_list)
{
	u8 flag, field_type, event_type, useQuant, useAnim, f;
	u32 i, NbRoutes, ID, numProtos, numFields, count, qpsftype, QP_Type, NumBits, Anim_Type;
	SFNode *node;
	char name[1000];
	LPPROTOFIELD proto_field;
	LPPROTO proto, ParentProto;
	M4Err e;
	u32 hasMinMax;
	void *qp_min_value, *qp_max_value;
	LPSCENEGRAPH rootSG;
	FieldInfo field;

	NumBits = qpsftype = 0;
	//store proto at codec level
	rootSG = codec->current_graph;
	ParentProto = codec->pCurrentProto;
	e = M4OK;

	numProtos = 0;
	proto = NULL;
	flag = BS_ReadInt(bs, 1);
	while (flag) {

		if (!codec->info->config.ProtoIDBits) return M4NonCompliantBitStream;

		/*1- proto interface declaration*/
		ID = BS_ReadInt(bs, codec->info->config.ProtoIDBits);

		if (codec->info->UseName) {
			BD_GetName(bs, name);
		} else {
			sprintf(name, "Proto%d", numProtos);
		}
		/*create a proto in the current graph*/
		proto = SG_NewProto(codec->current_graph, ID, name, proto_list ? 1 : 0);
		if (proto_list) ChainAddEntry(proto_list, proto);
		
		/*during parsing, this proto is the current active one - all nodes/proto defined/declared
		below it will belong to its namespace*/
		codec->current_graph = Proto_GetSceneGraph(proto);
		codec->pCurrentProto = proto;

		numFields = 0;
		flag = BS_ReadInt(bs, 1);
		while (flag) {
			event_type = BS_ReadInt(bs, 2);
			field_type = BS_ReadInt(bs, 6);
			
			if (codec->info->UseName) {
				BD_GetName(bs, name);
			} else {
				sprintf(name, "_field%d", numFields);
			}

			/*create field interface*/
			proto_field = Proto_NewField(proto, field_type, event_type, name);

			/*get field info */
			ProtoField_GetField(proto_field, &field);

			switch (event_type) {
			case ET_ExposedField:
			case ET_Field:
				/*parse default value except nodes ...*/
				if (VRML_IsSFField(field_type)) {
					e = BD_DecSFField(codec, bs, NULL, &field);
				} else {
					f = 0;
					if (codec->info->config.UsePredictiveMFField) {
						f = BS_ReadInt(bs, 1);
						/*predictive encoding of proto field is not possible since QP info is not present yet*/
						assert(!f);
					}
					/*reserved*/
					f = BS_ReadInt(bs, 1);
					if (!f) {
						if (BS_ReadInt(bs, 1)) {
							e = BD_DecMFFieldList(codec, bs, NULL, &field);
						} else {
							e = BD_DecMFFieldVec(codec, bs, NULL, &field);
						}
					}
				}
				if (e) goto exit;

				break;
			}

			flag = BS_ReadInt(bs, 1);
			numFields++;
		}

		/*2- parse proto code*/
		flag = BS_ReadInt(bs, 1);

		/*externProto*/
		if (flag) {
			memset(&field, 0, sizeof(FieldInfo));
			field.far_ptr = Proto_GetExternURLFieldPointer(proto);
			field.fieldType = FT_MFURL;
			field.name = "ExternProto";

			if (codec->info->config.UsePredictiveMFField) {
				flag = BS_ReadInt(bs, 1);
				assert(!flag);
			}
			/*reserved*/
			flag = BS_ReadInt(bs, 1);

			/*list or vector*/
			flag = BS_ReadInt(bs, 1);
			if (flag) {
				e = BD_DecMFFieldList(codec, bs, NULL, &field);
			} else {
				e = BD_DecMFFieldVec(codec, bs, NULL, &field);
			}
			if (e) goto exit;
		}
		/*get proto code*/
		else {
			/*parse sub-proto list - subprotos are ALWAYS registered with parent proto graph*/
			e = BD_DecProtoList(codec, bs, NULL);
			if (e) goto exit;

			flag = 1;
			
			while (flag) {
				/*parse all nodes in SFWorldNode table*/
				node = BD_DecSFNode(codec, bs, NDT_SFWorldNode);
				if (!node) {
					e = codec->LastError;
					goto exit;
				}
				e = Node_Register(node, NULL);
				if (e) goto exit;

				Proto_AddNodeCode(proto, node);
				flag = BS_ReadInt(bs, 1);
			}
			
			/*routes*/
			flag = BS_ReadInt(bs, 1);
			if (flag) {
				flag = BS_ReadInt(bs, 1);
				if (flag) {
					/*list route*/
					while (flag) {
						e = BD_DecRoute(codec, bs, 0);
						if (e) goto exit;
						flag = BS_ReadInt(bs, 1);
					}
				} else {
					/*vector*/
					i = BS_ReadInt(bs, 5);
					NbRoutes = BS_ReadInt(bs, i);
					for (i=0; i<NbRoutes; i++) {
						e = BD_DecRoute(codec, bs, 0);
						if (e) goto exit;
					}
				}
			}
		}

		/*restore the namespace*/
		codec->current_graph = rootSG;

		/*3- parse anim and Quantization stuff*/
		useQuant = BS_ReadInt(bs, 1);
		useAnim = BS_ReadInt(bs, 1);

		count = Proto_GetFieldCount(proto);
		for (i=0; i<count; i++) {
			proto_field = Proto_FindField(proto, i);
			ProtoField_GetField(proto_field, &field);

			/*quant*/
			if (useQuant && ( (field.eventType == ET_Field) || (field.eventType == ET_ExposedField) )) {
				QP_Type = BS_ReadInt(bs, 4);

				if (QP_Type==QC_LINEAR_SCALAR) {
					NumBits = BS_ReadInt(bs, 5);
				}
				hasMinMax = BS_ReadInt(bs, 1);
				qp_min_value = qp_max_value = NULL;
				if (hasMinMax) {
					/*parse min and max*/
					qpsftype = VRML_GetSFType(field.fieldType);
					switch (qpsftype) {
					case FT_SFInt32:
					case FT_SFTime:
						break;
					/*other fields are of elementary type SFFloat or shouldn't have min/max*/
					default:
						qpsftype = FT_SFFloat;
						break;
					}
					field.fieldType = qpsftype;
					
					qp_min_value = VRML_NewFieldPointer(qpsftype);
					field.name = "QPMinValue";
					field.far_ptr = qp_min_value;
					BD_DecSFField(codec, bs, NULL, &field);

					qp_max_value = VRML_NewFieldPointer(qpsftype);
					field.name = "QPMaxValue";
					field.far_ptr = qp_max_value;
					BD_DecSFField(codec, bs, NULL, &field);
				}

				/*and store*/
				if (QP_Type) {
					e = ProtoField_SetQuantizationInfo(proto_field, QP_Type, hasMinMax, qpsftype, qp_min_value, qp_max_value, NumBits);
					VRML_DeleteFieldPointer(qp_min_value, qpsftype);
					VRML_DeleteFieldPointer(qp_max_value, qpsftype);
				}
			}

			/*anim - not supported yet*/
			if (useAnim && ( (field.eventType == ET_EventIn) || (field.eventType == ET_ExposedField) )) {
				flag = BS_ReadInt(bs, 1);
				if (flag) {
					Anim_Type = BS_ReadInt(bs, 4);
				} else {
					Anim_Type = 0;
				}
			}
		}

		numProtos ++;

		/*4- get next proto*/
		flag = BS_ReadInt(bs, 1);
	}

exit:
	if (e) {
		if (proto) SG_DeleteProto(proto);
		codec->current_graph = rootSG;
	}
	/*restore original parent proto at codec level*/
	codec->pCurrentProto = ParentProto;
	return e;
}



M4Err BD_DecRoute(BifsDecoder * codec, BitStream *bs, Bool is_insert)
{
	M4Err e;
	u8 flag;
	LPROUTE r;
	SFNode *InNode, *OutNode;
	u32 RouteID, outField, inField, numBits, ind, node_id;
	char name[1000];

	RouteID = 0;

	flag = BS_ReadInt(bs, 1);
	/*def'ed route*/
	if (flag) {
		RouteID = 1 + BS_ReadInt(bs, codec->info->config.RouteIDBits);
		if (codec->info->UseName) BD_GetName(bs, name);
	}
	/*origin*/
	node_id = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	OutNode = BIFS_FindNode(codec, node_id);
	if (!OutNode) return M4InvalidNode;

	numBits = Node_GetNumFields(OutNode, FCM_OUT) - 1;
	numBits = GetNumBits(numBits);
	ind = BS_ReadInt(bs, numBits);
	e = Node_GetFieldIndex(OutNode, ind, FCM_OUT, &outField);

	/*target*/
	node_id = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	InNode = BIFS_FindNode(codec, node_id);
	if (!InNode) return M4InvalidNode;

	numBits = Node_GetNumFields(InNode, FCM_IN) - 1;
	numBits = GetNumBits(numBits);
	ind = BS_ReadInt(bs, numBits);
	e = Node_GetFieldIndex(InNode, ind, FCM_IN, &inField);
	if (e) return e;

	r = SG_NewRoute(codec->current_graph, OutNode, outField, InNode, inField);
	if (!r) return M4OutOfMem;
	if (RouteID) {
		e = SG_SetRouteID(r, RouteID);
		if (!e && codec->info->UseName) e = SG_SetRouteName(r, name);
	}
	return e;
}


M4Err BD_DecSceneReplace(BifsDecoder * codec, BitStream *bs, Chain *proto_list)
{
	u8 flag;
	u32 i, nbR;
	M4Err e;
	SFNode *root;

	/*Reset the existing scene / scene graph, protos and route lists*/
	SG_Reset(codec->current_graph);

	/*reserved*/
	i = BS_ReadInt(bs, 6);

	codec->info->UseName = BS_ReadInt(bs, 1);

	/*parse PROTOS*/
	e = BD_DecProtoList(codec, bs, proto_list);
	if (e) goto exit;

	assert(codec->pCurrentProto==NULL);
	/*Parse the top node - always of type SFTopNode*/
	root = BD_DecSFNode(codec, bs, NDT_SFTopNode);
	if (!root && codec->LastError) {
		e = codec->LastError;
		goto exit;
	}

	if (root) {
		e = Node_Register(root, NULL);
		if (e) goto exit;
	}
	SG_SetRootNode(codec->current_graph, root);

	/*Parse the routes*/
	flag = BS_ReadInt(bs, 1);

	
	if (flag) {
		flag = BS_ReadInt(bs, 1);
		if (flag) {
			/*list*/
			while (flag) {
				e = BD_DecRoute(codec, bs, 0);
				if (e) goto exit;
				flag = BS_ReadInt(bs, 1);
			}
		} else {
			/*vector*/
			i = BS_ReadInt(bs, 5);
			nbR = BS_ReadInt(bs, i);
			for (i=0; i<nbR; i++) {
				e = BD_DecRoute(codec, bs, 0);
				if (e) goto exit;
			}
		}
	}

exit:
	return e;
}

static u32 num_au = 0;

M4Err BIFS_ParseCommand(BifsDecoder * codec, BitStream *bs)
{
	u8 go, type;
	u32 count;
	M4Err e;
	go = 1;
	e = M4OK;

	codec->LastError = 0;
	count = 0;
	num_au++;

	while (go) {
		type = BS_ReadInt(bs, 2);
		switch (type) {
		case 0:
			e = BD_DecInsert(codec, bs);
			break;
		case 1:
			e = BD_DecDelete(codec, bs);
			break;
		case 2:
			e = BD_DecReplace(codec, bs);
			break;
		case 3:
			e = BD_DecSceneReplace(codec, bs, NULL);
			break;
		}
		if (e)
			return e;

		go = BS_ReadInt(bs, 1);
		count++;
	}
	while (ChainGetCount(codec->QPs)) {
		BD_RemoveQP(codec, 1);
	}
	return M4OK;
}

