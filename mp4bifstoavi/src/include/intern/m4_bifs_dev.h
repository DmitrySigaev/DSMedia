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


#ifndef _M4_BIFS_DEV_H
#define _M4_BIFS_DEV_H


#include <m4_mpeg4_nodes.h>
#include <m4_bifs.h>

typedef struct
{
	/*v1 or v2*/
	u8 version;
	/*if false this is a BIFS-ANIM stream*/
	Bool IsCommandStream;
	/*BIFS config - common fields*/
	u16 NodeIDBits;
	u16 RouteIDBits;
	Bool PixelMetrics;
	/*set to 0, 0 if no size is specified*/
	u16 Width, Height;

	/*BIFS-Anim - not supported */
	/*if 1 the BIFS_Anim codec is reset at each intra frame*/
	Bool BAnimRAP;
	/*will be specified once we have BIFS anim*/
	void *AnimMask;

	/*BIFS v2 add-on*/
	Bool Use3DMeshCoding;
	Bool UsePredictiveMFField;
	u16 ProtoIDBits;
} BIFSConfig;



/*per_stream config support*/
typedef struct 
{
	BIFSConfig config;
	Bool UseName;
	u16 ESID;
} BIFSStreamInfo;


typedef struct _tagBDecoder
{
	M4Err LastError;
	/*all attached streams*/
	Chain *streamInfo;
	/*active stream*/
	BIFSStreamInfo *info;

	LPSCENEGRAPH scenegraph;
	/*modified during conditional execution / proto parsing*/
	LPSCENEGRAPH current_graph;

	/*Quantization*/
	/*QP stack*/
	Chain *QPs;
	/*active QP*/
	M_QuantizationParameter *ActiveQP;

	/*QP 14 stuff: we need to store the last numb of fields in the last recieved Coord //field (!!!)*/
	
	/*number of iten in the Coord field*/
	u32 NumCoord;
	Bool coord_stored, storing_coord;

	/*active QP*/
	M_QuantizationParameter *GlobalQP;


	/*only set at SceneReplace during proto parsing, NULL otherwise*/
	LPPROTO pCurrentProto;

	Double (*GetSceneTime)(void *st_cbk);
	void *st_cbk;

	/*when set the decoder works with commands rather than modifying the scene graph directly*/
	Bool dec_memory_mode;
	Bool force_keep_qp;
	/*only set in mem mode. Conditionals/InputSensors are stacked while decoding, then decoded once the AU is decoded
	to make sure all nodes potentially used by the conditional command buffer are created*/
	Chain *conditionals;

	Bool ignore_size;
	
	M4Mutex *mx;
} BifsDecoder;


/*decodes an SFNode*/
SFNode *BD_DecSFNode(BifsDecoder * codec, BitStream *bs, u32 NDT_Tag);
/*decodes an SFField (to get a ptr to the field, use Node_GetField )
the FieldIndex is used for Quantzation*/
M4Err BD_DecSFField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);
/*decodes a Field (either SF or MF). The field MUST BE EMPTY*/
M4Err BD_DecField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);
/*decodes a route*/
M4Err BD_DecRoute(BifsDecoder * codec, BitStream *bs, Bool is_insert);
/*get name*/
void BD_GetName(BitStream *bs, char *name);

BIFSStreamInfo *BD_GetStream(BifsDecoder * codec, u16 ESID);
/*decodes a BIFS command frame*/
M4Err BIFS_ParseCommand(BifsDecoder * codec, BitStream *bs);
/*decodes proto list - if proto_list is not NULL, protos parsed are not registered with the parent graph
and added to the list*/
M4Err BD_DecProtoList(BifsDecoder * codec, BitStream *bs, Chain *proto_list);
/*decodes field(s) of a node - exported for MultipleReplace*/
M4Err BD_DecListNodeDesc(BifsDecoder * codec, BitStream *bs, SFNode *node);
M4Err BD_DecMaskNodeDesc(BifsDecoder * codec, BitStream *bs, SFNode *node);


SFNode *BIFS_FindNode(BifsDecoder * codec, u32 NodeID);

u32 GetNumBits(u32 MaxVal);

/*called once a field has been modified through a command, send eventOut or propagate eventIn if needed*/
void BD_CheckFieldChanges(SFNode *node, FieldInfo *field);


typedef struct _tagBEncoder
{
	M4Err LastError;
	/*all attached streams*/
	Chain *streamInfo;
	/*active stream*/
	BIFSStreamInfo *info;

	/*the scene graph the codec is encoding (set htrough ReplaceScene or manually)*/
	LPSCENEGRAPH scene_graph;
	/*current proto graph for DEF/USE*/
	LPSCENEGRAPH current_proto_graph;

	/*Quantization*/
	/*QP stack*/
	Chain *QPs;
	/*active QP*/
	M_QuantizationParameter *ActiveQP;

	/*active QP*/
	M_QuantizationParameter *GlobalQP;

	u32 NumCoord;
	Bool coord_stored, storing_coord;

	LPPROTO encoding_proto;

	M4Mutex *mx;

	/*keep track of DEF/USE*/
	Chain *encoded_nodes;

	FILE *trace;
} BifsEncoder;

M4Err BIFS_EncCommands(BifsEncoder *codec, Chain *comList, BitStream *bs);

M4Err BE_EncSFNode(BifsEncoder * codec, SFNode *node, u32 NDT_Tag, BitStream *bs);
M4Err BE_EncSFField(BifsEncoder *codec, BitStream *bs, SFNode *node, FieldInfo *field);
M4Err BE_EncField(BifsEncoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);
M4Err BE_EncMFField(BifsEncoder *codec, BitStream *bs, SFNode *node, FieldInfo *field);
M4Err BE_EncRoute(BifsEncoder *codec, LPROUTE r, BitStream *bs);
void BE_SetName(BifsEncoder *codec, BitStream *bs, char *name);
SFNode *BE_FindNode(BifsEncoder *codec, u32 nodeID);

void BE_LogBits(BifsEncoder *codec, s32 val, u32 nbBits, char *str, char *com);

#define BE_WRITE_INT(codec, bs, val, nbBits, str, com)	{\
	BS_WriteInt(bs, val, nbBits);	\
	BE_LogBits(codec, val, nbBits, str, com);	}\


LPROUTE NodeIsSedField(BifsEncoder *codec, SFNode *node, u32 fieldIndex);


/*get field QP and anim info*/
Bool Node_GetAQInfo(SFNode *Node, u32 FieldIndex, u8 *QType, u8 *AType, Float *b_min, Float *b_max, u32 *QT13_bits);

/*get the absolute field 0_based index (or ALL mode) given the field index in IndexMode*/
M4Err Node_GetFieldIndex(SFNode *Node, u32 inField, u8 IndexMode, u32 *allField);

/*returns the opaque NodeDataType of the node "children" field if any, or 0*/
u32 Node_GetChildTable(SFNode *Node);

/*returns binary type of node in the given version of the desired NDT*/
u32 NDT_GetNodeType(u32 NDT_Tag, u32 NodeTag, u32 Version);

/*converts field index from all_mode to given mode*/
M4Err BIFS_ModeFieldIndex(SFNode *node, u32 all_ind, u8 indexMode, u32 *outField);

/*return number of bits needed to code all nodes present in the specified NDT*/
u32 NDT_GetNumBits(u32 NDT_Tag, u32 Version);
/*return absolute node tag given its type in the NDT and the NDT version number*/
u32 NDT_GetNodeTag(u32 NDT_Tag, u32 NodeType, u32 Version);

/*set QP and anim info for a proto field (BIFS allows for that in proto coding)*/
M4Err ProtoField_SetQuantizationInfo(LPPROTOFIELD field, u32 QP_Type, u32 hasMinMax, u32 QPSFType, void *qp_min_value, void *qp_max_value, u32 QP13_NumBits);

#endif	//_M4_BIFS_DEV_H

