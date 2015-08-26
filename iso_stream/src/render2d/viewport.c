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



M4Err R2D_GetViewport(VisualRenderer *vr, u32 viewpoint_idx, const char **outName, Bool *is_bound)
{
	u32 count;
	SFNode *n;
	Render2D *sr = (Render2D *) vr->user_priv;
	if (!sr->surface) return M4BadParam;
	count = ChainGetCount(sr->surface->view_stack);
	if (!viewpoint_idx) return M4BadParam;
	if (viewpoint_idx>count) return M4EOF;

	n = ChainGetEntry(sr->surface->view_stack, viewpoint_idx-1);
	switch (Node_GetTag(n)) {
	case TAG_MPEG4_Viewport: *outName = ((M_Viewport*)n)->description.buffer; *is_bound = ((M_Viewport*)n)->isBound; return M4OK;
	default: *outName = NULL; return M4OK;
	}
}

M4Err R2D_SetViewport(VisualRenderer *vr, u32 viewpoint_idx, const char *viewpoint_name)
{
	u32 count, i;
	M_Viewport *n;
	Render2D *sr = (Render2D *) vr->user_priv;
	if (!sr->surface) return M4BadParam;
	count = ChainGetCount(sr->surface->view_stack);
	if (viewpoint_idx>count) return M4BadParam;
	if (!viewpoint_idx && !viewpoint_name) return M4BadParam;

	/*note we're sure only viewport nodes are in the 2D view stack*/
	if (viewpoint_idx) {
		n = (M_Viewport *) ChainGetEntry(sr->surface->view_stack, viewpoint_idx-1);
		n->set_bind = !n->set_bind;
		n->on_set_bind((SFNode *) n);
		return M4OK;
	}
	for (i=0; i<count;i++) {
		n = ChainGetEntry(sr->surface->view_stack, viewpoint_idx-1);
		if (n->description.buffer && !stricmp(n->description.buffer, viewpoint_name)) {
			n->set_bind = !n->set_bind;
			n->on_set_bind((SFNode *) n);
			return M4OK;
		}
	}
	return M4BadParam;
}

#define VPCHANGED(__comp) { M4Event evt; evt.type = M4E_VIEWPOINTS; M4USER_SENDEVENT(__comp->client, &evt); }

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	Chain *stack_list;
	u32 last_sim_time;
	Bool first_time;
} ViewportStack;


static void DestroyViewport(SFNode *node)
{
	s32 i;
	Chain *stack;
	ViewportStack *ptr = (ViewportStack *) Node_GetPrivate(node);
	M_Viewport *n_vp;

	while (ChainGetCount(ptr->stack_list)) {
		stack = ChainGetEntry(ptr->stack_list, 0);
		i = ChainDeleteItem(stack, node);
		if (i==0) {
			n_vp = (M_Viewport *) ChainGetEntry(stack, 0);
			/*we were bound so bind new top*/
			if (n_vp) {
				n_vp->isBound = 1;
				n_vp->bindTime = Node_GetSceneTime((SFNode *)n_vp);
				Node_OnEventOutSTR((SFNode *)n_vp, "isBound");
				Node_OnEventOutSTR((SFNode *)n_vp, "bindTime");
			}
		}
		ChainDeleteEntry(ptr->stack_list, 0);
	}
	DeleteChain(ptr->stack_list);
	/*notify change of vp stack*/
	VPCHANGED(ptr->compositor);
	free(ptr);
}

static void viewport_set_bind(SFNode *node)
{
	Bool on_top;
	u32 i;
	Chain *stack;
	SFNode *tmp;
	ViewportStack *ptr = (ViewportStack *) Node_GetPrivate(node);
	M_Viewport *vp = (M_Viewport *) ptr->owner;

	//notify all stacks using this node
	for (i=0; i<ChainGetCount(ptr->stack_list);i++) {
		stack = ChainGetEntry(ptr->stack_list, i);
		on_top = (ChainGetEntry(stack, 0) == node) ? 1 : 0;
	
		if (!vp->set_bind) {
			if (vp->isBound) {
				vp->isBound = 0;
				Node_OnEventOutSTR(node, "isBound");
			}

			if (on_top) {
				tmp = ChainGetEntry(stack, 0);
				ChainDeleteEntry(stack, 0);
				ChainAddEntry(stack, tmp);
				tmp = ChainGetEntry(stack, 0);
				if (tmp != node) {
					((M_Viewport *) tmp)->set_bind = 1;
					Node_OnEventOutSTR(tmp, "set_bind");
				}
			}
		} else {
			if (!vp->isBound) {
				vp->isBound = 1;
				vp->bindTime = Node_GetSceneTime(node);
				Node_OnEventOutSTR(node, "isBound");
				Node_OnEventOutSTR(node, "bindTime");
			}

			if (!on_top) {
				tmp = ChainGetEntry(stack, 0);
				ChainDeleteItem(stack, ptr);
				ChainInsertEntry(stack, ptr, 0);
				if (tmp != node) {
					((M_Viewport *) tmp)->set_bind = 0;
					Node_OnEventOutSTR(tmp, "isBound");
				}
			}
		}
	}
	SR_Invalidate(ptr->compositor, NULL);
	/*notify change of vp stack*/
	VPCHANGED(ptr->compositor);
}



static Chain *vp_get_stack(ViewportStack *vp, RenderEffect2D *eff)
{
	u32 i;	
	if (!eff->view_stack) return NULL;

	for (i=0; i<ChainGetCount(vp->stack_list); i++) {
		if (eff->view_stack == ChainGetEntry(vp->stack_list, i) ) return eff->view_stack;	
	}
	ChainAddEntry(vp->stack_list, eff->view_stack);
	ChainAddEntry(eff->view_stack, vp->owner);
	/*need a callback to user*/
	return eff->view_stack;
}

static void RenderViewport(SFNode *node, void *rs)
{
	ViewportStack *st = (ViewportStack *) Node_GetPrivate(node);
	M_Viewport *vp = (M_Viewport *) st->owner;

	if (st->first_time) {
		Chain *stack = vp_get_stack(st, (RenderEffect2D *)rs);

		if (ChainGetEntry(stack, 0) == node) {
			if (! vp->isBound) {
				vp->isBound = 1;
				vp->bindTime = Node_GetSceneTime(node);
				Node_OnEventOutSTR(node, "isBound");
				Node_OnEventOutSTR(node, "bindTime");
			}
		} else {
			if (IS_IsDefaultView(node)) {
				vp->set_bind = 1;
				vp->on_set_bind(node);
			}
		}
		st->first_time = 0;
		VPCHANGED(st->compositor);
	}
}

void R2D_InitViewport(Render2D *sr, SFNode *node)
{
	ViewportStack *ptr = malloc(sizeof(ViewportStack));
	memset(ptr, 0, sizeof(ViewportStack));
	ptr->first_time = 1;
	ptr->stack_list = NewChain();

	traversable_setup(ptr, node, sr->compositor);
	Node_SetPrivate(node, ptr);
	Node_SetRenderFunction(node, RenderViewport);
	Node_SetPreDestroyFunction(node, DestroyViewport);
	((M_Viewport*)node)->on_set_bind = viewport_set_bind;
}



void vp_setup(SFNode *n, RenderEffect2D *eff, M4Rect *surf_clip)
{
	Float ar, sx, sy, w, h, tx, ty;
	M4Matrix2D mat;
	M4Rect rc;
	M_Viewport *vp = (M_Viewport *) n;
	if (!vp->isBound || !surf_clip->width || !surf_clip->height) return;

	mx2d_init(mat);
	mx2d_add_translation(&mat, -1 * vp->position.x, -1 * vp->position.y);
	mx2d_add_rotation(&mat, 0, 0, -1 * vp->orientation);
	
	mx2d_add_matrix(&eff->transform, &mat);

	mx2d_copy(mat, eff->transform);

	//compute scaling ratio
	m4_rect_center(&rc, vp->size.x, vp->size.y);
	mx2d_apply_rect(&mat, &rc);

	w = surf_clip->width;
	h = surf_clip->height;
	ar = h / w;
	
	surf_clip->width = rc.width;
	surf_clip->height = rc.height;

	switch (vp->fit) {
	//covers all area and respect aspect ratio
	case 2:
		if (rc.width/w > rc.height/h) {
			rc.width *= h/rc.height;
			rc.height = h;
		} else {
			rc.height *= w/rc.width;
			rc.width = w;
		}
		break;
	//fits inside the area and respect AR
	case 1:
		if (rc.width/w> rc.height/h) {
			rc.height *= w/rc.width;
			rc.width = w;
		} else {
			rc.width *= h/rc.height;
			rc.height = h;
		}
		break;
	//fit entirely: nothing to change
	case 0:
		rc.width = w;
		rc.height = h;
		break;
	default:
		return;
	}
	sx = rc.width / surf_clip->width;
	sy = rc.height / surf_clip->height;

	surf_clip->width = rc.width;
	surf_clip->height = rc.height;
	surf_clip->x = - rc.width/2;
	surf_clip->y = rc.height/2;

	mx2d_init(mat);
	if (!vp->fit) {
		mx2d_add_scale(&mat, sx, sy);
		mx2d_add_matrix(&eff->transform, &mat);
		return;
	}

	//setup x-alignment
	switch (vp->alignment.vals[0]) {
	//left align: 
	case -1:
		tx = rc.width/2 - w/2;
		break;
	//right align
	case 1:
		tx = w/2 - rc.width/2;
		break;
	//center
	case 0:
	default:
		tx = 0;
		break;
	}

	//setup y-alignment
	switch (vp->alignment.vals[1]) {
	//left align: 
	case -1:
		ty = rc.height/2 - h/2;
		break;
	//right align
	case 1:
		ty = h/2 - rc.height/2;
		break;
	//center
	case 0:
	default:
		ty = 0;
		break;
	}

	mx2d_add_scale(&mat, sx, sy);
	mx2d_add_translation(&mat, tx, ty);
	mx2d_add_matrix(&eff->transform, &mat);
	surf_clip->x += tx;
	surf_clip->y += ty;
}

