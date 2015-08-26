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


#include <gpac/intern/m4_bifs_dev.h>
/*for scene replace tricks*/
#include <gpac/intern/m4_scenegraph_dev.h>
#include "Quantize.h"

M4Err ParseMFFieldList(LPBIFSDEC codec, BitStream *bs, SFNode *node, FieldInfo *field);
M4Err ParseMFFieldVec(LPBIFSDEC codec, BitStream *bs, SFNode *node, FieldInfo *field);


static void BM_SetCommandNode(SGCommand *com, SFNode *node)
{
	com->node = node;
	Node_Register(node, NULL);
}

static M4Err BM_ParseMultipleIndexedReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 ID, ind, field_ind, NumBits, lenpos, lennum, count;
	SFNode *node;
	M4Err e;
	SGCommand *com;
	CommandFieldInfo *inf;
	FieldInfo field;
	
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
	
	com = SG_NewCommand(codec->current_graph, SG_MultipleIndexedReplace);
	BM_SetCommandNode(com, node);
	field.fieldType = VRML_GetSFType(field.fieldType);

	while (count) {
		inf = SG_NewFieldCommand(com);
		inf->pos = BS_ReadInt(bs, lenpos);
		inf->fieldIndex = field.fieldIndex;
		inf->fieldType = field.fieldType;
		
		if (field.fieldType==FT_SFNode) {
			inf->new_node = BD_DecSFNode(codec, bs, field.NDTtype);
			if (codec->LastError) goto err;
			inf->field_ptr = &inf->new_node;
			Node_Register(inf->new_node, node);
		} else {
			field.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
			e = BD_DecSFField(codec, bs, node, &field);
			if (e) goto err;
		}
		count--;
	}
err:
	if (e) SG_DeleteCommand(com);
	else ChainAddEntry(com_list, com);
	return e;
}

static M4Err BM_ParseMultipleReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 i, numFields, index, flag, nbBits, field_ref, fieldind;
	M4Err e;
	FieldInfo field;
	u32 NodeID;
	SFNode *node;
	SGCommand *com;
	CommandFieldInfo *inf;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	
	e = M4OK;
	com = SG_NewCommand(codec->current_graph, SG_MultipleReplace);
	BM_SetCommandNode(com, node);
	flag = BS_ReadInt(bs, 1);
	if (flag) {
		numFields = Node_GetNumFields(node, FCM_DEF);
		for (i=0; i<numFields; i++) {
			flag = BS_ReadInt(bs, 1);
			if (!flag) continue;
			Node_GetFieldIndex(node, i, FCM_DEF, &index);
			e = Node_GetField(node, index, &field);
			if (e) goto exit;
			inf = SG_NewFieldCommand(com);
			inf->fieldType = field.fieldType;
			inf->fieldIndex = field.fieldIndex;
			if (inf->fieldType==FT_SFNode) {
				field.far_ptr = inf->field_ptr = &inf->new_node;
			} else if (inf->fieldType==FT_MFNode) {
				inf->node_list = NewChain();
				field.far_ptr = inf->field_ptr = &inf->node_list;
			} else {
				field.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
			}
			e = BD_DecField(codec, bs, node, &field);
			if (e) goto exit;
		}
	} else {
		flag = BS_ReadInt(bs, 1);
		nbBits = GetNumBits(Node_GetNumFields(node, FCM_DEF)-1);
		while (!flag) {
			field_ref = BS_ReadInt(bs, nbBits);
			e = Node_GetFieldIndex(node, field_ref, FCM_DEF, &fieldind);
			if (e) goto exit;
			e = Node_GetField(node, fieldind, &field);
			if (e) goto exit;
			inf = SG_NewFieldCommand(com);
			inf->fieldType = field.fieldType;
			inf->fieldIndex = field.fieldIndex;
			if (inf->fieldType==FT_SFNode) {
				field.far_ptr = inf->field_ptr = &inf->new_node;
			} else if (inf->fieldType==FT_MFNode) {
				inf->node_list = NewChain();
				field.far_ptr = inf->field_ptr = &inf->node_list;
			} else {
				field.far_ptr = inf->field_ptr = VRML_NewFieldPointer(inf->fieldType);
			}
			e = BD_DecField(codec, bs, node, &field);
			if (e) goto exit;
			flag = BS_ReadInt(bs, 1);
		}
	}

	
exit:
	if (e) SG_DeleteCommand(com);
	else ChainAddEntry(com_list, com);
	return e;
}

static M4Err BM_ParseGlobalQuantizer(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	SFNode *node;
	SGCommand *com;
	CommandFieldInfo *inf;
	node = BD_DecSFNode(codec, bs, NDT_SFWorldNode);

	/*reset global QP*/
	if (codec->GlobalQP) Node_Unregister((SFNode *) codec->GlobalQP, NULL);
	codec->GlobalQP = codec->ActiveQP = NULL;
	
	if (node && (Node_GetTag(node) != TAG_MPEG4_QuantizationParameter)) {
		Node_Unregister(node, NULL);
		return M4NonCompliantBitStream;
	}

	/*register global QP*/
	codec->GlobalQP = codec->ActiveQP = (M_QuantizationParameter *) node;
	codec->GlobalQP->isLocal = 0;
	if (node) Node_Register(node, NULL);
	com = SG_NewCommand(codec->current_graph, SG_GlobalQuantizer);
	inf = SG_NewFieldCommand(com);
	inf->new_node = node;
	inf->field_ptr = &inf->new_node;
	inf->fieldType = FT_SFNode;
	ChainAddEntry(com_list, com);
	return M4OK;
}

static M4Err BM_ParseProtoDelete(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 flag, count;
	SGCommand *com = SG_NewCommand(codec->current_graph, SG_ProtoDelete);
	flag = BS_ReadInt(bs, 1);
	if (flag) {
		count = 0;
		flag = BS_ReadInt(bs, 1);
		while (flag) {
			com->del_proto_list = realloc(com->del_proto_list, sizeof(u32) * (com->del_proto_list_size+1));
			com->del_proto_list[count] = BS_ReadInt(bs, codec->info->config.ProtoIDBits);
			com->del_proto_list_size++;
			flag = BS_ReadInt(bs, 1);
		}
	} else {
		flag = BS_ReadInt(bs, 5);
		com->del_proto_list_size = BS_ReadInt(bs, flag);
		com->del_proto_list = realloc(com->del_proto_list, sizeof(u32) * (com->del_proto_list_size));
		flag = 0;
		while (flag<com->del_proto_list_size) {
			com->del_proto_list[flag] = BS_ReadInt(bs, codec->info->config.ProtoIDBits);
			flag++;
		}
	}
	ChainAddEntry(com_list, com);
	return M4OK;
}

static M4Err BM_ParseExtendedUpdates(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 type = BS_ReadInt(bs, 8);
	M4Err e;

	switch (type) {
	case 0:
	{
		SGCommand *com = SG_NewCommand(codec->current_graph, SG_ProtoInsert);
		e = BD_DecProtoList(codec, bs, com->new_proto_list);
		if (e) SG_DeleteCommand(com);
		else ChainAddEntry(com_list, com);
	}
		return e;
	case 1:
		return BM_ParseProtoDelete(codec, bs, com_list);
	case 2:
	{
		SGCommand *com = SG_NewCommand(codec->current_graph, SG_ProtoDeleteAll);
		return ChainAddEntry(com_list, com);
	}
	case 3:
		return BM_ParseMultipleIndexedReplace(codec, bs, com_list);
	case 4:
		return BM_ParseMultipleReplace(codec, bs, com_list);
	case 5:
		return BM_ParseGlobalQuantizer(codec, bs, com_list);
	case 6:
	{
		SGCommand *com;
		u32 ID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
		SFNode *n = BIFS_FindNode(codec, ID);
		if (!n) return M4OK;
		com = SG_NewCommand(codec->current_graph, SG_NodeDeleteEx);
		BM_SetCommandNode(com, n);
		ChainAddEntry(com_list, com);
	}
		return M4OK;
	default:
		return M4UnknowBIFSVersion;
	}
}

/*inserts a node in a container (node.children)*/
M4Err BM_ParseNodeInsert(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 NodeID, NDT;
	SGCommand *com;
	CommandFieldInfo *inf;
	s32 type, pos;
	SFNode *node, *def;

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
	if (!codec->LastError) {
		com = SG_NewCommand(codec->current_graph, SG_NodeInsert);
		BM_SetCommandNode(com, def);
		inf = SG_NewFieldCommand(com);
		inf->pos = pos;
		inf->new_node = node;
		inf->field_ptr = &inf->new_node;
		inf->fieldType = FT_SFNode;
		ChainAddEntry(com_list, com);
		/*register*/
		Node_Register(node, def);
	}
	return codec->LastError;
}

/*NB This can insert a node as well (but usually not in the .children field)*/
M4Err BM_ParseIndexInsert(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	M4Err e;
	u32 NodeID;
	u32 NumBits, ind, field_ind;
	u8 type;
	SGCommand *com;
	CommandFieldInfo *inf;
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
		if (!codec->LastError) {
			com = SG_NewCommand(codec->current_graph, SG_IndexedInsert);
			BM_SetCommandNode(com, def);
			inf = SG_NewFieldCommand(com);
			inf->pos = pos;
			inf->fieldIndex = field_ind;
			inf->fieldType = sffield.fieldType;
			inf->new_node = node;
			inf->field_ptr = &inf->new_node;
			ChainAddEntry(com_list, com);
			/*register*/
			Node_Register(node, def);
		}
	} else {
		com = SG_NewCommand(codec->current_graph, SG_IndexedInsert);
		BM_SetCommandNode(com, def);
		inf = SG_NewFieldCommand(com);
		inf->pos = pos;
		inf->fieldIndex = field_ind;
		inf->fieldType = sffield.fieldType;
		sffield.far_ptr = inf->field_ptr = VRML_NewFieldPointer(sffield.fieldType);
		codec->LastError = BD_DecSFField(codec, bs, def, &sffield);
		ChainAddEntry(com_list, com);
	}
	return codec->LastError;
}


M4Err BM_ParseRouteInsert(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	M4Err e;
	u8 flag;
	SGCommand *com;
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

	com = SG_NewCommand(codec->current_graph, SG_RouteInsert);
	com->RouteID = RouteID;
	if (codec->info->UseName) com->def_name = strdup( name);
	com->fromNodeID = Node_GetID(OutNode);
	com->fromFieldIndex = outField;
	com->toNodeID = Node_GetID(InNode);
	com->toFieldIndex = inField;
	ChainAddEntry(com_list, com);
	return codec->LastError;
}


M4Err BM_ParseInsert(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u8 type;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		return BM_ParseNodeInsert(codec, bs, com_list);
	case 1:
		return BM_ParseExtendedUpdates(codec, bs, com_list);
	case 2:
		return BM_ParseIndexInsert(codec, bs, com_list);
	case 3:
		return BM_ParseRouteInsert(codec, bs, com_list);
	default:
		return M4NonCompliantBitStream;
	}
}


M4Err BM_ParseIndexDelete(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 NodeID, NumBits, ind, field_ind;
	s32 pos;
	SGCommand *com;
	u8 type;
	SFNode *node;
	M4Err e;
	CommandFieldInfo *inf;
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
	com = SG_NewCommand(codec->current_graph, SG_IndexedDelete);
	BM_SetCommandNode(com, node);
	inf = SG_NewFieldCommand(com);
	inf->pos = pos;
	inf->fieldIndex = field.fieldIndex;
	inf->fieldType = VRML_GetSFType(field.fieldType);
	ChainAddEntry(com_list, com);
	return codec->LastError;
}



M4Err BM_ParseDelete(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u8 type;
	u32 ID;
	SGCommand *com;
	SFNode *n;

	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		ID = 1+BS_ReadInt(bs, codec->info->config.NodeIDBits);
		n = BIFS_FindNode(codec, ID);
		if (!n) return M4OK;
		com = SG_NewCommand(codec->current_graph, SG_NodeDelete);
		BM_SetCommandNode(com, n);
		ChainAddEntry(com_list, com);
		return M4OK;
	case 2:
		return BM_ParseIndexDelete(codec, bs, com_list);
	case 3:
		com = SG_NewCommand(codec->current_graph, SG_RouteDelete);
		com->RouteID = 1+BS_ReadInt(bs, codec->info->config.RouteIDBits);
		ChainAddEntry(com_list, com);
		return M4OK;
	default:
		return M4NonCompliantBitStream;
	}
	return M4OK;
}


M4Err BM_ParseNodeReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 NodeID;
	SGCommand *com;
	SFNode *node;
	CommandFieldInfo *inf;
	
	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	/*this is delete / new on a DEF node: replace ALL instances*/
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;

	com = SG_NewCommand(codec->current_graph, SG_NodeReplace);
	BM_SetCommandNode(com, node);
	inf = SG_NewFieldCommand(com);
	inf->new_node = BD_DecSFNode(codec, bs, NDT_SFWorldNode);
	inf->fieldType = FT_SFNode;
	inf->field_ptr = &inf->new_node;
	ChainAddEntry(com_list, com);
	Node_Register(inf->new_node, NULL);
	return codec->LastError;
}

M4Err BM_ParseFieldReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	M4Err e;
	SGCommand *com;
	u32 NodeID, ind, field_ind, NumBits;
	SFNode *node;
	FieldInfo field;
	CommandFieldInfo *inf;

	NodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
	node = BIFS_FindNode(codec, NodeID);
	if (!node) return M4NonCompliantBitStream;
	NumBits = GetNumBits(Node_GetNumFields(node, FCM_IN)-1);
	ind = BS_ReadInt(bs, NumBits);
	e = Node_GetFieldIndex(node, ind, FCM_IN, &field_ind);
	if (e) return e;

	e = Node_GetField(node, field_ind, &field);

	com = SG_NewCommand(codec->current_graph, SG_FieldReplace);
	BM_SetCommandNode(com, node);
	inf = SG_NewFieldCommand(com);
	inf->fieldIndex = field_ind;
	inf->fieldType = field.fieldType;
	if (inf->fieldType == FT_SFNode) {
		field.far_ptr = inf->field_ptr = &inf->new_node;
	} else if (inf->fieldType == FT_MFNode) {
		inf->node_list = NewChain();
		field.far_ptr = inf->field_ptr = &inf->node_list;
	} else {
		field.far_ptr = inf->field_ptr = VRML_NewFieldPointer(field.fieldType);
	}
	/*parse the field*/
	codec->LastError = BD_DecField(codec, bs, node, &field);

	/*register nodes*/
	if (inf->fieldType == FT_SFNode) {
		Node_Register(inf->new_node, com->node);
	} else if (inf->fieldType == FT_MFNode) {
		u32 i;
		for (i=0; i<ChainGetCount(inf->node_list); i++) {
			SFNode *p = ChainGetEntry(inf->node_list, i);
			Node_Register(p, com->node);
		}
	}

	ChainAddEntry(com_list, com);
	return codec->LastError;
}

M4Err BM_ParseIndexValueReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u32 NodeID, ind, field_ind, NumBits;
	s32 type, pos;
	SGCommand *com;
	SFNode *node;
	M4Err e;
	FieldInfo field, sffield;
	CommandFieldInfo *inf;

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
	
	com = SG_NewCommand(codec->current_graph, SG_IndexedReplace);
	BM_SetCommandNode(com, node);
	inf = SG_NewFieldCommand(com);
	inf->fieldIndex = field.fieldIndex;
	inf->pos = pos;

	if (field.fieldType == FT_MFNode) {
		inf->fieldType = FT_SFNode;
		inf->new_node = BD_DecSFNode(codec, bs, field.NDTtype);
		inf->field_ptr = &inf->new_node;
		Node_Register(inf->new_node, com->node);
	} else {
		memcpy(&sffield, &field, sizeof(FieldInfo));
		sffield.fieldType = VRML_GetSFType(field.fieldType);
		inf->fieldType = sffield.fieldType;
		sffield.far_ptr = inf->field_ptr = VRML_NewFieldPointer(sffield.fieldType);
		codec->LastError = BD_DecSFField(codec, bs, node, &sffield);
	}
	ChainAddEntry(com_list, com);
	return codec->LastError;
}

u32 BM_ParseRouteReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	M4Err e;
	SGCommand *com;
	u32 RouteID, numBits, ind, node_id, fromID, toID;
	LPROUTE r;
	SFNode *OutNode, *InNode;

	RouteID = 1+BS_ReadInt(bs, codec->info->config.RouteIDBits);
	
	r = SG_FindRoute(codec->current_graph, RouteID);

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

	com = SG_NewCommand(codec->current_graph, SG_RouteReplace);
	com->RouteID = RouteID;
	com->fromNodeID = Node_GetID(OutNode);
	com->fromFieldIndex = fromID;
	com->toNodeID = Node_GetID(InNode);
	com->toFieldIndex = toID;
	ChainAddEntry(com_list, com);
	return codec->LastError;
}


M4Err BM_ParseReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u8 type;
	type = BS_ReadInt(bs, 2);
	switch (type) {
	case 0:
		return BM_ParseNodeReplace(codec, bs, com_list);
	case 1:
		return BM_ParseFieldReplace(codec, bs, com_list);
	case 2:
		return BM_ParseIndexValueReplace(codec, bs, com_list);
	case 3:
		return BM_ParseRouteReplace(codec, bs, com_list);
	}
	return M4OK;
}

M4Err BM_SceneReplace(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	SGCommand *com;
	SFNode *backup_root;
	Chain *backup_routes;
	M4Err BD_DecSceneReplace(BifsDecoder * codec, BitStream *bs, Chain *proto_list);

	backup_routes = codec->scenegraph->Routes;
	backup_root = codec->scenegraph->RootNode;
	com = SG_NewCommand(codec->current_graph, SG_SceneReplace);
	codec->scenegraph->Routes = NewChain();
	codec->current_graph = codec->scenegraph;
	codec->LastError = BD_DecSceneReplace(codec, bs, com->new_proto_list);

	/*restore*/
	com->node = codec->scenegraph->RootNode;
	codec->scenegraph->RootNode = backup_root;
	ChainAddEntry(com_list, com);
	/*insert routes*/
	while (ChainGetCount(codec->scenegraph->Routes)) {
		LPROUTE r = ChainGetEntry(codec->scenegraph->Routes, 0);
		SGCommand *ri = SG_NewCommand(codec->current_graph, SG_RouteInsert);
		ChainDeleteEntry(codec->scenegraph->Routes, 0);
		ri->fromFieldIndex = r->FromFieldIndex;
		ri->fromNodeID = r->FromNode->sgprivate->NodeID;
		ri->toFieldIndex = r->ToFieldIndex;
		ri->toNodeID = r->ToNode->sgprivate->NodeID;
		if (r->ID) ri->RouteID = r->ID;
		ri->def_name = r->name ? strdup(r->name) : NULL;
		ChainAddEntry(com_list, ri);
		SG_DeleteRoute(r);
	}
	DeleteChain(codec->scenegraph->Routes);
	codec->scenegraph->Routes = backup_routes;
	return codec->LastError;
}


M4Err BM_ParseCommand(LPBIFSDEC codec, BitStream *bs, Chain *com_list)
{
	u8 go, type;
	u32 count;
	M4Err e;
	go = 1;
	e = M4OK;

	codec->LastError = 0;
	count = 0;

	while (go) {
		type = BS_ReadInt(bs, 2);
		switch (type) {
		case 0:
			e = BM_ParseInsert(codec, bs, com_list);
			break;
		case 1:
			e = BM_ParseDelete(codec, bs, com_list);
			break;
		case 2:
			e = BM_ParseReplace(codec, bs, com_list);
			break;
		case 3:
			e = BM_SceneReplace(codec, bs, com_list);
			break;
		}
		if (e) return e;

		go = BS_ReadInt(bs, 1);
		count++;

	}
	while (ChainGetCount(codec->QPs)) {
		BD_RemoveQP(codec, 1);
	}
	return M4OK;
}

void BM_EndOfStream(void *co)
{
	((BifsDecoder *) co)->LastError = M4IOErr;
}

void BS_SetEOSCallback(BitStream *bs, void (*EndOfStream)(void *par), void *par);

M4Err BIFS_DecodeAUMemory(LPBIFSDEC codec, u16 ESID, char *data, u32 data_length, Chain *com_list)
{
	BitStream *bs;
	M4Err e;

	if (!codec || !data || !codec->dec_memory_mode || !com_list) return M4BadParam;

	MX_P(codec->mx);
	codec->info = BD_GetStream(codec, ESID);
	if (!codec->info || !codec->info->config.IsCommandStream) {
		MX_V(codec->mx);
		return M4BadParam;
	}
	/*root parse (not conditionals)*/
	assert(codec->scenegraph);
	/*setup current scene graph*/
	codec->current_graph = codec->scenegraph;

	bs = NewBitStream(data, data_length, BS_READ);
	BS_SetEOSCallback(bs, BM_EndOfStream, codec);

	e = BM_ParseCommand(codec, bs, com_list);
	DeleteBitStream(bs);

	/*decode conditionals / input sensors*/
	if (!e) {
		SFNode *n;
		SFCommandBuffer *c_bfr;
		u32 NbPass = ChainGetCount(codec->conditionals);
		Chain *nextPass = NewChain();
		while (NbPass) {
			while (ChainGetCount(codec->conditionals)) {
				n = ChainGetEntry(codec->conditionals, 0);
				ChainDeleteEntry(codec->conditionals, 0);
				codec->current_graph = Node_GetParentGraph((SFNode *)n);
				c_bfr = NULL;
				switch (Node_GetTag(n)) {
				case TAG_MPEG4_Conditional: c_bfr = & ((M_Conditional *)n)->buffer; break;
				case TAG_MPEG4_InputSensor: c_bfr = & ((M_InputSensor *)n)->buffer; break;
				}
				assert(c_bfr);
				bs = NewBitStream(c_bfr->buffer, c_bfr->bufferSize, BS_READ);
				BS_SetEOSCallback(bs, BM_EndOfStream, codec);

				e = BM_ParseCommand(codec, bs, c_bfr->commandList);
				DeleteBitStream(bs);
				if (!e) continue;
				/*this may be an error or a dependency pb - reset coimmand list and move to next pass*/
				while (ChainGetCount(c_bfr->commandList)) {
					SGCommand *com = ChainGetEntry(c_bfr->commandList, 0);
					ChainDeleteEntry(c_bfr->commandList, 0);
					SG_DeleteCommand(com);
				}
				ChainAddEntry(nextPass, n);
			}
			if (!ChainGetCount(nextPass)) break;
			/*prepare next pass*/
			while (ChainGetCount(nextPass)) {
				n = ChainGetEntry(nextPass, 0);
				ChainDeleteEntry(nextPass, 0);
				ChainAddEntry(codec->conditionals, n);
			}
			NbPass --;
			if (NbPass > ChainGetCount(codec->conditionals)) NbPass = ChainGetCount(codec->conditionals);
		}
		DeleteChain(nextPass);
	}
	/*if err or not reset conditionals*/
	while (ChainGetCount(codec->conditionals)) ChainDeleteEntry(codec->conditionals, 0);

	/*reset current config*/
	codec->info = NULL;
	codec->current_graph = NULL;



	MX_V(codec->mx);
	return e;
}
