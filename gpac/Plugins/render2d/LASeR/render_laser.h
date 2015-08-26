#ifndef _RENDERLASER_H
#define _RENDERLASER_H

#include <gpac/intern/m4_render_dev.h>
#include "../../M4Systems/LASeR/m4_laser_dev.h"

/*the traversing context: set_up at top-level and passed through SFNode_Render*/
typedef struct _render_laser_effect
{
	u32 traversing_mode;

	/*the one and only 2D surface currently traverse*/
//	struct _visual_surface_2D *surface;

	/*current transformation from top-level*/
	M4Matrix2D transform;

	/*if set all nodes shall be redrawn - set only at specific places in the tree*/
	Bool invalidate_all;
	/*indicates background nodes shall be drawn*/
	Bool draw_background;

	/*all sensors for the current level*/
	Chain *sensors;

	/*parent group for composition: can be Form, Layout or Layer2D*/
	struct _parent_group *parent;
} LASeRRenderEffect;

/*rendering modes for 2D renderer*/
enum
{
	/*when set objects are drawn as soon as traversed, at each frame*/
	LASER_TRAVERSE_RENDER_DIRECT
};

typedef struct _render_laser
{
	/*remember main renderer*/
	SceneRenderer *compositor;
	/*graphics driver*/
	Graphics2DDriver *g_hw;

	/*all outlines cached*/
	Chain *strike_bank;
	/*all 2D surfaces created*/
	Chain *surfaces_2D;
	/*all 2D sensors registered*/
	Chain *sensors;

	/*tracking status*/
	Bool is_tracking;
	struct _drawable_context *grab_ctx;
	struct _drawable *grab_node;
	u32 last_sensor;

	/*top level effect for zoom/pan*/
	struct _render_laser_effect *top_effect;

	/*main 2D surface we're writing on*/
//	struct _visual_surface_2D *surface;

	/*screen access*/
	void *hardware_context;
//	M4VideoSurface hw_surface;
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
} LASeRRenderer;

#endif