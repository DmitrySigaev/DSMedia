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



#ifndef NODE_STACKS_H
#define NODE_STACKS_H

#include "grouping.h"

void R2D_InitAnchor(Render2D *sr, SFNode *node);
SensorHandler *r2d_anchor_get_handler(SFNode *n);

void R2D_InitBackground2D(Render2D *sr, SFNode *node);
void R2D_Background2DModified(SFNode *node);
DrawableContext *b2D_GetContext(M_Background2D *ptr, Chain *from_stack);

void R2D_InitBitmap(Render2D *sr, SFNode *node);


/*CompositeTexture2D*/
void R2D_InitCompositeTexture2D(Render2D *sr, SFNode *node);
TextureHandler *r2d_ct2D_get_texture(SFNode *node);
Bool CT2D_has_sensors(TextureHandler *txh);
DrawableContext *CT2D_FindNode(TextureHandler *txh, DrawableContext *ctx, Float x, Float y);
void Composite2D_ResetGraphics(TextureHandler *txh);
/* this is to use carefully: picks a node based on the PREVIOUS frame state (no traversing)*/
SFNode *CT2D_PickNode(TextureHandler *txh, DrawableContext *ctx, Float x, Float y);


void R2D_InitDiscSensor(Render2D *sr, SFNode *node);
SensorHandler *r2d_ds_get_handler(SFNode *n);
void R2D_InitForm(Render2D *sr, SFNode *node);
void R2D_InitLayer2D(Render2D *sr, SFNode *node);
void R2D_InitLayout(Render2D *sr, SFNode *node);
void R2D_LayoutModified(SFNode *node);

void R2D_InitMatteTexture(Render2D *sr, SFNode *node);
void R2D_MatteTextureModified(SFNode *node);
TextureHandler *r2d_matte_get_texture(SFNode *node);
void R2D_InitOrderedGroup(Render2D *sr, SFNode *node);
void R2D_InitPlaneSensor2D(Render2D *sr, SFNode *node);
SensorHandler *r2d_ps2D_get_handler(SFNode *n);
void R2D_InitProximitySensor2D(Render2D *sr, SFNode *node);
SensorHandler *r2d_prox2D_get_handler(SFNode *n);
void R2D_InitSwitch(Render2D *sr, SFNode *node);

typedef struct
{
	Render2D *sr;
	u32 last_mod_time;
} LinePropStack;
u32 R2D_LP_GetLastUpdateTime(SFNode *node);


/*exported to access the strike list ...*/
typedef struct
{
	Drawable *graph;
	Float ascent, descent;
	Chain *text_lines;
	M4Rect bounds;
	Bool texture_text_flag;
} TextStack2D;

void R2D_InitText(Render2D *sr, SFNode *node);
void R2D_InitTextureText(Render2D *sr, SFNode *node);

void R2D_InitTouchSensor(Render2D *sr, SFNode *node);
SensorHandler *r2d_touch_sensor_get_handler(SFNode *n);

/*viewport*/
void R2D_InitViewport(Render2D *sr, SFNode *node);
void vp_setup(SFNode *n, RenderEffect2D *eff, M4Rect *surf_clip);

/*used by laser so exported*/
typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	M4Matrix2D mat;
	Bool is_identity;
} Transform2DStack;

void R2D_InitLinearGradient(Render2D *sr, SFNode *node);
TextureHandler *r2d_lg_get_texture(SFNode *node);
void R2D_InitRadialGradient(Render2D *sr, SFNode *node);
TextureHandler *r2d_rg_get_texture(SFNode *node);
void R2D_InitColorTransform(Render2D *sr, SFNode *node);
void R2D_InitCircle(Render2D *sr, SFNode *node);
void R2D_InitCurve2D(Render2D *sr, SFNode *node);
void R2D_InitEllipse(Render2D *sr, SFNode *node);
void R2D_InitGroup(Render2D *sr, SFNode *node);
void R2D_InitIFS2D(Render2D *sr, SFNode *node);
void R2D_InitILS2D(Render2D *sr, SFNode *node);
void R2D_InitPointSet2D(Render2D *sr, SFNode *node);
void R2D_InitRectangle(Render2D *sr, SFNode *node);
void R2D_InitShape(Render2D *sr, SFNode *node);
void R2D_InitTransform2D(Render2D *sr, SFNode *node);
void R2D_InitTransformMatrix2D(Render2D *sr, SFNode *node);
void TM2D_GetMatrix(SFNode *n, M4Matrix2D *mat);
void R2D_InitPathLayout(Render2D *sr, SFNode *node);
void R2D_InitSound2D(Render2D *sr, SFNode *node);

#endif


