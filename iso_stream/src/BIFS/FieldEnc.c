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
#include <intern/m4_bifs_tables.h>
#include "Quantize.h"


M4Err BIFS_ModeFieldIndex(SFNode *node, u32 all_ind, u8 indexMode, u32 *outField)
{
	M4Err e;
	u32 i, count, temp;
	count = Node_GetNumFields(node, indexMode);
	for (i=0; i<count; i++) {
		e = Node_GetFieldIndex(node, i, indexMode, &temp);
		if (e) return e;
		if (temp==all_ind) {
			*outField = i;
			return M4OK;
		}
	}
	return M4BadParam;
}

void BE_LogFloat(BifsEncoder *codec, Float val, u32 nbBits, char *str, char *com);

void BE_WriteSFFloat(BifsEncoder *codec, SFFloat val, BitStream *bs, char *com)
{
	if (codec->ActiveQP && codec->ActiveQP->useEfficientCoding) {
		BE_WriteMantissaFloat(codec, val, bs);
	} else {
		BS_WriteFloat(bs, val);
		BE_LogFloat(codec, (Float) val, 32, "SFFloat", com);
	}
}


M4Err BE_EncSFField(BifsEncoder *codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	M4Err e;

	if (node) {
		e = BE_QuantizeField(codec, bs, node, field);
		if (e != M4FieldNotQuantized) return e;
	}
	switch (field->fieldType) {
	case FT_SFBool:
		BE_WRITE_INT(codec, bs, * ((SFBool *)field->far_ptr), 1, "SFBool", NULL);
		break;
	case FT_SFColor:
		BE_WriteSFFloat(codec, ((SFColor *)field->far_ptr)->red, bs, "color.red");
		BE_WriteSFFloat(codec, ((SFColor *)field->far_ptr)->green, bs, "color.green");
		BE_WriteSFFloat(codec, ((SFColor *)field->far_ptr)->blue, bs, "color.blue");
		break;
	case FT_SFFloat:
		BE_WriteSFFloat(codec, * ((SFFloat *)field->far_ptr), bs, NULL);
		break;
	case FT_SFInt32:
		BE_WRITE_INT(codec, bs, * ((SFInt32 *)field->far_ptr), 32, "SFInt32", NULL);
		break;
	case FT_SFRotation:
		BE_WriteSFFloat(codec, ((SFRotation  *)field->far_ptr)->xAxis, bs, "rot.x");
		BE_WriteSFFloat(codec, ((SFRotation  *)field->far_ptr)->yAxis, bs, "rot.y");
		BE_WriteSFFloat(codec, ((SFRotation  *)field->far_ptr)->zAxis, bs, "rot.z");
		BE_WriteSFFloat(codec, ((SFRotation  *)field->far_ptr)->angle, bs, "rot.theta");
		break;

	case FT_SFString:
	{
		u32 i;
		char *str = ((SFString*)field->far_ptr)->buffer;
		u32 len = str ? strlen(str) : 0;
		u32 val = GetNumBits(len);
		BE_WRITE_INT(codec, bs, val, 5, "nbBits", NULL);
		BE_WRITE_INT(codec, bs, len, val, "length", NULL);
		for (i=0; i<len; i++) BS_WriteInt(bs, str[i], 8);
		if (codec->trace) fprintf(codec->trace, "string\t\t%d\t\t%s\n", 8*len, str);
	}
		break;

	case FT_SFTime:
		BS_WriteDouble(bs, *((SFTime *)field->far_ptr));
		if (codec->trace) fprintf(codec->trace, "SFTime\t\t%d\t\t%g\n", 64, *((SFTime *)field->far_ptr));
		break;

	case FT_SFVec2f:
		BE_WriteSFFloat(codec, ((SFVec2f *)field->far_ptr)->x, bs, "vec2f.x");
		BE_WriteSFFloat(codec, ((SFVec2f *)field->far_ptr)->y, bs, "vec2f.y");
		break;
	
	case FT_SFVec3f:
		BE_WriteSFFloat(codec, ((SFVec3f *)field->far_ptr)->x, bs, "vec3f.x");
		BE_WriteSFFloat(codec, ((SFVec3f *)field->far_ptr)->y, bs, "vec3f.y");
		BE_WriteSFFloat(codec, ((SFVec3f *)field->far_ptr)->z, bs, "vec3f.z");
		break;

	case FT_SFURL:
	{
		SFURL *url = (SFURL *) field->far_ptr;
		BE_WRITE_INT(codec, bs, (url->OD_ID>0) ? 1 : 0, 1, "hasODID", "SFURL");
		if (url->OD_ID>0) {
			BE_WRITE_INT(codec, bs, url->OD_ID, 10, "ODID", "SFURL");
		} else {
			u32 i;
			u32 len = url->url ? strlen(url->url) : 0;
			u32 val = GetNumBits(len);
			BE_WRITE_INT(codec, bs, val, 5, "nbBits", NULL);
			BE_WRITE_INT(codec, bs, len, val, "length", NULL);
			for (i=0; i<len; i++) BS_WriteInt(bs, url->url[i], 8);
			if (codec->trace) fprintf(codec->trace, "string\t\t%d\t\t%s\t\t//SFURL\n", 8*len, url->url);
		}
	}
		break;
	case FT_SFImage:
	{
		u32 size, i;
		SFImage *img = (SFImage *)field->far_ptr;
		BE_WRITE_INT(codec, bs, img->width, 12, "width", "SFImage");
		BE_WRITE_INT(codec, bs, img->height, 12, "height", "SFImage");
		BE_WRITE_INT(codec, bs, img->numComponents - 1, 2, "nbComp", "SFImage");
		size = img->width * img->height * img->numComponents;
		for (i=0; i<size; i++) BS_WriteInt(bs, img->pixels[i], 8);
		if (codec->trace) fprintf(codec->trace, "pixels\t\t%d\t\tnot dumped\t\t//SFImage\n", 8*size);
	}
		break;

	case FT_SFCommandBuffer:
	{
		SFCommandBuffer *cb = (SFCommandBuffer *) field->far_ptr;
		if (cb->buffer) free(cb->buffer);
		cb->buffer = NULL;
		cb->bufferSize = 0;
		if (ChainGetCount(cb->commandList)) {
			u32 i, nbBits;
			BitStream *bs_cond = NewBitStream(NULL, 0, BS_WRITE);
			if (codec->trace) fprintf(codec->trace, "/*SFCommandBuffer*/\n");
			e = BIFS_EncCommands(codec, cb->commandList, bs_cond);
			if (!e) BS_GetContent(bs_cond, &cb->buffer, &cb->bufferSize);
			DeleteBitStream(bs_cond);
			if (e) return e;
			if (codec->trace) fprintf(codec->trace, "/*End SFCommandBuffer*/\n");
			nbBits = GetNumBits(cb->bufferSize);
			BE_WRITE_INT(codec, bs, nbBits, 5, "NbBits", NULL);
			BE_WRITE_INT(codec, bs, cb->bufferSize, nbBits, "BufferSize", NULL);
			for (i=0; i<cb->bufferSize; i++) BE_WRITE_INT(codec, bs, cb->buffer[i], 8, "buffer byte", NULL);
		}
	}
		break;

	case FT_SFNode:
		return BE_EncSFNode(codec, *((SFNode **)field->far_ptr), field->NDTtype, bs);

	case FT_SFScript:
	{
		M4Err SFScript_Encode(BifsEncoder *codec, BitStream *bs, SFNode *n);
		codec->LastError = SFScript_Encode(codec, bs, node);
	}
		break;
	default:
		return M4NonCompliantBitStream;
	}
	return codec->LastError;
}


M4Err BE_EncMFField(BifsEncoder *codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	SFNode *child;
	Chain *list;
	M4Err e;
	u32 nbBits;
	Bool use_list;
	Bool qp_local, qp_on, initial_qp;
	u32 nbF, i;
	FieldInfo sffield;
		
	nbF = 0;
	if (field->fieldType != FT_MFNode) {
		nbF = ((GenMFField *)field->far_ptr)->count;
		list = NULL;
	} else {
		list = *((Chain **)field->far_ptr);
		nbF = ChainGetCount(list);
	}
	/*reserved*/
	BE_WRITE_INT(codec, bs, 0, 1, "reserved", NULL);
	if (!nbF) {
		/*is list*/
		BE_WRITE_INT(codec, bs, 1, 1, "isList", NULL);
		/*end flag*/
		BE_WRITE_INT(codec, bs, 1, 1, "end", NULL);
		return M4OK;
	}

	/*do we work in list or vector*/
	use_list = 0;
	nbBits = GetNumBits(nbF);
	if (nbBits + 5 > nbF + 1) use_list = 1;

	BE_WRITE_INT(codec, bs, use_list, 1, "isList", NULL);
	if (!use_list) {
		BE_WRITE_INT(codec, bs, nbBits, 5, "nbBits", NULL);
		BE_WRITE_INT(codec, bs, nbF, nbBits, "length", NULL);
	}

	memset(&sffield, 0, sizeof(FieldInfo));
	sffield.fieldIndex = field->fieldIndex;
	sffield.fieldType = VRML_GetSFType(field->fieldType);
	sffield.NDTtype = field->NDTtype;

	initial_qp = qp_on = qp_local = 0;
	initial_qp = codec->ActiveQP ? 1 : 0;
	for (i=0; i<nbF; i++) {

		if (use_list) BE_WRITE_INT(codec, bs, 0, 1, "end", NULL);

		if (field->fieldType != FT_MFNode) {
			VRML_MF_GetItem(field->far_ptr, field->fieldType, &sffield.far_ptr, i);
			e = BE_EncSFField(codec, bs, node, &sffield);
		} else {
			child = ChainGetEntry(list, i);
			e = BE_EncSFNode(codec, child, field->NDTtype, bs);

			/*activate QP*/
			if (child->sgprivate->tag == TAG_MPEG4_QuantizationParameter) {
				qp_local = ((M_QuantizationParameter *)child)->isLocal;
				if (qp_on) BE_RemoveQP(codec, 0);
				e = BE_RegisterQP(codec, child);
				if (e) return e;
				qp_on = 1;
				if (qp_local) qp_local = 2;
			}
		}
		
		if (e) return e;

		if (qp_on && qp_local) {
			if (qp_local == 2) qp_local -= 1;
			else {
				BE_RemoveQP(codec, initial_qp);
				qp_local = qp_on = 0;
			}
		}
	}

	if (use_list) BE_WRITE_INT(codec, bs, 1, 1, "end", NULL);
	if (qp_on) BE_RemoveQP(codec, initial_qp);
	/*for QP14*/
	BE_SetCoordLength(codec, nbF);
	return M4OK;
}


M4Err BE_EncField(BifsEncoder * codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	assert(node);
	if (field->fieldType == FT_Unknown) 
		return M4NonCompliantBitStream;
	
	if (VRML_IsSFField(field->fieldType)) {
		return BE_EncSFField(codec, bs, node, field);
	}

	/*TO DO : PMF support*/

	if (codec->info->config.UsePredictiveMFField) {
		BE_WRITE_INT(codec, bs, 0, 1, "usePredictive", NULL);
	}
	return BE_EncMFField(codec, bs, node, field);
}

/*we assume a node field is not ISed several times (that's stated as "undefined behaviour" in VRML*/
LPROUTE NodeIsSedField(BifsEncoder *codec, SFNode *node, u32 fieldIndex)
{
	LPROUTE r;
	u32 i;
	if (!codec->encoding_proto) return NULL;
	for (i=0; i<ChainGetCount(node->sgprivate->outRoutes); i++) {
		r = ChainGetEntry(node->sgprivate->outRoutes, i);
		if (!r->IS_route) continue;
		if ((r->ToNode == node) && (r->ToFieldIndex==fieldIndex)) return r;
		else if ((r->FromNode == node) && (r->FromFieldIndex==fieldIndex)) return r;
	}
	for (i=0; i<ChainGetCount(codec->encoding_proto->sub_graph->Routes); i++) {
		r = ChainGetEntry(codec->encoding_proto->sub_graph->Routes, i);
		if (!r->IS_route) continue;
		if ((r->ToNode == node) && (r->ToFieldIndex==fieldIndex)) return r;
		else if ((r->FromNode == node) && (r->FromFieldIndex==fieldIndex)) return r;
	}
	return NULL;
}

/**/
M4Err EncNodeFields(BifsEncoder * codec, BitStream *bs, SFNode *node)
{
	u8 mode;
	LPROUTE isedField;
	SFNode *clone;
	M4Err e;
	s32 *enc_fields;
	u32 numBitsALL, numBitsDEF, allInd, count, i, nbBitsProto, nbFinal;
	Bool use_list;
	FieldInfo field, clone_field;


	e = M4OK;
	if (node->sgprivate->tag == TAG_ProtoNode) {
		clone = Proto_CreateInstance(node->sgprivate->scenegraph, ((ProtoInstance *)node)->proto_interface);;
	} else {
		clone = SG_NewNode(node->sgprivate->scenegraph, node->sgprivate->tag);
	}
	if (clone) Node_Register(clone, NULL);


	if (codec->encoding_proto) {
		mode = FCM_ALL;
		nbBitsProto = GetNumBits(Proto_GetFieldCount(codec->encoding_proto) - 1);
		numBitsALL = GetNumBits(Node_GetNumFields(node, FCM_ALL) - 1);
	} else {
		mode = FCM_DEF;
		nbBitsProto = 0;
		numBitsALL = 0;
	}
	count = Node_GetNumFields(node, mode);
	if (node->sgprivate->tag==TAG_MPEG4_Script) count = 3;
	numBitsDEF = GetNumBits(Node_GetNumFields(node, FCM_DEF) - 1);


	enc_fields = malloc(sizeof(s32) * count);
	nbFinal = 0;
	for (i=0; i<count; i++) {
		enc_fields[i] = -1;
		/*get field in ALL mode*/
		if (mode == FCM_ALL) {
			allInd = i;
		} else {
			Node_GetFieldIndex(node, i, FCM_DEF, &allInd);
		}

		/*encode proto code*/
		if (codec->encoding_proto) {
			isedField = NodeIsSedField(codec, node, allInd);
			if (isedField) {
				enc_fields[i] = allInd;
				nbFinal ++;
				continue;
			}
		}
		/*common case*/
		Node_GetField(node, allInd, &field);
		/*if event don't encode (happens when encoding protos)*/
		if ((field.eventType == ET_EventIn) || (field.eventType == ET_EventOut)) continue;
		/*if field is default skip*/
		switch (field.fieldType) {
		case FT_SFNode:
			if (* (SFNode **) field.far_ptr) { enc_fields[i] = allInd; nbFinal++; }
			break;
		case FT_MFNode:
			if (ChainGetCount(* (Chain **) field.far_ptr) ) { enc_fields[i] = allInd; nbFinal++; }
			break;
		case FT_SFCommandBuffer:
		{
			SFCommandBuffer *cb = (SFCommandBuffer *)field.far_ptr;
			if (ChainGetCount(cb->commandList)) { enc_fields[i] = allInd; nbFinal++; }
		}
			break;
		default:
			Node_GetField(clone, allInd, &clone_field);
			if (!VRML_FieldsEqual(clone_field.far_ptr, field.far_ptr, field.fieldType)) { enc_fields[i] = allInd; nbFinal++; }
			break;
		}
	}
	if (clone) Node_Unregister(clone, NULL);

	/*number of bits in mask node is count*1, in list node is 1+nbFinal*(1+numBitsDEF) */
	use_list = 1;
	if (count < 1+nbFinal*(1+numBitsDEF)) use_list = 0;

	BE_WRITE_INT(codec, bs, use_list ? 0 : 1, 1, "isMask", NULL);

	for (i=0; i<count; i++) {
		if (enc_fields[i] == -1) {
			if (!use_list) BE_WRITE_INT(codec, bs, 0, 1, "Mask", NULL);
			continue;
		}
		allInd = (u32) enc_fields[i];

		/*encode proto code*/
		if (codec->encoding_proto) {
			isedField = NodeIsSedField(codec, node, allInd);
			if (isedField) {
				if (use_list) {
					BE_WRITE_INT(codec, bs, 0, 1, "end", NULL);
				} else {
					BE_WRITE_INT(codec, bs, 1, 1, "Mask", NULL);
				}
				BE_WRITE_INT(codec, bs, 1, 1, "isedField", NULL);
				if (use_list) BE_WRITE_INT(codec, bs, allInd, numBitsALL, "nodeField", NULL);

				if (isedField->ToNode == node) {
					BE_WRITE_INT(codec, bs, isedField->FromFieldIndex, nbBitsProto, "protoField", NULL);
				} else {
					BE_WRITE_INT(codec, bs, isedField->ToFieldIndex, nbBitsProto, "protoField", NULL);
				}
				continue;
			}
		}
		/*common case*/
		Node_GetField(node, allInd, &field);
		if (use_list) {
			/*not end flag*/
			BE_WRITE_INT(codec, bs, 0, 1, "end", NULL);
		} else {
			/*mask flag*/
			BE_WRITE_INT(codec, bs, 1, 1, "Mask", NULL);
		}
		/*not ISed field*/
		if (codec->encoding_proto) BE_WRITE_INT(codec, bs, 0, 1, "isedField", NULL);
		if (use_list) {
			if (codec->encoding_proto) {
				u32 ind;
				/*we're in ALL mode and we need DEF mode*/
				BIFS_ModeFieldIndex(node, allInd, FCM_DEF, &ind);
				BE_WRITE_INT(codec, bs, ind, numBitsDEF, "field", (char*)field.name);
			} else {
				BE_WRITE_INT(codec, bs, i, numBitsDEF, "field", (char*)field.name);
			}
		}
		e = BE_EncField(codec, bs, node, &field);
		if (e) goto exit;
	}
	/*end flag*/
	if (use_list) BE_WRITE_INT(codec, bs, 1, 1, "end", NULL);
exit:
	free(enc_fields);
	return e;
}

Bool BE_NodeIsUSE(BifsEncoder * codec, SFNode *node)
{
	u32 i;
	if (!node || !node->sgprivate->NodeID) return 0;
	for (i=0; i<ChainGetCount(codec->encoded_nodes); i++) {
		if (ChainGetEntry(codec->encoded_nodes, i) == node) return 1;
	}
	ChainAddEntry(codec->encoded_nodes, node);
	return 0;
}

M4Err BE_EncSFNode(BifsEncoder * codec, SFNode *node, u32 NDT_Tag, BitStream *bs)
{
	u32 NDTBits, node_type, node_tag, BVersion;
	Bool flag;
	SFNode *new_node;
	M4Err e;

	assert(codec->info);

	/*NULL node is a USE of maxID*/
	if (!node) {
		BE_WRITE_INT(codec, bs, 1, 1, "USE", NULL);
		BE_WRITE_INT(codec, bs, (1<<codec->info->config.NodeIDBits) - 1 , codec->info->config.NodeIDBits, "NodeID", "NULL");
		return M4OK;
	}

	flag = BE_NodeIsUSE(codec, node);
	BE_WRITE_INT(codec, bs, flag ? 1 : 0, 1, "USE", (char*)Node_GetName(node));

	if (flag) {
		BS_WriteInt(bs, node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits);
		new_node = BE_FindNode(codec, node->sgprivate->NodeID);
		if (!new_node) return codec->LastError = M4InvalidNode;
		
		/*restore QP14 length*/
		switch (Node_GetTag(new_node)) {
		case TAG_MPEG4_Coordinate:
		{
			u32 nbCoord = ((M_Coordinate *)new_node)->point.count;
			BE_EnterCoord(codec, 1);
			BE_SetCoordLength(codec, nbCoord);
			BE_EnterCoord(codec, 0);
		}
			break;
		case TAG_MPEG4_Coordinate2D:
		{
			u32 nbCoord = ((M_Coordinate2D *)new_node)->point.count;
			BE_EnterCoord(codec, 1);
			BE_SetCoordLength(codec, nbCoord);
			BE_EnterCoord(codec, 0);
		}
			break;
		}
		return M4OK;
	}

	BVersion = BIFS_V1;
	node_tag = node->sgprivate->tag;
	while (1) {
		node_type = NDT_GetNodeType(NDT_Tag, node_tag, BVersion);
		NDTBits = NDT_GetNumBits(NDT_Tag, BVersion);
		if (BVersion==2 && (node_tag==TAG_ProtoNode)) node_type = 1;
		BE_WRITE_INT(codec, bs, node_type, NDTBits, "ndt", NULL);
		if (node_type) break;

		BVersion += 1;
		if (BVersion > NUM_BIFS_VERSION) return codec->LastError = M4UnknowBIFSVersion;
	}
	if (BVersion==2 && node_type==1) {
		LPPROTO proto = ((ProtoInstance *)node)->proto_interface;
		BE_WRITE_INT(codec, bs, proto->ID, codec->info->config.ProtoIDBits, "protoID", NULL);
	}

	/*special handling of 3D mesh*/

	/*DEF'd node*/
	BE_WRITE_INT(codec, bs, node->sgprivate->NodeID ? 1 : 0, 1, "DEF", NULL);
	if (node->sgprivate->NodeID) {
		BE_WRITE_INT(codec, bs, node->sgprivate->NodeID - 1, codec->info->config.NodeIDBits, "NodeID", NULL);
		if (codec->info->UseName) BE_SetName(codec, bs, node->sgprivate->NodeName);
	}

	/*no updates of time fields for now - NEEDED FOR A LIVE ENCODER*/

	/*QP14 case*/
	switch (node_tag) {
	case TAG_MPEG4_Coordinate:
	case TAG_MPEG4_Coordinate2D:
		BE_EnterCoord(codec, 1);
	}

	e = EncNodeFields(codec, bs, node);
	if (e) return e;

	switch (node_tag) {
	case TAG_MPEG4_IndexedFaceSet:
	case TAG_MPEG4_IndexedFaceSet2D:
	case TAG_MPEG4_IndexedLineSet:
	case TAG_MPEG4_IndexedLineSet2D:
		BE_ResetQP14(codec);
		break;
	case TAG_MPEG4_Coordinate:
	case TAG_MPEG4_Coordinate2D:
		BE_EnterCoord(codec, 0);
		break;
	}
	return M4OK;
}


