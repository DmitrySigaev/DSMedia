/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / IETF RTP/RTSP/SDP sub-project
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

#include <gpac/intern/m4_ietf_dev.h>

static u32 SessionID_RandInit = 0;


RTSPSession *RTSPS_NewSession(M4Socket *rtsp_listener)
{
	RTSPSession *sess;
	M4Socket *new_conn;
	M4Err e;
	u32 fam;
	u16 port;
	char name[MAX_IP_NAME_LEN];

	if (!rtsp_listener) return NULL;
	
	
	e = SK_Accept(rtsp_listener, &new_conn);
	if (!new_conn || e) return NULL;

	e = SK_GetLocalInfo(new_conn, &port, &fam);
	if (e) {
		SK_Delete(new_conn);
		return NULL;
	}
	e = SK_SetBlockingMode(new_conn, 1);
	if (e) {
		SK_Delete(new_conn);
		return NULL;
	}
	e = SK_SetServerMode(new_conn, 1);
	if (e) {
		SK_Delete(new_conn);
		return NULL;
	}
	
	//OK create a new session
	sess = malloc(sizeof(RTSPSession));
	memset(sess, 0, sizeof(RTSPSession));

	sess->connection = new_conn;
	sess->Port = port;
	sess->ConnectionType = fam;
	SK_GetHostName(name);
	sess->Server = strdup(name);
	
	sess->TCPChannels = NewChain();
	return sess;
}


M4Err RTSP_LoadServiceName(RTSPSession *sess, char *URL)
{
	char server[1024], service[1024];
	M4Err e;
	u16 Port;
	Bool UseTCP;
	u32 type;

	if (!sess || !URL) return M4BadParam;
	e = RTSP_UnpackURL(URL, server, &Port, service, &UseTCP);
	if (e) return e;

	type = UseTCP ? SK_TYPE_TCP : SK_TYPE_UDP;
	//check the network type matches, otherwise deny client
	if (sess->ConnectionType != type) return M4ServiceNotFound;
	if (sess->Port != Port) return M4ServiceNotFound;

	//ok
	sess->Server = strdup(server);
	sess->Service = strdup(service);
	return M4OK;
}


M4Err RTSP_AssignSessionID(RTSPSession *sess, char *your_custom_id)
{
	u32 one, two;
	u64 res;
	char buffer[30];

	if (!sess) return M4BadParam;
	if (sess->SessionID) free(sess->SessionID);
	sess->SessionID = NULL;

	if (your_custom_id) {
		sess->SessionID = strdup(your_custom_id);
	} else {
		if (!SessionID_RandInit) {
			SessionID_RandInit = 1;
			M4_RandInit(0);
		}
		one = M4_Rand();
		//try to be as random as possible. if we had some global stats that'd be better
		two = (u32) sess + sess->CurrentPos + sess->CurrentSize;
		res = one;
		res <<= 32;
		res += two;
		sprintf(buffer, "%llu", res);
		sess->SessionID = strdup(buffer);
	}
	return M4OK;
}


M4Err RTSP_GetSessionIP(RTSPSession *sess, char *buffer)
{
	if (!sess || !sess->connection) return M4BadParam;
	SK_GetSocketIP(sess->connection, buffer);
	return M4OK;
}


u8 RTSP_GetNextInterleavedID(RTSPSession *sess)
{
	u32 i;
	u8 id;
	TCPChan *ch;

	id = 0;
	for (i=0; i<ChainGetCount(sess->TCPChannels); i++) {
		ch = ChainGetEntry(sess->TCPChannels, i);
		if (ch->rtpID >= id) id = ch->rtpID + 1;
		if (ch->rtcpID >= id) id = ch->rtcpID + 1;
	}
	return id;
}


M4Err RTSP_GetRemoteAddress(RTSPSession *sess, char *buf)
{
	if (!sess || !sess->connection) return M4BadParam;
	return SK_GetRemoteAddress(sess->connection, buf);
}
