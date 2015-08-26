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

#include "Quantize.h"


M4Err BE_RegisterQP(BifsEncoder *codec, SFNode *qp)
{
	if (Node_GetTag(qp) != TAG_MPEG4_QuantizationParameter) return M4BadParam;

	/*if we have an active QP, push it into the stack*/
	if (codec->ActiveQP && (codec->ActiveQP != codec->GlobalQP) ) 
		ChainInsertEntry(codec->QPs, codec->ActiveQP, 0);
	
	codec->ActiveQP = (M_QuantizationParameter *)qp;
	return M4OK;
}

M4Err BE_RemoveQP(BifsEncoder *codec, Bool ActivatePrev)
{
	codec->ActiveQP = NULL;
	if (!ActivatePrev) return M4OK;

	if (ChainGetCount(codec->QPs)) {
		codec->ActiveQP = ChainGetEntry(codec->QPs, 0);
		ChainDeleteEntry(codec->QPs, 0);
	} else if (codec->GlobalQP) {
		codec->ActiveQP = codec->GlobalQP;
	}
	return M4OK;
}


u32 BE_GetCoordBits(BifsEncoder *codec)
{
	if (!codec->ActiveQP || !codec->coord_stored) return 0;
	return (u32) ceil(log(codec->NumCoord+1) / log(2) ); 
}

void BE_EnterCoord(BifsEncoder *codec, Bool Enter)
{
	if (!codec->ActiveQP) return;
	if (Enter) codec->storing_coord = 1;
	else {
		if (codec->storing_coord) codec->coord_stored = 1;
		codec->storing_coord = 0;
	}
}

void BE_ResetQP14(BifsEncoder *codec)
{
	codec->coord_stored = 0;
	codec->storing_coord = 0;
	codec->NumCoord = 0;
}

void BE_SetCoordLength(BifsEncoder *codec, u32 NbElements)
{
	if (!codec->ActiveQP || !codec->storing_coord || codec->coord_stored) return;
	codec->NumCoord = NbElements;
}

void BE_WriteMantissaFloat(BifsEncoder *codec, SFFloat ft, BitStream *bs)
{
	u32 mantLength, expLength, mantSign, mantissa, expSign, i, nbBits;
	s32 exp;

	union
	{	
		SFFloat f;
		s32 l;
	} ft_val;
	ft_val.f = ft;
	if(ft ==0) {
	    BS_WriteInt(bs, 0, 4);
		return;
	}
  
	mantSign = ((ft_val.l & 0x80000000) >> 31) & 0x1;
	mantissa = (ft_val.l & 0x007FFFFF) >> 9;
	mantLength = 15;
	expSign=0;
	exp =(((ft_val.l & 0x7F800000) >> 23)-127);
	expLength = 8;
  
	if (mantissa == 0) mantLength = 1;
  

	if (exp) {
		if (exp< 0) {
			expSign = 1;
			exp = -exp;	    
		}
		while ((exp & (1<<(--expLength)))==0) { }
		exp &= ~(1<<expLength);
		expLength++;
	} else {
		expLength=0;
	}
  
	nbBits=0;
	for(i = mantissa; i>0; ++nbBits) i >>= 1;

	BS_WriteInt(bs, nbBits+1, 4);
	if (mantLength) {
		BS_WriteInt(bs, expLength, 3);
		BS_WriteInt(bs, mantSign, 1);
		BS_WriteInt(bs, mantissa, nbBits);
		if(expLength) {
			BS_WriteInt(bs, expSign, 1);
			BS_WriteInt(bs, exp, expLength - 1);
		}
	}
}


//Linear Quantization for floats
s32 Q_Quantize(SFFloat Min, SFFloat Max, u32 NbBits, SFFloat value)
{
	if (value <= Min) return 0;
	if (value >= Max) return (1<<NbBits)-1;
	return (s32) floor( (value - Min) * ((1 << NbBits) - 1) / (Max - Min) + 0.5);
}


M4Err Q_EncFloat(BifsEncoder *codec, BitStream *bs, u32 FieldType, SFVec3f BMin, SFVec3f BMax, u32 NbBits, void *field_ptr)
{
	s32 newVal;
	switch (FieldType) {
	case FT_SFInt32:
		return M4InvalidNode;
	case FT_SFFloat:
		newVal = Q_Quantize(BMin.x, BMax.x, NbBits, *((SFFloat *)field_ptr));
		BS_WriteInt(bs, newVal, NbBits);
		return M4OK;
	case FT_SFVec2f:
		newVal = Q_Quantize(BMin.x, BMax.x, NbBits, ((SFVec2f *)field_ptr)->x);
		BS_WriteInt(bs, newVal, NbBits);
		newVal = Q_Quantize(BMin.y, BMax.y, NbBits, ((SFVec2f *)field_ptr)->y);
		BS_WriteInt(bs, newVal, NbBits);
		return M4OK;
	case FT_SFVec3f:
		newVal = Q_Quantize(BMin.x, BMax.x, NbBits, ((SFVec3f *)field_ptr)->x);
		BS_WriteInt(bs, newVal, NbBits);
		newVal = Q_Quantize(BMin.y, BMax.y, NbBits, ((SFVec3f *)field_ptr)->y);
		BS_WriteInt(bs, newVal, NbBits);
		newVal = Q_Quantize(BMin.z, BMax.z, NbBits, ((SFVec3f *)field_ptr)->z);
		BS_WriteInt(bs, newVal, NbBits);
		return M4OK;
	case FT_SFColor:
		newVal = Q_Quantize(BMin.x, BMax.x, NbBits, ((SFColor *)field_ptr)->red);
		BS_WriteInt(bs, newVal, NbBits);
		newVal = Q_Quantize(BMin.y, BMax.y, NbBits, ((SFColor *)field_ptr)->green);
		BS_WriteInt(bs, newVal, NbBits);
		newVal = Q_Quantize(BMin.z, BMax.z, NbBits, ((SFColor *)field_ptr)->blue);
		BS_WriteInt(bs, newVal, NbBits);
		return M4OK;

	case FT_SFRotation:
		//forbidden in this Q mode
		return M4InvalidNode;
	}
	return M4OK;
}

//int in quant are either Linear Scalar fields or CoordIndex
//the quant is just a bitshifting into [0, 2^NbBits-1]
//so v = value - b_min
M4Err Q_EncInt(BifsEncoder *codec, BitStream *bs, u32 QType, SFInt32 b_min, u32 NbBits, void *field_ptr)
{
	switch (QType) {
	case QC_LINEAR_SCALAR:
	case QC_COORD_INDEX:
		BS_WriteInt(bs, *((SFInt32 *)field_ptr) - b_min, NbBits);
		return M4OK;
	default:
		return M4NonCompliantBitStream;
	}
}

M4Err Q_EncCoordOnUnitSphere(BifsEncoder *codec, BitStream *bs, u32 NbBits, u32 NbComp, SFFloat *m_ft) 
{
	u32 i;
	u32 len = NbComp+1;
	s32 orientation =-1;
	Float maxTmp=-FLT_MAX;
	for (i=0; i<len; i++) {
		if (fabs(m_ft[i]) > maxTmp) {
			maxTmp = (Float) fabs(m_ft[i]);
			orientation = i;
		}
	}
	if(NbComp==2) BS_WriteInt(bs, ((m_ft[orientation]>0) ? 0 : 1), 1); 
	BS_WriteInt(bs, orientation, 2);
	for (i=0; i<NbComp; i++) {
		Float v = (Float) (4/QP_PI * atan( m_ft[(orientation+i+1) % len] / m_ft[orientation]) );
		s32 qdt = Q_Quantize(0, 1, NbBits-1, (v>=0 ? v : -v));
		s32 qv = (1<<(NbBits-1)) + (v>=0 ? 1 : -1) * qdt;
		BS_WriteInt(bs, qv, NbBits);
	}
	return M4OK;
}

M4Err Q_EncNormal(BifsEncoder *codec, BitStream *bs, u32 NbBits, void *field_ptr)
{
	Float comp[3];
	Float delta;
	u32 i;
	comp[0] = ((SFVec3f *)field_ptr)->x;
	comp[1] = ((SFVec3f *)field_ptr)->y;
	comp[2] = ((SFVec3f *)field_ptr)->z;

	delta = 0;
	for(i=0; i<3; i++) delta += comp[i] * comp[i];
	delta = (Float) sqrt(delta);
	assert(delta);
	for(i=0; i<3; i++) comp[i] /= delta;
	return Q_EncCoordOnUnitSphere(codec, bs, NbBits, 2, comp);
}

M4Err Q_EncRotation(BifsEncoder *codec, BitStream *bs, u32 NbBits, void *field_ptr)
{
	Float comp[4];
	Float delta, scale, a;
	u32 i;
	/*get quaternion*/
	a = ((SFRotation *)field_ptr)->angle;
	comp[1] = ((SFRotation *)field_ptr)->xAxis;
	comp[2] = ((SFRotation *)field_ptr)->yAxis;
	comp[3] = ((SFRotation *)field_ptr)->zAxis;
	delta = 0;
	for(i=1; i<4; i++) delta += comp[i] * comp[i];
	delta = (Float) sqrt(delta);
	comp[0] = (Float) cos(a / 2);
	scale = (Float) sin(a/2)/delta;
	comp[1] *= scale;
	comp[2] *= scale;
	comp[3] *= scale;

	return Q_EncCoordOnUnitSphere(codec, bs, NbBits, 3, comp);
}

M4Err BE_QuantizeField(BifsEncoder *codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	Bool HasQ;
	u8 QType, AType;
	u32 NbBits;
	Float b_min, b_max;
	SFVec3f BMin, BMax;
	M4Err e;

	/*check QP*/
	if (!codec->ActiveQP) return M4FieldNotQuantized;
	/*check FieldType*/
	switch (field->fieldType) {
	case FT_SFInt32:
	case FT_SFFloat:
	case FT_SFRotation:
	case FT_SFVec2f:
	case FT_SFVec3f:
		break;
	case FT_SFColor:
		break;
	default:
		return M4FieldNotQuantized;
	}
	
	/*check NDT*/
	HasQ = Node_GetAQInfo(node, field->fieldIndex, &QType, &AType, &b_min, &b_max, &NbBits);
	if (!HasQ || !QType) return M4FieldNotQuantized;

	/*get NbBits for QP14 (QC_COORD_INDEX)*/
	if (QType == QC_COORD_INDEX) {
		NbBits = BE_GetCoordBits(codec);
		/*QP14 is always on, not having NbBits set means the coord field is set after the index field, hence not decodable*/
		if (!NbBits) 
			return M4NonCompliantBitStream;
	}

	BMin.x = BMin.y = BMin.z = b_min;
	BMax.x = BMax.y = BMax.z = b_max;

	/*check is the QP is on and retrieves the bounds*/
	if (!Q_IsTypeOn(codec->ActiveQP, QType, &NbBits, &BMin, &BMax)) return M4FieldNotQuantized;

	/*ok the field is Quantized, dequantize*/
	switch (QType) {
	//these are all SFFloat quantized on n fields
	case QC_3DPOS:
	case QC_2DPOS:
	case QC_ORDER:
	case QC_COLOR:
	case QC_TEXTURE_COORD:
	case QC_ANGLE:
	case QC_SCALE:
	case QC_INTERPOL_KEYS:
	case QC_SIZE_3D:
	case QC_SIZE_2D:
		e = Q_EncFloat(codec, bs, field->fieldType, BMin, BMax, NbBits, field->far_ptr);
		break;
	//SFInt types
	case QC_LINEAR_SCALAR:
	case QC_COORD_INDEX:
		e = Q_EncInt(codec, bs, QType, (SFInt32) b_min, NbBits, field->far_ptr);
		break;
	//normalized fields (normals and vectors)
	case QC_NORMALS:
		//normal quant is only for SFVec3F
		if (field->fieldType != FT_SFVec3f) return M4InvalidNode;
		e = Q_EncNormal(codec, bs, NbBits, field->far_ptr);
		break;
	case QC_ROTATION:
		//normal quant is only for SFVec3F
		if (field->fieldType != FT_SFRotation) return M4InvalidNode;
		e = Q_EncRotation(codec, bs, NbBits, field->far_ptr);
		break;
	default:
		return M4BadParam;
	}
	return e;
}
