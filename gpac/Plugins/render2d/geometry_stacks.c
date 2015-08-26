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


/*
		Shape 
*/
static void RenderShape(SFNode *node, void *rs)
{
	RenderEffect2D *eff;
	M_Shape *shape = (M_Shape *) node;
	if (!shape->geometry) return;
	eff = rs;
	eff->appear = (SFNode *) shape->appearance;

	Node_Render((SFNode *) shape->geometry, eff);
	eff->appear = NULL;
}

void R2D_InitShape(Render2D *sr, SFNode *node)
{
	Node_SetRenderFunction(node, RenderShape);
}


static void RenderCircle(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		m4_path_add_ellipse(cs->path, ((M_Circle *) node)->radius * 2, ((M_Circle *) node)->radius * 2);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void R2D_InitCircle(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, RenderCircle);
}

static void RenderEllipse(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		m4_path_add_ellipse(cs->path, ((M_Ellipse *) node)->radius.x, ((M_Ellipse *) node)->radius.y);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void R2D_InitEllipse(Render2D  *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, RenderEllipse);
}

static void RenderRectangle(SFNode *node, void *reff)
{
	DrawableContext *ctx;
	Drawable *rs = Node_GetPrivate(node);
	RenderEffect2D *eff = reff;

	if (Node_GetDirty(node)) {
		drawable_reset_path(rs);
		m4_path_add_rectangle(rs->path, 0, 0, ((M_Rectangle *) node)->size.x, ((M_Rectangle *) node)->size.y);
		Node_ClearDirty(node);
		rs->node_changed = 1;
	}
	ctx = drawable_init_context(rs, eff);
	if (!ctx) return;
	
	ctx->transparent = 0;
	/*if not filled, transparent*/
	if (!ctx->aspect.filled) {
		ctx->transparent = 1;
	} 
	/*if alpha, transparent*/
	else if (M4C_A(ctx->aspect.fill_color) != 0xFF) {
		ctx->transparent = 1;
	} 
	/*if rotated, transparent (doesn't fill bounds)*/
	else if (ctx->transform.m[1] || ctx->transform.m[3]) {
		ctx->transparent = 1;
	}
	else if (!eff->color_mat.identity) 
		ctx->transparent = 1;	

	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void R2D_InitRectangle(Render2D  *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, RenderRectangle);
}


//#define CHECK_VALID_C2D(nbPts) if (cur_index+nbPts>=pt_count) { gd->path_reset(cs->path); return; }
#define CHECK_VALID_C2D(nbPts)

static void build_curve2D(Drawable *cs, M_Curve2D *c2D)
{
	SFVec2f orig, ct_orig, ct_end, end;
	u32 cur_index, i, remain, type_count, pt_count;
	SFVec2f *pts;
	M_Coordinate2D *coord = (M_Coordinate2D *)c2D->point;

	pts = coord->point.vals;
	if (!pts) 
		return;

	cur_index = c2D->type.count ? 1 : 0;
	/*if the first type is a moveTo skip initial moveTo*/
	i=0;
	i=0;
	if (cur_index) {
		while (c2D->type.vals[i]==0) i++;
	}
	ct_orig = orig = pts[i];

	m4_path_add_move_to(cs->path, orig.x, orig.y);

	pt_count = coord->point.count;
	type_count = c2D->type.count;
	for (; i<type_count; i++) {

		switch (c2D->type.vals[i]) {
		/*moveTo, 1 point*/
		case 0:
			CHECK_VALID_C2D(0);
			orig = pts[cur_index];
			if (i) m4_path_add_move_to(cs->path, orig.x, orig.y);
			cur_index += 1;
			break;
		/*lineTo, 1 point*/
		case 1:
			CHECK_VALID_C2D(0);
			end = pts[cur_index];
			m4_path_add_line_to(cs->path, end.x, end.y);
			orig = end;
			cur_index += 1;
			break;
		/*curveTo, 3 points*/
		case 2:
			CHECK_VALID_C2D(2);
			ct_orig = pts[cur_index];
			ct_end = pts[cur_index+1];
			end = pts[cur_index+2];
			m4_path_add_cubic_to(cs->path, ct_orig.x, ct_orig.y, ct_end.x, ct_end.y, end.x, end.y);
			cur_index += 3;
			ct_orig = ct_end;
			orig = end;
			break;
		/*nextCurveTo, 2 points (cf spec)*/
		case 3:
			CHECK_VALID_C2D(1);
			ct_orig.x = 2*orig.x - ct_orig.x;
			ct_orig.y = 2*orig.y - ct_orig.y;
			ct_end = pts[cur_index];
			end = pts[cur_index+1];
			m4_path_add_cubic_to(cs->path, ct_orig.x, ct_orig.y, ct_end.x, ct_end.y, end.x, end.y);
			cur_index += 2;
			ct_orig = ct_end;
			orig = end;
			break;
		
		/*all XCurve2D specific*/

		/*CW and CCW ArcTo*/
		case 4:
		case 5:
			CHECK_VALID_C2D(2);
			ct_orig = pts[cur_index];
			ct_end = pts[cur_index+1];
			end = pts[cur_index+2];
			m4_path_add_arc_to(cs->path, end.x, end.y, ct_orig.x, ct_orig.y, ct_end.x, ct_end.y, (c2D->type.vals[i]==5) ? 1 : 0);
			cur_index += 3;
			ct_orig = ct_end;
			orig = end;
			break;
		/*ClosePath*/
		case 6:
			m4_path_close(cs->path);
			break;
		/*quadratic CurveTo, 2 points*/
		case 7:
			CHECK_VALID_C2D(1);
			ct_end = pts[cur_index];
			end = pts[cur_index+1];
			m4_path_add_quadratic_to(cs->path, ct_end.x, ct_end.y, end.x, end.y);
			cur_index += 2;
			ct_orig = ct_end;
			orig = end;
			break;
		}
		/*done*/
		if (cur_index>=pt_count) return;
	}

	/*what's left is an N-bezier spline*/
	if (pt_count > cur_index) {
		/*first moveto*/
		if (!cur_index) cur_index++;

		remain = pt_count - cur_index;

		if (remain>1)
			m4_path_add_bezier(cs->path, (M4Point2D *) &pts[cur_index], remain);
	}
}

static void RenderCurve2D(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	M_Curve2D *c2D = (M_Curve2D *)node;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (!c2D->point) return;


	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		cs->path->resolution = eff->surface->render->compositor->base_curve_resolution;
		cs->path->fineness = c2D->fineness;
		if (eff->surface->render->compositor->high_speed)  cs->path->fineness /= 2;
		build_curve2D(cs, c2D);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}

	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

void R2D_InitCurve2D(Render2D  *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, RenderCurve2D);
}


typedef struct _bitmap_stack
{
	/*rendering node*/
	Drawable *graph;
} BitmapStack;

static void DestroyBitmap(SFNode *n)
{
	BitmapStack *st = (BitmapStack *)Node_GetPrivate(n);

	DeleteDrawableNode(st->graph);

	/*destroy hw surface if any*/

	free(st);
}

static void Bitmap_BuildGraph(BitmapStack *st, DrawableContext *ctx, RenderEffect2D *eff)
{
	M_Bitmap *bmp = (M_Bitmap *)st->graph->owner;
	M4Matrix2D mat;
	Float w , h;

	w = ctx->h_texture->active_window.width;
	h = ctx->h_texture->active_window.height;

	/*get size with scale*/
	drawable_reset_path(st->graph);
	/*reverse meterMetrics conversion*/
	mx2d_init(mat);
	if (!eff->is_pixel_metrics) mx2d_add_scale(&mat, eff->min_hsize, eff->min_hsize);
	mx2d_inverse(&mat);
	/*the spec says STRICTLY positive or -1, but some content use 0...*/
	mx2d_add_scale(&mat, (bmp->scale.x>=0) ? bmp->scale.x : 1, (bmp->scale.y>=0) ? bmp->scale.y : 1);
	mx2d_apply_coords(&mat, &w, &h);
	m4_path_add_rectangle(st->graph->path, 0, 0, w, h);

	m4_rect_center(&ctx->original, w, h);
}

static void RenderBitmap(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	BitmapStack *st = (BitmapStack *)Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	/*we never cache anything with bitmap...*/
	Node_ClearDirty(node);

	ctx = drawable_init_context(st->graph, eff);
	if (!ctx || !ctx->h_texture ) return;
	/*always build the path*/
	Bitmap_BuildGraph(st, ctx, eff);
	/*even if set this is not true*/
	ctx->aspect.has_line = 0;
	/*this is to make sure we don't fill the path if the texture is transparent*/
	ctx->aspect.filled = 0;
	ctx->aspect.pen_props.width = 0.0;
	ctx->no_antialias = 1;

	ctx->transparent = 0;
	/*if clipper then transparent*/

	if (ctx->h_texture->transparent) {
		ctx->transparent = 1;
	} else {
		M_Appearance *app = (M_Appearance *)ctx->appear;
		if ( app->material && (Node_GetTag((SFNode *)app->material)==TAG_MPEG4_MaterialKey) ) {
			if (((M_MaterialKey*)app->material)->isKeyed) ctx->transparent = 1;
		} else if (!eff->color_mat.identity) ctx->transparent = 1;
	}

	/*bounds are stored when building graph*/	
	drawable_finalize_render(ctx, eff);
}

static void DrawBitmap(DrawableContext *ctx)
{
	u8 alpha;
	Render2D *sr;
	Bool use_hw, has_scale;
	M_Bitmap *bmp = (M_Bitmap *) ctx->node->owner;
	BitmapStack *st = (BitmapStack *) Node_GetPrivate(ctx->node->owner);


	sr = ctx->surface->render;
	/*bitmaps are NEVER rotated (forbidden in spec). In case a rotation was done we still display (reset the skew components)*/
	ctx->transform.m[1] = ctx->transform.m[3] = 0;

	has_scale = 0;
	if (bmp->scale.x>0 && bmp->scale.x!=1) has_scale = 1;
	if (bmp->scale.y>0 && bmp->scale.y!=1) has_scale = 1;

	use_hw = 1;
	alpha = M4C_A(ctx->aspect.fill_color);

	/*check if driver can handle alpha blit*/
	if (alpha!=255) {
		use_hw = sr->compositor->video_out->bHasAlpha;
		if (has_scale) use_hw = sr->compositor->video_out->bHasAlphaStretch;
	}
	/*for MatteTexture*/
	if (!ctx->cmat.identity || ctx->h_texture->has_cmat) use_hw = 0;

	/*to do - materialKey*/

	/*this is not a native texture, use graphics*/
	if (!ctx->h_texture->data) {
		use_hw = 0;
	} else {
		if (!ctx->surface->SupportsFormat || !ctx->surface->DrawBitmap ) use_hw = 0;
		/*format not supported directly, try with brush*/
		else if (!ctx->surface->SupportsFormat(ctx->surface, ctx->h_texture->pixelformat) ) use_hw = 0;
	}

	/*no HW, fall back to the graphics driver*/
	if (!use_hw) {
		drawable_reset_path(st->graph);
		m4_path_add_rectangle(st->graph->path, 0, 0, ctx->original.width, ctx->original.height);
		ctx->no_antialias = 1;
		VS2D_TexturePath(ctx->surface, st->graph->path, ctx);
		return;
	}

	/*direct rendering, render without clippers */
	if (ctx->surface->render->top_effect->trav_flags & TF_RENDER_DIRECT) {
		ctx->surface->DrawBitmap(ctx->surface, ctx->h_texture, &ctx->clip, &ctx->unclip);
	}
	/*render bitmap for all dirty rects*/
	else {
		u32 i;
		M4IRect clip;
		for (i=0; i<ctx->surface->to_redraw.count; i++) {
			/*there's an opaque region above, don't draw*/
			if (ctx->surface->draw_node_index<ctx->surface->to_redraw.opaque_node_index[i]) continue;
			clip = ctx->clip;
			m4_irect_intersect(&clip, &ctx->surface->to_redraw.list[i]);
			if (clip.width && clip.height) {
				ctx->surface->DrawBitmap(ctx->surface, ctx->h_texture, &clip, &ctx->unclip);
			}
		}
	}

}

static Bool Bitmap_PointOver(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	return 1;
}

void R2D_InitBitmap(Render2D  *sr, SFNode *node)
{
	BitmapStack *st = malloc(sizeof(BitmapStack));
	st->graph = NewDrawableNode();

	traversable_setup(st->graph, node, sr->compositor);
	st->graph->Draw = DrawBitmap;
	st->graph->IsPointOver = Bitmap_PointOver;
	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, RenderBitmap);
	Node_SetPreDestroyFunction(node, DestroyBitmap);
}

/*
	Note on point set 2D: this is a very bad node and should be avoided in DEF/USE, since the size 
	of the rectangle representing the pixel shall always be 1 pixel w/h on the final surface, therefore
	the path object is likely not the same depending on transformation context...

*/

static void get_point_size(M4Matrix2D *mat, Float *w, Float *h)
{
	M4Rect rc;
	M4Matrix2D inv;
	m4_rect_center(&rc, 1, 1);
	mx2d_copy(inv, *mat);
	mx2d_inverse(&inv);
	mx2d_apply_rect(&inv, &rc);
	*w = rc.width;
	*h = rc.height;
}

static void build_graph(Drawable *cs, M4Matrix2D *mat, M_PointSet2D *ps2D)
{
	u32 i;
	Float w, h;
	M_Coordinate2D *coord = (M_Coordinate2D *)ps2D->coord;
	get_point_size(mat, &w, &h);
	for (i=0; i < coord->point.count; i++) 
		m4_path_add_rectangle(cs->path, coord->point.vals[i].x, coord->point.vals[i].y, w, h);
}

static void RenderPointSet2D(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	M_PointSet2D *ps2D = (M_PointSet2D *)node;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (!ps2D->coord) return;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		build_graph(cs, &eff->transform, ps2D);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}

	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	ctx->aspect.filled = 1;
	ctx->aspect.has_line = 0;
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

static void PointSet2D_Draw(DrawableContext *ctx)
{
	LPM4PATH path;
	Float alpha, w, h;
	u32 i;
	SFColor col;
	M_PointSet2D *ps2D = (M_PointSet2D *)ctx->node->owner;
	M_Coordinate2D *coord = (M_Coordinate2D*) ps2D->coord;
	M_Color *color = (M_Color *) ps2D->color;

	/*never outline PS2D*/
	ctx->path_stroke = 1;
	if (!color || color->color.count<coord->point.count) {
		/*no texturing*/
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
		return;
	}

	get_point_size(&ctx->transform, &w, &h);

	path = m4_new_path();
	alpha = (Float) M4C_A(ctx->aspect.line_color) / 255;
	for (i = 0; i < coord->point.count; i++) {
		col = color->color.vals[i];
		ctx->aspect.line_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
		m4_path_add_rectangle(path, coord->point.vals[i].x, coord->point.vals[i].y, w, h);
		VS2D_DrawPath(ctx->surface, path, ctx, NULL, NULL);
		m4_path_reset(path);
		ctx->path_filled = 0;
	}
	m4_path_delete(path);
}

void R2D_InitPointSet2D(Render2D  *sr, SFNode *node)
{
	Drawable *stack = BaseDrawStack2D(sr, node);
	/*override draw*/
	stack->Draw = PointSet2D_Draw;
	Node_SetRenderFunction(node, RenderPointSet2D);
}

static void RenderPathExtrusion(SFNode *node, void *rs)
{
	FieldInfo field;
	SFNode *geom;
	if (Node_GetField(node, 0, &field) != M4OK) return;
	if (field.fieldType != FT_SFNode) return;
	geom = * (SFNode **) field.far_ptr;
	if (geom) Node_Render(geom, rs);
}

void R2D_InitPathExtrusion(Render2D *sr, SFNode *node)
{
	Node_SetRenderFunction(node, RenderPathExtrusion);
}
