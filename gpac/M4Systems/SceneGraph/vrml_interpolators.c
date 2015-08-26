/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Graph sub-project
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


#include <gpac/intern/m4_scenegraph_dev.h>
#include <gpac/m4_mpeg4_nodes.h>
#include <gpac/m4_x3d_nodes.h>

#include <math.h>


Float Interpolate(Float keyValue1, Float keyValue2, Float fraction)
{
	return (keyValue2 - keyValue1) * fraction + keyValue1;
}

Float GetInterpolateFraction(Float key1, Float key2, Float fraction)
{
	Float keyDiff = key2 - key1;
	assert((fraction >= key1) && (fraction <= key2));
	if (fabs(keyDiff) < M4_EPSILON_FLOAT) return 0;
	return ((fraction - key1) / keyDiff);
}

void CI2D_SetFraction(SFNode *n)
{
	Float frac;
	u32 numElemPerKey, i, j;
	M_CoordinateInterpolator2D *_this = (M_CoordinateInterpolator2D *) n;

	if (! _this->key.count) return;
	if (_this->keyValue.count % _this->key.count) return;
	
	numElemPerKey = _this->keyValue.count / _this->key.count;
	//set size
	if (_this->value_changed.count != numElemPerKey)
		VRML_MF_Alloc(&_this->value_changed, FT_MFVec2f, numElemPerKey);


	if (_this->set_fraction < _this->key.vals[0]) {
		for (i=0; i<numElemPerKey; i++)
			_this->value_changed.vals[i] = _this->keyValue.vals[i];
	} else if (_this->set_fraction > _this->key.vals[_this->key.count - 1]) {
		for (i=0; i<numElemPerKey; i++)
			_this->value_changed.vals[i] = _this->keyValue.vals[(_this->keyValue.count) - numElemPerKey + i];
	} else {
		for (j = 1; j < _this->key.count; j++) {
			// Find the key values the fraction lies between
			if ( _this->set_fraction < _this->key.vals[j-1]) continue;
			if (_this->set_fraction >= _this->key.vals[j]) continue;

			frac = GetInterpolateFraction(_this->key.vals[j-1], _this->key.vals[j], _this->set_fraction);
			for (i=0; i<numElemPerKey; i++) {
				_this->value_changed.vals[i].x = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].x,
															_this->keyValue.vals[(j)*numElemPerKey + i].x, 
															frac);
				_this->value_changed.vals[i].y = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].y,
															_this->keyValue.vals[(j)*numElemPerKey + i].y,
															frac);
			}
			break;
		}
	}
	//invalidate
	Node_OnEventOutSTR(n, "value_changed");
}

Bool CI_SetFraction(Float fraction, MFVec3f *vals, MFFloat *key, MFVec3f *keyValue)
{
	Float frac;
	u32 numElemPerKey, i, j;

	if (! key->count) return 0;
	if (keyValue->count % key->count) return 0;
	
	numElemPerKey = keyValue->count / key->count;

	if (vals->count != numElemPerKey) VRML_MF_Alloc(vals, FT_MFVec3f, numElemPerKey);

	if (fraction < key->vals[0]) {
		for (i=0; i<numElemPerKey; i++)
			vals->vals[i] = keyValue->vals[i];
	} else if (fraction > key->vals[key->count - 1]) {
		for (i=0; i<numElemPerKey; i++)
			vals->vals[i] = keyValue->vals[(keyValue->count) - numElemPerKey + i];
	} else {
		for (j = 1; j < key->count; j++) {
			// Find the key values the fraction lies between
			if (fraction < key->vals[j-1]) continue;
			if (fraction >= key->vals[j]) continue;

			frac = GetInterpolateFraction(key->vals[j-1], key->vals[j], fraction);
			for (i=0; i<numElemPerKey; i++) {
				vals->vals[i].x = Interpolate(keyValue->vals[(j-1)*numElemPerKey + i].x,
															keyValue->vals[(j)*numElemPerKey + i].x,
															frac);
				vals->vals[i].y = Interpolate(keyValue->vals[(j-1)*numElemPerKey + i].y,
															keyValue->vals[(j)*numElemPerKey + i].y,
															frac);
				vals->vals[i].z = Interpolate(keyValue->vals[(j-1)*numElemPerKey + i].z,
															keyValue->vals[(j)*numElemPerKey + i].z, 
															frac);
			}
			break;
		}
	}
	return 1;
}


void CoordInt_SetFraction(SFNode *n)
{
	M_CoordinateInterpolator *_this = (M_CoordinateInterpolator *) n;

	if (CI_SetFraction(_this->set_fraction, &_this->value_changed, &_this->key, &_this->keyValue)) 
		Node_OnEventOutSTR(n, "value_changed");
}


void NormInt_SetFraction(SFNode *n)
{
	u32 i;
	M_NormalInterpolator *_this = (M_NormalInterpolator *) n;

	if (!CI_SetFraction(_this->set_fraction, &_this->value_changed, &_this->key, &_this->keyValue)) return;
	/*renorm*/
	for (i=0; i<_this->value_changed.count; i++) {
		SFVec3f *v = &_this->value_changed.vals[i];
		Float res = (Float) sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
		v->x /= res;
		v->y /= res;
		v->z /= res;
	}
	Node_OnEventOutSTR(n, "value_changed");
}

void ColorInt_SetFraction(SFNode *node)
{
	u32 i;
	Float frac;
	M_ColorInterpolator *_this = (M_ColorInterpolator *)node;


	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed.red = Interpolate(_this->keyValue.vals[i-1].red, 
													_this->keyValue.vals[i].red, 
													frac);
			_this->value_changed.green = Interpolate(_this->keyValue.vals[i-1].green,
													_this->keyValue.vals[i].green, 
													frac);
			_this->value_changed.blue = Interpolate(_this->keyValue.vals[i-1].blue,
													_this->keyValue.vals[i].blue,
													frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}


void PosInt2D_SetFraction(SFNode *node)
{
	M_PositionInterpolator2D *_this = (M_PositionInterpolator2D *)node;
	u32 i;
	Float frac;

	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed.x = Interpolate(_this->keyValue.vals[i-1].x, _this->keyValue.vals[i].x, frac);
			_this->value_changed.y = Interpolate(_this->keyValue.vals[i-1].y, _this->keyValue.vals[i].y, frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}

void PosInt_SetFraction(SFNode *node)
{
	u32 i;
	Float frac;
	M_PositionInterpolator *_this = (M_PositionInterpolator *)node;

	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed.x = Interpolate(_this->keyValue.vals[i-1].x, _this->keyValue.vals[i].x, frac);
			_this->value_changed.y = Interpolate(_this->keyValue.vals[i-1].y, _this->keyValue.vals[i].y, frac);
			_this->value_changed.z = Interpolate(_this->keyValue.vals[i-1].z, _this->keyValue.vals[i].z, frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}

void ScalarInt_SetFraction(SFNode *node)
{
	M_ScalarInterpolator *_this = (M_ScalarInterpolator *)node;
	u32 i;
	Float frac;

	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed = Interpolate(_this->keyValue.vals[i-1], _this->keyValue.vals[i], frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}


#ifndef M_PI
#define M_PI					3.14159265358979323846f
#endif

/*taken from freeWRL*/
SFRotation Rotation_Interpolate(SFRotation kv1, SFRotation kv2, Float fraction)
{
	SFRotation res;
	Float newa, olda;
	Bool stzero = ( (Float) fabs(kv1.angle) < M4_EPSILON_FLOAT) ? 1 : 0;
	Bool endzero = ( (Float) fabs(kv2.angle) < M4_EPSILON_FLOAT) ? 1 : 0;
	Float testa = kv1.xAxis*kv2.xAxis + kv1.yAxis*kv2.yAxis + kv1.yAxis*kv2.yAxis;

	if (testa>= 0.0) {
		res.xAxis = kv1.xAxis + fraction*(kv2.xAxis-kv1.xAxis);
		res.yAxis = kv1.yAxis + fraction*(kv2.yAxis-kv1.yAxis);
		res.zAxis = kv1.zAxis + fraction*(kv2.zAxis-kv1.zAxis);
		newa = kv2.angle;
	} else {
		res.xAxis = kv1.xAxis + fraction*(-kv2.xAxis-kv1.xAxis);
		res.yAxis = kv1.yAxis + fraction*(-kv2.yAxis-kv1.yAxis);
		res.zAxis = kv1.zAxis + fraction*(-kv2.zAxis-kv1.zAxis);
		newa = -kv2.angle;
	}
	olda = kv1.angle;
	testa = newa - olda;
	/* make it so we smoothly transition */
	if (fabs(testa) > M_PI) {
		if (fabs(testa) > (M_PI*2)) {
			if (testa>0.0) {
				olda += M_PI*4;
			} else {
				newa += M_PI*4; 
			}
		} else {
			if (testa>0.0) {
				olda += M_PI*2;
			} else { 
				newa += M_PI*2; 
			}
		}
	}

	if (stzero || endzero) {
		res.xAxis = stzero ? kv2.xAxis : kv1.xAxis;
		res.yAxis = stzero ? kv2.yAxis : kv1.yAxis;
		res.zAxis = stzero ? kv2.zAxis : kv1.zAxis;
	}
	/* now that we have angles straight (hah!) bounds check result */
	res.angle = olda + fraction*(newa - olda);
	if (res.angle > M_PI*2) { 
		res.angle -= M_PI*2;
	} else if (res.angle <M_PI*2) {
		res.angle += M_PI*2;
	}
	return res;
}

void OrientInt_SetFraction(SFNode *node)
{
	u32 i;
	Float frac;
	M_OrientationInterpolator *_this = (M_OrientationInterpolator *)node;

	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed = Rotation_Interpolate(_this->keyValue.vals[i-1], _this->keyValue.vals[i], frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}

void CI4D_SetFraction(SFNode *n)
{
	Float frac;
	u32 numElemPerKey, i, j;
	M_CoordinateInterpolator4D *_this = (M_CoordinateInterpolator4D *) n;

	if (! _this->key.count) return;
	if (_this->keyValue.count % _this->key.count) return;
	
	numElemPerKey = _this->keyValue.count / _this->key.count;
	//set size
	if (_this->value_changed.count != numElemPerKey)
		VRML_MF_Alloc(&_this->value_changed, FT_MFVec4f, numElemPerKey);


	if (_this->set_fraction < _this->key.vals[0]) {
		for (i=0; i<numElemPerKey; i++)
			_this->value_changed.vals[i] = _this->keyValue.vals[i];
	} else if (_this->set_fraction > _this->key.vals[_this->key.count - 1]) {
		for (i=0; i<numElemPerKey; i++)
			_this->value_changed.vals[i] = _this->keyValue.vals[(_this->keyValue.count) - numElemPerKey + i];
	} else {
		for (j = 1; j < _this->key.count; j++) {
			// Find the key values the fraction lies between
			if ( _this->set_fraction < _this->key.vals[j-1]) continue;
			if (_this->set_fraction >= _this->key.vals[j]) continue;

			frac = GetInterpolateFraction(_this->key.vals[j-1], _this->key.vals[j], _this->set_fraction);
			for (i=0; i<numElemPerKey; i++) {
				_this->value_changed.vals[i].x = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].x,
															_this->keyValue.vals[(j)*numElemPerKey + i].x, 
															frac);
				_this->value_changed.vals[i].y = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].y,
															_this->keyValue.vals[(j)*numElemPerKey + i].y,
															frac);
				_this->value_changed.vals[i].z = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].z,
															_this->keyValue.vals[(j)*numElemPerKey + i].z,
															frac);
				_this->value_changed.vals[i].q = Interpolate(_this->keyValue.vals[(j-1)*numElemPerKey + i].q,
															_this->keyValue.vals[(j)*numElemPerKey + i].q,
															frac);
			}
			break;
		}
	}
	//invalidate
	Node_OnEventOutSTR(n, "value_changed");
}

void PI4D_SetFraction(SFNode *node)
{
	u32 i;
	Float frac;
	M_PositionInterpolator4D *_this = (M_PositionInterpolator4D *)node;

	if (! _this->key.count) return;
	if (_this->keyValue.count != _this->key.count) return;
	
	// The given fraction is less than the specified range
	if (_this->set_fraction < _this->key.vals[0]) {
		_this->value_changed = _this->keyValue.vals[0];
	} else if (_this->set_fraction >= _this->key.vals[_this->key.count-1]) {
		_this->value_changed = _this->keyValue.vals[_this->keyValue.count-1];
	} else {
		for (i=1; i<_this->key.count; i++) {
			// Find the key values the fraction lies between
			if (_this->set_fraction < _this->key.vals[i-1]) continue;
			if (_this->set_fraction >= _this->key.vals[i]) continue;

			frac = GetInterpolateFraction(_this->key.vals[i-1], _this->key.vals[i], _this->set_fraction);
			_this->value_changed.x = Interpolate(_this->keyValue.vals[i-1].x, _this->keyValue.vals[i].x, frac);
			_this->value_changed.y = Interpolate(_this->keyValue.vals[i-1].y, _this->keyValue.vals[i].y, frac);
			_this->value_changed.z = Interpolate(_this->keyValue.vals[i-1].z, _this->keyValue.vals[i].z, frac);
			_this->value_changed.q = Interpolate(_this->keyValue.vals[i-1].q, _this->keyValue.vals[i].q, frac);
			break;
		}
	}
	Node_OnEventOutSTR(node, "value_changed");
}



static void BooleanFilter_setValue(SFNode *n)
{
	X_BooleanFilter *bf = (X_BooleanFilter *)n;
	if (!bf->set_boolean) {
		bf->inputFalse = 1;
		Node_OnEventOutSTR(n, "inputFalse");
	}
	if (bf->set_boolean) {
		bf->inputTrue = 1;
		Node_OnEventOutSTR(n, "inputTrue");
	}
	bf->inputNegate = bf->set_boolean ? 0 : 1;
	Node_OnEventOutSTR(n, "inputNegate");
}

void InitBooleanFilter(SFNode *n)
{
	X_BooleanFilter *bf = (X_BooleanFilter *)n;
	bf->on_set_boolean = BooleanFilter_setValue;
}

static void BooleanSequencer_setFraction(SFNode *n)
{
	u32 i;
	X_BooleanSequencer *bs = (X_BooleanSequencer*)n;
	if (! bs->key.count) return;
	if (bs->keyValue.count != bs->key.count) return;
	
	if (bs->set_fraction < bs->key.vals[0]) {
		bs->value_changed = bs->keyValue.vals[0];
	} else if (bs->set_fraction >= bs->key.vals[bs->key.count-1]) {
		bs->value_changed = bs->keyValue.vals[bs->keyValue.count-1];
	} else {
		for (i=1; i<bs->key.count; i++) {
			if (bs->set_fraction < bs->key.vals[i-1]) continue;
			if (bs->set_fraction >= bs->key.vals[i]) continue;
			bs->value_changed = bs->keyValue.vals[i-1];
			break;
		}
	}
	Node_OnEventOutSTR(n, "value_changed");
}

static void BooleanSequencer_setNext(SFNode *n)
{
	s32 *prev_val, val;
	X_BooleanSequencer *bs = (X_BooleanSequencer*)n;
	if (!bs->next) return;

	prev_val = (s32 *)n->sgprivate->privateStack;
	val = (*prev_val + 1) % bs->keyValue.count;
	*prev_val = val;
	bs->value_changed = bs->keyValue.vals[val];
	Node_OnEventOutSTR(n, "value_changed");
}

static void BooleanSequencer_setPrevious(SFNode *n)
{
	s32 *prev_val, val;
	X_BooleanSequencer *bs = (X_BooleanSequencer*)n;
	if (!bs->previous) return;

	prev_val = (s32 *)n->sgprivate->privateStack;
	val = (*prev_val - 1);
	if (val<0) val += bs->keyValue.count;
	val %= bs->keyValue.count;
	*prev_val = val;
	bs->value_changed = bs->keyValue.vals[val];
	Node_OnEventOutSTR(n, "value_changed");
}
static void DestroyBooleanSequencer(SFNode *n)
{
	s32 *st = Node_GetPrivate(n);
	free(st);
}
void InitBooleanSequencer(SFNode *n)
{
	X_BooleanSequencer *bs = (X_BooleanSequencer*)n;
	bs->on_next = BooleanSequencer_setNext;
	bs->on_previous = BooleanSequencer_setPrevious;
	bs->on_set_fraction = BooleanSequencer_setFraction;
	n->sgprivate->privateStack = malloc(sizeof(s32));
	*(s32 *)n->sgprivate->privateStack = 0;
	n->sgprivate->PreDestroyNode = DestroyBooleanSequencer;
}

static void BooleanToggle_setValue(SFNode *n)
{
	X_BooleanToggle *bt = (X_BooleanToggle *)n;
	if (bt->set_boolean) {
		bt->toggle = !bt->toggle;
		Node_OnEventOutSTR(n, "toggle");
	}
}
void InitBooleanToggle(SFNode *n)
{
	X_BooleanToggle *bt = (X_BooleanToggle *)n;
	bt->on_set_boolean = BooleanToggle_setValue;
}

static void BooleanTrigger_setTime(SFNode *n)
{
	X_BooleanTrigger *bt = (X_BooleanTrigger *)n;
	bt->triggerTrue = 1;
	Node_OnEventOutSTR(n, "triggerTrue");
}
void InitBooleanTrigger(SFNode *n)
{
	X_BooleanTrigger *bt = (X_BooleanTrigger *)n;
	bt->on_set_triggerTime = BooleanTrigger_setTime;
}

static void IntegerSequencer_setFraction(SFNode *n)
{
	u32 i;
	X_IntegerSequencer *is = (X_IntegerSequencer *)n;
	if (! is->key.count) return;
	if (is->keyValue.count != is->key.count) return;
	
	if (is->set_fraction < is->key.vals[0]) {
		is->value_changed = is->keyValue.vals[0];
	} else if (is->set_fraction >= is->key.vals[is->key.count-1]) {
		is->value_changed = is->keyValue.vals[is->keyValue.count-1];
	} else {
		for (i=1; i<is->key.count; i++) {
			if (is->set_fraction < is->key.vals[i-1]) continue;
			if (is->set_fraction >= is->key.vals[i]) continue;
			is->value_changed = is->keyValue.vals[i-1];
			break;
		}
	}
	Node_OnEventOutSTR(n, "value_changed");
}

static void IntegerSequencer_setNext(SFNode *n)
{
	s32 *prev_val, val;
	X_IntegerSequencer *is = (X_IntegerSequencer*)n;
	if (!is->next) return;

	prev_val = (s32 *)n->sgprivate->privateStack;
	val = (*prev_val + 1) % is->keyValue.count;
	*prev_val = val;
	is->value_changed = is->keyValue.vals[val];
	Node_OnEventOutSTR(n, "value_changed");
}

static void IntegerSequencer_setPrevious(SFNode *n)
{
	s32 *prev_val, val;
	X_IntegerSequencer *is = (X_IntegerSequencer *)n;
	if (!is->previous) return;

	prev_val = (s32 *)n->sgprivate->privateStack;
	val = (*prev_val - 1);
	if (val<0) val += is->keyValue.count;
	val %= is->keyValue.count;
	*prev_val = val;
	is->value_changed = is->keyValue.vals[val];
	Node_OnEventOutSTR(n, "value_changed");
}
static void DestroyIntegerSequencer(SFNode *n)
{
	s32 *st = Node_GetPrivate(n);
	free(st);
}
void InitIntegerSequencer(SFNode *n)
{
	X_IntegerSequencer *bs = (X_IntegerSequencer *)n;
	bs->on_next = IntegerSequencer_setNext;
	bs->on_previous = IntegerSequencer_setPrevious;
	bs->on_set_fraction = IntegerSequencer_setFraction;
	n->sgprivate->privateStack = malloc(sizeof(s32));
	*(s32 *)n->sgprivate->privateStack = 0;
	n->sgprivate->PreDestroyNode = DestroyIntegerSequencer;
}

static void IntegerTrigger_setTrigger(SFNode *n)
{
	X_IntegerTrigger *it = (X_IntegerTrigger *)n;
	if (it->set_boolean) {
		it->triggerValue = it->integerKey;
		Node_OnEventOutSTR(n, "triggerValue");
	}
}
void InitIntegerTrigger(SFNode *n)
{
	X_IntegerTrigger *it = (X_IntegerTrigger *)n;
	it->on_set_boolean = IntegerTrigger_setTrigger;
}

static void TimeTrigger_setTrigger(SFNode *n)
{
	X_TimeTrigger *tt = (X_TimeTrigger *)n;
	tt->triggerTime = Node_GetSceneTime(n);
	Node_OnEventOutSTR(n, "triggerTime");
}
void InitTimeTrigger(SFNode *n)
{
	X_TimeTrigger *tt = (X_TimeTrigger*)n;
	tt->on_set_boolean = TimeTrigger_setTrigger;
}

