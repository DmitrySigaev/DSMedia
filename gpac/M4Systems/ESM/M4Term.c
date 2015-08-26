/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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


#include <gpac/intern/m4_esm_dev.h>
#include <gpac/intern/m4_render_dev.h>

u32 Term_GetTime(M4Client *term)
{
	assert(term);
	return SR_GetTime(term->renderer);
}

void Term_InvalidateScene(M4Client *term)
{
	SR_Invalidate(term->renderer, NULL);
}

static Bool check_user(M4User *user)
{
	if (!user->config) return 0;
	if (!user->plugins) return 0;
	if (!user->opaque) return 0;
	if (!user->os_window_handler) user->dont_override_window_proc = 0;
	/*we need an event proc in this case*/
	if (user->dont_override_window_proc && !user->EventProc) return 0;
	return 1;
}

/*script API*/
typedef struct
{
	LPFILEDOWNLOADER dnload;
	void (*OnDone)(void *cbck, Bool success, const char *the_file_path);
	void *cbk;
} JSDownload;

void JS_OnState(void *cbck)
{
	JSDownload *jsdnload = (JSDownload *)cbck;
	M4Err e = NM_GetDownloaderStats(jsdnload->dnload, NULL, NULL, NULL, NULL);
	if (e<M4OK) {
		jsdnload->OnDone(jsdnload->cbk, 0, NULL);
		NM_CloseDownloader(jsdnload->dnload);
		free(jsdnload);
	}
	else if (e==M4EOF) {
		const char *szCache = NM_GetCacheFileName(jsdnload->dnload);
		jsdnload->OnDone(jsdnload->cbk, 1, szCache);
		NM_CloseDownloader(jsdnload->dnload);
		free(jsdnload);
	}
}

Bool OnJSGetScriptFile(void *opaque, LPSCENEGRAPH parent_graph, const char *url, void (*OnDone)(void *cbck, Bool success, const char *the_file_path), void *cbk)
{
	JSDownload *jsdnload;
	InlineScene *is;
	if (!parent_graph || !OnDone) return 0;
	is = SG_GetPrivate(parent_graph);
	if (!is) return 0;
	SAFEALLOC(jsdnload, sizeof(JSDownload));
	jsdnload->OnDone = OnDone;
	jsdnload->cbk = cbk;
	if (NM_FetchFile(is->root_od->net_service, url, 0, JS_OnState, NULL, jsdnload, &jsdnload->dnload) != M4OK) {
		free(jsdnload);
		OnDone(cbk, 0, NULL);
		return 0;
	}
	return 1;
}

const char *OnJSGetOption(void *opaque, char *option)
{
	M4Client *term = (M4Client *) opaque;
	if (!stricmp(option, "WorldURL")) return term->root_scene->root_od->net_service->url;
	return NULL;
}
void OnJSError(void *opaque, const char *msg)
{
	M4Client *term = (M4Client *) opaque;
	M4_OnMessage(term, term->root_scene->root_od->net_service->url, msg, M4ScriptError);
}
void OnJSPrint(void *opaque, const char *msg)
{
	M4Client *term = (M4Client *) opaque;
	M4_OnMessage(term, term->root_scene->root_od->net_service->url, msg, M4ScriptInfo);
}

Bool OnJSLoadURL(void *opaque, const char *url, const char **params, u32 nb_params)
{
	M4Event evt;
	M4Client *term = (M4Client *) opaque;
	if (!term->user->EventProc) return 0;
	evt.type = M4E_NAVIGATE;
	evt.navigate.to_url = url;
	evt.navigate.parameters = params;
	evt.navigate.param_count = nb_params;
	return term->user->EventProc(term->user->opaque, &evt);
}

static void M4T_ReloadConfig(MPEG4CLIENT term)
{
	char *sOpt;
	Double fps;
	u32 mode;
	s32 prio;

	if (!term) return;
	
	/*reload term part*/

	sOpt = IF_GetKey(term->user->config, "Systems", "AlwaysDrawBIFS");
	if (sOpt && !stricmp(sOpt, "yes"))
		term->bifs_can_resync = 0;
	else
		term->bifs_can_resync = 1;

	sOpt = IF_GetKey(term->user->config, "Systems", "ForceSingleClock");
	if (sOpt && !stricmp(sOpt, "yes")) 
		term->force_single_clock = 1;
	else
		term->force_single_clock = 0;

	sOpt = IF_GetKey(term->user->config, "Rendering", "FrameRate");
	if (sOpt) {
		fps = atof(sOpt);
		if (term->system_fps != fps) {
			term->system_fps = fps;
			term->half_frame_duration = (u32) (500/fps);
			SR_SetFrameRate(term->renderer, fps);
		}
	}

	prio = TH_PRIOR_NORMAL;
	sOpt = IF_GetKey(term->user->config, "Systems", "Priority");
	if (sOpt) {
		if (!stricmp(sOpt, "low")) prio = TH_PRIOR_LOWEST;
		else if (!stricmp(sOpt, "normal")) prio = TH_PRIOR_NORMAL;
		else if (!stricmp(sOpt, "high")) prio = TH_PRIOR_HIGHEST;
		else if (!stricmp(sOpt, "real-time")) prio = TH_PRIOR_REALTIME;
	} else {
		IF_SetKey(term->user->config, "Systems", "Priority", "normal");
	}
	MM_SetPriority(term->mediaman, prio);

	sOpt = IF_GetKey(term->user->config, "Systems", "ThreadingPolicy");
	if (sOpt) {
		mode = 0;
		if (!stricmp(sOpt, "Single")) mode = 1;
		else if (!stricmp(sOpt, "Multi")) mode = 2;
		MM_SetThreadingMode(term->mediaman, mode);
	}

	if (term->root_scene) IS_SetSceneDuration(term->root_scene);
	/*reload renderer config*/
	SR_SetOption(term->renderer, M4O_ReloadConfig, 1);
}


M4Client *NewMPEG4Term(M4User *user)
{
	M4Client *tmp;
	char *cf;
	u32 mode;
	if (!check_user(user)) return NULL;

	tmp = malloc(sizeof(M4Client));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(M4Client));

	M4_InitClock();

	tmp->user = user;
	tmp->js_ifce.callback = tmp;
	tmp->js_ifce.Error = OnJSError;
	tmp->js_ifce.Print = OnJSPrint;
	tmp->js_ifce.GetOption = OnJSGetOption;
	tmp->js_ifce.GetScriptFile = OnJSGetScriptFile;
	tmp->js_ifce.LoadURL = OnJSLoadURL;

	cf = IF_GetKey(user->config, "Systems", "NoVisualThread");
	if (!cf || !stricmp(cf, "no")) {
		tmp->render_frames = 0;
	} else {
		tmp->render_frames = 1;
	}

	/*setup scene renderer*/
	tmp->renderer = NewSceneRender(user, !tmp->render_frames, 0, tmp);
	if (!tmp->renderer) {
		free(tmp);
		return NULL;
	}
	tmp->system_fps = 30.0;
	SR_SetFrameRate(tmp->renderer, tmp->system_fps);
	tmp->half_frame_duration = (u32) (500/tmp->system_fps);

	/*net setup*/
	Term_SetupNetwork(tmp);
	
	tmp->net_services = NewChain();
	tmp->net_services_to_remove = NewChain();
	tmp->channels_pending = NewChain();
	tmp->od_pending = NewChain();
	
	tmp->net_mx = NewMutex();
	tmp->input_streams = NewChain();
	tmp->x3d_sensors = NewChain();

	cf = IF_GetKey(user->config, "Systems", "ThreadingPolicy");
	mode = MM_THREAD_FREE;
	if (!cf || !stricmp(cf, "Free")) {
		if (!cf) IF_SetKey(user->config, "Systems", "ThreadingPolicy", "Free");
		mode = 0;
	} else if (!stricmp(cf, "Multi")) mode = MM_THREAD_MULTI;
	else if (!stricmp(cf, "Single")) mode = MM_THREAD_SINGLE;

	tmp->mediaman = NewMediaManager(tmp, mode);

	M4T_ReloadConfig(tmp);
	return tmp;
}

M4Err M4T_Delete(M4Client * term)
{
	M4Err e;
	u32 timeout;

	if (!term) return M4BadParam;

	/*disconnect main scene from the renderer*/
	SR_SetSceneGraph(term->renderer, NULL);

	/*close main service*/
	M4T_CloseURL(term);

	/*wait for destroy*/
	e = M4IOErr;
	timeout = 1000;
	while (term->root_scene || ChainGetCount(term->net_services) || ChainGetCount(term->net_services_to_remove)) {
		Sleep(30);
		/*this shouldn't be needed but unfortunately there's a bug hanging around there...*/
		timeout--;
		if (!timeout) break;
	}
	if (timeout) {
		assert(!ChainGetCount(term->net_services));
		assert(!ChainGetCount(term->net_services_to_remove));
		e = M4OK;
	} 

	/*stop the media manager */
	MM_Delete(term->mediaman);

	/*delete renderer before the input sensor stacks to avoid recieving events from the renderer
	when destroying these stacks*/
	SR_Delete(term->renderer);

	DeleteChain(term->net_services);
	DeleteChain(term->net_services_to_remove);
	DeleteChain(term->input_streams);
	DeleteChain(term->x3d_sensors);
	assert(!ChainGetCount(term->channels_pending));
	DeleteChain(term->channels_pending);
	assert(!ChainGetCount(term->od_pending));
	DeleteChain(term->od_pending);
	MX_Delete(term->net_mx);
	M4_StopClock();
	free(term);
	return e;
}



void M4_OnMessage(M4Client *term, const char *service, const char *message, M4Err error)
{
	if (!term || !term->user) return;
	M4USER_MESSAGE(term->user, service, message, error);
}


void M4T_ConnectURLWithStartTime(M4Client * term, const char *URL, u32 startTime)
{
	InlineScene *is;
	ODManager *odm;
	const char *main_url;
	if (!URL || !strlen(URL)) return;

	if (term->root_scene) {
		if (term->root_scene->root_od && term->root_scene->root_od->net_service) {
			main_url = term->root_scene->root_od->net_service->url;
			if (main_url && !strcmp(main_url, URL)) {
				M4T_PlayFromTime(term, 0);
				return;
			}
		}
		/*disconnect*/
		M4T_CloseURL(term);
	}

	Term_LockNet(term, 1);
	/*create a new scene*/
	is = NewInlineScene(NULL);
	odm = NewODManager();
	SG_SetJavaScriptAPI(is->graph, &term->js_ifce);

	is->root_od = odm;
	term->root_scene = is;
	odm->parentscene = NULL;
	odm->subscene = is;
	odm->term = term;
	Term_LockNet(term, 0);

	term->restart_time = startTime;

	/*connect - we don't have any parentID */
	Term_ConnectODManager(term, odm, (char *) URL, NULL);
}

void M4T_ConnectURL(M4Client * term, const char *URL)
{
	M4T_ConnectURLWithStartTime(term, URL, 0);
}

void M4T_CloseURL(M4Client *term)
{
	if (!term->root_scene) return;

	SR_SetSceneGraph(term->renderer, NULL);
	ODM_RemoveOD(term->root_scene->root_od);
	while (term->root_scene) Sleep(10);
}



/*set rendering option*/
M4Err M4T_SetOption(M4Client * term, u32 type, u32 value)
{
	if (!term) return M4BadParam;
	if (type==M4O_ReloadConfig) {
		M4T_ReloadConfig(term);
		return M4OK;
	} else {
		return SR_SetOption(term->renderer, type, value);
	}
}


/*returns 0 if any of the clock still hasn't seen EOS*/
u32 Term_CheckClocks(NetService *ns, InlineScene *is)
{
	u32 i;
	if (is) {
		if (is->root_od->net_service != ns) {
			if (!Term_CheckClocks(is->root_od->net_service, is)) return 0;
		}
		for (i=0; i<ChainGetCount(is->ODlist); i++) {
			ODManager *odm = ChainGetEntry(is->ODlist, i);
			if (odm->net_service != ns) {
				while (odm->remote_OD) odm = odm->remote_OD;
				if (!Term_CheckClocks(odm->net_service, NULL)) return 0;
			}
		}
	}
	for (i=0; i<ChainGetCount(ns->Clocks); i++) {
		Clock *ck = ChainGetEntry(ns->Clocks, i);
		if (!ck->has_seen_eos) return 0;
	}
	return 1;
}

u32 Term_CheckIsOver(M4Client *term)
{
	ODManager *odm;
	if (!term->root_scene) return 1;
	/*if input sensors consider the scene runs forever*/
	if (ChainGetCount(term->input_streams)) return 0;
	if (ChainGetCount(term->x3d_sensors)) return 0;
	odm = term->root_scene->root_od;
	while (odm->remote_OD) odm = odm->remote_OD;
	/*check no clocks are still running*/
	if (!Term_CheckClocks(odm->net_service, term->root_scene)) return 0;
	if (term->root_scene->is_dynamic_scene) return 1;
	/*ask renderer if there are sensors*/
	return SR_GetOption(term->renderer, M4O_IsOver);
}

/*get rendering option*/
u32 M4T_GetOption(M4Client * term, u32 type)
{
	if (!term) return 0;
	switch (type) {
	case M4O_HasScript: return SG_HasScripting();
	case M4O_IsOver: return Term_CheckIsOver(term);
	case M4O_HasSelectableStreams: return (term->root_scene && term->root_scene->is_dynamic_scene) ? 1 : 0;
	default: return SR_GetOption(term->renderer, type);
	}
}

/*call by owner when size / position has changed*/
void M4T_RefreshWindow(M4Client * term)
{
	if (!term) return;
	SR_RefreshWindow(term->renderer);
}

M4Err M4T_SizeChanged(M4Client * term, u32 NewWidth, u32 NewHeight)
{
	if (!term) return 0;
	return SR_SizeChanged(term->renderer, NewWidth, NewHeight);
}

M4Err M4T_SetSize(M4Client * term, u32 NewWidth, u32 NewHeight)
{
	if (!term) return 0;
	return SR_SetSize(term->renderer, NewWidth, NewHeight);
}

void Term_HandleServices(M4Client *term)
{
	LPNETSERVICE ns;

	/*play ODs that need it*/
	MX_P(term->net_mx);
	while (ChainGetCount(term->od_pending)) {
		ODManager *odm = ChainGetEntry(term->od_pending, 0);
		ChainDeleteEntry(term->od_pending, 0);
		ODM_Play(odm);
	}
	MX_V(term->net_mx);

	/*lock to avoid any start attemps from renderer*/
	SR_Lock(term->renderer, 1);
	while (ChainGetCount(term->net_services_to_remove)) {
		MX_P(term->net_mx);
		ns = ChainGetEntry(term->net_services_to_remove, 0);
		if (ns) ChainDeleteEntry(term->net_services_to_remove, 0);
		MX_V(term->net_mx);
		if (!ns) break;
		NM_DestroyService(ns);
	}
	SR_Lock(term->renderer, 0);

	/*need to reload*/
	if (term->reload_state == 1) {
		term->reload_state = 2;
		M4T_CloseURL(term);
	}
	if (term->reload_state == 2) {
		term->reload_state = 0;
		M4T_ConnectURL(term, term->reload_url);
		free(term->reload_url);
		term->reload_url = NULL;
	}
}

void Term_CloseService(M4Client *term, LPNETSERVICE ns)
{
	M4Err e = NM_CloseService(ns);
	ns->owner = NULL;
	/*if error don't wait for ACK to remove from main list*/
	if (e) {
		ChainDeleteItem(term->net_services, ns);
		ChainAddEntry(term->net_services_to_remove, ns);
	}
}

void Term_LockScene(M4Client *term, Bool LockIt)
{
	SR_Lock(term->renderer, LockIt);
}

void Term_LockNet(M4Client *term, Bool LockIt)
{
	if (LockIt) {
		MX_P(term->net_mx);
	} else {
		MX_V(term->net_mx);
	}
}


/*connects given OD manager to its URL*/
void Term_ConnectODManager(M4Client *term, ODManager *odm, char *serviceURL, LPNETSERVICE ParentService)
{
	u32 i;
	M4Err e;
	LPNETSERVICE ns;
	Term_LockNet(term, 1);

	/*for remoteODs/dynamic ODs, check if one of the running service cannot be used*/
	for (i=0; i<ChainGetCount(term->net_services); i++) {
		ns = ChainGetEntry(term->net_services, i);
		if (NM_CanHandleURLInService(ns, serviceURL)) {
			odm->net_service = ns;
			ODM_SetupService(odm, serviceURL);
			Term_LockNet(term, 0);
			return;
		}
	}

	odm->net_service = NM_NewService(term, odm, serviceURL, ParentService, &e);
	if (!odm->net_service) {
		Term_LockNet(term, 0);
		M4_OnMessage(term, serviceURL, "Cannot open service", e);
		ODM_RemoveOD(odm);
		return;
	}
	/*OK connect*/
	NM_OpenService(odm->net_service);

	Term_LockNet(term, 0);
}

/*connects given channel to its URL if needed*/
M4Err Term_ConnectChannelURL(M4Client *term, Channel *ch, char *URL)
{
	M4Err e;
	u32 i;
	LPNETSERVICE ns;

	Term_LockNet(term, 1);

	/*if service is handled by current service don't create new one*/
	if (NM_CanHandleURLInService(ch->service, URL)) {
		Term_LockNet(term, 0);
		return M4OK;
	}
	for (i=0; i<ChainGetCount(term->net_services); i++) {
		ns = ChainGetEntry(term->net_services, i);
		if (NM_CanHandleURLInService(ns, URL)) {
			ch->service = ns;
			Term_LockNet(term, 0);
			return M4OK;
		}
	}
	/*use parent OD for parent service*/
	ns = NM_NewService(term, NULL, URL, ch->odm->net_service, &e);
	if (!ns) return e;
	ch->service = ns;
	NM_OpenService(ns);

	Term_LockNet(term, 0);
	return M4OK;
}

void M4T_PlayFromTime(MPEG4CLIENT term, u32 from_time)
{
	if (!term || !term->root_scene) return;
	if (term->root_scene->root_od->no_time_ctrl) return;

	/*for dynamic scene OD ressources are static and all object use the same clock, so don't restart the root 
	OD, just act as a mediaControl on all playing streams*/
	if (term->root_scene->is_dynamic_scene) {
		SR_Lock(term->renderer, 1);
		IS_RestartDynamicScene(term->root_scene, from_time);
		SR_Lock(term->renderer, 0);
		return;
	}
	
	M4T_Pause(term, 1);
	SR_Lock(term->renderer, 1);
	SR_SetSceneGraph(term->renderer, NULL);
	/*stop root*/
	ODM_Stop(term->root_scene->root_od, 1);
	IS_Disconnect(term->root_scene);
	/*make sure we don't have OD queued*/
	while (ChainGetCount(term->od_pending)) ChainDeleteEntry(term->od_pending, 0);
	term->restart_time = from_time;

	ODM_Start(term->root_scene->root_od);
	SR_Lock(term->renderer, 0);
	M4T_Pause(term, 0);
}

void M4T_Pause(MPEG4CLIENT term, u32 DoPause)
{
	u32 i, j;
	if (!term || !term->root_scene) return;
	
	/*pause renderer*/
	SR_Pause(term->renderer, DoPause);
	/*control clock except in step mode*/
	if (DoPause==2) return;

	/*pause all clocks on all services*/
	for (i=0; i<ChainGetCount(term->net_services); i++) {
		NetService *ns = ChainGetEntry(term->net_services, i);
		for (j=0; j<ChainGetCount(ns->Clocks); j++) {
			Clock *ck = ChainGetEntry(ns->Clocks, j);
			if (DoPause) CK_Pause(ck);
			else CK_Resume(ck);
		}
	}
}




void M4T_UserInput(M4Client * term, M4Event *evt)
{
	if (term) SR_UserInput(term->renderer, evt);
}


Float M4T_GetCurrentFPS(MPEG4CLIENT term, Bool absoluteFPS)
{
	if (!term || !term->renderer) return 0;
	return SR_GetCurrentFPS(term->renderer, absoluteFPS);
}

/*get main scene current time in sec*/
u32 M4T_GetCurrentTimeInMS(MPEG4CLIENT term)
{
	if (!term || !term->root_scene || !term->root_scene->scene_codec) return 0;
	return CK_GetTime(term->root_scene->scene_codec->ck);
}

SFNode *M4T_PickNode(MPEG4CLIENT term, s32 X, s32 Y)
{
	if (!term || !term->renderer) return NULL;
	return SR_PickNode(term->renderer, X, Y);
}

void M4T_ReloadURL(MPEG4CLIENT term)
{
	if (!term->root_scene) return;
	if (term->reload_url) free(term->reload_url);
	term->reload_url = strdup(term->root_scene->root_od->net_service->url);
	term->reload_state = 1;
}

M4Err M4T_GetViewpoint(MPEG4CLIENT term, u32 viewpoint_idx, const char **outName, Bool *is_bound)
{
	return SR_GetViewpoint(term->renderer, viewpoint_idx, outName, is_bound);
}

M4Err M4T_SetViewpoint(MPEG4CLIENT term, u32 viewpoint_idx, const char *viewpoint_name)
{
	return SR_SetViewpoint(term->renderer, viewpoint_idx, viewpoint_name);
}

M4Err M4T_AddObject(MPEG4CLIENT term, const char *url, Bool auto_play)
{
	MediaObject *mo;
	SFURL sfurl;
	MFURL mfurl;
	if (!url || !term || !term->root_scene || !term->root_scene->is_dynamic_scene) return M4BadParam;

	sfurl.OD_ID = DYNAMIC_OD_ID;
	sfurl.url = (char *) url;
	mfurl.count = 1;
	mfurl.vals = &sfurl;
	/*only text tracks are supported for now...*/
	mo = IS_GetMediaObject(term->root_scene, &mfurl, NM_OD_TEXT);
	/*check if we must deactivate it*/
	if (mo && mo->odm) {
		if (mo->num_open && !auto_play) {
			IS_SelectODM(term->root_scene, mo->odm);
		} else {
			mo->odm->OD_PL = auto_play ? 1 : 0;
		}
	}
	return mo ? M4OK : M4NotSupported;
}

void M4T_OpenService(MPEG4CLIENT term, NetClientPlugin *service_hdl)
{
	Bool net_check_interface(NetClientPlugin *ifce);
	InlineScene *is;
	ODManager *odm;
	if (!net_check_interface(service_hdl)) return;

	if (term->root_scene) M4T_CloseURL(term);

	Term_LockNet(term, 1);
	
	/*create a new scene*/
	is = NewInlineScene(NULL);
	odm = NewODManager();
	SG_SetJavaScriptAPI(is->graph, &term->js_ifce);

	is->root_od = odm;
	term->root_scene = is;
	odm->parentscene = NULL;
	odm->subscene = is;
	odm->term = term;
	term->restart_time = 0;

	odm->net_service = malloc(sizeof(NetService));
	memset(odm->net_service, 0, sizeof(NetService));
	odm->net_service->term = term;
	odm->net_service->owner = odm;
	odm->net_service->ifce = service_hdl;
	odm->net_service->url = strdup("Internal Service Handler");
	odm->net_service->Clocks = NewChain();
	ChainAddEntry(term->net_services, odm->net_service);

	Term_LockNet(term, 0);

	/*OK connect*/
	NM_OpenService(odm->net_service);

}
