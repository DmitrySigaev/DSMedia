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

#ifndef VISUALSURFACE2D_H_
#define VISUALSURFACE2D_H_

#include "render2d.h"
#include "drawable.h"

/*sensors info*/
typedef struct 
{
	struct _drawable_context *ctx;
	Chain *nodes_on_top;
} SensorInfo;


static M4INLINE Bool m4_irect_overlaps(M4IRect rc1, M4IRect rc2)
{
	if (! rc2.height || !rc2.width || !rc1.height || !rc1.width) return 0;
	if (rc2.x+rc2.width<=rc1.x) return 0;
	if (rc2.x>=rc1.x+rc1.width) return 0;
	if (rc2.y-rc2.height>=rc1.y) return 0;
	if (rc2.y<=rc1.y-rc1.height) return 0;
	return 1;
}

/*adds @rc2 to @rc1 - the new @rc1 contains the old @rc1 and @rc2*/
static M4INLINE void m4_irect_union(M4IRect *rc1, M4IRect *rc2) 
{
	if (!rc1->width || !rc1->height) {*rc1=*rc2; return;}

	if (rc2->x < rc1->x) {
		rc1->width += rc1->x - rc2->x;
		rc1->x = rc2->x;
	}
	if (rc2->x + rc2->width > rc1->x+rc1->width) rc1->width = rc2->x + rc2->width - rc1->x;
	if (rc2->y > rc1->y) {
		rc1->height += rc2->y - rc1->y;
		rc1->y = rc2->y;
	}
	if (rc2->y - rc2->height < rc1->y - rc1->height) rc1->height = rc1->y - rc2->y + rc2->height;
}

/*@rc1 equales @rc2*/
static M4INLINE Bool m4_irect_equal(M4IRect rc1, M4IRect rc2) 
{ 
	if ( (rc1.x == rc2.x)  && (rc1.y == rc2.y) && (rc1.width == rc2.width) && (rc1.height == rc2.height) )
		return 1;
	return 0;
}

/*makes @rc empty*/
#define m4_rect_reset(rc)  { (rc)->x = (rc)->y = (rc)->width = (rc)->height = 0; }

/*is @rc empty*/
#define m4_rect_is_empty(rc) ( ((rc).width && (rc).height) ? 0 : 1 )

/*intersects @rc1 with @rc2 - the new @rc1 is the intersection*/
static M4INLINE void m4_irect_intersect(M4IRect *rc1, M4IRect *rc2)
{
	if (! m4_irect_overlaps(*rc1, *rc2)) {
		m4_rect_reset(rc1); 
		return;
	}
	if (rc2->x > rc1->x) {
		rc1->width -= rc2->x - rc1->x;
		rc1->x = rc2->x;
	} 
	if (rc2->x + rc2->width < rc1->x + rc1->width) {
		rc1->width = rc2->width + rc2->x - rc1->x;
	} 
	if (rc2->y < rc1->y) {
		rc1->height -= rc1->y - rc2->y; 
		rc1->y = rc2->y;
	} 
	if (rc2->y - rc2->height > rc1->y - rc1->height) {
		rc1->height = rc1->y - rc2->y + rc2->height;
	} 
}

M4IRect m4_rect_pixelize(M4Rect *r);

static M4INLINE M4Rect m4_rect_ft(M4IRect *rc)
{
	M4Rect rcft;
	rcft.x = (Float) rc->x; rcft.y = (Float) rc->y; rcft.width = (Float) rc->width; rcft.height = (Float) rc->height; 
	return rcft;
}

/*@rc2 fully contained in @rc1*/
static M4INLINE Bool m4_rect_inside(M4IRect rc1, M4IRect rc2) 
{
	if ( (rc1.x <= rc2.x)  && (rc1.y >= rc2.y)  && (rc1.x + rc1.width >= rc2.x + rc2.width) && (rc1.y - rc1.height <= rc2.y - rc2.height) )
		return 1;
	return 0;
}
/*@x, y in @rc*/
static M4INLINE Bool m4_point_in_rect(M4IRect rc, Float x, Float y) 
{
	if ( (x >= rc.x) && (y <= rc.y) && (x <= rc.x + rc.width) && (y >= rc.y - rc.height) )
		return 1;
	return 0;
}

/*ra_: rectangle array macros to speed dirty rects*/
#define RA_DEFAULT_STEP	50

typedef struct
{	
	M4IRect *list;
	u32 count, alloc;
	/*list of nodes covering (no transparency) each rect, or 0 otherwise.*/
	u32 *opaque_node_index;
} M4RectArray;

/*inits structure - called as a constructor*/
#define ra_init(ra) { (ra)->count = 0; (ra)->alloc = 1; (ra)->list = malloc(sizeof(M4IRect)); (ra)->opaque_node_index = NULL;}
/*deletes structure - called as a destructor*/
#define ra_del(ra) { free((ra)->list); if ((ra)->opaque_node_index) free((ra)->opaque_node_index); }
/*adds rect to list - expand if needed*/
#define ra_add(ra, rc) {	\
	if ((ra)->count==(ra)->alloc) { (ra)->alloc += RA_DEFAULT_STEP; (ra)->list = realloc((ra)->list, sizeof(M4IRect) * (ra)->alloc); }	\
	(ra)->list[(ra)->count] = rc; (ra)->count++;	}
/*clears list*/
#define ra_clear(ra) { (ra)->count = 0; }
/*is list empty*/
#define ra_is_empty(ra) (!((ra)->count))


/*adds rectangle to the list performing union test*/
static M4INLINE void ra_union_rect(M4RectArray *ra, M4IRect rc) 
{
	u32 i;

	for (i=0; i<ra->count; i++) { 
		if (m4_irect_overlaps(ra->list[i], rc)) { 
			m4_irect_union(&ra->list[i], &rc); 
			return; 
		} 
	}
	ra_add(ra, rc); 
}

/*merges all rects in ra2 into ra1*/
static M4INLINE void ra_merge(M4RectArray *ra1, M4RectArray *ra2)  
{
	u32 i;
	for (i=0; i<ra2->count; i++) {
		ra_union_rect(ra1, ra2->list[i]);
	}
}

/*refreshes the content of the array to have only non-overlapping rects*/
static M4INLINE void ra_refresh(M4RectArray *ra)
{
	u32 i, j, k;
	for (i=0; i<ra->count; i++) {
		for (j=i+1; j<ra->count; j++) {
			if (m4_irect_overlaps(ra->list[i], ra->list[j])) {
				m4_irect_union(&ra->list[i], &ra->list[j]);

				/*remove rect*/
				k = ra->count - j - 1;
				if (k) memmove(&ra->list[j], & ra->list[j+1], sizeof(M4IRect)*k);
				ra->count--; 

				ra_refresh(ra);
				return;
			}
		}
	}
}


typedef struct _visual_surface_2D 
{
	Render2D *render;
	
	/*the one and only dirty rect collector*/
	M4RectArray to_redraw;
	u32 draw_node_index;

	/*static list of context*/
	struct _drawable_context **contexts;
	/*nodes to draw (same alloc size as contexts)*/
	u32 *nodes_to_draw;	
	u32 num_contexts, alloc_contexts;

	/*background and viewport stacks*/
	Chain *back_stack;
	Chain *view_stack;

	/*top-level transform (active viewport, user zoom and pan) */
	M4Matrix2D top_transform;
	/*pixel area of surface in BIFS coords - eg surface to fill with background*/
	M4IRect surf_rect;
	/*top clipper (may be different than surf_rect when a viewport is active)*/
	M4IRect top_clipper;

	/*keeps track of nodes drawn last frame*/
	Chain *prev_nodes_drawn;
	/*currently active sensors*/
	Chain *sensors;	
	Bool last_was_direct_render;
	Bool last_had_back;
	/*black for BIFS, white for SVG/LASeR/etc*/
	u32 default_back_color;

	/*signals that the surface is attached to buffer/device/stencil*/
	Bool is_attached;

	/*size in pixels*/
	u32 width, height;
	Bool center_coords;

	/*this is set by the video renderer or the composite texture object */

	/*gets access to graphics handle*/
	M4Err (*GetSurfaceAccess)(struct _visual_surface_2D *);
	/*release graphics handle*/
	void (*ReleaseSurfaceAccess)(struct _visual_surface_2D *);

	/*draws specified texture as flat bitmap*/
	void (*DrawBitmap)(struct _visual_surface_2D *, struct _texture_handler *, M4IRect *clip, M4Rect *unclip);
	Bool (*SupportsFormat)(struct _visual_surface_2D *surf, u32 pixel_format);

	/*composite texture renderer if any*/
	struct _composite_2D *composite;
	
	LPM4SURFACE the_surface;
	LPM4STENCIL the_brush;
	LPM4STENCIL the_pen;

	u32 pixel_format;
} VisualSurface2D;
/*constructor/destructor*/
VisualSurface2D *NewVisualSurface2D();
void DeleteVisualSurface2D(VisualSurface2D *);
void VS2D_ResetSensors(VisualSurface2D *surf);
/*gets a drawable context on this surface*/
struct _drawable_context *VS2D_GetDrawableContext(VisualSurface2D *surf);
/*remove last drawable context*/
void VS2D_RemoveLastContext(VisualSurface2D *surf);
/*signal the given drawable is being deleted*/
void VS2D_DrawableDeleted(VisualSurface2D *surf, Drawable *node);

/*inits rendering cycle - called at each cycle start regardless of rendering mode*/
void VS2D_InitDraw(VisualSurface2D *surf, RenderEffect2D *eff);
/*terminates rendering cycle - called at each cycle end regardless of rendering mode
if rendering is indirect, actual drawing is performed here. Returns 1 if the surface has been modified*/
Bool VS2D_TerminateDraw(VisualSurface2D *surf, RenderEffect2D *eff);


/*locates drawable context under the given point - also locate context in composite textures*/
DrawableContext *VS2D_FindNode(VisualSurface2D *surf, Float x, Float y);

/*clear given rect or all surface if no rect specified - clear color depends on surface type - 0 for composite
surfaces, renderer clear color otherwise
BackColor is non 0 for background node only*/
void VS2D_Clear(VisualSurface2D *, M4IRect *clear, u32 BackColor);
/*texture the path with the given context info*/
void VS2D_TexturePath(VisualSurface2D *, LPM4PATH path, DrawableContext *ctx);
/*draw the path (fill and strike) - if brushes are NULL they are created if needed based on the context aspect
DrawPath shall always be called after TexturePath*/
void VS2D_DrawPath(VisualSurface2D *, LPM4PATH path, DrawableContext *ctx, LPM4STENCIL brush, LPM4STENCIL pen);
/*special texturing extension for text, using a given path (text rectangle) and texture (rendered text)*/
void VS2D_TexturePathText(VisualSurface2D *surf, DrawableContext *txt_ctx, LPM4PATH path, M4Rect *object_bounds, LPHWTEXTURE hwtx, M4Rect *texture_bounds);

/*inits graphics surface handler for subsequent draw*/
M4Err VS2D_InitSurface(VisualSurface2D *surf);
/*releases graphics surface handler */
void VS2D_TerminateSurface(VisualSurface2D *surf);
/*destroys graphics driver objects used by the surface*/
void VS2D_ResetGraphics(VisualSurface2D *surf);

/* this is to use carefully: picks a node based on the PREVIOUS frame state (no traversing)*/
SFNode *VS2D_PickNode(VisualSurface2D *surf, Float x, Float y);

/*fill given rect with given color with given ctx transform and clipper (used for text hilighting only)*/
void VS2D_FillRect(VisualSurface2D *surf, DrawableContext *ctx, M4Rect rc, u32 color);

#endif

