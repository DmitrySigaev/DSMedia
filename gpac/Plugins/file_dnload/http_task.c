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

void HTTP_CheckDone(FileDownload *fd)
{
	u32 runtime;
	if (fd->plug->bytes_done == fd->plug->total_size) {
		fd->plug->error = M4EOF;
		SK_Delete(fd->sock);
		fd->sock = NULL;
		fd->plug->net_status = DL_Disconnected;
		FD_PostError(fd);
		return;
	}
	/*update state if not done*/
	runtime = M4_GetSysClock() - fd->start_time;
	if (!runtime) {
		fd->plug->bytes_per_sec = 0.0;
	} else {
		fd->plug->bytes_per_sec = 1000.0f * (fd->plug->bytes_done - fd->cached_size);
		fd->plug->bytes_per_sec /= runtime;
	}
}

static M4INLINE void http_data_recieved(FileDownload *fd, char *bytes, u32 nbBytes)
{
	if (fd->use_cache) {
		if (fd->cache) {
			fwrite(bytes, nbBytes, 1, fd->cache);
			fflush(fd->cache);
		}
		fd->plug->bytes_done += nbBytes;
		fd->plug->error = M4OK;
		if (fd->plug->OnData) fd->plug->OnData(fd->plug, NULL, nbBytes);
	} else {
		fd->plug->bytes_done += nbBytes;
		if (fd->plug->OnData) fd->plug->OnData(fd->plug, bytes, nbBytes);
	}
	HTTP_CheckDone(fd);
}


static M4INLINE u32 http_skip_space(char *val)
{
	u32 ret = 0;
	while (val[ret] == ' ') ret+=1;
	return ret;
}

static M4INLINE char *http_is_header(char *line, char *header_name)
{
	char *res;
	if (strnicmp(line, header_name, strlen(header_name))) return NULL;
	res = line + strlen(header_name);
	while ((res[0] == ':') || (res[0] == ' ')) res+=1;
	return res;
}

void HTTP_SendRequest(FileDownload *fd)
{
	unsigned char range_buf[1024];
	char sHTTP[FD_BUFFER_SIZE];
	
	if (fd->cached_size) sprintf(range_buf, "Range: bytes=%d-\r\n", fd->cached_size);

	sprintf(sHTTP, "GET %s HTTP/1.0\r\n"
					"Host: %s\r\n"
					"Accept: */*\r\n"
					"%s"
					"User-Agent: %s\r\n"
					"\r\n", 
					fd->remote_path, fd->server_name,
					(fd->cached_size ? (const char *) range_buf: ""),
					FD_AGENT_NAME);

	fd->plug->error = SK_Send(fd->sock, sHTTP, strlen(sHTTP));
	if (fd->plug->error) {
		fd->plug->net_status = DL_Disconnected;
		SK_Delete(fd->sock);
		fd->sock = NULL;
	} else {
		fd->plug->net_status = DL_WaitingForAck;
	}
}

void HTTP_ProcessReply(FileDownload *fd)
{
	M4Err e;
	Bool is_ice;
	char sHTTP[2048];
	unsigned char buf[1024];
	unsigned char comp[400];
	unsigned char *new_location;
	char *hdr;
	s32 bytesRead, res;
	s32 LinePos, Pos;
	u32 rsp_code, BodyStart, ContentLength, first_byte, last_byte, total_size, range, no_range;

	bytesRead = res = 0;
	memset(sHTTP, 0, 2048);
	new_location = NULL;
	while (1) {
		e = SK_Receive(fd->sock, sHTTP, 2048, bytesRead, &res);
		switch (e) {
		case M4NetworkEmpty:
			if (!bytesRead) return;
			continue;
		/*socket has been closed while configuring, retry (not sure if the server got the GET)*/
		case M4ConnectionClosed:
			SK_Delete(fd->sock);
			fd->sock = NULL;
			fd->plug->net_status = DL_Setup;
			return;
		case M4OK:
			break;
		default:
			goto exit;
		}		
		if (!bytesRead) {
			bytesRead = res;
			continue;
		}
		bytesRead += res;

		/*locate body start*/
		BodyStart = SP_FindPattern(sHTTP, 0, bytesRead, "\r\n\r\n");
		if (!BodyStart) {
			BodyStart = SP_FindPattern(sHTTP, 0, bytesRead, "\n\n");
			if (!BodyStart) continue;
			BodyStart += 2;
		} else {
			BodyStart += 4;
		}
		break;
	}
	if (bytesRead < 0) {
		e = M4SignalingFailure;
		goto exit;
	}

	LinePos = SP_GetOneLine(sHTTP, 0, bytesRead, buf, 1024);
	Pos = SP_GetComponent(buf, 0, " \t\r\n", comp, 400);


	is_ice = 0;
	if (!strncmp("ICY", comp, 4)) {
		is_ice = 1;
		/*be prepared not to recieve any mime type from ShoutCast servers*/
		strcpy(fd->mime_type, "audio/mpeg");
	} else if ((strncmp("HTTP", comp, 4) != 0)) {
		e = M4SignalingFailure;
		goto exit;
	}
	Pos = SP_GetComponent(buf, Pos, " \t\r\n", comp, 400);
	if (Pos <= 0) {
		e = M4SignalingFailure;
		goto exit;
	}
	rsp_code = (u32) atoi(comp);	


	no_range = range = ContentLength = first_byte = last_byte = total_size = 0;
	//parse header
	while (1) {
		if ( (u32) LinePos + 4 > BodyStart) break;
		LinePos = SP_GetOneLine(sHTTP, LinePos , bytesRead, buf, 1024);
		if (LinePos < 0) break;

		if ((hdr = http_is_header(buf, "Content-Length")) ) {			
			ContentLength = (u32) atoi(hdr);
		}
		else if ((hdr = http_is_header(buf, "Content-Type")) ) {			
			strcpy(fd->mime_type, hdr);
			while (1) {
				char c = fd->mime_type[strlen(fd->mime_type)-1];
				if ((c=='\r') || (c=='\n')) {
					fd->mime_type[strlen(fd->mime_type)-1] = 0;
				} else {
					break;
				}
			}
			hdr = strchr(fd->mime_type, ';');
			if (hdr) hdr[0] = 0;
		}
		else if ((hdr = http_is_header(buf, "Content-Range")) ) {			
			range = 1;
			if (!strncmp(hdr, "bytes", 5)) {
				hdr += 5;
				if (*hdr == ':') hdr += 1;
				hdr += http_skip_space(hdr);
				if (*hdr == '*') {
					sscanf(hdr, "*/%d", &total_size);
				} else {
					sscanf(hdr, "%d-%d/%d", &first_byte, &last_byte, &total_size);
				}
			}
		}
		else if ((hdr = http_is_header(buf, "Accept-Ranges"))) {
			if (strstr(hdr, "none")) no_range = 1;
		}
		else if ((hdr = http_is_header(buf, "Location"))) {
			new_location = strdup(hdr);
		}
		else if (!strnicmp(buf, "ice", 3) || !strnicmp(buf, "icy", 3) ) is_ice = 1;
	}
	if (no_range) first_byte = 0;

	if (fd->cached_size) {
		if (total_size && fd->cached_size >= total_size) {
			rsp_code = 200;
			ContentLength = total_size;
		}
		if (ContentLength && fd->cached_size == ContentLength) rsp_code = 200;
	}	

	switch (rsp_code) {
	case 200:
	case 201:
	case 202:
	case 206:
		e = M4OK;
		break;
	/*redirection: extract the new location*/
	case 301:
	case 302:
		if (!new_location || !strlen(new_location) ) {
			e = M4URLNotFound;
			goto exit;
		}
		while (
			(new_location[strlen(new_location)-1] == '\n') 
			|| (new_location[strlen(new_location)-1] == '\r')  )
			new_location[strlen(new_location)-1] = 0;

		/*reset and reconnect*/
		SK_Delete(fd->sock);
		fd->sock = NULL;
		fd->plug->net_status = DL_Setup;
		if (!strnicmp(new_location, "http://", 7)) {
			sscanf(new_location, "http://%[^:/]:%hd", fd->server_name, &fd->session_port);
		}
		hdr = strstr(new_location, "://");
		hdr += 3;
		hdr = strstr(hdr, "/");
		strcpy(fd->remote_path, hdr);
		free(new_location);
		return;
	case 404:
	case 416:
		/*try without cache (some servers screw up when content-length is specified)*/
		if (fd->cached_size) {
			SK_Delete(fd->sock);
			fd->sock = NULL;
			fd->plug->net_status = DL_Setup;
			return;
		}
		e = M4URLNotFound;
		goto exit;
	case 503:
	default:
		e = M4RemotePeerError;
		goto exit;
	}

	if (!ContentLength && strstr(fd->mime_type, "ogg")) is_ice = 1;

	/*some servers may reply without content length, but we MUST have it*/
	if (!is_ice && !ContentLength) e = M4RemotePeerError;
	if (e) goto exit;

	/*force disabling cache (no content length)*/
	if (is_ice) {
		fd->use_cache = 0;
		if (!stricmp(fd->mime_type, "video/nsv")) strcpy(fd->mime_type, "audio/aac");
	}


	/*done*/
	if (fd->cached_size 
		&& ( (total_size && fd->cached_size >= total_size) || (ContentLength && fd->cached_size == ContentLength)) ) {
		fd->plug->total_size = fd->plug->bytes_done = fd->cached_size;
		/*disconnect*/
		fd->plug->net_status = DL_Disconnected;
		SK_Delete(fd->sock);
		fd->sock = NULL;
		BodyStart = bytesRead;
		fd->plug->error = M4EOF;
		FD_PostError(fd);
	}
	/*no range header, Accep-Ranges deny or dumb server : restart*/
	else if (!range || !first_byte || (first_byte != fd->cached_size) ) {
		fd->cached_size = fd->plug->bytes_done = 0;
		fd->plug->total_size = ContentLength;
		if (fd->use_cache) {
			fd->cache = fopen(fd->cache_name, "wb");
			if (!fd->cache) {
				e = M4IOErr;
				goto exit;
			}
		}
		fd->plug->net_status = DL_Running;
	}
	/*resume*/
	else {
		fd->plug->total_size = ContentLength + fd->cached_size;
		if (fd->use_cache) {
			fd->cache = fopen(fd->cache_name, "ab");
			if (!fd->cache) {
				e = M4IOErr;
				goto exit;
			}
		}
		fd->plug->net_status = DL_Running;
		fd->plug->bytes_done = fd->cached_size;
	}

	fd->start_time = M4_GetSysClock();
	
	//we may have existing data in this buffer ...
	if (!e && (BodyStart < (u32) bytesRead)) {
		http_data_recieved(fd, sHTTP+BodyStart, bytesRead - BodyStart);
		if (!fd->plug->OnData && !fd->use_cache) {
			if (fd->init_data) free(fd->init_data);
			fd->init_data_size = bytesRead - BodyStart;
			fd->init_data = malloc(sizeof(char) * fd->init_data_size);
			memcpy(fd->init_data, sHTTP+BodyStart, fd->init_data_size);
		}
	}

exit:
	if (e) {
		fd->plug->net_status = DL_Disconnected;
		fd->plug->error = e;
		SK_Delete(fd->sock);
		fd->sock = NULL;
		FD_PostError(fd);
	}
}

void HTTP_Read(FileDownload *fd)
{
	M4Err e;
	char buffer[FD_BUFFER_SIZE];
	u32 size = 0;

	while (1) {
		e = SK_Receive(fd->sock, buffer, FD_BUFFER_SIZE, 0, &size);
		if (!size || e == M4NetworkEmpty) return;

		if (e) {
			SK_Delete(fd->sock);
			fd->sock = NULL;
			fd->plug->net_status = DL_Disconnected;
			fd->plug->error = e;
			FD_PostError(fd);
			return;
		}
		http_data_recieved(fd, buffer, size);
		/*socket empty*/
		if (size < FD_BUFFER_SIZE) return;
	}
}


void HTTP_Task(FileDownload *fd)
{
	switch (fd->plug->net_status) {
	/*setup download*/
	case DL_Setup:
		FD_ConnectTask(fd);
		break;
	case DL_Connected:
		HTTP_SendRequest(fd);
		break;
	case DL_WaitingForAck:
		HTTP_ProcessReply(fd);
		break;
	case DL_Running:
		if (fd->plug->OnData) {
			HTTP_Read(fd);
		} else {
			Sleep(1000);
		}
		break;
	}
}

void HTTP_FetchMimeType(FileDownload *fd)
{
	while (1) {
		switch (fd->plug->net_status) {
		/*setup download*/
		case DL_Setup:
			FD_ConnectTask(fd);
			break;
		case DL_Connected:
			HTTP_SendRequest(fd);
			break;
		case DL_WaitingForAck:
			Sleep(20);
			HTTP_ProcessReply(fd);
			break;
		case DL_Running:
		case DL_Disconnected:
		case DL_Unavailable:
			return;
		}
	}
}

M4Err HTTP_ReadData(FileDownload *fd, char *buffer, u32 buffer_size, u32 *read_size)
{
	M4Err e;
	u32 size = 0;

	if (fd->init_data) {
		if (buffer_size>fd->init_data_size) {
			memcpy(buffer, fd->init_data, sizeof(char)*fd->init_data_size);
			fd->init_data = NULL;
			(*read_size) = fd->init_data_size;
		} else {
			char *d;
			memcpy(buffer, fd->init_data, sizeof(char)*buffer_size);
			fd->init_data_size -= buffer_size;
			d = malloc(sizeof(char) * fd->init_data_size);
			memcpy(d, fd->init_data+buffer_size, sizeof(char)*fd->init_data_size);
			free(fd->init_data);
			fd->init_data = d;
			(*read_size) = buffer_size;
		}
		return M4OK;
	}

	e = SK_Receive(fd->sock, buffer, buffer_size, 0, &size);
	if (!size || e == M4NetworkEmpty) return M4NetworkEmpty;
	else if (e) return M4OK;
	/*update stats*/
	http_data_recieved(fd, buffer, size);
	(*read_size) = size;
	return M4OK;
}
