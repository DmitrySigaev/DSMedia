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
#include "stacks2d.h"
#include "SVG/svg_stacks.h"

#define BOUNDSINFO_STEPALLOC		1

static Bool check_bounds_size(Drawable *node)
{
	u32 i;
	BoundsInfo **new_bounds;
	if (node->current_count < node->bounds_size) return 1;
	new_bounds = realloc(node->previous_bounds, sizeof(BoundsInfo *) * (node->bounds_size + BOUNDSINFO_STEPALLOC));
	if (!new_bounds) return 0;
	node->previous_bounds = new_bounds;
	new_bounds = realloc(node->current_bounds, sizeof(BoundsInfo *) * (node->bounds_size + BOUNDSINFO_STEPALLOC));
	if (!new_bounds) return 0;
	node->current_bounds = new_bounds;
	
	for (i=node->bounds_size; i<node->bounds_size + BOUNDSINFO_STEPALLOC; i++) {
		node->current_bounds[i] = malloc(sizeof(BoundsInfo));
		node->previous_bounds[i] = malloc(sizeof(BoundsInfo));
	}
	node->bounds_size += BOUNDSINFO_STEPALLOC;
	return 1;
}

static void bounds_remove_prev_item(Drawable *node, u32 pos)
{
	u32 i;
	BoundsInfo *bi;
	bi = node->previous_bounds[pos];
	for (i=pos; i<node->previous_count - 1; i++) 
		node->previous_bounds[i] = node->previous_bounds[i+1];

	node->previous_count--; 
	node->previous_bounds[node->previous_count] = bi;
}


/*default rendering routine*/
static void drawable_draw(DrawableContext *ctx) 
{
	VS2D_TexturePath(ctx->surface, ctx->node->path, ctx);
	VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
}

/*default point_over routine*/
static Bool drawable_point_over(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	M4Matrix2D inv;
	StrikeInfo2D *si;
	if (!ctx || !ctx->node->path) return 0;
	assert(ctx->surface);
	mx2d_copy(inv, ctx->transform);
	mx2d_inverse(&inv);
	mx2d_apply_coords(&inv, &x, &y);
	if (m4_path_point_over(ctx->node->path, x, y)) return 1;
	if (!check_outline) return 0;

	si = drawctx_get_strikeinfo(ctx, NULL);
	if (si && si->outline && m4_path_point_over(si->outline, x, y)) return 1;
	return 0;
}

/*get the orignal path without transform*/
void drawctx_store_original_bounds(struct _drawable_context *ctx)
{
	m4_path_get_bounds(ctx->node->path, &ctx->original);
}

Drawable *NewDrawableNode()
{
	Drawable *tmp;
	SAFEALLOC(tmp, sizeof(Drawable))
	tmp->on_surfaces = NewChain();
	tmp->path = m4_new_path();
	/*init with default*/
	tmp->Draw = drawable_draw;
	tmp->IsPointOver = drawable_point_over;
	tmp->strike_list = NewChain();

	/*alloc bounds storage*/
	check_bounds_size(tmp);	
	return tmp;
}

void drawable_reset_bounds(Drawable *dr)
{
	u32 i;
	/*destroy bounds storage*/
	for (i=0; i<dr->bounds_size; i++) {
		free(dr->current_bounds[i]);
		free(dr->previous_bounds[i]);
	}
	free(dr->current_bounds);
	free(dr->previous_bounds);
	dr->previous_count = dr->current_count = 0;
	dr->previous_bounds = dr->current_bounds = NULL;
	dr->bounds_size = 0;
}

void DeleteDrawableNode(Drawable *dr)
{
	u32 i;

	/*garbage collection*/
	for (i=0; i<dr->current_count; i++) {
		if (R2D_IsSurfaceRegistered((Render2D*)dr->compositor->visual_renderer->user_priv, dr->current_bounds[i]->surface)) 
			ra_add(&dr->current_bounds[i]->surface->to_redraw, dr->current_bounds[i]->clip);
	}
	for (i=0; i<dr->previous_count; i++) {
		if (R2D_IsSurfaceRegistered((Render2D*)dr->compositor->visual_renderer->user_priv, dr->previous_bounds[i]->surface)) 
			ra_add(&dr->previous_bounds[i]->surface->to_redraw, dr->previous_bounds[i]->clip);
	}


	drawable_reset_previous_bounds(dr);
	
	dr->compositor->draw_next_frame = 1;
	/*remove node from all surfaces it's on*/
	while (ChainGetCount(dr->on_surfaces)) {
		VisualSurface2D *surf = ChainGetEntry(dr->on_surfaces, 0);
		ChainDeleteEntry(dr->on_surfaces, 0);
		if (R2D_IsSurfaceRegistered((Render2D *)dr->compositor->visual_renderer->user_priv, surf)) 
			VS2D_DrawableDeleted(surf, dr);
	}
	DeleteChain(dr->on_surfaces);

	/*remove path object*/
	if (dr->path) m4_path_delete(dr->path);

	while (ChainGetCount(dr->strike_list)) {
		StrikeInfo2D *si = ChainGetEntry(dr->strike_list, 0);
		ChainDeleteEntry(dr->strike_list, 0);
		/*remove from main strike list*/
		ChainDeleteItem(((Render2D *)dr->compositor->visual_renderer->user_priv)->strike_bank, si);
		delete_strikeinfo2d(si);
	}
	DeleteChain(dr->strike_list);
	
	drawable_reset_bounds(dr);
	free(dr);
}

static void DestroyDrawableNode(SFNode *node)
{
	Drawable *ptr = Node_GetPrivate(node);
	DeleteDrawableNode(ptr);
}

Drawable *BaseDrawStack2D(Render2D *sr, SFNode *node)
{
	Drawable *stack = NewDrawableNode();
	traversable_setup(stack, node, sr->compositor);
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyDrawableNode);
	return stack;
}

/*store ctx bounds in current bounds*/
void drawable_store_bounds(struct _drawable_context *ctx)
{
	BoundsInfo *bi;
	Drawable *node = ctx->node;
	if (!check_bounds_size(ctx->node)) return;
	bi = node->current_bounds[node->current_count];
	node->current_count++;
	bi->appear = (M_Appearance *) ctx->appear;
	bi->clip = ctx->clip;
	bi->unclip = ctx->unclip_pix;
	bi->surface = ctx->surface;
}

/*move current bounds to previous bounds*/
void drawable_flush_bounds(Drawable *node)
{
	BoundsInfo **tmp;
	if (node->first_ctx_update) return;
	/*reset previous bounds on this surface*/
	tmp = node->previous_bounds;
	node->previous_bounds = node->current_bounds;
	node->previous_count = node->current_count;
	node->current_bounds = tmp;
	node->current_count = 0;
	node->first_ctx_update = 1;
	node->node_was_drawn = 0;
}	

/*
	return 1 if same bound is found in previous list (and remove it from the list)
	return 0 otherwise
*/
Bool drawable_has_same_bounds(struct _drawable_context *ctx)
{
	u32 i;
	Drawable *node = ctx->node;
	
	for(i=0; i<node->previous_count; i++) {
		BoundsInfo *bi = node->previous_bounds[i];
		if (bi->surface != ctx->surface) continue;
		if (bi->appear != (M_Appearance *) ctx->appear) continue;
		/*bounds shall match for unclip (large object pan) and clip */
		if (m4_irect_equal(bi->unclip, ctx->unclip_pix) && m4_irect_equal(bi->clip, ctx->clip)) {
			bounds_remove_prev_item(node, i);
			return 1;
		}
	}
	return 0;
}

/*
	return any previous bounds related to the same surface in @rc if any
	if nothing found return 0
*/
Bool drawable_get_previous_bound(Drawable *node, M4IRect *rc, struct _visual_surface_2D *surf)
{
	u32 i;
	for (i=0; i<node->previous_count; i++) {
		BoundsInfo *bi = node->previous_bounds[i];
		if (bi->surface != surf) continue;
		*rc = bi->clip;
		bounds_remove_prev_item(node, i);
		return 1;
	}
	return 0;
}

/*reset content of previous bounds list*/
void drawable_reset_previous_bounds(Drawable *node)
{
	node->previous_count = 0;
}


void drawable_register_on_surface(Drawable *node, struct _visual_surface_2D *surf)
{
	/*reset the draw state check in case the node is being rendered on several surfaces (in which case it
	may not be drawn on one but drawn on the other)*/
	node->first_ctx_update = 0;

	if (ChainFindEntry(node->on_surfaces, surf)>=0) return;
	ChainAddEntry(node->on_surfaces, surf);
	ChainAddEntry(surf->prev_nodes_drawn, node);
}
void drawable_unregister_from_surface(Drawable *node, struct _visual_surface_2D *surf)
{
	ChainDeleteItem(node->on_surfaces, surf);
	/*no longer registered, remove bounds*/
	if (!ChainGetCount(node->on_surfaces)) drawable_reset_bounds(node);
}



DrawableContext *NewDrawableContext()
{
	DrawableContext *tmp = malloc(sizeof(DrawableContext));
	memset(tmp, 0, sizeof(DrawableContext));
	tmp->sensors = NewChain();
	return tmp;
}
void DeleteDrawableContext(DrawableContext *ctx)
{
	drawctx_reset(ctx);
	if (ctx->sensors) DeleteChain(ctx->sensors);
	free(ctx);
}
void drawctx_reset(DrawableContext *ctx)
{
	Chain *bckup;
	drawctx_reset_sensors(ctx);
	bckup = ctx->sensors;
	memset(ctx, 0, sizeof(DrawableContext));
	ctx->sensors = bckup;
	cmat_init(&ctx->cmat);

	/*by default all nodes are transparent*/
	ctx->transparent = 1;

	/*BIFS has default value for 2D appearance ...*/
	ctx->aspect.fill_alpha  = 0xFF;
	ctx->aspect.fill_color = 0xFFCCCCCC;
	ctx->aspect.line_color = 0xFFCCCCCC;
	ctx->aspect.pen_props.width = 1.0;
	ctx->aspect.pen_props.cap = M4LineCapFlat;
	ctx->aspect.pen_props.join = M4LineJoinBevel;
	ctx->aspect.pen_props.miterLimit = 4.0;

}

void drawctx_reset_sensors(DrawableContext *ctx)
{
	while (ChainGetCount(ctx->sensors)) {
		SensorContext *ptr = ChainGetEntry(ctx->sensors, 0);
		ChainDeleteEntry(ctx->sensors, 0);
		free(ptr);
	}
}
static void drawctx_add_sensor(DrawableContext *ctx, SensorContext *handler)
{
	SensorContext *pNew = malloc(sizeof(SensorContext));
	pNew->h_node = handler->h_node;
	mx2d_copy(pNew->matrix, handler->matrix);
	ChainAddEntry(ctx->sensors, pNew);
}

void drawctx_update_info(DrawableContext *ctx)
{
	Bool moved, need_redraw;
	Drawable *node = ctx->node;
	need_redraw = ctx->redraw_flags ? 1 : 0;

	node->node_changed = 0;
	/*first update, remove all bounds*/
	if (node->first_ctx_update) {
		node->node_was_drawn = node->current_count; 
		node->first_ctx_update = 0;
	}
	/*only checked if node is not marked as dirty (otherwise we assume bounds have changed)*/
	if (! (ctx->redraw_flags & CTX_NODE_DIRTY) ) {
		moved = !drawable_has_same_bounds(ctx);
		if (!need_redraw) need_redraw = moved;
	}
	ctx->redraw_flags = need_redraw;

	/*in all cases reset dirty flag of appearance and its sub-nodes*/
	Node_ClearDirtyChildren(ctx->appear, 0);
}


static void setup_drawable_context(DrawableContext *ctx, RenderEffect2D *eff)
{
	M_Material2D *m;
	M_LineProperties *LP;
	M_XLineProperties *XLP;

	if (ctx->appear == NULL) return;
	m = (M_Material2D *) ((M_Appearance *)ctx->appear)->material;
	if ( m == NULL) return;
	if (Node_GetTag((SFNode *) m) != TAG_MPEG4_Material2D) return;

	/*store alpha*/
	ctx->aspect.fill_alpha = (u8) (255 * (1.0f - m->transparency));
	ctx->aspect.fill_color = MAKE_ARGB_FLOAT((1-m->transparency), m->emissiveColor.red, m->emissiveColor.green, m->emissiveColor.blue);
	ctx->aspect.fill_color = cmat_apply(&ctx->cmat, ctx->aspect.fill_color);

	ctx->aspect.line_color = ctx->aspect.fill_color;
	ctx->aspect.filled = m->filled;

	ctx->aspect.pen_props.cap = M4LineCapFlat;
	ctx->aspect.pen_props.join = M4LineJoinMiter;


	if (m->lineProps == NULL) {
		/*this is a bug in the spec: by default line width is 1.0, but in meterMetrics this means half of the screen :)*/
		ctx->aspect.pen_props.width = 1;
		if (!eff->is_pixel_metrics) ctx->aspect.pen_props.width /= eff->min_hsize;
		if (ctx->aspect.filled) {
			switch (Node_GetTag(ctx->node->owner)) {
			case TAG_MPEG4_IndexedLineSet2D:
			case TAG_MPEG4_PointSet2D:
				break;
			default:
				ctx->aspect.pen_props.width = 0;
				break;
			}
		} else if (m->transparency==1.0) {
			ctx->aspect.pen_props.width = 0;
		}
		return;
	}
	LP = NULL;
	XLP = NULL;
	switch (Node_GetTag((SFNode *) m->lineProps) ) {
	case TAG_MPEG4_LineProperties:
		LP = (M_LineProperties *) m->lineProps;
		break;
	case TAG_MPEG4_XLineProperties:
		XLP = (M_XLineProperties *) m->lineProps;
		break;
	default:
		return;
	}
	ctx->aspect.has_line = 1;

	if (LP) {
		ctx->aspect.pen_props.dash = LP->lineStyle;
		ctx->aspect.line_color = MAKE_ARGB_FLOAT((1-m->transparency), LP->lineColor.red, LP->lineColor.green, LP->lineColor.blue);
		ctx->aspect.pen_props.width = LP->width;
		ctx->aspect.line_color = cmat_apply(&ctx->cmat, ctx->aspect.line_color);
		return;
	} 

	ctx->aspect.pen_props.dash = LP ? LP->lineStyle : XLP->lineStyle;
	ctx->aspect.line_color = MAKE_ARGB_FLOAT((1-XLP->transparency), XLP->lineColor.red, XLP->lineColor.green, XLP->lineColor.blue);
	ctx->aspect.pen_props.width = XLP->width;
	ctx->aspect.line_color = cmat_apply(&ctx->cmat, ctx->aspect.line_color);
	
	ctx->aspect.pen_props.is_scalable = XLP->isScalable;
	ctx->aspect.pen_props.align = XLP->isCenterAligned ? M4LineCentered : M4LineInside;
	ctx->aspect.pen_props.cap = XLP->lineCap;
	ctx->aspect.pen_props.join = XLP->lineJoin;
	ctx->aspect.pen_props.miterLimit = XLP->miterLimit;
	ctx->aspect.pen_props.dash_offset = XLP->dashOffset;

	/*dash settings strutc is the same as MFFloat from XLP, typecast without storing*/
	if (XLP->dashes.count) {
		ctx->aspect.pen_props.dash_set = (M4DashSettings *) &XLP->dashes;
	} else {
		ctx->aspect.pen_props.dash_set = NULL;
	}
	ctx->aspect.line_texture = R2D_GetTextureHandler(XLP->texture);
}

static Bool check_transparent_skip(DrawableContext *ctx, Bool skipFill)
{
	/*if sensor cannot skip*/
	if (ChainGetCount(ctx->sensors)) return 0;
	/*if texture, cannot skip*/
	if (ctx->h_texture) return 0;
	if (! M4C_A(ctx->aspect.fill_color) && !M4C_A(ctx->aspect.line_color) ) return 1;
	if (ctx->aspect.pen_props.width == 0) {
		if (skipFill) return 1;
		if (!ctx->aspect.filled) return 1;
	}
	return 0;
}


TextureHandler *drawable_get_texture(RenderEffect2D *eff)
{
	M_Appearance *appear = (M_Appearance *) eff->appear;
	if (!appear || !appear->texture) return NULL;
	return R2D_GetTextureHandler(appear->texture);
}

DrawableContext *drawable_init_context(Drawable *node, RenderEffect2D *eff)
{
	DrawableContext *ctx;
	u32 i;
	Bool skipFill;
	assert(eff->surface);

	/*switched-off geometry nodes are not rendered*/
	if (eff->trav_flags & TF_SWITCHED_OFF) return NULL;

	//Get a empty context from the current surface
	ctx = VS2D_GetDrawableContext(eff->surface);

	mx2d_copy(ctx->transform, eff->transform);
	ctx->node = node;
	if (eff->invalidate_all || node->node_changed) 
		ctx->redraw_flags |= CTX_NODE_DIRTY;

	ctx->h_texture = NULL;
	if (eff->appear) {
		ctx->appear = eff->appear;
		if (Node_GetDirty(eff->appear)) ctx->redraw_flags |= CTX_APP_DIRTY;
	}
#ifndef FIXME
	/*todo cliper*/
#else
	else {
		VS2D_RemoveLastContext(eff->surface);
		return NULL;
	}
#endif

	/*FIXME - only needed for texture*/
	cmat_copy(&ctx->cmat, &eff->color_mat);

	/*IndexedLineSet2D and PointSet2D ignores fill flag and texturing*/
	skipFill = 0;
	ctx->h_texture = NULL;
	switch (Node_GetTag(ctx->node->owner) ) {
	case TAG_MPEG4_IndexedLineSet2D: skipFill = 1; break;
	case TAG_MPEG4_PointSet2D: skipFill = 1; break;
	default:
		ctx->h_texture = drawable_get_texture(eff);
		break;
	}
	
	/*setup sensors*/
	for (i=0; i<ChainGetCount(eff->sensors); i++) 
		drawctx_add_sensor(ctx, ChainGetEntry(eff->sensors, i));

	setup_drawable_context(ctx, eff);


	/*Update texture info - draw even if texture not created (this may happen if the media is removed)*/
	if (ctx->h_texture && ctx->h_texture->needs_refresh) ctx->redraw_flags |= CTX_TEXTURE_DIRTY;

	/*not clear in the spec: what happens when a transparent node is in form/layout ?? this may 
	completely break layout of children. We consider the node should be drawn*/
	if (!eff->parent && check_transparent_skip(ctx, skipFill)) {
		VS2D_RemoveLastContext(eff->surface);
		return NULL;
	}

	//setup clipper if needed

	return ctx;
}

void drawable_finalize_render(struct _drawable_context *ctx, RenderEffect2D *eff)
{
	Float sx, sy, pw;

	ctx->unclip = ctx->original;
	mx2d_apply_rect(&eff->transform, &ctx->unclip);

	/*apply pen width*/
	if (ctx->aspect.has_line && ctx->aspect.pen_props.width && (ctx->aspect.pen_props.align==M4LineCentered) ) {
		StrikeInfo2D *si = NULL;

		/*if pen is scalable, the width is scaled by max(scale_x, scale_y) of the current transorm
		otherwise apply user/viewport transform so that original aspect is kept*/
		if (ctx->aspect.pen_props.is_scalable) {
			sx = eff->transform.m[0];
			sy = eff->transform.m[4];
		} else {
			sx = eff->surface->top_transform.m[0];
			sy = eff->surface->top_transform.m[4];
		}
		if (sx<0) sx *= -1;
		if (sy<0) sy *= -1;
		if (sy>sx) sx = sy;
		ctx->aspect.line_scale = sx;

		/*get strike info & outline for exact bounds compute. If failure use default offset*/
		si = drawctx_get_strikeinfo(ctx, ctx->node->path);
		if (si && si->outline) {
			m4_path_get_bounds(si->outline, &ctx->unclip);
			mx2d_apply_rect(&eff->transform, &ctx->unclip);
		} else {
			pw = ctx->aspect.pen_props.width;
			pw *= sx;
			ctx->unclip.x -= pw;
			ctx->unclip.y += pw;
			ctx->unclip.width += 2*pw;
			ctx->unclip.height += 2*pw;
		}
	}

	if (!ctx->no_antialias) {
		/*grow of 2 pixels (-1 and +1) to handle AA*/
		pw = (eff->is_pixel_metrics) ? 1 : (2.0f/eff->surface->width);
		ctx->unclip.x -= pw;
		ctx->unclip.y += pw;
		ctx->unclip.width += 2*pw;
		ctx->unclip.height += 2*pw;
	}

	ctx->clip = m4_rect_pixelize(&ctx->unclip);
	ctx->unclip_pix = ctx->clip;

	if (eff->parent) {
		group2d_add_to_context_list(eff->parent, ctx);
	} else if (eff->trav_flags & TF_RENDER_DIRECT) {
		ctx->node->Draw(ctx);
	}
}


void delete_strikeinfo2d(StrikeInfo2D *info)
{
	if (info->outline) m4_path_delete(info->outline);
	free(info);
}

StrikeInfo2D *drawctx_get_strikeinfo(DrawableContext *ctx, LPM4PATH path)
{
	StrikeInfo2D *si;
	SFNode *lp;
	u32 now, i;
	if (ctx->appear && !ctx->aspect.pen_props.width) return NULL;

	lp = NULL;
	if (ctx->appear) {
		lp = ((M_Appearance *)ctx->appear)->material;
		if (lp) lp = ((M_Material2D *) lp)->lineProps;
	}

	si = NULL;
	for (i=0; i<ChainGetCount(ctx->node->strike_list); i++) {
		si = ChainGetEntry(ctx->node->strike_list, i);
		/*note this includes default LP (NULL)*/
		if ((si->lineProps == lp) && (!path || (path==si->original)) ) break;
		si = NULL;
	}
	/*not found, add*/
	if (!si) {
		si = malloc(sizeof(StrikeInfo2D));
		memset(si, 0, sizeof(StrikeInfo2D));
		si->lineProps = lp;
		si->node = ctx->node->owner;
		ChainAddEntry(ctx->node->strike_list, si);
		ChainAddEntry(((Render2D *)ctx->node->compositor->visual_renderer->user_priv)->strike_bank, si);
	}

	/*node changed or outline not build*/
	now = lp ? R2D_LP_GetLastUpdateTime(lp) : si->last_update_time;
	if (!si->outline || (now!=si->last_update_time) || (si->line_scale != ctx->aspect.line_scale)) {
		u32 i;
		Float w = ctx->aspect.pen_props.width;
		si->last_update_time = now;
		si->line_scale = ctx->aspect.line_scale;
		if (si->outline) m4_path_delete(si->outline);
		if (!ctx->aspect.pen_props.is_scalable) {
			ctx->aspect.pen_props.width/=ctx->aspect.line_scale;
			ctx->aspect.pen_props.dash_offset *= ctx->aspect.line_scale;
			if (ctx->aspect.pen_props.dash_set) {
				for(i=0; i<ctx->aspect.pen_props.dash_set->num_dash; i++) {
					ctx->aspect.pen_props.dash_set->dashes[i] *= ctx->aspect.line_scale;
				}
			}
		}
		if (path) {
			si->outline = m4_path_get_outline(path, ctx->aspect.pen_props);
			si->original = path;
		} else {
			si->outline = m4_path_get_outline(ctx->node->path, ctx->aspect.pen_props);
		}
		ctx->aspect.pen_props.width = w;
		if (!ctx->aspect.pen_props.is_scalable && ctx->aspect.pen_props.dash_set) {
			for(i=0; i<ctx->aspect.pen_props.dash_set->num_dash; i++) {
				ctx->aspect.pen_props.dash_set->dashes[i] /= ctx->aspect.line_scale;
			}
		}
	}
	return si;
}

void drawable_reset_path(Drawable *st)
{
	u32 i;
	for (i=0; i<ChainGetCount(st->strike_list); i++) {
		StrikeInfo2D *si = ChainGetEntry(st->strike_list, i);
		if (si->outline) m4_path_delete(si->outline);
		si->outline = NULL;
		si->original = NULL;
	}
	if (st->path) m4_path_reset(st->path);
}


void R2D_LinePropsRemoved(Render2D *sr, SFNode *n)
{
	u32 i;
	for (i=0; i<ChainGetCount(sr->strike_bank); i++) {
		StrikeInfo2D *si = ChainGetEntry(sr->strike_bank, i);
		if (si->lineProps == n) {
			/*remove from node*/
			if (si->node) {
				s32 res;
				Drawable *st = (Drawable *) Node_GetPrivate(si->node);
				/*yerk this is ugly, but text is not a regular drawable and adding a fct just to get
				the drawable would be just pain*/
				if (Node_GetTag(si->node)==TAG_MPEG4_Text) st = ((TextStack2D*)st)->graph;
				assert(st && st->strike_list);
				res = ChainDeleteItem(st->strike_list, si);
				assert(res>=0);
			}
			ChainDeleteEntry(sr->strike_bank, i);
			delete_strikeinfo2d(si);
			i--;
		}
	}
}

static void DestroyLineProps(SFNode *n)
{
	LinePropStack *st = Node_GetPrivate(n);
	R2D_LinePropsRemoved(st->sr, n);
	free(st);
}
void R2D_InitLineProps(Render2D *sr, SFNode *node)
{
	LinePropStack *st = malloc(sizeof(LinePropStack));
	st->sr = sr;
	st->last_mod_time = 1;
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyLineProps);
}

u32 R2D_LP_GetLastUpdateTime(SFNode *node)
{
	LinePropStack *st = Node_GetPrivate(node);
	if (!st) return 0;
	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		st->last_mod_time ++;
		Node_ClearDirty(node);
	}
	return st->last_mod_time;
}


#ifdef M4_USE_LASeR

static void setup_LASeR_drawable_context(DrawableContext *ctx)
{
	LASeRNode *node = (LASeRNode *)ctx->node->owner;

	if (Node_GetTag((SFNode *)node) == TAG_LASeRShape) {
	
		LASeRShape *shape = (LASeRShape *)node;

		ctx->aspect.fill_alpha = 255;
		ctx->aspect.filled = shape->filled;
		if (shape->filled) {
			ctx->aspect.fill_color = MAKE_ARGB_FLOAT(1, shape->fill.r, shape->fill.g, shape->fill.b);
			ctx->aspect.fill_color = cmat_apply(&ctx->cmat, ctx->aspect.fill_color);
			if (shape->stroked) {
				ctx->aspect.line_color = MAKE_ARGB_FLOAT(1, shape->linecolor.r, shape->linecolor.g, shape->linecolor.b);
			} else {
				ctx->aspect.line_color = MAKE_ARGB_FLOAT(1, shape->fill.r, shape->fill.g, shape->fill.b);
			}
		} else {
			ctx->aspect.fill_color = MAKE_ARGB_FLOAT(1, 0, 0, 0);
			if (shape->stroked) ctx->aspect.line_color = MAKE_ARGB_FLOAT(1, shape->linecolor.r, shape->linecolor.g, shape->linecolor.b);
			else ctx->aspect.line_color = MAKE_ARGB_FLOAT(1, 0, 0, 0);
		}
		ctx->aspect.pen_props.is_scalable = 0;
		ctx->aspect.pen_props.cap = M4LineCapFlat;
		ctx->aspect.pen_props.join = M4LineJoinMiter;
		ctx->aspect.pen_props.width = shape->width;
		ctx->aspect.has_line = (shape->width ? 1 : 0);

	} else if (Node_GetTag((SFNode *)node) == TAG_LASeRText) {
		LASeRText *text = (LASeRText *)node;

		if (text->filled) ctx->aspect.fill_color = MAKE_ARGB_FLOAT(1, text->color.r, text->color.g, text->color.b);
		else ctx->aspect.fill_color = MAKE_ARGB_FLOAT(1, 0, 0, 0);
		ctx->aspect.fill_color = cmat_apply(&ctx->cmat, ctx->aspect.fill_color);

		ctx->aspect.filled = 1;

		ctx->aspect.pen_props.cap = M4LineCapFlat;
		ctx->aspect.pen_props.join = M4LineJoinMiter;


		ctx->aspect.pen_props.width = 0;
		ctx->aspect.has_line = 0;
	} else {
		return;
	}
}

DrawableContext *LASeR_drawable_init_context(Drawable *node, RenderEffect2D *eff)
{
	DrawableContext *ctx;
	u32 i;
	Bool skipFill = 0;
	assert(eff->surface);

	if (!Node_GetActive(node->owner)) return NULL;

	/*switched-off geometry nodes are not rendered*/
	if (eff->trav_flags & TF_SWITCHED_OFF) return NULL;
	
	//Get a empty context from the current surface
	ctx = VS2D_GetDrawableContext(eff->surface);

	mx2d_copy(ctx->transform, eff->transform);
	if (Node_GetTag(node->owner) == TAG_LASeRText) {
		ctx->transform.m[0] = 1; ctx->transform.m[4] = 1;
		ctx->transform.m[1] = 0; ctx->transform.m[3] = 0;
	} 
	ctx->node = node;
	if (eff->invalidate_all || node->node_changed) ctx->redraw_flags |= CTX_NODE_DIRTY;

	ctx->h_texture = NULL;

/*
	if (eff->appear) {
		ctx->appear = eff->appear;
		if (Node_GetDirty(eff->appear)) ctx->redraw_flags |= CTX_APP_DIRTY;
	}
*/
#ifndef FIXME
	/*todo cliper*/
#else
	else {
		VS2D_RemoveLastContext(eff->surface);
		return NULL;
	}
#endif

	/*FIXME - only needed for texture*/
	cmat_copy(&ctx->cmat, &eff->color_mat);
	
	/*setup sensors*/
	for (i=0; i<ChainGetCount(eff->sensors); i++) 
		drawctx_add_sensor(ctx, ChainGetEntry(eff->sensors, i));

	setup_LASeR_drawable_context(ctx);


	/*Update texture info - draw even if texture not created (this may happen if the media is removed)*/
	if (ctx->h_texture && ctx->h_texture->needs_refresh) ctx->redraw_flags |= CTX_TEXTURE_DIRTY;

	if (check_transparent_skip(ctx, skipFill)) {
		VS2D_RemoveLastContext(eff->surface);
		return NULL;
	}

	//setup clipper if needed

	return ctx;
}

#endif	//LASeR

#ifndef M4_DISABLE_SVG

static void setup_SVG_drawable_context(DrawableContext *ctx, SVGStylingProperties props)
{
	ctx->aspect.fill_alpha = 255;
	ctx->aspect.filled = (props.fill->paintType != SVG_PAINTTYPE_NONE);
	ctx->aspect.fill_color = MAKE_ARGB_FLOAT(props.fill_opacity->value, props.fill->color.red, props.fill->color.green, props.fill->color.blue);
	ctx->aspect.has_line = (props.stroke->paintType != SVG_PAINTTYPE_NONE);
	ctx->aspect.line_color = MAKE_ARGB_FLOAT(props.stroke_opacity->value, props.stroke->color.red, props.stroke->color.green, props.stroke->color.blue);
	ctx->aspect.pen_props.is_scalable = 1;
	ctx->aspect.pen_props.cap = *props.stroke_linecap;
	ctx->aspect.pen_props.join = *props.stroke_linejoin;
	ctx->aspect.pen_props.width = (ctx->aspect.has_line?props.stroke_width->number:0);
}

DrawableContext *SVG_drawable_init_context(Drawable *node, RenderEffect2D *eff)
{
	DrawableContext *ctx;
	u32 i;
	Bool skipFill = 0;
	assert(eff->surface);

	/*switched-off geometry nodes are not rendered*/
	if (eff->trav_flags & TF_SWITCHED_OFF) return NULL;
	
	//Get a empty context from the current surface
	ctx = VS2D_GetDrawableContext(eff->surface);

	mx2d_copy(ctx->transform, eff->transform);

	ctx->node = node;
	if (eff->invalidate_all || node->node_changed) ctx->redraw_flags |= CTX_NODE_DIRTY;

	ctx->h_texture = NULL;

	/*FIXME - only needed for texture*/
	cmat_copy(&ctx->cmat, &eff->color_mat);
	
	switch (Node_GetTag(ctx->node->owner) ) {
	case TAG_SVG_image:
		{
			SVG_image_stack *st = (SVG_image_stack*) Node_GetPrivate(ctx->node->owner);
			ctx->h_texture = &(st->txh);
		}
		break;
	case TAG_SVG_video:
		{
			SVG_video_stack *st = (SVG_video_stack*) Node_GetPrivate(ctx->node->owner);
			ctx->h_texture = &(st->txh);
		}
		break;
	default:
		break;
	}

	/*setup sensors*/
	for (i=0; i<ChainGetCount(eff->sensors); i++) 
		drawctx_add_sensor(ctx, ChainGetEntry(eff->sensors, i));

	setup_SVG_drawable_context(ctx, *(eff->svg_props));

	/*Update texture info - draw even if texture not created (this may happen if the media is removed)*/
	if (ctx->h_texture && ctx->h_texture->needs_refresh) ctx->redraw_flags |= CTX_TEXTURE_DIRTY;

	if (check_transparent_skip(ctx, skipFill)) {
		VS2D_RemoveLastContext(eff->surface);
		return NULL;
	}

	return ctx;
}

#endif	//SVG
