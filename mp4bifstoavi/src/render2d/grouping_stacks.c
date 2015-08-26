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

typedef struct
{
	s32 last_switch;
} SwitchStack;

static void DestroySwitch(SFNode *node)
{
	SwitchStack *st = (SwitchStack *)Node_GetPrivate(node);
	free(st);
}
static void RenderSwitch(SFNode *node, void *rs)
{
	u32 i;
	Bool prev_switch;
	SFNode *child;
	SwitchStack *st = (SwitchStack *)Node_GetPrivate(node);
	M_Switch *sw = (M_Switch *) node;
	RenderEffect2D *eff; 
	u32 count = ChainGetCount(sw->choice);

	eff = (RenderEffect2D *)rs;

	prev_switch = eff->trav_flags;
	/*check changes in choice field*/
	if ((Node_GetDirty(node) & SG_NODE_DIRTY) || (st->last_switch != sw->whichChoice) ) {
		eff->trav_flags |= TF_SWITCHED_OFF;
		/*deactivation must be signaled because switch may contain audio nodes...*/
		for (i=0; i<count; i++) {
			if ((s32) i==sw->whichChoice) continue;
			child = ChainGetEntry(sw->choice, i);
			Node_Render(child, eff);
		}
		eff->trav_flags &= ~TF_SWITCHED_OFF;
		st->last_switch = sw->whichChoice;
	}

	Node_ClearDirty(node);

	/*no need to check for sensors since a sensor is active for the whole parent group, that is for switch itself
	CSQ: switch cannot be used to switch sensors, too bad...*/
	eff->trav_flags = prev_switch;

	if (sw->whichChoice>=0) {
		child = ChainGetEntry(sw->choice, sw->whichChoice);
		Node_Render(child, eff);
	}
}

void R2D_InitSwitch(Render2D *sr, SFNode *node)
{
	SwitchStack *st = malloc(sizeof(SwitchStack));
	st->last_switch = -1;
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroySwitch);
	Node_SetRenderFunction(node, RenderSwitch);
}


/*transform2D*/
static void DestroyTransform2D(SFNode *n)
{
	Transform2DStack *ptr = (Transform2DStack *)Node_GetPrivate(n);
	DeleteGroupingNode2D((GroupingNode2D *)ptr);
	free(ptr);
}

static void RenderTransform2D(SFNode *node, void *rs)
{
	M4Matrix2D bckup;
	M_Transform2D *tr = (M_Transform2D *)node;
	Transform2DStack *ptr = (Transform2DStack *)Node_GetPrivate(node);
	RenderEffect2D *eff;

	eff = (RenderEffect2D *) rs;

	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		mx2d_init(ptr->mat);
		ptr->is_identity = 1;
		if ((tr->scale.x != 1.0) || (tr->scale.y != 1.0)) {
			mx2d_add_scale_at(&ptr->mat, tr->scale.x, tr->scale.y, 0, 0, tr->scaleOrientation);
			ptr->is_identity = 0;
		}
		if (tr->rotationAngle) {
			mx2d_add_rotation(&ptr->mat, tr->center.x, tr->center.y, tr->rotationAngle);
			ptr->is_identity = 0;
		}
		if (tr->translation.x || tr->translation.y) {
			ptr->is_identity = 0;
			mx2d_add_translation(&ptr->mat, tr->translation.x, tr->translation.y);
		}
	}

	/*note we don't clear dirty flag, this is done in traversing*/
	if (ptr->is_identity) {
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
	} else {
		mx2d_copy(bckup, eff->transform);
		mx2d_copy(eff->transform, ptr->mat);
		mx2d_add_matrix(&eff->transform, &bckup);
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
		mx2d_copy(eff->transform, bckup);
	}
}

void R2D_InitTransform2D(Render2D *sr, SFNode *node)
{
	Transform2DStack *stack = malloc(sizeof(Transform2DStack));
	SetupGroupingNode2D((GroupingNode2D *)stack, sr, node);
	mx2d_init(stack->mat);
	stack->is_identity = 1;
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyTransform2D);
	Node_SetRenderFunction(node, RenderTransform2D);
}

void TM2D_GetMatrix(SFNode *n, M4Matrix2D *mat)
{
	M_TransformMatrix2D *tr = (M_TransformMatrix2D*)n;
	mx2d_init(*mat);
	mat->m[0] = tr->mxx;
	mat->m[1] = tr->mxy;
	mat->m[2] = tr->tx;
	mat->m[3] = tr->myx;
	mat->m[4] = tr->myy;
	mat->m[5] = tr->ty;
}


/*TransformMatrix2D*/
static void RenderTransformMatrix2D(SFNode *node, void *rs)
{
	M4Matrix2D bckup;
	M_TransformMatrix2D *tr = (M_TransformMatrix2D*)node;
	Transform2DStack *ptr = (Transform2DStack *) Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		TM2D_GetMatrix(node, &ptr->mat);
		if ((tr->mxx==1.0) && (tr->mxy==0.0) && (tr->tx==0.0)
			&& (tr->myx==0.0) && (tr->myy==1.0) && (tr->ty==0.0) )
			ptr->is_identity = 1;
		else
			ptr->is_identity = 0;
	}

	/*note we don't clear dirty flag, this is done in traversing*/
	if (ptr->is_identity) {
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
	} else {
		mx2d_copy(bckup, eff->transform);
		mx2d_copy(eff->transform, ptr->mat);
		mx2d_add_matrix(&eff->transform, &bckup);
		group2d_traverse((GroupingNode2D *)ptr, tr->children, eff);
		mx2d_copy(eff->transform, bckup);
	}
}


void R2D_InitTransformMatrix2D(Render2D *sr, SFNode *node)
{
	Transform2DStack *stack = malloc(sizeof(Transform2DStack));
	SetupGroupingNode2D((GroupingNode2D *)stack, sr, node);
	mx2d_init(stack->mat);
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyTransform2D);
	Node_SetRenderFunction(node, RenderTransformMatrix2D);
}


typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	M4ColorMatrix cmat;
} ColorTransformStack;

static void DestroyColorTransform(SFNode *n)
{
	ColorTransformStack *ptr = (ColorTransformStack *)Node_GetPrivate(n);
	DeleteGroupingNode2D((GroupingNode2D *)ptr);
	free(ptr);
}

/*ColorTransform*/
static void RenderColorTransform(SFNode *node, void *rs)
{
	Bool c_changed;
	M_ColorTransform *tr = (M_ColorTransform *)node;
	ColorTransformStack *ptr = (ColorTransformStack  *)Node_GetPrivate(node);
	RenderEffect2D *eff;
	eff = (RenderEffect2D *) rs;

	c_changed = 0;
	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		cmat_set(&ptr->cmat, 
			tr->mrr , tr->mrg, tr->mrb, tr->mra, tr->tr, 
			tr->mgr , tr->mgg, tr->mgb, tr->mga, tr->tg, 
			tr->mbr, tr->mbg, tr->mbb, tr->mba, tr->tb, 
			tr->mar, tr->mag, tr->mab, tr->maa, tr->ta); 
		c_changed = 1;
	}
	/*note we don't clear dirty flag, this is done in traversing*/
	if (ptr->cmat.identity) {
		group2d_traverse((GroupingNode2D *) ptr, tr->children, eff);
	} else {
		M4ColorMatrix cmat_bck;
		Bool prev_inv = eff->invalidate_all;
		/*if modified redraw all nodes*/
		if (c_changed) eff->invalidate_all = 1;
		cmat_copy(&cmat_bck, &eff->color_mat);
		cmat_multiply(&eff->color_mat, &ptr->cmat);
		group2d_traverse((GroupingNode2D *) ptr, tr->children, eff);
		/*restore effects*/
		cmat_copy(&eff->color_mat, &cmat_bck);
		eff->invalidate_all = prev_inv;
	}
}

void R2D_InitColorTransform(Render2D *sr, SFNode *node)
{
	ColorTransformStack *stack = malloc(sizeof(ColorTransformStack));
	SetupGroupingNode2D((GroupingNode2D *)stack, sr, node);
	cmat_init(&stack->cmat);
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyColorTransform);
	Node_SetRenderFunction(node, RenderColorTransform);
}

static void RenderGroup(SFNode *node, void *rs)
{
	GroupingNode2D *group = (GroupingNode2D *) Node_GetPrivate(node);
	group2d_traverse(group, ((M_Group *)node)->children, (RenderEffect2D*)rs);
}

void R2D_InitGroup(Render2D *sr, SFNode *node)
{
	GroupingNode2D *stack = malloc(sizeof(GroupingNode2D));
	SetupGroupingNode2D(stack, sr, node);
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyBaseGrouping2D);
	Node_SetRenderFunction(node, RenderGroup);
}


typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D
	Bool enabled;
	SensorHandler hdl;
} AnchorStack;

static void DestroyAnchor(SFNode *n)
{
	AnchorStack *st = (AnchorStack*)Node_GetPrivate(n);
	R2D_UnregisterSensor(st->compositor, &st->hdl);
	if (st->compositor->interaction_sensors) st->compositor->interaction_sensors--;
	DeleteGroupingNode2D((GroupingNode2D *)st);
	free(st);
}

static void RenderAnchor(SFNode *node, void *rs)
{
	AnchorStack *st = (AnchorStack *) Node_GetPrivate(node);
	M_Anchor *an = (M_Anchor *) node;
	RenderEffect2D *eff = rs;

	/*update enabled state*/
	if (Node_GetDirty(node) & SG_NODE_DIRTY) {
		st->enabled = 0;
		if (an->url.count && an->url.vals[0].url && strlen(an->url.vals[0].url) )
			st->enabled = 1;
	}
	/*note we don't clear dirty flag, this is done in traversing*/
	group2d_traverse((GroupingNode2D*)st, an->children, eff);
}

static Bool anchor_is_enabled(SensorHandler *sh)
{
	AnchorStack *st = (AnchorStack *) Node_GetPrivate(sh->owner);
	return st->enabled;
}

static void OnAnchor(SensorHandler *sh, UserEvent2D *ev, M4Matrix2D *sensor_matrix)
{
	u32 i;
	AnchorStack *st;
	M4Event evt;
	M_Anchor *an;
	if (ev->event_type != M4E_LEFTUP) return;
	st = (AnchorStack *) Node_GetPrivate(sh->owner);
	an = (M_Anchor *) sh->owner;

	if (!st->compositor->client->EventProc) return;
	evt.type = M4E_NAVIGATE;
	evt.navigate.param_count = an->parameter.count;
	evt.navigate.parameters = (const char **) an->parameter.vals;
	i=0;
	while (i<an->url.count) {
		evt.navigate.to_url = an->url.vals[i].url;
		if (!evt.navigate.to_url) break;
		/*current scene navigation*/
		if (evt.navigate.to_url[0] == '#') {
			SFNode *n;
			evt.navigate.to_url++;
			n = SG_FindNodeByName(Node_GetParentGraph(sh->owner), (char *) evt.navigate.to_url);
			if (n) {
				switch (Node_GetTag(n)) {
				case TAG_MPEG4_Viewport:
					((M_Viewport *)n)->set_bind = 1;
					((M_Viewport *)n)->on_set_bind(n);
					break;
				}
				break;
			}
		} else {
			if (st->compositor->client->EventProc(st->compositor->client->opaque, &evt))
				break;
		}

		i++;
	}
}

static void on_activate_anchor(SFNode *node)
{
	UserEvent2D ev;
	AnchorStack *st = (AnchorStack *) Node_GetPrivate(node);
	if (!((M_Anchor *)node)->activate) return;

	ev.event_type = M4E_LEFTUP;
	OnAnchor(&st->hdl, &ev, NULL);
}

SensorHandler *r2d_anchor_get_handler(SFNode *n)
{
	AnchorStack *st = (AnchorStack *) Node_GetPrivate(n);
	return &st->hdl;
}

void R2D_InitAnchor(Render2D *sr, SFNode *node)
{
	M_Anchor *an = (M_Anchor *)node;
	AnchorStack *stack = malloc(sizeof(AnchorStack));
	memset(stack, 0, sizeof(AnchorStack));

	SetupGroupingNode2D((GroupingNode2D*)stack, sr, node);

	sr->compositor->interaction_sensors++;

	an->on_activate = on_activate_anchor;
	stack->hdl.IsEnabled = anchor_is_enabled;
	stack->hdl.OnUserEvent = OnAnchor;
	stack->hdl.owner = node;
	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyAnchor);
	Node_SetRenderFunction(node, RenderAnchor);
}

struct og_pos
{
	Float priority;
	u32 position;
};
typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D

	struct og_pos *priorities;
	u32 count;
} OrderedGroupStack;

static void DestroyOrderedGroup(SFNode *node)
{
	OrderedGroupStack *ptr = (OrderedGroupStack *) Node_GetPrivate(node);
	DeleteGroupingNode2D((GroupingNode2D *)ptr);
	if (ptr->priorities) free(ptr->priorities);
	free(ptr);
}

static s32 compare_priority(const void* elem1, const void* elem2)
{
	struct og_pos *p1, *p2;
	p1 = (struct og_pos *)elem1;
	p2 = (struct og_pos *)elem2;
	if (p1->priority < p2->priority) return -1;
	if (p1->priority > p2->priority) return 1;
	return 0;
}


static void RenderOrderedGroup(SFNode *node, void *rs)
{
	u32 i, count;
	SFNode *child;
	Bool split_text_backup, invalidate_backup;
	M_OrderedGroup *og;
	u32 count2;
	Chain *sensor_backup;
	SensorHandler *hsens;
	
	OrderedGroupStack *ogs = (OrderedGroupStack *) Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	og = (M_OrderedGroup *) ogs->owner;

	if (!og->order.count) {
		group2d_traverse((GroupingNode2D*)ogs, og->children, eff);
		return;
	}
	count = ChainGetCount(og->children);
	invalidate_backup = eff->invalidate_all;

	/*check whether the OrderedGroup node has changed*/
	if (Node_GetDirty(node) & SG_NODE_DIRTY) {

		if (ogs->priorities) free(ogs->priorities);
		ogs->priorities = malloc(sizeof(struct og_pos)*count);
		for (i=0; i<count; i++) {
			ogs->priorities[i].position = i;
			ogs->priorities[i].priority = (i<og->order.count) ? og->order.vals[i] : 0;
		}
		qsort(ogs->priorities, count, sizeof(struct og_pos), compare_priority);
		eff->invalidate_all = 1;
	}

	sensor_backup = NULL;
	if (Node_GetDirty(node) & SG_CHILD_DIRTY) {
		/*rebuild sensor list*/
		if (ChainGetCount(ogs->sensors)) {
			DeleteChain(ogs->sensors);
			ogs->sensors = NewChain();
		}

		for (i=0; i<count; i++) {
			child = ChainGetEntry(og->children, ogs->priorities[i].position);
			if (!child || !is_sensor_node(child) ) continue;
			hsens = get_sensor_handler(child);
			if (hsens) ChainAddEntry(ogs->sensors, hsens);
		}
	}

	/*if we have an active sensor at this level discard all sensors in current render context (cf VRML)*/
	count2 = ChainGetCount(ogs->sensors);
	if (count2) {
		sensor_backup = eff->sensors;
		eff->sensors = NewChain();
		/*add sensor to effects*/	
		for (i=0; i <count2; i++) {
			SensorHandler *hsens = ChainGetEntry(ogs->sensors, i);
			effect_add_sensor(eff, hsens, &eff->transform);
		}
	}
	Node_ClearDirty(node);

	if (eff->parent == (GroupingNode2D *) ogs) {
		for (i=0; i<count; i++) {
			group2d_start_child((GroupingNode2D *) ogs);
			child = ChainGetEntry(og->children, ogs->priorities[i].position);
			Node_Render(child, eff);
			group2d_end_child((GroupingNode2D *) ogs);
		}
	} else {
		split_text_backup = eff->text_split_mode;
		if (count>1) eff->text_split_mode = 0;
		for (i=0; i<count; i++) {
			child = ChainGetEntry(og->children, ogs->priorities[i].position);
			Node_Render(child, eff);
		}
		eff->text_split_mode = split_text_backup;
	}

	/*restore effect*/
	invalidate_backup = eff->invalidate_all;
	if (count2) {
		/*destroy current effect list and restore previous*/
		effect_reset_sensors(eff);
		DeleteChain(eff->sensors);
		eff->sensors = sensor_backup;
	}
}

void R2D_InitOrderedGroup(Render2D *sr, SFNode *node)
{
	OrderedGroupStack *ptr = malloc(sizeof(OrderedGroupStack));
	memset(ptr, 0, sizeof(OrderedGroupStack));

	SetupGroupingNode2D((GroupingNode2D*)ptr, sr, node);
	
	Node_SetPrivate(node, ptr);
	Node_SetPreDestroyFunction(node, DestroyOrderedGroup);
	Node_SetRenderFunction(node, RenderOrderedGroup);
}

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	GROUPINGNODESTACK2D

	Chain *backs;
	Chain *views;
	Bool first;
	M4Rect clip;
} Layer2DStack;

static void DestroyLayer2D(SFNode *node)
{
	Layer2DStack *l2D = (Layer2DStack *) Node_GetPrivate(node);
	DeleteGroupingNode2D((GroupingNode2D *)l2D);
	DeleteChain(l2D->backs);
	DeleteChain(l2D->views);
	free(l2D);
}

static void RenderLayer2D(SFNode *node, void *rs)
{
	u32 i;
	Chain *prevback, *prevviews;
	M4Rect clip;
	M_Viewport *vp;
	M4Matrix2D mx2d_bck;
	GroupingNode2D *parent_bck;
	DrawableContext *back_ctx;
	Bool bool_bck;
	DrawableContext *ctx;
	M_Background2D *back;
	M_Layer2D *l = (M_Layer2D *)node;
	Layer2DStack *l2D = (Layer2DStack *) Node_GetPrivate(node);
	RenderEffect2D *eff;


	eff = (RenderEffect2D *) rs;
	mx2d_copy(mx2d_bck, eff->transform);
	parent_bck = eff->parent;
	eff->parent = (GroupingNode2D *) l2D;
	mx2d_init(eff->transform);
	bool_bck = eff->draw_background;
	prevback = eff->back_stack;
	prevviews = eff->view_stack;
	eff->back_stack = l2D->backs;
	eff->view_stack = l2D->views;

	if (l2D->first) {
		/*render on back first to register with stack*/
		if (l->background) {
			eff->draw_background = 0;
			Node_Render((SFNode*) l->background, eff);
			group2d_reset_children((GroupingNode2D*) l2D);
			eff->draw_background = 1;
		}
		vp = (M_Viewport*)l->viewport;
		if (vp) {
			ChainAddEntry(l2D->views, vp);
			if (!vp->isBound) {
				vp->isBound = 1;
				Node_OnEventOutSTR((SFNode*)vp, "isBound");
			}
		}
	}

	back = NULL;
	if (ChainGetCount(l2D->backs) ) back = ChainGetEntry(l2D->backs, 0);
	vp = NULL;
	if (ChainGetCount(l2D->views)) vp = ChainGetEntry(l2D->views, 0);

	if (!eff->is_pixel_metrics) mx2d_add_scale(&eff->transform, eff->min_hsize, eff->min_hsize);
	l2D->clip = R2D_ClipperToPixelMetrics(eff, l->size);

	/*apply viewport*/
	if (vp) {
		clip = l2D->clip;
		vp_setup((SFNode *) vp, eff, &clip);
	}


	back_ctx = NULL;
	if (back) {
		/*setup back size and render*/
		group2d_start_child((GroupingNode2D *)l2D);
		
		eff->draw_background = 1;
		ctx = b2D_GetContext(back, l2D->backs);
		ctx->unclip = l2D->clip;
		ctx->clip = m4_rect_pixelize(&ctx->unclip);
		mx2d_init(ctx->transform);
		Node_Render((SFNode *) back, eff);
		eff->draw_background = 0;
	
		/*we need a trick since we're not using a dedicated surface for layer rendering, 
		we emulate the back context: remove previous context and insert fake one*/
		if (!(eff->trav_flags & TF_RENDER_DIRECT) && (ChainGetCount(l2D->groups)==1)) {
			ChildGroup2D *cg = ChainGetEntry(l2D->groups, 0);
			back_ctx = VS2D_GetDrawableContext(eff->surface);
			ChainDeleteEntry(cg->contexts, 0);
			ChainAddEntry(cg->contexts, back_ctx);
			back_ctx->unclip = ctx->unclip;
			back_ctx->clip = ctx->clip;
			back_ctx->h_texture = ctx->h_texture;
			back_ctx->transparent = 0;
			back_ctx->redraw_flags = ctx->redraw_flags;
			back_ctx->is_background = 1;
			back_ctx->aspect = ctx->aspect;
			back_ctx->node = ctx->node;
		}
		group2d_end_child((GroupingNode2D *)l2D);
	}

	group2d_traverse((GroupingNode2D *)l2D, l->children, eff);
	/*restore effect*/
	eff->draw_background = bool_bck;
	mx2d_copy(eff->transform, mx2d_bck);
	eff->parent = parent_bck;
	eff->back_stack = prevback;
	eff->view_stack = prevviews;

	/*check bindables*/
	if (l2D->first) {
		Bool redraw = 0;
		l2D->first = 0;
		if (!back && ChainGetCount(l2D->backs)) redraw = 1;
		if (!vp && ChainGetCount(l2D->views) ) redraw = 1;

		/*we missed background or viewport (was not declared as bound during traversal, and is bound now)*/
		if (redraw) {
			group2d_reset_children((GroupingNode2D*)l2D);
			SR_Invalidate(l2D->compositor, NULL);
			return;
		}
	}

	for (i=0; i<ChainGetCount(l2D->groups); i++) {
		child2d_render_done(ChainGetEntry(l2D->groups, i), eff, &l2D->clip);
	}
	group2d_reset_children((GroupingNode2D*)l2D);

	group2d_force_bounds(eff->parent, &l2D->clip);
}

void R2D_InitLayer2D(Render2D *sr, SFNode *node)
{
	Layer2DStack *stack = malloc(sizeof(Layer2DStack));
	SetupGroupingNode2D((GroupingNode2D*)stack, sr, node);
	stack->backs = NewChain();
	stack->views = NewChain();
	stack->first = 1;

	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, DestroyLayer2D);
	Node_SetRenderFunction(node, RenderLayer2D);
}

