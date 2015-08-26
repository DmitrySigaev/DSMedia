/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Rendering sub-project
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

#include "../visualsurface2d.h"

#ifndef M4_DISABLE_SVG
#include "svg_stacks.h"

DrawableContext *SVG_drawable_init_context(Drawable *, RenderEffect2D *);

static void SVGInitContext(SVGStylingProperties *svg_props) 
{
	if (!svg_props) return;

	SAFEALLOC(svg_props->fill, sizeof(SVG_Paint));
	svg_props->fill->paintType = SVG_PAINTTYPE_RGBCOLOR;
	svg_props->fill->color.red = 0;
	svg_props->fill->color.green = 0;
	svg_props->fill->color.blue = 0;

	SAFEALLOC(svg_props->fill_rule, sizeof(SVG_ClipFillRule));
	*svg_props->fill_rule = SVGFillRule_nonzero;

	SAFEALLOC(svg_props->fill_opacity, sizeof(SVG_OpacityValue));
	svg_props->fill_opacity->type = SVGFLOAT_VALUE;
	svg_props->fill_opacity->value = 1;
	
	SAFEALLOC(svg_props->stroke, sizeof(SVG_Paint));
	svg_props->stroke->paintType = SVG_PAINTTYPE_NONE;

	SAFEALLOC(svg_props->stroke_opacity, sizeof(SVG_OpacityValue));
	svg_props->stroke_opacity->type = SVGFLOAT_VALUE;
	svg_props->stroke_opacity->value = 1;

	SAFEALLOC(svg_props->stroke_width, sizeof(SVG_StrokeWidthValue));
	svg_props->stroke_width->unitType = SVG_LENGTHTYPE_PX;
	svg_props->stroke_width->number = 1;

	SAFEALLOC(svg_props->stroke_linecap, sizeof(SVG_StrokeLineCapValue));
	*(svg_props->stroke_linecap) = SVGStrokeLineCap_butt;
	SAFEALLOC(svg_props->stroke_linejoin, sizeof(SVG_StrokeLineJoinValue));
	*(svg_props->stroke_linejoin) = SVGStrokeLineJoin_miter;

	SAFEALLOC(svg_props->stroke_miterlimit, sizeof(SVGInheritableFloat));
	svg_props->stroke_miterlimit->type = SVGFLOAT_VALUE;
	svg_props->stroke_miterlimit->value = 4;

	SAFEALLOC(svg_props->font_family, sizeof(SVG_FontFamilyValue));
	svg_props->font_family->type = SVGFontFamily_string;
	svg_props->font_family->value.string = strdup("Arial");
	svg_props->font_family->value.length = 6;

	SAFEALLOC(svg_props->font_size, sizeof(SVGInheritableFloat));
	svg_props->font_size->type = SVGFLOAT_VALUE;
	svg_props->font_size->value = 12;

	SAFEALLOC(svg_props->font_style, sizeof(SVG_FontStyleValue));
	*(svg_props->font_style) = SVGFontStyle_normal;

}

void SVG_DestroyContext(SVGStylingProperties *svg_props)
{
	if(svg_props->fill) free(svg_props->fill);
	if(svg_props->fill_rule) free(svg_props->fill_rule);
	if(svg_props->fill_opacity) free(svg_props->fill_opacity);
	if(svg_props->stroke) free(svg_props->stroke);
	if(svg_props->stroke_opacity) free(svg_props->stroke_opacity);
	if(svg_props->stroke_width) free(svg_props->stroke_width);
	if(svg_props->stroke_linecap) free(svg_props->stroke_linecap);
	if(svg_props->stroke_linejoin) free(svg_props->stroke_linejoin);
	if(svg_props->stroke_miterlimit) free(svg_props->stroke_miterlimit);
	if(svg_props->font_family) {
		if (svg_props->font_family->value.string) free(svg_props->font_family->value.string);
		free(svg_props->font_family);
	}
	if(svg_props->font_size) free(svg_props->font_size);
	if(svg_props->font_style) free(svg_props->font_style);
	free(svg_props);
}

/* Apply the current svg properties in the current node to modify the global properties 
   which will be forwarded down the rendering tree */
void SVGMergeContexts(SVGStylingProperties *render_svg_props, SVGStylingProperties current_svg_props)
{
	if (!render_svg_props) return;

	if (current_svg_props.fill && current_svg_props.fill->paintType != SVG_PAINTTYPE_INHERIT) {
		render_svg_props->fill = current_svg_props.fill;
	}
	if (current_svg_props.fill_rule && *current_svg_props.fill_rule != SVGFillRule_inherit) {
		render_svg_props->fill_rule = current_svg_props.fill_rule;
	}
	if (current_svg_props.fill_opacity && current_svg_props.fill_opacity->type != SVGFLOAT_INHERIT) {
		render_svg_props->fill_opacity = current_svg_props.fill_opacity;
	}
	if (current_svg_props.stroke && current_svg_props.stroke->paintType != SVG_PAINTTYPE_INHERIT) {
		render_svg_props->stroke = current_svg_props.stroke;
	}
	if (current_svg_props.stroke_opacity && current_svg_props.stroke_opacity->type != SVGFLOAT_INHERIT) {
		render_svg_props->stroke_opacity = current_svg_props.stroke_opacity;
	}
	if (current_svg_props.stroke_width && current_svg_props.stroke_width->unitType != SVG_LENGTHTYPE_INHERIT) {
		render_svg_props->stroke_width = current_svg_props.stroke_width;
	}
	if (current_svg_props.stroke_miterlimit && current_svg_props.stroke_miterlimit->type != SVGFLOAT_INHERIT) {
		render_svg_props->stroke_miterlimit = current_svg_props.stroke_miterlimit;
	}
	if (current_svg_props.stroke_linecap && *current_svg_props.stroke_linecap != SVGStrokeLineCap_inherit) {
		render_svg_props->stroke_linecap = current_svg_props.stroke_linecap;
	}
	if (current_svg_props.stroke_linejoin && *current_svg_props.stroke_linejoin != SVGStrokeLineJoin_inherit) {
		render_svg_props->stroke_linejoin = current_svg_props.stroke_linejoin;
	}
	if (current_svg_props.font_family && current_svg_props.font_family->type != SVGFontFamily_inherit) {
		render_svg_props->font_family->value.string = current_svg_props.font_family->value.string;
		render_svg_props->font_family->value.length = current_svg_props.font_family->value.length;
	}
	if (current_svg_props.font_size && current_svg_props.font_size->type != SVGFLOAT_INHERIT) {
		render_svg_props->font_size = current_svg_props.font_size;
	}
	if (current_svg_props.font_style && *current_svg_props.font_style != SVGFontStyle_inherit) {
		render_svg_props->font_style = current_svg_props.font_style;
	}
}

static void SVGSetViewport(RenderEffect2D *eff, SVGsvgElement *svg) 
{
	M4Matrix2D mat;
	Float real_width, real_height;

	mx2d_init(mat);

	real_width = (svg->width.unitType == SVG_LENGTHTYPE_PX?eff->surface->render->compositor->width:eff->surface->render->compositor->width*svg->width.number/100);
	real_height = (svg->height.unitType == SVG_LENGTHTYPE_PX?eff->surface->render->compositor->height:eff->surface->render->compositor->height*svg->height.number/100);

	if (svg->viewBox.width != 0 && svg->viewBox.height != 0) {
		mat.m[0] = real_width/svg->viewBox.width;
		mat.m[4] = real_height/svg->viewBox.height;
		mat.m[2] = -(svg->viewBox.x*real_width)/svg->viewBox.width; 
		mat.m[5] = -(svg->viewBox.y*real_height)/svg->viewBox.height; 
	}
	mx2d_add_matrix(&eff->transform, &mat);
}

static void SVG_Render_svg(SFNode *node, void *rs)
{
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);
	SVGsvgElement *svg = (SVGsvgElement *)node;
	RenderEffect2D *eff = (RenderEffect2D *) rs;

	if (!eff->svg_props) {
		SAFEALLOC(eff->svg_props, sizeof(SVGStylingProperties));
		SVGInitContext(eff->svg_props);
		Node_SetPrivate(node, eff->svg_props);
		mx2d_init(eff->transform);
		SVGSetViewport(eff, svg);
	}

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, svg->properties);	

	Node_RenderChildren(node, eff);

	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Destroy_svg(SFNode *node)
{
	SVGStylingProperties *svg_props = Node_GetPrivate(node);
	SVG_DestroyContext(svg_props);
}
void SVG_Init_svg(Render2D *sr, SFNode *node)
{
	Node_SetRenderFunction(node, SVG_Render_svg);
	Node_SetPreDestroyFunction(node, SVG_Destroy_svg);
}

static void SVG_Render_g(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	SVGgElement *g = (SVGgElement *)node;
	RenderEffect2D *eff = (RenderEffect2D *) rs;

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, g->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(g->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	Node_RenderChildren(node, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_g(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_g);
}

static void SVG_Render_rect(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGrectElement *rect = (SVGrectElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, rect->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(rect->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		Float rx = rect->rx.number;
		Float ry = rect->ry.number;
		Float x = rect->x.number;
		Float y = rect->y.number;
		Float width = rect->width.number;
		Float height = rect->height.number;
		drawable_reset_path(cs);
		if (rx || ry) {
			if (rx >= width/2) rx = width/2;
			if (ry >= height/2) ry = height/2;
			if (rx == 0) rx = ry;
			if (ry == 0) ry = rx;
			m4_path_add_move_to(cs->path, x+rx, y);
			m4_path_add_line_to(cs->path, x+width-rx, y);
			m4_path_add_quadratic_to(cs->path, x+width, y, x+width, y+ry);
			m4_path_add_line_to(cs->path, x+width, y+height-ry);
			m4_path_add_quadratic_to(cs->path, x+width, y+height, x+width-rx, y+height);
			m4_path_add_line_to(cs->path, x+rx, y+height);
			m4_path_add_quadratic_to(cs->path, x, y+height, x, y+height-ry);
			m4_path_add_line_to(cs->path, x, y+ry);
			m4_path_add_quadratic_to(cs->path, x, y+height, x, y+height-ry);
			m4_path_add_line_to(cs->path, x, y+ry);
			m4_path_add_quadratic_to(cs->path, x, y, x+rx, y);
			m4_path_close(cs->path);
		} else {
			m4_path_add_move_to(cs->path, x, y);
			m4_path_add_line_to(cs->path, x+width, y);		
			m4_path_add_line_to(cs->path, x+width, y+height);		
			m4_path_add_line_to(cs->path, x, y+height);		
			m4_path_close(cs->path);		
		}
		cs->node_changed = 1;
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_rect(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_rect);
}

static void SVG_Render_circle(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGcircleElement *circle = (SVGcircleElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, circle->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(circle->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		Float r = circle->r.number;
		Float cx = circle->cx.number;
		Float cy = circle->cy.number;
		M4Matrix2D tmp, translate;
		mx2d_init(translate);
		translate.m[2] = cx;
		translate.m[5] = cy;
		mx2d_copy(tmp, eff->transform);
		mx2d_copy(eff->transform, translate);
		mx2d_add_matrix(&eff->transform, &tmp);
		drawable_reset_path(cs);
		m4_path_add_ellipse(cs->path, 2*r, 2*r);
		cs->node_changed = 1;
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_circle(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_circle);
}

static void SVG_Render_ellipse(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGellipseElement *ellipse = (SVGellipseElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, ellipse->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(ellipse->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		Float rx = ellipse->rx.number;
		Float ry = ellipse->ry.number;
		Float cx = ellipse->cx.number;
		Float cy = ellipse->cy.number;
		M4Matrix2D tmp, translate;
		mx2d_init(translate);
		translate.m[2] = cx;
		translate.m[5] = cy;
		mx2d_copy(tmp, eff->transform);
		mx2d_copy(eff->transform, translate);
		mx2d_add_matrix(&eff->transform, &tmp);
		drawable_reset_path(cs);
		m4_path_add_ellipse(cs->path, 2*rx, 2*ry);
		cs->node_changed = 1;
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_ellipse(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_ellipse);
}

static void SVG_Render_line(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGlineElement *line = (SVGlineElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, line->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(line->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		Float x1 = line->x1.number;
		Float y1 = line->y1.number;
		Float x2 = line->x2.number;
		Float y2 = line->y2.number;
		drawable_reset_path(cs);
		m4_path_add_move_to(cs->path, x1, y1);
		m4_path_add_line_to(cs->path, x2, y2);
		cs->node_changed = 1;
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_line(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_line);
}

static void SVG_Render_polyline(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGpolylineElement *polyline = (SVGpolylineElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, polyline->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(polyline->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		u32 i;
		u32 nbPoints = ChainGetCount(polyline->points);
		drawable_reset_path(cs);
		if (nbPoints) {
			SVG_Point *p = ChainGetEntry(polyline->points, 0);
			m4_path_add_move_to(cs->path, p->x, p->y);
			for (i = 1; i < nbPoints; i++) {
				p = ChainGetEntry(polyline->points, i);
				m4_path_add_line_to(cs->path, p->x, p->y);
			}
			cs->node_changed = 1;
		} else {
			m4_path_add_move_to(cs->path, 0, 0);
		}
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_polyline(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_polyline);
}

static void SVG_Render_polygon(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGpolygonElement *polygon = (SVGpolygonElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, polygon->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(polygon->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		u32 i;
		u32 nbPoints = ChainGetCount(polygon->points);
		drawable_reset_path(cs);
		if (nbPoints) {
			SVG_Point *p = ChainGetEntry(polygon->points, 0);
			m4_path_add_move_to(cs->path, p->x, p->y);
			for (i = 1; i < nbPoints; i++) {
				p = ChainGetEntry(polygon->points, i);
				m4_path_add_line_to(cs->path, p->x, p->y);
			}
			m4_path_close(cs->path);
			cs->node_changed = 1;
		} else {
			m4_path_add_move_to(cs->path, 0, 0);
		}
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_polygon(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_polygon);
}

static void SVG_Render_path(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGpathElement *path = (SVGpathElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, path->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(path->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		u32 i, j;
		SVG_Point orig, ct_orig, ct_end, end, *tmp;
		drawable_reset_path(cs);

		m4_path_set_fill_mode(cs->path, *(eff->svg_props->fill_rule));
		
		for (i=0, j=0; i<ChainGetCount(path->d.path_commands); i++) {
			u8 *command = ChainGetEntry(path->d.path_commands, i);
			switch (*command) {
			case 0: /* Move To */
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&orig, tmp, sizeof(SVG_Point));
				m4_path_add_move_to(cs->path, orig.x, orig.y);
				j++;
				break;
			case 1: /* Line To */
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&end, tmp, sizeof(SVG_Point));
				m4_path_add_line_to(cs->path, end.x, end.y);
				memcpy(&orig, &end, sizeof(SVG_Point));
				j++;
				break;
			case 2: /* Curve To */
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&ct_orig, tmp, sizeof(SVG_Point));
				tmp = ChainGetEntry(path->d.path_points, j+1);
				memcpy(&ct_end, tmp, sizeof(SVG_Point));
				tmp = ChainGetEntry(path->d.path_points, j+2);
				memcpy(&end, tmp, sizeof(SVG_Point));				 
				m4_path_add_cubic_to(cs->path, ct_orig.x, ct_orig.y, ct_end.x, ct_end.y, end.x, end.y);
				memcpy(&ct_orig, &ct_end, sizeof(SVG_Point));				 
				memcpy(&orig, &end, sizeof(SVG_Point));				 
				j+=3;
				break;
			case 3: /* Next Curve To */
				ct_orig.x = 2*orig.x - ct_orig.x;
				ct_orig.y = 2*orig.y - ct_orig.y;
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&ct_end, tmp, sizeof(SVG_Point));
				tmp = ChainGetEntry(path->d.path_points, j+1);
				memcpy(&end, tmp, sizeof(SVG_Point));
				m4_path_add_cubic_to(cs->path, ct_orig.x, ct_orig.y, ct_end.x, ct_end.y, end.x, end.y);
				memcpy(&ct_orig, &ct_end, sizeof(SVG_Point));				 
				memcpy(&orig, &end, sizeof(SVG_Point));				 
				j+=2;
				break;
			case 4: /* Quadratic Curve To */
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&ct_orig, tmp, sizeof(SVG_Point));
				tmp = ChainGetEntry(path->d.path_points, j+1);
				memcpy(&end, tmp, sizeof(SVG_Point));
				m4_path_add_quadratic_to(cs->path, ct_orig.x, ct_orig.y, end.x, end.y);			
				memcpy(&orig, &end, sizeof(SVG_Point));				 
				j+=2;
				break;
			case 5: /* Next Quadratic Curve To */
				ct_orig.x = 2*orig.x - ct_orig.x;
				ct_orig.y = 2*orig.y - ct_orig.y;
				tmp = ChainGetEntry(path->d.path_points, j);
				memcpy(&end, tmp, sizeof(SVG_Point));
				m4_path_add_quadratic_to(cs->path, ct_orig.x, ct_orig.y, end.x, end.y);
				memcpy(&orig, &end, sizeof(SVG_Point));				 
				j++;
				break;
			case 6: /* Close */
				m4_path_close(cs->path);
				break;
			}
		}
		cs->node_changed = 1;
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_path(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_path);
}

static void SVG_Render_use(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGuseElement *use = (SVGuseElement *)node;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, use->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(use->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}
	mx2d_add_translation(&eff->transform, use->x.number, use->y.number);

	if (Node_GetDirty(node)) {
		Node_Render((SFNode *)use->xlink_href.target_element, eff);
	}
	ctx = SVG_drawable_init_context(cs, eff);
	if (!ctx) return;
			
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);

	mx2d_copy(eff->transform, backup_matrix);  
	memcpy(eff->svg_props, &backup_props, styling_size);
}

void SVG_Init_use(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_use);
}

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	Bool enabled;
	SensorHandler hdl;
} SVG_Stack_a;

static void SVG_Destroy_a(SFNode *n)
{
	SVG_Stack_a *st = (SVG_Stack_a*)Node_GetPrivate(n);
	R2D_UnregisterSensor(st->compositor, &st->hdl);
	if (st->compositor->interaction_sensors) st->compositor->interaction_sensors--;
	DeleteGroupingNode2D((GroupingNode2D *)st);
	free(st);
}

static void SVG_Render_a(SFNode *node, void *rs)
{
	SVG_Stack_a *st = (SVG_Stack_a *) Node_GetPrivate(node);
	SVGaElement *a = (SVGaElement *) node;
	RenderEffect2D *eff = rs;

	/*update enabled state*/
	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		st->enabled = 1;
	}
	/*note we don't clear dirty flag, this is done in traversing*/
	group2d_traverse((GroupingNode2D*)st, a->children, eff);
}

static Bool SVG_IsEnabled_a(SensorHandler *sh)
{
	return 1;
}

static void SVG_OnUserEvent_a(SensorHandler *sh, UserEvent2D *ev, M4Matrix2D *sensor_matrix)
{
	SVG_Stack_a *st;
	M4Event evt;
	SVGaElement *a;
	if (ev->event_type != M4E_LEFTUP) return;
	st = (SVG_Stack_a *) Node_GetPrivate(sh->owner);
	a = (SVGaElement *) sh->owner;

	if (!st->compositor->client->EventProc) return;
	evt.type = M4E_NAVIGATE;
	
	if (a->xlink_href.type == SVGUri_uri) {
		evt.navigate.to_url = a->xlink_href.uri.string;
		if (evt.navigate.to_url) {
			st->compositor->client->EventProc(st->compositor->client->opaque, &evt);
		}
	}
}

SensorHandler *SVG_GetHandler_a(SFNode *n)
{
	SVG_Stack_a *st = (SVG_Stack_a *) Node_GetPrivate(n);
	return &st->hdl;
}

void SVG_Init_a(Render2D *sr, SFNode *node)
{
	SVG_Stack_a *stack;
	SAFEALLOC(stack, sizeof(SVG_Stack_a))

	SetupGroupingNode2D((GroupingNode2D*)stack, sr, node);

	sr->compositor->interaction_sensors++;

	stack->hdl.IsEnabled = SVG_IsEnabled_a;
	stack->hdl.OnUserEvent = SVG_OnUserEvent_a;
	stack->hdl.owner = node;
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, SVG_Destroy_a);
	Node_SetRenderFunction(node, SVG_Render_a);
}

#endif //M4_DISABLE_SVG
