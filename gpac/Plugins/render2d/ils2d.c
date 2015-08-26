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

static void build_graph(Drawable *cs, M_IndexedLineSet2D *ils2D)
{
	u32 i;
	Bool started;
	SFVec2f *pts;
	M_Coordinate2D *coord = (M_Coordinate2D *)ils2D->coord;

	pts = coord->point.vals;
	if (ils2D->coordIndex.count > 0) {
		started = 0;
		for (i=0; i < ils2D->coordIndex.count; i++) {
			/*NO close on ILS2D*/
			if (ils2D->coordIndex.vals[i] == -1) {
				started = 0;
			} else if (!started) {
				started = 1;
				m4_path_add_move_to(cs->path, pts[ils2D->coordIndex.vals[i]].x, pts[ils2D->coordIndex.vals[i]].y);
			} else {
				m4_path_add_line_to(cs->path, pts[ils2D->coordIndex.vals[i]].x, pts[ils2D->coordIndex.vals[i]].y);
			}
		}
	} else if (coord->point.count) {
		m4_path_add_move_to(cs->path, pts[0].x, pts[0].y);
		for (i=1; i < coord->point.count; i++) {
			m4_path_add_line_to(cs->path, pts[i].x, pts[i].y);
		}
	}
}


static void RenderILS2D(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	M_IndexedLineSet2D *ils2D = (M_IndexedLineSet2D *)node;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;

	if (!ils2D->coord) return;

	if (Node_GetDirty(node)) {
		drawable_reset_path(cs);
		build_graph(cs, ils2D);
		Node_ClearDirty(node);
		cs->node_changed = 1;
	}

	ctx = drawable_init_context(cs, eff);
	if (!ctx) return;
	/*ILS2D are NEVER filled*/
	ctx->aspect.filled = 0;
	drawctx_store_original_bounds(ctx);
	drawable_finalize_render(ctx, eff);
}

static void ILS2D_Draw(DrawableContext *ctx)
{
	LPM4PATH path;
	LPM4STENCIL grad;
	SFVec2f start, end, *pts;
	SFColor col;
	Graphics2DDriver *g_hw;
	Float alpha;
	u32 i, j, count, num_col, col_ind, ind, end_at;
	u32 linear[2], *colors;
	M_IndexedLineSet2D *ils2D = (M_IndexedLineSet2D *)ctx->node->owner;
	M_Coordinate2D *coord = (M_Coordinate2D*) ils2D->coord;
	M_Color *color = (M_Color *) ils2D->color;


	if (!coord->point.count) return;

	if (! ils2D->color) {
		/*no texturing*/
		VS2D_DrawPath(ctx->surface, ctx->node->path, ctx, NULL, NULL);
		return;
	}
	
	alpha = (Float) M4C_A(ctx->aspect.line_color) / 255;
	pts = coord->point.vals;

	if (!ils2D->colorPerVertex || (color->color.count<2) ) {
		count = 0;
		end_at = ils2D->coordIndex.count;
		if (!end_at) end_at = coord->point.count;
		ind = ils2D->coordIndex.count ? ils2D->coordIndex.vals[0] : 0;
		i=1;
		path = m4_new_path();
		m4_path_add_move_to(path, pts[ind].x, pts[ind].y);

		for (; i<=end_at; i++) {
			if ((i==end_at) || (ils2D->coordIndex.count && ils2D->coordIndex.vals[i] == -1)) {

				/*draw current*/
				col_ind = (ils2D->colorIndex.count) ? ils2D->colorIndex.vals[count] : count;
				if (col_ind>=color->color.count) col_ind=color->color.count-1;
				col = color->color.vals[col_ind];
				ctx->aspect.line_color = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);

				VS2D_DrawPath(ctx->surface, path, ctx, NULL, NULL);

				i++;
				if (i>=end_at) break;
				m4_path_reset(path);

				ind = ils2D->coordIndex.count ? ils2D->coordIndex.vals[i] : i;
				m4_path_add_move_to(path, pts[ind].x, pts[ind].y);

				if (ils2D->coordIndex.count) count++;
				continue;
			} else {
				ind = ils2D->coordIndex.count ? ils2D->coordIndex.vals[i] : i;
				m4_path_add_line_to(path, pts[ind].x, pts[ind].y);
			}
		}
		m4_path_delete(path);
		return;
	}

	g_hw = NULL;
	end_at = ils2D->coordIndex.count;
	if (!end_at) end_at = coord->point.count;
	count = 0;
	col_ind = 0;
	ind = 0;
	i=0;
	path = m4_new_path();
	while (1) {
		m4_path_reset(path);
		ind = ils2D->coordIndex.count ? ils2D->coordIndex.vals[i] : i;
		start = pts[ind];
		num_col = 1;
		i++;
		m4_path_add_move_to(path, start.x, start.y);

		if (ils2D->coordIndex.count) {
			while (ils2D->coordIndex.vals[i] != -1) {
				end = pts[ils2D->coordIndex.vals[i]];
				m4_path_add_line_to(path, end.x, end.y);
				i++;
				num_col++;
				if (i >= ils2D->coordIndex.count) break;
			}
		} else {
			while (i<end_at) {
				end = pts[i];
				m4_path_add_line_to(path, end.x, end.y);
				i++;
				num_col++;
			}
		}

		g_hw = ctx->surface->render->compositor->g_hw;
		/*use linear gradient*/
		if (num_col==2) {
			grad = g_hw->new_stencil(g_hw, M4StencilLinearGradient);
			if (ils2D->colorIndex.count) {
				col = color->color.vals[ils2D->colorIndex.vals[col_ind]];
				linear[0] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
				col = color->color.vals[ils2D->colorIndex.vals[col_ind+1]];
				linear[1] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			} else if (ils2D->coordIndex.count) {
				col = color->color.vals[ils2D->coordIndex.vals[col_ind]];
				linear[0] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
				col = color->color.vals[ils2D->coordIndex.vals[col_ind+1]];
				linear[1] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			} else {
				col = color->color.vals[col_ind];
				linear[0] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
				col = color->color.vals[col_ind+1];
				linear[1] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
			}
			g_hw->set_linear_gradient(grad, start.x, start.y, end.x, end.y, linear[0], linear[1]);
		} else {
			grad = g_hw->new_stencil(g_hw, M4StencilVertexGradient);
			if (grad) {
				g_hw->set_vertex_path(grad, path);

				colors = malloc(sizeof(u32) * num_col);
				for (j=0; j<num_col; j++) {
					if (ils2D->colorIndex.count>0) {
						col = color->color.vals[ils2D->colorIndex.vals[col_ind+j]];
					} else if (ils2D->coordIndex.count) {
						col = color->color.vals[ils2D->coordIndex.vals[col_ind+j]];
					} else {
						col = color->color.vals[col_ind+j];
					}
					colors[j] = MAKE_ARGB_FLOAT(alpha, col.red, col.green, col.blue);
				}
				g_hw->set_vertex_colors(grad, colors, num_col);
				free(colors);
			}
		}
		g_hw->stencil_set_matrix(grad, &ctx->transform);
		VS2D_DrawPath(ctx->surface, path, ctx, NULL, grad);
		if (grad) g_hw->delete_stencil(grad);

		i ++;
		col_ind += num_col + 1;
		if (i >= ils2D->coordIndex.count) break;
		ctx->path_stroke = 0;
	}
	m4_path_delete(path);

}

static void ILS2D_SetColorIndex(SFNode *node)
{
	M_IndexedLineSet2D *ils2D = (M_IndexedLineSet2D *)node;
	VRML_FieldCopy(&ils2D->colorIndex, &ils2D->set_colorIndex, FT_MFInt32);
}

static void ILS2D_SetCoordIndex(SFNode *node)
{
	M_IndexedLineSet2D *ils2D = (M_IndexedLineSet2D *)node;
	VRML_FieldCopy(&ils2D->coordIndex, &ils2D->set_coordIndex, FT_MFInt32);
}

void R2D_InitILS2D(Render2D *sr, SFNode *node)
{
	M_IndexedLineSet2D *ils2D = (M_IndexedLineSet2D *)node;
	Drawable * stack = BaseDrawStack2D(sr, node);
	/*override draw*/
	stack->Draw = ILS2D_Draw;
	Node_SetRenderFunction(node, RenderILS2D);
	ils2D->on_set_colorIndex = ILS2D_SetColorIndex;
	ils2D->on_set_coordIndex = ILS2D_SetCoordIndex;
}
