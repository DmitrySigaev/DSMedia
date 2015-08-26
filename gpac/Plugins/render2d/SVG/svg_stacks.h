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
#ifndef _SVG_STACKS_H
#define _SVG_STACKS_H

#include "../render2d.h"

#ifndef M4_DISABLE_SVG
#include <gpac/m4_svg_nodes.h>

#include "../../../M4Systems/render/common_stacks.h"
#include "../stacks2d.h"
#include "../SMIL/smil_stacks.h"

DrawableContext *SVG_drawable_init_context(Drawable *node, RenderEffect2D *eff);

/* Basic shapes rendering functions */

void SVG_Init_svg(Render2D *sr, SFNode *node);
void SVG_Init_g(Render2D *sr, SFNode *node);
void SVG_Init_rect(Render2D *sr, SFNode *node);
void SVG_Init_circle(Render2D *sr, SFNode *node);
void SVG_Init_ellipse(Render2D *sr, SFNode *node);
void SVG_Init_line(Render2D *sr, SFNode *node);
void SVG_Init_polyline(Render2D *sr, SFNode *node);
void SVG_Init_polygon(Render2D *sr, SFNode *node);
void SVG_Init_path(Render2D *sr, SFNode *node);

/* Text rendering functions */
void SVG_Init_text(Render2D *sr, SFNode *node);

/* SVG Specific Animation functions */
void SVG_Init_set(Render2D *sr, SFNode *node);
void SVG_Init_animate(Render2D *sr, SFNode *node);
void SVG_Init_animateTransform(Render2D *sr, SFNode *node);
void SVG_Init_animateMotion(Render2D *sr, SFNode *node);
void SVG_Init_animateColor(Render2D *sr, SFNode *node);

/* Media rendering functions */
typedef struct {
	TextureHandler txh;
	Drawable *graph;
} SVG_image_stack;

void SVG_Init_image(Render2D *se, SFNode *node);

void SVG_Init_use(Render2D *se, SFNode *node);
void SVG_Init_a(Render2D *se, SFNode *node);

typedef struct
{
	TextureHandler txh;
	Drawable *graph;
	TimeNode time_handle;
	Bool fetch_first_frame, first_frame_fetched;
	Double start_time;
	Bool isActive;
} SVG_video_stack;
void SVG_Init_video(Render2D *se, SFNode *node);


#endif

#endif

