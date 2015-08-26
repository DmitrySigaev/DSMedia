/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / file downloader plugin
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


#include "file_dnload.h"

u32 FD_MSG(void *p)
{
	FileDownload *fd = (FileDownload *)p;
	if (fd->plug->OnState) fd->plug->OnState(fd->plug);
	return 1;
}

void FD_PostError(FileDownload *fd)
{
	if (fd->plug->OnState) fd->plug->OnState(fd->plug);
}

void FD_ConfigureCache(FileDownload *fd)
{
	u32 i, last_sep;
	FILE *cached;
	char tmp[MAX_URI_LENGTH], *sOpt;
	char *cache_dir = PMI_GetOpt(fd->plug, "General", "CacheDirectory");
	if (!cache_dir || !strlen(cache_dir) ) return;


	if (cache_dir[strlen(cache_dir)-1] != M4_PATH_SEPARATOR) {
		sprintf(fd->cache_name, "%s%c", cache_dir, M4_PATH_SEPARATOR);
	} else {
		strcpy(fd->cache_name, cache_dir);
	}

	strcpy(tmp, fd->server_name);
	strcat(tmp, fd->remote_path);
	last_sep = 0;
	for (i=0; i<strlen(tmp); i++) {
		if (tmp[i] == '/') tmp[i] = '_';
		else if (tmp[i] == '.') {
			tmp[i] = '_';
			last_sep = i;
		}
	}
	if (last_sep) tmp[last_sep] = '.';

	strcat(fd->cache_name, tmp);

	/*first try, checvk cached file*/
	if (!fd->cached_size) {
		/*if file present figure out how much of the file is downloaded - we assume 2^31 byte file max*/
		cached = fopen(fd->cache_name, "rb");
		if (cached) {
			fseek(cached, 0, SEEK_END);
			fd->cached_size = ftell(cached);
			fclose(cached);
		}
	}
	/*second try, disable cached file*/
	else {
		fd->cached_size = 0;
	}

	fd->use_cache = 1;
	/*are we using existing cached files ?*/
	sOpt = PMI_GetOpt(fd->plug, "Downloader", "RestartFiles");
	if (sOpt && !stricmp(sOpt, "yes")) fd->cached_size = 0;
}


void FD_ConnectTask(FileDownload *fd)
{
	if (!fd->sock) {
		fd->num_connect_retry = 40;
		fd->sock = NewSocket(SK_TYPE_TCP);
	}

	/*connect*/
	fd->plug->error = M4OK;
	fd->plug->net_status = DL_WaitingForAck;
	fd->plug->OnState(fd->plug);

	fd->plug->error = SK_Connect(fd->sock, fd->server_name, fd->session_port);
	if (fd->plug->error == M4SockWouldBlock) {
		/*retry*/
		if (fd->num_connect_retry) {
			fd->num_connect_retry--;
			fd->plug->error = M4OK;
			return;
		}
	}

	/*failed*/
	if (fd->plug->error) {
		fd->plug->net_status = DL_Unavailable;
		FD_PostError(fd);
	} else {
		fd->plug->net_status = DL_Connected;
		fd->plug->OnState(fd->plug);
		SK_SetBlockingMode(fd->sock, 1);

		fd->use_cache = 0;
		if (!fd->disable_cache) FD_ConfigureCache(fd);
	}
}

u32 FD_RunService(void *par)
{
	char *sOpt;
	NetDownloader *plug = par;
	FileDownload *fd = plug->priv;

	fd->thread_dead = 0;
	while (plug->net_status < DL_Disconnected) {
		switch (fd->session_type) {
		case FD_HTTP:
			HTTP_Task(fd);
			break;
		default:
			break;
		}
		Sleep(1);
	}

	/*destroy all sessions*/
	if (fd->sock) SK_Delete(fd->sock);
	fd->sock = NULL;
	plug->net_status = DL_Unavailable;

	if (fd->cache) fclose(fd->cache);
	if (fd->sock) SK_Delete(fd->sock);

	sOpt = PMI_GetOpt(plug, "Downloader", "CleanCache");
	if (sOpt && !stricmp(sOpt, "yes")) M4_DeleteFile(fd->cache_name);
	
	fd->thread_dead = 1;
	return 1;
}

static Bool FD_CanHandleURL(const char *url)
{
	return strnicmp(url, "http://", 7) ? 0 : 1;
}



static char *FD_GetMimeType(NetDownloader *plug, const char *url)
{
	s32 ret;
	char *tmp;
	FileDownload *fd = plug->priv;
	if (!fd) return NULL;
	if (!url) { plug->error = M4BadParam; return NULL; }
	
	plug->net_status = DL_Setup;
	plug->error = M4UnsupportedURL;
	if (!strnicmp(url, "http://", 7) ) {
		ret = sscanf(url, "http://%[^:/]:%hd", fd->server_name, &fd->session_port);
		if (ret < 1 ) {
			plug->error = M4InvalidURL;
		} else {
			if (ret<2) fd->session_port = HTTP_DEFAULT_PORT;
			fd->session_type = FD_HTTP;
			plug->error = M4OK;
		}
	}
	else if (!strnicmp(url, "ftp://", 6) ) {
		//extract the host name and the file name
		ret = sscanf(url, "ftp://%[^:/]:%hd", fd->server_name, &fd->session_port);
		if (ret < 1) {
			plug->error = M4InvalidURL;
		} else {
			if (ret<2) fd->session_port = FTP_DEFAULT_PORT;
			fd->session_type = FD_FTP;
			plug->error = M4OK;
		}
	}

	tmp = strstr(url, "://");
	tmp += 3;
	/*we need a target to download*/
	tmp = strstr(tmp, "/");

	if (plug->error != M4OK) {
		plug->net_status = DL_Unavailable;
		return NULL;
	}
	plug->net_status = DL_Setup;
	fd->plug = plug;
	if (tmp)
		strcpy(fd->remote_path, tmp);
	else
		strcpy(fd->remote_path, "/");
	fd->disable_cache = 1;
	switch (fd->session_type) {
	case FD_HTTP: HTTP_FetchMimeType(fd); break;
	default: break;
	}

	if (fd->sock) SK_Delete(fd->sock);
	fd->sock = NULL;
	plug->net_status = DL_Unavailable;
	if (plug->error) return NULL;
	return strdup(fd->mime_type);
}

static M4Err FD_Connect(NetDownloader *plug, const char *url, Bool disable_cache)
{
	s32 ret;
	char *tmp;
	FileDownload *fd = plug->priv;
	if (!fd) return M4ServiceError;
	if (!url) return M4BadParam;
	
	fd->disable_cache = disable_cache;
	fd->cached_size = 0;
	plug->net_status = DL_Setup;

	plug->error = M4UnsupportedURL;
	if (!strnicmp(url, "http://", 7) ) {
		//extract the host name and the file name
		ret = sscanf(url, "http://%[^:/]:%hd", fd->server_name, &fd->session_port);
		if (ret < 1 ) {
			plug->error = M4InvalidURL;
		} else {
			if (ret<2) fd->session_port = HTTP_DEFAULT_PORT;
			fd->session_type = FD_HTTP;
			plug->error = M4OK;
		}
	}
	else if (!strnicmp(url, "ftp://", 6) ) {
		//extract the host name and the file name
		ret = sscanf(url, "ftp://%[^:/]:%hd", fd->server_name, &fd->session_port);
		if (ret < 1) {
			plug->error = M4InvalidURL;
		} else {
			if (ret<2) fd->session_port = FTP_DEFAULT_PORT;
			fd->session_type = FD_FTP;
			plug->error = M4OK;
		}
	}

	tmp = strstr(url, "://");
	tmp += 3;
	/*we need a target to download*/
	tmp = strstr(tmp, "/");

	if (plug->error != M4OK) return plug->error;
	plug->net_status = DL_Setup;
	fd->plug = plug;
	if (tmp) strcpy(fd->remote_path, tmp);
	else strcpy(fd->remote_path, "/");
	/*let's go*/
	TH_Run(fd->service_thread, FD_RunService, plug);
	return M4OK;
}

static M4Err FD_Close(NetDownloader *plug)
{
	FileDownload *fd= plug->priv;
	if (fd->init_data) free(fd->init_data);
	fd->init_data = NULL;

	if (plug->net_status == DL_Unavailable) return M4OK;
	plug->net_status = DL_Disconnected;

	/*don't wait for exit, we may be called here from within our own thread...*/
	return M4OK;
}

const char *FD_GetCacheFileName(NetDownloader *plug)
{
	FileDownload *fd= plug->priv;
	if (!fd->use_cache) return NULL;
	return (const char *) fd->cache_name;
}



M4Err FD_FetchData(NetDownloader *plug, char *buffer, u32 buffer_size, u32 *read_size)
{
	FileDownload *fd= plug->priv;
	if (fd->use_cache || !buffer || !buffer_size) return M4BadParam;
	if (plug->net_status != DL_Running) return plug->error;

	*read_size = 0;
	switch (fd->session_type) {
	case FD_HTTP: return HTTP_ReadData(fd, buffer, buffer_size, read_size);
	default: return M4ServiceError;
	}
}

NetDownloader *FD_LoadPlugin()
{
	FileDownload *dnload;
	NetDownloader *plug = malloc(sizeof(NetDownloader));
	memset(plug, 0, sizeof(NetDownloader));
	M4_REG_PLUG(plug, M4FILEDOWNLOADER, "HTTP Downloader", "gpac distribution", 0);

	plug->CanHandleURL = FD_CanHandleURL;
	plug->GetMimeType = FD_GetMimeType;
	plug->Connect = FD_Connect;
	plug->FetchData = FD_FetchData;
	plug->Close= FD_Close;
	plug->GetCacheFileName = FD_GetCacheFileName;

	dnload = malloc(sizeof(FileDownload));
	memset(dnload, 0, sizeof(FileDownload));
	dnload->service_thread = NewThread();
	plug->net_status = DL_Unavailable;

	plug->priv = dnload;

	/*also init clock*/
	M4_InitClock();
	return plug;
}

void FD_Delete(BaseInterface *bi)
{
	NetDownloader *plug = (NetDownloader *) bi;
	FileDownload *fd = plug->priv;

	/*this will close and clean cache*/
	if (plug->net_status != DL_Unavailable) {
		while (!fd->thread_dead) Sleep(10);
	}
	TH_Delete(fd->service_thread);

	free(fd);
	free(bi);
	M4_StopClock();
}

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4FILEDOWNLOADER) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4FILEDOWNLOADER) return FD_LoadPlugin();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4FILEDOWNLOADER:
		FD_Delete(ptr);
		break;
	}
}
