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

#include "visualsurface2d.h"
#include "drawable.h"
#include "stacks2d.h"

//#define SKIP_DRAW

M4Err VS2D_InitSurface(VisualSurface2D *surf)
{
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;
	if (!surf->the_surface) {
		surf->the_surface = g_hw->new_surface(g_hw, surf->center_coords);
		if (!surf->the_surface) return M4IOErr;
	}
	if (!surf->the_brush) {
		surf->the_brush = g_hw->new_stencil(g_hw, M4StencilSolid);
		if (!surf->the_brush) return M4IOErr;
	}
	if (!surf->the_pen) {
		surf->the_pen = g_hw->new_stencil(g_hw, M4StencilSolid);
		if (!surf->the_pen) return M4IOErr;
	}
	return surf->GetSurfaceAccess(surf);
}

void VS2D_TerminateSurface(VisualSurface2D *surf)
{
	if (surf->the_surface) {
		surf->render->compositor->g_hw->surface_flush(surf->the_surface);
		surf->ReleaseSurfaceAccess(surf);
	}
}

void VS2D_ResetGraphics(VisualSurface2D *surf)
{
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;
	if (surf->the_surface) g_hw->delete_surface(surf->the_surface);
	surf->the_surface = NULL;
	if (surf->the_brush) g_hw->delete_stencil(surf->the_brush);
	surf->the_brush = NULL;
	if (surf->the_pen) g_hw->delete_stencil(surf->the_pen);
	surf->the_pen = NULL;
}

void VS2D_Clear(VisualSurface2D *surf, M4IRect *rc, u32 BackColor)
{
#ifdef SKIP_DRAW
	return;
#endif
	if (!surf->the_surface) return;
	
	if (!BackColor) {
		if (!surf->composite) BackColor = surf->default_back_color;
	}
	surf->render->compositor->g_hw->surface_clear(surf->the_surface, rc, BackColor);
}


static void draw_clipper(VisualSurface2D *surf, struct _drawable_context *ctx)
{
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;
	M4PenSettings clipset;
	LPM4PATH clippath, cliper;

	if (ctx->is_background) return;

	memset(&clipset, 0, sizeof(M4PenSettings));
	clipset.width = 2.0;

	clippath = m4_new_path();
	m4_path_add_rectangle(clippath, ctx->original.x + ctx->original.width/2, ctx->original.y - ctx->original.height/2, ctx->original.width, ctx->original.height);
	cliper = m4_path_get_outline(clippath, clipset);
	m4_path_delete(clippath);
	g_hw->surface_set_matrix(surf->the_surface, &ctx->transform);
	g_hw->surface_set_clipper(surf->the_surface, NULL);
	g_hw->surface_set_path(surf->the_surface, cliper);
	g_hw->set_brush_color(surf->the_pen, 0xFF000000);
	g_hw->surface_fill(surf->the_surface, surf->the_pen);

	m4_path_delete(cliper);
}

static void VS2D_DoFill(VisualSurface2D *surf, DrawableContext *ctx, LPM4STENCIL stencil)
{
	u32 i;
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;

	/*background rendering - direct rendering: use ctx clip*/
	if (ctx->is_background || (surf->render->top_effect->trav_flags & TF_RENDER_DIRECT)) {
		g_hw->surface_set_clipper(surf->the_surface, &ctx->clip);
		g_hw->surface_fill(surf->the_surface, stencil);
	} 
	/*indirect rendering, draw path in all dirty areas*/
	else {
		M4IRect clip;
		for (i=0; i<surf->to_redraw.count; i++) {
			/*there's an opaque region above, don't draw*/
			if (surf->draw_node_index<surf->to_redraw.opaque_node_index[i]) continue;
			clip = ctx->clip;
			m4_irect_intersect(&clip, &surf->to_redraw.list[i]);
			if (clip.width && clip.height) {
				g_hw->surface_set_clipper(surf->the_surface, &clip);
				g_hw->surface_fill(surf->the_surface, stencil);
			}
		}
	}
}

void VS2D_SetOptions(Render2D *sr, LPM4SURFACE rend, Bool forText, Bool no_antialias)
{
	Graphics2DDriver *g_hw = sr->compositor->g_hw;
	if (no_antialias) {
		g_hw->surface_set_raster_level(rend, sr->compositor->high_speed ? M4RasterHighSpeed : M4RasterInter);
	} else {
		switch (sr->compositor->antiAlias) {
		case M4_AL_None:
			g_hw->surface_set_raster_level(rend, M4RasterHighSpeed);
			break;
		case M4_AL_Text:
			if (forText) {
				g_hw->surface_set_raster_level(rend, M4RasterHighQuality);
			} else {
				g_hw->surface_set_raster_level(rend, sr->compositor->high_speed ? M4RasterHighSpeed : M4RasterInter);
			}
			break;
		case M4_AL_All:
		default:
			g_hw->surface_set_raster_level(rend, M4RasterHighQuality);
			break;
		}
	}
}


void get_texture_transform(SFNode *__appear, TextureHandler *txh, M4Matrix2D *mat, Bool line_texture, Float final_width, Float final_height)
{
	u32 node_tag;
	M_Appearance *appear;		
	SFNode *txtrans = NULL;
	mx2d_init(*mat);

	if (!__appear || !txh) return;
	appear = (M_Appearance *)__appear;

	if (!line_texture) {
		if (!appear->textureTransform) return;
		txtrans = appear->textureTransform;
	} else {
		if (Node_GetTag(appear->material) != TAG_MPEG4_Material2D) return;
		if (Node_GetTag(((M_Material2D *)appear->material)->lineProps) != TAG_MPEG4_XLineProperties) return;
		txtrans = ((M_XLineProperties *) ((M_Material2D *)appear->material)->lineProps)->textureTransform;
	}
	if (!txtrans) return;

	/*gradient doesn't need bounds info in texture transform*/
	if (txh->compute_gradient_matrix) {
		final_width = final_height = 1.0;
	}
	node_tag = Node_GetTag(txtrans);
	if (node_tag==TAG_MPEG4_TextureTransform) {
		/*VRML: Tc' = -C × S × R × C × T × Tc*/
		M_TextureTransform *txt = (M_TextureTransform *) txtrans;
		SFVec2f scale = txt->scale;
		if (!scale.x) scale.x = 0.01f;
		if (!scale.y) scale.y = 0.01f;

		mx2d_add_translation(mat, -txt->center.x * final_width, -txt->center.y * final_height);
		mx2d_add_scale(mat, scale.x, scale.y);
		mx2d_add_rotation(mat, 0, 0, txt->rotation);
		mx2d_add_translation(mat, txt->center.x * final_width, txt->center.y * final_height);
		mx2d_add_translation(mat, txt->translation.x * final_width, txt->translation.y * final_height);
		/*and inverse the matrix (this is texture transform, cf VRML)*/
		mx2d_inverse(mat);
		return;
	}
	if (node_tag==TAG_MPEG4_TransformMatrix2D) {
		TM2D_GetMatrix((SFNode *) txtrans, mat);
		mat->m[2] *= final_width;
		mat->m[5] *= final_height;
		mx2d_inverse(mat);
		return;
	}
}


static void VS2D_DrawGradient(VisualSurface2D *surf, LPM4PATH path, TextureHandler *txh, struct _drawable_context *ctx)
{
	M4Rect rc;
	Bool fill;
	M4Matrix2D g_mat, txt_mat;
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;

	if (!txh) txh = ctx->h_texture;
	rc = ctx->original;
	if (!rc.width || !rc.height || !txh->hwtx) return;
	txh->compute_gradient_matrix(txh, &rc, &g_mat);

	get_texture_transform(ctx->appear, txh, &txt_mat, (txh == ctx->h_texture) ? 0 : 1, txh->active_window.width, txh->active_window.height);
	mx2d_add_matrix(&g_mat, &txt_mat);
	mx2d_add_matrix(&g_mat, &ctx->transform);

	g_hw->stencil_set_matrix(txh->hwtx, &g_mat);

	fill = ctx->aspect.filled;
	ctx->aspect.filled = 1;
//	VS2D_DrawPath(surf, path, ctx, txh->hwtx, NULL);

	g_hw->surface_set_matrix(surf->the_surface, &ctx->transform);

	g_hw->surface_set_path(surf->the_surface, path);
	VS2D_DoFill(surf, ctx, txh->hwtx);
	g_hw->surface_set_path(surf->the_surface, NULL);

	ctx->aspect.filled = fill;
	ctx->path_filled = 1;
}



void VS2D_TexturePathText(VisualSurface2D *surf, DrawableContext *txt_ctx, LPM4PATH path, M4Rect *object_bounds, LPHWTEXTURE hwtx, M4Rect *texture_bounds)
{
	Float sS, sT;
	M4Matrix2D mx2d_txt;
	M4Rect rc, orig_rc;
	u8 alpha, r, g, b;
	M4ColorMatrix cmat;
	Graphics2DDriver *g_hw = surf->render->compositor->g_hw;

	VS2D_SetOptions(surf->render, surf->the_surface, 0, 1);

	/*get original bounds*/
	orig_rc = *object_bounds;
	rc = *texture_bounds;

	/*get scaling ratio so that active texture view is stretched to original bounds (std 2D shape texture mapping in MPEG4)*/
	sS = orig_rc.width / rc.width;
	sT = orig_rc.height / rc.height;
	
	mx2d_init(mx2d_txt);
	mx2d_add_scale(&mx2d_txt, sS, sT);

	/*move to bottom-left corner of bounds */
	mx2d_add_translation(&mx2d_txt, (orig_rc.x), (orig_rc.y - orig_rc.height));

	/*move to final coordinate system*/	
	mx2d_add_matrix(&mx2d_txt, &txt_ctx->transform);

	/*set path transform, except for background2D node which is directly build in the final coord system*/
	g_hw->stencil_set_matrix(hwtx, &mx2d_txt);
	g_hw->set_texture_view(hwtx, NULL);

	alpha = M4C_A(txt_ctx->aspect.fill_color);
	r = M4C_R(txt_ctx->aspect.fill_color);
	g = M4C_G(txt_ctx->aspect.fill_color);
	b = M4C_B(txt_ctx->aspect.fill_color);

	/*if col do a cxmatrix*/
	if (!r && !g && !b) {
		g_hw->set_texture_alpha(hwtx, alpha);
	} else {
		g_hw->set_texture_alpha(hwtx, 0xFF);
		memset(cmat.m, 0, sizeof(Float) * 20);
		cmat.m[4] = (Float) (r)/255;
		cmat.m[9] = (Float) (g)/255;
		cmat.m[14] = (Float) (b)/255;
		cmat.m[18] = (Float) (alpha)/255;
		cmat.identity = 0;
		g_hw->stencil_set_color_matrix(hwtx, &cmat);
	}

	g_hw->surface_set_matrix(surf->the_surface, &txt_ctx->transform);

	/*push path*/
	g_hw->surface_set_path(surf->the_surface, path);

	VS2D_DoFill(surf, txt_ctx, hwtx);

	g_hw->surface_set_path(surf->the_surface, NULL);

	txt_ctx->path_filled = 1;
}

void VS2D_TexturePathIntern(VisualSurface2D *surf, LPM4PATH path, TextureHandler *txh, struct _drawable_context *ctx)
{
	Float sS, sT;
	M4Matrix2D mx2d_txt, texture_transform;
	M4Rect rc, orig_rc;
	Graphics2DDriver *g_hw;
	g_hw = surf->render->compositor->g_hw;

	if (!txh) txh = ctx->h_texture;
	if (!txh || !txh->hwtx) return;

	/*this is gradient draw*/
	if (txh->compute_gradient_matrix) {
		VS2D_DrawGradient(surf, path, txh, ctx);
		return;
	}

	/*setup quality even for background (since quality concerns images)*/
	VS2D_SetOptions(surf->render, surf->the_surface, ctx->is_text, ctx->no_antialias);

	/*get original bounds*/
//	orig_rc = ctx->original;
	m4_path_get_bounds(path, &orig_rc);

	/*get active texture window in pixels*/
	rc = txh->active_window;

	/*get scaling ratio so that active texture view is stretched to original bounds (std 2D shape texture mapping in MPEG4)*/
	sS = orig_rc.width / rc.width;
	sT = orig_rc.height / rc.height;
	
	mx2d_init(mx2d_txt);
	mx2d_add_scale(&mx2d_txt, sS, sT);
	/*apply texture transform*/
	get_texture_transform(ctx->appear, txh, &texture_transform, (txh == ctx->h_texture) ? 0 : 1, txh->active_window.width * sS, txh->active_window.height * sT);
	mx2d_add_matrix(&mx2d_txt, &texture_transform);

	/*move to bottom-left corner of bounds */
	mx2d_add_translation(&mx2d_txt, (orig_rc.x), (orig_rc.y - orig_rc.height));

	/*move to final coordinate system (except background which is built directly in final coord system)*/	
	if (!ctx->is_background) mx2d_add_matrix(&mx2d_txt, &ctx->transform);

	/*set path transform, except for background2D node which is directly build in the final coord system*/
	g_hw->stencil_set_matrix(txh->hwtx, &mx2d_txt);
	g_hw->set_texture_view(txh->hwtx, &txh->active_window);


	if (!ctx->is_background) {
		/*texture alpha scale is the original material transparency, NOT the one after color transform*/
		g_hw->set_texture_alpha(txh->hwtx, ctx->aspect.fill_alpha);
		if (!ctx->cmat.identity) g_hw->stencil_set_color_matrix(txh->hwtx, &ctx->cmat);
		else g_hw->stencil_reset_color_matrix(txh->hwtx);

		g_hw->surface_set_matrix(surf->the_surface, &ctx->transform);
	} else {
		g_hw->surface_set_matrix(surf->the_surface, NULL);
	}

	/*push path & draw*/
	g_hw->surface_set_path(surf->the_surface, path);
	VS2D_DoFill(surf, ctx, txh->hwtx);
	g_hw->surface_set_path(surf->the_surface, NULL);

	ctx->path_filled = 1;
}

void VS2D_TexturePath(VisualSurface2D *surf, LPM4PATH path, struct _drawable_context *ctx)
{
#ifdef SKIP_DRAW
	return;
#endif
	if (!surf->the_surface || ctx->path_filled || !ctx->h_texture || surf->render->compositor->is_hidden) return;

	/*this is ambiguous in the spec, what if the material is filled and the texture is transparent ?
	let's draw, it's nicer */
	if (ctx->aspect.filled && ctx->h_texture->transparent) {
		VS2D_DrawPath(surf, path, ctx, NULL, NULL);
		ctx->path_filled = 0;
	}

	VS2D_TexturePathIntern(surf, path, NULL, ctx);
}


void VS2D_DrawPath(VisualSurface2D *surf, LPM4PATH path, DrawableContext *ctx, LPM4STENCIL brush, LPM4STENCIL pen)
{
	Float width, orig_width;
	Bool dofill, dostrike;
	Graphics2DDriver *g_hw;
#ifdef SKIP_DRAW
	return;
#endif

	g_hw = surf->render->compositor->g_hw;

	if (!surf->the_surface) return;
	if (ctx->path_filled && ctx->path_stroke) {
		if (surf->render->compositor->draw_bvol) draw_clipper(surf, ctx);
		return;
	}

	if (!ctx->is_background) VS2D_SetOptions(surf->render, surf->the_surface, ctx->is_text, 0);

	dofill = dostrike = 0;
	if (!ctx->path_filled && ctx->aspect.filled) {
		dofill = 1;
		if (!brush) {
			brush = surf->the_brush;
			g_hw->set_brush_color(brush, ctx->aspect.fill_color);
		}
	}

	/*compute width based on transform and top_level transform*/
	width = orig_width = ctx->aspect.pen_props.width;
	if (!ctx->path_stroke && orig_width) {
		dostrike = 1;
		if (!pen) {
			pen = surf->the_pen;
			g_hw->set_brush_color(pen, ctx->aspect.line_color);
		}
		if (ctx->aspect.pen_props.is_scalable) {
			ctx->aspect.line_scale = 1;
		} else {
			SFVec2f pt;
			pt.x = ctx->transform.m[0] + ctx->transform.m[1];
			pt.y = ctx->transform.m[3] + ctx->transform.m[4];
			ctx->aspect.line_scale = (Float) (sqrt(pt.x*pt.x + pt.y*pt.y) / sqrt(2));
		}
	}

	/*set path transform, except for background2D node which is directly build in the final coord system*/
	g_hw->surface_set_matrix(surf->the_surface, ctx->is_background ? NULL : &ctx->transform);


	/*fill path*/
	if (dofill) {
		/*push path*/
		g_hw->surface_set_path(surf->the_surface, path);
		VS2D_DoFill(surf, ctx, brush);
		g_hw->surface_set_path(surf->the_surface, NULL);
	}

	if (dostrike) {
		StrikeInfo2D *si = drawctx_get_strikeinfo(ctx, path);
		if (si && si->outline) {
			if (ctx->aspect.line_texture) {
				VS2D_TexturePathIntern(surf, si->outline, ctx->aspect.line_texture, ctx);
			} else {
				g_hw->surface_set_path(surf->the_surface, si->outline);
				VS2D_DoFill(surf, ctx, pen);
			}
			/*that's ugly, but we cannot cache path outline for IFS2D/ILS2D*/
			if (path && !ctx->is_text) {
				m4_path_delete(si->outline);
				si->outline = NULL;
			}
		}
	}

	if (surf->render->compositor->draw_bvol) draw_clipper(surf, ctx);
}

void VS2D_FillRect(VisualSurface2D *surf, DrawableContext *ctx, M4Rect rc, u32 color)
{
	Graphics2DDriver *g_hw;
	M4Path *path;
#ifdef SKIP_DRAW
	return;
#endif

	g_hw = surf->render->compositor->g_hw;

	if (!surf->the_surface) return;
	if (ctx->path_filled && ctx->path_stroke) {
		if (surf->render->compositor->draw_bvol) draw_clipper(surf, ctx);
		return;
	}

	/*no aa*/
	VS2D_SetOptions(surf->render, surf->the_surface, 0, 1);

	g_hw->set_brush_color(surf->the_brush, color);
	g_hw->surface_set_matrix(surf->the_surface, &ctx->transform);
	path = m4_new_path();
	m4_path_add_move_to(path, rc.x, rc.y);
	m4_path_add_line_to(path, rc.x+rc.width, rc.y);
	m4_path_add_line_to(path, rc.x+rc.width, rc.y-rc.height);
	m4_path_add_line_to(path, rc.x, rc.y-rc.height);
	m4_path_close(path);

	/*push path*/
	g_hw->surface_set_path(surf->the_surface, path);
	VS2D_DoFill(surf, ctx, surf->the_brush);
	g_hw->surface_set_path(surf->the_surface, NULL);
	m4_path_delete(path);
}
