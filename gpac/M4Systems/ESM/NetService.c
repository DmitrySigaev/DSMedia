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



#define M4TERM()	M4Client *term = (M4Client *) user_priv; if (!term) return;

static M4INLINE Channel *get_mpeg4_channel(LPNETSERVICE service, LPNETCHANNEL ns)
{
	Channel *ch = (Channel *)ns;
	if (!service || !ch) return NULL;
	if (ch->chan_id != (u32) ch) return NULL;
	if (ch->service != service) return NULL;
	return ch;
}

static void m4_on_message(void *user_priv, LPNETSERVICE service, M4Err error, const char *message)
{
	M4TERM();

	/*check for UDP timeout*/
	if (error==M4UDPTimeOut) {
		char *sOpt = IF_GetKey(term->user->config, "Network", "AutoReconfigUDP");
		if (sOpt && !stricmp(sOpt, "yes")) {
			sOpt = IF_GetKey(term->user->config, "Network", "UDPNotAvailable");
			/*if option is already set don't bother try reconfig*/
			if (!sOpt || stricmp(sOpt, "yes")) {
				char szMsg[1024];
				sprintf(szMsg, "UDP down (%s) - Retrying with TCP", message);
				M4_OnMessage(term, service->url, szMsg, M4OK);
				/*reconnect top-level*/
				sOpt = strdup(term->root_scene->root_od->net_service->url);
				M4T_CloseURL(term);
				IF_SetKey(term->user->config, "Network", "UDPNotAvailable", "yes");
				M4T_ConnectURL(term, sOpt);
				free(sOpt);
				return;
			}
		}
	}
	M4_OnMessage(term, service->url, message, error);
}

static void m4_on_connect(void *user_priv, LPNETSERVICE service, LPNETCHANNEL netch, M4Err err)
{
	Channel *ch;
	ODManager *root;
	M4TERM();

	root = service->owner;
	if (root && (root->net_service != service)) {
		M4_OnMessage(term, service->url, "Incomaptible plugin type", M4ServiceError);
		return;
	}
	/*this is service connection*/
	if (!netch) {
		if (err) {
			char msg[5000];
			sprintf(msg, "Cannot open %s", service->url);
			M4_OnMessage(term, service->url, msg, err);

			/*destroy service only if attached*/
			if (root) {
				Term_LockNet(term, 1);
				root->net_service = NULL;
				ChainDeleteItem(term->net_services, service);
				/*and queue for destroy*/
				ChainAddEntry(term->net_services_to_remove, service);
				Term_LockNet(term, 0);
				if (!root->parentscene) {
					M4Event evt;
					evt.type = M4E_CONNECT;
					evt.connect.is_connected = 0;
					M4USER_SENDEVENT(term->user, &evt);
				}
				return;
			}
		}

		if (!root) {
			/*channel service connect*/
			u32 i;
			Chain *ODs = NewChain();;
			Term_LockNet(term, 1);
			for (i=0; i<ChainGetCount(term->channels_pending); i++) {
				ChannelSetup *cs = ChainGetEntry(term->channels_pending, i);
				if (cs->ch->service != service) continue;
				ChainDeleteEntry(term->channels_pending, i);
				i--;
				/*even if error do setup (channel needs to be deleted)*/
				if (ODM_SetupChannel(cs->ch, cs->dec, err) == M4OK) {
					if (cs->ch->odm && (ChainFindEntry(ODs, cs->ch->odm)==-1) ) ChainAddEntry(ODs, cs->ch->odm);
				}
				free(cs);
			}
			Term_LockNet(term, 0);
			/*finally setup all ODs concerned (we do this later in case of scalability)*/
			while (ChainGetCount(ODs)) {
				ODManager *odm = ChainGetEntry(ODs, 0);
				ChainDeleteEntry(ODs, 0);
				/*force re-setup*/
				IS_SetupOD(odm->parentscene, odm);
			}
			DeleteChain(ODs);
			return;
		}
		/*setup od*/
		ODM_SetupService(root, NULL);

	}

	ch = get_mpeg4_channel(service, netch);
	if (!ch) return;

	/*this is channel connection*/
	if (err) {
		M4_OnMessage(term, service->url, "Channel Connection Failed", err);
		ch->es_state = ES_Unavailable;
		return;
	}

	Term_LockNet(term, 1);
	Channel_OnConnect(ch);
	Term_LockNet(term, 0);

	/*in case the OD user has requested a play send a PLAY on the object (Play request are skiped
	until all channels are connected) */
	if (ch->odm->mo && ch->odm->mo->num_open) {
		ODM_Start(ch->odm);
	}
	/*if this is a channel in the root OD play */
	else if (! ch->odm->parentscene) {
		ODM_Start(ch->odm);
	}
}

static void m4_on_disconnect(void *user_priv, LPNETSERVICE service, LPNETCHANNEL netch, M4Err response)
{
	ODManager *root;
	Channel *ch;
	M4TERM();

	/*may be null upon destroy*/
	root = service->owner;
	if (root && (root->net_service != service)) {
		M4_OnMessage(term, service->url, "Incompatible plugin type", M4ServiceError);
		return;
	}
	/*this is service disconnect*/
	if (!netch) {
		Term_LockNet(term, 1);
		/*unregister from valid services*/
		ChainDeleteItem(term->net_services, service);
		/*and queue for destroy*/
		ChainAddEntry(term->net_services_to_remove, service);
		Term_LockNet(term, 0);
		return;
	}
	/*this is channel disconnect*/

	/*no notif in case of failure for disconnection*/
	ch = get_mpeg4_channel(service, netch);
	if (!ch) return;
	/*signal channel state*/
	ch->es_state = ES_Disconnected;
}

static void m4_on_slp_recieved(void *user_priv, LPNETSERVICE service, LPNETCHANNEL netch, char *data, u32 data_size, struct tagSLHeader *hdr, M4Err reception_status)
{
	Channel *ch;
	M4TERM();

	ch = get_mpeg4_channel(service, netch);
	if (!ch) return;
	
	if (reception_status==M4EOF) {
		Channel_EndOfStream(ch);
		return;
	}
	/*otherwise dispatch with error code*/
	Channel_RecieveSLP(service, ch, data, data_size, hdr, reception_status);
}


static void m4_on_command(void *user_priv, LPNETSERVICE service, NetworkCommand *com, M4Err response)
{
	Channel *ch;
	M4TERM();

	if (!com->base.on_channel) return;

	ch = get_mpeg4_channel(service, com->base.on_channel);
	if (!ch) return;

	switch (com->command_type) {
	/*SL reconfiguration*/
	case CHAN_RECONFIG:
		Term_LockNet(term, 1);
		Channel_ReconfigSL(ch, &com->cfg.sl_config);
		Term_LockNet(term, 0);
		return;
	/*time mapping (TS to media-time)*/
	case CHAN_MAP_TIME:
		ch->seed_ts = com->map_time.timestamp;
		ch->ts_offset = (u32) (com->map_time.media_time*1000);
		/*
		if (Channel_OwnsClock(ch)) {
			ch->ts_offset = (u32) (com->map_time.media_time*1000);
		} else {
			ch->ts_offset = CK_GetTime(ch->clock);
		}
		*/
		Channel_TimeMapped(ch, com->map_time.reset_buffers);
		break;
	/*duration changed*/
	case CHAN_DURATION:
		ODM_SetDuration(ch->odm, ch, (u32) (1000*com->duration.duration));
		break;
	case CHAN_BUFFER_QUERY:
		com->buffer.max = ch->MaxBuffer;
		com->buffer.min = ch->MinBuffer;
		com->buffer.occupancy = ch->BufferTime;
		break;
	default:
		return;
	}
}

void Term_SetupNetwork(M4Client *app)
{
	app->client_sink.on_connect = m4_on_connect;
	app->client_sink.on_disconnect = m4_on_disconnect;
	app->client_sink.on_message = m4_on_message;
	app->client_sink.on_command = m4_on_command;
	app->client_sink.on_slp_recieved = m4_on_slp_recieved;
	app->client_sink.user_priv = app;
}

Bool net_check_interface(NetClientPlugin *ifce)
{
	if (!ifce->CanHandleURL) return 0;
	if (!ifce->ConnectService) return 0;
	if (!ifce->CloseService) return 0;
	if (!ifce->ConnectChannel) return 0;
	if (!ifce->DisconnectChannel) return 0;
	if (!ifce->Get_MPEG4_IOD) return 0;
	if (!ifce->ServiceCommand) return 0;
	return 1;
}

void NM_OnMimeState(NetDownloader *dnld)
{
}

char *NM_GetMimeType(struct _m4_client *term, const char *url, M4Err *ret_code)
{
	u32 i;
	char *mime_type;
	NetDownloader *ifce;

	(*ret_code) = M4OK;
	ifce = NULL;
	for (i=0; i< PM_GetPluginsCount(term->user->plugins); i++) {
		PM_LoadInterface(term->user->plugins, i, M4FILEDOWNLOADER, (void **) &ifce);
		if (!ifce) continue;
		if (ifce->CanHandleURL(url)) break;
		PM_ShutdownInterface(ifce);
		ifce = NULL;
	}
	if (!ifce) return NULL;
	ifce->OnState = NM_OnMimeState;
	ifce->user_cbck = term;
	mime_type = ifce->GetMimeType(ifce, url);
	(*ret_code) = ifce->error;
	ifce->Close(ifce);
	PM_ShutdownInterface(ifce);
	return mime_type;
}


static Bool check_extension(char *szExtList, char *szExt)
{
	char szExt2[50];
	if (szExtList[0] != '"') return 0;
	szExtList += 1;

	while (1) {
		u32 i = 0;
		while ((szExtList[0] != ' ') && (szExtList[0] != '"')) {
			szExt2[i] = szExtList[0];
			i++;
			szExtList++;
		}
		szExt2[i] = 0;
		if (!strncmp(szExt, szExt2, strlen(szExt2))) return 1;
		if (szExtList[0]=='"') break;
		else szExtList++;
	}
	return 0;
}

LPNETSERVICE NM_NewService(struct _m4_client *term, struct _od_manager *owner, const char *url, LPNETSERVICE parent_service, M4Err *ret_code)
{
	LPNETSERVICE serv;
	u32 i;
	M4Err e;
	char *sURL, *ext, *mime_type;
	char szExt[50];
	NetClientPlugin *ifce;

	if (!url) {
		(*ret_code) = M4URLNotFound;
		return NULL;
	}

	if (parent_service && parent_service->url) {
		sURL = URL_Concatenate(parent_service->url, url);
		/*path was absolute*/
		if (!sURL) sURL = strdup(url);
	} else {
		sURL = strdup(url);
	}

	/*fetch a mime type if any. If error don't even attempt to open the service*/
	mime_type = NM_GetMimeType(term, sURL, &e);
	if (e) {
		free(sURL);
		(*ret_code) = e;
		return NULL;
	}
	
	if (mime_type && !stricmp(mime_type, "text/plain")) {
		free(mime_type);
		mime_type = NULL;
	}

	ifce = NULL;

	/*load from mime type*/
	if (mime_type) {
		char *sPlug = IF_GetKey(term->user->config, "MimeTypes", mime_type);
		if (sPlug) sPlug = strrchr(sPlug, '"');
		if (sPlug) {
			sPlug += 2;
			PM_LoadInterfaceByName(term->user->plugins, sPlug, M4STREAMINGCLIENT, (void **) &ifce);
			if (ifce && !net_check_interface(ifce) ) {
				PM_ShutdownInterface(ifce);
				ifce = NULL;
			}
		}
	}

	ext = strrchr(sURL, '.');
	/*no mime type: either local or streaming. If streaming discard extension checking*/
	if (!ifce && !mime_type && strstr(sURL, "://") && strnicmp(sURL, "file://", 7)) ext = NULL;

	/*browse extensions for prefered plugin*/
	if (!ifce && ext) {
		u32 keyCount;
		strcpy(szExt, &ext[1]);
		ext = strrchr(szExt, '#');
		if (ext) ext[0] = 0;

		keyCount = IF_GetKeyCount(term->user->config, "MimeTypes");
		for (i=0; i<keyCount; i++) {
			char *sMime, *sKey, *sPlug;
			sMime = (char *) IF_GetKeyName(term->user->config, "MimeTypes", i);
			if (!sMime) continue;
			sKey = IF_GetKey(term->user->config, "MimeTypes", sMime);
			if (!sKey) continue;
			if (!check_extension(sKey, szExt)) continue;
			sPlug = strrchr(sKey, '"');
			if (!sPlug) continue;	/*bad format entry*/
			sPlug += 2;

			PM_LoadInterfaceByName(term->user->plugins, sPlug, M4STREAMINGCLIENT, (void **) &ifce);
			if (!ifce) continue;
			if (!net_check_interface(ifce)) {
				PM_ShutdownInterface(ifce);
				ifce = NULL;
				continue;
			}
			break;
		}
	}

	/*browse all plugins*/
	if (!ifce) {
		for (i=0; i< PM_GetPluginsCount(term->user->plugins); i++) {
			PM_LoadInterface(term->user->plugins, i, M4STREAMINGCLIENT, (void **) &ifce);
			if (!ifce) continue;
			if (net_check_interface(ifce) && ifce->CanHandleURL(ifce, sURL)) break;
			PM_ShutdownInterface(ifce);
			ifce = NULL;
		}
	}

	if (mime_type) free(mime_type);

	if (!ifce) {
		free(sURL);
		(*ret_code) = M4UnsupportedURL;
		return NULL;
	}
	serv = malloc(sizeof(NetService));
	memset(serv, 0, sizeof(NetService));
	serv->term = term;
	serv->owner = owner;
	serv->ifce = ifce;
	serv->url = sURL;
	serv->Clocks = NewChain();
	serv->dnloads = NewChain();
	ChainAddEntry(term->net_services, serv);
	return serv;
}

M4Err NM_OpenService(LPNETSERVICE ns)
{
	return ns->ifce->ConnectService(ns->ifce, ns, ns->url);
}
M4Err NM_CloseService(LPNETSERVICE ns) 
{ 
	return ns->ifce->CloseService(ns->ifce, 0); 
}
Bool NM_CanHandleURLInService(LPNETSERVICE ns, char *url)
{
	/*if no owner attached the service is being deleted, don't query it*/
	if (!ns->owner || !ns->ifce->CanHandleURLInService) return 0;
	return ns->ifce->CanHandleURLInService(ns->ifce, url);
}
M4Err NM_Get_MPEG4_IOD(LPNETSERVICE ns, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size) 
{
	return ns->ifce->Get_MPEG4_IOD(ns->ifce, expect_type, sub_url, raw_iod, raw_iod_size); 
}
M4Err NM_ConnectChannel(LPNETSERVICE ns, LPNETCHANNEL channel, const char *url, Bool upstream) 
{ 
	return ns->ifce->ConnectChannel(ns->ifce, channel, url, upstream); 
}
M4Err NM_DisconnectChannel(LPNETSERVICE ns, LPNETCHANNEL channel) 
{
	return ns->ifce->DisconnectChannel(ns->ifce, channel); 
}
M4Err NM_ServiceCommand(LPNETSERVICE ns, NetworkCommand *com) 
{ 
	return ns->ifce->ServiceCommand(ns->ifce, com); 
}
M4Err NM_ChannelGetSLP(LPNETSERVICE ns, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	if (!ns->ifce->ChannelGetSLP) return M4NotSupported;
	return ns->ifce->ChannelGetSLP(ns->ifce, channel, out_data_ptr, out_data_size, out_sl_hdr, sl_compressed, out_reception_status, is_new_data);
}
M4Err NM_ChannelReleaseSLP(LPNETSERVICE ns, LPNETCHANNEL channel)
{
	if (!ns->ifce->ChannelGetSLP) return M4NotSupported;
	return ns->ifce->ChannelReleaseSLP(ns->ifce, channel);
}
void NM_OnMessage(LPNETSERVICE service, M4Err error, const char *message)
{
	assert(service);
	service->term->client_sink.on_message(service->term, service, error, message);
}
void NM_OnConnect(LPNETSERVICE service, LPNETCHANNEL ns, M4Err response)
{
	assert(service);
	service->term->client_sink.on_connect(service->term, service, ns, response);
}
void NM_OnDisconnect(LPNETSERVICE service, LPNETCHANNEL ns, M4Err response)
{
	assert(service);
	service->term->client_sink.on_disconnect(service->term, service, ns, response);
}
void NM_OnCommand(LPNETSERVICE service, NetworkCommand *com, M4Err response)
{
	assert(service);
	service->term->client_sink.on_command(service->term, service, com, response);
}
void NM_OnSLPRecieved(LPNETSERVICE service, LPNETCHANNEL ns, char *data, u32 data_size, struct tagSLHeader *hdr, M4Err reception_status)
{
	assert(service);
	service->term->client_sink.on_slp_recieved(service->term, service, ns, data, data_size, hdr, reception_status);
}
const char *NM_GetServiceURL(LPNETSERVICE service)
{
	if (!service) return NULL;
	return service->url;
}

void NM_DeleteService(LPNETSERVICE ns)
{
	PM_ShutdownInterface(ns->ifce);
	free(ns->url);

	/*delete all the clocks*/
	while (ChainGetCount(ns->Clocks)) {
		Clock *ck = ChainGetEntry(ns->Clocks, 0);
		ChainDeleteEntry(ns->Clocks, 0);
		DeleteClock(ck);
	}
	DeleteChain(ns->Clocks);

	assert(!ChainGetCount(ns->dnloads));
	DeleteChain(ns->dnloads);

	free(ns);
}


typedef struct _filedownloader 
{
	/*download*/
	NetDownloader *ifce;

	/*service this download runs in*/
	LPNETSERVICE service;
	char *sURL;

	void (*OnData)(void *usr_cbk, char *data, u32 data_size);
	void (*OnState)(void *usr_cbk);
	void *usr_cbk;
} FileDownloader;

static void NM_OnData(struct _netdownloader *nd, char *data, u32 data_size)
{
	FileDownloader *dnload;
	dnload = (FileDownloader *)nd->user_cbck;
	if (dnload->OnData) {
		/*send prog*/

		dnload->OnData(dnload->usr_cbk, data, data_size);
	}
}

static void NM_OnState(struct _netdownloader *nd)
{
	FileDownloader *dnload;
	dnload = (FileDownloader *)nd->user_cbck;
	if (dnload->OnState) dnload->OnState(dnload->usr_cbk);
}


M4Err NM_FetchFile(LPNETSERVICE service, const char *url, Bool disable_cache, void (*OnState)(void *cbk), void (*OnData)(void *cbk, char *data, u32 data_size), void *cbk, FileDownloader **out_dnload)
{
	FileDownloader *dnload;
	M4Err e;
	u32 i, count;
	*out_dnload = NULL;
	if (!service || !OnState) return M4BadParam;

	dnload = malloc(sizeof(FileDownloader));
	dnload->service = service;

	dnload->sURL = URL_Concatenate(service->url, url);
	/*path was absolute*/
	if (!dnload->sURL) dnload->sURL = strdup(url);

	count = PM_GetPluginsCount(service->term->user->plugins);
	for (i=0; i<count; i++) {
		if (PM_LoadInterface(service->term->user->plugins, i, M4FILEDOWNLOADER, (void **) &dnload->ifce)) {
			if (dnload->ifce->CanHandleURL(dnload->sURL)) break;
			PM_ShutdownInterface(dnload->ifce);
		}
		dnload->ifce = NULL;
	}
	if (!dnload->ifce) {
		free(dnload->sURL);
		free(dnload);
		return M4UnsupportedURL;
	}
	dnload->OnState = OnState;
	dnload->OnData = OnData;
	dnload->usr_cbk = cbk;
	dnload->ifce->OnState = NM_OnState;
	dnload->ifce->OnData = OnData ? NM_OnData : NULL;
	dnload->ifce->user_cbck = dnload;
	*out_dnload = dnload;
	ChainAddEntry(service->dnloads, dnload);
	e = dnload->ifce->Connect(dnload->ifce, dnload->sURL, disable_cache);
	if (e<M4OK) {
		*out_dnload = NULL;
		ChainDeleteItem(service->dnloads, dnload);
		PM_ShutdownInterface(dnload->ifce);
		free(dnload->sURL);
		free(dnload);
	}
	return e;
}

M4Err NM_FetchData(LPFILEDOWNLOADER dnload, char *buffer, u32 buffer_size, u32 *read_size)
{
	return dnload->ifce->FetchData(dnload->ifce, buffer, buffer_size, read_size);
}

void NM_CloseDownloader(LPFILEDOWNLOADER dnload)
{
	if (!dnload) return;
	/*avoid sending data back to plugin*/
	dnload->OnData = NULL;
	/*unregister from service*/
	ChainDeleteItem(dnload->service->dnloads, dnload);

	/*same as service: this may be called in the downloader thread (typically when download fails)
	so we must queue the downloader and let the term delete it later on*/
	ChainAddEntry(dnload->service->term->net_services_to_remove, dnload);
}


const char *NM_GetCacheFileName(LPFILEDOWNLOADER dnload)
{
	if (!dnload) return NULL;
	return dnload->ifce->GetCacheFileName(dnload->ifce);
}

M4Err NM_GetDownloaderStats(LPFILEDOWNLOADER dnload, u32 *total_size, u32 *bytes_done, Float *bytes_per_sec, u32 *net_status)
{
	if (!dnload) return M4BadParam;
	if (total_size) *total_size = dnload->ifce->total_size;
	if (bytes_done) *bytes_done = dnload->ifce->bytes_done;
	if (bytes_per_sec) *bytes_per_sec = dnload->ifce->bytes_per_sec;
	if (net_status) *net_status = dnload->ifce->net_status;
	return dnload->ifce->error;
}

void NM_GetDownloaderInfo(LPFILEDOWNLOADER dnload, const char **url, u32 *bytes_done, u32 *total_bytes, Float *bytes_per_sec)
{
	if (url) *url = dnload->sURL;
	if (total_bytes) *total_bytes = dnload->ifce->total_size;
	if (bytes_done) *bytes_done = dnload->ifce->bytes_done;
	if (bytes_per_sec) *bytes_per_sec = dnload->ifce->bytes_per_sec;
}

static void NM_DeleteDownloader(LPFILEDOWNLOADER dnload)
{
	dnload->ifce->Close(dnload->ifce);
	PM_ShutdownInterface(dnload->ifce);
	free(dnload->sURL);
	free(dnload);
}

void NM_DestroyService(LPNETSERVICE ns)
{
	if (ns->ifce->InterfaceType==M4FILEDOWNLOADER) {
		NM_DeleteDownloader((FileDownloader *) ns);
	} else {
		NM_DeleteService(ns);
	}
}

void NM_RegisterMimeType(NetClientPlugin *ifce, const char *mimeType, const char *extList, const char *description)
{
	u32 len;
	char *buf;
	if (!ifce || !mimeType || !extList || !description) return;

	len = strlen(extList) + 3 + strlen(description) + 3 + strlen(ifce->plugin_name) + 1;
	buf = malloc(sizeof(char)*len);
	sprintf(buf, "\"%s\" ", extList);
	strlwr(buf);
	strcat(buf, "\"");
	strcat(buf, description);
	strcat(buf, "\" ");
	strcat(buf, ifce->plugin_name);
	PMI_SetOpt(ifce, "MimeTypes", mimeType, buf);
	free(buf);
}

Bool NM_CheckExtension(NetClientPlugin *ifce, const char *mimeType, const char *extList, const char *description, const char *fileExt)
{
	char *szExtList, szExt[50];
	if (!ifce || !mimeType || !extList || !description || !fileExt) return 0;
	if (fileExt[0]=='.') fileExt++;
	strcpy(szExt, fileExt);
	strlwr(szExt);
	szExtList = strchr(szExt, '#');
	if (szExtList) szExtList[0]=0;


	szExtList = PMI_GetOpt(ifce, "MimeTypes", mimeType);
	if (!szExtList) {
		NM_RegisterMimeType(ifce, mimeType, extList, description);
		szExtList = PMI_GetOpt(ifce, "MimeTypes", mimeType);
	}
	if (!strstr(szExtList, ifce->plugin_name)) return 0;
	return check_extension(szExtList, szExt);
}
