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
#include "svg_stacks.h"
#include "../visualsurface2d.h"

#ifndef M4_DISABLE_SVG

static void SVG_Destroy_image(SFNode *node)
{
	SVG_image_stack *st = (SVG_image_stack *)Node_GetPrivate(node);

	texture_destroy(&st->txh);
	DeleteDrawableNode(st->graph);
	free(st);
}

static void SVG_Update_image(TextureHandler *txh)
{
	SVGimageElement *txnode = (SVGimageElement *) txh->owner;
	MFURL txurl;
	SFURL *imgurl = NULL;
	txurl.count = 1;
	SAFEALLOC(txurl.vals, sizeof(SFURL))
	imgurl = txurl.vals;
	imgurl->OD_ID = DYNAMIC_OD_ID;
	imgurl->url = txnode->xlink_href.uri.string;

	/*setup texture if needed*/
	if (!txh->is_open && txurl.count) {
		texture_play(txh, &txurl);
	}
	texture_update_frame(txh, 0);
	/*URL is present but not opened - redraw till fetch*/
	if (txh->stream && !txh->hwtx) SR_Invalidate(txh->compositor, NULL);
}

static void SVG_BuildGraph_image(SVG_image_stack *st, DrawableContext *ctx)
{
	SVGimageElement *img = (SVGimageElement *)st->graph->owner;

	/* size of the texture */
	drawable_reset_path(st->graph);

	m4_path_add_rectangle(st->graph->path, img->x.number+img->width.number/2, img->y.number+img->height.number/2, img->width.number, img->height.number);
	m4_path_get_bounds(st->graph->path, &ctx->original);
}

static void SVG_Draw_image(DrawableContext *ctx)
{
	u8 alpha;
	Render2D *sr;
	Bool use_hw, has_scale;
	SVG_image_stack *st = (SVG_image_stack *) Node_GetPrivate(ctx->node->owner);

	sr = ctx->surface->render;

	/* no rotation allowed, remove skew components */
	/* ctx->transform.m[1] = ctx->transform.m[3] = 0; */

	has_scale = 0;
	use_hw = 1;
	alpha = M4C_A(ctx->aspect.fill_color);

	/*check if driver can handle alpha blit*/
	if (alpha!=255) {
		use_hw = sr->compositor->video_out->bHasAlpha;
		if (has_scale) use_hw = sr->compositor->video_out->bHasAlphaStretch;
	}

	/*this is not a native texture, use graphics*/
	if (!ctx->h_texture->data) {
		use_hw = 0;
	} else {
		if (!ctx->surface->SupportsFormat || !ctx->surface->DrawBitmap ) use_hw = 0;
		/*format not supported directly, try with brush*/
		else if (!ctx->surface->SupportsFormat(ctx->surface, ctx->h_texture->pixelformat) ) use_hw = 0;
	}

	/*no HW, fall back to the graphics driver*/
	if (!use_hw) {
		SVG_BuildGraph_image(st, ctx);
		ctx->no_antialias = 1;
		VS2D_TexturePath(ctx->surface, st->graph->path, ctx);
		return;
	}

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

static Bool SVG_PointOver_image(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	return 1;
}

static void SVG_Render_image(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	SVG_image_stack *st = (SVG_image_stack *)Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	/*we never cache anything with bitmap...*/
	Node_ClearDirty(node);

	ctx = SVG_drawable_init_context(st->graph, eff);
	if (!ctx || !ctx->h_texture ) return;
	/*always build the path*/
	SVG_BuildGraph_image(st, ctx);
	/*even if set this is not true*/
	ctx->aspect.has_line = 0;
	/*this is to make sure we don't fill the path if the texture is transparent*/
	ctx->aspect.filled = 0;
	ctx->aspect.pen_props.width = 0.0;

	ctx->no_antialias = 1;

	ctx->transparent = 0;
	/*if clipper then transparent*/

	if (ctx->h_texture->transparent) {
		ctx->transparent = 1;
	}
	/*global transparency is checked independently*/

	/*bounds are stored when building graph*/	
	drawable_finalize_render(ctx, eff);
}

void SVG_Init_image(Render2D *sr, SFNode *node)
{
	SVG_image_stack *st;
	SAFEALLOC(st, sizeof(SVG_image_stack))
	st->graph = NewDrawableNode();

	traversable_setup(st->graph, node, sr->compositor);
	st->graph->Draw = SVG_Draw_image;
	st->graph->IsPointOver = SVG_PointOver_image;

	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = SVG_Update_image;
	st->txh.flags = 0;

	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, SVG_Render_image);
	Node_SetPreDestroyFunction(node, SVG_Destroy_image);

}

static void SVG_Activate_video(SVG_video_stack *stack, SVGvideoElement *video)
{
	MFURL txurl;
	SFURL *videourl = NULL;
	txurl.count = 1;
	SAFEALLOC(txurl.vals, sizeof(SFURL))
	videourl = txurl.vals;
	videourl->OD_ID = DYNAMIC_OD_ID;
	videourl->url = video->xlink_href.uri.string;

	stack->isActive = 1;
	/*Node_OnEventOutSTR((SFNode*)mt, "isActive");*/
	if (!stack->txh.is_open) {
		texture_play(&stack->txh, &txurl);
	}
	MO_SetSpeed(stack->txh.stream, 1);
}

static void SVG_Deactivate_video(SVG_video_stack *stack, SVGvideoElement *video)
{
	stack->isActive = 0;
	/*Node_OnEventOutSTR((SFNode*)mt, "isActive");*/
	stack->time_handle.needs_unregister = 1;

	if (stack->txh.is_open) {
		texture_stop(&stack->txh);
	}
}

static void SVG_Update_video(TextureHandler *txh)
{
	SVGvideoElement *txnode = (SVGvideoElement *) txh->owner;
	SVG_video_stack *st = (SVG_video_stack *) Node_GetPrivate(txh->owner);
	MFURL txurl;
	SFURL *videourl = NULL;
	txurl.count = 1;
	SAFEALLOC(txurl.vals, sizeof(SFURL))
	videourl = txurl.vals;
	videourl->OD_ID = DYNAMIC_OD_ID;
	videourl->url = txnode->xlink_href.uri.string;

	/*setup texture if needed*/
	if (!txh->is_open) return;
	if (!st->isActive && st->first_frame_fetched) return;

	/*when fetching the first frame disable resync*/
	texture_update_frame(txh, 0);

	if (txh->stream_finished) {
		/*
		if (MT_GetLoop(st, txnode)) {
			texture_restart(txh);
		}
		//if active deactivate
		else if (txnode->isActive && MO_ShouldDeactivate(st->txh.stream) ) {
			MT_Deactivate(st, txnode);
		}
		*/
	}
	/* first frame is fetched */
	if (!st->first_frame_fetched && (txh->needs_refresh) ) {
		st->first_frame_fetched = 1;
		/*
		txnode->duration_changed = MO_GetDuration(txh->stream);
		Node_OnEventOutSTR(txh->owner, "duration_changed");
		*/
		/*stop stream if needed
		if (!txnode->isActive && txh->is_open) {
			texture_stop(txh);
			//make sure the refresh flag is not cleared
			txh->needs_refresh = 1;
		}
		*/
	}
	/*we have no choice but retraversing the graph until we're inactive since the movie framerate and
	the renderer framerate are likely to be different */
	if (!txh->stream_finished) SR_Invalidate(txh->compositor, NULL);
}

static void SVG_UpdateTime_video(TimeNode *st)
{
	Double time;
	SVGvideoElement *video = (SVGvideoElement *)st->obj;
	SVG_video_stack *stack = (SVG_video_stack *)Node_GetPrivate(st->obj);

	MFURL txurl;
	SFURL *videourl = NULL;
	txurl.count = 1;
	SAFEALLOC(txurl.vals, sizeof(SFURL))
	videourl = txurl.vals;
	videourl->OD_ID = DYNAMIC_OD_ID;
	videourl->url = video->xlink_href.uri.string;
	
	/*not active, store start time and speed */
	if (!stack->isActive) {
		//stack->start_time = mt->startTime;
		stack->start_time = 0;
	}

	time = Node_GetSceneTime(st->obj);

	if (time < stack->start_time //||
		/*special case if we're getting active AFTER stoptime 
		(!mt->isActive && (mt->stopTime > stack->start_time) && (time>=mt->stopTime)) */
		) {
		/*opens stream only at first access to fetch first frame*/
		if (stack->fetch_first_frame) {
			stack->fetch_first_frame = 0;
			if (!stack->txh.is_open)
				texture_play(&stack->txh, &txurl);
		}
		return;
	}

	/*
	if (MT_GetSpeed(stack, mt) && mt->isActive) {
		// if stoptime is reached (>startTime) deactivate
		if ((mt->stopTime > stack->start_time) && (time >= mt->stopTime) ) {
			MT_Deactivate(stack, mt);
			return;
		}
	}
	*/

	if (!stack->isActive) SVG_Activate_video(stack, video);
}

static void SVG_BuildGraph_video(SVG_video_stack *st, DrawableContext *ctx)
{
	SVGvideoElement *video = (SVGvideoElement *)st->graph->owner;

	/* size of the texture */
	drawable_reset_path(st->graph);

	m4_path_add_rectangle(st->graph->path, video->x.number+video->width.number/2, video->y.number+video->height.number/2, video->width.number, video->height.number);
	m4_path_get_bounds(st->graph->path, &ctx->original);
}

static void SVG_Render_video(SFNode *node, void *rs)
{
	DrawableContext *ctx;
	SVG_video_stack *st = (SVG_video_stack *)Node_GetPrivate(node);
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	/*we never cache anything with bitmap...*/
	Node_ClearDirty(node);

	ctx = SVG_drawable_init_context(st->graph, eff);
	if (!ctx || !ctx->h_texture ) return;
	/*always build the path*/
	SVG_BuildGraph_video(st, ctx);
	/*even if set this is not true*/
	ctx->aspect.has_line = 0;
	/*this is to make sure we don't fill the path if the texture is transparent*/
	ctx->aspect.filled = 0;
	ctx->aspect.pen_props.width = 0.0;

	ctx->no_antialias = 1;

	ctx->transparent = 0;
	/*if clipper then transparent*/

	if (ctx->h_texture->transparent) {
		ctx->transparent = 1;
	}
	/*global transparency is checked independently*/

	/*bounds are stored when building graph*/	
	drawable_finalize_render(ctx, eff);
}

static void SVG_Draw_video(DrawableContext *ctx)
{
	u8 alpha;
	Render2D *sr;
	Bool use_hw, has_scale;
	SVG_video_stack *st = (SVG_video_stack *) Node_GetPrivate(ctx->node->owner);

	sr = ctx->surface->render;

	/* no rotation allowed, remove skew components */
	/* ctx->transform.m[1] = ctx->transform.m[3] = 0; */

	has_scale = 0;
	use_hw = 1;
	alpha = M4C_A(ctx->aspect.fill_color);

	/*check if driver can handle alpha blit*/
	if (alpha!=255) {
		use_hw = sr->compositor->video_out->bHasAlpha;
		if (has_scale) use_hw = sr->compositor->video_out->bHasAlphaStretch;
	}

	/*this is not a native texture, use graphics*/
	if (!ctx->h_texture->data) {
		use_hw = 0;
	} else {
		if (!ctx->surface->SupportsFormat || !ctx->surface->DrawBitmap ) use_hw = 0;
		/*format not supported directly, try with brush*/
		else if (!ctx->surface->SupportsFormat(ctx->surface, ctx->h_texture->pixelformat) ) use_hw = 0;
	}

	/*no HW, fall back to the graphics driver*/
	if (!use_hw) {
		SVG_BuildGraph_video(st, ctx);
		ctx->no_antialias = 1;
		VS2D_TexturePath(ctx->surface, st->graph->path, ctx);
		return;
	}

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

static Bool SVG_PointOver_video(DrawableContext *ctx, Float x, Float y, Bool check_outline)
{
	return 1;
}

static void SVG_Destroy_video(SFNode *node)
{
	SVG_video_stack *st = (SVG_video_stack *)Node_GetPrivate(node);
	texture_destroy(&st->txh);
	if (st->time_handle.is_registered) SR_UnregisterTimeNode(st->txh.compositor, &st->time_handle);
	DeleteDrawableNode(st->graph);
	free(st);
}

void SVG_Init_video(Render2D *sr, SFNode *node)
{
	SVG_video_stack *st;
	SAFEALLOC(st, sizeof(SVG_video_stack))
	st->graph = NewDrawableNode();

	traversable_setup(st->graph, node, sr->compositor);
	st->graph->Draw = SVG_Draw_video;
	st->graph->IsPointOver = SVG_PointOver_video;

	texture_setup(&st->txh, sr->compositor, node);
	st->txh.update_texture_fcnt = SVG_Update_video;
	st->txh.flags = 0;
	st->time_handle.UpdateTimeNode = SVG_UpdateTime_video;
	st->time_handle.obj = node;
	st->fetch_first_frame = 1;
	
	SR_RegisterTimeNode(st->txh.compositor, &st->time_handle);	
	
	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, SVG_Render_video);
	Node_SetPreDestroyFunction(node, SVG_Destroy_video);

}
#endif //M4_DISABLE_SVG

