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

#ifndef GROUPING_H
#define GROUPING_H

#include "drawable.h"

/*
		NOTES about grouping nodes

***before traversing the children for pre-rendering, all children are first checked for interactions
and sensor nodes are added to the current list of valid sensors - this is needed because BIFS doesn't define
tree-depth order for sensors, so a sensor is active on a shape even when declared after the shape in the tree

***when a grouping node impacts placement of its children, rendering is done in 2 step: first render
the children with a blank transform and store their context (childGroup), then replace each 
child's context bounds according to the display rules of the node (form, layout). When everything is 
done the final matrix is computed

***in order to avoid browsing for sensor nodes at each traversing we store sensor nodes locally if the node
hasn't been modified


*/



typedef struct 
{
	/*set ti true if the bounding rect of this group is not the sum of bounding rects of its children
	(typically all grouping node with post-layout/clipping)*/
	Bool bounds_forced;
	Chain *contexts;	
	/*bounds before and after placement*/
	M4Rect original, final;
	/*set if this group contains only text graphic nodes*/
	Bool is_text_group;
	/*in which case ascent and descent of the text is stored for baseline alignment*/
	Float ascent, descent;
} ChildGroup2D;


/*
	@groups: list of ChildGroup visually render - used for post placement
	@sensors: list of sensors (recomputed if any changes)
	@get_background_stack: fct pointer to retrieve background stack (Layer2D defines its own)
	@get_viewport_stack: fct pointer to retrieve viewport stack (Layer2D defines its own)
*/

#define GROUPINGNODESTACK2D	\
	Chain *groups;		\
	Chain *sensors;		\

typedef struct _parent_group
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
} GroupingNode2D;
/*performs stack init/destroy - doesn't free stack*/
void SetupGroupingNode2D(GroupingNode2D *ptr, Render2D *sr, SFNode *node);
void DeleteGroupingNode2D(GroupingNode2D *gr);
/*destroy base stack*/
void DestroyBaseGrouping2D(SFNode *node);


/*traverse all children of the node - the children field is passed for security reasons...*/
void group2d_traverse(GroupingNode2D *group, Chain *children, RenderEffect2D *effects);

void group2d_start_child(GroupingNode2D *group);
void group2d_end_child(GroupingNode2D *group);
/*reset ChildGroup info - does NOT reset real children*/
void group2d_reset_children(GroupingNode2D *group);

void group2d_add_to_context_list(GroupingNode2D *group, DrawableContext *ctx);

/*update clipers and matrices in regular mode (translations only)*/
void child2d_render_done(ChildGroup2D *cg, RenderEffect2D *eff, M4Rect *par_clip);
/*update clipers and matrices in complex mode (free-form transforms) - if @mat is NULL group isn't rendered*/
void child2d_render_done_complex(ChildGroup2D *cg, RenderEffect2D *eff, M4Matrix2D *mat);

/*for form, layout, layer2D which define bounds != than sum of their children bounds
@group is the PARENT of the group forcing its bounds*/
void group2d_force_bounds(GroupingNode2D *group, M4Rect *clip);

Bool is_sensor_node(SFNode *node);

#endif

