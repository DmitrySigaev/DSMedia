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


#ifndef __QUANTIZE_H
#define __QUANTIZE_H

#include <intern/m4_bifs_dev.h>

/*math.h is not included in main config (C++ clash on win32)*/
#include <math.h>

#define QP_PI		3.1415926535897932384626433832795

/*Quantization Categories*/
enum
{
	QC_NONE				=	0,
	QC_3DPOS			=	1, 
	QC_2DPOS			=	2, 
	QC_ORDER			=	3, 
	QC_COLOR			=	4, 
	QC_TEXTURE_COORD	=	5, 
	QC_ANGLE			=	6, 
	QC_SCALE			=	7, 
	QC_INTERPOL_KEYS	=	8, 
	QC_NORMALS			=	9, 
	QC_ROTATION			=	10, 
	QC_SIZE_3D			=	11, 
	QC_SIZE_2D			=	12, 
	QC_LINEAR_SCALAR	=	13, 
	QC_COORD_INDEX		=	14, 
	QC_RESERVED			=	15,
	QC_NOTDEF			=	16,
};

M4Err BD_DecPredMFField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);

Bool Q_IsTypeOn(M_QuantizationParameter *qp, u32 q_type, u32 *NbBits, SFVec3f *b_min, SFVec3f *b_max);


/*QP14 decode*/
u32 BD_GetCoordBits(BifsDecoder *codec);
void BD_EnterCoord(BifsDecoder * codec, Bool Enter);
void BD_ResetQP14(BifsDecoder * codec);
void BD_SetCoordLength(BifsDecoder * codec, u32 NbElements);
/*QP decoder (un)registration*/
M4Err BD_RegisterQP(BifsDecoder *codec, SFNode *qp);
M4Err BD_RemoveQP(BifsDecoder *codec, Bool ActivatePrev);

SFFloat BD_ReadMantissaFloat(BifsDecoder * codec, BitStream *bs);
M4Err BD_UnquantizeField(BifsDecoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);

/*QP14 encode*/
u32 BE_GetCoordBits(BifsEncoder *codec);
void BE_EnterCoord(BifsEncoder *codec, Bool Enter);
void BE_ResetQP14(BifsEncoder *codec);
void BE_SetCoordLength(BifsEncoder *codec, u32 NbElements);
/*QP encoder (un)registration*/
M4Err BE_RegisterQP(BifsEncoder *codec, SFNode *qp);
M4Err BE_RemoveQP(BifsEncoder *codec, Bool ActivatePrev);
void BE_WriteMantissaFloat(BifsEncoder * codec, SFFloat val, BitStream *bs);
M4Err BE_QuantizeField(BifsEncoder * codec, BitStream *bs, SFNode *node, FieldInfo *field);



/*
		Predictive MFField decode - mainly IM1 code (GPL H263 AA coder used)
*/

typedef struct _aamodel AAModel;
AAModel *NewAAModel();
void DeleteAAModel(AAModel *model);
void InitAAModel(AAModel *model, u32 nbBits);

typedef struct _aadecoder AADecoder;
AADecoder *NewAADecoder(BitStream *bs);
void DeleteAADecoder(AADecoder *dec);
void AADec_FlushChoice(AADecoder *dec);
/*get input bit*/
s32 AADec_GetBitFlag(AADecoder *dec);
/*resync after input bit has been fetched (full buffer (16 bit) rewind in source stream)*/
void AADec_ResyncBitFlag(AADecoder *dec);
/*resync bitstream*/
void AADec_Resync(AADecoder *dec);
/*decode symbol in given model*/
s32 AADec_Decode(AADecoder *dec, AAModel *model);
/*reset decoder - called after each parsed I frame*/
void AADec_Reset(AADecoder *dec);


#endif	
