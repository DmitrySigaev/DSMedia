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


#include <intern/m4_bifs_dev.h>
#include <intern/m4_scenegraph_dev.h>
#include "Quantize.h"

M4Err BE_EncProtoList(BifsEncoder *codec, Chain *protoList, BitStream *bs);


void BE_SetName(BifsEncoder *codec, BitStream *bs, char *name)
{
	u32 i = 0;
	while (name[i]) {
		BS_WriteInt(bs, name[i], 8);
		i++;
	}
	BS_WriteInt(bs, 0, 8);
	if (codec->trace) fprintf(codec->trace, "DEF_name\t\t%d\t\t%s\n", 8*i, name);
}


static M4Err BE_MultipleIndexedReplace(BifsEncoder * codec, SGCommand *com, BitStream *bs)
{
	u32 i,nbBits, count, maxPos, nbBitsPos;
	FieldInfo field;
	M4Err e;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK; 
	inf = ChainGetEntry(com->command_fields, 0);

	BS_WriteInt(bs, com->node->sgprivate->NodeID-1, codec->info->config.NodeIDBits);
	nbBits = GetNumBits(Node_GetNumFields(com->node, FCM_IN)-1);
	BIFS_ModeFieldIndex(com->node, inf->fieldIndex, FCM_IN, &i);
	BE_WRITE_INT(codec, bs, i, nbBits, "field", NULL);

	Node_GetField(com->node, inf->fieldIndex, &field);
	field.fieldType = inf->fieldType;

	count = ChainGetCount(com->command_fields);
	maxPos = 0;
	for (i=0; i<count; i++) {
		inf = ChainGetEntry(com->command_fields, i);
		if (maxPos < (u32) inf->pos) maxPos = inf->pos;
	}
	nbBitsPos = GetNumBits(maxPos);
	BE_WRITE_INT(codec, bs, nbBitsPos, 5, "nbBitsPos", NULL);
	
	nbBits = GetNumBits(count);
	BE_WRITE_INT(codec, bs, nbBits, 5, "nbBits", NULL);
	BE_WRITE_INT(codec, bs, count, nbBits, "count", NULL);

	for (i=0; i<count; i++) {
		inf = ChainGetEntry(com->command_fields, i);
		BE_WRITE_INT(codec, bs, inf->pos, nbBitsPos, "idx", NULL);
		field.far_ptr = inf->field_ptr;
		e = BE_EncField(codec, bs, com->node, &field);
		if (e) return e;
	}
	return M4OK;
}

static M4Err BE_MultipleReplace(BifsEncoder * codec, SGCommand *com, BitStream *bs)
{
	u32 i, j, nbBits, count, numFields, allField;
	Bool use_list;
	FieldInfo field;
	M4Err e;

	BS_WriteInt(bs, com->node->sgprivate->NodeID-1, codec->info->config.NodeIDBits);

	count = ChainGetCount(com->command_fields);
	use_list = 1;
	numFields = Node_GetNumFields(com->node, FCM_DEF);
	nbBits = GetNumBits(numFields - 1);
	if (count < 1+count*(1+nbBits)) use_list = 0;
	BE_WRITE_INT(codec, bs, use_list ? 0 : 1, 1, "isMask", NULL);

	for (i=0; i<numFields; i++) {
		CommandFieldInfo *inf = NULL;
		Node_GetFieldIndex(com->node, i, FCM_DEF, &allField);
		for (j=0; j<count; j++) {
			inf = ChainGetEntry(com->command_fields, j);
			if (inf->fieldIndex==allField) break;
			inf = NULL;
		}
		if (!inf) {
			if (!use_list) BE_WRITE_INT(codec, bs, 0, 1, "Mask", NULL);
			continue;
		}
		/*common case*/
		Node_GetField(com->node, inf->fieldIndex, &field);
		if (use_list) {
			/*not end flag*/
			BE_WRITE_INT(codec, bs, 0, 1, "end", NULL);
		} else {
			/*mask flag*/
			BE_WRITE_INT(codec, bs, 1, 1, "Mask", NULL);
		}
		if (use_list) BE_WRITE_INT(codec, bs, i, nbBits, "field", (char*)field.name);
		field.far_ptr = inf->field_ptr;
		e = BE_EncField(codec, bs, com->node, &field);
		if (e) return e;
	}
	/*end flag*/
	if (use_list) BE_WRITE_INT(codec, bs, 1, 1, "end", NULL);
	return M4OK;
}

static M4Err BE_GlobalQuantizer(BifsEncoder * codec, SGCommand *com, BitStream *bs)
{
	M4Err e;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);
	if (inf->new_node) ((M_QuantizationParameter *)inf->new_node)->isLocal = 0;
	e = BE_EncSFNode(codec, inf->new_node, NDT_SFWorldNode, bs);
	if (e) return e;

	/*reset global QP*/
	if (codec->GlobalQP) Node_Unregister((SFNode *) codec->GlobalQP, NULL);
	codec->GlobalQP = codec->ActiveQP = NULL;
	
	/*no QP*/
	if (!inf->new_node) return M4OK;

	/*register global QP*/
	codec->GlobalQP = codec->ActiveQP = (M_QuantizationParameter *) inf->new_node;
	codec->GlobalQP->isLocal = 0;
	Node_Register(inf->new_node, NULL);
	return M4OK;
}

static M4Err BE_EncProtoDelete(BifsEncoder * codec, SGCommand *com, BitStream *bs)
{
	u32 nbBits, i;
	Bool use_list = 0;
	nbBits = GetNumBits(com->del_proto_list_size);
	if (nbBits+5>com->del_proto_list_size) use_list = 1;
	BE_WRITE_INT(codec, bs, use_list, 1, "isList", NULL);
	if (!use_list) {
		BE_WRITE_INT(codec, bs, nbBits, 5, "len", NULL);
		BE_WRITE_INT(codec, bs, com->del_proto_list_size, nbBits, "len", NULL);
	}
	for (i=0; i<com->del_proto_list_size; i++) {
		if (use_list) BE_WRITE_INT(codec, bs, 1, 1, "moreProto", NULL);
		BE_WRITE_INT(codec, bs, com->del_proto_list[i], codec->info->config.ProtoIDBits, "protoID", NULL);
	}
	if (use_list) BE_WRITE_INT(codec, bs, 0, 1, "moreProto", NULL);
	return M4OK;
}

static M4Err BE_ExtendedUpdate(BifsEncoder * codec, SGCommand *com, BitStream *bs)
{
	BE_WRITE_INT(codec, bs, 0, 2, "Insert", NULL);
	BE_WRITE_INT(codec, bs, 1, 2, "ExtendedUpdate", NULL);
	switch (com->tag) {
	case SG_ProtoInsert:
		BE_WRITE_INT(codec, bs, 0, 8, "MultipleReplace", NULL);
		return BE_EncProtoList(codec, com->new_proto_list, bs);
	case SG_ProtoDelete:
		BE_WRITE_INT(codec, bs, 1, 8, "ProtoDelete", NULL);
		return BE_EncProtoDelete(codec, com, bs);
	case SG_ProtoDeleteAll:
		BE_WRITE_INT(codec, bs, 2, 8, "DeleteAllProtos", NULL);
		return M4OK;
	case SG_MultipleIndexedReplace:
		BE_WRITE_INT(codec, bs, 3, 8, "MultipleReplace", NULL);
		return BE_MultipleIndexedReplace(codec, com, bs);
	case SG_MultipleReplace:
		BE_WRITE_INT(codec, bs, 4, 8, "MultipleReplace", NULL);
		return BE_MultipleReplace(codec, com, bs);
	case SG_GlobalQuantizer:
		BE_WRITE_INT(codec, bs, 5, 8, "GlobalQuantizer", NULL);
		return BE_GlobalQuantizer(codec, com, bs);
	case SG_NodeDeleteEx:
		BE_WRITE_INT(codec, bs, 6, 8, "MultipleReplace", NULL);
		BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);
		return M4OK;
	default:
		return M4BadParam;
	}
}

/*inserts a node in a container (node.children)*/
M4Err BE_NodeInsert(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	u32 NDT;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);

	NDT = Node_GetChildTable(com->node);

	switch (inf->pos) {
	case 0:
		BE_WRITE_INT(codec, bs, 2, 2, "FIRST", "idx");
		break;
	case -1:
		BE_WRITE_INT(codec, bs, 3, 2, "LAST", "idx");
		break;
	default:
		BE_WRITE_INT(codec, bs, 0, 2, "pos", "idx");
		BE_WRITE_INT(codec, bs, inf->pos, 8, "pos", NULL);
		break;
	}
	return BE_EncSFNode(codec, inf->new_node, NDT, bs);
}

M4Err BE_IndexInsert(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	M4Err e;
	u32 NumBits, ind;
	FieldInfo field, sffield;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);

	/*index insertion uses IN mode for field index*/
	NumBits = GetNumBits(Node_GetNumFields(com->node, FCM_IN)-1);
	BIFS_ModeFieldIndex(com->node, inf->fieldIndex, FCM_IN, &ind);
	BE_WRITE_INT(codec, bs, ind, NumBits, "field", NULL);

	switch (inf->pos) {
	case 0:
		BE_WRITE_INT(codec, bs, 2, 2, "FIRST", "idx");
		break;
	case -1:
		BE_WRITE_INT(codec, bs, 3, 2, "LAST", "idx");
		break;
	default:
		BE_WRITE_INT(codec, bs, 0, 2, "pos", "idx");
		BE_WRITE_INT(codec, bs, inf->pos, 16, "pos", NULL);
		break;
	}
	e = Node_GetField(com->node, inf->fieldIndex, &field);
	if (e) return e;
	if (VRML_IsSFField(field.fieldType)) 
		return M4NonCompliantBitStream;

	memcpy(&sffield, &field, sizeof(FieldInfo));
	sffield.fieldType = VRML_GetSFType(field.fieldType);
	sffield.far_ptr = inf->field_ptr;

	/*rescale the MFField and parse the SFField*/
	if (field.fieldType==FT_MFNode) {
		return BE_EncSFNode(codec, inf->new_node, field.NDTtype, bs);
	} else {
		return BE_EncSFField(codec, bs, com->node, &sffield);
	}
}


M4Err BE_IndexDelete(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	u32 NumBits, ind;
	M4Err e;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);

	NumBits = GetNumBits(Node_GetNumFields(com->node, FCM_IN) - 1);
	e = BIFS_ModeFieldIndex(com->node, inf->fieldIndex, FCM_IN, &ind);
	if (e) return e;
	BE_WRITE_INT(codec, bs, ind, NumBits, "field", NULL);

	switch (inf->pos) {
	case 0:
		BE_WRITE_INT(codec, bs, 2, 2, "FIRST", "idw");
		break;
	case -1:
		BE_WRITE_INT(codec, bs, 3, 2, "LAST", "idx");
		break;
	default:
		BE_WRITE_INT(codec, bs, 0, 2, "pos", "idx");
		BE_WRITE_INT(codec, bs, inf->pos, 16, "pos", NULL);
		break;
	}
	return M4OK;
}


M4Err BE_NodeReplace(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);
	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);
	return BE_EncSFNode(codec, inf->new_node, NDT_SFWorldNode, bs);
}

M4Err BE_FieldReplace(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	M4Err e;
	u32 ind, NumBits;
	FieldInfo field;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);

	NumBits = GetNumBits(Node_GetNumFields(com->node, FCM_IN)-1);
	BIFS_ModeFieldIndex(com->node, inf->fieldIndex, FCM_IN, &ind);
	BE_WRITE_INT(codec, bs, ind, NumBits, "field", NULL);

	e = Node_GetField(com->node, inf->fieldIndex, &field);
	if (e) return e;
	field.far_ptr = inf->field_ptr;
	return BE_EncField(codec, bs, com->node, &field);
}

M4Err BE_IndexFieldReplace(BifsEncoder *codec, SGCommand *com, BitStream *bs)
{
	u32 ind, NumBits;
	M4Err e;
	FieldInfo field, sffield;
	CommandFieldInfo *inf;
	if (!ChainGetCount(com->command_fields)) return M4OK;
	inf = ChainGetEntry(com->command_fields, 0);

	BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);
	NumBits = GetNumBits(Node_GetNumFields(com->node, FCM_IN)-1);
	BIFS_ModeFieldIndex(com->node, inf->fieldIndex, FCM_IN, &ind);
	BE_WRITE_INT(codec, bs, ind, NumBits, "field", NULL);

	e = Node_GetField(com->node, inf->fieldIndex, &field);
	if (VRML_IsSFField(field.fieldType)) 
		return M4NonCompliantBitStream;
	
	switch (inf->pos) {
	case 0:
		BE_WRITE_INT(codec, bs, 2, 2, "FIRST", "idx");
		break;
	case -1:
		BE_WRITE_INT(codec, bs, 3, 2, "LAST", "idx");
		break;
	default:
		BE_WRITE_INT(codec, bs, 0, 2, "pos", "idx");
		BE_WRITE_INT(codec, bs, inf->pos, 16, "pos", NULL);
		break;
	}
	
	if (field.fieldType == FT_MFNode) {
		e = BE_EncSFNode(codec, inf->new_node, field.NDTtype, bs);
	} else {
		memcpy(&sffield, &field, sizeof(FieldInfo));
		sffield.fieldType = VRML_GetSFType(field.fieldType);
		sffield.far_ptr = inf->field_ptr;
		e = BE_EncSFField(codec, bs, com->node, &sffield);
	}
	return e;
}

M4Err BE_RouteReplace(BifsEncoder *codec, SGCommand *com, BitStream *bs, Bool isInsert)
{
	M4Err e;
	SFNode *n;
	u32 numBits, ind;

	if (isInsert) {
		BE_WRITE_INT(codec, bs, com->RouteID ? 1 : 0, 1, "isDEF", NULL);
		if (com->RouteID) {
			BE_WRITE_INT(codec, bs, com->RouteID-1, codec->info->config.RouteIDBits, "RouteID", NULL);
			if (codec->info->UseName) BE_SetName(codec, bs, com->def_name);
		}
	} else {
		BE_WRITE_INT(codec, bs, com->RouteID - 1, codec->info->config.RouteIDBits, "RouteID", NULL);
	}

	/*origin*/
	BE_WRITE_INT(codec, bs, com->fromNodeID - 1, codec->info->config.NodeIDBits, "outNodeID", NULL);
	n = BE_FindNode(codec, com->fromNodeID);
	numBits = Node_GetNumFields(n, FCM_OUT) - 1;
	numBits = GetNumBits(numBits);
	e = BIFS_ModeFieldIndex(n, com->fromFieldIndex, FCM_OUT, &ind);
	if (e) return e;
	BE_WRITE_INT(codec, bs, ind, numBits, "outField", NULL);

	/*target*/
	BE_WRITE_INT(codec, bs, com->toNodeID - 1, codec->info->config.NodeIDBits, "inNodeID", NULL);
	n = BE_FindNode(codec, com->toNodeID);
	numBits = Node_GetNumFields(n, FCM_IN) - 1;
	numBits = GetNumBits(numBits);
	e = BIFS_ModeFieldIndex(n, com->toFieldIndex, FCM_IN, &ind);
	BE_WRITE_INT(codec, bs, ind, numBits, "inField", NULL);
	return e;
}


M4Err BE_EncProtoList(BifsEncoder *codec, Chain *protoList, BitStream *bs)
{
	u8 useQuant, useAnim;
	u32 i, j, nbRoutes, nbBits, numProtos, numFields, count;
	SFNode *node;
	LPPROTOFIELD proto_field;
	LPPROTO proto, prev_proto;
	LPROUTE r;
	M4Err e;
	LPSCENEGRAPH rootSG;
	FieldInfo field;

	e = M4OK;
	if (!protoList || !ChainGetCount(protoList)) {
		BE_WRITE_INT(codec, bs, 0, 1, "moreProto", NULL);
		return M4OK;
	}
	if (!codec->info->config.ProtoIDBits) 
		return M4NonCompliantBitStream;

	/*store state*/
	rootSG = codec->current_proto_graph;
	prev_proto = codec->encoding_proto;

	numProtos = ChainGetCount(protoList);
	for (i=0; i<numProtos; i++) {
		proto = ChainGetEntry(protoList, i);
		useQuant = useAnim = 0;
		/*set current proto state*/
		codec->encoding_proto = proto;
		codec->current_proto_graph = proto->sub_graph;

		BE_WRITE_INT(codec, bs, 1, 1, "moreProto", NULL);

		/*1- proto interface declaration*/
		BE_WRITE_INT(codec, bs, proto->ID, codec->info->config.ProtoIDBits, "protoID", NULL);

		if (codec->info->UseName) BE_SetName(codec, bs, proto->Name);

		numFields = ChainGetCount(proto->proto_fields);
		if (numFields) {
			for (j=0; j<numFields; j++) {
				proto_field = ChainGetEntry(proto->proto_fields, j);

				BE_WRITE_INT(codec, bs, 1, 1, "moreField", NULL);
				BE_WRITE_INT(codec, bs, proto_field->EventType, 2, "eventType", NULL);
				BE_WRITE_INT(codec, bs, proto_field->FieldType, 6, "fieldType", NULL);
				
				if (codec->info->UseName) BE_SetName(codec, bs, proto_field->FieldName);
				switch (proto_field->EventType) {
				case ET_ExposedField:
				case ET_Field:
					ProtoField_GetField(proto_field, &field);
					if (VRML_IsSFField(field.fieldType)) {
						e = BE_EncSFField(codec, bs, NULL, &field);
					} else {
						if (codec->info->config.UsePredictiveMFField) BE_WRITE_INT(codec, bs, 0, 1, "usePredictive", NULL);
						e = BE_EncMFField(codec, bs, NULL, &field);
					}
					if (e) goto exit;
					break;
				}
				if (proto_field->QP_Type) useQuant = 1;
				if (proto_field->Anim_Type) useAnim = 1;
			}
			BE_WRITE_INT(codec, bs, 0, 1, "moreField", NULL);
		}
		
		BE_WRITE_INT(codec, bs, proto->ExternProto.count ? 1 : 0, 1, "externProto", NULL);
		/*externProto*/
		if (proto->ExternProto.count) {
			memset(&field, 0, sizeof(FieldInfo));
			field.far_ptr = &proto->ExternProto;
			field.fieldType = FT_MFURL;
			field.name = "ExternProto";

			if (codec->info->config.UsePredictiveMFField) BE_WRITE_INT(codec, bs, 0, 1, "usePredictive", NULL);
			e = BE_EncMFField(codec, bs, NULL, &field);
			if (e) goto exit;
		} else {
			/*encode sub-proto list*/
			e = BE_EncProtoList(codec, proto->sub_graph->protos, bs);
			if (e) goto exit;

			count = ChainGetCount(proto->node_code);
			for (j=0; j<count; j++) {
				/*parse all nodes in SFWorldNode table*/
				node = ChainGetEntry(proto->node_code, j);
				e = BE_EncSFNode(codec, node, NDT_SFWorldNode, bs);
				if (e) goto exit;
				BE_WRITE_INT(codec, bs, (j+1==count) ? 0 : 1, 1, "moreNodes", NULL);
			}
			
			/*encode routes routes*/
			nbRoutes = count = ChainGetCount(proto->sub_graph->Routes);
			for (j=0; j<count; j++) {
				r = ChainGetEntry(proto->sub_graph->Routes, j);
				if (r->IS_route) nbRoutes--;
			}

			BE_WRITE_INT(codec, bs, nbRoutes ? 1 : 0, 1, "hasRoute", NULL);
			if (nbRoutes) {
				nbBits = GetNumBits(nbRoutes);
				if (nbBits + 5 > nbRoutes) {
					BE_WRITE_INT(codec, bs, 1, 1, "isList", NULL);
					/*list*/
					for (j=0; j<count; j++) {
						r = ChainGetEntry(proto->sub_graph->Routes, j);
						if (r->IS_route) continue;
						e = BE_EncRoute(codec, r, bs);
						if (e) goto exit;
						nbRoutes--;
						BE_WRITE_INT(codec, bs, nbRoutes ? 1 : 0, 1, "moreRoute", NULL);
					}
				} else {
					BE_WRITE_INT(codec, bs, 0, 1, "isList", NULL);
					BE_WRITE_INT(codec, bs, nbBits, 5, "nbBits", NULL);
					BE_WRITE_INT(codec, bs, nbRoutes, nbBits, "length", NULL);
					for (j=0; j<count; j++) {
						r = ChainGetEntry(proto->sub_graph->Routes, j);
						if (r->IS_route) continue;
						e = BE_EncRoute(codec, r, bs);
						if (e) goto exit;
					}
				}
			}
		}

		/*anim and Quantization stuff*/
		BE_WRITE_INT(codec, bs, useQuant, 1, "useQuant", NULL);
		BE_WRITE_INT(codec, bs, useAnim, 1, "useAnim", NULL);

		if (!useAnim && !useQuant) continue;

		count = Proto_GetFieldCount(proto);
		for (i=0; i<count; i++) {
			proto_field = Proto_FindField(proto, i);
			ProtoField_GetField(proto_field, &field);

			/*quant*/
			if (useQuant && ( (field.eventType == ET_Field) || (field.eventType == ET_ExposedField) )) {
				BE_WRITE_INT(codec, bs, proto_field->QP_Type, 4, "QPType", NULL);
				if (proto_field->QP_Type==QC_LINEAR_SCALAR) BE_WRITE_INT(codec, bs, proto_field->NumBits, 5, "nbBits", NULL);
				BE_WRITE_INT(codec, bs, proto_field->hasMinMax, 1, "hasMinMax", NULL);
				if (proto_field->hasMinMax) {
					field.fieldType = VRML_GetSFType(field.fieldType);
					switch (field.fieldType) {
					case FT_SFInt32:
					case FT_SFTime:
						break;
					default:
						field.fieldType = FT_SFFloat;
						break;
					}
					field.name = "QPMinValue";
					field.far_ptr = proto_field->qp_min_value;
					BE_EncSFField(codec, bs, NULL, &field);

					field.name = "QPMaxValue";
					field.far_ptr = proto_field->qp_max_value;
					BE_EncSFField(codec, bs, NULL, &field);
				}
			}

			/*anim - not supported yet*/
			if (useAnim && ( (field.eventType == ET_EventIn) || (field.eventType == ET_ExposedField) )) {
				e = M4NotSupported;
				goto exit;
			}
		}
	}
	BE_WRITE_INT(codec, bs, 0, 1, "moreProto", NULL);

exit:
	/*restore scene graph state*/
	codec->encoding_proto = prev_proto;
	codec->current_proto_graph = rootSG;
	return e;
}



M4Err BE_EncRoute(BifsEncoder *codec, LPROUTE r, BitStream *bs)
{
	M4Err e;
	u32 numBits, ind;

	if (!r) return M4BadParam;

	BE_WRITE_INT(codec, bs, r->ID ? 1: 0, 1, "isDEF", NULL);
	/*def'ed route*/
	if (r->ID) {
		BE_WRITE_INT(codec, bs, r->ID-1, codec->info->config.RouteIDBits, "routeID", NULL);
		if (codec->info->UseName) BE_SetName(codec, bs, r->name);
	}
	/*origin*/
	BE_WRITE_INT(codec, bs, r->FromNode->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "outNodeID", NULL);
	numBits = Node_GetNumFields(r->FromNode, FCM_OUT) - 1;
	numBits = GetNumBits(numBits);
	e = BIFS_ModeFieldIndex(r->FromNode, r->FromFieldIndex, FCM_OUT, &ind);
	if (e) return e;
	BE_WRITE_INT(codec, bs, ind, numBits, "outField", NULL);

	/*target*/
	BE_WRITE_INT(codec, bs, r->ToNode->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "inNodeID", NULL);
	numBits = Node_GetNumFields(r->ToNode, FCM_IN) - 1;
	numBits = GetNumBits(numBits);
	e = BIFS_ModeFieldIndex(r->ToNode, r->ToFieldIndex, FCM_IN, &ind);
	BE_WRITE_INT(codec, bs, ind, numBits, "inField", NULL);
	return e;
}

M4Err BE_SceneReplaceEx(BifsEncoder *codec, SGCommand *com, BitStream *bs, Chain *routes)
{
	u32 i, nbR, nbBits;
	M4Err e;

	/*reserved*/
	BE_WRITE_INT(codec, bs, 0, 6, "reserved", NULL);
	BE_WRITE_INT(codec, bs, codec->info->UseName ? 1 : 0, 1, "useName", NULL);

	e = BE_EncProtoList(codec, com->new_proto_list, bs);
	if (e) goto exit;

	/*NULL root is valid for ProtoLibraries*/
	e = BE_EncSFNode(codec, com->node, NDT_SFTopNode, bs);
	if (e || !ChainGetCount(routes) ) {
		BE_WRITE_INT(codec, bs, 0, 1, "hasRoute", NULL);
		return codec->LastError = e;
	}
	BE_WRITE_INT(codec, bs, 1, 1, "hasRoute", NULL);
	nbR = ChainGetCount(routes);
	nbBits = GetNumBits(nbR);
	if (nbBits + 5 > nbR) {
		BE_WRITE_INT(codec, bs, 1, 1, "isList", NULL);
		/*list*/
		for (i=0; i<nbR; i++) {
			e = BE_EncRoute(codec, ChainGetEntry(routes, i), bs);
			if (e) goto exit;
			BE_WRITE_INT(codec, bs, (i+1==nbR) ? 0 : 1, 1, "moreRoute", NULL);
		}
	} else {
		BE_WRITE_INT(codec, bs, 0, 1, "isList", NULL);
		BE_WRITE_INT(codec, bs, nbBits, 5, "nbBits", NULL);
		BE_WRITE_INT(codec, bs, nbR, nbBits, "nbRoutes", NULL);
		for (i=0; i<nbR; i++) {
			e = BE_EncRoute(codec, ChainGetEntry(routes, i), bs);
			if (e) goto exit;
		}
	}

exit:
	return codec->LastError = e;
}


M4Err BE_SceneReplace(BifsEncoder *codec, LPSCENEGRAPH graph, BitStream *bs)
{
	u32 i, nbR, nbBits;
	M4Err e;

	/*reserved*/
	BE_WRITE_INT(codec, bs, 0, 6, "reserved", NULL);
	BE_WRITE_INT(codec, bs, codec->info->UseName ? 1 : 0, 1, "useName", NULL);

	/*assign current graph*/
	codec->scene_graph = graph;

	e = BE_EncProtoList(codec, codec->scene_graph ? codec->scene_graph->protos : NULL, bs);
	if (e) goto exit;

	/*NULL root is valid for ProtoLibraries*/
	e = BE_EncSFNode(codec, graph ? graph->RootNode : NULL, NDT_SFTopNode, bs);
	if (e || !graph || !ChainGetCount(graph->Routes) ) {
		BE_WRITE_INT(codec, bs, 0, 1, "hasRoute", NULL);
		return codec->LastError = e;
	}
	BE_WRITE_INT(codec, bs, 1, 1, "hasRoute", NULL);
	nbR = ChainGetCount(graph->Routes);
	nbBits = GetNumBits(nbR);
	if (nbBits + 5 > nbR) {
		BE_WRITE_INT(codec, bs, 1, 1, "isList", NULL);
		/*list*/
		for (i=0; i<nbR; i++) {
			e = BE_EncRoute(codec, ChainGetEntry(graph->Routes, i), bs);
			if (e) goto exit;
			BE_WRITE_INT(codec, bs, (i+1==nbR) ? 0 : 1, 1, "moreRoute", NULL);
		}
	} else {
		BE_WRITE_INT(codec, bs, 0, 1, "isList", NULL);
		BE_WRITE_INT(codec, bs, nbBits, 5, "nbBits", NULL);
		BE_WRITE_INT(codec, bs, nbR, nbBits, "nbRoutes", NULL);
		for (i=0; i<nbR; i++) {
			e = BE_EncRoute(codec, ChainGetEntry(graph->Routes, i), bs);
			if (e) goto exit;
		}
	}

exit:
	return codec->LastError = e;
}


M4Err BIFS_EncCommands(BifsEncoder *codec, Chain *comList, BitStream *bs)
{
	u32 i;
	u32 count;
	Chain *routes;
	M4Err e = M4OK;

	routes = NULL;

	codec->LastError = 0;
	count = ChainGetCount(comList);

	for (i=0; i<count; i++) {
		SGCommand *com = ChainGetEntry(comList, i);
		switch (com->tag) {
		case SG_SceneReplace:
		{
			/*reset node context*/
			while (ChainGetCount(codec->encoded_nodes)) ChainDeleteEntry(codec->encoded_nodes, 0);
			BE_WRITE_INT(codec, bs, 3, 2, "SceneReplace", NULL);
		
			routes = NewChain();
			/*now the trick: get all following InsertRoutes and convert as routes*/
			for (; i<count-1; i++) {
				LPROUTE r;
				SGCommand *rcom = ChainGetEntry(comList, i+1);
				if (rcom->tag!=SG_RouteInsert) break;
				SAFEALLOC(r, sizeof(Route));
				r->FromFieldIndex = rcom->fromFieldIndex;
				r->FromNode = SG_FindNode(codec->scene_graph, rcom->fromNodeID);
				r->ToFieldIndex = rcom->toFieldIndex;
				r->ToNode = SG_FindNode(codec->scene_graph, rcom->toNodeID);
				r->ID = rcom->RouteID;
				r->name = rcom->def_name;
				ChainAddEntry(routes, r);
			}
			e = BE_SceneReplaceEx(codec, com, bs, routes);
		}
			break;
		/*replace commands*/
		case SG_NodeReplace:
			BE_WRITE_INT(codec, bs, 2, 2, "Replace", NULL);
			BE_WRITE_INT(codec, bs, 0, 2, "Node", NULL);
			e = BE_NodeReplace(codec, com, bs);
			break;
		case SG_FieldReplace:
			BE_WRITE_INT(codec, bs, 2, 2, "Replace", NULL);
			BE_WRITE_INT(codec, bs, 1, 2, "Field", NULL);
			e = BE_FieldReplace(codec, com, bs);
			break;
		case SG_IndexedReplace:
			BE_WRITE_INT(codec, bs, 2, 2, "Replace", NULL);
			BE_WRITE_INT(codec, bs, 2, 2, "FieldIndex", NULL);
			e = BE_IndexFieldReplace(codec, com, bs);
			break;
		case SG_RouteReplace:
			BE_WRITE_INT(codec, bs, 2, 2, "Replace", NULL);
			BE_WRITE_INT(codec, bs, 3, 2, "Route", NULL);
			e = BE_RouteReplace(codec, com, bs, 0);
			break;
		case SG_NodeInsert:
			BE_WRITE_INT(codec, bs, 0, 2, "Insert", NULL);
			BE_WRITE_INT(codec, bs, 0, 2, "Node", NULL);
			e = BE_NodeInsert(codec, com, bs);
			break;
		case SG_IndexedInsert:
			BE_WRITE_INT(codec, bs, 0, 2, "Insert", NULL);
			BE_WRITE_INT(codec, bs, 2, 2, "FieldIndex", NULL);
			e = BE_IndexInsert(codec, com, bs);
			break;
		case SG_RouteInsert:
			BE_WRITE_INT(codec, bs, 0, 2, "Insert", NULL);
			BE_WRITE_INT(codec, bs, 3, 2, "Route", NULL);
			e = BE_RouteReplace(codec, com, bs, 1);
			break;
		case SG_NodeDelete:
			BE_WRITE_INT(codec, bs, 1, 2, "Delete", NULL);
			BE_WRITE_INT(codec, bs, 0, 2, "Node", NULL);
			BE_WRITE_INT(codec, bs, com->node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);
			break;
		case SG_IndexedDelete:
			BE_WRITE_INT(codec, bs, 1, 2, "Delete", NULL);
			BE_WRITE_INT(codec, bs, 2, 2, "FieldIndex", NULL);
			e = BE_IndexDelete(codec, com, bs);
			break;
		case SG_RouteDelete:
			BE_WRITE_INT(codec, bs, 1, 2, "Delete", NULL);
			BE_WRITE_INT(codec, bs, 3, 2, "Route", NULL);
			BE_WRITE_INT(codec, bs, com->RouteID - 1, codec->info->config.RouteIDBits, "RouteID", NULL);
			break;

		default:
			e = BE_ExtendedUpdate(codec, com, bs);
			break;
		}
		if (e) break;

		BE_WRITE_INT(codec, bs, (i+1==count) ? 0 : 1, 1, "moreCommands", NULL);
	}

	if (routes) {
		while (ChainGetCount(routes)) {
			LPROUTE r = ChainGetEntry(routes, 0);
			ChainDeleteEntry(routes, 0);
			free(r);
		}
		DeleteChain(routes);
	}
	while (ChainGetCount(codec->QPs)) BE_RemoveQP(codec, 1);
	return e;
}


M4Err BIFS_GetRAP(LPBIFSENC codec, char **out_data, u32 *out_data_length)
{
	BitStream *bs;
	M4Err e;
	Chain *ctx_bck;

	/*reset context for RAP encoding*/
	ctx_bck = codec->encoded_nodes;
	codec->encoded_nodes = NewChain();

	if (!codec->info) codec->info = ChainGetEntry(codec->streamInfo, 0);

	bs = NewBitStream(NULL, 0, BS_WRITE);
	BE_WRITE_INT(codec, bs, 3, 2, "SceneReplace", NULL);
	e = BE_SceneReplace(codec, codec->scene_graph, bs);
	if (e == M4OK) {
		BE_WRITE_INT(codec, bs, 0, 1, "moreCommands", NULL);
		BS_GetContent(bs, (unsigned char **)out_data, out_data_length);
	}
	DeleteBitStream(bs);
	
	/*restore context*/
	DeleteChain(codec->encoded_nodes);
	codec->encoded_nodes = ctx_bck;

	return e;
}

