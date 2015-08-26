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

/*default value when no fontStyle*/
#define FSFAMILY	(fs && fs->family.count) ? (const char *)fs->family.vals[0]	: ""

/*here it's tricky since it depends on our metric system...*/
#define FSSIZE		(fs ? fs->size : -1)
#define FSSTYLE		(fs && fs->style.buffer) ? (const char *)fs->style.buffer : ""
#define FSMAJOR		( (fs && fs->justify.count && fs->justify.vals[0]) ? (const char *)fs->justify.vals[0] : "FIRST")
#define FSMINOR		( (fs && (fs->justify.count>1) && fs->justify.vals[1]) ? (const char *)fs->justify.vals[1] : "FIRST")
#define FSHORIZ		(fs ? fs->horizontal : 1)
#define FSLTR		(fs ? fs->leftToRight : 1)
#define FSTTB		(fs ? fs->topToBottom : 1)
#define FSLANG		(fs ? fs->language : "")
#define FSSPACE		(fs ? fs->spacing : 1)


void text2D_get_ascent_descent(DrawableContext *ctx, Float *a, Float *d)
{
	TextStack2D *stack = (TextStack2D *) Node_GetPrivate(ctx->node->owner);
	*a = stack->ascent;
	*d = stack->descent;
}

typedef struct
{
	/*regular drawing*/
	LPM4PATH path;
	M4Rect bounds;

	/*texture drawing*/	
	LPHWTEXTURE hwtx;
	Render2D *sr;
	LPM4PATH tx_path;
	Bool tx_ready;
	/*take only into account the window-scaling zoom, not any navigation one (would be too big)*/
	SFVec2f last_scale;
	M4Rect tx_bounds;
	Bool failed;
} TextLineEntry2D;


TextLineEntry2D *NewTextLine2D(Render2D *sr)
{
	TextLineEntry2D *tl;
	SAFEALLOC(tl, sizeof(TextLineEntry2D));
	tl->path = m4_new_path();
	tl->path->fineness = 1.0;
	/*text texturing enabled*/
	tl->sr = sr;
	tl->last_scale.x = sr->scale_x;
	tl->last_scale.y = sr->scale_y;
	return tl;
}

void TextLine_StoreBounds(TextLineEntry2D *tl)
{
	tl->bounds.x = tl->path->min_x;
	tl->bounds.y = tl->path->max_y;
	tl->bounds.width = (tl->path->max_x-tl->path->min_x);
	tl->bounds.height = (tl->path->max_y-tl->path->min_y);
}

Bool TextLine2D_TextureIsReady(TextLineEntry2D *tl)
{
	M4Matrix2D mx;
	LPM4STENCIL stenc;
	LPM4SURFACE surf;
	Float cx, cy;
	u32 width, height;
	Float scale;
	M4Err e;
	Graphics2DDriver *g_hw = tl->sr->compositor->g_hw;
	
	/*something failed*/
	if (tl->failed) return 0;
	if (!tl->hwtx) tl->hwtx = g_hw->new_stencil(g_hw, M4StencilTexture);

	if (tl->tx_ready) {
		if ((tl->last_scale.x == tl->sr->scale_x)
			&& (tl->last_scale.y == tl->sr->scale_y)
			) return 1;

		if (tl->hwtx) g_hw->delete_stencil(tl->hwtx);
		if (tl->tx_path) m4_path_delete(tl->tx_path);
		tl->tx_path = NULL;
		tl->hwtx = g_hw->new_stencil(g_hw, M4StencilTexture);

		tl->last_scale.x = tl->sr->scale_x;
		tl->last_scale.y = tl->sr->scale_y;
	}

	scale = MAX(tl->last_scale.x, tl->last_scale.y);
	if ((scale*tl->bounds.width>512) || (scale*tl->bounds.height>512)) {
		scale = MIN(512/tl->bounds.width, 512/tl->bounds.height);
	}


	width = (u32) ceil(scale*tl->bounds.width);
	height = (u32) ceil(scale*tl->bounds.height);
/*	if (width%1) width += 1; 
	if (height%1) height += 1;
	//width += 1; 
	height += 1;
*/
	width += 1; 
	height += 1;

	surf = g_hw->new_surface(g_hw, 1);
	if (!surf) {
		g_hw->delete_stencil(tl->hwtx);
		tl->hwtx = NULL;
		tl->failed = 1;
		return 0;
	}
	/*FIXME - make it work with alphagrey...*/
	e = g_hw->stencil_create_texture(tl->hwtx, width, height, M4PF_ARGB);
	if (!e) e = g_hw->attach_surface_to_texture(surf, tl->hwtx);
	g_hw->surface_clear(surf, NULL, 0);
	stenc = g_hw->new_stencil(g_hw, M4StencilSolid);
	g_hw->set_brush_color(stenc, 0xFF000000);

	cx = (tl->path->max_x + tl->path->min_x) / 2.0f; 
	cy = (tl->path->max_y + tl->path->min_y) / 2.0f;

	mx2d_init(mx);
	mx2d_add_translation(&mx, -cx, -cy);
	mx2d_add_scale(&mx, scale, scale);
	mx2d_add_translation(&mx, 0.3f, 0.3f);
	g_hw->surface_set_matrix(surf, &mx);

	g_hw->surface_set_raster_level(surf, M4RasterHighQuality);
	g_hw->surface_set_path(surf, tl->path);
	g_hw->surface_fill(surf, stenc);
	g_hw->delete_stencil(stenc);
	g_hw->delete_surface(surf);

	tl->tx_path = m4_new_path();
	m4_path_add_move_to(tl->tx_path, tl->path->min_x, tl->path->min_y);
	m4_path_add_line_to(tl->tx_path, tl->path->max_x, tl->path->min_y);
	m4_path_add_line_to(tl->tx_path, tl->path->max_x, tl->path->max_y);
	m4_path_add_line_to(tl->tx_path, tl->path->min_x, tl->path->max_y);
	m4_path_close(tl->tx_path);

	tl->tx_bounds.x = tl->tx_bounds.y = 0;
	tl->tx_bounds.width = (Float) width;
	tl->tx_bounds.height = (Float) height;

	if (e) {
		g_hw->delete_stencil(tl->hwtx);
		tl->hwtx = NULL;
		tl->failed = 1;
		return 0;
	}
	tl->tx_ready = 1;
	return 1;
}



void TextStack2D_clean_paths(TextStack2D *stack)
{
	TextLineEntry2D *tl;
	/*delete all path objects*/
	while (ChainGetCount(stack->text_lines)) {
		tl = ChainGetEntry(stack->text_lines, 0);
		ChainDeleteEntry(stack->text_lines, 0);
		if (tl->path) m4_path_delete(tl->path);
		if (tl->hwtx) tl->sr->compositor->g_hw->delete_stencil(tl->hwtx);
		if (tl->tx_path) m4_path_delete(tl->tx_path);
		free(tl);
	}
	m4_rect_reset(&stack->bounds);
	drawable_reset_path(stack->graph);
}

static void DestroyText(SFNode *node)
{
	TextStack2D *stack = (TextStack2D *) Node_GetPrivate(node);
	TextStack2D_clean_paths(stack);
	DeleteDrawableNode(stack->graph);
	DeleteChain(stack->text_lines);
	free(stack);
}


static void split_text_letters(TextStack2D *st, M_Text *txt, RenderEffect2D *eff)
{
	unsigned short wcTemp[5000];
	unsigned short letter[2];
	DrawableContext *ctx;
	TextLineEntry2D *tl;
	u32 i, j, len;
	Float fontSize, start_y, font_height, line_spacing;
	M4Rect rc;
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
	M_FontStyle *fs = (M_FontStyle *)txt->fontStyle;

	fontSize = FSSIZE;
	if (fontSize <= 0) {
		fontSize = 12;
		if (!R2D_IsPixelMetrics((SFNode *)txt)) fontSize /= eff->surface->render->cur_width;
    }
	line_spacing = FSSPACE * fontSize;

	if (ft_dr->set_font(ft_dr, FSFAMILY, FSSTYLE) != M4OK) {
		if (ft_dr->set_font(ft_dr, NULL, FSSTYLE) != M4OK) {
			return;
		}
	}
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);
	/*adjust size*/
	fontSize *= fontSize / (font_height);
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);

			
	if (!strcmp(FSMINOR, "MIDDLE")) {
		start_y = (st->descent + st->ascent)/2;
	}
	else if (!strcmp(FSMINOR, "BEGIN")) {
		start_y = st->descent;
	}
	else if (!strcmp(FSMINOR, "END")) {
		start_y = st->descent + st->ascent;
	}
	else {
		start_y = st->ascent;
	}
	
	m4_rect_reset(&st->bounds);

	for (i=0; i < txt->string.count; i++) {

		char *str = txt->string.vals[i];
		if (!str) continue;
		len = utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
		if (len == (size_t) (-1)) continue;

		letter[1] = (unsigned short) 0;
		for (j=0; j<len; j++) {
			if (FSLTR) {
				letter[0] = wcTemp[j];
			} else {
				letter[0] = wcTemp[len - j - 1];
			}
			/*request a context (first one is always valid when entering render)*/
			if (j) group2d_start_child(eff->parent);
			ctx = drawable_init_context(st->graph, eff);
			if (!ctx) return;

			ctx->is_text = 1;

			tl = NewTextLine2D(eff->surface->render);

			ChainAddEntry(st->text_lines, tl);
			ctx->sub_path_index = ChainGetCount(st->text_lines);

			ft_dr->add_text_to_path(ft_dr, tl->path, 1, letter, 0, start_y, 1, 1, st->ascent, &rc);
			ctx->original.width = rc.width;
			ctx->original.x = rc.x;
			ctx->original.height = MAX(st->ascent + st->descent, rc.height);
			ctx->original.y = start_y;

			TextLine_StoreBounds(tl);

			drawable_finalize_render(ctx, eff);
			group2d_end_child(eff->parent);
		}
	}
}

static void split_text_words(TextStack2D *st, M_Text *txt, RenderEffect2D *eff)
{
	unsigned short wcTemp[5000];
	unsigned short letter[5000];
	DrawableContext *ctx;
	TextLineEntry2D *tl;
	u32 i, j, len, k, first_char;
	Float fontSize, font_height, line_spacing;
	M4Rect rc;
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
	M_FontStyle *fs = (M_FontStyle *)txt->fontStyle;

	fontSize = FSSIZE;
	if (fontSize <= 0) {
		fontSize = 12;
		if (!R2D_IsPixelMetrics((SFNode *)txt)) fontSize /= eff->surface->render->cur_width;
    }
	line_spacing = FSSPACE * fontSize;

	if (ft_dr->set_font(ft_dr, FSFAMILY, FSSTYLE) != M4OK) {
		if (ft_dr->set_font(ft_dr, NULL, FSSTYLE) != M4OK) {
			return;
		}
	}
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);
	/*adjust size*/
	fontSize *= fontSize / (font_height);
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);

	m4_rect_reset(&st->bounds);

	for (i=0; i < txt->string.count; i++) {
		char *str = txt->string.vals[i];
		if (!str) continue;
		len = utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
		if (len == (size_t) (-1)) continue;

		first_char = 0;
		for (j=0; j<len; j++) {
			/*we currently only split sentences at spaces*/
			if ((j+1!=len) && (wcTemp[j] != (unsigned short) ' ')) continue;

			if (FSLTR) {
				for (k=0; k<=j - first_char; k++) letter[k] = wcTemp[first_char+k];
			} else {
				for (k=0; k<=j - first_char; k++) letter[k] = wcTemp[len - first_char - k - 1];
			}
			letter[k] = (unsigned short) 0;
			/*request a context (first one is always valid when entering render)*/
			if (first_char) group2d_start_child(eff->parent);
			
			ctx = drawable_init_context(st->graph, eff);
			if (!ctx) return;

			ctx->is_text = 1;
			tl = NewTextLine2D(eff->surface->render);

			ChainAddEntry(st->text_lines, tl);
			ctx->sub_path_index = ChainGetCount(st->text_lines);

			/*word splitting only happen in layout, so we don't need top/left anchors*/
			ft_dr->add_text_to_path(ft_dr, tl->path, 1, letter, 0, 0, 1, 1, st->ascent, &rc);
			m4_path_get_bounds(tl->path, &ctx->original);
			ctx->original.width = rc.width;
			if (ctx->original.x != 0) ctx->original.width -= ctx->original.x;
			ctx->original.x = 0;
			ctx->original.height = MAX(st->ascent + st->descent, ctx->original.height);
			if (ctx->original.y != 0) ctx->original.height -= ctx->original.y;
			ctx->original.y = 0;

			TextLine_StoreBounds(tl);

			drawable_finalize_render(ctx, eff);
			group2d_end_child(eff->parent);

			first_char = j+1;
		}
	}
}

/*for vert and horiz text*/
typedef struct
{
	unsigned short *wcText;
	u32 length;
	Float width, height;
	Float x_scaling, y_scaling;
} TextLine2D;

static void BuildVerticalTextGraph(TextStack2D *st, M_Text *txt, RenderEffect2D *eff)
{
	TextLine2D *lines;
	unsigned short wcTemp[5000];
	u32 i, int_major, len, k;
	Float fontSize, start_x, start_y, space, line_spacing, tot_width, tot_height, max_scale;
	M4Rect rc, final;
	Float lw, lh, max_lw;
	unsigned short letter[2];
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
	M_FontStyle *fs = (M_FontStyle *)txt->fontStyle;

	fontSize = FSSIZE;
	if (fontSize <= 0) {
		fontSize = 12;
		if (!R2D_IsPixelMetrics((SFNode *)txt)) fontSize /= eff->surface->render->cur_width;
    }

	line_spacing = FSSPACE * fontSize;

	if (ft_dr->set_font(ft_dr, FSFAMILY, FSSTYLE) != M4OK) {
		if (ft_dr->set_font(ft_dr, NULL, FSSTYLE) != M4OK) {
			return;
		}
	}
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &space);


	/*compute overall bounding box size*/
	tot_width = 0;
	tot_height = 0;
	lines = (TextLine2D *) malloc(sizeof(TextLine2D)*txt->string.count);
	memset(lines, 0, sizeof(TextLine2D)*txt->string.count);
		
	letter[1] = (unsigned short) '\0';

/*	space = st->ascent + st->descent;
	space = fontSize - st->ascent + st->descent;
*/

	for (i=0; i < txt->string.count; i++) {
		char *str = txt->string.vals[i];
		if (!str) continue;
		lines[i].length = 0;
		len = utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
		if (len == (size_t) (-1)) continue;

		lines[i].wcText = malloc(sizeof(unsigned short) * len);
		memcpy(lines[i].wcText, wcTemp, sizeof(unsigned short) * len);
		lines[i].length = len;
		
		lines[i].y_scaling = lines[i].x_scaling = 1.0;
		lines[i].height = len * space;
		if (!lines[i].height) continue;

		if ((txt->length.count>i) && (txt->length.vals[i]>0) ) 
			lines[i].y_scaling = txt->length.vals[i] / lines[i].height;
	
		if (tot_height < lines[i].height * lines[i].y_scaling) tot_height = lines[i].height * lines[i].y_scaling;
	}

	tot_width = txt->string.count * line_spacing;
	st->bounds.width = tot_width;

	max_scale = 1.0;
	if ((txt->maxExtent>0) && (tot_height>txt->maxExtent)) {
		max_scale = txt->maxExtent / tot_height;
		tot_height = txt->maxExtent;
	}

	if (!strcmp(FSMINOR, "MIDDLE")) {
		if (FSLTR) {
			start_x = -tot_width/2;
			st->bounds.x = start_x;
		} else {
			start_x = tot_width/2 - line_spacing;
			st->bounds.x = - tot_width + line_spacing;
		}
	}
	else if (!strcmp(FSMINOR, "END")) {
		if (FSLTR) {
			start_x = -tot_width;
			st->bounds.x = start_x;
		} else {
			start_x = tot_width-line_spacing;
			st->bounds.x = 0;
		}
	}
	else {
		if (FSLTR) {
			start_x = 0;
			st->bounds.x = start_x;
		} else {
			start_x = -line_spacing;
			st->bounds.x = -tot_width;
		}
	}

	if (!strcmp(FSMAJOR, "MIDDLE")) {
		int_major = 0;
	} else if (!strcmp(FSMAJOR, "END")) {
		int_major = 1;
	} else {
		int_major = 2;
	}

	m4_rect_reset(&final);
	for (i=0; i < txt->string.count; i++) {
		switch (int_major) {
		case 0:
			if (FSTTB) 
				start_y = lines[i].height/2;
			else
				start_y = -lines[i].height/2 + space;
			break;
		case 1:
			if (FSTTB)
				start_y = lines[i].height;
			else
				start_y = -lines[i].height + space;
			break;
		default:
			if (FSTTB)
				start_y = 0;
			else
				start_y = space;
			break;
		}

		if (lines[i].length) {
			TextLineEntry2D *tl = NewTextLine2D(eff->surface->render);
			ChainAddEntry(st->text_lines, tl);

			/*adjust horizontal offset on first column*/
			if (!i) {
				max_lw = 0;
				for (k=0; k<lines[i].length; k++) {
					letter[0] = lines[i].wcText[k];
					/*get glyph width so that all letters are centered on the same vertical line*/
					ft_dr->get_text_size(ft_dr, letter, &lw, &lh);
					if (max_lw < lw) max_lw = lw;
				}
				st->bounds.width += max_lw/2;
				start_x += max_lw/2;
			}
			
			for (k=0; k<lines[i].length; k++) {
				letter[0] = lines[i].wcText[k];
				/*get glyph width so that all letters are centered on the same vertical line*/
				ft_dr->get_text_size(ft_dr, letter, &lw, &lh);
				ft_dr->add_text_to_path(ft_dr, tl->path,  1, letter, start_x - lw/2, start_y, lines[i].x_scaling, lines[i].y_scaling*max_scale, st->ascent, &rc);

				if (FSTTB)
					start_y -= space;
				else
					start_y += space;
			}
			m4_path_get_bounds(tl->path, &rc);
			m4_rect_union(&final, &rc);
			TextLine_StoreBounds(tl);
		}

		if (FSLTR) {
			start_x += line_spacing;
		} else {
			start_x -= line_spacing;
		}

		if (eff->parent) {
			rc = final;
			mx2d_apply_rect(&eff->transform, &rc);
			if (FSLTR && (rc.x > eff->surface->top_clipper.x + eff->surface->top_clipper.width) ) {
				break;
			}
			else if (!FSLTR && (rc.x + rc.width < eff->surface->top_clipper.x) ) {
				break;
			}
		}

		/*free unicode buffer*/
		free(lines[i].wcText);
	}

	/*free remaining unicode buffers*/
	for (; i < txt->string.count; i++) free(lines[i].wcText);

	free(lines);

	st->bounds.height = final.height;
	st->bounds.y = final.y;
}


static void BuildTextGraph(TextStack2D *st, M_Text *txt, RenderEffect2D *eff)
{
	TextLine2D *lines;
	unsigned short wcTemp[5000];
	u32 i, int_major, len, k;
	Float fontSize, start_x, start_y, font_height, line_spacing, tot_width, tot_height, max_scale;
	M4Rect rc, final;
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
	M_FontStyle *fs = (M_FontStyle *)txt->fontStyle;

	if (!FSHORIZ) {
		BuildVerticalTextGraph(st, txt, eff);
		return;
	}

	fontSize = FSSIZE;
	if (fontSize <= 0) {
		fontSize = 12;
		if (!R2D_IsPixelMetrics((SFNode *)txt)) fontSize /= eff->surface->render->cur_width;
    }

	if (ft_dr->set_font(ft_dr, FSFAMILY, FSSTYLE) != M4OK) {
		if (ft_dr->set_font(ft_dr, NULL, FSSTYLE) != M4OK) {
			return;
		}
	}
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);

	/*spacing= FSSPACING * fontSize and fontSize adjusted to m-ascent+m-descent (removed COR)*/
#if 0
	line_spacing = FSSPACE * fontSize;
	/*adjust size*/
	fontSize *= fontSize / (font_height);
	ft_dr->set_font_size(ft_dr, fontSize);
	ft_dr->get_font_metrics(ft_dr, &st->ascent, &st->descent, &font_height);
#else
	/*spacing= FSSPACING * (font_height) and fontSize not adjusted */
//	line_spacing = FSSPACE * (font_height);
	line_spacing = FSSPACE * fontSize;
#endif
	
	tot_width = 0;
	lines = (TextLine2D *) malloc(sizeof(TextLine2D)*txt->string.count);
	memset(lines, 0, sizeof(TextLine2D)*txt->string.count);
	
	for (i=0; i < txt->string.count; i++) {
		char *str = txt->string.vals[i];
		if (!str) continue;
		lines[i].length = 0;
		len = utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
		if (len == (size_t) (-1)) continue;

		lines[i].length = len;
		lines[i].wcText = malloc(sizeof(unsigned short) * (len+1));
		if (!FSLTR) {
			for (k=0; k<len; k++) lines[i].wcText[k] = wcTemp[len-k-1];
		} else {
			memcpy(lines[i].wcText, wcTemp, sizeof(unsigned short) * len);
		}
		lines[i].wcText[len] = (unsigned short) '\0';

		lines[i].y_scaling = lines[i].x_scaling = 1.0;
		ft_dr->get_text_size(ft_dr, lines[i].wcText, &lines[i].width, &lines[i].height);

		if (!lines[i].width) continue;
		if ((txt->length.count>i) && (txt->length.vals[i]>0)) {
			lines[i].x_scaling = txt->length.vals[i] / lines[i].width;
		}
		if (tot_width < lines[i].width * lines[i].x_scaling) tot_width = lines[i].width * lines[i].x_scaling;
	}
	
	max_scale = 1.0;
	if ((txt->maxExtent > 0) && (tot_width>txt->maxExtent)) {
		max_scale = txt->maxExtent / tot_width;
		tot_width = txt->maxExtent;
	}

	tot_height = (txt->string.count-1) * line_spacing + (st->ascent + st->descent);
	st->bounds.height = tot_height;
	
	if (!strcmp(FSMINOR, "MIDDLE")) {
		if (FSTTB) {
			start_y = tot_height/2;
			st->bounds.y = start_y;
		} else {
			start_y = st->descent + st->ascent - tot_height/2;
			st->bounds.y = tot_height/2;
		}
	}
	else if (!strcmp(FSMINOR, "BEGIN")) {
		if (FSTTB) {
			start_y = st->descent;
			start_y = 0;
			st->bounds.y = start_y;
		} else {
			st->bounds.y = st->bounds.height;
			start_y = st->descent + st->ascent;
		}
	}
	else if (!strcmp(FSMINOR, "END")) {
		if (FSTTB) {
			start_y = tot_height;
			st->bounds.y = start_y;
		} else {
			start_y = -tot_height + 2*st->descent + st->ascent;
			st->bounds.y = start_y - (st->descent + st->ascent) + tot_height;
		}
	}
	else {
		start_y = st->ascent;
		st->bounds.y = FSTTB ? start_y : (tot_height - st->descent);
	}
	
	/*major-justification*/
	if (!strcmp(FSMAJOR, "MIDDLE") ) {
		int_major = 0;
	} else if (!strcmp(FSMAJOR, "END") ) {
		int_major = 1;
	} else {
		int_major = 2;
	}
	m4_rect_reset(&final);


	for (i=0; i < txt->string.count; i++) {
		switch (int_major) {
		/*major-justification MIDDLE*/
		case 0:
			start_x = -lines[i].width/2;
			break;
		/*major-justification END*/
		case 1:
			start_x = (FSLTR) ? -lines[i].width : 0;
			break;
		/*BEGIN, FIRST or default*/
		default:
			start_x = (FSLTR) ? 0 : -lines[i].width;
			break;
		}

		if (lines[i].length) {
			TextLineEntry2D *tl = NewTextLine2D(eff->surface->render);

			/*if using the font engine the font is already configured*/
			ft_dr->add_text_to_path(ft_dr, tl->path, 1, lines[i].wcText, start_x, start_y, lines[i].x_scaling*max_scale, lines[i].y_scaling, st->ascent, &rc);

			ChainAddEntry(st->text_lines, tl);
			TextLine_StoreBounds(tl);
			m4_rect_union(&final, &rc);
		}
		if (FSTTB) {
			start_y -= line_spacing;
		} else {
			start_y += line_spacing;
		}

		if (! eff->parent) {
			rc = final;
			mx2d_apply_rect(&eff->transform, &rc);
			if (FSTTB && (rc.y < eff->surface->top_clipper.y - eff->surface->top_clipper.height) ) {
				break;
			}
			else if (! FSTTB && (rc.y - rc.height > eff->surface->top_clipper.y) ) {
				break;
			}
		}

		/*free unicode buffer*/
		free(lines[i].wcText);
	}
	/*free remaining unicode buffers*/
	for (; i < txt->string.count; i++) free(lines[i].wcText);

	free(lines);

	st->bounds.width = final.width;
	st->bounds.x = final.x;
}


void Text2D_Draw(DrawableContext *ctx)
{
	u32 i;
	Bool can_texture_text;
	TextLineEntry2D *tl;
	const char *fs_style;
	char *hlight;
	u32 hl_color;
	TextStack2D *st = (TextStack2D *) Node_GetPrivate((SFNode *) ctx->node->owner);
	M_FontStyle *fs = (M_FontStyle *) ((M_Text *) ctx->node->owner)->fontStyle;

	if (!ctx->aspect.filled && !ctx->aspect.pen_props.width) return;

	hl_color = 0;
	fs_style = FSSTYLE;
	hlight = strstr(fs_style, "HIGHLIGHT");
	if (hlight) hlight = strchr(hlight, '#');
	if (hlight) {
		hlight += 1;
		/*reverse video: highlighting uses the text color, and text color is inverted (except alpha channel)
		the ideal impl would be to use the background color for the text, but since the text may be 
		displayed over anything non uniform this would require clipping the highlight rect with the text
		which is too onerous (and not supported anyway) */
		if (!strnicmp(hlight, "RV", 2)) {
			u32 a, r, g, b;
			hl_color = ctx->aspect.fill_color;
			
			a = M4C_A(ctx->aspect.fill_color);
			if (a) {
				r = M4C_R(ctx->aspect.fill_color);
				g = M4C_G(ctx->aspect.fill_color);
				b = M4C_B(ctx->aspect.fill_color);
				ctx->aspect.fill_color = M4C_ARGB(a, 255-r, 255-g, 255-b);
			}
		} else {
			sscanf(hlight, "%x", &hl_color);
		}
		if (M4C_A(hl_color) == 0) hl_color = 0;
	}
	if (strstr(fs_style, "TEXTURED")) st->texture_text_flag = 1;

	/*text has been splited*/
	if (ctx->sub_path_index > 0) {
		tl = ChainGetEntry(st->text_lines, ctx->sub_path_index - 1);
		if (!tl || !tl->path) return;
		if (hl_color) VS2D_FillRect(ctx->surface, ctx, tl->bounds, hl_color);

		VS2D_TexturePath(ctx->surface, tl->path, ctx);
		VS2D_DrawPath(ctx->surface, tl->path, ctx, NULL, NULL);
		return;
	}

	can_texture_text = 0;
	if (st->graph->compositor->use_texture_text || st->texture_text_flag) {
		can_texture_text = !ctx->h_texture && !ctx->aspect.pen_props.width;
	}

	for (i=0; i<ChainGetCount(st->text_lines); i++) {
		tl = ChainGetEntry(st->text_lines, i);
		
		if (hl_color) VS2D_FillRect(ctx->surface, ctx, tl->bounds, hl_color);

		if (can_texture_text && TextLine2D_TextureIsReady(tl)) {
			VS2D_TexturePathText(ctx->surface, ctx, tl->tx_path, &tl->bounds, tl->hwtx, &tl->tx_bounds);
		} else {
			VS2D_TexturePath(ctx->surface, tl->path, ctx);
			VS2D_DrawPath(ctx->surface, tl->path, ctx, NULL, NULL);
		}
		/*reset fill/strike flags since we perform several draw per context*/
		ctx->path_filled = ctx->path_stroke = 0;
	}
}

Bool Text2D_PointOver(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	M4Matrix2D inv;
	u32 i;
	TextLineEntry2D *tl;
	TextStack2D *st;
	/*this is not documented anywhere but it speeds things up*/
	if (!check_outline) return 1;
	
	
	st = (TextStack2D *) Node_GetPrivate((SFNode *) ctx->node->owner);
	
	mx2d_copy(inv, ctx->transform);
	mx2d_inverse(&inv);
	mx2d_apply_coords(&inv, &x, &y);

	/*otherwise get all paths*/
	if (ctx->sub_path_index > 0) {
		tl = ChainGetEntry(st->text_lines, ctx->sub_path_index - 1);
		if (!tl || !tl->path) return 0;
		return m4_path_point_over(tl->path, x, y);
	}

	for (i=0; i<ChainGetCount(st->text_lines); i++) {
		tl = ChainGetEntry(st->text_lines, i);
		if (!tl->path) return 0;
		if (m4_path_point_over(tl->path, x, y)) return 1;
	}
	return 0;
}


static void Text_Render(SFNode *n, void *rs)
{
	DrawableContext *ctx;
	M_Text *txt = (M_Text *) n;
	TextStack2D *st = (TextStack2D *) Node_GetPrivate(n);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	if (!st->graph->compositor->font_engine) return;

	if (!txt->string.count) return;

	if (eff->text_split_mode == 2) {
		split_text_letters(st, txt, eff);
		return;
	}
	else if (eff->text_split_mode == 1) {
		split_text_words(st, txt, eff);
		return;
	}

	/*check for geometry change*/
	if (Node_GetDirty(n)) {
		TextStack2D_clean_paths(st);
		BuildTextGraph(st, txt, eff);
		Node_ClearDirty(n);
		st->graph->node_changed = 1;
	}

	/*get the text bounds*/
	ctx = drawable_init_context(st->graph, eff);
	if (!ctx) return;

	/*store bounds*/
	ctx->original = st->bounds;

	ctx->is_text = 1;
	if (!ctx->aspect.filled) {
		/*override line join*/
		ctx->aspect.pen_props.join = M4LineJoinMiter;
		ctx->aspect.pen_props.cap = M4LineCapFlat;
	}
	drawable_finalize_render(ctx, eff);
}


void R2D_InitText(Render2D *sr, SFNode *node)
{
	TextStack2D *stack = malloc(sizeof(TextStack2D));
	stack->graph = NewDrawableNode();
	/*override all funct*/
	stack->graph->Draw = Text2D_Draw;
	stack->graph->IsPointOver = Text2D_PointOver;
	stack->ascent = stack->descent = 0;
	stack->text_lines = NewChain();
	stack->texture_text_flag = 0;
	
	traversable_setup(stack->graph, node, sr->compositor);
	Node_SetPrivate(node, stack);
	Node_SetRenderFunction(node, Text_Render);
	Node_SetPreDestroyFunction(node, DestroyText);
}


static void RenderTextureText(SFNode *node, void *rs)
{
	TextStack2D *stack;
	SFNode *text;
	FieldInfo field;
	if (Node_GetField(node, 0, &field) != M4OK) return;
	if (field.fieldType != FT_SFNode) return;
	text = *(SFNode **)field.far_ptr;
	if (!text) return;

	if (Node_GetField(node, 1, &field) != M4OK) return;
	if (field.fieldType != FT_SFBool) return;

	if (Node_GetTag(text) != TAG_MPEG4_Text) return;
	stack = (TextStack2D *) Node_GetPrivate(text);
	stack->texture_text_flag = *(SFBool*)field.far_ptr ? 1 : 0;
}


void R2D_InitTextureText(Render2D *sr, SFNode *node)
{
	Node_SetRenderFunction(node, RenderTextureText);
}

