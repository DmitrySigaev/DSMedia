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


#ifndef __FILE_DNLOAD_H
#define __FILE_DNLOAD_H


#include <gpac/m4_network.h>

#define FD_AGENT_NAME		"GPAC " M4_VERSION "File Downloader"
#define FD_BUFFER_SIZE		8192
#define FTP_DEFAULT_PORT	21
#define HTTP_DEFAULT_PORT	80

#define MAX_URI_LENGTH		4096

/*session types*/
enum
{
	FD_HTTP,
	FD_FTP,
};


typedef struct
{
	NetDownloader *plug;

	M4Thread *service_thread;
	Bool thread_dead;

	u32 sleep_time;
	Bool disable_cache, use_cache;

	char server_name[MAX_URI_LENGTH];
	char remote_path[MAX_URI_LENGTH];
	char cache_name[MAX_URI_LENGTH];
	u16 session_port;
	u32 session_type;
	u32 cached_size;
	M4Socket *sock;
	u32 num_connect_retry;

	FILE *cache;
	u32 start_time;

	char *init_data;
	u32 init_data_size;

	char mime_type[MAX_URI_LENGTH];
} FileDownload;


/*common connection task for all download session*/
void FD_ConnectTask(FileDownload *sess);
void FD_PostError(FileDownload *fd);


void HTTP_Task(FileDownload *sess);
M4Err HTTP_ReadData(FileDownload *fd, char *buffer, u32 buffer_size, u32 *read_size);
void HTTP_FetchMimeType(FileDownload *fd);

#endif

