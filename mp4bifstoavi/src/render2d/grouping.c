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


#include "grouping.h"
#include "visualsurface2d.h"


void child2d_compute_bounds(ChildGroup2D *cg)
{
	u32 i, count;
	Float a, d;
	if (cg->bounds_forced) return;
	cg->is_text_group = 1;
	cg->ascent = cg->descent = 0;
	m4_rect_reset(&cg->original);

	count = ChainGetCount(cg->contexts);
	for (i=0; i<count; i++) {
		void text2D_get_ascent_descent(DrawableContext *ctx, Float *a, Float *d);
		DrawableContext *ctx = ChainGetEntry(cg->contexts, i);
		m4_rect_union(&cg->original, &ctx->unclip);
		if (!cg->is_text_group) continue;
		if (!ctx->is_text) {
			cg->is_text_group = 0;
			continue;
		}
		text2D_get_ascent_descent(ctx, &a, &d);
		if (a>cg->ascent) cg->ascent = a;
		if (d>cg->descent) cg->descent = d;
	}
}

void group2d_force_bounds(GroupingNode2D *group, M4Rect *clip)
{
	ChildGroup2D *cg;
	if (!group || !clip) return;
	cg = ChainGetEntry(group->groups, ChainGetCount(group->groups)-1);
	if (!cg) return;
	cg->ascent = cg->descent = 0;
	cg->is_text_group = 0;
	cg->original = *clip;
	cg->final = cg->original;
	cg->bounds_forced = 1;
}

void group2d_start_child(GroupingNode2D *group)
{
	ChildGroup2D *cg = malloc(sizeof(ChildGroup2D));
	memset(cg, 0, sizeof(ChildGroup2D));
	cg->contexts = NewChain();
	ChainAddEntry(group->groups, cg);
}
void group2d_end_child(GroupingNode2D *group)
{
	ChildGroup2D *cg = ChainGetEntry(group->groups, ChainGetCount(group->groups)-1);
	if (!cg) return;
	child2d_compute_bounds(cg);
	cg->final = cg->original;
}

void group2d_reset_children(GroupingNode2D *group)
{
	while (ChainGetCount(group->groups)) {
		ChildGroup2D *cg = ChainGetEntry(group->groups, 0);
		ChainDeleteEntry(group->groups, 0);
		DeleteChain(cg->contexts);
		free(cg);
	}
}

void group2d_add_to_context_list(GroupingNode2D *group, DrawableContext *ctx)
{
	ChildGroup2D *cg = ChainGetEntry(group->groups, ChainGetCount(group->groups)-1);
	if (!cg) return;
	ChainAddEntry(cg->contexts, ctx);
}
/*
	This is the generic routine for child traversing - note we are not duplicating the effect
*/
void group2d_traverse(GroupingNode2D *group, Chain *children, RenderEffect2D *eff)
{
	u32 i, count;
	Bool split_text_backup;
	SFNode *child;
	u32 count2;
	SensorHandler *hsens;
	Chain *sensors_backup = NULL;
	count = ChainGetCount(children);

	/*rebuild sensor list */
	if (Node_GetDirty(group->owner) & SG_CHILD_DIRTY) {
		/*reset*/
		ChainReset(group->sensors);

		/*special case for anchor which is a parent node acting as a sensor*/
		if (Node_GetTag(group->owner)==TAG_MPEG4_Anchor) {
			SensorHandler *r2d_anchor_get_handler(SFNode *n);
			hsens = r2d_anchor_get_handler(group->owner);
			if (hsens) ChainAddEntry(group->sensors, hsens);
		}

		for (i=0; i<count; i++) {
			child = ChainGetEntry(children, i);
			if (!child || !is_sensor_node(child) ) continue;
			hsens = get_sensor_handler(child);
			/*only keep track of locally enabled sensors*/
			if (hsens) ChainAddEntry(group->sensors, hsens);
		}
	}

	/*if we have an active sensor at this level discard all sensors in current render context (cf VRML)*/
	count2 = ChainGetCount(group->sensors);
	if (count2) {
		sensors_backup = eff->sensors;
		eff->sensors = NewChain();
	
		/*add sensor(s) to effects*/	
		for (i=0; i <count2; i++) {
			SensorHandler *hsens = ChainGetEntry(group->sensors, i);
			effect_add_sensor(eff, hsens, &eff->transform);
		}
	}

	Node_ClearDirty(group->owner);

	if (eff->parent == group) {
		for (i=0; i<count; i++) {
			group2d_start_child(group);
			child = ChainGetEntry(children, i);
			Node_Render(child, eff);
			group2d_end_child(group);
		}
	} else {
		split_text_backup = eff->text_split_mode;
		if (count>1) eff->text_split_mode = 0;
		for (i=0; i<count; i++) {
			child = ChainGetEntry(children, i);
			Node_Render(child, eff);
		}
		eff->text_split_mode = split_text_backup;
	}
	if (count2) {
		/*destroy current effect list and restore previous*/
		effect_reset_sensors(eff);
		DeleteChain(eff->sensors);
		eff->sensors = sensors_backup;
	}
}

void mx2d_apply_rect_int(M4Matrix2D *mat, M4IRect *rc)
{
	M4Rect rcft = m4_rect_ft(rc);
	mx2d_apply_rect(mat, &rcft);
	*rc = m4_rect_pixelize(&rcft);
}

void child2d_render_done(ChildGroup2D *cg, RenderEffect2D *eff, M4Rect *par_clipper)
{
	M4Matrix2D mat, loc_mx;
	Float x, y;
	u32 i, count;
	u32 j, scount;
	M4Rect _clip;
	M4IRect clipper;

	_clip = *par_clipper;
	mx2d_apply_rect(&eff->transform, &_clip);
	clipper = m4_rect_pixelize(&_clip);

	mx2d_init(mat);
	mx2d_add_translation(&mat, cg->final.x - cg->original.x, cg->final.y - cg->original.y);

	count = ChainGetCount(cg->contexts);
	for (i=0; i<count; i++) {
		DrawableContext *ctx = ChainGetEntry(cg->contexts, i);

		mx2d_apply_coords(&mat, &ctx->unclip.x, &ctx->unclip.y);
		x = (Float) ctx->clip.x; y = (Float) ctx->clip.y;
		mx2d_apply_coords(&mat, &x, &y);
		ctx->clip.x = (s32) floor(x);
		ctx->clip.y = (s32) ceil(y);

		mx2d_add_matrix(&ctx->transform, &mat);
		if (!eff->is_pixel_metrics) mx2d_add_scale(&ctx->transform, 1.0f/eff->min_hsize, 1.0f/eff->min_hsize);
		mx2d_add_matrix(&ctx->transform, &eff->transform);

		scount = ChainGetCount(ctx->sensors);
		for (j=0; j<scount; j++) {
			SensorContext *sc = ChainGetEntry(ctx->sensors, j);

			if (!eff->is_pixel_metrics) mx2d_add_scale(&sc->matrix, 1.0f/eff->min_hsize, 1.0f/eff->min_hsize);
			mx2d_add_matrix(&sc->matrix, &eff->transform);
		}
		mx2d_init(loc_mx);
		if (!eff->is_pixel_metrics) mx2d_add_scale(&loc_mx, 1.0f/eff->min_hsize, 1.0f/eff->min_hsize);
		mx2d_add_matrix(&loc_mx, &eff->transform);

		mx2d_apply_rect(&loc_mx, &ctx->unclip);
		ctx->unclip_pix = m4_rect_pixelize(&ctx->unclip);

		mx2d_apply_rect_int(&loc_mx, &ctx->clip);
		m4_irect_intersect(&ctx->clip, &clipper);

		if (eff->parent) {
			group2d_add_to_context_list(eff->parent, ctx);
		} else if (eff->trav_flags & TF_RENDER_DIRECT) {
			ctx->node->Draw(ctx);
		}
	}
}
void child2d_render_done_complex(ChildGroup2D *cg, RenderEffect2D *eff, M4Matrix2D *mat)
{
	u32 i, count;
	u32 j, scount;
	
	count = ChainGetCount(cg->contexts);
	for (i=0; i<count; i++) {
		DrawableContext *ctx = ChainGetEntry(cg->contexts, i);
		if (!mat) {
			m4_rect_reset(&ctx->clip);
			m4_rect_reset(&ctx->unclip);
			continue;
		}
		mx2d_add_matrix(&ctx->transform, mat);
		mx2d_add_matrix(&ctx->transform, &eff->transform);

		scount = ChainGetCount(ctx->sensors);
		for (j=0; j<scount; j++) {
			SensorContext *sc = ChainGetEntry(ctx->sensors, j);
			mx2d_add_matrix(&sc->matrix, &eff->transform);
		}
		mx2d_apply_rect(&ctx->transform, &ctx->unclip);
		ctx->unclip_pix = m4_rect_pixelize(&ctx->unclip);
		mx2d_apply_rect_int(&ctx->transform, &ctx->clip);

		if (eff->parent) {
			group2d_add_to_context_list(eff->parent, ctx);
		} else if (eff->trav_flags & TF_RENDER_DIRECT) {
			ctx->node->Draw(ctx);
		}
	}
}


void SetupGroupingNode2D(GroupingNode2D *group, Render2D *sr, SFNode *node)
{
	memset(group, 0, sizeof(GroupingNode2D));
	traversable_setup(group, node, sr->compositor);
	group->sensors = NewChain();
	group->groups = NewChain();
}

void DeleteGroupingNode2D(GroupingNode2D *group)
{
	/*just in case*/
	group2d_reset_children(group);
	DeleteChain(group->sensors);
	group2d_reset_children(group);
	DeleteChain(group->groups);
}

void DestroyBaseGrouping2D(SFNode *node)
{
	GroupingNode2D *group = (GroupingNode2D *)Node_GetPrivate(node);
	DeleteGroupingNode2D(group);
	free(group);
}

