/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / 2D rendering plugin
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



#include "stacks2d.h"

#include "grouping.h"
#include "visualsurface2d.h"

/*storage of group info*/
typedef struct
{
	Chain *children;
	M4Rect origin, final;
} FormGroup;

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	Chain *grouplist;
	M4Rect clip;
} FormStack;

static void form_reset(FormStack *st)
{
	while (ChainGetCount(st->grouplist)) {
		FormGroup * fg = ChainGetEntry(st->grouplist, 0);
		ChainDeleteEntry(st->grouplist, 0);
		DeleteChain(fg->children);
		free(fg);
	}
}

static FormGroup *form_new_group(FormStack *st)
{
	FormGroup *fg = malloc(sizeof(FormGroup));
	m4_rect_reset(&fg->final);
	m4_rect_reset(&fg->origin);
	fg->children = NewChain();
	ChainAddEntry(st->grouplist, fg);
	return fg;
}

static M4INLINE FormGroup *form_get_group(FormStack *st, u32 i)
{
	return (FormGroup *) ChainGetEntry(st->grouplist, i);
}

static void fg_compute_bounds(FormGroup *fg)
{
	u32 i;
	m4_rect_reset(&fg->origin);
	for (i=0; i<ChainGetCount(fg->children); i++) {
		ChildGroup2D *cg = ChainGetEntry(fg->children, i);
		m4_rect_union(&fg->origin, &cg->final);
	}
	fg->final = fg->origin;
}

static void fg_update_bounds(FormGroup *fg)
{
	u32 i;
	Float x, y;
	x = fg->final.x - fg->origin.x;
	y = fg->final.y - fg->origin.y;
	for (i=0; i<ChainGetCount(fg->children); i++) {
		ChildGroup2D *cg = ChainGetEntry(fg->children, i);
		cg->final.x += x;
		cg->final.y += y;
	}
	fg_compute_bounds(fg);
}


static void DestroyForm(SFNode *n)
{
	FormStack *st = (FormStack *)Node_GetPrivate(n);
	DeleteGroupingNode2D((GroupingNode2D *)st);

	form_reset(st);
	DeleteChain(st->grouplist);
	free(st);
}

static void shin_apply(FormStack *st, u32 *group_idx, u32 count);
static void sh_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void svin_apply(FormStack *st, u32 *group_idx, u32 count);
static void sv_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void al_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void ar_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void at_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void ab_apply(FormStack *st, Float space, u32 *group_idx, u32 count);
static void ah_apply(FormStack *st, u32 *group_idx, u32 count);
static void av_apply(FormStack *st, u32 *group_idx, u32 count);

static void form_apply(FormStack *st, const char *constraint, u32 *group_idx, u32 count)
{
	Float val;
	if (!constraint || !strlen(constraint)) return;

	/*SHin*/
	if (!strnicmp(constraint, "SHin", 4)) {
		shin_apply(st, group_idx, count);
		return;
	}
	/*SH*/
	if (!strnicmp(constraint, "SH", 2)) {
		if (sscanf(constraint, "SH %f", &val)==1) {
			sh_apply(st, val, group_idx, count);
		} else {
			sh_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*SVin*/
	if (!strnicmp(constraint, "SVin", 4)) {
		svin_apply(st, group_idx, count);
		return;
	}
	/*SV*/
	if (!strnicmp(constraint, "SV", 2)) {
		if (sscanf(constraint, "SV %f", &val)==1) {
			sv_apply(st, val, group_idx, count);
		} else {
			sv_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*AL*/
	if (!strnicmp(constraint, "AL", 2)) {
		if (sscanf(constraint, "AL %f", &val)==1) {
			al_apply(st, val, group_idx, count);
		} else {
			al_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*AR*/
	if (!strnicmp(constraint, "AR", 2)) {
		if (sscanf(constraint, "AR %f", &val)==1) {
			ar_apply(st, val, group_idx, count);
		} else {
			ar_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*AT*/
	if (!strnicmp(constraint, "AT", 2)) {
		if (sscanf(constraint, "AT %f", &val)==1) {
			at_apply(st, val, group_idx, count);
		} else {
			at_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*AB*/
	if (!strnicmp(constraint, "AB", 2)) {
		if (sscanf(constraint, "AB %f", &val)==1) {
			ab_apply(st, val, group_idx, count);
		} else {
			ab_apply(st, -1, group_idx, count);
		}
		return;
	}
	/*AH*/
	if (!strnicmp(constraint, "AH", 2)) {
		ah_apply(st, group_idx, count);
		return;
	}
	/*AV*/
	if (!strnicmp(constraint, "AV", 2)) {
		av_apply(st, group_idx, count);
		return;
	}
}


#ifndef MAX_FORM_GROUP_INDEX
#define MAX_FORM_GROUP_INDEX	100
#endif

static void RenderForm(SFNode *n, void *rs)
{
	u32 idx[MAX_FORM_GROUP_INDEX];
	u32 i, index, last_ind, j;
	M4Matrix2D mx2d_bck;
	GroupingNode2D *parent_bck;
	FormGroup *fg;
	ChildGroup2D *cg;
	M_Form *fm = (M_Form *) n;
	FormStack *st = (FormStack *)Node_GetPrivate(n);
	RenderEffect2D *eff = (RenderEffect2D *) rs;

	/*init effect*/
	mx2d_copy(mx2d_bck, eff->transform);
	parent_bck = eff->parent;
	eff->parent = (GroupingNode2D *) st;
	mx2d_init(eff->transform);

	if (!eff->is_pixel_metrics) mx2d_add_scale(&eff->transform, eff->min_hsize, eff->min_hsize);
	st->clip = R2D_ClipperToPixelMetrics(eff, fm->size);

	group2d_traverse((GroupingNode2D *)st, fm->children, eff);
	/*restore effect*/
	mx2d_copy(eff->transform, mx2d_bck);
	eff->parent = parent_bck;

	/*center all nodes*/
	for (i=0; i<ChainGetCount(st->groups); i++) {
		ChildGroup2D *cg = ChainGetEntry(st->groups, i);
		cg->final.x = - cg->final.width/2;
		cg->final.y = cg->final.height/2;
	}
	/*build groups*/
	form_reset(st);
	fg = form_new_group(st);
	fg->origin = fg->final = st->clip;
	fg = NULL;
	for (i=0; i<fm->groups.count; i++) {
		if (!fg) {
			fg = form_new_group(st);
		}
		if (fm->groups.vals[i]==-1) {
			fg_compute_bounds(fg);
			fg = NULL;
			continue;
		}
		/*broken form*/
		if ((u32) fm->groups.vals[i]>ChainGetCount(st->groups)) goto exit;
		cg = ChainGetEntry(st->groups, fm->groups.vals[i]-1);
		ChainAddEntry(fg->children, cg);
	}

	last_ind = 0;
	for (i=0; i<fm->constraints.count; i++) {
		index = 0;
		while (1) {
			if (last_ind+index > fm->groupsIndex.count) goto exit;
			if (fm->groupsIndex.vals[last_ind+index]==-1) break;
			if (index>MAX_FORM_GROUP_INDEX) goto exit;
			idx[index] = fm->groupsIndex.vals[last_ind+index];
			index++;
		}
		/*apply*/
		form_apply(st, fm->constraints.vals[i], idx, index);
		index++;
		last_ind += index;

		/*refresh all group bounds*/
		for (j=1; j<ChainGetCount(st->grouplist);j++) {
			fg = ChainGetEntry(st->grouplist, j);
			fg_compute_bounds(fg);
		}
		/*done*/
		if (last_ind>=fm->groupsIndex.count) break;
	}

	for (i=0; i<ChainGetCount(st->groups); i++) {
		cg = ChainGetEntry(st->groups, i);
		child2d_render_done(cg, eff, &st->clip);
	}

exit:
	group2d_reset_children((GroupingNode2D*)st);
	group2d_force_bounds(eff->parent, &st->clip);
	form_reset(st);
}

void R2D_InitForm(Render2D *sr, SFNode *node)
{
	FormStack *stack = malloc(sizeof(FormStack));
	memset(stack, 0, sizeof(FormStack));
	SetupGroupingNode2D((GroupingNode2D*)stack, sr, node);
	stack->grouplist = NewChain();

	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyForm);
	Node_SetRenderFunction(node, RenderForm);
}







/*
			FORM CONSTRAINTS
*/


static void shin_apply(FormStack *st, u32 *group_idx, u32 count)
{
	u32 i, len;
	Float tot_len, inter_space;
	tot_len = 0;
	inter_space = st->clip.width;

	len = 0;
	for (i=0; i<count; i++) {
		if (group_idx[i] != 0) {
			tot_len += form_get_group(st, group_idx[i])->final.width;
			len++;
		}
	}
	inter_space -= tot_len;
	inter_space /= (len+1);

	for (i=0; i<count; i++) {
		if(group_idx[i] == 0) continue;
		if (!i) {
			form_get_group(st, group_idx[0])->final.x = st->clip.x + inter_space;
		} else {
			form_get_group(st, group_idx[i])->final.x = 
				form_get_group(st, group_idx[i-1])->final.x + form_get_group(st, group_idx[i-1])->final.width 
				+ inter_space;
		}
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void sh_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	u32 i, k;
	M4Rect *l, *r;
	Float inter_space, tot_len;

	tot_len = 0;
	if (space == -1) {
		r = &form_get_group(st, group_idx[count-1])->final;
		l = &form_get_group(st, group_idx[0])->final;
		inter_space = r->x - l->x;
		if(group_idx[0] != 0) inter_space -= l->width;
		for (i=1; i<count-1; i++) tot_len += form_get_group(st, group_idx[i])->final.width;
		inter_space -= tot_len;
		inter_space /= (count-1);
	} else {
		inter_space = (Float) space;
	}	
	
	k = count - 1;
	if (space != -1) k += 1;
	for (i=1; i<k; i++) {
		if (group_idx[i] ==0) continue;
		l = &form_get_group(st, group_idx[i-1])->final;
		r = &form_get_group(st, group_idx[i])->final;
		r->x = l->x + inter_space;
		if(group_idx[i-1] != 0) r->x += l->width;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void svin_apply(FormStack *st, u32 *group_idx, u32 count)
{
	u32 i, len;
	Float tot_len, inter_space;
	tot_len = 0;
	inter_space = st->clip.height;
	len = 0;
	for (i=0; i<count;i++) {
		if (group_idx[i] != 0) {
			tot_len += form_get_group(st, group_idx[i])->final.height;
			len++;
		}
	}
	inter_space -= tot_len;
	inter_space /= (len+1);
				
	for (i=0; i<count; i++) {
		if (group_idx[i] == 0) continue;
		if (!i) {
			form_get_group(st, group_idx[0])->final.y = st->clip.y - inter_space;
		} else {
			form_get_group(st, group_idx[i])->final.y =
				form_get_group(st, group_idx[i-1])->final.y - form_get_group(st, group_idx[i-1])->final.height -
				inter_space;				
		}
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void sv_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	u32 i, k;
	Float tot_len, inter_space;
	M4Rect *t, *b;

	tot_len = 0;
	if(space > -1) {
		inter_space = space;
	} else {
		t = &form_get_group(st, group_idx[count-1])->final;
		b = &form_get_group(st, group_idx[0])->final;
		inter_space = b->y - t->y;
		if (group_idx[0]!=0) inter_space -= t->height;
		for (i=1; i<count-1; i++) tot_len += form_get_group(st, group_idx[i])->final.height;
		inter_space -= tot_len;
		inter_space /= count-1;
	}	
	
	k = count-1;
	if (space > -1) k += 1;
	for (i=1; i<k; i++) {
		if (group_idx[i] == 0) continue;
		form_get_group(st, group_idx[i])->final.y = form_get_group(st, group_idx[i-1])->final.y - inter_space;
		if (group_idx[i-1] != 0) {
			form_get_group(st, group_idx[i])->final.y -= form_get_group(st, group_idx[i-1])->final.height;
		}
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void al_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	Float min_x;
	M4Rect *rc;
	u32 i, start;

	start = 0;
	min_x = form_get_group(st, group_idx[0])->final.x;
	if (space>-1) {
		start = 1;
		min_x += space;
	} else {
		for (i=1; i<count; i++) {
			rc = &form_get_group(st, group_idx[0])->final;
			if (group_idx[i]==0) {
				min_x = rc->x;
				break;
			}
			if (rc->x < min_x) min_x = rc->x;
		}
	}
	for (i=start; i<count; i++) {
		if( group_idx[i] == 0) continue;
		form_get_group(st, group_idx[i])->final.x = min_x;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void ar_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	Float max_x;
	u32 i, start;
	M4Rect *rc;

	start = 0;
	rc = &form_get_group(st, group_idx[0])->final;
	max_x = rc->x + rc->width;
	
	if(space>-1) {
		max_x -= space;
		start = 1;
	} else {
		for (i=1; i<count; i++) {
			rc = &form_get_group(st, group_idx[i])->final;
			if (group_idx[i]==0) {
				max_x = rc->x + rc->width;
				break;
			}
			if (rc->x + rc->width > max_x) max_x = rc->x + rc->width;
		}
	}
	for (i=start; i<count; i++) {
		if(group_idx[i] == 0) continue;
		rc = &form_get_group(st, group_idx[i])->final;
		rc->x = max_x - rc->width;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void at_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	Float max_y;
	u32 i, start;
	M4Rect *rc;

	start = 0;
	max_y = form_get_group(st, group_idx[0])->final.y;
	if(space>-1) {
		start = 1;
		max_y -= space;
	} else {
		for (i=1; i<count; i++) {
			rc = &form_get_group(st, group_idx[i])->final;
			if (group_idx[i]==0) {
				max_y = rc->y;
				break;
			}
			if (rc->y > max_y) max_y = rc->y;
		}
	}

	for (i=start; i<count; i++) {
		if(group_idx[i] == 0) continue;
		form_get_group(st, group_idx[i])->final.y = max_y;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void ab_apply(FormStack *st, Float space, u32 *group_idx, u32 count)
{
	Float min_y;
	u32 i, start;
	M4Rect *rc;

	start = 0;
	rc = &form_get_group(st, group_idx[0])->final;
	min_y = rc->y - rc->height;
	if(space>-1) {
		start = 1;
		min_y += space;
	} else {
		for (i=1; i<count; i++) {
			rc = &form_get_group(st, group_idx[i])->final;
			if (group_idx[i]==0) {
				min_y = rc->y - rc->height;
				break;
			}
			if (rc->y - rc->height < min_y) min_y = rc->y - rc->height;
		}
	}
	for (i=start; i<count; i++) {
		if(group_idx[i] == 0) continue;
		rc = &form_get_group(st, group_idx[i])->final;
		rc->y = min_y + rc->height;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void ah_apply(FormStack *st, u32 *group_idx, u32 count)
{
	M4Rect *rc;
	u32 i;
	Float left, right, center;
	left = right = center = 0;

	for (i=0; i<count; i++) {
		rc = &form_get_group(st, group_idx[i])->final;
		if(group_idx[i] == 0) {
			center = rc->x + rc->width / 2;
			break;
		}
		if (left > rc->x) left = rc->x;
		if (right < rc->x + rc->width) right = rc->x + rc->width;
		center = (left+right)/2;
	}

	for (i=0; i<count; i++) {
		if(group_idx[i] == 0) continue;
		rc = &form_get_group(st, group_idx[i])->final;
		rc->x = center - rc->width/2;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

static void av_apply(FormStack *st, u32 *group_idx, u32 count)
{
	u32 i;
	Float top, bottom, center;
	M4Rect *rc;
	top = bottom = center = 0;

	for (i=0; i<count; i++) {
		rc = &form_get_group(st, group_idx[i])->final;
		if (group_idx[i] == 0) {
			center = rc->y - rc->height / 2;
			break;
		}
		if (top < rc->y) top = rc->y;
		if (bottom > rc->y - rc->height) bottom = rc->y - rc->height;
		center = (top+bottom)/2;
	}

	for (i=0; i<count; i++) {
		if(group_idx[i] == 0) continue;
		rc = &form_get_group(st, group_idx[i])->final;
		rc->y = center + rc->height/2;
		fg_update_bounds(form_get_group(st, group_idx[i]));
	}
}

