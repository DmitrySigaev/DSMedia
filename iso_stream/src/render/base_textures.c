/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Rendering sub-project
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

#include "common_stacks.h"

#include <m4_x3d_nodes.h>

typedef struct
{
	TextureHandler txh;

	TimeNode time_handle;
	Bool fetch_first_frame, first_frame_fetched;
	Double start_time;
} MovieTextureStack;

static void DestroyMovieTexture(SFNode *node)
{
	MovieTextureStack *st = (MovieTextureStack *) Node_GetPrivate(node);
	texture_destroy(&st->txh);
	if (st->time_handle.is_registered) SR_UnregisterTimeNode(st->txh.compositor, &st->time_handle);
	free(st);
}
static Float MT_GetSpeed(MovieTextureStack *stack, M_MovieTexture *mt)
{
	return MO_GetSpeed(stack->txh.stream, mt->speed);
}
static Bool MT_GetLoop(MovieTextureStack *stack, M_MovieTexture *mt)
{
	return MO_GetLoop(stack->txh.stream, mt->loop);
}
static void MT_Activate(MovieTextureStack *stack, M_MovieTexture *mt)
{
	mt->isActive = 1;
	Node_OnEventOutSTR((SFNode*)mt, "isActive");
	if (!stack->txh.is_open) {
		texture_play(&stack->txh, &mt->url);
	}
	MO_SetSpeed(stack->txh.stream, mt->speed);
}
static void MT_Deactivate(MovieTextureStack *stack, M_MovieTexture *mt)
{
	mt->isActive = 0;
	Node_OnEventOutSTR((SFNode*)mt, "isActive");
	stack->time_handle.needs_unregister = 1;

	if (stack->txh.is_open) {
		texture_stop(&stack->txh);
	}
}
static void UpdateMovieTexture(TextureHandler *txh)
{
	M_MovieTexture *txnode = (M_MovieTexture *) txh->owner;
	MovieTextureStack *st = (MovieTextureStack *) Node_GetPrivate(txh->owner);
	
	/*setup texture if needed*/
	if (!txh->is_open) return;
	if (!txnode->isActive && st->first_frame_fetched) return;

	/*when fetching the first frame disable resync*/
	texture_update_frame(txh, !txnode->isActive);

	if (txh->stream_finished) {
		if (MT_GetLoop(st, txnode)) {
			texture_restart(txh);
		}
		/*if active deactivate*/
		else if (txnode->isActive && MO_ShouldDeactivate(st->txh.stream) ) {
			MT_Deactivate(st, txnode);
		}
	}
	/*first frame is fetched*/
	if (!st->first_frame_fetched && (txh->needs_refresh) ) {
		st->first_frame_fetched = 1;
		txnode->duration_changed = MO_GetDuration(txh->stream);
		Node_OnEventOutSTR(txh->owner, "duration_changed");
		/*stop stream if needed*/
		if (!txnode->isActive && txh->is_open) {
			texture_stop(txh);
			/*make sure the refresh flag is not cleared*/
			txh->needs_refresh = 1;
		}
	}
	/*we have no choice but retraversing the graph until we're inactive since the movie framerate and
	the renderer framerate are likely to be different*/
	if (!txh->stream_finished) SR_Invalidate(txh->compositor, NULL);

}
static void MT_UpdateTime(TimeNode *st)
{
	Double time;
	M_MovieTexture *mt = (M_MovieTexture *)st->obj;
	MovieTextureStack *stack = Node_GetPrivate(st->obj);
	
	/*not active, store start time and speed*/
	if ( ! mt->isActive) {
		stack->start_time = mt->startTime;
	}
	time = Node_GetSceneTime(st->obj);

	if (time < stack->start_time ||
		/*special case if we're getting active AFTER stoptime */
		(!mt->isActive && (mt->stopTime > stack->start_time) && (time>=mt->stopTime))
		) {
		/*opens stream only at first access to fetch first frame*/
		if (stack->fetch_first_frame) {
			stack->fetch_first_frame = 0;
			if (!stack->txh.is_open)
				texture_play(&stack->txh, &mt->url);
		}
		return;
	}

	if (MT_GetSpeed(stack, mt) && mt->isActive) {
		/*if stoptime is reached (>startTime) deactivate*/
		if ((mt->stopTime > stack->start_time) && (time >= mt->stopTime) ) {
			MT_Deactivate(stack, mt);
			return;
		}
	}

	/*we're (about to be) active: VRML:
	"A time-dependent node is inactive until its startTime is reached. When time now becomes greater than or 
	equal to startTime, an isActive TRUE event is generated and the time-dependent node becomes active 	*/

	if (! mt->isActive) MT_Activate(stack, mt);
}
void InitMovieTexture(SceneRenderer *sr, SFNode *node)
{
	MovieTextureStack *st = malloc(sizeof(MovieTextureStack));
	memset(st, 0, sizeof(MovieTextureStack));
	texture_setup(&st->txh, sr, node);
	st->txh.update_texture_fcnt = UpdateMovieTexture;
	st->time_handle.UpdateTimeNode = MT_UpdateTime;
	st->time_handle.obj = node;
	st->fetch_first_frame = 1;
	st->txh.flags = 0;
	if (((M_MovieTexture*)node)->repeatS) st->txh.flags |= TX_REPEAT_S;
	if (((M_MovieTexture*)node)->repeatT) st->txh.flags |= TX_REPEAT_T;
	
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyMovieTexture);
	
	SR_RegisterTimeNode(sr, &st->time_handle);
}

TextureHandler *mt_get_texture(SFNode *node)
{
	MovieTextureStack *st = (MovieTextureStack *) Node_GetPrivate(node);
	return &st->txh;
}

void MovieTextureModified(SFNode *node)
{
	M_MovieTexture *mt = (M_MovieTexture *)node;
	MovieTextureStack *st = (MovieTextureStack *) Node_GetPrivate(node);
	if (!st) return;

	/*if open and changed, stop and play*/
	if (st->txh.is_open && texture_check_url_change(&st->txh, &mt->url)) {
		texture_stop(&st->txh);
		texture_play(&st->txh, &mt->url);
	} 
	/*update state if we're active*/
	else if (mt->isActive) {
		MT_UpdateTime(&st->time_handle);
		if (!mt->isActive) return;
	}
	/*reregister if needed*/
	st->time_handle.needs_unregister = 0;
	if (!st->time_handle.is_registered) SR_RegisterTimeNode(st->txh.compositor, &st->time_handle);
}

typedef struct
{
	TextureHandler txh;
} ImageTextureStack;

static void DestroyImageTexture(SFNode *node)
{
	ImageTextureStack *st = (ImageTextureStack *) Node_GetPrivate(node);
	texture_destroy(&st->txh);
	free(st);
}
static void UpdateImageTexture(TextureHandler *txh)
{
	M_ImageTexture *txnode = (M_ImageTexture *) txh->owner;
	
	/*setup texture if needed*/
	if (!txh->is_open && txnode->url.count) {
		texture_play(txh, &txnode->url);
	}
	texture_update_frame(txh, 0);
	/*URL is present but not opened - redraw till fetch*/
	if (txh->stream && !txh->hwtx) SR_Invalidate(txh->compositor, NULL);
}

void InitImageTexture(SceneRenderer *sr, SFNode *node)
{
	ImageTextureStack *st = malloc(sizeof(ImageTextureStack));
	texture_setup(&st->txh, sr, node);
	st->txh.update_texture_fcnt = UpdateImageTexture;
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyImageTexture);
	st->txh.flags = 0;
	if (((M_ImageTexture*)node)->repeatS) st->txh.flags |= TX_REPEAT_S;
	if (((M_ImageTexture*)node)->repeatT) st->txh.flags |= TX_REPEAT_T;
}

TextureHandler *it_get_texture(SFNode *node)
{
	ImageTextureStack *st = (ImageTextureStack *) Node_GetPrivate(node);
	return &st->txh;
}
void ImageTextureModified(SFNode *node)
{
	M_ImageTexture *im = (M_ImageTexture *)node;
	ImageTextureStack *st = (ImageTextureStack *) Node_GetPrivate(node);
	if (!st) return;

	/*if open and changed, stop and play*/
	if (st->txh.is_open) {
		if (! texture_check_url_change(&st->txh, &im->url)) return;
		texture_stop(&st->txh);
		texture_play(&st->txh, &im->url);
		return;
	}
	/*if not open and changed play*/
	if (im->url.count) 
		texture_play(&st->txh, &im->url);
}



typedef struct
{
	TextureHandler txh;
	char *pixels;
} PixelTextureStack;

static void DestroyPixelTexture(SFNode *node)
{
	PixelTextureStack *st = (PixelTextureStack *) Node_GetPrivate(node);
	if (st->pixels) free(st->pixels);
	texture_destroy(&st->txh);
	free(st);
}
static void UpdatePixelTexture(TextureHandler *txh)
{
	u32 pix_format, stride, i;
	M_PixelTexture *pt = (M_PixelTexture *) txh->owner;
	PixelTextureStack *st = (PixelTextureStack *) Node_GetPrivate(txh->owner);

	if (!Node_GetDirty(txh->owner)) return;
	Node_ClearDirty(txh->owner);

	
	/*pixel texture doesn not use any media object but has data in the content. 
	However we still use the same texture object, just be carefull not to use media funtcions*/
	txh->transparent = 0;
	stride = pt->image.width;
	/*num_components are as in VRML (1->4) not as in BIFS bitstream (0->3)*/
	switch (pt->image.numComponents) {
	case 1:
		pix_format = M4PF_GREYSCALE;
		break;
	case 2:
		pix_format = M4PF_ALPHAGREY;
		txh->transparent = 1;
		stride *= 2;
		break;
	case 3:
		pix_format = M4PF_RGB_24;
		txh->transparent = 0;
		stride *= 3;
		break;
	case 4:
		pix_format = M4PF_ARGB;
		txh->transparent = 1;
		stride *= 4;
		break;
	default:
		return;
	}

	if (!txh->hwtx) txh->compositor->visual_renderer->AllocTexture(txh);
	if (!txh->hwtx) return;

	if (st->pixels) free(st->pixels);
	st->pixels = malloc(sizeof(char) * stride * pt->image.height);
	if (txh->compositor->visual_renderer->bNeedsGL) {
		for (i=0; i<pt->image.height; i++) {
			memcpy(st->pixels + i * stride, pt->image.pixels + i * stride, stride);
		}
	} 
	/*revert pixel ordering...*/
	else {
		for (i=0; i<pt->image.height; i++) {
			memcpy(st->pixels + i * stride, pt->image.pixels + (pt->image.height - 1 - i) * stride, stride);
		}
	}

	txh->width = pt->image.width;
	txh->height = pt->image.height;
	txh->stride = stride;
	txh->pixelformat = pix_format;
	txh->data = st->pixels;

	txh->active_window.x = txh->active_window.y = 0;
	txh->active_window.width = (Float) pt->image.width;
	txh->active_window.height = (Float) pt->image.height;

	txh->compositor->visual_renderer->SetTextureData(txh);
}

void InitPixelTexture(SceneRenderer *sr, SFNode *node)
{
	PixelTextureStack *st = malloc(sizeof(PixelTextureStack));
	texture_setup(&st->txh, sr, node);
	st->pixels = NULL;
	st->txh.update_texture_fcnt = UpdatePixelTexture;

	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyPixelTexture);
	st->txh.flags = 0;
	if (((M_PixelTexture*)node)->repeatS) st->txh.flags |= TX_REPEAT_S;
	if (((M_PixelTexture*)node)->repeatT) st->txh.flags |= TX_REPEAT_T;
}

TextureHandler *pt_get_texture(SFNode *node)
{
	PixelTextureStack *st = (PixelTextureStack *) Node_GetPrivate(node);
	return &st->txh;
}


TextureHandler *texture_get_handler(SFNode *n)
{
	if (!n) return NULL;
	switch (Node_GetTag(n)) {
	case TAG_MPEG4_ImageTexture: case TAG_X3D_ImageTexture: return it_get_texture(n);
	case TAG_MPEG4_MovieTexture: case TAG_X3D_MovieTexture: return mt_get_texture(n);
	case TAG_MPEG4_PixelTexture: case TAG_X3D_PixelTexture: return pt_get_texture(n);
	default: return NULL;
	}
}
