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


#include "svg_stacks.h"

#ifndef M4_DISABLE_SVG

#include "../visualsurface2d.h"


void SVGMergeContexts(SVGStylingProperties *render_svg_props, SVGStylingProperties current_svg_props);

static void SVG_Render_text(SFNode *node, void *rs)
{
	M4Matrix2D backup_matrix;
	M4Matrix2D *tr;
	DrawableContext *ctx;
	Drawable *cs = Node_GetPrivate(node);
	RenderEffect2D *eff = rs;
	SVGtextElement *text = (SVGtextElement *)node;
	FontRaster *ft_dr = eff->surface->render->compositor->font_engine;
  
	SVGStylingProperties backup_props;
	u32 styling_size = sizeof(SVGStylingProperties);

	memcpy(&backup_props, eff->svg_props, styling_size);
	SVGMergeContexts(eff->svg_props, text->properties);

	mx2d_copy(backup_matrix, eff->transform);
	tr = ChainGetEntry(text->transform, 0);
	if (tr) {
		mx2d_copy(eff->transform, *tr);
		mx2d_add_matrix(&eff->transform, &backup_matrix);
	}

	if (Node_GetDirty(node)) {
		unsigned short wcTemp[5000];
		char styles[1000];
		char *str = text->string.string;
		Float x = (ChainGetCount(text->x)?*(Float *)ChainGetEntry(text->x, 0):0);
		Float y = (ChainGetCount(text->y)?*(Float *)ChainGetEntry(text->y, 0):0);
		unsigned short *wcText;
		Float ascent, descent, font_height;
		M4Rect rc;
		u32 len;

		drawable_reset_path(cs);
		if (str) {
			len = utf8_mbstowcs(wcTemp, 5000, (const char **) &str);
			if (len == (u32) -1) return;

			wcText = malloc(sizeof(unsigned short) * (len+1));
			memcpy(wcText, wcTemp, sizeof(unsigned short) * (len+1));
			wcText[len] = 0;

			switch(*eff->svg_props->font_style) {
			case SVGFontStyle_normal:
				sprintf(styles, "%s", "PLAIN");
				break;
			case SVGFontStyle_italic:
				sprintf(styles, "%s", "ITALIC");
				break;
			case SVGFontStyle_oblique:
				sprintf(styles, "%s", "ITALIC");
				break;
			}
			if (ft_dr->set_font(ft_dr, eff->svg_props->font_family->value.string, styles) != M4OK) {
				if (ft_dr->set_font(ft_dr, NULL, styles) != M4OK) {
					return;
				}
			}
			ft_dr->set_font_size(ft_dr, eff->svg_props->font_size->value);
			ft_dr->get_font_metrics(ft_dr, &ascent, &descent, &font_height);

			ft_dr->add_text_to_path(ft_dr, cs->path, 0, wcText, x, y, 1, 1, ascent, &rc);
			free(wcText);
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

void SVG_Init_text(Render2D *sr, SFNode *node)
{
	BaseDrawStack2D(sr, node);
	Node_SetRenderFunction(node, SVG_Render_text);
}

#endif
