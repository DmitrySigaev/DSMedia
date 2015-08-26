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

u32 BD_GetCoordBits(BifsDecoder *codec)
{
	if (!codec->ActiveQP || !codec->coord_stored) return 0;
	return (u32) ceil(log(codec->NumCoord+1) / log(2) ); 
}

void BD_EnterCoord(BifsDecoder * codec, Bool Enter)
{
	if (!codec->ActiveQP) return;
	if (Enter) codec->storing_coord = 1;
	else {
		if (codec->storing_coord) codec->coord_stored = 1;
		codec->storing_coord = 0;
	}
}

void BD_ResetQP14(BifsDecoder * codec)
{
	codec->coord_stored = 0;
	codec->storing_coord = 0;
	codec->NumCoord = 0;
}

void BD_SetCoordLength(BifsDecoder * codec, u32 NbElements)
{
	if (!codec->ActiveQP || !codec->storing_coord || codec->coord_stored) return;
	codec->NumCoord = NbElements;
}

M4Err BD_RegisterQP(BifsDecoder *codec, SFNode *qp)
{
	if (Node_GetTag(qp) != TAG_MPEG4_QuantizationParameter) return M4BadParam;

	/*if we have an active QP, push it into the stack*/
	if (codec->ActiveQP && (codec->ActiveQP != codec->GlobalQP) ) 
		ChainInsertEntry(codec->QPs, codec->ActiveQP, 0);
	
	codec->ActiveQP = (M_QuantizationParameter *)qp;
	return M4OK;
}

M4Err BD_RemoveQP(BifsDecoder *codec, Bool ActivatePrev)
{
	if (!codec->force_keep_qp && codec->ActiveQP && (codec->ActiveQP != codec->GlobalQP) ) {
		Node_Unregister((SFNode *) codec->ActiveQP, NULL);
	}
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

//parses efficient float
SFFloat BD_ReadMantissaFloat(BifsDecoder *codec, BitStream *bs)
{
	u32 mantLength, expLength, mantSign, mantissa, expSign, exponent;
	unsigned char exp;

	union {	
		SFFloat f;
		long l;
	} ft_value;

	mantLength = BS_ReadInt(bs, 4);
	if (!mantLength) return (SFFloat) 0.0;

	expLength = BS_ReadInt(bs, 3);
	mantSign = BS_ReadInt(bs, 1);
	mantissa = BS_ReadInt(bs, mantLength - 1);

	expSign = exponent = 0;
	exp = 0;

	exp = 127;
	if (expLength) {
		expSign = BS_ReadInt(bs, 1);
		exponent = BS_ReadInt(bs, expLength-1);
		exp += (1-2*expSign)*( (1 << (expLength-1) ) + exponent);
	}

	ft_value.l = mantSign << 31;
	ft_value.l |= (exp & 0xff) << 23;
	ft_value.l |= mantissa << 9;
	return ft_value.f;
}

//check if the quant type is on in the QP, and if so retrieves NbBits and Min Max
//specified for the field
Bool Q_IsTypeOn(M_QuantizationParameter *qp, u32 q_type, u32 *NbBits, SFVec3f *b_min, SFVec3f *b_max)
{
	switch (q_type) {
	case QC_3DPOS:
		if (!qp->position3DQuant) return 0;
		*NbBits = qp->position3DNbBits;
		b_min->x = MAX(b_min->x, qp->position3DMin.x);
		b_min->y = MAX(b_min->y, qp->position3DMin.y);
		b_min->z = MAX(b_min->z, qp->position3DMin.z);
		b_max->x = MIN(b_max->x, qp->position3DMax.x);
		b_max->y = MIN(b_max->y, qp->position3DMax.y);
		b_max->z = MIN(b_max->z, qp->position3DMax.z);
		return 1;
	case QC_2DPOS:
		if (!qp->position2DQuant) return 0;
		*NbBits = qp->position2DNbBits;
		b_min->x = MAX(b_min->x, qp->position2DMin.x);
		b_min->y = MAX(b_min->y, qp->position2DMin.y);
		b_max->x = MIN(b_max->x, qp->position2DMax.x);
		b_max->y = MIN(b_max->y, qp->position2DMax.y);
		return 1;
	case QC_ORDER:
		if (!qp->drawOrderQuant) return 0;
		*NbBits = qp->drawOrderNbBits;
		b_min->x = MAX(b_min->x, qp->drawOrderMin);
		b_max->x = MIN(b_max->x, qp->drawOrderMax);
		return 1;
	case QC_COLOR:
		if (!qp->colorQuant) return 0;
		*NbBits = qp->colorNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->colorMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->colorMax);
		return 1;
	case QC_TEXTURE_COORD:
		if (!qp->textureCoordinateQuant) return 0;
		*NbBits = qp->textureCoordinateNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->textureCoordinateMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->textureCoordinateMax);
		return 1;
	case QC_ANGLE:
		if (!qp->angleQuant) return 0;
		*NbBits = qp->angleNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->angleMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->angleMax);
		return 1;
	case QC_SCALE:
		if (!qp->scaleQuant) return 0;
		*NbBits = qp->scaleNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->scaleMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->scaleMax);
		return 1;
	case QC_INTERPOL_KEYS:
		if (!qp->keyQuant) return 0;
		*NbBits = qp->keyNbBits;
		b_min->x = MAX(b_min->x, qp->keyMin);
		b_min->y = MAX(b_min->y, qp->keyMin);
		b_min->z = MAX(b_min->z, qp->keyMin);
		b_max->x = MIN(b_max->x, qp->keyMax);
		b_max->y = MIN(b_max->y, qp->keyMax);
		b_max->z = MIN(b_max->z, qp->keyMax);
		return 1;
	case QC_NORMALS:
		if (!qp->normalQuant) return 0;
		*NbBits = qp->normalNbBits;
		b_min->x = b_min->y = b_min->z = 0;
		b_max->x = b_max->y = b_max->z = 1.0;
		return 1;
	case QC_ROTATION:
		if (!qp->normalQuant) return 0;
		*NbBits = qp->normalNbBits;
		b_min->x = b_min->y = b_min->z = 0;
		b_max->x = b_max->y = b_max->z = 1.0;
		return 1;
	case QC_SIZE_3D:
		if (!qp->sizeQuant) return 0;
		*NbBits = qp->sizeNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->sizeMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->sizeMax);
		return 1;
	case QC_SIZE_2D:
		if (!qp->sizeQuant) return 0;
		*NbBits = qp->sizeNbBits;
		b_min->x = b_min->y = b_min->z = MAX(b_min->x, qp->sizeMin);
		b_max->x = b_max->y = b_max->z = MIN(b_max->x, qp->sizeMax);
		return 1;

	//cf specs, from here ALWAYS ON
	case QC_LINEAR_SCALAR:
		//nbBits is the one from the FCT - DO NOT CHANGE IT
		return 1;
	case QC_COORD_INDEX:
		//nbBits has to be recomputed on the fly
		return 1;
	case QC_RESERVED:
		*NbBits = 0;
		return 1;
	default:
		return 0;
	}
}


//Linear inverse Quantization for floats
SFFloat Q_InverseQuantize(SFFloat Min, SFFloat Max, u32 NbBits, u32 value)
{
	if (!value) return Min;
	if (value == (u32) ((1 << NbBits) - 1) ) return Max;
	return (SFFloat) (Min + (Max - Min)/((SFFloat) (1 << NbBits) - 1) * (SFFloat) value);
}


M4Err Q_DecFloat(BifsDecoder *codec, BitStream *bs, u32 FieldType, SFVec3f BMin, SFVec3f BMax, u32 NbBits, void *field_ptr)
{
	switch (FieldType) {
	case FT_SFInt32:
		return M4InvalidNode;
	case FT_SFFloat:
		*((SFFloat *)field_ptr) = Q_InverseQuantize(BMin.x, BMax.x, NbBits, BS_ReadInt(bs, NbBits)); 
		return M4OK;
	case FT_SFVec2f:
		((SFVec2f *)field_ptr)->x = Q_InverseQuantize(BMin.x, BMax.x, NbBits, BS_ReadInt(bs, NbBits));
		((SFVec2f *)field_ptr)->y = Q_InverseQuantize(BMin.y, BMax.y, NbBits, BS_ReadInt(bs, NbBits));
		return M4OK;
	case FT_SFVec3f:
		((SFVec3f *)field_ptr)->x = Q_InverseQuantize(BMin.x, BMax.x, NbBits, BS_ReadInt(bs, NbBits));
		((SFVec3f *)field_ptr)->y = Q_InverseQuantize(BMin.y, BMax.y, NbBits, BS_ReadInt(bs, NbBits));
		((SFVec3f *)field_ptr)->z = Q_InverseQuantize(BMin.z, BMax.z, NbBits, BS_ReadInt(bs, NbBits));
		return M4OK;
	case FT_SFColor:
		((SFColor *)field_ptr)->red = Q_InverseQuantize(BMin.x, BMax.x, NbBits, BS_ReadInt(bs, NbBits));
		((SFColor *)field_ptr)->green = Q_InverseQuantize(BMin.y, BMax.y, NbBits, BS_ReadInt(bs, NbBits));
		((SFColor *)field_ptr)->blue = Q_InverseQuantize(BMin.z, BMax.z, NbBits, BS_ReadInt(bs, NbBits));
		return M4OK;

	case FT_SFRotation:
		//forbidden in this Q mode
		return M4InvalidNode;
	}
	return M4OK;
}

//int in quant are either Linear Scalar fields or CoordIndex
//the quant is just a bitshifting into [0, 2^NbBits-1]
//so IntMin + ReadBit(NbBits) = value
M4Err Q_DecInt(BifsDecoder *codec, BitStream *bs, u32 QType, SFInt32 b_min, u32 NbBits, void *field_ptr)
{
	switch (QType) {
	case QC_LINEAR_SCALAR:
	case QC_COORD_INDEX:
		*((SFInt32 *)field_ptr) = BS_ReadInt(bs, NbBits) + b_min;
		return M4OK;
	default:
		return M4NonCompliantBitStream;
	}
}

//SFRotation and SFVec3f are quantized as normalized vectors ,mapped on a cube 
//in the UnitSphere (R=1.0)
M4Err Q_DecCoordOnUnitSphere(BifsDecoder *codec, BitStream *bs, u32 NbBits, u32 NbComp, SFFloat *m_ft)
{
    u32 i, orient, value, sign;
	SFFloat tang[4];
	SFFloat dir, delta;

	if (NbComp != 2 && NbComp != 3) return M4BadParam;

	//only 2 or 3 comp in the quantized version
	dir = 1;
	if(NbComp == 2) 
		dir -= (SFFloat) (2 * BS_ReadInt(bs, 1) );

	//the main orientation of the vector is coded IN the bitstream (eg
	//the prnincipal component of the vector 0, 1, 2)
	orient = BS_ReadInt(bs, 2);
	
	//extract composantes
	for(i=0; i<NbComp; i++) {
		value = BS_ReadInt(bs, NbBits) - (1 << (NbBits-1) );
		sign = (value >=0 ) ? 1 : -1;
		m_ft[i] = sign * Q_InverseQuantize(0, 1, NbBits-1, sign*value);
	}
	//extract angle
	delta = 1;
	for (i=0; i<NbComp; i++) {
		tang[i] = (SFFloat) tan( QP_PI * m_ft[i] / 4);
		delta += tang[i] * tang[i];
	}
	delta = (SFFloat) ( dir / sqrt(delta) );
	m_ft[orient] = delta;

	for (i=0;i<NbComp;i++) {
		m_ft[ (orient + i+1) % (NbComp+1) ] = tang[i] * delta;  
	}
	return M4OK;
}

//parses a rotation
M4Err Q_DecRotation(BifsDecoder *codec, BitStream *bs, u32 NbBits, void *field_ptr)
{
	u32 i;
	SFFloat angle, epsil, sin2, comp[4];
	M4Err e;

	e = Q_DecCoordOnUnitSphere(codec, bs, NbBits, 3, comp);
	if (e) return e;

	angle = (SFFloat) ( 2 * acos(comp[0]) );
	epsil = M4_EPSILON_FLOAT;
	sin2 = (SFFloat) sin(angle / 2);

	if (fabs(sin2) <= epsil) {
		for(i=1; i<4; i++) comp[i] = 0;
		comp[3] = 1;
	} else {
		for(i=1; i<4; i++) comp[i] /= sin2;
	}
	((SFRotation *)field_ptr)->xAxis = comp[1];
	((SFRotation *)field_ptr)->yAxis = comp[2];
	((SFRotation *)field_ptr)->zAxis = comp[3];
	((SFRotation *)field_ptr)->angle = angle;
	return M4OK;
}

//parses a Normal vec
M4Err Q_DecNormal(BifsDecoder *codec, BitStream *bs, u32 NbBits, void *field_ptr)
{
	SFFloat comp[3];
	SFFloat delta;
	u32 i;
	M4Err e;

	e = Q_DecCoordOnUnitSphere(codec, bs, NbBits, 2, comp);
	if (e) return e;

	// normalize to avoid round-off problems
	delta=0;
	for (i=0; i<3; i++) delta += comp[i]*comp[i];
	delta = (SFFloat) sqrt(delta);

	if(delta > 0) {
		for (i=0; i<3; i++) comp[i] /= delta;
	}
	((SFVec3f *)field_ptr)->x = comp[0];
	((SFVec3f *)field_ptr)->y = comp[1];
	((SFVec3f *)field_ptr)->z = comp[2];
	return M4OK;
}


M4Err BD_UnquantizeField(BifsDecoder *codec, BitStream *bs, SFNode *node, FieldInfo *field)
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
		NbBits = BD_GetCoordBits(codec);
		/*QP14 is always on, not having NbBits set means the coord field is set after the index field, hence not decodable*/
		if (!NbBits) return M4NonCompliantBitStream;
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
		e = Q_DecFloat(codec, bs, field->fieldType, BMin, BMax, NbBits, field->far_ptr);
		break;
	//SFInt types
	case QC_LINEAR_SCALAR:
	case QC_COORD_INDEX:
		e = Q_DecInt(codec, bs, QType, (SFInt32) b_min, NbBits, field->far_ptr);
		break;
	//normalized fields (normals and vectors)
	case QC_NORMALS:
		//normal quant is only for SFVec3F
		if (field->fieldType != FT_SFVec3f) return M4InvalidNode;
		e = Q_DecNormal(codec, bs, NbBits, field->far_ptr);
		break;
	case QC_ROTATION:
		//normal quant is only for SFRotation
		if (field->fieldType != FT_SFRotation) return M4InvalidNode;
		e = Q_DecRotation(codec, bs, NbBits, field->far_ptr);
		break;
	default:
		return M4BadParam;
	}
	if (e) return e;
	return M4OK;
}

