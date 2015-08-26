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

#ifdef _WIN32_WCE
#define RTSP_TRACE		0
#else
#define RTSP_TRACE		1
#endif


M4Err RTSP_UnpackURL(char *sURL, char *Server, u16 *Port, char *Service, Bool *useTCP)
{
	char schema[10], *test, text[1024], *retest;
	u32 i;
	if (!sURL) return M4BadParam;

	strcpy(Server, "");
	strcpy(Service, "");
	*Port = *useTCP =0;
	
	//extract the schema
	i = 0;
	while (i<=strlen(sURL)) {
		if (sURL[i] == ':') goto found;
		schema[i] = sURL[i];
		i += 1;
	}
	return M4BadParam;

found:
	schema[i] = 0;
	if (stricmp(schema, "rtsp") && stricmp(schema, "rtspu")) return M4InvalidURL; 
	//check for user/pass - not allowed
/*
	test = strstr(sURL, "@");
	if (test) return M4NotSupported;
*/
	test = strstr(sURL, "://");
	if (!test) return M4InvalidURL;
	test += 3;
	//check for service
	retest = strstr(test, "/");
	if (!retest) return M4InvalidURL;

	if (!stricmp(schema, "rtsp")) *useTCP = 1;

	//check for port
	retest = strstr(test, ":");
	if (retest && strstr(retest, "/")) {
		retest += 1;
		i=0;
		while (i<strlen(retest)) {
			if (retest[i] == '/') break;
			text[i] = retest[i];
			i += 1;
		}
		text[i] = 0;
		*Port = atoi(text);
	}
	//get the server name
	strcpy(text, test);
	i=0;
	while (i<strlen(text)) {
		if ( (text[i] == '/') || (text[i] == ':') ) break;
		text[i] = test[i];
		i += 1;
	}
	text[i] = 0;
	strcpy(Server, text);

	while (test[i] != '/') i += 1;
	strcpy(Service, test+i+1);

	return M4OK;
}


//create a new RTSPSession from URL - DO NOT USE WITH SDP
RTSPSession *RTSP_NewSession(char *sURL, u16 DefaultPort)
{
	RTSPSession *sess;
	char server[1024], service[1024];
	M4Err e;
	u16 Port;
	Bool UseTCP;
	
	if (!sURL) return NULL;

	e = RTSP_UnpackURL(sURL, server, &Port, service, &UseTCP);
	if (e) return NULL;

	sess = malloc(sizeof(RTSPSession));
	memset(sess, 0, sizeof(RTSPSession));

	sess->ConnectionType = UseTCP ? SK_TYPE_TCP : SK_TYPE_UDP;
	if (Port) sess->Port = Port;
	else if (DefaultPort) sess->Port = DefaultPort;
	else sess->Port = 554;

	//HTTP tunnel
	if (sess->Port == 80) {
		sess->ConnectionType = SK_TYPE_TCP;
		sess->HasTunnel = 1;
	}	

	sess->Server = strdup(server);
	sess->Service = strdup(service);
	sess->mx = NewMutex();
	sess->TCPChannels = NewChain();
	RTSP_ResetSession(sess, 0);
	return sess;
}


void RTSP_ResetAggregation(LPRTSPSESSION sess)
{
	if (!sess) return;
	
	MX_P(sess->mx);
	if (sess->RTSP_State == RSM_WaitForControl) {
		strcpy(sess->RTSPLastRequest, "RESET");
		//skip all we haven't recieved
		sess->CSeq += sess->NbPending;
		sess->NbPending = 0;
	}
	sess->RTSP_State = RSM_Init;
	MX_V(sess->mx);
}

void RTSP_AcknowledgeError(LPRTSPSESSION sess)
{
}

void RemoveTCPChannels(RTSPSession *sess)
{
	TCPChan *ch;
	while (ChainGetCount(sess->TCPChannels)) {
		ch = ChainGetEntry(sess->TCPChannels, 0);
		free(ch);
		ChainDeleteEntry(sess->TCPChannels, 0);
	}
}


void RTSP_ResetSession(RTSPSession *sess, Bool ResetConnection)
{
	MX_P(sess->mx);
	if (sess->SessionID) free(sess->SessionID);
	sess->SessionID = NULL;
	sess->NeedConnection = 1;

	if (ResetConnection) {
		if (sess->connection) SK_Delete(sess->connection);
		sess->connection = NULL;
		if (sess->http) {
			SK_Delete(sess->http);
			sess->http = NULL;
		}
	}
	
	sess->RTSP_State = RSM_Init;
	sess->CSeq = sess->NbPending = 0;
	sess->InterID = -1;
	sess->pck_start = sess->payloadSize = 0;
	sess->CurrentPos = sess->CurrentSize = 0;
	strcpy(sess->RTSPLastRequest, "");
	RemoveTCPChannels(sess);
	MX_V(sess->mx);
}

void RTSP_DeleteSession(RTSPSession *sess)
{
	if (!sess) return;

	RTSP_ResetSession(sess, 0);

	if (sess->connection) SK_Delete(sess->connection);
	if (sess->http) SK_Delete(sess->http);
	if (sess->Server) free(sess->Server);
	if (sess->Service) free(sess->Service);
	DeleteChain(sess->TCPChannels);
	if (sess->rtsp_pck_buf) free(sess->rtsp_pck_buf);
	MX_Delete(sess->mx);
	free(sess);
}

u32 RTSP_GetSessionState(RTSPSession *sess)
{
	u32 state;
	if (!sess) return RSM_Invalidated;
	
	MX_P(sess->mx);
	state = sess->RTSP_State;
	MX_V(sess->mx);
	return state;
}

char *RTSP_GetLastRequest(RTSPSession *sess)
{
	char *ret;
	if (!sess) return NULL;
	MX_P(sess->mx);
	ret = sess->RTSPLastRequest;
	MX_V(sess->mx);
	return ret;
}


//check whether the url contains server and service name
//no thread protection as this is const throughout the session
u32 RTSP_IsMySession(RTSPSession *sess, char *url)
{
	if (!sess) return 0;
	if (!strstr(url, sess->Server)) return 0;
	//same url or sub-url
	if (strstr(url, sess->Service)) return 1;
	return 0;
}

char *RTSP_GetSessionID(RTSPSession *sess)
{
	char *sessID;
	if (!sess) return NULL;
	MX_P(sess->mx);
	sessID = sess->SessionID;
	MX_V(sess->mx);
	return sessID;
}

char *RTSP_GetServerName(RTSPSession *sess)
{
	if (!sess) return NULL;
	return sess->Server;
}

char *RTSP_GetServiceName(RTSPSession *sess)
{
	if (!sess) return NULL;
	return sess->Service;
}

u16 RTSP_GetSessionPort(RTSPSession *sess)
{
	return (sess ? sess->Port : 0);
}

M4Err RTSP_CheckConnection(RTSPSession *sess)
{
	M4Err e;
	//active, return
	if (!sess->NeedConnection) return M4OK;

	//socket is destroyed, recreate
	if (!sess->connection) {
		sess->connection = NewSocket(sess->ConnectionType);
		if (!sess->connection) return M4OutOfMem;
		if (sess->SockBufferSize) SK_SetBufferSize(sess->connection, 0, sess->SockBufferSize);
	}
	//the session is down, reconnect
	e = SK_Connect(sess->connection, sess->Server, sess->Port);
	if (e) return e;

	if (!sess->http && sess->HasTunnel) {
		e = RTSP_HTTPStart(sess, "toto is the king of RTSP");
		if (e) return e;
	}
	sess->NeedConnection = 0;
	return M4OK;
}


M4Err RTSP_Send(RTSPSession *sess, unsigned char *buffer, u32 Size)
{
	M4Err e;
	u32 Size64;

	e = RTSP_CheckConnection(sess);
	if (e) return e;

	//RTSP requests on HTTP are base 64 encoded
	if (sess->HasTunnel) {
		char buf64[3000];
		Size64 = Base64_enc(buffer, Size, buf64, 3000);
		buf64[Size64] = 0;
		//send on http connection
		return SK_SendWait(sess->http, buf64, Size64, 30);
	} else {
		return SK_Send(sess->connection, buffer, Size);
	}
}



static TCPChan *GetTCPChannel(RTSPSession *sess, u8 rtpID, u8 rtcpID, Bool RemoveIt)
{
	TCPChan *ptr;
	u32 i;
	for (i=0; i<ChainGetCount(sess->TCPChannels); i++) {
		ptr = ChainGetEntry(sess->TCPChannels, i);
		if (ptr->rtpID == rtpID) goto exit;;
		if (ptr->rtcpID == rtcpID) goto exit;
	}
	return NULL;
exit:
	if (RemoveIt) ChainDeleteEntry(sess->TCPChannels, i);
	return ptr;
}


M4Err RTSP_DeInterleave(RTSPSession *sess)
{
	TCPChan *ch;
	Bool IsRTCP;
	u8 InterID;
	u16 paySize;
	u32 res, Size;
	char *buffer;

	if (!sess) return M4ServiceError;

	Size = sess->CurrentSize - sess->CurrentPos;
	buffer = sess->TCPBuffer + sess->CurrentPos;

	if (!Size) return M4NetworkEmpty;

	//we do not work with just a header -> force a refill
	if (Size <= 4) return RTSP_RefillBuffer(sess);

	//break if we get RTSP response on the wire
	if (!strncmp(buffer, "RTSP", 4)) 
		return M4NetworkEmpty;

	//new packet
	if (!sess->pck_start && (buffer[0] == '$')) {
		InterID = buffer[1];
		paySize = ((buffer[2] << 8) & 0xFF00) | (buffer[3] & 0xFF);
		/*this may be NULL (data fetched after a teardown) - resync and return*/
		ch = GetTCPChannel(sess, InterID, InterID, 0);

		/*then check wether this is a full packet or a split*/
		if (paySize <= Size-4) {
			if (ch) {
				IsRTCP = (ch->rtcpID == InterID);
				sess->RTSP_SignalData(sess, ch->ch_ptr, buffer+4, paySize, IsRTCP);
			}
			sess->CurrentPos += paySize+4;
			assert(sess->CurrentPos <= sess->CurrentSize);
		} else {
			/*missed end of pck ?*/
			if (sess->payloadSize) {
				ch = GetTCPChannel(sess, sess->InterID, sess->InterID, 0);
				if (ch) {
					IsRTCP = (ch->rtcpID == sess->InterID);
					sess->RTSP_SignalData(sess, ch->ch_ptr, sess->rtsp_pck_buf, sess->payloadSize, IsRTCP);
				}
			}
			sess->InterID = InterID;
			sess->payloadSize = paySize;
			sess->pck_start = Size-4;
			if (sess->rtsp_pck_size < paySize) {
				sess->rtsp_pck_buf = realloc(sess->rtsp_pck_buf, paySize);
				sess->rtsp_pck_size = paySize;
			}
			memcpy(sess->rtsp_pck_buf, buffer+4, Size-4);
			sess->CurrentPos += Size;
			assert(sess->CurrentPos <= sess->CurrentSize);
		}
	} 
	/*end of packet*/
	else if (sess->payloadSize - sess->pck_start <= Size) {
		if (!sess->pck_start && sess->rtsp_log) fprintf(sess->rtsp_log, "Missed begining of RTP packet %d\n", Size);

		res = sess->payloadSize - sess->pck_start;
		memcpy(sess->rtsp_pck_buf + sess->pck_start, buffer, res);
		//flush - same as above, don't complain if channel not found
		ch = GetTCPChannel(sess, sess->InterID, sess->InterID, 0);
		if (ch) {
			IsRTCP = (ch->rtcpID == sess->InterID);
			sess->RTSP_SignalData(sess, ch->ch_ptr, sess->rtsp_pck_buf, sess->payloadSize, IsRTCP);
		}
		sess->payloadSize = 0;
		sess->pck_start = 0;
		sess->InterID = -1;
		sess->CurrentPos += res;
		assert(sess->CurrentPos <= sess->CurrentSize);
	}
	/*middle of packet*/
	else {
		if (!sess->pck_start && sess->rtsp_log) fprintf(sess->rtsp_log, "Missed begining of RTP packet\n");
		memcpy(sess->rtsp_pck_buf + sess->pck_start, buffer, Size);
		sess->pck_start += Size;
		sess->CurrentPos += Size;
		assert(sess->CurrentPos <= sess->CurrentSize);
	}
	return M4OK;
}








/*
		Exposed API, thread-safe
*/

M4Err RTSP_ResetInterleaving(RTSPSession *sess, Bool ResetChannels)
{
	if (!sess) return M4BadParam;

	MX_P(sess->mx);
	sess->payloadSize = 0;
	sess->pck_start = 0;
	sess->InterID = -1;
	if (ResetChannels) RemoveTCPChannels(sess);
	MX_V(sess->mx);
	
	return M4OK;
}


M4Err RTSP_ReadTCP(RTSPSession *sess)
{
	M4Err e;
	if (!sess) return M4BadParam;

	MX_P(sess->mx);
	e = RTSP_FillTCPBuffer(sess);
	if (!e) {
		//only read if not RTSP
		while (1) {
			e = RTSP_DeInterleave(sess);
			if (e) break;
		}
	}
	MX_V(sess->mx);
	return e;
}


u32 RTSP_UnregisterTCPChannel(LPRTSPSESSION sess, u8 LowInterID)
{
	TCPChan *ptr;

	MX_P(sess->mx);
	ptr = GetTCPChannel(sess, LowInterID, LowInterID, 1);
	if (ptr) free(ptr);
	MX_V(sess->mx);
	return ChainGetCount(sess->TCPChannels);
}

M4Err RTSP_RegisterTCPChannel(RTSPSession *sess, void *the_ch, u8 LowInterID, u8 HighInterID)
{
	TCPChan *ptr;

	if (!sess) return M4BadParam;

	MX_P(sess->mx);
	//do NOT register twice
	ptr = GetTCPChannel(sess, LowInterID, HighInterID, 0);
	if (!ptr) {
		ptr = malloc(sizeof(TCPChan));
		ptr->ch_ptr = the_ch;
		ptr->rtpID = LowInterID;
		ptr->rtcpID = HighInterID;
		ChainAddEntry(sess->TCPChannels, ptr);
	}
	MX_V(sess->mx);
	return M4OK;
}


M4Err RTSP_SetCallbackOnInterleaving(RTSPSession *sess,
						M4Err (*SignalData)(RTSPSession *sess, void *chan, char *buffer, u32 bufferSize, Bool IsRTCP)
				)
{
	if (!sess) return M4BadParam;

	MX_P(sess->mx);

	//only if existing
	if (SignalData) sess->RTSP_SignalData = SignalData;

	//realloc or alloc
	if (sess->rtsp_pck_buf && sess->rtsp_pck_size != RTSP_PCK_SIZE) {
		sess->rtsp_pck_size = RTSP_PCK_SIZE;
		sess->rtsp_pck_buf = realloc(sess->rtsp_pck_buf, sess->rtsp_pck_size);
	} else if (!sess->rtsp_pck_buf) {
		sess->rtsp_pck_size = RTSP_PCK_SIZE;
		sess->rtsp_pck_buf = realloc(sess->rtsp_pck_buf, sess->rtsp_pck_size);
		sess->pck_start = 0;
	}
	MX_V(sess->mx);
	return M4OK;
}

M4Err RTSP_SetBufferSize(RTSPSession *sess, u32 BufferSize)
{
	if (!sess) return M4BadParam;
	sess->SockBufferSize = BufferSize;
	return M4OK;
}

void RTSP_SetLog(LPRTSPSESSION sess, FILE *log)
{
	if (sess) sess->rtsp_log = log;
}
