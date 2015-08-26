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

static Bool HTTP_RandInit = 1;

#define HTTP_WAIT_SEC		30

#define HTTP_RSP_OK	"HTTP/1.0 200 OK"


void RTSP_GenerateHTTPCookie(RTSPSession *sess)
{
	u32 i, num, temp;

	if (HTTP_RandInit) {
		M4_RandInit(0);
		HTTP_RandInit = 0;
	}
	if (!sess->CookieRadLen) {
		strcpy(sess->HTTP_Cookie, "MPEG4M4");
		sess->CookieRadLen = 8;
	}
	num = M4_Rand();
	for (i=0; i < 8; i++) {
		temp = (num >> (i * 4)) & 0x0f;
		sess->HTTP_Cookie[sess->CookieRadLen + i] = (u8) temp + sess->HTTP_Cookie[0];
	}
	sess->HTTP_Cookie[sess->CookieRadLen + i] = 0;	
}




//http tunnelling start.
M4Err RTSP_HTTPStart(RTSPSession *sess, char *UserAgent)
{
	M4Err e;
	u32 size;
	s32 pos;
	char buffer[RTSP_DEFAULT_BUFFER];

	RTSP_GenerateHTTPCookie(sess);

	//  1. send "GET /sample.mov HTTP/1.0\r\n ..."
	memset(buffer, 0, RTSP_DEFAULT_BUFFER);
	pos = 0;
	pos += sprintf(buffer + pos, "GET /%s HTTP/1.0\r\n", sess->Service);
	pos += sprintf(buffer + pos, "User-Agent: %s\r\n", UserAgent);
	pos += sprintf(buffer + pos, "x-sessioncookie: %s\r\n", sess->HTTP_Cookie);
	pos += sprintf(buffer + pos, "Accept: application/x-rtsp-tunnelled\r\n" );
	pos += sprintf(buffer + pos, "Pragma: no-cache\r\n" );
	pos += sprintf(buffer + pos, "Cache-Control: no-cache\r\n\r\n" );	
	
	//	send it!
	e = SK_SendWait(sess->connection, buffer, strlen(buffer), HTTP_WAIT_SEC);
	if (e) return e;
	
	//	2. wait for "HTTP/1.0 200 OK"
	e = SK_ReceiveWait(sess->connection, buffer, RTSP_DEFAULT_BUFFER, 0, &size, HTTP_WAIT_SEC);
	if (e) return e;
	
	//get HTTP/1.0 200 OK
	if (strncmp(buffer, HTTP_RSP_OK, strlen(HTTP_RSP_OK))) return M4SignalingFailure;


	//	3. send "POST /sample.mov HTTP/1.0\r\n ..."
	sess->http = NewSocket(SK_TYPE_TCP);
	if (!sess->http ) return M4NetworkUnreachable;
	if (SK_Connect(sess->http, sess->Server, sess->Port)) return M4ConnectionFailed; 

	memset(buffer, 0, RTSP_DEFAULT_BUFFER);
	pos = 0;
	pos += sprintf(buffer + pos, "POST /%s HTTP/1.0\r\n", sess->Service);
	pos += sprintf(buffer + pos, "User-Agent: %s\r\n", UserAgent);
	pos += sprintf(buffer + pos, "x-sessioncookie: %s\r\n", sess->HTTP_Cookie);
	pos += sprintf(buffer + pos, "Accept: application/x-rtsp-tunnelled\r\n");
	pos += sprintf(buffer + pos, "Pragma: no-cache\r\n");
	pos += sprintf(buffer + pos, "Cache-Control: no-cache\r\n");	
	pos += sprintf(buffer + pos, "Content-Length: 32767\r\n");
	pos += sprintf(buffer + pos, "Expires: Sun. 9 Jan 1972 00:00:00 GMT\r\n\r\n");
 
	//	send it!
	e = SK_SendWait(sess->http, buffer, strlen(buffer), HTTP_WAIT_SEC);
	
	return e;
}
