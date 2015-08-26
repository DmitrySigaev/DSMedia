/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / MP4 reader plugin
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


#include "mp4_io.h"


void mp4_on_status(void *cbk)
{
	M4Err e;
	u32 bytes, net_status;
	M4Reader *read = (M4Reader *) cbk;
	e = NM_GetDownloaderStats(read->dnload, NULL, &bytes, NULL, &net_status);

	if (e<0) {
		/*error opening service*/
		if (!read->mov) NM_OnConnect(read->service, NULL, e);
		return;
	}
	if (net_status==DL_WaitingForAck) {
		NM_OnMessage(read->service, M4OK, "Connecting...");
		return;
	}
	if (net_status==DL_Connected) {
		if (!bytes) NM_OnMessage(read->service, M4OK, "Connected");
		return;
	}
	if (e==M4EOF) {
		const char *local_name;
		if (read->mov) return;
		local_name = NM_GetCacheFileName(read->dnload);
		if (!local_name) {
			NM_OnConnect(read->service, NULL, M4ServiceError);
			return;
		}
		e = M4OK;
		read->mov = M4_MovieOpen(local_name, M4_OPEN_READ);
		if (!read->mov) e = M4_GetLastError(NULL);
		else read->time_scale = M4_GetTimeScale(read->mov);
		NM_OnConnect(read->service, NULL, M4OK);
	}
}

void mp4_on_data(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	char sMsg[1024];
	Float perc, bytes_per_sec;
	const char *local_name;
	u32 total_size, bytes_done, net_status;
	M4Reader *read = (M4Reader *) cbk;

	e = NM_GetDownloaderStats(read->dnload, &total_size, &bytes_done, &bytes_per_sec, &net_status);

	perc = (Float) (100 * bytes_done) / (Float) total_size;
	sprintf(sMsg, "Download %.2f %% (%.2f kBps)", perc, bytes_per_sec/1024);
	NM_OnMessage(read->service, M4OK, sMsg);

	/*service is opened, nothing to do*/
	if (read->mov) return;

	/*try to open the service*/
	local_name = NM_GetCacheFileName(read->dnload);
	if (!local_name) {
		NM_OnConnect(read->service, NULL, M4ServiceError);
		return;
	}

	/*not enogh data yet*/
	if (read->missing_bytes && read->missing_bytes>data_size) {
		read->missing_bytes -= data_size;
		return;
	}
	
	e = M4_MovieOpenEx(local_name, &read->mov, &read->missing_bytes);
	switch (e) {
	case M4UncompleteFile:
		return;
	case M4OK:
		break;
	default:
		NM_OnConnect(read->service, NULL, e);
		return;
	}
	
	/*ok let's go*/
	read->time_scale = M4_GetTimeScale(read->mov);
	NM_OnConnect(read->service, NULL, M4OK);
}


void MP4_SetupDownload(NetClientPlugin *plug, const char *url)
{
	M4Err e;
	M4Reader *read = (M4Reader *) plug->priv;
	e = NM_FetchFile(read->service, url, 0, mp4_on_status, mp4_on_data, read, &read->dnload);
	if (e) NM_OnConnect(read->service, NULL, e);
	/*service confirm is done once IOD can be fetched*/
}


