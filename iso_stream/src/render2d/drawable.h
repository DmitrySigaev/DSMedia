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


#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "render2d.h"

/*

		ALL THE DRAWING ARCHITECTURE HERE IS FOR 2D SURFACES ONLY

	a drawable node is a node that is drawn:) eg all geometry nodes and background2D
a node may be drawn in different places (DEF/USE) so we also define a drawble context (eg context of one
instance of the drawn node)

*/



/*this is used to store context info between 2 frames, since the DrawableContext is not kept between frames */
typedef struct _boundinfo
{
	/*integer bounds in pixels (full and clip) */
	M4IRect clip, unclip;
	/* a node may have several appearances (due to DEF/USE), 
	   an the appearance is needed to compute bounds for the drawable in this context*/
	M_Appearance *appear;
	/*surface the node is drawn on (to handle compositeTexture2D)*/
	struct _visual_surface_2D *surface;
} BoundsInfo;


typedef struct _drawable
{
	SFNode *owner;
	struct scene_renderer *compositor;

	/*
			part to overload per drawable node if needed (IFS2D, ILS2D, Text, ...)
		simple objects using the default graphics path and a single texture/appearance use
		the default methods
	*/

	/*actual drawing of the node */
	void (*Draw)(struct _drawable_context *ctx);
	/*returns TRUE if point is over node - by default use drawable path
	if check_outline is set, the test is be performed on the vectorial outline, not the path itself (authoring only)
	*/
	Bool (*IsPointOver)(struct _drawable_context *ctx, Float x, Float y, Bool check_outline);

	/*
			common data for all	drawable nodes
	*/

	/*default graphic path - this is also the path used in PathLayout*/
	M4Path *path;

	/*
			Private data for compositor use
	*/

	/*current bounds and previous frame bounds list*/
	struct _boundinfo **current_bounds, **previous_bounds;
	u32 current_count, previous_count, bounds_size;

	/*unused node detetction*/
	Bool first_ctx_update;
	Bool node_was_drawn;
	Bool node_changed;
	/*list of surfaces the node is still attached to for dirty rects*/
	Chain *on_surfaces;

	/*cached outlines*/
	Chain *strike_list;
} Drawable;

/*construction destruction*/
Drawable *NewDrawableNode();
void DeleteDrawableNode(Drawable *);
/*store ctx bounds in current bounds*/
void drawable_store_bounds(struct _drawable_context *ctx);

/*move current bounds to previous bounds - THIS MUST BE CALLED IN BEFORE UPDATING ANY CONTEXT INFO
OF THE NODE, otherwise some bounds info may be lost when using composite textures*/
void drawable_flush_bounds(Drawable *node);

/*register/unregister node on surface for dirty rect*/
void drawable_register_on_surface(Drawable *node, struct _visual_surface_2D *surf);
void drawable_unregister_from_surface(Drawable *node, struct _visual_surface_2D *surf);
/*
	return 1 if same bound is found in previous list (and remove it from the list)
	return 0 otherwise
*/
Bool drawable_has_same_bounds(struct _drawable_context *ctx);
/*
	return any previous bounds related to the same surface in @rc if any
	if nothing found return 0
*/
Bool drawable_get_previous_bound(Drawable *node, M4IRect *rc, struct _visual_surface_2D *surf);
/*reset content of previous bounds list*/
void drawable_reset_previous_bounds(Drawable *node);

/*
decide whether drawing is needed or not based on rendering settings and parent node - must be called
at the end of each render of drawable nodes
*/
void drawable_finalize_render(struct _drawable_context *ctx, RenderEffect2D *effects);

/*base constructor for geometry objects that work without overloading the drawable stuff*/
Drawable *BaseDrawStack2D(Render2D *sr, SFNode *node);
/*reset all paths (main path and any outline) of the stack*/
void drawable_reset_path(Drawable *st);



typedef struct
{
	/*including alpha*/
	M4Color fill_color, line_color;
	Bool filled, has_line;
	Float line_scale;
	M4PenSettings pen_props;
	/*texture fill handler*/
	struct _texture_handler *line_texture;
	/*original alpha without color transforms*/
	u8 fill_alpha;
} DrawAspect2D;

enum
{
	/*set whenever geometry node changed*/
	CTX_NODE_DIRTY = 1,
	/*set whenever appearance changed*/
	CTX_APP_DIRTY = 1<<1,
	/*set whenever texture data changed*/
	CTX_TEXTURE_DIRTY = 1<<2
};

typedef struct _drawable_context
{
	/*clipped (drawned) and uncliped (for sensors) rect in pixels*/
	M4IRect clip, unclip_pix;
	/*exact unclipped rect for sensors*/
	M4Rect unclip;
	/*original bounds (in local coord system, float)*/
	M4Rect original;
	/*draw info*/
	DrawAspect2D aspect;
	/*transform matrix from top*/
	M4Matrix2D transform;
	/*color matrix*/
	M4ColorMatrix cmat;
	/*sensors attached to this context*/
	Chain *sensors;
	/*visual surface this ctx belongs to*/
	struct _visual_surface_2D *surface;
	/*drawable using this context*/
	Drawable *node;
	/*current appearance node*/
	SFNode *appear;
	/*texture fill handler*/
	struct _texture_handler *h_texture;

	/*any of the above flags*/
	u32 redraw_flags;

	/*set if node completely fits its bounds (flat rect and bitmap) then non transparent*/
	Bool transparent;
	/*set if text ctx*/
	Bool is_text;
	/*private for render, indicates path has been textured, in which case FILL is skiped*/
	Bool path_filled;
	/*private for render, indicates path outline has been textured, in which case STRIKE is skiped*/
	Bool path_stroke;
	/*set by background*/
	Bool is_background;
	/*used by bitmap - no antialiasing when drawn*/
	Bool no_antialias;
	/*only used by text when splitting strings into chars / substrings*/
	s32 sub_path_index;
} DrawableContext;	

DrawableContext *NewDrawableContext();
void DeleteDrawableContext(DrawableContext *);
void drawctx_reset(DrawableContext *ctx);
void drawctx_update_info(DrawableContext *ctx);
void drawctx_reset_sensors(DrawableContext *ctx);

/*inits context - may return NULL if the node doesn't have to be drawn*/
DrawableContext *drawable_init_context(Drawable *node, RenderEffect2D *effects);

/*store untransformed bounds for this context - provides bounds storing for basic nodes 
otherwise complex nodes shall store their bounds manually, and always before calling drawable_finalize_render*/
void drawctx_store_original_bounds(DrawableContext *ctx);



/*stored at compositor level and in each drawable node*/
typedef struct
{
	M4Path *outline;
	SFNode *lineProps;
	SFNode *node;
	u32 last_update_time;
	/*we also MUST handle width changes due to scale transforms, most content is designed with width=cst 
	whatever the transformation*/
	Float line_scale;
	/*set only for text*/
	M4Path *original;
} StrikeInfo2D;

void delete_strikeinfo2d(StrikeInfo2D *info);
/*get strike and manage any scale change&co. This avoids recomputing outline at each frame...*/
StrikeInfo2D *drawctx_get_strikeinfo(DrawableContext *ctx, M4Path *txt_path);

#endif
