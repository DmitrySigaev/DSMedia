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


#include <intern/m4_scenegraph_dev.h>
/*MPEG4 tags (for internal nodes)*/
#include <m4_mpeg4_nodes.h>

#include <math.h>

/*from interpolators.c*/
Float GetInterpolateFraction(Float key1, Float key2, Float fraction);
Float Interpolate(Float keyValue1, Float keyValue2, Float fraction);

enum
{
	ANIM_LINE,
	ANIM_QUADRATIC,
	ANIM_CUBIC,
	ANIM_NURBS,
	/*NOT SUPPORTED*/
	ANIM_SPLINE
};

enum
{
	ANIM_DEFAULT,
	ANIM_DISCRETE,
	ANIM_LINEAR,
	/*NOT SUPPORTED ON SPLINES*/
	ANIM_PACED,
	ANIM_VELOCITY
};


/* Bisection algorithm to find u=a*t^3+b*t^2+c*t+d */
#define BISEC_SEARCH_TO		0.001f

Float do_bisection(Float t, SFVec2f a, SFVec2f b, SFVec2f c, SFVec2f d)
{
	Float left, right, usearch, tsearch;
	left = 0;
	right = 1;
	
	do {
		usearch = (left+right)/2;
		tsearch = usearch*(c.x + usearch * (b.x + usearch * a.x)) + d.x;
		if (t<tsearch+BISEC_SEARCH_TO) right = usearch;
		else left = usearch;
	} while ((t > tsearch + BISEC_SEARCH_TO) || (t<tsearch - BISEC_SEARCH_TO));
	return usearch*(c.y + usearch * (b.y + usearch * a.y)) + d.y;
}



typedef struct
{
	Float *knots, *weights, *n, *left, *right;
	u32 nknots, nweights, npoints;
	u32 p;
	u32 type;
	Bool valid;
} anim_nurbs;

static Float cubic_knots[] = {0,0,0,0,1,1,1,1};
static Float quadratic_knots[] = {0,0,0,1,1,1};

static void anurbs_reset(anim_nurbs *nurbs)
{
	if (nurbs->n) free(nurbs->n);
	if (nurbs->left) free(nurbs->left);
	if (nurbs->right) free(nurbs->right);
	nurbs->n = nurbs->left = nurbs->right = NULL;
}

static void anurbs_init(anim_nurbs *nurbs, u32 type, u32 nCtrl, u32 nKnots, Float *knots, u32 nWeight, Float *weights)
{
	memset(nurbs, 0, sizeof(anim_nurbs));
	nurbs->type = type;
	switch (type) {
	case ANIM_CUBIC:
		nurbs->npoints = 4;
		nurbs->nknots = 8;
		nurbs->knots = cubic_knots;
		break;
	case ANIM_QUADRATIC:
		nurbs->npoints = 3;
		nurbs->nknots = 6;
		nurbs->knots = quadratic_knots;
		break;
	default:
		nurbs->npoints = nCtrl;
		nurbs->knots = knots;
		nurbs->nknots = nKnots;
		nurbs->weights = weights;
		nurbs->nweights = nWeight;
		break;
	}
	nurbs->p = nurbs->nknots - nurbs->npoints - 1;
	if ((nurbs->p<=0) || (nurbs->p >= nurbs->nknots -1) 
		|| ((nurbs->nweights>0) && (nurbs->npoints != nurbs->nweights)) ) {
		nurbs->valid = 0;
	} else {
		nurbs->valid = 1;
	}
}

static void anurbs_basis(anim_nurbs *nurbs, s32 span, Float t)
{
	u32 i, j;
	Float saved, temp;
	if (!nurbs->n) {
		nurbs->n = malloc(sizeof(Float) * (nurbs->p+1));
		nurbs->left = malloc(sizeof(Float) * (nurbs->p+1));
		nurbs->right = malloc(sizeof(Float) * (nurbs->p+1));
	}
	nurbs->n[0] = 1;

	for(i=1; i<=nurbs->p; i++) {
		nurbs->left[i] = t - nurbs->knots[span+1-i];
		nurbs->right[i] = nurbs->knots[span+i]-t;
		saved = 0;
		
		for(j=0; j<i; j++) {
			temp = nurbs->n[j] / (nurbs->right[j+1] + nurbs->left[i-j]);
			nurbs->n[j] = saved + nurbs->right[j+1] * temp;
			saved = nurbs->left[i-j] * temp;
		}
		nurbs->n[i]=saved;
	}
}

static s32 anurbs_find_span(anim_nurbs *nurbs, Float u)
{
#if 0
	s32 span;
	if (u == nurbs->knots[nurbs->npoints]) return nurbs->npoints - 1;
	for (span = (s32) nurbs->p; span < (s32) nurbs->nknots - (s32) nurbs->p; span++) {
		if (u<nurbs->knots[span]) break;
	}
	span--;
	return span;
#else
	s32 low, high, mid;	
	if (u == nurbs->knots[nurbs->npoints]) return nurbs->npoints - 1;
	low = nurbs->p; 
	high = nurbs->npoints; 
	mid = (low + high)/2;
	
	while (u < nurbs->knots[mid] || u >= nurbs->knots[mid+1]) {
		if (u < nurbs->knots[mid]) high = mid; 
		else low = mid;
		mid = (low + high)/2;
	}	
	return (mid); 

#endif
}

static SFVec3f anurbs_get_vec3f(anim_nurbs *nurbs, s32 span, SFVec3f *pts)
{
	SFVec3f res, tmp;
	Float w, wi;
	u32 i;
	tmp.x = tmp.y = tmp.z = 0;
	res = tmp;
	w=0;
	for(i=0; i<=nurbs->p;i++) {
		tmp = pts[span - nurbs->p + i];
		if (nurbs->nweights>0) {
			wi = nurbs->weights[span - nurbs->p + i];
			tmp.x *= wi; tmp.y *= wi; tmp.z *= wi;
			w += nurbs->n[i]*wi;
		}
		res.x += nurbs->n[i]*tmp.x; res.y += nurbs->n[i]*tmp.y; res.z += nurbs->n[i]*tmp.z;
	}
	if (nurbs->nweights>0) {
		if (w) {
			w = 1/w;
			res.x *= w; res.y *= w; res.z *= w;
		}
	}
	return res;
}

static SFVec2f anurbs_get_vec2f(anim_nurbs *nurbs, s32 span, SFVec2f *pts)
{
	SFVec2f res, tmp;
	Float w, wi;
	u32 i;
	tmp.x = tmp.y = 0;
	res = tmp;
	w=0;
	for(i=0; i<=nurbs->p;i++) {
		tmp = pts[span - nurbs->p + i];
		if (nurbs->nweights>0) {
			wi = nurbs->weights[span - nurbs->p + i];
			tmp.x *= wi; tmp.y *= wi;
			w += nurbs->n[i]*wi;
		}
		res.x += nurbs->n[i]*tmp.x; res.y += nurbs->n[i]*tmp.y;
	}
	if (nurbs->nweights>0) {
		if (w) {
			w = 1/w;
			res.x *= w; res.y *= w;
		}
	}
	return res;
}

static Float anurbs_get_float(anim_nurbs *nurbs, s32 span, Float *vals)
{
	Float res, tmp;
	Float w, wi;
	u32 i;
	res = tmp = 0;
	w=0;
	for(i=0; i<=nurbs->p;i++) {
		tmp = vals[span - nurbs->p + i];
		if (nurbs->nweights>0) {
			wi = nurbs->weights[span - nurbs->p + i];
			tmp *= wi;
			w += nurbs->n[i]*wi;
		}
		res += nurbs->n[i]*tmp;
	}
	if (nurbs->nweights>0) res /= w;
	return res;
}

typedef struct
{
	Bool is_dirty;
	u32 anim_type;
	/*for paced anim*/
	Float length;
	/*for spline anim*/
	SFVec2f a, b, c, d;
	/*nurbs path*/
	anim_nurbs anurbs;
} AnimatorStack;

static void Anim_Destroy(SFNode *node)
{
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);

	anurbs_reset(&stack->anurbs);
	free(stack);
}

static void Animator_Update(AnimatorStack *stack, u32 keyValueType, u32 nCtrl, MFVec2f *keySpline, u32 nWeight, Float *weights)
{
	if (stack->anim_type==ANIM_SPLINE) {
		stack->a.x = (keySpline->vals[0].x - keySpline->vals[1].x)*3+1;
		stack->a.y = (keySpline->vals[0].y - keySpline->vals[1].y)*3+1;
		stack->b.x = (keySpline->vals[1].x - 2*keySpline->vals[0].x)*3;
		stack->b.y = (keySpline->vals[1].y - 2*keySpline->vals[0].y)*3;
		stack->c.x = keySpline->vals[0].x*3;
		stack->c.y = keySpline->vals[0].y*3;
		stack->d.x = stack->d.y = 0;
	}
	anurbs_reset(&stack->anurbs);
	switch (keyValueType) {
	case ANIM_CUBIC:
		anurbs_init(&stack->anurbs, ANIM_CUBIC, 0, 0, NULL, 0, NULL);
		break;
	case ANIM_QUADRATIC:
		anurbs_init(&stack->anurbs, ANIM_QUADRATIC, 0, 0, NULL, 0, NULL);
		break;
	case ANIM_NURBS:
		anurbs_init(&stack->anurbs, ANIM_NURBS, nCtrl, keySpline->count, (Float *) &keySpline->vals[0].x, nWeight, weights);
		break;
	}
}


static Bool anim_check_frac(Float frac, SFVec2f *fromTo)
{
	if (frac<0) return 0;
	if (frac>1) return 0;
	if (fromTo->x > fromTo->y) return 0;
	/*not active*/
	if (frac<fromTo->x) return 0;
	if (frac>fromTo->y) return 0;
	return 1;
}

static void PA_Update(M_PositionAnimator *pa, AnimatorStack *stack)
{
	u32 i;
	Float dx, dy, dz;
	stack->is_dirty = 0;
	stack->anim_type = pa->keyType;
	/*if empty key and default anim switch to linear*/
	if (!pa->key.count && !stack->anim_type) stack->anim_type = ANIM_LINEAR;

	if (stack->anim_type == ANIM_PACED) {
		stack->length = 0;
		for (i=0; i<pa->keyValue.count-1; i++) {
			dx = pa->keyValue.vals[i+1].x - pa->keyValue.vals[i].x;
			dy = pa->keyValue.vals[i+1].y - pa->keyValue.vals[i].y;
			dz = pa->keyValue.vals[i+1].z - pa->keyValue.vals[i].z;
			stack->length += (Float) sqrt(dx*dx + dy*dy + dz*dz);
		}
	} 
	Animator_Update(stack, pa->keyValueType, pa->keyValue.count, &pa->keySpline, pa->weight.count, pa->weight.vals);
}
static void PA_SetFraction(SFNode *node)
{
	Float frac;
	u32 nbKeys, nbVals, i;
	Float dx, dy, dz;
	Float len, dlen, dist;
	M_PositionAnimator *pa = (M_PositionAnimator *)node;
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);

	frac = pa->set_fraction;
	if (!anim_check_frac(frac, &pa->fromTo)) return;

	if (stack->is_dirty) PA_Update(pa, stack);

	nbKeys = pa->key.count;
	nbVals = pa->keyValue.count;

	i=0;
	switch (pa->keyValueType) {
	/*linear interpolate*/
	case ANIM_LINE:
		/*compute frac and segment start index*/
		switch (stack->anim_type) {
		case ANIM_DEFAULT:
			if (nbKeys != nbVals) return;
			if (frac<pa->key.vals[0]) { i=0; frac = 0; }
			else if (frac>pa->key.vals[nbKeys-1]) { i=nbVals-2; frac=1; }
			else {
				for (i=0; i<nbKeys-1; i++) {
					if ((frac>=pa->key.vals[i]) && (frac<pa->key.vals[i+1])) break;
				}
				frac = GetInterpolateFraction(pa->key.vals[i], pa->key.vals[i+1], frac);
			}
			break;
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = 0;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i)/ (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_PACED:
			/*at cst speed, this is the length done*/
			dist = frac*stack->length;
			/*then figure out in which seg we are*/
			len = 0;
			dlen = 0;
			for (i=0; i<nbVals-1; i++) {
				dx = pa->keyValue.vals[i+1].x - pa->keyValue.vals[i].x;
				dy = pa->keyValue.vals[i+1].y - pa->keyValue.vals[i].y;
				dz = pa->keyValue.vals[i+1].z - pa->keyValue.vals[i].z;
				dlen = (Float) sqrt(dx*dx+dy*dy+dz*dz);
				if (len+dlen>dist) break;
				len += dlen;
			}
			/*that's our fraction inside the seg*/
			frac = (dist-len)/dlen;
			break;
		case ANIM_SPLINE:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			i = (u32) (frac * (nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		default:
			return;
		}
		/*interpolate*/
		pa->value_changed.x = Interpolate(pa->keyValue.vals[i].x, pa->keyValue.vals[i+1].x, frac);
		pa->value_changed.y = Interpolate(pa->keyValue.vals[i].y, pa->keyValue.vals[i+1].y, frac);
		pa->value_changed.z = Interpolate(pa->keyValue.vals[i].z, pa->keyValue.vals[i+1].z, frac);
		break;
	/*bezier interpolate*/
	case ANIM_QUADRATIC:
	case ANIM_CUBIC:
	case ANIM_NURBS:
		if (!stack->anurbs.valid) return;
		/*compute frac*/
		switch (stack->anim_type) {
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = ((Float) i) / nbVals;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_VELOCITY:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			break;
		/*nothing to do for this one here*/
		case ANIM_DEFAULT:
		/*not supported - use frac as is*/
		case ANIM_PACED: 
		default:
			break;
		}
		/*evaluate*/
		i = anurbs_find_span(&stack->anurbs, frac);
		anurbs_basis(&stack->anurbs, i, frac);
		pa->value_changed = anurbs_get_vec3f(&stack->anurbs, i, pa->keyValue.vals);
		break;
	/*not supported*/
	case ANIM_SPLINE: 
	default:
		return;
	}

	pa->value_changed.x += pa->offset.x;
	pa->value_changed.y += pa->offset.y;
	pa->value_changed.z += pa->offset.z;
	Node_OnEventOutSTR(node, "value_changed");
}

void PA_Modified(SFNode *node, FieldInfo *field)
{
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);
	M_PositionAnimator *pa = (M_PositionAnimator *)node;

	if ( /*all fields impacting cached path len / nurbs*/
		(field->far_ptr == &pa->keyValue) 
		|| (field->far_ptr == &pa->keyValueType) 
		|| (field->far_ptr == &pa->key) 
		|| (field->far_ptr == &pa->keyType) 
		|| (field->far_ptr == &pa->keySpline) 
		|| (field->far_ptr == &pa->weight) 
		|| (field->far_ptr == &pa->key) 
		
		) stack->is_dirty = 1;
}
void PA_Init(SFNode *n)
{
	M_PositionAnimator *sa = (M_PositionAnimator*)n;
	AnimatorStack *stack;
	SAFEALLOC(stack, sizeof(AnimatorStack));
	stack->is_dirty = 1;
	Node_SetPrivate(n, stack);
	Node_SetPreDestroyFunction(n, Anim_Destroy);
	sa->on_set_fraction = PA_SetFraction;
}

static void PA2D_Update(M_PositionAnimator2D *pa, AnimatorStack *stack)
{
	u32 i;
	Float dx, dy;
	stack->is_dirty = 0;
	stack->anim_type = pa->keyType;
	/*if empty key and default anim switch to linear*/
	if (!pa->key.count && !stack->anim_type) stack->anim_type = ANIM_LINEAR;

	if (stack->anim_type == ANIM_PACED) {
		stack->length = 0;
		for (i=0; i<pa->keyValue.count-1; i++) {
			dx = pa->keyValue.vals[i+1].x - pa->keyValue.vals[i].x;
			dy = pa->keyValue.vals[i+1].y - pa->keyValue.vals[i].y;
			stack->length += (Float) sqrt(dx*dx + dy*dy);
		}
	} 
	Animator_Update(stack, pa->keyValueType, pa->keyValue.count, &pa->keySpline, pa->weight.count, pa->weight.vals);
}
static void PA2D_SetFraction(SFNode *node)
{
	Float frac;
	u32 nbKeys, nbVals, i;
	Float dx, dy;
	Float len, dlen, dist;
	M_PositionAnimator2D *pa = (M_PositionAnimator2D *)node;
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);

	frac = pa->set_fraction;
	if (!anim_check_frac(frac, &pa->fromTo)) return;

	if (stack->is_dirty) PA2D_Update(pa, stack);

	nbKeys = pa->key.count;
	nbVals = pa->keyValue.count;

	i=0;
	switch (pa->keyValueType) {
	/*linear interpolate*/
	case ANIM_LINE:
		/*compute frac and segment start index*/
		switch (stack->anim_type) {
		case ANIM_DEFAULT:
			if (nbKeys != nbVals) return;
			if (frac<pa->key.vals[0]) { i=0; frac = 0; }
			else if (frac>pa->key.vals[nbKeys-1]) { i=nbVals-2; frac=1; }
			else {
				for (i=0; i<nbKeys-1; i++) {
					if ((frac>=pa->key.vals[i]) && (frac<pa->key.vals[i+1])) break;
				}
				frac = GetInterpolateFraction(pa->key.vals[i], pa->key.vals[i+1], frac);
			}
			break;
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = 0;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i)/ (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_PACED:
			/*at cst speed, this is the length done*/
			dist = frac*stack->length;
			/*then figure out in which seg we are*/
			len = 0;
			dlen = 0;
			for (i=0; i<nbVals-1; i++) {
				dx = pa->keyValue.vals[i+1].x - pa->keyValue.vals[i].x;
				dy = pa->keyValue.vals[i+1].y - pa->keyValue.vals[i].y;
				dlen = (Float) sqrt(dx*dx+dy*dy);
				if (len+dlen>dist) break;
				len += dlen;
			}
			/*that's our fraction inside the seg*/
			frac = (dist-len)/dlen;
			break;
		case ANIM_SPLINE:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			i = (u32) (frac * (nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		default:
			return;
		}
		/*interpolate*/
		pa->value_changed.x = Interpolate(pa->keyValue.vals[i].x, pa->keyValue.vals[i+1].x, frac);
		pa->value_changed.y = Interpolate(pa->keyValue.vals[i].y, pa->keyValue.vals[i+1].y, frac);
		break;
	/*bezier interpolate*/
	case ANIM_QUADRATIC:
	case ANIM_CUBIC:
	case ANIM_NURBS:
		if (!stack->anurbs.valid) return;
		/*compute frac*/
		switch (stack->anim_type) {
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = ((Float) i) / nbVals;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_VELOCITY:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			break;
		/*nothing to do for this one here*/
		case ANIM_DEFAULT:
		/*not supported - use frac as is*/
		case ANIM_PACED: 
		default:
			break;
		}
		/*evaluate*/
		i = anurbs_find_span(&stack->anurbs, frac);
		anurbs_basis(&stack->anurbs, i, frac);
		pa->value_changed = anurbs_get_vec2f(&stack->anurbs, i, pa->keyValue.vals);
		break;
	/*not supported*/
	case ANIM_SPLINE: 
	default:
		return;
	}

	pa->value_changed.x += pa->offset.x;
	pa->value_changed.y += pa->offset.y;
	Node_OnEventOutSTR(node, "value_changed");
}

void PA2D_Modified(SFNode *node, FieldInfo *field)
{
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);
	M_PositionAnimator2D *pa = (M_PositionAnimator2D *)node;

	if ( /*all fields impacting cached path len / nurbs*/
		(field->far_ptr == &pa->keyValue) 
		|| (field->far_ptr == &pa->keyValueType) 
		|| (field->far_ptr == &pa->key) 
		|| (field->far_ptr == &pa->keyType) 
		|| (field->far_ptr == &pa->keySpline) 
		|| (field->far_ptr == &pa->weight) 
		|| (field->far_ptr == &pa->key) 
		
		) stack->is_dirty = 1;
}
void PA2D_Init(SFNode *n)
{
	M_PositionAnimator2D *sa = (M_PositionAnimator2D *)n;
	AnimatorStack *stack;
	SAFEALLOC(stack, sizeof(AnimatorStack));
	stack->is_dirty = 1;
	Node_SetPrivate(n, stack);
	Node_SetPreDestroyFunction(n, Anim_Destroy);
	sa->on_set_fraction = PA2D_SetFraction;
}

static void SA_Update(M_ScalarAnimator *sa, AnimatorStack *stack)
{
	u32 i;
	Float len;
	stack->is_dirty = 0;
	stack->anim_type = sa->keyType;
	/*if empty key and default anim switch to linear*/
	if (!sa->key.count && !stack->anim_type) stack->anim_type = ANIM_LINEAR;

	if (stack->anim_type == ANIM_PACED) {
		stack->length = 0;
		for (i=0; i<sa->keyValue.count-1; i++) {
			len = sa->keyValue.vals[i+1] - sa->keyValue.vals[i];
			stack->length += (Float) fabs(len);
		}
	} 
	Animator_Update(stack, sa->keyValueType, sa->keyValue.count, &sa->keySpline, sa->weight.count, sa->weight.vals);
}

void SA_SetFraction(SFNode *node)
{
	Float frac;
	u32 nbKeys, nbVals, i;
	Float len, dlen, dist;
	M_ScalarAnimator *sa = (M_ScalarAnimator *)node;
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);

	frac = sa->set_fraction;
	if (!anim_check_frac(frac, &sa->fromTo)) return;

	if (stack->is_dirty) SA_Update(sa, stack);

	nbKeys = sa->key.count;
	nbVals = sa->keyValue.count;

	i = 0;
	switch (sa->keyValueType) {
	/*linear interpolate*/
	case ANIM_LINE:
		/*compute frac & segment start index*/
		switch (stack->anim_type) {
		case ANIM_DEFAULT:
			if (nbKeys != nbVals) return;
			if (frac<sa->key.vals[0]) { i=0; frac = 0; }
			else if (frac>sa->key.vals[nbKeys-1]) { i=nbVals-2; frac=1; }
			else {
				for (i=0; i<nbKeys-1; i++) {
					if ((frac>=sa->key.vals[i]) && (frac<sa->key.vals[i+1])) break;
				}
				frac = GetInterpolateFraction(sa->key.vals[i], sa->key.vals[i+1], frac);
			}
			break;
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = 0;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i)/ (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_PACED:
			/*at cst speed, this is the length done*/
			dist = frac*stack->length;
			/*then figure out in which seg we are*/
			len = 0;
			dlen = 0;
			for (i=0; i<nbVals-1; i++) {
				dlen = (Float) fabs(sa->keyValue.vals[i+1] - sa->keyValue.vals[i]);
				if (len+dlen>dist) break;
				len += dlen;
			}
			/*that's our fraction inside the seg*/
			frac = (dist-len)/dlen;
			break;
		case ANIM_SPLINE:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			i = (u32) (frac * (nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		}
		/*interpolate*/
		sa->value_changed = Interpolate(sa->keyValue.vals[i], sa->keyValue.vals[i+1], frac);
		break;
	/*bezier interpolate*/
	case ANIM_QUADRATIC:
	case ANIM_CUBIC:
	case ANIM_NURBS:
		if (!stack->anurbs.valid) return;
		/*compute frac*/
		switch (stack->anim_type) {
		case ANIM_DISCRETE:
			i = (u32) (frac*nbVals);
			frac = ((Float) i) / nbVals;
			break;
		case ANIM_LINEAR:
			i = (u32) (frac*(nbVals-1));
			frac = (frac - ((Float) i) / (nbVals-1) ) * (nbVals-1);
			break;
		case ANIM_VELOCITY:
			frac = do_bisection(frac, stack->a, stack->b, stack->c, stack->d);
			break;
		/*nothing to do for this one here*/
		case ANIM_DEFAULT:
		/*not supported - use frac as is*/
		case ANIM_PACED: 
		default:
			break;
		}
		/*evaluate nurbs*/
		i = anurbs_find_span(&stack->anurbs, frac);
		anurbs_basis(&stack->anurbs, i, frac);
		sa->value_changed = anurbs_get_float(&stack->anurbs, i, sa->keyValue.vals);
		break;
	/*not supported*/
	case ANIM_SPLINE: 
	default:
		return;
	}

	sa->value_changed += sa->offset;
	Node_OnEventOutSTR(node, "value_changed");
}

void SA_Modified(SFNode *node, FieldInfo *field)
{
	AnimatorStack *stack = (AnimatorStack *)Node_GetPrivate(node);
	M_ScalarAnimator *sa = (M_ScalarAnimator *)node;

	if ( /*all fields impacting cached path len / nurbs*/
		(field->far_ptr == &sa->keyValue) 
		|| (field->far_ptr == &sa->keyValueType) 
		|| (field->far_ptr == &sa->key) 
		|| (field->far_ptr == &sa->keyType) 
		|| (field->far_ptr == &sa->keySpline) 
		|| (field->far_ptr == &sa->weight) 
		|| (field->far_ptr == &sa->key) 
		
		) stack->is_dirty = 1;
}

void SA_Init(SFNode *n)
{
	M_ScalarAnimator *sa = (M_ScalarAnimator *)n;
	AnimatorStack *stack;
	SAFEALLOC(stack, sizeof(AnimatorStack));
	stack->is_dirty = 1;
	Node_SetPrivate(n, stack);
	Node_SetPreDestroyFunction(n, Anim_Destroy);
	sa->on_set_fraction = SA_SetFraction;
}


