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

#ifndef RENDER2D_H
#define RENDER2D_H

#include <gpac/intern/m4_render_dev.h>

#ifdef M4_USE_LASeR
#include "../../M4Systems/LASeR/m4_laser_dev.h"
#endif

#ifndef M4_DISABLE_SVG
#include <gpac/m4_scenegraph_svg.h>
#endif

typedef struct _render_2d
{
	/*remember main renderer*/
	SceneRenderer *compositor;
	/*all outlines cached*/
	Chain *strike_bank;
	/*all 2D surfaces created*/
	Chain *surfaces_2D;
	/*all 2D sensors registered*/
	Chain *sensors;

	/*current background color*/
	u32 back_color;

	/*tracking status*/
	Bool is_tracking;
	struct _drawable_context *grab_ctx;
	struct _drawable *grab_node;
	u32 last_sensor;

	/*top level effect for zoom/pan*/
	struct _render2d_effect *top_effect;
	/*main 2D surface we're writing on*/
	struct _visual_surface_2D *surface;
	Bool main_surface_setup;

	/*screen access*/
	void *hardware_context;
	M4VideoSurface hw_surface;
	Bool locked;
	/*pool surfaces used for HW blitting - one for YUV, one for RGB*/
	u32 pool_yuv, pool_rgb;
	/*current YUV pool surface format*/
	u32 current_yuv_format;
	Bool scalable_zoom, enable_yuv_hw;

	/*current output info: screen size and top-left point of video surface, and current scaled scene size*/
	u32 out_width, out_height, out_x, out_y, cur_width, cur_height;
	/*scale factor (scaleble zoom only)*/
	Float scale_x, scale_y;

	Bool grabbed;
	s32 grab_x, grab_y;
	Float zoom, trans_x, trans_y;
	u32 navigate_mode;
} Render2D;



/*user interaction event*/
typedef struct
{
	u32 event_type;
	Float x, y;
	/*current context passed to the sensor - if NULL the event is not over the node (deactivation)*/
	struct _drawable_context *context;
} UserEvent2D;

/*sensor node handler - this is not defined as a stack (cf below) because Anchor is both a grouping
node and a sensor node, and we DO need the groupingnode stack...*/
typedef struct _sensorhandler
{
	/*sensor enabled or not ?*/
	Bool (*IsEnabled)(struct _sensorhandler *sh);
	/*user input on sensor*/
	void (*OnUserEvent)(struct _sensorhandler *sh, UserEvent2D *ev, M4Matrix2D *sensor_matrix);
	/*set the node pointer here*/
	SFNode *owner;
	/*private to compositor for deactivating sensors*/
	Bool skip_second_pass;
} SensorHandler;

/*returns TRUE if the node is a pointing device sensor node that can be stacked during traversing (all sensor except anchor)*/
Bool is_sensor_node(SFNode *node);
/*returns associated sensor handler from traversable stack (the node handler is always responsible for creation/deletion)
returns NULL if not a sensor or sensor is not activated*/
SensorHandler *get_sensor_handler(SFNode *node);

void R2D_RegisterSensor(SceneRenderer *sr, SensorHandler *sh);
void R2D_UnregisterSensor(SceneRenderer *sr, SensorHandler *sh);

/*sensor context is needed for DEF/USE of a sensor over several shapes*/
typedef struct
{
	SensorHandler *h_node;
	M4Matrix2D matrix;
} SensorContext;

enum
{
	/*when set objects are drawn as soon as traversed, at each frame*/
	TF_RENDER_DIRECT	= (1<<2),
};

/*the traversing context: set_up at top-level and passed through SFNode_Render*/
typedef struct _render2d_effect
{
	AUDIO_EFFECT_CLASS

	/*current graph traversed is in pixel metrics*/
	Bool is_pixel_metrics;
	Float min_hsize;

	/*the one and only 2D surface currently traverse*/
	struct _visual_surface_2D *surface;
	
	/*current background and viewport stacks*/
	Chain *back_stack, *view_stack;

	/*current transformation from top-level*/
	M4Matrix2D transform;
	/*current color transformation from top-level*/
	M4ColorMatrix color_mat;

	/*if set all nodes shall be redrawn - set only at specific places in the tree*/
	Bool invalidate_all;
	/*indicates background nodes shall be drawn*/
	Bool draw_background;
	/*text splitting: 0: no splitting, 1: word by word, 2:letter by letter*/
	u32 text_split_mode;

	/*all sensors for the current level*/
	Chain *sensors;

	/*current appearance when traversing geometry nodes*/
	SFNode *appear;
	/*parent group for composition: can be Form, Layout or Layer2D*/
	struct _parent_group *parent;

	/* Styling Property and others for SVG context */
#ifndef M4_DISABLE_SVG
	SVGStylingProperties *svg_props;
#endif

} RenderEffect2D;

void effect_reset(RenderEffect2D *eff);
void effect_delete(RenderEffect2D *eff);

void effect_add_sensor(RenderEffect2D *eff, SensorHandler *ptr, M4Matrix2D *mat);
void effect_reset_sensors(RenderEffect2D *eff);

void R2D_RegisterSurface(Render2D *sr, struct _visual_surface_2D *);
void R2D_UnregisterSurface(Render2D *sr, struct _visual_surface_2D *);
Bool R2D_IsSurfaceRegistered(Render2D *sr, struct _visual_surface_2D *);

void R2D_LinePropsRemoved(Render2D *sr, SFNode *n);

Bool R2D_IsPixelMetrics(SFNode *n);

Bool R2D_NodeChanged(VisualRenderer *vr, SFNode *byObj);
void R2D_NodeInit(VisualRenderer *vr, SFNode *node);
/*copy pixels*/
void R2D_copyPixels(M4VideoSurface *vs, unsigned char *src, u32 src_stride, u32 src_w, u32 src_h, u32 src_pf, M4Window *src_wnd);

TextureHandler *R2D_GetTextureHandler(SFNode *n);

/*converts clipper to pixel metrics (used by layer, layout, etc)*/
M4Rect R2D_ClipperToPixelMetrics(RenderEffect2D *eff, SFVec2f size);

#endif

