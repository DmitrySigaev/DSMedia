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


#include "render2d.h"
#include "stacks2d.h"
#include "visualsurface2d.h"


void R2D_MapCoordsToAR(VisualRenderer *vr, s32 inX, s32 inY, Float *x, Float *y)
{
	Float ft;
	Render2D *sr = (Render2D*)vr->user_priv;

	/*add offsets of aspect ratio*/
	*x = (Float) inX - sr->out_x;
	*y = (Float) inY + sr->out_y;

	/*if no size info scaling is never applied*/
	if (!sr->compositor->has_size_info) return;

	if (sr->scalable_zoom) {
		ft = (Float) sr->cur_width / (Float) sr->out_width;
		*x *= ft;
		ft = (Float) sr->cur_height / (Float) sr->out_height;
		*y *= ft;
	} else {
		*x -= ((s32)sr->out_width - (s32)sr->compositor->scene_width) / 2;
		*y += ((s32)sr->out_height - (s32)sr->compositor->scene_height) / 2;
		ft = (Float)sr->compositor->scene_width / (Float) sr->out_width;
		*x *= ft;
		ft = (Float)sr->compositor->scene_height / (Float) sr->out_height;
		*y *= ft;
	}
}

static void R2D_SetZoom(Render2D *sr, Float zoom) 
{
	Float ratio;

	SR_Lock(sr->compositor, 1);
	if (zoom <= 0.0) zoom = 0.001f;
	if (zoom != sr->zoom) {
		ratio = zoom/sr->zoom;
		sr->trans_x *= ratio;
		sr->trans_y *= ratio;
		sr->zoom = zoom;
	}
	mx2d_init(sr->top_effect->transform);
	mx2d_add_scale(&sr->top_effect->transform, sr->scale_x, sr->scale_y);
	mx2d_add_scale(&sr->top_effect->transform, sr->zoom, sr->zoom);
	mx2d_add_translation(&sr->top_effect->transform, sr->trans_x, sr->trans_y);
	sr->compositor->draw_next_frame = 1;
	SR_Lock(sr->compositor, 0);
}

void R2D_SetScaling(Render2D *sr, Float scaleX, Float scaleY)
{
	sr->scale_x = scaleX;
	sr->scale_y = scaleY;
	R2D_SetZoom(sr, sr->zoom);
}

void R2D_ResetSurfaces(Render2D *sr)
{
	u32 i;
	for (i=0; i<ChainGetCount(sr->surfaces_2D); i++) {
		VisualSurface2D *surf = ChainGetEntry(sr->surfaces_2D, i);
		while (ChainGetCount(surf->prev_nodes_drawn)) ChainDeleteEntry(surf->prev_nodes_drawn, 0);
		surf->to_redraw.count = 0;
		VS2D_ResetSensors(surf);
	}
}

void R2D_SceneReset(VisualRenderer *vr)
{
	u32 flag;
	Render2D *sr = (Render2D*) vr->user_priv;
	if (!sr) return;
	R2D_ResetSurfaces(sr);
	while (ChainGetCount(sr->sensors)) {
		ChainDeleteEntry(sr->sensors, 0);
	}

	flag = sr->top_effect->trav_flags;
	effect_reset(sr->top_effect);
	sr->top_effect->trav_flags = flag;
	sr->compositor->reset_graphics = 1;
	sr->trans_x = sr->trans_y = 0;
	sr->zoom = 1.0;
	sr->main_surface_setup = 0;
	R2D_SetScaling(sr, sr->scale_x, sr->scale_y);
}

M4Rect R2D_ClipperToPixelMetrics(RenderEffect2D *eff, SFVec2f size)
{
	M4Rect res;

	if (eff->surface->composite) {
		res.width = (Float) eff->surface->width;
		res.height = (Float) eff->surface->height;
	} else {
		res.width = (Float) eff->surface->render->compositor->scene_width;
		res.height = (Float) eff->surface->render->compositor->scene_height;
	}
	if (eff->is_pixel_metrics) {
		if (size.x>=0) res.width = size.x;
		if (size.y>=0) res.height = size.y;
	} else {
		if (size.x>=0) res.width *= size.x / 2;
		if (size.y>=0) res.height *= size.y / 2;
	}
	m4_rect_center(&res, res.width, res.height);
	return res;
}


void R2D_RegisterSurface(Render2D *sr, struct _visual_surface_2D  *surf)
{
	if (R2D_IsSurfaceRegistered(sr, surf)) return;
	ChainAddEntry(sr->surfaces_2D, surf);
}

void R2D_UnregisterSurface(Render2D *sr, struct _visual_surface_2D  *surf)
{
	ChainDeleteItem(sr->surfaces_2D, surf);
}

Bool R2D_IsSurfaceRegistered(Render2D *sr, struct _visual_surface_2D *surf)
{
	u32 i;
	for (i=0; i<ChainGetCount(sr->surfaces_2D); i++) {
		if (ChainGetEntry(sr->surfaces_2D, i) == surf) return 1;
	}
	return 0;
}

void effect_add_sensor(RenderEffect2D *eff, SensorHandler *ptr, M4Matrix2D *mat)
{
	SensorContext *ctx;
	if (!ptr) return;
	ctx = malloc(sizeof(SensorContext));
	ctx->h_node = ptr;
	
	if (mat) {
		mx2d_copy(ctx->matrix, *mat);
	} else {
		mx2d_init(ctx->matrix);
	}
	ChainAddEntry(eff->sensors, ctx);
}

void effect_reset_sensors(RenderEffect2D *eff)
{
	SensorContext *ctx;
	while (ChainGetCount(eff->sensors)) {
		ctx = ChainGetEntry(eff->sensors, 0);
		ChainDeleteEntry(eff->sensors, 0);
		free(ctx);
	}
}

void effect_reset(RenderEffect2D *eff)
{
	Chain *bck = eff->sensors;
	memset(eff, 0, sizeof(RenderEffect2D));
	eff->sensors = bck;
	if (bck) effect_reset_sensors(eff);
	mx2d_init(eff->transform);
	cmat_init(&eff->color_mat);
}

void effect_delete(RenderEffect2D *eff)
{
	if (eff->sensors) {
		effect_reset_sensors(eff);
		DeleteChain(eff->sensors);
	}
	free(eff);
}

Bool is_sensor_node(SFNode *node)
{
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_TouchSensor:
	case TAG_MPEG4_PlaneSensor2D:
	case TAG_MPEG4_DiscSensor:
	case TAG_MPEG4_ProximitySensor2D: 
		return 1;

		/*anchor is not considered as a child sensor node when picking sensors*/
	/*case TAG_MPEG4_Anchor:*/
	default:
		return 0;
	}
}

SensorHandler *get_sensor_handler(SFNode *n)
{
	SensorHandler *hs;

	switch (Node_GetTag(n)) {
	case TAG_MPEG4_Anchor: hs = r2d_anchor_get_handler(n); break;
	case TAG_MPEG4_DiscSensor: hs = r2d_ds_get_handler(n); break;
	case TAG_MPEG4_TouchSensor: hs = r2d_touch_sensor_get_handler(n); break;
	case TAG_MPEG4_PlaneSensor2D: hs = r2d_ps2D_get_handler(n); break;
	case TAG_MPEG4_ProximitySensor2D: hs = r2d_prox2D_get_handler(n); break;
	default:
		return NULL;
	}
	if (hs && hs->IsEnabled(hs)) return hs;
	return NULL;
}

void R2D_RegisterSensor(SceneRenderer *compositor, SensorHandler *sh)
{
	u32 i;
	Render2D *sr = (Render2D *)compositor->visual_renderer->user_priv;
	for (i=0; i<ChainGetCount(sr->sensors); i++) {
		if (ChainGetEntry(sr->sensors, i) == sh) return;
	}
	ChainAddEntry(sr->sensors, sh);
}

void R2D_UnregisterSensor(SceneRenderer *compositor, SensorHandler *sh)
{
	Render2D *sr = (Render2D *)compositor->visual_renderer->user_priv;
	ChainDeleteItem(sr->sensors, sh);
}


#define R2DSETCURSOR(t) { M4Event evt; evt.type = M4E_SET_CURSOR; evt.cursor.cursor_type = (t); sr->compositor->video_out->PushEvent(sr->compositor->video_out, &evt); }

static Bool R2D_ExecuteEvent(VisualRenderer *vr, M4UserEvent *event)
{
	u32 i, type, count;
	Bool act;
	Float key_inv, key_trans;
	DrawableContext *ctx;
	UserEvent2D evt, *ev;
	Render2D *sr = (Render2D *)vr->user_priv;

	evt.context = NULL;
	evt.event_type = event->event_type;
	evt.x = 0;
	evt.y = 0;
	ev = &evt;

	if (event->event_type<=M4E_MOUSEWHEEL) R2D_MapCoordsToAR(vr, event->mouse.x, event->mouse.y, &evt.x, &evt.y);
	if (event->event_type>M4E_LEFTUP) goto no_sensor;
	
	if (sr->is_tracking) {
		/*in case a node is inserted at the depth level of a node previously tracked (rrrhhhaaaa...) */
		if (sr->grab_ctx && sr->grab_ctx->node != sr->grab_node) {
			sr->is_tracking = 0;
			sr->grab_ctx = NULL;
		}
	}
	
	if (!sr->is_tracking) {
		ctx = VS2D_FindNode(sr->surface, ev->x, ev->y);
		sr->grab_ctx = ctx;
		if (ctx) sr->grab_node = ctx->node;
	} else {
		ctx = sr->grab_ctx;
	}

	//3- mark all sensors of the context to skip deactivation
	ev->context = ctx;
	if (ctx) {	
		count = ChainGetCount(ctx->sensors);
		for (i=0; i<count; i++) {
			SensorContext *sc = ChainGetEntry(ctx->sensors, i);
			sc->h_node->skip_second_pass = 1;
		}

		if (sr->compositor->video_out->PushEvent) {
			SensorContext *sc = ChainGetEntry(ctx->sensors, count-1);
			//also notify the app we're above a sensor
			type = M4CursorNormal;
			switch (Node_GetTag(sc->h_node->owner)) {
			case TAG_MPEG4_Anchor: type = M4CursorAnchor; break;
			case TAG_MPEG4_PlaneSensor2D: type = M4CursorPlane; break;
			case TAG_MPEG4_DiscSensor: type = M4CursorRotate; break;
			case TAG_MPEG4_ProximitySensor2D: type = M4CursorProximity; break;
			case TAG_MPEG4_TouchSensor: type = M4CursorTouch; break;
			}
			if (type != M4CursorNormal) {
				if (sr->last_sensor != type) {
					M4Event evt;
					evt.type = M4E_SET_CURSOR;
					evt.cursor.cursor_type = type;
					sr->compositor->video_out->PushEvent(sr->compositor->video_out, &evt);
					sr->last_sensor = type;
				}
			}
		}
	}

	if (sr->compositor->video_out->PushEvent && !ctx && (sr->last_sensor != M4CursorNormal)) {
		R2DSETCURSOR(M4CursorNormal);
		sr->last_sensor = M4CursorNormal;
	}

	/*deactivate all other registered sensors*/
	ev->context = NULL;
	for (i=0; i< ChainGetCount(sr->sensors); i++) {
		SensorHandler *sh = ChainGetEntry(sr->sensors, i);
		act = ! sh->skip_second_pass;
		sh->skip_second_pass = 0;
		count = ChainGetCount(sr->sensors);
		if (act)
			sh->OnUserEvent(sh, ev, NULL);
		if (count != ChainGetCount(sr->sensors)) i-= 1;
	}	
	
	/*activate current one if any*/
	if (ctx) {
		ev->context = ctx;
		for (i=ChainGetCount(ctx->sensors); i>0; i--) {
			SensorContext *sc = ChainGetEntry(ctx->sensors, i-1);
			sc->h_node->skip_second_pass = 0;
			sc->h_node->OnUserEvent(sc->h_node, ev, &sc->matrix);
		}
		return 1;
	}


no_sensor:
	/*no object, perform zoom & pan*/
	if (!(sr->compositor->interaction_level & M4_InteractZoomPan) || !sr->navigate_mode) return 0;

	key_inv = 1;
	key_trans = 2;
	if (sr->compositor->key_states&M4KM_SHIFT) key_trans *= 4;

	switch (event->event_type) {
	case M4E_LEFTDOWN:
		sr->grab_x = (s32) ev->x;
		sr->grab_y = (s32) ev->y;
		sr->grabbed = 1;
		break;
		break;
	case M4E_LEFTUP:
		sr->grabbed = 0;
		break;
	case M4E_MOUSEMOVE:
		if (sr->grabbed && (sr->navigate_mode == M4_NavSlide)) {
			Float dx, dy;
			dx = ev->x - sr->grab_x;
			dy = ev->y - sr->grab_y;
			if (! SG_UsePixelMetrics(sr->compositor->scene)) {
				dx /= sr->cur_width;
				dy /= sr->cur_height;
			}
			/*set zoom*/
			if (sr->compositor->key_states & M4KM_CTRL) {
				Float new_zoom = sr->zoom;
				if (new_zoom > 1.0) new_zoom += dy/10;
				else new_zoom += dy/40;
				R2D_SetZoom(sr, new_zoom);
			}
			/*set pan*/
			else {
				sr->trans_x += dx;
				sr->trans_y += dy;
				R2D_SetZoom(sr, sr->zoom);
			}
			sr->grab_x = (s32) ev->x;
			sr->grab_y = (s32) ev->y;
		}
		break;
	case M4E_VKEYDOWN:
		switch (event->key.m4_vk_code) {
		case M4VK_HOME:
			if (!sr->grabbed) {
				sr->zoom = 1.0;
				sr->trans_x = sr->trans_y = 0.0;
				R2D_SetZoom(sr, sr->zoom);
			}
			break;
		case M4VK_LEFT: key_inv = -1;
		case M4VK_RIGHT:
			sr->trans_x += key_inv*key_trans;
			break;
		case M4VK_DOWN: key_inv = -1;
		case M4VK_UP:
			if (sr->compositor->key_states & M4KM_CTRL) {
				Float new_zoom = sr->zoom;
				if (new_zoom > 1.0) new_zoom += key_inv*0.1f;
				else new_zoom += key_inv*0.05f;
				R2D_SetZoom(sr, new_zoom);
			} else {
				sr->trans_y += key_inv*key_trans;
			}
			break;
		}
		break;
	}
	return 0;
}

void R2D_DrawScene(VisualRenderer *vr)
{
	M4Window rc;
	u32 i;
	RenderEffect2D static_eff;
	Render2D *sr = (Render2D *)vr->user_priv;
	SFNode *top_node = SG_GetRootNode(sr->compositor->scene);
	if (!top_node) return;

	if (!sr->compositor->scene) {
		sr->compositor->video_out->Clear(sr->compositor->video_out, sr->back_color);
		return;
	}

	memcpy(&static_eff, sr->top_effect, sizeof(RenderEffect2D));

	if (!sr->main_surface_setup) {
		sr->main_surface_setup = 1;
		sr->surface->center_coords = 1;
		sr->surface->default_back_color = 0xFF000000;

#ifdef M4_USE_LASeR
		{
			u32 node_tag = Node_GetTag(top_node);
			if ((node_tag>=TAG_RANGE_FIRST_LASeR) && (node_tag<=TAG_RANGE_LAST_LASeR))
				sr->surface->default_back_color = 0xFFFFFFFF;
		}
#endif
#ifndef M4_DISABLE_SVG
		{
			u32 node_tag = Node_GetTag(top_node);
			if ((node_tag>=TAG_RANGE_FIRST_SVG) && (node_tag<=TAG_RANGE_LAST_SVG)) {
				sr->surface->default_back_color = 0xFFFFFFFF;
				sr->surface->center_coords = 0;
			}
		}
#endif
	}

	sr->surface->width = sr->cur_width;
	sr->surface->height = sr->cur_height;

	sr->top_effect->is_pixel_metrics = SG_UsePixelMetrics(sr->compositor->scene);
	sr->top_effect->min_hsize = MIN(sr->compositor->scene_width, sr->compositor->scene_height) * 0.5f;

	VS2D_InitDraw(sr->surface, sr->top_effect);
	Node_Render(top_node, sr->top_effect);

	for (i=0; i<ChainGetCount(sr->compositor->extra_scenes); i++) {
		LPSCENEGRAPH sg = ChainGetEntry(sr->compositor->extra_scenes, i);
		SFNode *n = SG_GetRootNode(sg);
		if (n) Node_Render(n, sr->top_effect);
	}

	VS2D_TerminateDraw(sr->surface, sr->top_effect);
	memcpy(sr->top_effect, &static_eff, sizeof(RenderEffect2D));
	sr->top_effect->invalidate_all = 0;
	/*and flush*/
	rc.x = sr->out_x; 
	rc.y = sr->out_y; 
	rc.w = sr->out_width;	
	rc.h = sr->out_height;		
	sr->compositor->video_out->FlushVideo(sr->compositor->video_out, &rc);
}

Bool R2D_IsPixelMetrics(SFNode *n)
{
	LPSCENEGRAPH sg = Node_GetParentGraph(n);
	return SG_UsePixelMetrics(sg);
}


static M4Err R2D_RecomputeAR(VisualRenderer *vr)
{
	Double ratio;
	Float scaleX, scaleY;
	Render2D *sr = (Render2D *)vr->user_priv;
	if (!sr->compositor->scene_height || !sr->compositor->scene_width) return M4OK;
	if (!sr->compositor->height || !sr->compositor->width) return M4OK;

	sr->out_width = sr->compositor->width;
	sr->out_height = sr->compositor->height;
	sr->cur_width = sr->compositor->scene_width;
	sr->cur_height = sr->compositor->scene_height;
	sr->out_x = 0;
	sr->out_y = 0;

	/*force complete clean*/
	sr->top_effect->invalidate_all = 1;

	if (!sr->compositor->has_size_info && !(sr->compositor->override_size_flags & 2) ) {
		sr->compositor->scene_width = sr->cur_width = sr->out_width;
		sr->compositor->scene_height = sr->cur_height = sr->out_height;
		R2D_SetScaling(sr, 1, 1);
		/*and resize hardware surface*/
		return sr->compositor->video_out->Resize(sr->compositor->video_out, sr->cur_width, sr->cur_height);
	}

	switch (sr->compositor->aspect_ratio) {
	case M4_AR_None:
		break;
	case M4_AR_16_9:
		sr->out_width = sr->compositor->width;
		sr->out_height = 9 * sr->compositor->width / 16;
		if (sr->out_height>sr->compositor->height) {
			sr->out_height = sr->compositor->height;
			sr->out_width = 16 * sr->compositor->height / 9;
		}
		break;
	case M4_AR_4_3:
		sr->out_width = sr->compositor->width;
		sr->out_height = 3 * sr->compositor->width / 4;
		if (sr->out_height>sr->compositor->height) {
			sr->out_height = sr->compositor->height;
			sr->out_width = 4 * sr->compositor->height / 3;
		}
		break;
	default:
		ratio = sr->compositor->scene_height;
		ratio /= sr->compositor->scene_width;
		if (sr->out_width * ratio > sr->out_height) {
			sr->out_width = sr->out_height * sr->compositor->scene_width;
			sr->out_width /= sr->compositor->scene_height;
		}
		else {
			sr->out_height = sr->out_width * sr->compositor->scene_height;
			sr->out_height /= sr->compositor->scene_width;
		}
		break;
	}
	sr->out_x = (sr->compositor->width - sr->out_width) / 2;
	sr->out_y = (sr->compositor->height - sr->out_height) / 2;
	/*clear screen*/
	sr->compositor->video_out->Clear(sr->compositor->video_out, sr->back_color);

	if (!sr->scalable_zoom) {
		sr->cur_width = sr->compositor->scene_width;
		sr->cur_height = sr->compositor->scene_height;
		scaleX = 1.0;
		scaleY = 1.0;
	} else {
		sr->cur_width = sr->out_width;
		sr->cur_height = sr->out_height;
		scaleX = (Float)sr->out_width / sr->compositor->scene_width;
		scaleY = (Float)sr->out_height / sr->compositor->scene_height;
	}
	/*set scale factor*/
	R2D_SetScaling(sr, scaleX, scaleY);
	SR_Invalidate(sr->compositor, NULL);
	/*and resize hardware surface*/
	return sr->compositor->video_out->Resize(sr->compositor->video_out, sr->cur_width, sr->cur_height);
}

SFNode *R2D_PickNode(VisualRenderer *vr, s32 X, s32 Y)
{
	Float x, y;
	SFNode *res = NULL;
	Render2D *sr = (Render2D *)vr->user_priv;

	if (!sr) return NULL;
	/*lock to prevent any change while picking*/
	SR_Lock(sr->compositor, 1);
	if (sr->compositor->scene) {
		R2D_MapCoordsToAR(vr, X, Y, &x, &y);
		res = VS2D_PickNode(sr->surface, x, y);
	}
	SR_Lock(sr->compositor, 0);
	return res;
}


M4Err R2D_GetSurfaceAccess(VisualSurface2D *surf)
{
	M4Err e;
	Render2D *sr = surf->render;

	if (!sr->compositor->g_hw || !surf->the_surface) return M4BadParam;
	sr->locked = 0;
	e = M4IOErr;
	/*try from device*/
	if (sr->compositor->g_hw->attach_surface_to_device && sr->compositor->video_out->GetContext) {
		sr->hardware_context = sr->compositor->video_out->GetContext(sr->compositor->video_out, 0);
		if (sr->hardware_context) {
			e = sr->compositor->g_hw->attach_surface_to_device(surf->the_surface, sr->hardware_context, sr->cur_width, sr->cur_height);
			if (!e) {
				surf->is_attached = 1;
				return M4OK;
			}
			sr->compositor->video_out->ReleaseContext(sr->compositor->video_out, 0, sr->hardware_context);
		}
	}

	/*try from buffer*/
	if (sr->compositor->g_hw->attach_surface_to_buffer) {
		if (sr->compositor->video_out->LockSurface(sr->compositor->video_out, 0, &sr->hw_surface)==M4OK) {
			sr->locked = 1;
			e = sr->compositor->g_hw->attach_surface_to_buffer(surf->the_surface, sr->hw_surface.video_buffer, 
								sr->hw_surface.width, 
								sr->hw_surface.height,
								sr->hw_surface.pitch,
								sr->hw_surface.pixel_format);
			if (!e) {
				surf->is_attached = 1;
				return M4OK;
			}
			sr->compositor->video_out->UnlockSurface(sr->compositor->video_out, 0);
		}
		sr->locked = 0;
	}
	surf->is_attached = 0;
	return e;		
}

void R2D_ReleaseSurfaceAccess(VisualSurface2D *surf)
{
	Render2D *sr = surf->render;
	if (surf->is_attached) {
		sr->compositor->g_hw->detach_surface(surf->the_surface);
		surf->is_attached = 0;
	}
	if (sr->hardware_context) {
		sr->compositor->video_out->ReleaseContext(sr->compositor->video_out, 0, sr->hardware_context);
		sr->hardware_context = NULL;
	} else if (sr->locked) {
		sr->compositor->video_out->UnlockSurface(sr->compositor->video_out, 0);
		sr->locked = 0;
	}
}

Bool R2D_SupportsFormat(VisualSurface2D *surf, u32 pixel_format)
{
	switch (pixel_format) {
	case M4PF_RGB_24:
	case M4PF_BGR_24:
	case M4PF_YV12:
	case M4PF_IYUV:
	case M4PF_I420:
		return 1;
	/*the rest has to be displayed through brush for now, we only use YUV and RGB pool*/
	default:
		return 0;
	}
}

void R2D_DrawBitmap(VisualSurface2D *surf, struct _texture_handler *txh, M4IRect *clip, M4Rect *unclip)
{
	Float w_scale, h_scale;
	M4VideoSurface dest;
	M4Err e;
	M4Window src_wnd, dst_wnd;
	u32 start_x, start_y, format, cur_width, cur_height;
	u32 *pool_id;
	M4IRect clipped_final = *clip;
	M4Rect final = *unclip;

	if (!txh->data) return;

	if (!surf->render->compositor->has_size_info && (surf->render->compositor->msg_type != 1) 
		&& (surf->render->compositor->override_size_flags & 1) 
		&& !(surf->render->compositor->override_size_flags & 2) 
		) {
		if ( (surf->render->compositor->scene_width < txh->width) 
			|| (surf->render->compositor->scene_height < txh->height)) {
			surf->render->compositor->scene_width = txh->width;
			surf->render->compositor->scene_height = txh->height;
			surf->render->compositor->msg_type = 1;
			return;
		}
	}
	
	/*this should never happen but we check for float rounding safety*/
	if (final.width<=0 || final.height <=0) return;

	w_scale = final.width / txh->width;
	h_scale = final.height / txh->height;

	/*take care of pixel rounding for odd width/height and make sure we strictly draw in the clipped bounds*/
	cur_width = surf->render->cur_width;
	cur_height = surf->render->cur_height;

	if (surf->center_coords) {
		if (cur_width % 2) {
			clipped_final.x += (cur_width-1) / 2;
			final.x += (cur_width-1) / 2;
		} else {
			clipped_final.x += cur_width / 2;
			final.x += cur_width / 2;
		}
		if (cur_height % 2) {
			clipped_final.y = (cur_height-1) / 2 - clipped_final.y;
			final.y = (cur_height - 1) / 2 - final.y;
		} else {
			clipped_final.y = cur_height/ 2 - clipped_final.y;
			final.y = cur_height / 2 - final.y;
		}
	} else {
		final.y -= final.height;
		clipped_final.y -= clipped_final.height;
	}

	/*make sure we lie in the final rect (this is needed for directRender mode)*/
	if (clipped_final.x<0) {
		clipped_final.width += clipped_final.x;
		clipped_final.x = 0;
		if (clipped_final.width <= 0) return;
	}
	if (clipped_final.y<0) {
		clipped_final.height += clipped_final.y;
		clipped_final.y = 0;
		if (clipped_final.height <= 0) return;
	}
	if (clipped_final.x + clipped_final.width > (s32) cur_width) {
		clipped_final.width = cur_width - clipped_final.x;
		clipped_final.x = cur_width - clipped_final.width;
	}
	if (clipped_final.y + clipped_final.height > (s32) cur_height) {
		clipped_final.height = cur_height - clipped_final.y;
		clipped_final.y = cur_height - clipped_final.height;
	}
	/*needed in direct rendering since clipping is not performed*/
	if (clipped_final.width<=0 || clipped_final.height <=0) 
		return;

	start_x = 0;
	if (clipped_final.x >= final.x)
		start_x = (u32) ((clipped_final.x - final.x) / w_scale );


	start_y = 0;
	if (clipped_final.y >= final.y)
		start_y = (u32) ( (clipped_final.y - final.y) / h_scale );
	
	dst_wnd.x = (u32) clipped_final.x;
	dst_wnd.y = (u32) clipped_final.y;
	dst_wnd.w = (u32) clipped_final.width;
	dst_wnd.h = (u32) clipped_final.height;

	src_wnd.w = (u32) (dst_wnd.w / w_scale);
	src_wnd.h = (u32) (dst_wnd.h / h_scale);
	if (src_wnd.w>txh->width) src_wnd.w=txh->width;
	if (src_wnd.h>txh->height) src_wnd.h=txh->height;
	
	src_wnd.x = start_x;
	src_wnd.y = start_y;


	if (!src_wnd.w || !src_wnd.h) return;
	/*make sure we lie in src bounds*/
	if (src_wnd.x + src_wnd.w>txh->width) src_wnd.w = txh->width - src_wnd.x;
	if (src_wnd.y + src_wnd.h>txh->height) src_wnd.h = txh->height - src_wnd.y;

	/*get the right surface and copy the part of the image on it*/
	switch (txh->pixelformat) {
	case M4PF_RGB_24:
	case M4PF_BGR_24:
		format = surf->pixel_format;
		pool_id = &surf->render->pool_rgb;
		break;
	case M4PF_YV12:
	case M4PF_IYUV:
	case M4PF_I420:
		/*we must use even coords rect for YUV src otherwise we'll introduce artefacts on U and V planes*/
		if (src_wnd.x % 2) {
			src_wnd.x -= 1;
			src_wnd.w += 1;
		}
		if (src_wnd.y % 2) {
			src_wnd.y -= 1;
			src_wnd.h += 1;
		}
		if (src_wnd.w % 2) src_wnd.w -= 1;
		if (src_wnd.h % 2) src_wnd.h -= 1;
		if (surf->render->compositor->video_out->bHasYUV && surf->render->enable_yuv_hw) {
			format = M4PF_YV12;
			pool_id = &surf->render->pool_yuv;
		} else {
			format = surf->pixel_format;
			pool_id = &surf->render->pool_rgb;
		}
		break;
	default:
		return;
	}

	e = M4OK;
	if (! *pool_id || !surf->render->compositor->video_out->IsSurfaceValid(surf->render->compositor->video_out, *pool_id)) {
		e = surf->render->compositor->video_out->CreateSurface(surf->render->compositor->video_out, src_wnd.w, src_wnd.h, format, pool_id);
		if (!e && (pool_id == &surf->render->pool_yuv)) {
			surf->render->compositor->video_out->GetPixelFormat(surf->render->compositor->video_out, *pool_id, &surf->render->current_yuv_format);
		}

		if ((e!=M4OK) || !*pool_id) {
			surf->render->current_yuv_format = 0;
			/*otherwise try with soft YUV*/
			pool_id = &surf->render->pool_rgb;
			format = surf->pixel_format;
			if (! *pool_id || !surf->render->compositor->video_out->IsSurfaceValid(surf->render->compositor->video_out, *pool_id) ) {
				e = surf->render->compositor->video_out->CreateSurface(surf->render->compositor->video_out, src_wnd.w, src_wnd.h, format, pool_id);
				if ((e!=M4OK) || ! *pool_id) return;
			} else {
				 e = surf->render->compositor->video_out->ResizeSurface(surf->render->compositor->video_out, *pool_id, src_wnd.w, src_wnd.h);
			}
		}
	} else {
		e = surf->render->compositor->video_out->ResizeSurface(surf->render->compositor->video_out, *pool_id, src_wnd.w, src_wnd.h);
	}
	if (e) return;

	/*lock*/
	e = surf->render->compositor->video_out->LockSurface(surf->render->compositor->video_out, *pool_id, &dest);
	if (e) return;
	
	
	R2D_copyPixels(&dest, txh->data, txh->stride, txh->width, txh->height, txh->pixelformat, &src_wnd);

	src_wnd.x = src_wnd.y = 0;

	/*unlock*/
	e = surf->render->compositor->video_out->UnlockSurface(surf->render->compositor->video_out, *pool_id);
	if (e) return;

	/*arg - most graphic cards can't perform bliting on locked surface - force unlock by releasing the hardware*/
	VS2D_TerminateSurface(surf);
	surf->render->compositor->video_out->Blit(surf->render->compositor->video_out, *pool_id, 0, &src_wnd, &dst_wnd);
	VS2D_InitSurface(surf);

	return;
}

M4Err R2D_LoadRenderer(VisualRenderer *vr, SceneRenderer *compositor)
{
	Render2D *sr;
	const char *sOpt;
	if (vr->user_priv) return M4BadParam;

	sr = malloc(sizeof(Render2D));
	if (!sr) return M4OutOfMem;
	memset(sr, 0, sizeof(Render2D));

	sr->compositor = compositor;

	sr->strike_bank = NewChain();
	sr->surfaces_2D = NewChain();

	sr->top_effect = malloc(sizeof(RenderEffect2D));
	memset(sr->top_effect, 0, sizeof(RenderEffect2D));
	sr->top_effect->sensors = NewChain();
	sr->sensors = NewChain();
	
	/*and create main surface*/
	sr->surface = NewVisualSurface2D();
	sr->surface->GetSurfaceAccess = R2D_GetSurfaceAccess;
	sr->surface->ReleaseSurfaceAccess = R2D_ReleaseSurfaceAccess;

	sr->surface->DrawBitmap = R2D_DrawBitmap;
	sr->surface->SupportsFormat = R2D_SupportsFormat;
	sr->surface->render = sr;
	sr->surface->pixel_format = 0;
	ChainAddEntry(sr->surfaces_2D, sr->surface);

	sr->zoom = sr->scale_x = sr->scale_y = 1.0;
	vr->user_priv = sr;

	/*load options*/
	sOpt = IF_GetKey(compositor->client->config, "Render2D", "DirectRender");
	if (sOpt && ! stricmp(sOpt, "yes")) 
		sr->top_effect->trav_flags |= TF_RENDER_DIRECT;
	else
		sr->top_effect->trav_flags &= ~TF_RENDER_DIRECT;
	
	sOpt = IF_GetKey(compositor->client->config, "Render2D", "ScalableZoom");
	sr->scalable_zoom = (!sOpt || !stricmp(sOpt, "yes") ) ? 1 : 0;
	sOpt = IF_GetKey(compositor->client->config, "Render2D", "DisableYUV");
	sr->enable_yuv_hw = (sOpt && !stricmp(sOpt, "yes") ) ? 0 : 1;
	return M4OK;
}



void R2D_UnloadRenderer(VisualRenderer *vr)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	DeleteVisualSurface2D(sr->surface);
	DeleteChain(sr->sensors);
	DeleteChain(sr->surfaces_2D);
	DeleteChain(sr->strike_bank);
	effect_delete(sr->top_effect);
	free(sr);
	vr->user_priv = NULL;
}


M4Err R2D_AllocTexture(TextureHandler *hdl)
{
	if (hdl->hwtx) return M4BadParam;
	hdl->hwtx = hdl->compositor->g_hw->new_stencil(hdl->compositor->g_hw, M4StencilTexture);
	return M4OK;
}

void R2D_ReleaseTexture(TextureHandler *hdl)
{
	Render2D *sr = (Render2D *)hdl->compositor->visual_renderer->user_priv;
	if (hdl->hwtx) sr->compositor->g_hw->delete_stencil(hdl->hwtx);
	hdl->hwtx = NULL;
}

M4Err R2D_SetTextureData(TextureHandler *hdl)
{
	Render2D *sr = (Render2D *)hdl->compositor->visual_renderer->user_priv;
	return hdl->compositor->g_hw->stencil_set_texture(hdl->hwtx, hdl->data, hdl->width, hdl->height, hdl->stride, hdl->pixelformat, sr->surface->pixel_format, 0);
}

/*no plugins use HW for texturing for now*/
void R2D_TextureHWReset(TextureHandler *hdl)
{
	return;
}

void R2D_GraphicsReset(VisualRenderer *vr)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	sr->compositor->video_out->GetPixelFormat(sr->compositor->video_out, 0, &sr->surface->pixel_format);
	sr->compositor->video_out->DeleteSurface(sr->compositor->video_out, sr->pool_rgb);
	sr->compositor->video_out->DeleteSurface(sr->compositor->video_out, sr->pool_yuv);
	sr->pool_yuv = sr->pool_rgb = 0;
}

void R2D_ReloadConfig(VisualRenderer *vr)
{
	const char *sOpt;
	Render2D *sr = (Render2D *)vr->user_priv;

	SR_Lock(sr->compositor, 1);

	sOpt = PMI_GetOpt(vr, "Render2D", "DirectRender");

	if ((!sOpt || !stricmp(sOpt, "yes") ) ) {
		sr->top_effect->trav_flags |= TF_RENDER_DIRECT;
	} else {
		sr->top_effect->trav_flags &= ~TF_RENDER_DIRECT;
	}

	sOpt = PMI_GetOpt(vr, "Render2D", "ScalableZoom");
	sr->scalable_zoom = (!sOpt || !stricmp(sOpt, "yes") ) ? 1 : 0;
	sOpt = PMI_GetOpt(vr, "Render2D", "DisableYUV");
	sr->enable_yuv_hw = (sOpt && !stricmp(sOpt, "yes") ) ? 0 : 1;
	/*emulate size message to force AR recompute*/
	SR_SizeChanged(sr->compositor, sr->compositor->width, sr->compositor->height);
	sr->compositor->draw_next_frame = 1;
	SR_Lock(sr->compositor, 0);
}

M4Err R2D_SetOption(VisualRenderer *vr, u32 option, u32 value)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	switch (option) {
	case M4O_DirectRender:
		SR_Lock(sr->compositor, 1);
		if (value) {
			sr->top_effect->trav_flags |= TF_RENDER_DIRECT;
		} else {
			sr->top_effect->trav_flags &= ~TF_RENDER_DIRECT;
		}
		/*force redraw*/
		SR_Invalidate(sr->compositor, NULL);
		SR_Lock(sr->compositor, 0);
		return M4OK;
	case M4O_ScalableZoom:
		sr->scalable_zoom = value;
		/*emulate size message to force AR recompute*/
		SR_SizeChanged(sr->compositor, sr->compositor->width, sr->compositor->height);
		return M4OK;
	case M4O_YUVHardware:
		sr->enable_yuv_hw = value;
		if (!value) sr->current_yuv_format = 0;
		return M4OK;
	case M4O_ReloadConfig: R2D_ReloadConfig(vr); return M4OK;
	case M4O_OriginalView: 
		sr->trans_x = sr->trans_y = 0;
		R2D_SetZoom(sr, 1.0);
		return M4OK;
	case M4O_NavigationType: 
		sr->trans_x = sr->trans_y = 0;
		R2D_SetZoom(sr, 1.0);
		return M4OK;
	case M4O_NavigationMode:
		if ((value!=M4_NavNone) && (value!=M4_NavSlide)) return M4NotSupported;
		sr->navigate_mode = value;
		return M4OK;
	case M4O_Headlight: return M4NotSupported;
	case M4O_Collision: return M4NotSupported;
	case M4O_Gravity: return M4NotSupported;
	default: return M4BadParam;
	}
}

u32 R2D_GetOption(VisualRenderer *vr, u32 option)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	switch (option) {
	case M4O_ScalableZoom: return sr->scalable_zoom;
	case M4O_YUVHardware: return sr->enable_yuv_hw;
	case M4O_YUVFormat: return sr->enable_yuv_hw ? sr->current_yuv_format : 0;
	case M4O_NavigationType: return M4_Navigation2D;
	case M4O_NavigationMode: return sr->navigate_mode;
	case M4O_Headlight: return 0;
	case M4O_Collision: return M4_CollideNone;
	case M4O_Gravity: return 0;
	default: return 0;
	}
}

/*render inline scene*/
void R2D_RenderInline(VisualRenderer *vr, SFNode *inline_root, void *rs)
{
	Bool use_pm;
	u32 h, w;
	M4Matrix2D mx_bck, mx;
	LPSCENEGRAPH in_scene;
	RenderEffect2D *eff = (RenderEffect2D *)rs;

	in_scene = Node_GetParentGraph(inline_root);
	use_pm = SG_UsePixelMetrics(in_scene);
	if (use_pm == eff->is_pixel_metrics) {
		Node_Render(inline_root, rs);
		return;
	}
	mx2d_copy(mx_bck, eff->transform);
	/*override aspect ratio if any size info is given in the scene*/
	if (SG_GetSizeInfo(in_scene, &w, &h)) {
		Float scale = 0.5f * MIN(w, h);
		if (scale) eff->min_hsize = scale;
	}
	mx2d_init(mx);
	/*apply meterMetrics<->pixelMetrics scale*/
	if (!use_pm) {
		mx2d_add_scale(&mx, eff->min_hsize, eff->min_hsize);
	} else {
		Float inv_scale = 1 / eff->min_hsize;
		mx2d_add_scale(&mx, inv_scale, inv_scale);
	}
	eff->is_pixel_metrics = use_pm;
	mx2d_add_matrix(&eff->transform, &mx);
	Node_Render(inline_root, rs);
	eff->is_pixel_metrics = !use_pm;
	mx2d_copy(eff->transform, mx_bck);
}

M4Err R2D_GetScreenBuffer(VisualRenderer *vr, M4VideoSurface *framebuffer)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	return sr->compositor->video_out->LockSurface(sr->compositor->video_out, 0, framebuffer);
}

M4Err R2D_ReleaseScreenBuffer(VisualRenderer *vr, M4VideoSurface *framebuffer)
{
	Render2D *sr = (Render2D *)vr->user_priv;
	return sr->compositor->video_out->UnlockSurface(sr->compositor->video_out, 0);
}


M4Err R2D_GetViewport(VisualRenderer *vr, u32 viewpoint_idx, const char **outName, Bool *is_bound);
M4Err R2D_SetViewport(VisualRenderer *vr, u32 viewpoint_idx, const char *viewpoint_name);

/*interface create*/
void *LoadInterface(u32 InterfaceType)
{
	VisualRenderer *sr;
	if (InterfaceType != M4_RENDERER_INTERFACE) return NULL;
	
	sr = malloc(sizeof(VisualRenderer));
	if (!sr) return NULL;
	memset(sr, 0, sizeof(VisualRenderer));
	M4_REG_PLUG(sr, M4_RENDERER_INTERFACE, "GPAC 2D Renderer", "gpac distribution", 0);

	sr->LoadRenderer = R2D_LoadRenderer;
	sr->UnloadRenderer = R2D_UnloadRenderer;
	sr->GraphicsReset = R2D_GraphicsReset;
	sr->NodeChanged = R2D_NodeChanged;
	sr->NodeInit = R2D_NodeInit;
	sr->DrawScene = R2D_DrawScene;
	sr->ExecuteEvent = R2D_ExecuteEvent;
	sr->RecomputeAR = R2D_RecomputeAR;
	sr->SceneReset = R2D_SceneReset;
	sr->RenderInline = R2D_RenderInline;
	sr->AllocTexture = R2D_AllocTexture;
	sr->ReleaseTexture = R2D_ReleaseTexture;
	sr->SetTextureData = R2D_SetTextureData;
	sr->TextureHWReset = R2D_TextureHWReset;
	sr->SetOption = R2D_SetOption;
	sr->GetOption = R2D_GetOption;
	sr->GetScreenBuffer = R2D_GetScreenBuffer;
	sr->ReleaseScreenBuffer = R2D_ReleaseScreenBuffer;
	sr->GetViewpoint = R2D_GetViewport;
	sr->SetViewpoint = R2D_SetViewport;

	sr->user_priv = NULL;
	return sr;
}


/*interface destroy*/
void ShutdownInterface(void *ifce)
{
	VisualRenderer *rend = (VisualRenderer *)ifce;
	if (rend->InterfaceType != M4_RENDERER_INTERFACE) return;
	assert(rend->user_priv==NULL);
	free(rend);
}

/*interface query*/
Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_RENDERER_INTERFACE) return 1;
	return 0;
}
