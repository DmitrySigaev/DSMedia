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

#ifndef M4_DEF_QuantizationParameter
M4Err BD_DecPredMFField(LPBIFSDEC codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	return M4NotSupported;
}

#else

u32 BD_GetCoordBits(BifsDecoder *codec);

typedef struct
{
	s32 comp_min[3];
	s32 previous_val[3];
	s32 current_val[3];
	s32 m_delta[3];

	u32 intra_mode, intra_inter, compNbBits, num_bounds, num_comp, num_fields, QNbBits;
	u8 QType;
	Bool use_default;
	SFVec3f BMin, BMax;

	s32 direction, orientation, inverse;
	
	u32 cur_field;

	AAModel *models[3];
	AAModel *dir_model;
	AADecoder *dec;
} PredMF;


void PMF_ResetModels(PredMF *pmf)
{
	u32 i;
	for (i=0; i<pmf->num_bounds; i++) {
		InitAAModel(pmf->models[i], pmf->compNbBits);
	}
	InitAAModel(pmf->dir_model, 1);
}


Float PMF_UnquantizeFloat(s32 vq, Float BMin, Float BMax, u32 NbBits, Bool unit_vector)
{
	Float scale = 0;
	Float width = BMax - BMin;
	if (unit_vector) NbBits -= 1;
	if (width > 1e-8f) {
		if (NbBits) {
			scale = width / (Float) ( (1<<NbBits) - 1);
		} else {
			scale = width/2;
		}
	}
    return BMin + scale * vq;
}

M4Err PMF_UnquantizeNormal(PredMF *pmf, FieldInfo *field)
{
	void *slot;
	Float comp[3];
	Float tang[2];
 	u32 i;
	Float delta=1;
    for (i=0; i<2; i++) {
		Float v = PMF_UnquantizeFloat(pmf->current_val[i] - (1<< (pmf->QNbBits -1) ), 0.0 , 1.0, pmf->QNbBits, 1);
		tang[i]= (Float) tan(QP_PI * v * 4);
		delta += tang[i] * tang[i];
	}
    delta = (Float) (pmf->direction / sqrt(delta) );

    comp[(pmf->orientation) % 3] = delta;
    for (i=0; i<2; i++) 
		comp[(pmf->orientation + i+1)%3] = tang[i] * delta;

	VRML_MF_GetItem(field->far_ptr, field->fieldType, &slot, pmf->cur_field);
	((SFVec3f *)slot)->x = comp[0];
	((SFVec3f *)slot)->y = comp[1];
	((SFVec3f *)slot)->z = comp[2];
	return M4OK;
}

M4Err PMF_UnquantizeRotation(PredMF *pmf, FieldInfo *field)
{
	u32 i;
	void *slot;
	Float comp[4];
	Float tang[3];
	Float sine, delta = 1;

	for (i=0; i<3; i++) {
		Float v = PMF_UnquantizeFloat(pmf->current_val[i] - (1<<(pmf->QNbBits - 1)), 0.0, 1.0, pmf->QNbBits, 1);

		tang[i] = (Float) tan(QP_PI * v / 4);
		delta += tang[i] * tang[i];
	}
    delta = (Float) (pmf->direction / sqrt(delta) );

    comp[(pmf->orientation)%4] = delta;
    for (i=0; i<3; i++) 
		comp[(pmf->orientation + i+1)%4] = tang[i] * delta;
  

	VRML_MF_GetItem(field->far_ptr, field->fieldType, &slot, pmf->cur_field);
	delta = 2 * (Float) acos(comp[0]);
	sine = (Float) sin(delta / 2);
	if (sine != 0.0){
		for(i=1; i<4; i++)
			comp[i] /= sine;

		((SFRotation *)slot)->xAxis = comp[1];
		((SFRotation *)slot)->yAxis = comp[2];
		((SFRotation *)slot)->zAxis = comp[3];
	} else {
		((SFRotation *)slot)->xAxis = 1;
		((SFRotation *)slot)->yAxis = 0;
		((SFRotation *)slot)->zAxis = 0;
	}
	((SFRotation *)slot)->angle = delta;
	return M4OK;
}

M4Err PMF_Unquantize(PredMF *pmf, FieldInfo *field)
{
	void *slot;
	if (pmf->QType == QC_NORMALS) {
		return PMF_UnquantizeNormal(pmf, field);
	}
	if (pmf->QType == QC_ROTATION) {
		return PMF_UnquantizeRotation(pmf, field);
	}
	/*regular*/
	VRML_MF_GetItem(field->far_ptr, field->fieldType, &slot, pmf->cur_field);
	switch (field->fieldType) {
	case FT_MFVec3f:
		((SFVec3f *) slot)->x = PMF_UnquantizeFloat(pmf->current_val[0], pmf->BMin.x, pmf->BMax.x, pmf->QNbBits, 0);
		((SFVec3f *) slot)->y = PMF_UnquantizeFloat(pmf->current_val[1], pmf->BMin.y, pmf->BMax.y, pmf->QNbBits, 0);
		((SFVec3f *) slot)->z = PMF_UnquantizeFloat(pmf->current_val[2], pmf->BMin.z, pmf->BMax.z, pmf->QNbBits, 0);
		break;
	case FT_MFVec2f:
		((SFVec2f *) slot)->x = PMF_UnquantizeFloat(pmf->current_val[0], pmf->BMin.x, pmf->BMax.x, pmf->QNbBits, 0);
		((SFVec2f *) slot)->y = PMF_UnquantizeFloat(pmf->current_val[1], pmf->BMin.y, pmf->BMax.y, pmf->QNbBits, 0);
		break;
	case FT_MFFloat:
		*((SFFloat *) slot) = PMF_UnquantizeFloat(pmf->current_val[0], pmf->BMin.x, pmf->BMax.x, pmf->QNbBits, 0);
		break;
	case FT_MFColor:
		((SFColor *) slot)->red = PMF_UnquantizeFloat(pmf->current_val[0], pmf->BMin.x, pmf->BMax.x, pmf->QNbBits, 0);
		((SFColor *) slot)->green = PMF_UnquantizeFloat(pmf->current_val[1], pmf->BMin.y, pmf->BMax.y, pmf->QNbBits, 0);
		((SFColor *) slot)->blue = PMF_UnquantizeFloat(pmf->current_val[2], pmf->BMin.z, pmf->BMax.z, pmf->QNbBits, 0);
		break;
	case FT_MFInt32:
		switch (pmf->QType) {
		case QC_LINEAR_SCALAR:
		case QC_COORD_INDEX:
			*((SFInt32 *) slot) = pmf->current_val[0] + (s32) pmf->BMin.x;
			break;
		}
		break;
	}
	return M4OK;
}


M4Err PMF_ParsePValue(PredMF *pmf, BitStream *bs, FieldInfo *field)
{
	u32 i, numModel;
	s32 prev_dir = 0;
	switch (pmf->QType) {
	case QC_NORMALS:
		prev_dir = pmf->direction;
		pmf->direction = AADec_Decode(pmf->dec, pmf->dir_model);
		break;
	}
	/*decode (one model per component)*/
	numModel = 0;
	for (i=0; i<pmf->num_comp; i++) {
		pmf->previous_val[i]= pmf->current_val[i];
		pmf->current_val[i] =  AADec_Decode(pmf->dec, pmf->models[numModel]) + pmf->comp_min[numModel];
		numModel += (pmf->num_bounds==1) ? 0 : 1;
	}

	/*compensate values*/
	switch (pmf->QType) {
	case QC_NORMALS:
	case QC_ROTATION:
		/*NOT TESTED*/
		{
			s32 temp_val[3];
			s32 diff_dir = prev_dir * (pmf->direction ? -1 : 1);
			s32 inv=1;
			s32 diff_ori = 0;
			s32 shift = 1 << (pmf->QNbBits - 1);

			for (i=0; i<3; i++) {
				pmf->previous_val[i] -= shift;
				pmf->current_val[i] -= shift;
			}
			for (i=0; i< pmf->num_comp; i++) {
				temp_val[i] = pmf->previous_val[i] + pmf->current_val[i];
				if ( abs(temp_val[i]) > shift - 1) {
					diff_ori = i+1;
					inv = ( temp_val[i] > 0) ? 1 : -1;
					break;
				}
			}
			if (diff_ori != 0) {
				s32 k=0;
				for (i=0; i< pmf->num_comp - diff_ori; i++) {
					k = (i + diff_ori) % pmf->num_comp;
					temp_val[i] = inv * ( pmf->previous_val[i] + pmf->current_val[i]);
				}
				k = diff_ori - 1;
				temp_val[pmf->num_comp - diff_ori] = inv * 2 * (shift - 1) - (pmf->previous_val[k] + pmf->current_val[k]) ; 
				for (i = pmf->num_comp - diff_ori + 1; i<pmf->num_comp; i++) {
					k = (i+diff_ori-1) % pmf->num_comp;
					temp_val[i] = inv * (pmf->previous_val[k] + pmf->current_val[k]);
				}
			}
			pmf->orientation = (pmf->orientation + diff_ori) % (pmf->num_comp + 1);
			pmf->direction = diff_dir * inv;
			for (i=0; i< pmf->num_comp; i++) 
				pmf->current_val[i]= temp_val[i] + shift;
		}
		break;
	default:
		for (i=0; i< pmf->num_comp; i++)
			pmf->current_val[i] += pmf->previous_val[i];
	}
	/*unquantize*/
	return PMF_Unquantize(pmf, field);
}

M4Err PMF_ParseIValue(PredMF *pmf, BitStream *bs, FieldInfo *field)
{
	u32 i;
	switch (pmf->QType) {
	case QC_NORMALS:
		i = BS_ReadInt(bs, 1);
		pmf->direction = i ? -1 : 1;
	case QC_ROTATION:
		pmf->orientation = BS_ReadInt(bs, 2);
		break;
	}
	/*read all vals*/
	for (i=0; i<pmf->num_comp; i++) {
		pmf->current_val[i] = BS_ReadInt(bs, pmf->QNbBits);
	}
	/*reset after each I*/
	if (pmf->cur_field + 1<pmf->num_fields) AADec_Reset(pmf->dec);

	return PMF_Unquantize(pmf, field);
}

/*bit access shall be done through the AA decoder since bits may be cached there*/
M4Err PMF_UpdateArrayQP(PredMF *pmf, BitStream *bs)
{
	u32 flag, i;
	switch (pmf->intra_mode) {
	case 1:
		flag = BS_ReadInt(bs, 5);
		pmf->intra_inter = BS_ReadInt(bs, flag);
	case 2:
	case 0:
		flag = BS_ReadInt(bs, 1);
		if (flag) {
			pmf->compNbBits = BS_ReadInt(bs, 5);
		}
		flag = BS_ReadInt(bs, 1);
		if (flag) {
			for (i=0; i<pmf->num_bounds; i++) {
				flag = BS_ReadInt(bs, pmf->QNbBits + 1);
				pmf->comp_min[i] = flag - (1<<pmf->QNbBits);
			}
		}
		break;
	}
	/*reset all models when new settings are recieved*/
	PMF_ResetModels(pmf);
	return M4OK;
}



M4Err BD_DecPredMFField(BifsDecoder *codec, BitStream *bs, SFNode *node, FieldInfo *field)
{
	M4Err e;
	Bool HasQ;
	u8 AType;
	Float b_min, b_max;
	u32 i, flag;
	PredMF pmf;
	
	memset(&pmf, 0, sizeof(PredMF));
	
	HasQ = Node_GetAQInfo(node, field->fieldIndex, &pmf.QType, &AType, &b_min, &b_max, &pmf.QNbBits);
	if (!HasQ || !pmf.QType) return M4FieldNotQuantized;

	/*get NbBits for QP14 (QC_COORD_INDEX)*/
	if (pmf.QType == QC_COORD_INDEX) 
		pmf.QNbBits = BD_GetCoordBits(codec);

	pmf.BMin.x = pmf.BMin.y = pmf.BMin.z = b_min;
	pmf.BMax.x = pmf.BMax.y = pmf.BMax.z = b_max;

	/*check is the QP is on and retrieves the bounds*/
	if (!Q_IsTypeOn(codec->ActiveQP, pmf.QType, &pmf.QNbBits, &pmf.BMin, &pmf.BMax)) return M4FieldNotQuantized;
	
	switch (field->fieldType) {
	case FT_MFColor:
	case FT_MFVec3f:
		if (pmf.QType==QC_NORMALS) {
			pmf.num_comp = 2;
			break;
		}
	case FT_MFRotation:
		pmf.num_comp = 3;
		break;
	case FT_MFVec2f:
		pmf.num_comp = 2;
		break;
	case FT_MFFloat:
	case FT_MFInt32:
		pmf.num_comp = 1;
		break;
	default:
		return M4NonCompliantBitStream;
	}


	/*parse array header*/
	flag = BS_ReadInt(bs, 5);
	pmf.num_fields = BS_ReadInt(bs, flag);
	pmf.intra_mode = BS_ReadInt(bs, 2);
	switch (pmf.intra_mode) {
	case 1:
		flag = BS_ReadInt(bs, 5);
		pmf.intra_inter = BS_ReadInt(bs, flag);
		/*no break*/
	case 2:
	case 0:
		pmf.compNbBits = BS_ReadInt(bs, 5);
		if (pmf.QType==1) pmf.num_bounds = 3;
		else if (pmf.QType==2) pmf.num_bounds = 2;
		else pmf.num_bounds = 1;
		for (i=0; i<pmf.num_bounds; i++) {
			flag = BS_ReadInt(bs, pmf.QNbBits + 1);
			pmf.comp_min[i] = flag - (1<<pmf.QNbBits);
		}
		break;
	case 3:
		break;
	}


	pmf.dec = NewAADecoder(bs);
	pmf.models[0] = NewAAModel();
	pmf.models[1] = NewAAModel();
	pmf.models[2] = NewAAModel();
	pmf.dir_model = NewAAModel();

	PMF_ResetModels(&pmf);

	VRML_MF_Alloc(field->far_ptr, field->fieldType, pmf.num_fields);
	pmf.cur_field = 0;
	/*parse initial I*/
	e = PMF_ParseIValue(&pmf, bs, field);
	if (e) return e;

	for (pmf.cur_field=1; pmf.cur_field<pmf.num_fields; pmf.cur_field++) {
		switch (pmf.intra_mode) {
		case 0:
			e = PMF_ParsePValue(&pmf, bs, field);
			break;

		/*NOT TESTED*/
		case 1:
			if (!(pmf.cur_field % pmf.intra_inter)) {
				/*resync bitstream*/
				AADec_Resync(pmf.dec);
				flag = BS_ReadInt(bs, 1);
				/*update settings ?*/
				if (flag) {
					e = PMF_UpdateArrayQP(&pmf, bs);
					if (e) goto err_exit;
				}
				e = PMF_ParseIValue(&pmf, bs, field);
			} else {
				e = PMF_ParsePValue(&pmf, bs, field);
			}
			break;

		/*NOT TESTED*/
		case 2:
			/*is intra ? - WARNING: this is from the arithmetic context !!*/
			flag = AADec_GetBitFlag(pmf.dec);
			if (flag) {
				/*resync bitstream*/
				AADec_ResyncBitFlag(pmf.dec);
				flag = BS_ReadInt(bs, 1);
				/*update settings ?*/
				if (flag) {
					e = PMF_UpdateArrayQP(&pmf, bs);
					if (e) goto err_exit;
				}
				e = PMF_ParseIValue(&pmf, bs, field);
			} else {
				e = PMF_ParsePValue(&pmf, bs, field);
				AADec_FlushChoice(pmf.dec);
			}
			break;
		}
		if (e) goto err_exit;
	}


	if (pmf.intra_mode==2) {
		AADec_ResyncBitFlag(pmf.dec);
	} else {
		AADec_Resync(pmf.dec);
	}

err_exit:
	DeleteAAModel(pmf.models[0]);
	DeleteAAModel(pmf.models[1]);
	DeleteAAModel(pmf.models[2]);
	DeleteAAModel(pmf.dir_model);
	DeleteAADecoder(pmf.dec);
	return e;
}


#endif

