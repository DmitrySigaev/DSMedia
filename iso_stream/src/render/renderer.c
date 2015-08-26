/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
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

#include <intern/m4_render_dev.h>
#include <m4_user.h>

void SR_SimulationTick(LPSCENERENDER sr);


M4Err SR_SetOutputSize(LPSCENERENDER sr, u32 Width, u32 Height)
{
	M4Err e;

	SR_Lock(sr, 1);
	/*FIXME: need to check for max resolution*/
	sr->width = Width;
	sr->height = Height;
	e = sr->visual_renderer->RecomputeAR(sr->visual_renderer);
	sr->draw_next_frame = 1;
	SR_Lock(sr, 0);
	return e;
}

void SR_SetFullScreen(LPSCENERENDER sr)
{
	M4Err e;

	/*move to FS*/
	sr->fullscreen = !sr->fullscreen;
	e = sr->video_out->SetFullScreen(sr->video_out, sr->fullscreen, &sr->width, &sr->height);
	if (e) {
		M4USER_MESSAGE(sr->client, "VideoRenderer", "Cannot switch to fullscreen", e);
		sr->fullscreen = 0;
		sr->video_out->SetFullScreen(sr->video_out, 0, &sr->width, &sr->height);
	}
	e = sr->visual_renderer->RecomputeAR(sr->visual_renderer);
	/*force signaling graphics reset*/
	sr->reset_graphics = 1;
}

/*this is needed for:
- audio: since the audio renderer may not be threaded, it must be reconfigured by another thread otherwise 
we lock the audio plugin
- video: this is typical to OpenGL&co: multithreaded is forbidden, so resizing/fullscreen MUST be done by the same 
thread accessing the HW ressources
*/
static void SR_ReconfigTask(LPSCENERENDER sr)
{
	u32 width,height;
	
	/*reconfig audio if needed*/
	if (sr->audio_renderer) AR_Reconfig(sr->audio_renderer);

	/*resize visual if needed*/
	switch (sr->msg_type) {
	/*scene size has been overriden*/
	case 1:
		assert(!(sr->override_size_flags & 2));
		sr->override_size_flags |= 2;
		width = sr->scene_width;
		height = sr->scene_height;
		sr->has_size_info = 1;
		SR_SetSize(sr, width, height);
		M4USER_SETSIZE(sr->client, width, height);
		break;
	/*fullscreen on/off request*/
	case 2:
		SR_SetFullScreen(sr);
		sr->draw_next_frame = 1;
		break;
	/*aspect ratio modif*/
	case 3:
		sr->visual_renderer->RecomputeAR(sr->visual_renderer);
		break;
	/*size changed*/
	case 4:
		SR_SetOutputSize(sr, sr->new_width, sr->new_height);
		sr->new_width = sr->new_height = 0;
		break;
	/*size changed from scene cfg: resize window first*/
	case 5:
		{
			M4Event evt;
			/*send resize event*/
			evt.type = M4E_NEEDRESIZE;
			evt.size.width = sr->new_width;
			evt.size.height = sr->new_height;
			sr->video_out->PushEvent(sr->video_out, &evt);
			SR_SetOutputSize(sr, sr->new_width, sr->new_height);
			sr->new_width = sr->new_height = 0;
		}
		break;
	}
	/*3D driver changed message, recheck extensions*/
	if (sr->reset_graphics) sr->visual_renderer->GraphicsReset(sr->visual_renderer);
	sr->msg_type = 0;
}

M4Err SR_RenderFrame(LPSCENERENDER sr)
{	
	SR_ReconfigTask(sr);
	/*render*/
	SR_SimulationTick(sr);
	return M4OK;
}

u32 SR_RenderRun(void *par)
{	
	LPSCENERENDER sr = (LPSCENERENDER) par;
	sr->video_th_state = 1;

	while (sr->video_th_state == 1) {
		/*sleep or render*/
		if (sr->is_hidden) 
			Sleep(sr->frame_duration);
		else
			SR_SimulationTick(sr);
	}

	/*destroy video out*/
	sr->video_out->Shutdown(sr->video_out);
	PM_ShutdownInterface(sr->video_out);
	sr->video_out = NULL;

	sr->video_th_state = 3;
	return 0;
}

/*forces graphics redraw*/
void SR_ResetGraphics(LPSCENERENDER sr)
{	
	if (sr) {
		SR_Lock(sr, 1);
		sr->reset_graphics = 1;
		SR_Lock(sr, 0);
	}
}

void SR_ResetFrameRate(SceneRenderer *sr)
{
	u32 i;
	for (i=0; i<FPS_COMPUTE_SIZE; i++) sr->frame_time[i] = 0;
	sr->current_frame = 0;
}

void SR_SetFontEngine(SceneRenderer *sr);
void SR_OnEvent(void *cbck, M4Event *event);

static Bool check_graphics2D_driver(Graphics2DDriver *ifce)
{
	/*check base*/
	if (!ifce->new_stencil || !ifce->new_surface) return 0;
	/*if these are not set we cannot draw*/
	if (!ifce->surface_clear || !ifce->surface_set_path || !ifce->surface_fill) return 0;
	/*check we can init a surface with the current driver (the rest is optional)*/
	if (ifce->attach_surface_to_buffer) return 1;
	return 0;
}


static LPSCENERENDER SR_New(M4User *client)
{
	char *sOpt;
	M4GLConfig cfg, *gl_cfg;
	LPSCENERENDER tmp = (LPSCENERENDER) malloc(sizeof(SceneRenderer));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(SceneRenderer));
	tmp->client = client;

	/*load renderer to check for GL flag*/
	sOpt = IF_GetKey(client->config, "Rendering", "RendererName");
	if (sOpt) {
		if (!PM_LoadInterfaceByName(client->plugins, sOpt, M4_RENDERER_INTERFACE, (void **) &tmp->visual_renderer) ) {
			tmp->visual_renderer = NULL;
			sOpt = NULL;
		}
	}
	if (!tmp->visual_renderer) {
		u32 i, count;
		count = PM_GetPluginsCount(client->plugins);
		for (i=0; i<count; i++) {
			if (PM_LoadInterface(client->plugins, i, M4_RENDERER_INTERFACE, (void **) &tmp->visual_renderer)) break;
			tmp->visual_renderer = NULL;
		}
		if (tmp->visual_renderer) IF_SetKey(client->config, "Rendering", "RendererName", tmp->visual_renderer->plugin_name);
	}

	if (!tmp->visual_renderer) {
		free(tmp);
		return NULL;
	}

	memset(&cfg, 0, sizeof(cfg));
	cfg.double_buffered = 1;
	gl_cfg = tmp->visual_renderer->bNeedsGL ? &cfg : NULL;

	/*load video out*/
	sOpt = IF_GetKey(client->config, "Video", "DriverName");
	if (sOpt) {
		if (!PM_LoadInterfaceByName(client->plugins, sOpt, M4_VIDEO_OUTPUT_INTERFACE, (void **) &tmp->video_out)) {
			tmp->video_out = NULL;
			sOpt = NULL;
		} else {
			tmp->video_out->evt_cbk_hdl = tmp;
			tmp->video_out->on_event = SR_OnEvent;
			/*init hw*/
			if (tmp->video_out->SetupHardware(tmp->video_out, client->os_window_handler, client->dont_override_window_proc, gl_cfg) != M4OK) {
				PM_ShutdownInterface(tmp->video_out);
				tmp->video_out = NULL;
			}
		}
	}

	if (!tmp->video_out) {
		u32 i, count;
		count = PM_GetPluginsCount(client->plugins);
		for (i=0; i<count; i++) {
			if (!PM_LoadInterface(client->plugins, i, M4_VIDEO_OUTPUT_INTERFACE, (void **) &tmp->video_out)) continue;
			tmp->video_out->evt_cbk_hdl = tmp;
			tmp->video_out->on_event = SR_OnEvent;
			/*init hw*/
			if (tmp->video_out->SetupHardware(tmp->video_out, client->os_window_handler, client->dont_override_window_proc, gl_cfg)==M4OK) {
				IF_SetKey(client->config, "Video", "DriverName", tmp->video_out->plugin_name);
				break;
			}
			PM_ShutdownInterface(tmp->video_out);
			tmp->video_out = NULL;
		}
	}

	if (!tmp->video_out ) {
		PM_ShutdownInterface(tmp->visual_renderer);
		free(tmp);
		return NULL;
	}

	/*try to load a graphics driver */
	sOpt = IF_GetKey(client->config, "Rendering", "GraphicsDriver");
	if (sOpt) {
		if (!PM_LoadInterfaceByName(client->plugins, sOpt, M4_GRAPHICS_2D_INTERFACE, (void **) &tmp->g_hw)) {
			tmp->g_hw = NULL;
			sOpt = NULL;
		} else if (!check_graphics2D_driver(tmp->g_hw)) {
			PM_ShutdownInterface(tmp->g_hw);
			tmp->g_hw = NULL;
			sOpt = NULL;
		}
	}
	if (!tmp->g_hw) {
		u32 i, count;
		count = PM_GetPluginsCount(client->plugins);
		for (i=0; i<count; i++) {
			if (!PM_LoadInterface(client->plugins, i, M4_GRAPHICS_2D_INTERFACE, (void **) &tmp->g_hw)) continue;
			if (check_graphics2D_driver(tmp->g_hw)) break;
			PM_ShutdownInterface(tmp->g_hw);
			tmp->g_hw = NULL;
		}
		if (tmp->g_hw) IF_SetKey(client->config, "Rendering", "GraphicsDriver", tmp->g_hw->plugin_name);
	}

	/*and init*/
	if (tmp->visual_renderer->LoadRenderer(tmp->visual_renderer, tmp) != M4OK) {
		PM_ShutdownInterface(tmp->visual_renderer);
		tmp->video_out->Shutdown(tmp->video_out);
		PM_ShutdownInterface(tmp->video_out);
		if (tmp->g_hw) PM_ShutdownInterface(tmp->g_hw);
		free(tmp);
		return NULL;
	}

	tmp->mx = NewMutex();
	tmp->textures = NewChain();
	tmp->frame_rate = 30.0;	
	tmp->frame_duration = 33;
	tmp->time_nodes = NewChain();
	tmp->events = NewChain();
	tmp->ev_mx = NewMutex();
	
	SR_ResetFrameRate(tmp);	
	/*set font engine if any*/
	SR_SetFontEngine(tmp);
	
	tmp->extra_scenes = NewChain();
	tmp->base_curve_resolution = DEFAULT_CURVE_RESOLUTION;
	tmp->interaction_level = M4_InteractNormal | M4_InteractInputSensor | M4_InteractWM;
	return tmp;
}

LPSCENERENDER NewSceneRender(M4User *client, Bool self_threaded, Bool no_audio, MPEG4CLIENT term)
{
	LPSCENERENDER tmp = SR_New(client);
	if (!tmp) return NULL;
	tmp->term = term;

	if (!no_audio) tmp->audio_renderer = LoadAudioRenderer(client);	

	MX_P(tmp->mx);

	/*run threaded*/
	if (self_threaded) {
		tmp->VisualThread = NewThread();
		TH_Run(tmp->VisualThread, SR_RenderRun, tmp);
		while (tmp->video_th_state!=1) {
			Sleep(10);
			if (tmp->video_th_state==3) {
				MX_V(tmp->mx);
				SR_Delete(tmp);
				return NULL;
			}
		}
	}

	/*set default size if owning output*/
	if (!tmp->client->os_window_handler) {
		SR_SetSize(tmp, 320, 20);
	} else {
		M4USER_SETSIZE(client, 320, 20);
	}
	MX_V(tmp->mx);

	return tmp;
}


void SR_Delete(LPSCENERENDER sr)
{
	if (!sr) return;

	SR_Lock(sr, 1);
	if (sr->VisualThread) {
		sr->video_th_state = 2;
		while (sr->video_th_state!=3) Sleep(10);
		TH_Delete(sr->VisualThread);
	}

	if (sr->video_out) {
		sr->video_out->Shutdown(sr->video_out);
		PM_ShutdownInterface(sr->video_out);
	}
	sr->visual_renderer->UnloadRenderer(sr->visual_renderer);
	PM_ShutdownInterface(sr->visual_renderer);

	if (sr->audio_renderer) AR_Delete(sr->audio_renderer);

	if (sr->g_hw) PM_ShutdownInterface(sr->g_hw);

	MX_P(sr->ev_mx);
	while (ChainGetCount(sr->events)) {
		M4UserEvent *ev = ChainGetEntry(sr->events, 0);
		ChainDeleteEntry(sr->events, 0);
		free(ev);
	}
	MX_V(sr->ev_mx);
	MX_Delete(sr->ev_mx);
	DeleteChain(sr->events);

	if (sr->font_engine) {
		sr->font_engine->shutdown_font_engine(sr->font_engine);
		PM_ShutdownInterface(sr->font_engine);
	}
	DeleteChain(sr->textures);
	DeleteChain(sr->time_nodes);
	DeleteChain(sr->extra_scenes);

	SR_Lock(sr, 0);
	MX_Delete(sr->mx);
	free(sr);
}

void SR_SetFrameRate(LPSCENERENDER sr, Double fps)
{
	if (fps) {
		sr->frame_rate = fps;
		sr->frame_duration = (u32) (1000 / fps);
		SR_ResetFrameRate(sr);	
	}
}

u32 SR_GetAudioBufferLength(LPSCENERENDER sr)
{
	if (!sr || !sr->audio_renderer || !sr->audio_renderer->audio_out) return 0;
	return sr->audio_renderer->audio_out->GetTotalBufferTime(sr->audio_renderer->audio_out);
}


void SR_Pause(LPSCENERENDER sr, Bool DoPause)
{
	if (!sr || !sr->audio_renderer) return;
	/*step mode*/
	if (DoPause==2) {
		sr->step_mode = 1;
		/*resume for next step*/
		if (sr->paused && sr->term) M4T_Pause(sr->term, 0);
	} else if (sr->paused != DoPause) {
		if (sr->audio_renderer) AR_Pause(sr->audio_renderer, DoPause, sr->step_mode);
		sr->paused = DoPause;
	}
}

u32 SR_GetTime(LPSCENERENDER sr)
{
	return AR_GetTime(sr->audio_renderer);
}

static M4Err SR_SetSceneSize(LPSCENERENDER sr, u32 Width, u32 Height)
{
	if (!Width || !Height) {
		sr->has_size_info = 0;
		if (sr->override_size_flags) {
			/*specify a small size to detect biggest bitmap but not 0 in case only audio..*/
			sr->scene_height = 20;
			sr->scene_width = 320;
		} else {
			sr->scene_height = 240;
			sr->scene_width = 320;
		}
	} else {
		sr->has_size_info = 1;
		sr->scene_height = Height;
		sr->scene_width = Width;
	}
	return M4OK;
}


M4Err SR_SetSceneGraph(LPSCENERENDER sr, LPSCENEGRAPH scene_graph)
{
	u32 width, height;

	if (!sr) return M4BadParam;

	SR_Lock(sr, 1);

	if (sr->audio_renderer && (sr->scene != scene_graph)) 
		AR_ResetSources(sr->audio_renderer);	

	MX_P(sr->ev_mx);
	while (ChainGetCount(sr->events)) {
		M4UserEvent *ev = ChainGetEntry(sr->events, 0);
		ChainDeleteEntry(sr->events, 0);
		free(ev);
	}
	
	/*reset main surface*/
	sr->visual_renderer->SceneReset(sr->visual_renderer);

	/*set current graph*/
	sr->scene = scene_graph;
	
	if (scene_graph) {
		/*get pixel size if any*/
		SG_GetSizeInfo(sr->scene, &width, &height);
		/*set scene size only if different, otherwise keep scaling/FS*/
		if ( !width || (sr->scene_width!=width) || !height || (sr->scene_height!=height)) {
			SR_SetSceneSize(sr, width, height);
			/*get actual size in pixels*/
			width = sr->scene_width;
			height = sr->scene_height;

			/*only notify user if we are attached to a window*/
			if (sr->client->os_window_handler) {
				/*security in case the user doesn't get the message (this happens on w32, resize message
				with same values are discarded)*/
				SR_SizeChanged(sr, width, height);
				M4USER_SETSIZE(sr->client, width, height);
				sr->override_size_flags &= ~2;
			} else {
				/*signal size changed*/
				SR_SetSize(sr,width, height);
			}
		}
		sr->has_size_info = (width && height) ? 1 : 0;
	}

	SR_ResetFrameRate(sr);	
	sr->draw_next_frame = 1;
	MX_V(sr->ev_mx);
	SR_Lock(sr, 0);
	return M4OK;
}

void SR_Lock(LPSCENERENDER sr, Bool doLock)
{
	if (doLock)
		MX_P(sr->mx);
	else {
		MX_V(sr->mx);
	}
}

void SR_RefreshWindow(LPSCENERENDER sr)
{
	if (sr) sr->draw_next_frame = 1;
}

M4Err SR_SizeChanged(LPSCENERENDER sr, u32 NewWidth, u32 NewHeight)
{
	if (!NewWidth || !NewHeight) {
		sr->override_size_flags &= ~2;
		return M4OK;
	}
	SR_Lock(sr, 1);
	sr->new_width = NewWidth;
	sr->new_height = NewHeight;
	sr->msg_type = 4;
	SR_Lock(sr, 0);
	return M4OK;
}

M4Err SR_SetSize(LPSCENERENDER sr, u32 NewWidth, u32 NewHeight)
{
	if (!NewWidth || !NewHeight) {
		sr->override_size_flags &= ~2;
		return M4OK;
	}
	/*the safe way wouldhave the scene locked, but it hangs with win32 osmo4 & not fixed yet*/
//	SR_Lock(sr, 1);
	sr->new_width = NewWidth;
	sr->new_height = NewHeight;
	sr->msg_type = 5;
//	SR_Lock(sr, 0);
	/*force video resize*/
	if (!sr->VisualThread) SR_ReconfigTask(sr);
	return M4OK;
}

void SR_ReloadConfig(LPSCENERENDER sr)
{
	const char *sOpt, *dr_name;

	/*changing drivers needs exclusive access*/
	SR_Lock(sr, 1);
	
	sOpt = IF_GetKey(sr->client->config, "Rendering", "ForceSceneSize");
	if (sOpt && ! stricmp(sOpt, "yes")) {
		sr->override_size_flags = 1;
	} else {
		sr->override_size_flags = 0;
	}

	sOpt = IF_GetKey(sr->client->config, "Rendering", "AntiAlias");
	if (sOpt)
	{
		if (! stricmp(sOpt, "None"))
			SR_SetOption(sr, M4O_Antialias, M4_AL_None);
		else 
			if (! stricmp(sOpt, "Text"))
				SR_SetOption(sr, M4O_Antialias, M4_AL_Text);
			else 
				SR_SetOption(sr, M4O_Antialias, M4_AL_All);
	}
	else
	{
		IF_SetKey(sr->client->config, "Rendering", "AntiAlias", "All");
		SR_SetOption(sr, M4O_Antialias, M4_AL_All);
	}

	sOpt = IF_GetKey(sr->client->config, "Rendering", "StressMode");
	SR_SetOption(sr, M4O_StressMode, (sOpt && !stricmp(sOpt, "yes") ) ? 1 : 0);

	sOpt = IF_GetKey(sr->client->config, "Rendering", "FastRender");
	SR_SetOption(sr, M4O_HighSpeed, (sOpt && !stricmp(sOpt, "yes") ) ? 1 : 0);

	sOpt = IF_GetKey(sr->client->config, "Rendering", "BoundingVolume");
	if (sOpt) {
		if (! stricmp(sOpt, "Box")) SR_SetOption(sr, M4O_BoundingVolume, M4_Bounds_Box);
		else if (! stricmp(sOpt, "AABB")) SR_SetOption(sr, M4O_BoundingVolume, M4_Bounds_AABB);
		else SR_SetOption(sr, M4O_BoundingVolume, M4_Bounds_None);
	} else {
		IF_SetKey(sr->client->config, "Rendering", "BoundingVolume", "None");
		SR_SetOption(sr, M4O_BoundingVolume, M4_Bounds_None);
	}

	sOpt = IF_GetKey(sr->client->config, "FontEngine", "DriverName");
	if (sOpt && sr->font_engine) {
		dr_name = sr->font_engine->plugin_name;
		if (stricmp(dr_name, sOpt)) SR_SetFontEngine(sr);
	}

	sOpt = IF_GetKey(sr->client->config, "FontEngine", "UseTextureText");
	sr->use_texture_text = (sOpt && ! stricmp(sOpt, "yes")) ? 1 : 0;

	sOpt = IF_GetKey(sr->client->config, "Rendering", "DisableZoomPan");
	if (sOpt && !stricmp(sOpt, "yes")) 
		sr->interaction_level &= ~M4_InteractZoomPan;
	else
		sr->interaction_level |= M4_InteractZoomPan;

	sr->draw_next_frame = 1;

	SR_Lock(sr, 0);
}

M4Err SR_SetOption(LPSCENERENDER sr, u32 type, u32 value)
{
	M4Err e;
	SR_Lock(sr, 1);

	e = M4OK;
	switch (type) {
	case M4O_AudioVolume: AR_SetVolume(sr->audio_renderer, value); break;
	case M4O_AudioPan: AR_SetPan(sr->audio_renderer, value); break;
	case M4O_OverrideSize:
		sr->override_size_flags = value ? 1 : 0;
		sr->draw_next_frame = 1;
		break;
	case M4O_StressMode: sr->stress_mode = value; break;
	case M4O_Antialias: sr->antiAlias = value; break;
	case M4O_HighSpeed: sr->high_speed = value; break;
	case M4O_BoundingVolume: sr->draw_bvol = value; break;
	case M4O_UseTextureText: sr->use_texture_text = value; break;
	case M4O_AspectRatio: 
		sr->aspect_ratio = value; 
		sr->msg_type = 3;
		break;
	case M4O_InteractLevel: sr->interaction_level = value; break;
	case M4O_ForceRedraw:
		sr->reset_graphics = 1;
		break;
	case M4O_Fullscreen:
		if (sr->fullscreen != value) sr->msg_type = 2; break;
	case M4O_OriginalView:
		e = sr->visual_renderer->SetOption(sr->visual_renderer, type, value);
		SR_SetSize(sr, sr->scene_width, sr->scene_height);
		break;
	case M4O_Visible:
		sr->is_hidden = !value;
		if (sr->video_out->PushEvent) {
			M4Event evt;
			evt.type = M4E_SHOWHIDE;
			evt.show.is_visible = value;
			e = sr->video_out->PushEvent(sr->video_out, &evt);
		}
		break;
	case M4O_FreezeDisplay: 
		sr->freeze_display = value;
		break;
	case M4O_ReloadConfig: 
		SR_ReloadConfig(sr);
	default: e = sr->visual_renderer->SetOption(sr->visual_renderer, type, value);
	}
	sr->draw_next_frame = 1; 
	SR_Lock(sr, 0);
	return e;
}

u32 SR_GetOption(LPSCENERENDER sr, u32 type)
{
	switch (type) {
	case M4O_OverrideSize: return (sr->override_size_flags & 1) ? 1 : 0;
	case M4O_IsOver:
	{
		if (sr->interaction_sensors) return 0;
		if (ChainGetCount(sr->time_nodes)) return 0;
		return 1;
	}
	case M4O_StressMode: return sr->stress_mode;
	case M4O_Antialias: return sr->antiAlias;
	case M4O_HighSpeed: return sr->high_speed;
	case M4O_AspectRatio: return sr->aspect_ratio;
	case M4O_Fullscreen: return sr->fullscreen;
	case M4O_InteractLevel: return sr->interaction_level;
	case M4O_Visible: return !sr->is_hidden;
	case M4O_FreezeDisplay: return sr->freeze_display;
	case M4O_UseTextureText: return sr->use_texture_text;
	default: return sr->visual_renderer->GetOption(sr->visual_renderer, type);
	}
}

void SR_MapCoordinates(LPSCENERENDER sr, s32 X, s32 Y, Float *bifsX, Float *bifsY)
{
	*bifsX = (Float) X;
	*bifsY = (Float) Y;
}


void SR_SetFontEngine(SceneRenderer *sr)
{
	const char *sOpt;
	u32 i, count;
	FontRaster *ifce;

	ifce = NULL;
	sOpt = IF_GetKey(sr->client->config, "FontEngine", "DriverName");
	if (sOpt) {
		if (!PM_LoadInterfaceByName(sr->client->plugins, sOpt, M4_FONT_RASTER_INTERFACE, (void **) &ifce)) 
			ifce = NULL;
	}

	if (!ifce) {
		count = PM_GetPluginsCount(sr->client->plugins);
		for (i=0; i<count; i++) {
			if (PM_LoadInterface(sr->client->plugins, i, M4_FONT_RASTER_INTERFACE, (void **) &ifce)) {
				IF_SetKey(sr->client->config, "FontEngine", "DriverName", ifce->plugin_name);
				sOpt = ifce->plugin_name;
				break;
			}
		}
	}
	if (!ifce) return;

	/*cannot init font engine*/
	if (ifce->init_font_engine(ifce) != M4OK) {
		PM_ShutdownInterface(ifce);
		return;
	}


	/*shutdown current*/
	SR_Lock(sr, 1);
	if (sr->font_engine) {
		sr->font_engine->shutdown_font_engine(sr->font_engine);
		PM_ShutdownInterface(sr->font_engine);
	}
	sr->font_engine = ifce;

	/*success*/
	IF_SetKey(sr->client->config, "FontEngine", "DriverName", sOpt);
		
	sr->draw_next_frame = 1;
	SR_Lock(sr, 0);
}

M4Err SR_GetScreenBuffer(LPSCENERENDER sr, M4VideoSurface *framebuffer)
{
	M4Err e;
	if (!sr || !framebuffer) return M4BadParam;
	MX_P(sr->mx);
	e = sr->visual_renderer->GetScreenBuffer(sr->visual_renderer, framebuffer);
	if (e != M4OK) MX_V(sr->mx);
	return e;
}

M4Err SR_ReleaseScreenBuffer(LPSCENERENDER sr, M4VideoSurface *framebuffer)
{
	M4Err e;
	if (!sr || !framebuffer) return M4BadParam;
	e = sr->visual_renderer->ReleaseScreenBuffer(sr->visual_renderer, framebuffer);
	MX_V(sr->mx);
	return e;
}

Float SR_GetCurrentFPS(LPSCENERENDER sr, Bool absoluteFPS)
{
	Float fps;
	u32 ind, num, frames, run_time;

	/*start from last frame and get first frame time*/
	ind = sr->current_frame;
	frames = 0;
	run_time = sr->frame_time[ind];
	for (num=0; num<FPS_COMPUTE_SIZE; num++) {
		if (absoluteFPS) {
			run_time += sr->frame_time[ind];
		} else {
			run_time += MAX(sr->frame_time[ind], sr->frame_duration);
		}
		frames++;
		if (frames==FPS_COMPUTE_SIZE) break;
		if (!ind) {
			ind = FPS_COMPUTE_SIZE;
		} else {
			ind--;
		}
	}
	if (!run_time) return (Float) sr->frame_rate;
	fps = (Float) (1000*frames);
	fps /= (Float) (run_time);
	return fps;
}

void SR_RegisterTimeNode(LPSCENERENDER sr, TimeNode *tn)
{
	/*may happen with DEF/USE */
	if (tn->is_registered) return;
	if (tn->needs_unregister) return;
	ChainAddEntry(sr->time_nodes, tn);
	tn->is_registered = 1;
}
void SR_UnregisterTimeNode(LPSCENERENDER sr, TimeNode *tn)
{
	ChainDeleteItem(sr->time_nodes, tn);
}


void SR_UserInput(LPSCENERENDER sr, M4Event *event)
{
	M4UserEvent *ev;

	if (sr->term && (sr->interaction_level & M4_InteractInputSensor) && (event->type!=M4E_VKEYDOWN) && (event->type!=M4E_VKEYUP))
		M4T_MouseInput(sr->term, &event->mouse);

	if (!(sr->interaction_level & M4_InteractNormal)) return;

	switch (event->type) {
	case M4E_MOUSEMOVE:
	{
		u32 i;
		MX_P(sr->ev_mx);
		for (i=0; i<ChainGetCount(sr->events); i++) {
			ev = ChainGetEntry(sr->events, i);
			if (ev->event_type == M4E_MOUSEMOVE) {
				ev->mouse =  event->mouse;
				MX_V(sr->ev_mx);
				return;
			}
		}
		MX_V(sr->ev_mx);
	}
	default:
		ev = malloc(sizeof(M4UserEvent));
		ev->event_type = event->type;
		if (event->type<=M4E_MOUSEWHEEL) {
			ev->mouse = event->mouse;
		} else {
			ev->key = event->key;
		}
		MX_P(sr->ev_mx);
		ChainAddEntry(sr->events, ev);
		MX_V(sr->ev_mx);
		break;
	}
}

SFNode *SR_PickNode(LPSCENERENDER sr, s32 X, s32 Y)
{
	return NULL;
}

static u32 last_lclick_time = 0;

static void SR_DoShortcuts(LPSCENERENDER sr, M4UserEvent *ev)
{
	u32 now;
	switch (ev->event_type) {
	case M4E_LEFTUP:
		now = M4_GetSysClock();
		if (now - last_lclick_time < 250) SR_SetOption(sr, M4O_Fullscreen, !sr->fullscreen);
		last_lclick_time = now;
		break;
	case M4E_VKEYDOWN:
		switch (ev->key.m4_vk_code) {
		case M4VK_HOME:
			sr->visual_renderer->SetOption(sr->visual_renderer, M4O_NavigationType, 1);
			break;
		default:
			if ((ev->key.m4_vk_code == M4VK_ESCAPE) || ((sr->key_states & M4KM_ALT) && (ev->key.m4_vk_code == M4VK_RETURN))) {
				SR_SetOption(sr, M4O_Fullscreen, !sr->fullscreen);
			}
			break;
		}
		break;
	default:
		break;
	}
}

void SR_SimulationTick(LPSCENERENDER sr)
{	
	u32 in_time, end_time, i, count;

	if (sr->freeze_display) {
		Sleep(sr->frame_duration);
		return;
	}

	SR_Lock(sr, 1);
	
	SR_ReconfigTask(sr);

	if (!sr->scene) {
		sr->visual_renderer->DrawScene(sr->visual_renderer);
		SR_Lock(sr, 0);
		Sleep(sr->frame_duration);
		return;
	}

	in_time = M4_GetSysClock();
	if (sr->reset_graphics) sr->draw_next_frame = 1;

	/*process pending user events*/
	MX_P(sr->ev_mx);
	while (ChainGetCount(sr->events)) {
		M4UserEvent *ev = ChainGetEntry(sr->events, 0);
		ChainDeleteEntry(sr->events, 0);
		if (!sr->visual_renderer->ExecuteEvent(sr->visual_renderer, ev)) {
			SR_DoShortcuts(sr, ev);
		}
		free(ev);
	}
	MX_V(sr->ev_mx);

	/*execute all routes before updating textures, otherwise nodes inside composite texture may never see their
	dirty flag set*/
	SG_ActivateRoutes(sr->scene);

	/*update all textures*/
	count = ChainGetCount(sr->textures);
	for (i=0; i<count; i++) {
		TextureHandler *st = ChainGetEntry(sr->textures, i);
		/*signal graphics reset before updating*/
		if (sr->reset_graphics && st->hwtx) sr->visual_renderer->TextureHWReset(st);
		st->update_texture_fcnt(st);
	}

	/*if invalidated, draw*/
	if (sr->draw_next_frame) {
		sr->draw_next_frame = 0;
		sr->visual_renderer->DrawScene(sr->visual_renderer);
		sr->reset_graphics = 0;

		if (sr->stress_mode) {
			sr->draw_next_frame = 1;
			sr->reset_graphics = 1;
		}
	}

	/*update all timed nodes*/
	for (i=0; i<ChainGetCount(sr->time_nodes); i++) {
		TimeNode *tn = ChainGetEntry(sr->time_nodes, i);
		if (!tn->needs_unregister) tn->UpdateTimeNode(tn);
		if (tn->needs_unregister) {
			tn->is_registered = 0;
			tn->needs_unregister = 0;
			ChainDeleteEntry(sr->time_nodes, i);
			i--;
			continue;
		}
	}

	/*release all textures - we must release them to handle a same OD being used by several textures*/
	count = ChainGetCount(sr->textures);
	for (i=0; i<count; i++) {
		TextureHandler *st = ChainGetEntry(sr->textures, i);
		texture_release_stream(st);
	}
	end_time = M4_GetSysClock() - in_time;

	SR_Lock(sr, 0);

	sr->current_frame = (sr->current_frame+1) % FPS_COMPUTE_SIZE;
	sr->frame_time[sr->current_frame] = end_time;

	/*step mode on, pause and return*/
	if (sr->step_mode) {
		sr->step_mode = 0;
		if (sr->term) M4T_Pause(sr->term, 1);
		return;
	}
	/*not threaded, let the owner decide*/
	if (!sr->VisualThread) return;


	/*compute sleep time till next frame, otherwise we'll kill the CPU*/
	i=1;
	while (i * sr->frame_duration < end_time) i++;
	in_time = i * sr->frame_duration - end_time;
	Sleep(in_time);
}

M4Err SR_GetViewpoint(LPSCENERENDER sr, u32 viewpoint_idx, const char **outName, Bool *is_bound)
{
	if (!sr->visual_renderer->GetViewpoint) return M4NotSupported;
	return sr->visual_renderer->GetViewpoint(sr->visual_renderer, viewpoint_idx, outName, is_bound);
}
M4Err SR_SetViewpoint(LPSCENERENDER sr, u32 viewpoint_idx, const char *viewpoint_name)
{
	if (!sr->visual_renderer->SetViewpoint) return M4NotSupported;
	return sr->visual_renderer->SetViewpoint(sr->visual_renderer, viewpoint_idx, viewpoint_name);
}

void SR_RenderInline(LPSCENERENDER sr, SFNode *inline_root, void *rs)
{
	if (sr->visual_renderer->RenderInline) sr->visual_renderer->RenderInline(sr->visual_renderer, inline_root, rs);
}


#define M4SETCURSOR(t) { M4Event evt; evt.type = M4E_SET_CURSOR; evt.cursor.cursor_type = (t); sr->video_out->PushEvent(sr->video_out, &evt); }

void SR_OnEvent(void *cbck, M4Event *event)
{
	u32 s, c, m;
	SceneRenderer *sr = (SceneRenderer *)cbck;
	/*not assigned yet*/
	if (!sr || !sr->video_out) return;

	switch (event->type) {
	case M4E_REFRESH:
		SR_RefreshWindow(sr);
		break;
	case M4E_GL_CHANGED:
		SR_ResetGraphics(sr);
		break;
	case M4E_WINDOWSIZE:
		SR_SizeChanged(sr, event->size.width, event->size.height);
		break;
	case M4E_NEEDRESIZE:
		SR_SetSize(sr, event->size.width, event->size.height);
		break;
	case M4E_VKEYDOWN:
		s = c = m = 0;
		switch (event->key.m4_vk_code) {
		case M4VK_SHIFT: s = 2; sr->key_states |= M4KM_SHIFT; break;
		case M4VK_CONTROL: c = 2; sr->key_states |= M4KM_CTRL; break;
		case M4VK_MENU: m = 2; sr->key_states |= M4KM_ALT; break;
		}
		event->key.key_states = sr->key_states;
		/*key sensor*/
		if (sr->term && (sr->interaction_level & M4_InteractInputSensor) ) {
			if (event->key.m4_vk_code<=M4VK_RIGHT) M4T_KeyboardInput(sr->term, 0, 0, event->key.m4_vk_code, 0, s, c, m);
			else M4T_KeyboardInput(sr->term, event->key.virtual_code, 0, 0, 0, s, c, m);
		}		
		SR_UserInput(sr, event);
		break;

	case M4E_VKEYUP:
		s = c = m = 0;
		switch (event->key.m4_vk_code) {
		case M4VK_SHIFT: s = 1; sr->key_states &= ~M4KM_SHIFT; break;
		case M4VK_CONTROL: c = 1; sr->key_states &= ~M4KM_CTRL; break;
		case M4VK_MENU: m = 1; sr->key_states &= ~M4KM_ALT; break;
		}
		event->key.key_states = sr->key_states;

		/*key sensor*/
		if (sr->term && (sr->interaction_level & M4_InteractInputSensor) ) {
			if (event->key.m4_vk_code<=M4VK_RIGHT) M4T_KeyboardInput(sr->term, 0, 0, 0, event->key.m4_vk_code, s, c, m);
			else M4T_KeyboardInput(sr->term, 0, event->key.virtual_code, 0, 0, s, c, m);
		}
		SR_UserInput(sr, event);
		break;

	/*for key sensor*/
	case M4E_KEYDOWN:
		event->key.key_states = sr->key_states;
		if (sr->term && (sr->interaction_level & M4_InteractInputSensor) )
			M4T_KeyboardInput(sr->term, event->key.virtual_code, 0, 0, 0, 0, 0, 0);
		if (sr->interaction_level & M4_InteractWM) M4USER_SENDEVENT(sr->client, event);

		SR_UserInput(sr, event);
		break;

	case M4E_KEYUP:
		event->key.key_states = sr->key_states;
		if (sr->term && (sr->interaction_level & M4_InteractInputSensor) )
			M4T_KeyboardInput(sr->term, 0, event->key.virtual_code, 0, 0, 0, 0, 0);
		if (sr->interaction_level & M4_InteractWM) M4USER_SENDEVENT(sr->client, event);

		SR_UserInput(sr, event);
		break;

	case M4E_CHAR:
		if (sr->term && (sr->interaction_level & M4_InteractInputSensor) )
			M4T_StringInput(sr->term , event->character.unicode_char);
		if (sr->interaction_level & M4_InteractWM) M4USER_SENDEVENT(sr->client, event);

		SR_UserInput(sr, event);
		break;

	case M4E_MOUSEMOVE:
	case M4E_LEFTDOWN:
	case M4E_LEFTUP:
	case M4E_RIGHTDOWN:
	case M4E_RIGHTUP:
	case M4E_MIDDLEDOWN:
	case M4E_MIDDLEUP:
	case M4E_MOUSEWHEEL:
		event->mouse.key_states = sr->key_states;
		SR_UserInput(sr, event);
		break;

	/*when we process events we don't forward them to the user*/
	default:
		if (sr->interaction_level & M4_InteractWM) M4USER_SENDEVENT(sr->client, event);
		break;
	}
	/*send ALL events to the user*/
	if (!(sr->interaction_level & M4_InteractWM)) M4USER_SENDEVENT(sr->client, event);
}

void SR_RegisterExtraGraph(LPSCENERENDER sr, LPSCENEGRAPH extra_scene, Bool do_remove)
{
	SR_Lock(sr, 1);
	if (do_remove) ChainDeleteItem(sr->extra_scenes, extra_scene);
	else if (ChainFindEntry(sr->extra_scenes, extra_scene)<0) ChainAddEntry(sr->extra_scenes, extra_scene);
	SR_Lock(sr, 0);
}
