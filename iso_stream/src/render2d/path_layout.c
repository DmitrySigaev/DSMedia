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
#include "visualsurface2d.h"

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	
	SFNode *last_geom;
	M4PathIterator *iter;
} PathLayoutStack;

static void DestroyPathLayout(SFNode *node)
{
	PathLayoutStack *st = (PathLayoutStack *)Node_GetPrivate(node);
	DeleteGroupingNode2D((GroupingNode2D *)st);
	if (st->iter) m4_path_delete_iterator(st->iter);
	free(st);
}

static void RenderPathLayout(SFNode *node, void *rs)
{
	u32 i, count, minor, major, int_bck;
	Float length, offset, length_after_point;
	Bool res;
	ChildGroup2D *cg;
	M4Matrix2D mat;
	GroupingNode2D *parent_bck;
	PathLayoutStack *gr = (PathLayoutStack *) Node_GetPrivate(node);
	M_PathLayout *pl = (M_PathLayout *)node;
	RenderEffect2D *eff = (RenderEffect2D *) rs;
	
	if (!pl->geometry) return;
	
	/*only low-level primitives allowed*/
	switch (Node_GetTag((SFNode *) pl->geometry)) {
	case TAG_MPEG4_Rectangle: return;
	case TAG_MPEG4_Circle: return;
	case TAG_MPEG4_Ellipse: return;
	}

	/*store effect*/
	mx2d_copy(mat, eff->transform);
	parent_bck = eff->parent;

	mx2d_init(eff->transform);
	eff->parent = NULL;

	/*check geom changes*/
	if ((pl->geometry != gr->last_geom) || Node_GetDirty(pl->geometry)) {
		if (gr->iter) m4_path_delete_iterator(gr->iter);
		gr->iter = NULL;

		int_bck = eff->trav_flags;
		eff->trav_flags |= TF_SWITCHED_OFF;
		Node_Render(pl->geometry, eff);
		eff->trav_flags = int_bck;

		gr->last_geom = pl->geometry;
	}

	if (!gr->iter) {
		/*get the drawable*/
		Drawable *dr = (Drawable *) Node_GetPrivate( (SFNode *) pl->geometry);
		/*init iteration*/
		if (!dr || !dr->path) return;
		gr->iter = m4_path_new_iterator(dr->path);
		if (!gr->iter) return;
	}
	
	eff->parent = (GroupingNode2D*)gr;
	int_bck = eff->text_split_mode;
	eff->text_split_mode = 2;
	group2d_traverse((GroupingNode2D*)gr, pl->children, eff);
	eff->text_split_mode = int_bck;

	/*restore effect*/
	mx2d_copy(eff->transform, mat);
	eff->parent = parent_bck;

	count = ChainGetCount(gr->groups);
	
	length = gr->iter->length;
	offset = length * pl->pathOffset;

	major = pl->alignment.count ? pl->alignment.vals[0] : 0;
	minor = (pl->alignment.count==2) ? pl->alignment.vals[1] : 0;

	if (pl->wrapMode==1) {
		while (offset<0) offset += length;
	}

	for (i=0; i<count; i++) {
		cg = ChainGetEntry(gr->groups, i);
		if (cg->original.width>length) break;

		/*first set our center and baseline*/
		mx2d_init(mat);

		/*major align*/
		switch (major) {
		case 2:
			if (cg->is_text_group) mx2d_add_translation(&mat, -1*cg->original.x - cg->original.width, 0);
			else mx2d_add_translation(&mat, -1 * cg->original.width/2, 0);
			length_after_point = 0;
			break;
		case 1:
			length_after_point = cg->original.width/2;
			if (cg->is_text_group) mx2d_add_translation(&mat, -1*cg->original.x - cg->original.width / 2, 0);
			break;
		default:
		case 0:
			if (cg->is_text_group) mx2d_add_translation(&mat, cg->original.x, 0);
			else mx2d_add_translation(&mat, cg->original.width/2, 0);
			length_after_point = cg->original.width;
			break;
		}

		/*if wrapping and out of path, restart*/
		if ((pl->wrapMode==1) && (offset+length_after_point>=length)) {
			offset += length_after_point;
			offset -= length;
			i--;
			continue;
		}
		/*if not wrapping and not yet in path skip */
		if (!pl->wrapMode && (offset+length_after_point < 0)) {
			child2d_render_done_complex(cg, (RenderEffect2D *)rs, NULL);
			goto next;
		}

		/*minor justify*/
		switch (minor) {
		/*top alignment*/
		case 3:
			if (cg->is_text_group) 
				mx2d_add_translation(&mat, 0, -1 * cg->ascent);
			else 
				mx2d_add_translation(&mat, 0, -1 * cg->original.height / 2);
			
			break;
		/*baseline*/
		case 1:
			/*move to bottom align if not text*/
			if (!cg->is_text_group) 
				mx2d_add_translation(&mat, 0, cg->original.height / 2);
			break;
		/*middle*/
		case 2:
			/*if text use (asc+desc) /2 as line height since glyph height differ*/
			if (cg->is_text_group) 
				mx2d_add_translation(&mat, 0, cg->descent - (cg->ascent + cg->descent) / 2);
			break;
		/*bottomline alignment*/
		case 0:
		default:
			if (cg->is_text_group)
				mx2d_add_translation(&mat, 0, cg->descent);
			else
				mx2d_add_translation(&mat, 0, cg->original.height / 2);
			
			break;
		}
		res = m4_pathiterator_get_transform_at_offset(gr->iter, offset, (Bool) (pl->wrapMode==2), &mat, 1, length_after_point);
		if (!res) break;

		child2d_render_done_complex(cg, (RenderEffect2D *)rs, &mat);

next:
		if (i+1<count) {
			ChildGroup2D *cg_next = ChainGetEntry(gr->groups, i+1);

			/*update offset according to major alignment */
			switch (major) {
			case 2:
				if (cg_next->is_text_group) offset += pl->spacing * cg_next->original.x;
				offset += pl->spacing * cg_next->original.width;
				break;
			case 1:
				if (cg->is_text_group) offset += pl->spacing * cg->original.x / 2;
				offset += pl->spacing * cg->original.width / 2;
				offset += cg_next->original.width / 2;
				break;
			default:
			case 0:
				if (cg->is_text_group) offset += pl->spacing * cg->original.x;
				offset += pl->spacing * cg->original.width;
				break;
			}
		}
		/*wrap*/
		if ((pl->wrapMode==1) && (offset>=length)) offset-=length;
	}

	/*undrawn nodes*/
	for (;i<count; i++) {
		cg = ChainGetEntry(gr->groups, i);
		child2d_render_done_complex(cg, (RenderEffect2D *)rs, NULL);
	}
	group2d_reset_children((GroupingNode2D *) gr);
}

void R2D_InitPathLayout(Render2D *sr, SFNode *node)
{
	PathLayoutStack *stack;
	SAFEALLOC(stack, sizeof(PathLayoutStack));
	SetupGroupingNode2D((GroupingNode2D*)stack, sr, node);

	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyPathLayout);
	Node_SetRenderFunction(node, RenderPathLayout);
}

