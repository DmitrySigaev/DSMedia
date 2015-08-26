/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / RTP input plugin
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

#include "rtp_in.h"


void RP_SDPFromData(RTPClient *rtp, char *s_url, RTPStream *stream)
{
	char *url;
	char buf[2000];
	u32 size;

	url = strstr(s_url, ",");
	if (!url) {
		NM_OnConnect(rtp->service, NULL, M4InvalidURL);
		return;
	}
	url += 1;
	if (strstr(url, ";base64")) {
		//decode
		size = Base64_dec(url, strlen(url), buf, 2000);
		buf[size] = 0;
		url = buf;
	}
	RP_LoadSDP(rtp, url, strlen(url), stream);
}

void RP_SDPFromFile(RTPClient *rtp, char *file_name, RTPStream *stream)
{
	FILE *_sdp;
	char *sdp_buf;
	u32 sdp_size;

	sdp_buf = NULL;

	if (file_name && strstr(file_name, "file://")) file_name += strlen("file://");
	if (!file_name || !(_sdp = fopen(file_name, "rt")) ) {
		NM_OnConnect(rtp->service, NULL, M4InvalidURL);
		return;
	}

	fseek(_sdp, 0, SEEK_END);
	sdp_size = ftell(_sdp);
	fseek(_sdp, 0, SEEK_SET);
	sdp_buf = malloc(sdp_size);
	fread(sdp_buf, sdp_size, 1, _sdp);
	RP_LoadSDP(rtp, sdp_buf, sdp_size, stream);

	fclose(_sdp);
	free(sdp_buf);
}

void SDP_OnStatus(void *cbk)
{
}

void SDP_OnData(void *cbk, char *data, u32 data_size)
{
	M4Err e;
	RTPClient *rtp = (RTPClient *)cbk;
	u32 net_status;
	SDPFetch *sdp = rtp->sdp_temp;

	e = NM_GetDownloaderStats(rtp->dnload, NULL, NULL, NULL, &net_status);
	if (e== M4OK) return;
	else if (e == M4EOF) {
		const char *szFile = NM_GetCacheFileName(rtp->dnload);
		if (!szFile) {
			e = M4ServiceError;
		} else {
			e = M4OK;
			RP_SDPFromFile(rtp, (char *) szFile, sdp->chan);
			free(sdp->remote_url);
			free(sdp);
			rtp->sdp_temp = NULL;
			return;
		}
	}
	/*error*/
	if (sdp->chan) {
		NM_OnConnect(rtp->service, sdp->chan->channel, e);
	} else {
		NM_OnConnect(rtp->service, NULL, e);
		rtp->sdp_temp = NULL;
	}
	free(sdp->remote_url);
	free(sdp);
}

void RP_FetchSDP(NetClientPlugin *plug, char *url, RTPStream *stream)
{
	M4Err e;
	SDPFetch *sdp;
	RTPClient *rtp = (RTPClient *) plug->priv;
	/*if local URL get file*/
	if (strstr(url, "data:application/sdp")) {
		RP_SDPFromData(rtp, url, stream);
		return;
	}
	if (!strnicmp(url, "file://", 7) || !strstr(url, "://")) {
		RP_SDPFromFile(rtp, url, stream);
		return;
	}
	
	sdp = malloc(sizeof(SDPFetch));
	memset(sdp, 0, sizeof(SDPFetch));
	sdp->client = rtp;
	sdp->remote_url = strdup(url);
	sdp->chan = stream;

	/*otherwise setup download*/
	if (rtp->dnload) NM_CloseDownloader(rtp->dnload);
	rtp->dnload = NULL;

	rtp->sdp_temp = sdp;
	e = NM_FetchFile(rtp->service, url, 0, SDP_OnStatus, SDP_OnData, rtp, &rtp->dnload);
	if (e) NM_OnConnect(rtp->service, NULL, e);
	/*service confirm is done once fetched*/
}

