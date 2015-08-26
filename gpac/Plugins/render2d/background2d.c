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
	SFNode *owner;
	struct scene_renderer *compositor;
	Chain *surfaces_links;
	Bool first_render;

	Drawable *node;

	/*for image background*/
	TextureHandler txh;
} Background2DStack;

M_Background2D *b2D_getnode(Background2DStack *ptr) { return (M_Background2D *) ptr->owner; }
Bool b2D_isBound(Background2DStack *ptr) { return ((M_Background2D *) ptr->owner)->isBound; }

typedef struct
{
	DrawableContext ctx;
	Chain *bind_stack;
} BackgroundStatus;

static void DestroyBackground2D(SFNode *node)
{
	M_Background2D *top;
	Background2DStack *ptr;
	BackgroundStatus *status;
	
	ptr = (Background2DStack *) Node_GetPrivate(node);

	DeleteDrawableNode(ptr->node);


	while (ChainGetCount(ptr->surfaces_links)) {
		status = ChainGetEntry(ptr->surfaces_links, 0);
		ChainDeleteEntry(ptr->surfaces_links, 0);
		ChainDeleteItem(status->bind_stack, node);

		/*force bind - bindable nodes are the only cases where we generate eventIn in the scene graph*/
		if (ChainGetCount(status->bind_stack)) {
			top = ChainGetEntry(status->bind_stack, 0);
			if (!top->set_bind) {
				top->set_bind = 1;
				if (top->on_set_bind) top->on_set_bind((SFNode *) top);
			}
		}
		free(status);
	}

	texture_destroy(&ptr->txh);
	DeleteChain(ptr->surfaces_links);
	free(ptr);
}


static BackgroundStatus *b2D_GetStatus(Background2DStack *bck, RenderEffect2D *eff)
{
	u32 i;
	BackgroundStatus *status;
	Chain *stack;

	stack = eff->back_stack;
	if (!stack) return NULL;

	for (i=0; i<ChainGetCount(bck->surfaces_links); i++) {
		status = ChainGetEntry(bck->surfaces_links, i);
		if (status->bind_stack == stack) return status;
	}

	status = malloc(sizeof(BackgroundStatus));
	memset(status, 0, sizeof(BackgroundStatus));
	mx2d_init(status->ctx.transform);
	status->ctx.surface = eff->surface;
	status->ctx.aspect.filled = 1;
	status->ctx.node = bck->node;
	status->ctx.h_texture = &bck->txh;
	status->ctx.is_background = 1;

	status->bind_stack = stack;
	status->ctx.aspect.fill_color = MAKE_ARGB(0, 0, 0, 0);
	ChainAddEntry(bck->surfaces_links, status);
	ChainAddEntry(stack, bck->owner);
	return status;
}


static Bool back_use_texture(M_Background2D *bck)
{
	if (!bck->url.count) return 0;
	if (bck->url.vals[0].OD_ID > 0) return 1;
	if (bck->url.vals[0].url && strlen(bck->url.vals[0].url)) return 1;
	return 0;
}

static void RenderBackground2D(SFNode *node, void *rs)
{
	u32 col;
	BackgroundStatus *status;
	M_Background2D *bck;
	Background2DStack *bcks = (Background2DStack *) Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	status = b2D_GetStatus(bcks, eff);
	if (!status) return;

	status->ctx.redraw_flags = 0;
	if (Node_GetDirty(node)) {
		status->ctx.redraw_flags = CTX_APP_DIRTY;
		Node_ClearDirty(node);
	}

	bck = b2D_getnode(bcks);

	if (bcks->first_render) {
		bcks->first_render = 0;
		if (ChainGetEntry(status->bind_stack, 0) == node) {
			if (!bck->isBound) {
				bck->isBound = 1;
				Node_OnEventOutSTR((SFNode *)bck, "isBound");
			}
		}
		/*open the stream if any*/
		if (back_use_texture(bck) && !bcks->txh.is_open) {
			texture_play(&bcks->txh, &bck->url);
		}

		/*we're in direct rendering and we missed background drawing - reset*/
		if (bck->isBound && (eff->trav_flags & TF_RENDER_DIRECT) && !eff->draw_background) {
			SR_Invalidate(bcks->compositor, NULL);
			return;
		}
	}

	if (!bck->isBound) return;
	if (!eff->draw_background && (eff->trav_flags & TF_RENDER_DIRECT)) return;

	/*background context bounds are always setup by parent group/surface*/
	if (back_use_texture(bck) ) {
		if (bcks->txh.hwtx) {
			if (!status->ctx.redraw_flags && bcks->txh.needs_refresh) status->ctx.redraw_flags |= CTX_TEXTURE_DIRTY;
		} else {
			SR_Invalidate(bcks->compositor, NULL);
			status->ctx.redraw_flags = CTX_APP_DIRTY;
		}
	} else {
		col = MAKE_ARGB_FLOAT(1.0, bck->backColor.red, bck->backColor.green, bck->backColor.blue);
		if (col != status->ctx.aspect.fill_color) {
			status->ctx.aspect.fill_color = col;
			status->ctx.redraw_flags = CTX_APP_DIRTY;
		}
	}

	if (!eff->draw_background) return;

	if (eff->back_stack == eff->surface->back_stack)
		eff->surface->render->back_color = MAKE_ARGB_FLOAT(1.0, bck->backColor.red, bck->backColor.green, bck->backColor.blue);

	if (eff->parent) {
		group2d_add_to_context_list(eff->parent, &status->ctx);
	} else if (eff->trav_flags & TF_RENDER_DIRECT) {
		bcks->node->Draw(&status->ctx);
	}
}


static void b2D_set_bind(SFNode *node)
{
	u32 i;
	Bool isOnTop;
	M_Background2D *newTop;
	M_Background2D *bck = (M_Background2D *) node;
	Background2DStack *bcks = (Background2DStack *)Node_GetPrivate(node);

	for (i=0; i<ChainGetCount(bcks->surfaces_links); i++) {
		BackgroundStatus *status = ChainGetEntry(bcks->surfaces_links, i);
		isOnTop = (ChainGetEntry(status->bind_stack, 0)==node) ? 1 : 0;

		if (! bck->set_bind) {
			if (bck->isBound) {
				bck->isBound = 0;
				Node_OnEventOutSTR(node, "isBound");
			}
			if (isOnTop && (ChainGetCount(status->bind_stack)>1)) {
				ChainDeleteEntry(status->bind_stack, 0);
				ChainAddEntry(status->bind_stack, node);
				newTop = ChainGetEntry(status->bind_stack, 0);
				newTop->set_bind = 1;
				newTop->on_set_bind((SFNode *) newTop);
			}
		} else {
			if (! bck->isBound) {
				bck->isBound = 1;
				Node_OnEventOutSTR(node, "isBound");
				Node_SetDirty(node, 0);
			}
			if (!isOnTop) {
				newTop = ChainGetEntry(status->bind_stack, 0);
				ChainDeleteItem(status->bind_stack, node);

				ChainInsertEntry(status->bind_stack, node, 0);
				newTop->set_bind = 0;
				newTop->on_set_bind((SFNode *) newTop);
			}
		}
	}
	/*and redraw scene*/
	SR_Invalidate(bcks->compositor, NULL);
}

static Bool b2D_point_over(struct _drawable_context *ctx, Float x, Float y, Bool check_outline) { return 0; }


DrawableContext *b2D_GetContext(M_Background2D *n, Chain *from_stack)
{
	u32 i;
	BackgroundStatus *status;
	Background2DStack *ptr = Node_GetPrivate((SFNode *)n);
	for (i=0; i<ChainGetCount(ptr->surfaces_links); i++) {
		status = ChainGetEntry(ptr->surfaces_links, i);
		if (status->bind_stack == from_stack) return &status->ctx;
	}
	return NULL;
}


static void DrawBackground(DrawableContext *ctx)
{
	Background2DStack *bcks = (Background2DStack *) Node_GetPrivate(ctx->node->owner);

	if (m4_rect_is_empty(ctx->clip) ) return;

	ctx->path_filled = 0;

	if ( back_use_texture(b2D_getnode(bcks)) ) {

		if (!ctx->surface->DrawBitmap) {
			/*set target rect*/
			m4_path_reset(bcks->node->path);
			m4_path_add_rectangle(bcks->node->path, 
								ctx->unclip.x + ctx->unclip.width/2,
								ctx->unclip.y - ctx->unclip.height/2,
								ctx->unclip.width, ctx->unclip.height);

			ctx->original = ctx->unclip;

			/*draw texture*/
			VS2D_TexturePath(ctx->surface, bcks->node->path, ctx);

		} else {

			ctx->original = ctx->unclip;
			ctx->clip = m4_rect_pixelize(&ctx->unclip);

			/*direct rendering, render without clippers */
			if (ctx->surface->render->top_effect->trav_flags & TF_RENDER_DIRECT) {
				ctx->surface->DrawBitmap(ctx->surface, ctx->h_texture, &ctx->clip, &ctx->unclip);
			}
			/*render bitmap for all dirty rects*/
			else {
				u32 i;
				M4IRect clip;
				for (i=0; i<ctx->surface->to_redraw.count; i++) {
					/*there's an opaque region above, don't draw*/
					if (ctx->surface->draw_node_index<ctx->surface->to_redraw.opaque_node_index[i]) continue;
					clip = ctx->clip;
					m4_irect_intersect(&clip, &ctx->surface->to_redraw.list[i]);
					if (clip.width && clip.height) {
						ctx->surface->DrawBitmap(ctx->surface, ctx->h_texture, &clip, &ctx->unclip);
					}
				}
			}
		}		
		ctx->redraw_flags = bcks->txh.hwtx ? 0 : CTX_APP_DIRTY;
	} else {
		/*directly clear with specified color*/
		VS2D_Clear(ctx->surface, &ctx->clip, ctx->aspect.fill_color);
		ctx->redraw_flags = 0;
	}
}

static void UpdateBackgroundTexture(TextureHandler *txh)
{
	texture_update_frame(txh, 0);
	/*restart texture if needed (movie background controled by MediaControl)*/
	if (txh->stream_finished && MO_GetLoop(txh->stream, 0)) texture_restart(txh);
}

void R2D_InitBackground2D(Render2D *sr, SFNode *node)
{
	Background2DStack *ptr = malloc(sizeof(Background2DStack));
	memset(ptr, 0, sizeof(Background2DStack));

	traversable_setup(ptr, node, sr->compositor);
	ptr->surfaces_links = NewChain();
	ptr->first_render = 1;
	/*setup rendering object for background*/
	ptr->node = BaseDrawStack2D(sr, node);
	ptr->node->IsPointOver = b2D_point_over;
	ptr->node->Draw = DrawBackground;
	((M_Background2D *)node)->on_set_bind = b2D_set_bind;


	texture_setup(&ptr->txh, sr->compositor, node);
	ptr->txh.update_texture_fcnt = UpdateBackgroundTexture;

	Node_SetPrivate(node, ptr);
	Node_SetPreDestroyFunction(node, DestroyBackground2D);
	Node_SetRenderFunction(node, RenderBackground2D);
}

void R2D_Background2DModified(SFNode *node)
{
	M_Background2D *bck = (M_Background2D *)node;
	Background2DStack *st = (Background2DStack *) Node_GetPrivate(node);
	if (!st) return;

	/*if open and changed, stop and play*/
	if (st->txh.is_open) {
		if (! texture_check_url_change(&st->txh, &bck->url)) return;
		texture_stop(&st->txh);
		texture_play(&st->txh, &bck->url);
		return;
	}
	/*if not open and changed play*/
	if (bck->url.count) 
		texture_play(&st->txh, &bck->url);
	SR_Invalidate(st->txh.compositor, NULL);
}
