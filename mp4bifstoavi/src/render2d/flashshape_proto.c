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

#include "drawable.h"
#include "visualsurface2d.h"

typedef struct
{
	LPM4PATH path;
	Float width;
	u32 fill_col, line_col;
} FSItem;


typedef struct
{
	M4Rect bounds;
	Drawable *graph;
	Chain *items;
	Float max_width;
} FSStack;


static void clean_paths(FSStack *stack, Graphics2DDriver *dr)
{
	/*delete all path objects*/
	while (ChainGetCount(stack->items)) {
		FSItem *it = ChainGetEntry(stack->items, 0);
		ChainDeleteEntry(stack->items, 0);
		if (it->path) m4_path_delete(it->path);
		free(it);
	}
}

static void FS_Destroy(SFNode *node)
{
	FSStack *stack = (FSStack *) Node_GetPrivate(node);
	
	clean_paths(stack, stack->graph->compositor->g_hw);
	DeleteDrawableNode(stack->graph);
	DeleteChain(stack->items);
	free(stack);
}

static FSItem *new_fs_item(FSStack *st, u32 line_col, u32 fill_col, Float width)
{
	FSItem *item = malloc(sizeof(FSItem));
	memset(item, 0, sizeof(FSItem));
	ChainAddEntry(st->items, item);
	item->fill_col = fill_col;
	item->width = width;
	item->line_col = line_col;
	item->path = m4_new_path();
	return item;
}

#define SFCOL_MAKE_ARGB(c) MAKE_ARGB_FLOAT(1.0, c.red, c.green, c.blue);

static void build_shape(FSStack *st, SFNode *node)
{
	FieldInfo field;
	MFVec2f *coords;
	MFInt32 *com, *widthIndex, *lineIndex, *fillIndex, *coordIndex;
	MFFloat *widths;
	MFColor *colors;
	u32 wi, li, fi, ci, command, i, has_ci;
	FSItem *fill_item, *line_item;
	Float w;
	SFVec2f cur, pt, ct1, ct2, *pts;
	M4Rect rc;
	Graphics2DDriver *dr;
	u32 line_col, fill_col;
	Bool need_line, need_fill;

	dr = st->graph->compositor->g_hw;

	/*get all fields*/
	Node_GetField(node, 0, &field);
	coords = field.far_ptr;
	Node_GetField(node, 1, &field);
	com = field.far_ptr;
	Node_GetField(node, 2, &field);
	widths = field.far_ptr;
	Node_GetField(node, 3, &field);
	colors = field.far_ptr;
	Node_GetField(node, 4, &field);
	widthIndex = field.far_ptr;
	Node_GetField(node, 5, &field);
	lineIndex = field.far_ptr;
	Node_GetField(node, 6, &field);
	fillIndex = field.far_ptr;
	Node_GetField(node, 7, &field);
	coordIndex = field.far_ptr;

	wi = li = fi = ci = 0;
	w = 0;

	st->max_width = 0;
	cur.x = cur.y = 0;
	fill_item = line_item = NULL;
	need_line = need_fill = 0;
	cur.x = cur.y = 0;
	has_ci = coordIndex->count ? 1 : 0;
	pts = coords->vals;
	line_col = fill_col = 0;

	/*implicit commands: 0 1 2 3*/

/*
	if (widthIndex->count) {
		w = (widthIndex->vals[wi]==-1) ? 0 : widths->vals[widthIndex->vals[wi]];
		if (!w) {
			need_line = 0;
			line_item = NULL;
		} else {
			need_line = 1;
			if (st->max_width<w) st->max_width = w;
		}
		wi++;
	}
	if (lineIndex->count) {
		if (w) {
			line_col = SFCOL_MAKE_ARGB(colors->vals[lineIndex->vals[li]]);
			need_line = 1;
		}
		li++;
	}
	if (fillIndex->count) {
		if (fillIndex->vals[fi]==-1) {
			fill_col = 0;
			fill_item = NULL;
		} else {
			fill_col = SFCOL_MAKE_ARGB(colors->vals[fillIndex->vals[fi]]);
			need_fill = 1;
		}
		fi++;
	}
	if (!coordIndex->count) return;
	cur = coords->vals[coordIndex->vals[ci]];
	ci++;
*/

	for (command=0; command<com->count; command++) {
		switch (com->vals[command]) {
		/*set line width*/
		case 0:
			if (wi >= widthIndex->count) return;
			w = (widthIndex->vals[wi]==-1) ? 0 : widths->vals[widthIndex->vals[wi]];
			if (!w) 
				line_item = NULL;
			else {
				need_line = 1;
				if (st->max_width<w) st->max_width = w;
			}
			wi++;
			break;
		/*set line color*/
		case 1:
			if (li > lineIndex->count) return;
			if (w) {
				line_col = SFCOL_MAKE_ARGB(colors->vals[lineIndex->vals[li]]);
				need_line = 1;
			}
			li++;
			break;
		/*set fill color*/
		case 2:
			if (fi >= fillIndex->count) return;
			if (fillIndex->vals[fi]==-1) {
				fill_col = 0;
				fill_item = NULL;
			} else {
				fill_col = SFCOL_MAKE_ARGB(colors->vals[fillIndex->vals[fi]]);
				need_fill = 1;
			}
			fi++;
			break;
		/*moveTo*/
		case 3:
			if ((has_ci && ci >= coordIndex->count) || (!has_ci && ci >= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				need_line = 0;
			}
			if (has_ci) {
				pt = pts[coordIndex->vals[ci]];
			} else {
				pt = pts[ci];
			}
			if (fill_item) m4_path_add_move_to(fill_item->path, pt.x, pt.y);
			if (line_item) m4_path_add_move_to(line_item->path, pt.x, pt.y);
			cur = pt;
			ci++;
			break;
		/*lineTo*/
		case 4:
			if ((has_ci && ci >= coordIndex->count) || (!has_ci && ci >= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				m4_path_add_move_to(fill_item->path, cur.x, cur.y);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				m4_path_add_move_to(line_item->path, cur.x, cur.y);
				need_line = 0;
			}
			if (has_ci) {
				pt = pts[coordIndex->vals[ci]];
			} else {
				pt = pts[ci];
			}
			if (fill_item) m4_path_add_line_to(fill_item->path, pt.x, pt.y);
			if (line_item) m4_path_add_line_to(line_item->path, pt.x, pt.y);
			cur = pt;
			ci++;
			break;
		/*cubic curveTo*/
		case 5:
			if ((has_ci && ci + 2 >= coordIndex->count) || (!has_ci && ci + 2>= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				m4_path_add_move_to(fill_item->path, cur.x, cur.y);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				m4_path_add_move_to(line_item->path, cur.x, cur.y);
				need_line = 0;
			}
			if (has_ci) {
				ct1 = pts[coordIndex->vals[ci]];
				ct2 = pts[coordIndex->vals[ci+1]];
				pt = pts[coordIndex->vals[ci+2]];
			} else {
				ct1 = pts[ci];
				ct2 = pts[ci+1];
				pt = pts[ci+2];
			}
			if (fill_item) m4_path_add_cubic_to(fill_item->path, ct1.x, ct1.y, ct2.x, ct2.y, pt.x, pt.y);
			if (line_item) m4_path_add_cubic_to(line_item->path, ct1.x, ct1.y, ct2.x, ct2.y, pt.x, pt.y);
			ct1 = ct2;
			cur = pt;
			ci += 3;
			break;
		/*cubic nextCurveTo*/
		case 6:
			if ((has_ci && ci + 1 >= coordIndex->count) || (!has_ci && ci + 1>= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				m4_path_add_move_to(fill_item->path, cur.x, cur.y);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				m4_path_add_move_to(line_item->path, cur.x, cur.y);
				need_line = 0;
			}
			ct1.x = 2*cur.x - ct1.x;
			ct1.y = 2*cur.y - ct1.y;
			if (has_ci) {
				ct2 = pts[coordIndex->vals[ci]];
				pt = pts[coordIndex->vals[ci+1]];
			} else {
				ct2 = pts[ci];
				pt = pts[ci+1];
			}
			if (fill_item) m4_path_add_cubic_to(fill_item->path, ct1.x, ct1.y, ct2.x, ct2.y, pt.x, pt.y);
			if (line_item) m4_path_add_cubic_to(line_item->path, ct1.x, ct1.y, ct2.x, ct2.y, pt.x, pt.y);
			ct1 = ct2;
			cur = pt;
			ci += 2;
			break;
		/*quadratic CurveTo*/
		case 7:
			if ((has_ci && ci + 1 >= coordIndex->count) || (!has_ci && ci + 1>= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				m4_path_add_move_to(fill_item->path, cur.x, cur.y);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				m4_path_add_move_to(line_item->path, cur.x, cur.y);
				need_line = 0;
			}
			if (has_ci) {
				ct1 = pts[coordIndex->vals[ci]];
				pt = pts[coordIndex->vals[ci+1]];
			} else {
				ct1 = pts[ci];
				pt = pts[ci+1];
			}
			if (fill_item) m4_path_add_quadratic_to(fill_item->path, ct1.x, ct1.y, pt.x, pt.y);
			if (line_item) m4_path_add_quadratic_to(line_item->path, ct1.x, ct1.y, pt.x, pt.y);
			ct1 = ct2;
			cur = pt;
			ci += 2;
			break;
		/*quadratic nextCurveTo*/
		case 8:
			if ((has_ci && ci >= coordIndex->count) || (!has_ci && ci >= coords->count) ) return;
			if (need_fill) {
				fill_item = new_fs_item(st, 0, fill_col, 0);
				m4_path_add_move_to(fill_item->path, cur.x, cur.y);
				need_fill = 0;
			}
			if (need_line) {
				line_item = new_fs_item(st, line_col, 0, w);
				m4_path_add_move_to(line_item->path, cur.x, cur.y);
				need_line = 0;
			}
			ct1.x = 2*cur.x - ct1.x;
			ct1.y = 2*cur.y - ct1.y;
			if (has_ci) {
				pt = pts[coordIndex->vals[ci]];
			} else {
				pt = pts[ci];
			}
			if (fill_item) m4_path_add_quadratic_to(fill_item->path, ct1.x, ct1.y, pt.x, pt.y);
			if (line_item) m4_path_add_quadratic_to(line_item->path, ct1.x, ct1.y, pt.x, pt.y);
			cur = pt;
			ci += 2;
			break;
		/*close*/
		case 9:
			if (fill_item) m4_path_close(fill_item->path);
			if (line_item) m4_path_close(line_item->path);
			break;
		}
	}
	
	/*compute bounds*/
	m4_rect_reset(&st->bounds);
	for (i=0; i<ChainGetCount(st->items); i++) {
		line_item = ChainGetEntry(st->items, i);
		m4_path_get_bounds(line_item->path, &rc);
		m4_rect_union(&st->bounds, &rc);
	}
}

static void FS_Render(SFNode *node, void *rs)
{
	Bool build;
	DrawableContext *ctx;
	FSStack *st = (FSStack *) Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	/*check for geometry change*/
	build = 0;
	if (Node_GetDirty(node)) {
		/*build*/
		clean_paths(st, st->graph->compositor->g_hw);
		build_shape(st, node);
		Node_ClearDirty(node);
		st->graph->node_changed = 1;
	}

	/*finalize*/
	ctx = drawable_init_context(st->graph, eff);
	if (!ctx) return;

	ctx->original = st->bounds;
	/*force width to maw width used for clipper compute*/
	if (st->max_width) {
		ctx->aspect.has_line = 1;
		ctx->aspect.pen_props.width = st->max_width;
	}
	drawable_finalize_render(ctx, eff);
}

static void FS_Draw(DrawableContext *ctx)
{
	u32 i;
	FSStack *st = (FSStack *) Node_GetPrivate((SFNode *) ctx->node->owner);

	for (i=0; i<ChainGetCount(st->items); i++) {
		FSItem *item = ChainGetEntry(st->items, i);
		memset(&ctx->aspect, 0, sizeof(DrawAspect2D));
		if (item->fill_col) {
			ctx->aspect.fill_color = item->fill_col;
			ctx->aspect.filled = 1;
		}
		if (item->width) {
			ctx->aspect.has_line = 1;
			ctx->aspect.line_color = item->line_col;
			ctx->aspect.pen_props.width = item->width;
		}
		VS2D_DrawPath(ctx->surface, item->path, ctx, NULL, NULL);
		ctx->path_filled = ctx->path_stroke = 0;
	}
}

/*to do*/
static Bool FS_PointOver(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	return 1;
}

void R2D_InitFlashShape(Render2D *sr, SFNode *node)
{
	u32 proto_ID;
	LPPROTO proto;
	FSStack *stack;
	proto = Node_GetProto(node);
	if (!proto) return;
	proto_ID = Proto_GetID(proto);
	if (proto_ID) return;

	stack = malloc(sizeof(FSStack));
	stack->graph = NewDrawableNode();
	stack->graph->Draw = FS_Draw;
	stack->graph->IsPointOver = FS_PointOver;
	stack->items = NewChain();

	traversable_setup(stack->graph, node, sr->compositor);
	Node_SetPrivate(node, stack);
	Node_SetRenderFunction(node, FS_Render);
	Node_SetPreDestroyFunction(node, FS_Destroy);
}

