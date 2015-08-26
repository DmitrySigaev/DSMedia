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

static void build_graph(Drawable *cs, M_IndexedFaceSet2D *ifs2D)
{
	u32 i;
	SFVec2f *pts;
	u32 ci_count, c_count;
	Bool started;
	M_Coordinate2D *coord = (M_Coordinate2D *)ifs2D->coord;

	c_count = coord->point.count;
	ci_count = ifs2D->coordIndex.count;
	pts = coord->point.vals;

	if (ci_count > 0) {
		started = 0;
		for (i=0; i < ci_count; i++) {
			if (ifs2D->coordIndex.vals[i] == -1) {
				m4_path_close(cs->path);
				started = 0;
			} else if (!started) {
				started = 1;
				m4_path_add_move_to(cs->path, pts[ifs2D->coordIndex.vals[i]].x, pts[ifs2D->coordIndex.vals[i]].y);
			} else {
				m4_path_add_line_to(cs->path, pts[ifs2D->coordIndex.vals[i]].x, pts[ifs2D->coordIndex.vals[i]].y);
			}
		}
		if (started) m4_path_close(cs->path);
	} else if (c_count) {
		m4_path_add_move_to(cs->path, pts[0].x, pts[0].y);
		for (i=1; i < c_count; i++) {
			m4_path_add_line_to(cs->path, pts[i].x, pts[i].y);
		}
		m4_path_close(cs->path);
	}
}

static void RenderIFS2D(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	M_IndexedFaceSet2D *ifs2D = (M_IndexedFaceSet2D *)node;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (!ifs2D->coord) return;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		build_graph(cs, ifs2D);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}
	
	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}


static void IFS2D_Draw(DrawableContext *ctx)
{
	u32 i, count, ind_col, num_col, j, start_pts, ci_count;
	LPM4PATH path;
	SFVec2f center, start, end;
	SFVec2f *pts;
	Graphics2DDriver *g_hw;
	LPM4STENCIL grad;
	u32 *colors;
	SFColor col, col_cen;
	Float alpha;
	M_IndexedFaceSet2D *ifs2D = (M_IndexedFaceSet2D *)ctx->node->owner;
	M_Coordinate2D *coord = (M_Coordinate2D*) ifs2D->coord;
	M_Color *color = (M_Color *) ifs2D->color;

	/*simple case, no color specified*/
	if (!ifs2D->color) {
		VS2D_TexturePath(ctx->surface, ctx->node->path, ctx);
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
		return;
	}

	/*if default face use first color*/
	ci_count = ifs2D->coordIndex.count;
	pts = coord->point.vals;

	if (ci_count == 0) {
		col = (ifs2D->colorIndex.count > 0) ? color->color.vals[ifs2D->colorIndex.vals[0]] : color->color.vals[0];

		if (!ctx->aspect.filled || !ctx->aspect.has_line) {
			alpha = (Float) M4C_A(ctx->aspect.line_color) / 255;
			ctx->aspect.line_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
		} else {
			alpha = (Float) M4C_A(ctx->aspect.fill_color) / 255;
			ctx->aspect.fill_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
		}
		VS2D_TexturePath(ctx->surface, ctx->node->path, ctx);
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
		return;
	}

	/*we have color per faces so we need N path :(*/
	if (! ifs2D->colorPerVertex) {
		path = m4_new_path();

		count = 0;
		i = 0;
		while (1) {
			m4_path_reset(path);
			start = pts[ifs2D->coordIndex.vals[i]];
			m4_path_add_move_to(path, start.x, start.y);
			i++;

			while (ifs2D->coordIndex.vals[i] != -1) {	
				start = pts[ifs2D->coordIndex.vals[i]];
				m4_path_add_line_to(path, start.x, start.y);
				i++;
				if (i >= ci_count) break;
			}
			/*close in ALL cases because even if the start/end points are the same the line join needs to be present*/
			m4_path_close(path);

			col = (ifs2D->colorIndex.count > 0) ? color->color.vals[ifs2D->colorIndex.vals[count]] : color->color.vals[count];

			if (!ctx->aspect.filled) {
				alpha = (Float) M4C_A(ctx->aspect.line_color) / 255;
				ctx->aspect.line_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			} else {
				alpha = (Float) M4C_A(ctx->aspect.fill_color) / 255;
				ctx->aspect.fill_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			}

			VS2D_TexturePath(ctx->surface, path, ctx);
			VS2D_DrawPath(ctx->surface, path, ctx, NULL, NULL);
			count++;
			i++;
			if (i >= ci_count) break;
			ctx->path_filled = ctx->path_stroke = 0;
		}
		m4_path_delete(path);
		return;
	}

	/*final case, color per vertex means gradient fill/strike*/
	g_hw = ctx->surface->render->compositor->g_hw;
	grad = g_hw->new_stencil(g_hw, M4StencilVertexGradient);
	/*not supported, fill default*/
	if (!grad) {
		VS2D_TexturePath(ctx->surface, ctx->node->path, ctx);
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
		return;
	}


	path = m4_new_path();

	ind_col = 0;
	i = 0;
	while (1) {
		m4_path_reset(path);
		start = pts[ifs2D->coordIndex.vals[i]];
		center = start;
		m4_path_add_move_to(path, start.x, start.y);
		start_pts = i;
		num_col = 1;
		i+=1;
		while (ifs2D->coordIndex.vals[i] != -1) {	
			end = pts[ifs2D->coordIndex.vals[i]];
			m4_path_add_line_to(path, end.x, end.y);
			i++;
			center.x += end.x;
			center.y += end.y;
			num_col ++;
			if (i >= ci_count) break;
		}
		m4_path_close(path);
		num_col++;

		alpha = (Float) (M4C_A(ctx->aspect.fill_color)/255);

		colors = malloc(sizeof(u32) * num_col);
		col_cen.blue = col_cen.red = col_cen.green = 0;
		for (j=0; j<num_col-1; j++) {
			if (ifs2D->colorIndex.count > ind_col + j) {
				col = color->color.vals[ifs2D->colorIndex.vals[ind_col + j]];
			} else if (ci_count > ind_col + j) {
				col = color->color.vals[ifs2D->coordIndex.vals[ind_col + j]];
			}
			colors[j] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			col_cen.blue += col.blue;
			col_cen.green += col.green;
			col_cen.red += col.red;
		}
		colors[num_col-1] = colors[0];

		if (ifs2D->colorIndex.count > ind_col) {
			col = color->color.vals[ifs2D->colorIndex.vals[ind_col]];
		} else if (ci_count > ind_col) {
			col = color->color.vals[ifs2D->coordIndex.vals[ind_col]];
		}
		col_cen.blue += col.blue;
		col_cen.green += col.green;
		col_cen.red += col.red;

		g_hw->set_vertex_path(grad, path);
		g_hw->set_vertex_colors(grad, colors, num_col);

		free(colors);
		
		col_cen.blue /= num_col;
		col_cen.green /= num_col;
		col_cen.red /= num_col;
		center.x /= num_col;
		center.y /= num_col;
		g_hw->set_vertex_center(grad, center.x, center.y, MAKE_ARGB_FLOAT(alpha, col_cen.red, col_cen.green, col_cen.blue) );

		g_hw->stencil_set_matrix(grad, &ctx->transform);

		/*draw*/
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, grad, grad);

		g_hw->delete_stencil(grad);

		//goto next point
		i++;
		ind_col += num_col + 1;	
		if (i >= ci_count) break;
		grad = g_hw->new_stencil(g_hw, M4StencilVertexGradient);
		ctx->path_filled = ctx->path_stroke = 0;
	}
	m4_path_delete(path);
}

static void IFS2D_SetColorIndex(SFNode *node)
{
	M_IndexedFaceSet2D *ifs2D = (M_IndexedFaceSet2D *)node;
	VRML_FieldCopy(&ifs2D->colorIndex, &ifs2D->set_colorIndex, FT_MFInt32);
}

static void IFS2D_SetCoordIndex(SFNode *node)
{
	M_IndexedFaceSet2D *ifs2D = (M_IndexedFaceSet2D *)node;
	VRML_FieldCopy(&ifs2D->coordIndex, &ifs2D->set_coordIndex, FT_MFInt32);
}

void R2D_InitIFS2D(Render2D *sr, SFNode *node)
{
	M_IndexedFaceSet2D *ifs2D = (M_IndexedFaceSet2D *)node;
	Drawable * stack = BaseDrawStack2D(sr, node);
	/*override draw*/
	stack->Draw = IFS2D_Draw;
	Node_SetRenderFunction(node, RenderIFS2D);
	ifs2D->on_set_colorIndex = IFS2D_SetColorIndex;
	ifs2D->on_set_coordIndex = IFS2D_SetCoordIndex;
}

