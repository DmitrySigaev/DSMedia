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
#include <gpac/intern/m4_scenegraph_dev.h>
#include "Quantize.h"


void SFCommandBufferChanged(BifsDecoder * codec, SFNode *node)
{
	void Conditional_BufferReplaced(BifsDecoder * codec, SFNode *node);

	switch (Node_GetTag(node)) {
	case TAG_MPEG4_Conditional:
		Conditional_BufferReplaced(codec, node);
		break;
	}
}


//startTimes, stopTimes and co are coded as relative to their AU timestamp when received
//on the wire. If from scripts or within proto the offset doesn't apply
void BD_OffsetSFTime(BifsDecoder * codec, Double *time)
{
	Double now;
	if (codec->pCurrentProto || !codec->GetSceneTime || codec->dec_memory_mode) return;
	now = codec->GetSceneTime(codec->st_cbk);
	*time += now;
}

void BD_CheckSFTimeOffset(BifsDecoder *codec, SFNode *node, FieldInfo *inf)
{
	if (Node_GetTag(node) != TAG_ProtoNode) {
		if (!stricmp(inf->name, "startTime") || !stricmp(inf->name, "stopTime")) 
			BD_OffsetSFTime(codec,  (Double *)inf->far_ptr);
	} else if (Proto_FieldIsSFTimeOffset(node, inf)) {
		BD_OffsetSFTime(codec,  (Double *)inf->far_ptr);
	}
}


SFFloat BD_ReadSFFloat(BifsDecoder * codec, BitStream *bs)
{
	SFFloat ret;
	if (codec->ActiveQP && codec->ActiveQP->useEfficientCoding) {
		ret = BD_ReadMantissaFloat(codec, bs);
	} else {
		ret = BS_ReadFloat(bs);
	}
	return ret;
}


M4Err BD_DecSFField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	M4Err e;
	SFNode *new_node;
	u32 size, length, w, h, i;
	char *buffer;

	//blindly call unquantize. return is OK, error or M4FieldNotQuantized
	if (node) {
		e = BD_UnquantizeField(codec, bs, node, field);
		if (e != M4FieldNotQuantized) return e;
	}
	//not quantized, use normal scheme
	switch (field->fieldType) {
	case FT_SFBool:
		* ((SFBool *) field->far_ptr) = (SFBool) BS_ReadInt(bs, 1);
		break;

	case FT_SFColor:
		((SFColor *)field->far_ptr)->red = BD_ReadSFFloat(codec, bs);;
		((SFColor *)field->far_ptr)->green = BD_ReadSFFloat(codec, bs);
		((SFColor *)field->far_ptr)->blue = BD_ReadSFFloat(codec, bs);
		break;

	case FT_SFFloat:
		*((SFFloat *)field->far_ptr) = BD_ReadSFFloat(codec, bs);
		break;

	case FT_SFInt32:
		*((SFInt32 *)field->far_ptr) = (s32) BS_ReadInt(bs, 32);
		break;

	case FT_SFRotation:
		((SFRotation *)field->far_ptr)->xAxis = BD_ReadSFFloat(codec, bs);
		((SFRotation *)field->far_ptr)->yAxis = BD_ReadSFFloat(codec, bs);
		((SFRotation *)field->far_ptr)->zAxis = BD_ReadSFFloat(codec, bs);
		((SFRotation *)field->far_ptr)->angle = BD_ReadSFFloat(codec, bs);
		break;

	case FT_SFString:
		size = BS_ReadInt(bs, 5);
		length = BS_ReadInt(bs, size);
		if (BS_Available(bs) < length) return M4NonCompliantBitStream;

		if ( ((SFString *)field->far_ptr)->buffer ) free( ((SFString *)field->far_ptr)->buffer);
		((SFString *)field->far_ptr)->buffer = malloc(sizeof(char)*(length+1));
		memset(((SFString *)field->far_ptr)->buffer , 0, length+1);
		for (i=0; i<length; i++) {
			 ((SFString *)field->far_ptr)->buffer[i] = BS_ReadInt(bs, 8);
		}
		break;

	case FT_SFTime:
		*((SFTime *)field->far_ptr) = BS_ReadDouble(bs);
		if (node) BD_CheckSFTimeOffset(codec, node, field);
		break;

	case FT_SFVec2f:
		((SFVec2f *)field->far_ptr)->x = BD_ReadSFFloat(codec, bs);
		((SFVec2f *)field->far_ptr)->y = BD_ReadSFFloat(codec, bs);
		break;
	
	case FT_SFVec3f:
		((SFVec3f *)field->far_ptr)->x = BD_ReadSFFloat(codec, bs);
		((SFVec3f *)field->far_ptr)->y = BD_ReadSFFloat(codec, bs);
		((SFVec3f *)field->far_ptr)->z = BD_ReadSFFloat(codec, bs);
		break;

	case FT_SFURL:
	{
		SFURL *url = (SFURL *) field->far_ptr;
		size = BS_ReadInt(bs, 1);
		if (size) {
			if (url->url) free(url->url );
			url->url = NULL;
			length = BS_ReadInt(bs, 10);
			url->OD_ID = length;
		} else {
			if ( url->OD_ID ) url->OD_ID = -1;
			size = BS_ReadInt(bs, 5);
			length = BS_ReadInt(bs, size);
			if (BS_Available(bs) < length) return M4NonCompliantBitStream;
			buffer = NULL;
			if (length) {
				buffer = malloc(sizeof(char)*(length+1));
				memset(buffer, 0, length+1);
				for (i=0; i<length; i++) buffer[i] = BS_ReadInt(bs, 8);
			}
			if (url->url) free( url->url);
			/*if URL is empty set it to NULL*/
			if (buffer && strlen(buffer)) {
				url->url = buffer;
			} else {
				free(buffer);
				url->url = NULL;
			}
		}
	}
		break;
	case FT_SFImage:
		if (((SFImage *)field->far_ptr)->pixels) free(((SFImage *)field->far_ptr)->pixels);
		w = BS_ReadInt(bs, 12);
		h = BS_ReadInt(bs, 12);
		length = BS_ReadInt(bs, 2);

		if (length > 3) length = 3;
		length += 1;
		size = w * h * length;
		if (BS_Available(bs) < size) return M4NonCompliantBitStream;
		((SFImage *)field->far_ptr)->width = w;
		((SFImage *)field->far_ptr)->height = h;
		((SFImage *)field->far_ptr)->numComponents = length;
		((SFImage *)field->far_ptr)->pixels = malloc(sizeof(char)*size);
		//WARNING: Buffers are NOT ALIGNED IN THE BITSTREAM
		for (i=0; i<size; i++) {
			((SFImage *)field->far_ptr)->pixels[i] = BS_ReadInt(bs, 8);
		}
		break;

	case FT_SFCommandBuffer:
	{
		SFCommandBuffer *sfcb = (SFCommandBuffer *)field->far_ptr;
		if (sfcb->buffer) free(sfcb->buffer);		
		while (ChainGetCount(sfcb->commandList)) {
			SGCommand *com = ChainGetEntry(sfcb->commandList, 0);
			ChainDeleteEntry(sfcb->commandList, 0);
			SG_DeleteCommand(com);
		}

		size = BS_ReadInt(bs, 5);
		length = BS_ReadInt(bs, size);
		if (BS_Available(bs) < length) return M4NonCompliantBitStream;

		sfcb->bufferSize = length;
		if (length) {
			sfcb->buffer = malloc(sizeof(char)*(length));
			//WARNING Buffers are NOT ALIGNED IN THE BITSTREAM
			for (i=0; i<length; i++) {
				sfcb->buffer[i] = BS_ReadInt(bs, 8);
			}
		}
		//notify the node - this is needed in case an enhencement layer replaces the buffer, in which case 
		//the # ID Bits may change
		SFCommandBufferChanged(codec, node);
		/*memory mode, register command buffer for later parsing*/
		if (codec->dec_memory_mode) ChainAddEntry(codec->conditionals, node);
		/*InputSensor only work on decompressed commands*/
		else if (node->sgprivate->tag==TAG_MPEG4_InputSensor) {
			M4Err BM_ParseCommand(LPBIFSDEC codec, BitStream *bs, Chain *com_list);
			BitStream *is_bs;
			is_bs = NewBitStream(sfcb->buffer, sfcb->bufferSize, BS_READ);
			e = BM_ParseCommand(codec, is_bs, sfcb->commandList);
			DeleteBitStream(is_bs);
		}
	}
		break;

	//for nodes the field ptr is a ptr to the field, which is a node ptr ;)
	case FT_SFNode:
		new_node = BD_DecSFNode(codec, bs, field->NDTtype);
		if (new_node) {
			e = Node_Register(new_node, node);
			if (e) return e;
		}

		//it may happen that new_node is NULL (this is valid for a proto declaration)
		*((SFNode **) field->far_ptr) = new_node;
		break;

	case FT_SFScript:
	{
		M4Err SFScript_Parse(BifsDecoder * codec, BitStream *bs, SFNode *n);
		e = SFScript_Parse(codec, bs, node);
	}
		break;
	default:
		return M4NonCompliantBitStream;
	}
	return codec->LastError;
}


M4Err BD_DecMFFieldList(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	SFNode *new_node;
	M4Err e;
	u8 endFlag, qp_local, qp_on, initial_qp;
	u32 nbF;

	FieldInfo sffield;
	
	memset(&sffield, 0, sizeof(FieldInfo));
	sffield.fieldIndex = field->fieldIndex;
	sffield.fieldType = VRML_GetSFType(field->fieldType);
	sffield.NDTtype = field->NDTtype;

	nbF = 0;
	qp_on = qp_local = 0;
	initial_qp = codec->ActiveQP ? 1 : 0;

	endFlag = BS_ReadInt(bs, 1);
	while (!endFlag) {
		e = M4OK;;
		if (field->fieldType != FT_MFNode) {
			e = VRML_MF_Append(field->far_ptr, field->fieldType, & sffield.far_ptr);
			e = BD_DecSFField(codec, bs, node, &sffield);
		} else {
			new_node = BD_DecSFNode(codec, bs, field->NDTtype);
			//append
			if (new_node) {
				e = Node_Register(new_node, node);
				if (e) return e;

				//regular coding
				if (node) {
					//special case for QP, register as the current QP
					if (Node_GetTag(new_node) == TAG_MPEG4_QuantizationParameter) {
						qp_local = ((M_QuantizationParameter *)new_node)->isLocal;
						//we have a QP in the same scope, remove previous
						if (qp_on) BD_RemoveQP(codec, 0);
						e = BD_RegisterQP(codec, new_node);
						if (e) return e;
						qp_on = 1;
						if (qp_local) qp_local = 2;
						if (codec->force_keep_qp) {
							e = InsertSFNode(field->far_ptr, new_node, -1);
						} else {
							Node_Register(new_node, NULL);
							Node_Unregister(new_node, node);
						}
					} else 
						//this is generic MFNode container
						e = InsertSFNode(field->far_ptr, new_node, -1);
					
				}
				//proto coding: directly add the child
				else if (codec->pCurrentProto) {
					//TO DO: what happens if this is a QP node on the interface ?
					ChainAddEntry(*(Chain **)field->far_ptr, new_node);
				}
			} else {
				return codec->LastError;
			}
		}
		if (e) return e;

		endFlag = BS_ReadInt(bs, 1);

		//according to the spec, the QP applies to the current node itself, 
		//not just children. If IsLocal is TRUE remove the node
		if (qp_on && qp_local) {
			if (qp_local == 2) {
				qp_local = 1;
			} else {
				//ask to get rid of QP and reactivate if we had a QP when entering
				BD_RemoveQP(codec, initial_qp);
				qp_local = 0;
				qp_on = 0;
			}
		}
		nbF += 1;
	}
	/*finally delete the QP if any (local or not) as we get out of this node
	and reactivate previous one*/
	if (qp_on) BD_RemoveQP(codec, initial_qp);
	/*this is for QP 14*/
	BD_SetCoordLength(codec, nbF);
	return M4OK;
}

M4Err BD_DecMFFieldVec(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	M4Err e;
	u32 NbBits, nbFields;
	u32 i;
	u8 qp_local, qp_on, initial_qp;
	SFNode *new_node;
	FieldInfo sffield;
	
	memset(&sffield, 0, sizeof(FieldInfo));
	sffield.fieldIndex = field->fieldIndex;
	sffield.fieldType = VRML_GetSFType(field->fieldType);
	sffield.NDTtype = field->NDTtype;

	initial_qp = qp_local = qp_on = 0;

	//vector description - alloc the MF size before 
	NbBits = BS_ReadInt(bs, 5);
	nbFields = BS_ReadInt(bs, NbBits);

	if (codec->ActiveQP) initial_qp = 1;

	/*this is for QP 14*/
	BD_SetCoordLength(codec, nbFields);

	//alloc
	if (field->fieldType != FT_MFNode) {
		e = VRML_MF_Alloc(field->far_ptr, field->fieldType, nbFields);
		if (e) return e;
	}

	for (i=0;i<nbFields; i++) {
		if (field->fieldType != FT_MFNode) {
			e = VRML_MF_GetItem(field->far_ptr, field->fieldType, & sffield.far_ptr, i);
			e = BD_DecSFField(codec, bs, node, &sffield);
		} else {
			new_node = BD_DecSFNode(codec, bs, field->NDTtype);

			//append
			if (new_node) {

				e = Node_Register(new_node, node);
				if (e) return e;

				//regular node
				if (node) {
					//special case for QP, register as the current QP
					if (Node_GetTag(new_node) == TAG_MPEG4_QuantizationParameter) {
						qp_local = ((M_QuantizationParameter *)new_node)->isLocal;
						//we have a QP in the same scope, remove previous
						//NB: we assume this is the right behaviour, the spec doesn't say 
						//whether QP is cumulative or not
						if (qp_on) BD_RemoveQP(codec, 0);

						e = BD_RegisterQP(codec, new_node);
						if (e) return e;
						qp_on = 1;
						if (qp_local) qp_local = 2;
						if (codec->force_keep_qp) {
							e = InsertSFNode(field->far_ptr, new_node, -1);
						} else {
							Node_Register(new_node, NULL);
							Node_Unregister(new_node, node);
						}
					} else
						//this is generic MFNode container
						e = InsertSFNode(field->far_ptr, new_node, -1);
				} 
				//proto coding
				else if (codec->pCurrentProto) {
					//TO DO: what happens if this is a QP node on the interface ?
					ChainAddEntry(*(Chain **)field->far_ptr, new_node);
				}
			} else {
				return codec->LastError ? codec->LastError : M4NonCompliantBitStream;
			}
		}
		//according to the spec, the QP applies to the current node itself, 
		//not just children. If IsLocal is TRUE remove the node
		if (qp_on && qp_local) {
			if (qp_local == 2) {
				qp_local = 1;
			} else {
				//ask to get rid of QP and reactivate if we had a QP when entering the node
				BD_RemoveQP(codec, initial_qp);
				qp_local = 0;
			}
		}
	}
	//finally delete the QP if any (local or not) as we get out of this node
	if (qp_on) BD_RemoveQP(codec, 1);
	return M4OK;
}


void BD_CheckFieldChanges(SFNode *node, FieldInfo *field)
{
	if ((field->fieldType==FT_MFNode) || (field->fieldType==FT_MFNode)) node->sgprivate->is_dirty |= SG_CHILD_DIRTY;
	/*signal node modif*/
	SG_NodeChanged(node, field);
	/*Notify eventOut in all cases to handle protos*/
	Node_OnEventOut(node, field->fieldIndex);
	/*and propagate eventIn if any*/
	if (field->on_event_in) {
		field->on_event_in(node);
	} else if ((Node_GetTag(node) == TAG_MPEG4_Script) && (field->eventType==ET_EventIn)) {
		Script_EventIn(node, field);
	}

}

M4Err BD_DecField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	M4Err e;
	u8 flag, usePredictive;

	if (codec->LastError) return codec->LastError;

	assert(node);
	if (field->fieldType == FT_Unknown) return M4NonCompliantBitStream;
	
	if (VRML_IsSFField(field->fieldType)) {
		e = BD_DecSFField(codec, bs, node, field);
		if (e) return e;
	} else {
		/*clean up the eventIn field if not done*/
		if (field->eventType == ET_EventIn) {
			if (field->fieldType == FT_MFNode) {
				Node_UnregisterChildren(node, * (Chain **)field->far_ptr);
			} else {
				//remove all items of the MFField
				VRML_MF_Reset(field->far_ptr, field->fieldType);
			}
		}

		/*predictiveMFField*/
		usePredictive = 0;
		if (codec->info->config.UsePredictiveMFField) {
			usePredictive = BS_ReadInt(bs, 1);
		}

		if (usePredictive) 
			return BD_DecPredMFField(codec, bs, node, field);

		/*reserved*/
		flag = BS_ReadInt(bs, 1);
		if (!flag) {
			/*destroy the field content...*/
			if (field->fieldType != FT_MFNode) {
				e = VRML_MF_Reset(field->far_ptr, field->fieldType);
				if (e) return e;
			}
			/*List description - alloc is dynamic*/
			flag = BS_ReadInt(bs, 1);
			if (flag) {
				e = BD_DecMFFieldList(codec, bs, node, field);
			} else {
				e = BD_DecMFFieldVec(codec, bs, node, field);
			}
			if (e) return e;
		}
	}
	return M4OK;
}


M4Err BD_SetProtoISed(BifsDecoder * codec, u32 protofield, SFNode *n, u32 nodefield)
{
	/*take care of conditional execution in proto*/
	if (codec->current_graph->pOwningProto) {
		return ProtoInstance_SetISField((SFNode *) codec->current_graph->pOwningProto, protofield, n, nodefield);
	}
	/*regular ISed fields*/
	else {
		return Proto_SetISField(codec->pCurrentProto, protofield, n, nodefield);
	}
}

M4Err BD_DecListNodeDesc(BifsDecoder * codec, BitStream *bs, SFNode *node)
{
	u8 flag;
	M4Err e;
	u32 numBitsALL, numBitsDEF, field_all, field_ref, numProtoBits;
	FieldInfo field;

	e = M4OK;

	numProtoBits = numBitsALL = 0;
	if (codec->pCurrentProto) {
		numProtoBits = GetNumBits(Proto_GetFieldCount(codec->pCurrentProto) - 1);
		numBitsALL = GetNumBits(Node_GetNumFields(node, FCM_ALL)-1);
	}
	numBitsDEF = GetNumBits(Node_GetNumFields(node, FCM_DEF)-1);

	flag = BS_ReadInt(bs, 1);
	while (!flag) {
		if (codec->pCurrentProto) {
			//IS'ed flag
			flag = BS_ReadInt(bs, 1);
			if (flag) {
				//get field index in ALL mode for node
				field_ref = BS_ReadInt(bs, numBitsALL);
				//get field index in ALL mode for proto
				field_all = BS_ReadInt(bs, numProtoBits);
				e = Node_GetField(node, field_ref, &field);
				if (e) return e;
				e = BD_SetProtoISed(codec, field_all, node, field_ref);
				if (e) return e;
				flag = BS_ReadInt(bs, 1);
				continue;
			}
		}

		//fields are coded in DEF mode
		field_ref = BS_ReadInt(bs, numBitsDEF);
		e = Node_GetFieldIndex(node, field_ref, FCM_DEF, &field_all);
		if (e) return e;
		e = Node_GetField(node, field_all, &field);
		if (e) return e;
		e = BD_DecField(codec, bs, node, &field);
		if (e) return e;
		flag = BS_ReadInt(bs, 1);
	}
	return codec->LastError;
}

M4Err BD_DecMaskNodeDesc(BifsDecoder * codec, BitStream *bs, SFNode *node)
{
	u32 i, numFields, numProtoFields, index, flag, nbBits;
	M4Err e;
	FieldInfo field;

	//proto coding
	if (codec->pCurrentProto) {
		numFields = Node_GetNumFields(node, FCM_ALL);
		numProtoFields = Proto_GetFieldCount(codec->pCurrentProto);
		nbBits = GetNumBits(numProtoFields-1);

		for (i=0; i<numFields; i++) {
			flag = BS_ReadInt(bs, 1);
			if (!flag) continue;
			flag = BS_ReadInt(bs, 1);
			//IS'ed field, create route for binding to Proto declaration
			if (flag) {
				//reference index of our IS'ed proto field
				flag = BS_ReadInt(bs, nbBits);
				e = Node_GetField(node, i, &field);
				if (e) return e;
				e = BD_SetProtoISed(codec, flag, node, i);
			}
			//regular field, parse it (nb: no contextual coding for protos in maskNode, 
			//all node fields are coded
			else {
				e = Node_GetField(node, i, &field);
				if (e) return e;
				e = BD_DecField(codec, bs, node, &field);
			}
			if (e) return e;
		}
	}
	//Anim coding
	else {
		numFields = Node_GetNumFields(node, FCM_DEF);
		for (i=0; i<numFields; i++) {
			flag = BS_ReadInt(bs, 1);
			if (!flag) continue;
			Node_GetFieldIndex(node, i, FCM_DEF, &index);
			e = Node_GetField(node, index, &field);
			if (e) return e;
			e = BD_DecField(codec, bs, node, &field);
			if (e) return e;
		}
	}
	return M4OK;
}


static void UpdateTimeNode(BifsDecoder * codec, SFNode *node)
{
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_AnimationStream:
		BD_OffsetSFTime(codec, & ((M_AnimationStream*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_AnimationStream*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioBuffer:
		BD_OffsetSFTime(codec, & ((M_AudioBuffer*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_AudioBuffer*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioClip:
		BD_OffsetSFTime(codec, & ((M_AudioClip*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_AudioClip*)node)->stopTime);
		break;
	case TAG_MPEG4_AudioSource:
		BD_OffsetSFTime(codec, & ((M_AudioSource*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_AudioSource*)node)->stopTime);
		break;
	case TAG_MPEG4_MovieTexture:
		BD_OffsetSFTime(codec, & ((M_MovieTexture*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_MovieTexture*)node)->stopTime);
		break;
	case TAG_MPEG4_TimeSensor:
		BD_OffsetSFTime(codec, & ((M_TimeSensor*)node)->startTime);
		BD_OffsetSFTime(codec, & ((M_TimeSensor*)node)->stopTime);
		break;
	case TAG_ProtoNode:
	{
		u32 i, nbFields;
		FieldInfo inf;
		nbFields = Node_GetNumFields(node, FCM_ALL);
		for (i=0; i<nbFields; i++) {
			Node_GetField(node, i, &inf);
			if (inf.fieldType != FT_SFTime) continue;
			BD_CheckSFTimeOffset(codec, node, &inf);
		}
	}
		break;
	}
}

SFNode *BD_DecSFNode(BifsDecoder * codec, BitStream *bs, u32 NDT_Tag)
{
	u32 nodeID, NDTBits, node_type, node_tag, ProtoID, BVersion;
	u8 flag, node_flag;
	Bool skip_init;
	SFNode *new_node;
	M4Err e;
	LPPROTO proto;
	void SetupConditional(BifsDecoder *codec, SFNode *node);

	//to store the UseName
	char name[1000];

	/*should only happen with inputSensor, in which case this is BAAAAD*/
	if (!codec->info) {
		codec->LastError = M4BadParam;
		return NULL;
	}

	BVersion = BIFS_V1;
	node_flag = 0;
	flag = BS_ReadInt(bs, 1);

	//this is a USE statement
	if (flag) {
		nodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
		/*NULL node is encoded as USE with ID = all bits to 1*/
		if (nodeID == (u32) (1<<codec->info->config.NodeIDBits))
			return NULL;
		//find node and return it
		new_node = BIFS_FindNode(codec, nodeID);

		if (!new_node) {
			codec->LastError = M4InvalidNode;
		} else {
			/*restore QP14 length*/
			switch (Node_GetTag(new_node)) {
			case TAG_MPEG4_Coordinate:
				{
					u32 nbCoord = ((M_Coordinate *)new_node)->point.count;
					BD_EnterCoord(codec, 1);
					BD_SetCoordLength(codec, nbCoord);
					BD_EnterCoord(codec, 0);
				}
				break;
			case TAG_MPEG4_Coordinate2D:
				{
					u32 nbCoord = ((M_Coordinate2D *)new_node)->point.count;
					BD_EnterCoord(codec, 1);
					BD_SetCoordLength(codec, nbCoord);
					BD_EnterCoord(codec, 0);
				}
				break;
			}
		}
		return new_node;
	}

	//this is a new node
	nodeID = 0;
	strcpy(name, "");
	node_tag = 0;
	proto = NULL;

	//browse all node groups
	while (1) {
		NDTBits = NDT_GetNumBits(NDT_Tag, BVersion);
		/*this happens in replacescene where no top-level node is present (externProto)*/
		if ((BVersion==1) && (NDTBits > 8 * BS_Available(bs)) ) {
			codec->LastError = M4OK;
			return NULL;
		}

		node_type = BS_ReadInt(bs, NDTBits);
		if (node_type) break;

		//increment BIFS version
		BVersion += 1;
		//not supported
		if (BVersion > NUM_BIFS_VERSION) {
			codec->LastError = M4UnknowBIFSVersion;
			return NULL;
		}
	}
	if (BVersion==2 && node_type==1) {
		ProtoID = BS_ReadInt(bs, codec->info->config.ProtoIDBits);
		/*look in current graph for the proto - this may be a proto graph*/
		proto = SG_FindProto(codec->current_graph, ProtoID, NULL);
		/*this was in proto so look in main scene*/
		if (!proto && codec->current_graph != codec->scenegraph)
			proto = SG_FindProto(codec->scenegraph, ProtoID, NULL);

		if (!proto) {
			codec->LastError = M4UnknownNode;
			return NULL;
		}
	} else {
		node_tag = NDT_GetNodeTag(NDT_Tag, node_type, BVersion);
	}

	/*special handling of 3D mesh*/
	if ((node_tag == TAG_MPEG4_IndexedFaceSet) && codec->info->config.Use3DMeshCoding) {
		flag = BS_ReadInt(bs, 1);
		if (flag) {
			nodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
			if (codec->info->UseName) BD_GetName(bs, name);
		}
		/*parse the 3DMesh node*/
		return NULL;
	}
	/*unknow node*/
	if (!node_tag && !proto) {
		codec->LastError = M4UnknownNode;
		return NULL;
	}


	/*DEF'd flag*/
	flag = BS_ReadInt(bs, 1);
	
	if (flag) {
		if (!codec->info->config.NodeIDBits) {
			codec->LastError = M4NonCompliantBitStream;
			return NULL;
		}
		nodeID = 1 + BS_ReadInt(bs, codec->info->config.NodeIDBits);
		if (codec->info->UseName) BD_GetName(bs, name);
	}

	new_node = NULL;
	skip_init = 0;
	/*if a node with same DEF is already in the scene, use it
	we don't do that in memory mode because commands may force replacement
	of a node with a new node with same ID, and we want to be able to dump it (otherwise we would
	dump a USE)*/
	if (nodeID && !codec->dec_memory_mode) {
		new_node = BIFS_FindNode(codec, nodeID);
		if (new_node) {
			if (proto) {
				if ((Node_GetTag(new_node) != TAG_ProtoNode) || (Node_GetProto(new_node) != proto)) {
					codec->LastError = M4NonCompliantBitStream;
					return NULL;
				}
				skip_init = 1;
			} else {
				if (Node_GetTag(new_node) != node_tag) {
					codec->LastError = M4NonCompliantBitStream;
					return NULL;
				}
				skip_init = 1;
			}
		}
	}
	if (!new_node) {
		if (proto) {
			/*create proto interface*/
			new_node = Proto_CreateInstance(codec->current_graph, proto);
		} else {
			new_node = SG_NewNode(codec->current_graph, node_tag);
		}
	}
	if (!new_node) {
		codec->LastError = M4NotSupported;
		return NULL;
	}

	/*VRML: "The transformation hierarchy shall be a directed acyclic graph; results are undefined if a node 
	in the transformation hierarchy is its own ancestor"
	that's good, because the scene graph can't handle cyclic graphs (destroy will never be called).
	We therefore only register the node once parsed*/
	if (nodeID) {
		if (strlen(name)) {
			Node_SetDEF(new_node, nodeID, name);
		} else {
			Node_SetDEF(new_node, nodeID, NULL);
		}
	}


	/*update default time fields except in proto parsing*/
	if (!codec->pCurrentProto) UpdateTimeNode(codec, new_node);

	/*QP 14 is a special quant mode for IndexFace/Line(2D)Set to quantize the 
	coordonate(2D) child, based on the first field parsed
	we must check the type of the node and notfy the QP*/
	switch (node_tag) {
	case TAG_MPEG4_Coordinate:
	case TAG_MPEG4_Coordinate2D:
		BD_EnterCoord(codec, 1);
	}

	flag = BS_ReadInt(bs, 1);
	if (flag) {
		e = BD_DecMaskNodeDesc(codec, bs, new_node);
	} else {
		e = BD_DecListNodeDesc(codec, bs, new_node);
	}
	if (e) {
		codec->LastError = e;
		/*register*/
		Node_Register(new_node, NULL);
		/*unregister (deletes)*/
		Node_Unregister(new_node, NULL);
		return NULL;
	}

	/*nodes are only init outside protos */
	if (!proto && !codec->pCurrentProto && new_node && !skip_init) Node_Init(new_node);

	switch (node_tag) {
	case TAG_MPEG4_IndexedFaceSet:
	case TAG_MPEG4_IndexedFaceSet2D:
	case TAG_MPEG4_IndexedLineSet:
	case TAG_MPEG4_IndexedLineSet2D:
		BD_ResetQP14(codec);
		break;
	case TAG_MPEG4_Coordinate:
	case TAG_MPEG4_Coordinate2D:
		BD_EnterCoord(codec, 0);
		break;
	case TAG_MPEG4_Script:
		/*load script if in main graph (useless to load in proto declaration)*/
		if (codec->scenegraph == codec->current_graph) {
			Script_Load(new_node);
		}
		break;
	/*conditionals must be init*/
	case TAG_MPEG4_Conditional:
		SetupConditional(codec, new_node);
		break;
	}

	/*if new node is a proto and we're in the top scene, load proto code*/
	if (proto && new_node && (codec->scenegraph == codec->current_graph)) {
		codec->LastError = Proto_LoadCode(new_node);
	}
	return new_node;
}

