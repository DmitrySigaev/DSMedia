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

typedef struct _composite_2D
{
	TextureHandler txh;
	u32 width, height;
	/*the surface object handling the texture*/
	struct _visual_surface_2D *surf;
	Chain *sensors;
	Bool first;
} Composite2DStack;

static void DestroyComposite2D(SFNode *node)
{
	Composite2DStack *st = (Composite2DStack *) Node_GetPrivate(node);
	/*unregister surface*/
	R2D_UnregisterSurface(st->surf->render, st->surf);
	DeleteVisualSurface2D(st->surf);
	DeleteChain(st->sensors);
	/*destroy texture*/
	texture_destroy(&st->txh);
	free(st);
}


static Bool Composite_CheckBindables(SFNode *n, RenderEffect2D *eff, Bool force_check)
{
	SFNode *btop;
	Bool ret = 0;
	M_CompositeTexture2D *c2d = (M_CompositeTexture2D *)n;
	if (force_check || Node_GetDirty(c2d->background)) { Node_Render(c2d->background, eff); ret = 1; }
	btop = ChainGetEntry(eff->back_stack, 0);
	if (btop != c2d->background) {
		Node_Unregister(c2d->background, n);
		Node_Register(btop, n); 
		c2d->background = btop;
		Node_OnEventOutSTR(n, "background");
		ret = 1;
	}

	if (force_check || Node_GetDirty(c2d->viewport)) { Node_Render(c2d->viewport, eff); ret = 1; }
	btop = ChainGetEntry(eff->view_stack, 0);
	if (btop != c2d->viewport) { 
		Node_Unregister(c2d->viewport, n);
		Node_Register(btop, n); 
		c2d->viewport = btop;
		Node_OnEventOutSTR(n, "viewport");
		ret = 1;
	}
	return ret;
}

static void UpdateComposite2D(TextureHandler *txh)
{
	M4Err e;
	u32 count;
	Bool changed;
	u32 i;
	SensorHandler *hsens;
	SFNode *child;
	RenderEffect2D *eff;

	M_CompositeTexture2D *ct2D = (M_CompositeTexture2D *)txh->owner;
	Composite2DStack *st = (Composite2DStack *) Node_GetPrivate(txh->owner);
	Graphics2DDriver *ghw = st->surf->render->compositor->g_hw;
	/*rebuild stencil*/
	if (!st->surf->the_surface || !txh->hwtx || ((s32) st->width != ct2D->pixelWidth) || ( (s32) st->height != ct2D->pixelHeight) ) {
		if (txh->hwtx) ghw->delete_stencil(txh->hwtx);
		txh->hwtx = NULL;

		if (ct2D->pixelWidth<=0) return;
		if (ct2D->pixelHeight<=0) return;
		st->width = ct2D->pixelWidth;
		st->height = ct2D->pixelHeight;

		txh->hwtx = ghw->new_stencil(ghw, M4StencilTexture);
		if (ghw->stencil_create_texture) {
			e = ghw->stencil_create_texture(txh->hwtx, st->width, st->height, M4PF_ARGB);
		}
		/*TODO*/
		else {
			e = M4NotSupported;
		}
		if (e) {
			if (txh->hwtx) ghw->delete_stencil(txh->hwtx);
			txh->hwtx = NULL;
		}
	}
	if (!txh->hwtx) return;

	eff = malloc(sizeof(RenderEffect2D));
	memset(eff, 0, sizeof(RenderEffect2D));
	eff->sensors = NewChain();
	eff->surface = st->surf;

	if (st->surf->render->top_effect->trav_flags & TF_RENDER_DIRECT) {
		eff->trav_flags = TF_RENDER_DIRECT;
	} else {
		eff->trav_flags = 0;
	}


	mx2d_init(eff->transform);
	cmat_init(&eff->color_mat);
	st->surf->width = st->width;
	st->surf->height = st->height;
	eff->back_stack = st->surf->back_stack;
	eff->view_stack = st->surf->view_stack;
	eff->is_pixel_metrics = SG_UsePixelMetrics(Node_GetParentGraph(st->txh.owner));
	eff->min_hsize = MIN(st->width, st->height) * 0.5f;

	Composite_CheckBindables(st->txh.owner, eff, st->first);
	st->first = 0;
	
	VS2D_InitDraw(st->surf, eff);

	/*render children*/
	count = ChainGetCount(ct2D->children);
	if (Node_GetDirty(st->txh.owner) & SG_NODE_DIRTY) {
		/*rebuild sensor list */
		if (ChainGetCount(st->sensors)) {
			DeleteChain(st->sensors);
			st->sensors = NewChain();
		}
		for (i=0; i<count; i++) {
			child = ChainGetEntry(ct2D->children, i);
			if (!child || !is_sensor_node(child) ) continue;
			hsens = get_sensor_handler(child);
			if (hsens) ChainAddEntry(st->sensors, hsens);
		}

		/*if we have an active sensor at this level discard all sensors in current render context (cf VRML)*/
		if (ChainGetCount(st->sensors)) {
			effect_reset_sensors(eff);
		}
	}

	/*add sensor to effects*/	
	for (i=0; i <ChainGetCount(st->sensors); i++) {
		SensorHandler *hsens = ChainGetEntry(st->sensors, i);
		effect_add_sensor(eff, hsens, &eff->transform);
	}

	Node_ClearDirty(st->txh.owner);

	/*render*/
	Node_RenderChildren(st->txh.owner, eff);

	/*finalize draw*/
	changed = VS2D_TerminateDraw(st->surf, eff);

	st->txh.transparent = st->surf->last_had_back ? 0 : 1;
	st->txh.active_window.x = 0;
	st->txh.active_window.y = 0;
	st->txh.active_window.width = (Float) st->width;
	st->txh.active_window.height = (Float) st->height;

	/*set active viewport in image coordinates top-left=(0, 0), not in BIFS*/
	if (ChainGetCount(st->surf->view_stack)) {
		M_Viewport *vp = ChainGetEntry(st->surf->view_stack, 0);

		if (vp->isBound) {
			SFVec2f size = vp->size;
			if (size.x >=0 && size.y>=0) {
				st->txh.active_window.width = size.x;
				st->txh.active_window.height = size.y;
				st->txh.active_window.x = (st->width - size.x) / 2;
				st->txh.active_window.y = (st->height - size.y) / 2;

				/*FIXME - we need tracking of VP changes*/
				changed = 1;
			}
		}
	} 

	if (changed) {
		txh->needs_refresh = 1;
		ghw->stencil_texture_modified(st->txh.hwtx); 
		SR_Invalidate(st->txh.compositor, NULL);
	} else {
		txh->needs_refresh = 0;
	}
	effect_delete(eff);
}

static M4Err C2D_GetSurfaceAccess(VisualSurface2D *surf)
{
	M4Err e;
	if (!surf->composite->txh.hwtx || !surf->the_surface) return M4BadParam;

	e = M4IOErr;
	if (surf->render->compositor->g_hw->attach_surface_to_texture) {
		e = surf->render->compositor->g_hw->attach_surface_to_texture(surf->the_surface, surf->composite->txh.hwtx);
		if (!e) surf->is_attached = 1;;
	}
	return e;
}

static void C2D_ReleaseSurfaceAccess(VisualSurface2D *surf)
{
	surf->render->compositor->g_hw->detach_surface(surf->the_surface);
}

void R2D_InitCompositeTexture2D(Render2D *sr, SFNode *node)
{
	Composite2DStack *st = malloc(sizeof(Composite2DStack));
	memset(st, 0, sizeof(Composite2DStack));
	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = UpdateComposite2D;
	
	/*create composite surface*/
	st->surf = NewVisualSurface2D();
	st->surf->composite = st;
	st->surf->GetSurfaceAccess = C2D_GetSurfaceAccess;
	st->surf->ReleaseSurfaceAccess = C2D_ReleaseSurfaceAccess;

	/*Bitmap drawn with brush, not hardware since we don't know how the graphics driver handles the texture bytes*/
	st->surf->DrawBitmap = NULL;
	st->surf->SupportsFormat = NULL;
	st->first = 1;
	st->surf->render = sr;
	st->sensors = NewChain();
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyComposite2D);
	R2D_RegisterSurface(sr, st->surf);

}

TextureHandler *ct2D_get_texture(SFNode *node)
{
	Composite2DStack *st = (Composite2DStack*) Node_GetPrivate(node);
	return &st->txh;
}


Bool CT2D_has_sensors(TextureHandler *txh)
{
	Composite2DStack *st = (Composite2DStack *) Node_GetPrivate(txh->owner);
	assert(st->surf);
	return ChainGetCount(st->surf->sensors) ? 1 : 0;
}

void get_texture_transform(SFNode *__appear, TextureHandler *txh, M4Matrix2D *mat, Bool line_texture, Float final_width, Float final_height);

DrawableContext *CT2D_FindNode(TextureHandler *txh, DrawableContext *ctx, Float x, Float y)
{
	M4Rect rc;
	M4Matrix2D mat, tx_trans;
	Float width, height;
	Composite2DStack *st = (Composite2DStack *) Node_GetPrivate(txh->owner);
	assert(st->surf);

	rc = ctx->original;
	width = ctx->h_texture->active_window.width;
	height = ctx->h_texture->active_window.height;

	mx2d_init(mat);
	mx2d_add_scale(&mat, rc.width / width, rc.height / height);
	get_texture_transform(ctx->appear, &st->txh, &tx_trans, (ctx->h_texture==&st->txh) ? 0 : 1, ctx->original.width, ctx->original.height);
	mx2d_add_matrix(&mat, &tx_trans);
	mx2d_add_translation(&mat, (rc.x), (rc.y - rc.height));
	mx2d_add_matrix(&mat, &ctx->transform);

	mx2d_inverse(&mat);
	mx2d_apply_coords(&mat, &x, &y);
	while (x>width) x -= width;
	while (x < 0) x += width;
	while (y>height) y -= height;
	while (y < 0) y += height;
	x -= width / 2;
	y -= height / 2;

	return VS2D_FindNode(st->surf, x, y);
}


SFNode *CT2D_PickNode(TextureHandler *txh, DrawableContext *ctx, Float x, Float y)
{
	M4Rect rc;
	M4Matrix2D mat, tx_trans;
	Float width, height;
	Composite2DStack *st = (Composite2DStack *) Node_GetPrivate(txh->owner);
	assert(st->surf);

	rc = ctx->original;
	width = ctx->h_texture->active_window.width;
	height = ctx->h_texture->active_window.height;

	mx2d_init(mat);
	mx2d_add_scale(&mat, rc.width / width, rc.height / height);
	get_texture_transform(ctx->appear, &st->txh, &tx_trans, (ctx->h_texture==&st->txh) ? 0 : 1, ctx->original.width, ctx->original.height);
	mx2d_add_matrix(&mat, &tx_trans);
	mx2d_add_translation(&mat, (rc.x), (rc.y - rc.height));
	mx2d_add_matrix(&mat, &ctx->transform);

	mx2d_inverse(&mat);
	mx2d_apply_coords(&mat, &x, &y);
	while (x>width) x -= width;
	while (x < 0) x += width;
	while (y>height) y -= height;
	while (y < 0) y += height;
	x -= width / 2;
	y -= height / 2;

	return VS2D_PickNode(st->surf, x, y);
}


static void GradientGetMatrix(SFNode *transform, M4Matrix2D *mat)
{
	mx2d_init(*mat);
	if (transform) {
		switch (Node_GetTag(transform) ) {
		case TAG_MPEG4_Transform2D:
		{
			M_Transform2D *tr = (M_Transform2D *)transform;
			mx2d_add_scale_at(mat, 0, 0, tr->scale.x, tr->scale.y, tr->scaleOrientation);
			mx2d_add_rotation(mat, tr->center.x, tr->center.y, tr->rotationAngle);
			mx2d_add_translation(mat, tr->translation.x, tr->translation.y);
		}
			break;

		case TAG_MPEG4_TransformMatrix2D:
			TM2D_GetMatrix(transform, mat);
			break;
		default:
			break;
		}
	}
}

typedef struct
{
	TextureHandler txh;
} GradientStack;

/*
		linear gradient
*/

static void DestroyLinearGradient(SFNode *node)
{
	GradientStack *st = (GradientStack *) Node_GetPrivate(node);
	texture_destroy(&st->txh);
	free(st);
}

static void UpdateLinearGradient(TextureHandler *txh)
{
	u32 i;
	M_LinearGradient *lg = (M_LinearGradient *) txh->owner;
	GradientStack *st = (GradientStack *) Node_GetPrivate(txh->owner);
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;
	if (!txh->hwtx) txh->hwtx = ghw->new_stencil(ghw, M4StencilLinearGradient);

	if (!Node_GetDirty(txh->owner)) return;
	Node_ClearDirty(txh->owner);

	txh->needs_refresh = 1;

	st->txh.transparent = 0;
	for (i=0; i<lg->opacity.count; i++) {
		if (lg->opacity.vals[i] != 1.0) {
			st->txh.transparent = 1;
			break;
		}
	}
}

static void LG_ComputeMatrix(TextureHandler *txh, M4Rect *bounds, M4Matrix2D *mat)
{
	SFVec2f start, end;
	u32 i, *cols;
	Float a;
	Bool const_a;
	Graphics2DDriver *dr;
	M_LinearGradient *lg = (M_LinearGradient *) txh->owner;

	if (lg->key.count<2) return;
	if (lg->key.count != lg->keyValue.count) return;

	start = lg->startPoint;
	end = lg->endPoint;

	/*create gradient brush if needed*/
	dr = txh->compositor->g_hw;
	if (!txh->hwtx) return;

	GradientGetMatrix((SFNode *) lg->transform, mat);
	
	/*move line to object space*/
	start.x *= bounds->width;
	end.x *= bounds->width;
	start.y *= bounds->height;
	end.y *= bounds->height;

	/*move transform to object space*/
	mat->m[2] *= bounds->width;
	mat->m[5] *= bounds->height;
	mat->m[1] *= bounds->width / bounds->height;
	mat->m[3] *= bounds->height / bounds->width;

	/*translate to the center of the bounds*/
	mx2d_add_translation(mat, bounds->x, bounds->y - bounds->height);


	dr->set_linear_gradient(txh->hwtx, start.x, start.y, end.x, end.y, 0xFFFF0000, 0xFFFF00FF);
	
	const_a = (lg->opacity.count == 1) ? 1 : 0;
	cols = malloc(sizeof(u32) * lg->key.count);
	for (i=0; i<lg->key.count; i++) {
		a = (const_a ? lg->opacity.vals[0] : lg->opacity.vals[i]);
		cols[i] = MAKE_ARGB_FLOAT(a, lg->keyValue.vals[i].red, lg->keyValue.vals[i].green, lg->keyValue.vals[i].blue);
	}
	dr->set_gradient_interpolation(txh->hwtx, lg->key.vals, cols, lg->key.count);
	free(cols);
	dr->set_gradient_mode(txh->hwtx, lg->spreadMethod);

}

void R2D_InitLinearGradient(Render2D *sr, SFNode *node)
{
	GradientStack *st = malloc(sizeof(GradientStack));
	memset(st, 0, sizeof(GradientStack));

	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = UpdateLinearGradient;

	st->txh.compute_gradient_matrix = LG_ComputeMatrix;
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyLinearGradient);
}

TextureHandler *r2d_lg_get_texture(SFNode *node)
{
	GradientStack *st = (GradientStack*) Node_GetPrivate(node);
	return &st->txh;
}


/*
		radial gradient
*/


static void DestroyRadialGradient(SFNode *node)
{
	GradientStack *st = (GradientStack *) Node_GetPrivate(node);
	texture_destroy(&st->txh);
	free(st);
}

static void UpdateRadialGradient(TextureHandler *txh)
{
	u32 i;
	M_RadialGradient *rg = (M_RadialGradient*) txh->owner;
	GradientStack *st = (GradientStack *) Node_GetPrivate(txh->owner);
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;
	if (!txh->hwtx) txh->hwtx = ghw->new_stencil(ghw, M4StencilRadialGradient);

	if (!Node_GetDirty(txh->owner)) return;
	Node_ClearDirty(txh->owner);
	txh->needs_refresh = 1;

	st->txh.transparent = 0;
	for (i=0; i<rg->opacity.count; i++) {
		if (rg->opacity.vals[i] != 1.0) {
			st->txh.transparent = 1;
			break;
		}
	}
}

static void RG_ComputeMatrix(TextureHandler *txh, M4Rect *bounds, M4Matrix2D *mat)
{
	SFVec2f center, focal;
	u32 i, *cols;
	Float a;
	Bool const_a;
	Graphics2DDriver *dr;
	M_RadialGradient *rg = (M_RadialGradient *) txh->owner;

	if (rg->key.count<2) return;
	if (rg->key.count != rg->keyValue.count) return;

	/*create gradient brush if needed*/
	dr = txh->compositor->g_hw;
	if (!txh->hwtx) return;

	GradientGetMatrix((SFNode *) rg->transform, mat);

	center = rg->center;
	focal = rg->focalPoint;

	/*move circle to object space*/
	center.x *= bounds->width;
	center.y *= bounds->height;
	focal.x *= bounds->width;
	focal.y *= bounds->height;


	/*move transform to object space*/
	mat->m[2] *= bounds->width;
	mat->m[5] *= bounds->height;
	mat->m[1] *= bounds->width / bounds->height;
	mat->m[3] *= bounds->height / bounds->width;

	dr->set_radial_gradient(txh->hwtx, center.x, center.y, focal.x, focal.y, rg->radius * bounds->width, rg->radius * bounds->height);

	const_a = (rg->opacity.count == 1) ? 1 : 0;
	cols = malloc(sizeof(u32) * rg->key.count);
	for (i=0; i<rg->key.count; i++) {
		a = (const_a ? rg->opacity.vals[0] : rg->opacity.vals[i]);
		cols[i] = MAKE_ARGB_FLOAT(a, rg->keyValue.vals[i].red, rg->keyValue.vals[i].green, rg->keyValue.vals[i].blue);
	}
	dr->set_gradient_interpolation(txh->hwtx, rg->key.vals, cols, rg->key.count);
	free(cols);

	dr->set_gradient_mode(txh->hwtx, rg->spreadMethod);
	mx2d_add_translation(mat, bounds->x, bounds->y - bounds->height);
}

void R2D_InitRadialGradient(Render2D *sr, SFNode *node)
{
	GradientStack *st = malloc(sizeof(GradientStack));
	memset(st, 0, sizeof(GradientStack));

	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = UpdateRadialGradient;

	st->txh.compute_gradient_matrix = RG_ComputeMatrix;

	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyRadialGradient);
}

TextureHandler *r2d_rg_get_texture(SFNode *node)
{
	GradientStack *st = (GradientStack*) Node_GetPrivate(node);
	return &st->txh;
}

static void mattetexture_clone(TextureHandler *txh, TextureHandler *from)
{
	u32 i, j;
	if (!from->hwtx) return;

	if (!txh->hwtx) txh->hwtx = txh->compositor->g_hw->new_stencil(txh->compositor->g_hw, M4StencilTexture);
	if (!txh->hwtx) return;

	txh->width = from->width;
	txh->height = from->height;
	txh->stride = from->stride;
	txh->active_window = from->active_window;
	txh->transparent = from->transparent;
	/*we only create true ARGB clones*/
	txh->compositor->g_hw->stencil_create_texture(txh->hwtx, txh->width, txh->height, M4PF_ARGB);
	txh->pixelformat = M4PF_ARGB;

	/*and copy image*/
	for (j=0; j<from->height; j++) {
		for (i=0; i<from->width; i++) {
			u32 c;
			txh->compositor->g_hw->stencil_get_pixel(from->hwtx, i, j, &c);
			txh->compositor->g_hw->stencil_set_pixel(txh->hwtx, i, j, c);
		}
	}
}

static void mattetexture_flush_clone(TextureHandler *txh)
{
	if (!txh->hwtx) return;
	txh->compositor->g_hw->stencil_texture_modified(txh->hwtx);
}

static void mattetexture_delete_clone(TextureHandler *txh)
{
	if (txh->hwtx) txh->compositor->g_hw->delete_stencil(txh->hwtx);
	txh->hwtx = NULL;
	txh->data = NULL;
	txh->width = txh->height = txh->stride = txh->pixelformat = txh->frame_size = 0;
}


typedef struct
{
	/*resulting texture*/
	TextureHandler txh;
	/*input texture*/
	TextureHandler *surf_a, *surf_b, *surf_alpha;
	/*current overwrite*/
	Bool overwrite;
} MatteTextureStack;

/*all effects implemented*/
static void Matte_Invert(MatteTextureStack *st, M_MatteTexture *n);
static void Matte_Scale(MatteTextureStack *st, M_MatteTexture *n);
static void Matte_Bias(MatteTextureStack *st, M_MatteTexture *n);
static void Matte_ColorMatrix(MatteTextureStack *st, M_MatteTexture *n);
static void Matte_ReplaceAlpha(MatteTextureStack *st, M_MatteTexture *n);

static void DestroyMatteTexture(SFNode *node)
{
	MatteTextureStack *st = (MatteTextureStack *) Node_GetPrivate(node);
	if (st->overwrite) 
		st->txh.hwtx = NULL;
	else
		mattetexture_delete_clone(&st->txh);

	texture_destroy(&st->txh);
	free(st);
}

static void UpdateMatteTexture(TextureHandler *txh)
{
	Bool changed;
	M_MatteTexture *txnode = (M_MatteTexture *) txh->owner;
	MatteTextureStack *st = (MatteTextureStack *) Node_GetPrivate(txh->owner);
	
	if (!st->surf_b) return;
	changed = 0;
	if (st->surf_a) {
		st->surf_a->update_texture_fcnt(st->surf_a);
		if (st->surf_a->needs_refresh) changed = 1;
	}
	if (st->surf_b) {
		st->surf_b->update_texture_fcnt(st->surf_b);
		if (st->surf_b->needs_refresh) changed = 1;
	}
	if (st->surf_alpha) {
		st->surf_alpha->update_texture_fcnt(st->surf_alpha);
		if (st->surf_alpha->needs_refresh) changed = 1;
	}
	if (!st->surf_b->data) return;

	if (Node_GetDirty(txh->owner)) {
		changed = 1;
		Node_ClearDirty(txh->owner);
	}
	if (!changed) return;

	if (!st->overwrite) {
		mattetexture_clone(&st->txh, st->surf_b);
	} else {
		st->txh.hwtx = st->surf_b->hwtx;
		st->txh.data = st->surf_b->data;
		st->txh.width = st->surf_b->width;
		st->txh.height = st->surf_b->height;
		st->txh.stride = st->surf_b->stride;
		st->txh.pixelformat = st->surf_b->pixelformat;
	}
	if (!st->txh.hwtx) return;
	txh->needs_refresh = 1;
	st->txh.active_window = st->surf_b->active_window;

	st->txh.has_cmat = 0;

	if (!stricmp(txnode->operation.buffer, "")) return;
	if (!stricmp(txnode->operation.buffer, "INVERT")) Matte_Invert(st, txnode);
	else if (!stricmp(txnode->operation.buffer, "SCALE")) Matte_Scale(st, txnode);
	else if (!stricmp(txnode->operation.buffer, "BIAS")) Matte_Bias(st, txnode);
	else if (!stricmp(txnode->operation.buffer, "COLOR_MATRIX")) Matte_ColorMatrix(st, txnode);
	else if (!stricmp(txnode->operation.buffer, "REPLACE_ALPHA")) Matte_ReplaceAlpha(st, txnode);


	if (!st->overwrite) mattetexture_flush_clone(&st->txh);

}

void R2D_InitMatteTexture(Render2D *sr, SFNode *node)
{
	MatteTextureStack *st = malloc(sizeof(MatteTextureStack));
	memset(st, 0, sizeof(MatteTextureStack));
	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = UpdateMatteTexture;
	
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyMatteTexture);
}

TextureHandler *r2d_matte_get_texture(SFNode *node)
{
	MatteTextureStack *st = (MatteTextureStack *) Node_GetPrivate(node);
	return &st->txh;
}


TextureHandler *get_texture_handler(SFNode *n);

void R2D_MatteTextureModified(SFNode *node)
{
	TextureHandler *surf_a, *surf_b, *surf_alpha;
	M_MatteTexture *mt = (M_MatteTexture *)node;
	MatteTextureStack *st = (MatteTextureStack *) Node_GetPrivate(node);
	if (!st) return;

	surf_a = R2D_GetTextureHandler(mt->surfaceA);
	surf_b = R2D_GetTextureHandler(mt->surfaceB);
	surf_alpha = R2D_GetTextureHandler(mt->alphaSurface);

	/*destroy previous texture*/
	if (!st->overwrite) mattetexture_delete_clone(&st->txh);

	st->surf_a = surf_a;
	st->surf_b = surf_b;
	st->surf_alpha = surf_alpha;
	st->overwrite = mt->overwrite;

}


static void Matte_Copy(MatteTextureStack *st)
{
	u32 i, j, col;

	if (st->overwrite) return;
	for (j=0; j<st->surf_b->height; j++) {
		for (i=0; i<st->surf_b->width; i++) {
			st->txh.compositor->g_hw->stencil_get_pixel(st->surf_b->hwtx, i, j, &col);
			st->txh.compositor->g_hw->stencil_set_pixel(st->txh.hwtx, i, j, col);
		}
	}
}

static void Matte_Invert(MatteTextureStack *st, M_MatteTexture *n)
{
	M4ColorMatrix cmat;
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;

	Matte_Copy(st);
	st->txh.has_cmat = 1;

	memset(&cmat, 0, sizeof(cmat));
	cmat.m[0] = cmat.m[6] = cmat.m[12] = -1;
	cmat.m[4] = cmat.m[9] = cmat.m[14] = 1;
	if (n->parameter.count && n->parameter.vals[0]) {
		cmat.m[18] = -1;
		cmat.m[19] = 1;
	} else {
		cmat.m[18] = 1;
		cmat.m[19] = 0;
	}
	ghw->stencil_set_color_matrix(st->txh.hwtx, &cmat);
}

static void Matte_Scale(MatteTextureStack *st, M_MatteTexture *n)
{
	M4ColorMatrix cmat;
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;
	Matte_Copy(st);
	st->txh.has_cmat = 1;

	memset(&cmat, 0, sizeof(cmat));
	cmat.m[0] = cmat.m[6] = cmat.m[12] = cmat.m[18] = 1;
	if (n->parameter.count >= 1) cmat.m[0] = n->parameter.vals[0];
	if (n->parameter.count >= 2) cmat.m[6] = n->parameter.vals[1];
	if (n->parameter.count >= 3) cmat.m[12] = n->parameter.vals[2];
	if (n->parameter.count >= 4) cmat.m[18] = n->parameter.vals[3];

	ghw->stencil_set_color_matrix(st->txh.hwtx, &cmat);
}

static void Matte_Bias(MatteTextureStack *st, M_MatteTexture *n)
{
	M4ColorMatrix cmat;
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;

	Matte_Copy(st);
	st->txh.has_cmat = 1;

	memset(&cmat, 0, sizeof(cmat));
	cmat.m[0] = cmat.m[6] = cmat.m[12] = cmat.m[18] = 1;
	if (n->parameter.count >= 1) cmat.m[4] = n->parameter.vals[0];
	if (n->parameter.count >= 2) cmat.m[9] = n->parameter.vals[1];
	if (n->parameter.count >= 3) cmat.m[14] = n->parameter.vals[2];
	if (n->parameter.count >= 4) cmat.m[19] = n->parameter.vals[3];

	ghw->stencil_set_color_matrix(st->txh.hwtx, &cmat);
}

static void Matte_ColorMatrix(MatteTextureStack *st, M_MatteTexture *n)
{
	M4ColorMatrix cmat;
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;
	Matte_Copy(st);
	st->txh.has_cmat = 1;

	if (n->parameter.count < 16) return;
	memset(&cmat, 0, sizeof(cmat));
	cmat.m[0] = n->parameter.vals[0];
	cmat.m[5] = n->parameter.vals[1];
	cmat.m[10] = n->parameter.vals[2];
	cmat.m[15] = n->parameter.vals[3];

	cmat.m[1] = n->parameter.vals[4];
	cmat.m[6] = n->parameter.vals[5];
	cmat.m[11] = n->parameter.vals[6];
	cmat.m[16] = n->parameter.vals[7];

	cmat.m[2] = n->parameter.vals[8];
	cmat.m[7] = n->parameter.vals[9];
	cmat.m[12] = n->parameter.vals[10];
	cmat.m[17] = n->parameter.vals[11];

	cmat.m[3] = n->parameter.vals[12];
	cmat.m[8] = n->parameter.vals[13];
	cmat.m[13] = n->parameter.vals[14];
	cmat.m[18] = n->parameter.vals[15];

	cmat.m[4] = cmat.m[9] = cmat.m[14] = cmat.m[19] = 0;

	ghw->stencil_set_color_matrix(st->txh.hwtx, &cmat);
}


static void Matte_ReplaceAlpha(MatteTextureStack *st, M_MatteTexture *n)
{
	Float sX, sY;
	u32 w, h, i, j, xA, yA, col, colA, prevA;
	u8 alpha;
	Bool useRplane;
	Graphics2DDriver *ghw = st->txh.compositor->g_hw;

	if (!st->surf_alpha || !st->surf_alpha->hwtx || !ghw->stencil_get_pixel || !ghw->stencil_set_pixel) return;

    sX = (Float) st->surf_alpha->width;
	sX /= st->surf_b->width;
    sY = (Float) st->surf_alpha->height;
	sY /= st->surf_b->height;

	w = st->surf_b->width;
	h = st->surf_b->height;
	useRplane = !st->surf_alpha->transparent;

	alpha = 0;
	st->txh.transparent = 1;

    for (j=0; j<h; j++) {
        yA = (u32) (sY * j);
		prevA = 0xFFFFFFFF;
        for (i=0; i<w; i++) {        
            xA = (s32) (sX*i);
			if (xA != prevA) {
				ghw->stencil_get_pixel(st->surf_alpha->hwtx, xA, yA, &colA);
				alpha = (colA>>24) & 0xFF;
				xA = prevA;
			}
			ghw->stencil_get_pixel(st->surf_b->hwtx, i, j, &col);
			colA = alpha;
			col = (colA<<24) | (col & 0x00FFFFFF);
			ghw->stencil_set_pixel(st->txh.hwtx, i, j, col);
        }
    }
}



TextureHandler *R2D_GetTextureHandler(SFNode *n)
{
	if (!n) return NULL;
	switch (Node_GetTag(n)) {
	case TAG_MPEG4_CompositeTexture2D: return ct2D_get_texture(n);
	case TAG_MPEG4_MatteTexture: return r2d_matte_get_texture(n);
	case TAG_MPEG4_LinearGradient: return r2d_lg_get_texture(n);
	case TAG_MPEG4_RadialGradient: return r2d_rg_get_texture(n);
	default: return texture_get_handler(n);
	}
}
