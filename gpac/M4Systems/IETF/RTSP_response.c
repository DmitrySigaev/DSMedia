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


RTSPResponse *RTSP_NewResponse()
{
	RTSPResponse *tmp = malloc(sizeof(RTSPResponse));
	memset(tmp, 0, sizeof(RTSPResponse));
	tmp->Transports = NewChain();
	tmp->RTP_Infos = NewChain();
	tmp->Xtensions = NewChain();
	return tmp;
}


#define RSP_FREE_CLEAN(hdr)		if (rsp->hdr) free(rsp->hdr);	\
								rsp->hdr = NULL;

void RTSP_ResetResponse(RTSPResponse *rsp)
{
	RTP_Info *inf;
	RTSPTransport *trans;
	X_Attribute *att;
	if (!rsp) return;

	//free all headers
	RSP_FREE_CLEAN(Accept);
	RSP_FREE_CLEAN(Accept_Encoding);
	RSP_FREE_CLEAN(Accept_Language);
	RSP_FREE_CLEAN(Allow);
	RSP_FREE_CLEAN(Authorization);
	RSP_FREE_CLEAN(Cache_Control);
	RSP_FREE_CLEAN(Conference);
	RSP_FREE_CLEAN(Connection);
	RSP_FREE_CLEAN(Content_Base);
	RSP_FREE_CLEAN(Content_Encoding);
	RSP_FREE_CLEAN(Content_Language);
	RSP_FREE_CLEAN(Content_Location);
	RSP_FREE_CLEAN(Content_Type);
	RSP_FREE_CLEAN(Date);
	RSP_FREE_CLEAN(Expires);
	RSP_FREE_CLEAN(From);
	RSP_FREE_CLEAN(Host);
	RSP_FREE_CLEAN(If_Match);
	RSP_FREE_CLEAN(If_Modified_Since);
	RSP_FREE_CLEAN(Last_Modified);
	RSP_FREE_CLEAN(Location);
	RSP_FREE_CLEAN(Proxy_Authenticate);
	RSP_FREE_CLEAN(Proxy_Require);
	RSP_FREE_CLEAN(Public);
	RSP_FREE_CLEAN(Referer);
	RSP_FREE_CLEAN(Require);
	RSP_FREE_CLEAN(Retry_After);
	RSP_FREE_CLEAN(Server);
	RSP_FREE_CLEAN(Session);
	RSP_FREE_CLEAN(Timestamp);
	RSP_FREE_CLEAN(Unsupported);
	RSP_FREE_CLEAN(User_Agent);
	RSP_FREE_CLEAN(Vary);
	RSP_FREE_CLEAN(Via);
	RSP_FREE_CLEAN(WWW_Authenticate);

	//this is for us
	RSP_FREE_CLEAN(ResponseInfo);
	RSP_FREE_CLEAN(body);

	rsp->Bandwidth = rsp->Blocksize = rsp->ResponseCode = rsp->Content_Length = rsp->CSeq = 0;
	rsp->Scale = rsp->Speed = 0.0;
	if (rsp->Range) free(rsp->Range);
	rsp->Range = NULL;
	
	rsp->SessionTimeOut = 0;

	while (ChainGetCount(rsp->Transports)) {
		trans = ChainGetEntry(rsp->Transports, 0);
		ChainDeleteEntry(rsp->Transports, 0);
		RTSP_DeleteTransport(trans);
	}

	while (ChainGetCount(rsp->RTP_Infos)) {
		inf = ChainGetEntry(rsp->RTP_Infos, 0);
		ChainDeleteEntry(rsp->RTP_Infos, 0);
		if (inf->url) free(inf->url);
		free(inf);
	}
	while (ChainGetCount(rsp->Xtensions)) {
		att = ChainGetEntry(rsp->Xtensions, 0);
		ChainDeleteEntry(rsp->Xtensions, 0);
		free(att->Name);
		free(att->Value);
		free(att);
	}
}

void RTSP_DeleteResponse(RTSPResponse *rsp)
{
	if (!rsp) return;
	
	RTSP_ResetResponse(rsp);
	DeleteChain(rsp->RTP_Infos);
	DeleteChain(rsp->Xtensions);
	DeleteChain(rsp->Transports);
	free(rsp);
}



RTSPRange *RTSP_ParseRange(char *range_buf)
{
	RTSPRange *rg;
	u32 i;

	//only support for NPT
	if (!strstr(range_buf, "npt")) return NULL;
	
	rg = malloc(sizeof(RTSPRange));
	memset(rg, 0, sizeof(RTSPRange));
	i = sscanf(range_buf, "npt=%f-%f", &rg->start, &rg->end);
	if (i == 1) {
		rg->end = 0.0;
		sscanf(range_buf, "npt=%f-", &rg->start);
	}	
	return rg;
}

void RTSP_DeleteTransport(RTSPTransport *transp)
{
	if (!transp) return;
	if (transp->destination) free(transp->destination);
	if (transp->Profile) free(transp->Profile);
	if (transp->source) free(transp->source);
	free(transp);
}

RTSPTransport *RTSP_DuplicateTransport(RTSPTransport *original)
{
	RTSPTransport *tr;

	if (!original) return NULL;	
	
	tr = malloc(sizeof(RTSPTransport));
	memcpy(tr, original, sizeof(RTSPTransport));
	tr->destination = tr->source = tr->Profile = NULL;
	if (original->destination) tr->destination = strdup(original->destination);
	if (original->source) tr->source = strdup(original->source);
	if (original->Profile) tr->Profile = strdup(original->Profile);
	return tr;
}

RTSPRange *RTSP_NewRange()
{
	RTSPRange *tmp = malloc(sizeof(RTSPRange));
	memset(tmp, 0, sizeof(RTSPRange));
	return tmp;
}

void RTSP_DeleteRange(RTSPRange *range)
{
	if (!range) return;
	free(range);
}

void RTSP_SetResponseVal(RTSPResponse *rsp, char *Header, char *Value)
{
	char LineBuffer[400], buf[100], param_name[100], param_val[100];
	s32 LinePos, Pos, nPos, s_val;
	RTP_Info *info;
	RTSPTransport *trans;
	X_Attribute *x_Att;

	if (!stricmp(Header, "Accept")) rsp->Accept = strdup(Value);
	else if (!stricmp(Header, "Accept-Encoding")) rsp->Accept_Encoding = strdup(Value);
	else if (!stricmp(Header, "Accept-Language")) rsp->Accept_Language = strdup(Value);
	else if (!stricmp(Header, "Allow")) rsp->Allow = strdup(Value);
	else if (!stricmp(Header, "Authorization")) rsp->Authorization = strdup(Value);
	else if (!stricmp(Header, "Bandwidth")) sscanf(Value, "%d", &rsp->Bandwidth);
	else if (!stricmp(Header, "Blocksize")) sscanf(Value, "%d", &rsp->Blocksize);
	else if (!stricmp(Header, "Cache-Control")) rsp->Cache_Control = strdup(Value);
	else if (!stricmp(Header, "Conference")) rsp->Conference = strdup(Value);
	else if (!stricmp(Header, "Connection")) rsp->Connection = strdup(Value);
	else if (!stricmp(Header, "Content-Base")) rsp->Content_Base = strdup(Value);	
	else if (!stricmp(Header, "Content-Encoding")) rsp->Content_Encoding = strdup(Value);	
	else if (!stricmp(Header, "Content-Length")) sscanf(Value, "%d", &rsp->Content_Length);
	else if (!stricmp(Header, "Content-Language")) rsp->Content_Language = strdup(Value);	
	else if (!stricmp(Header, "Content-Location")) rsp->Content_Location = strdup(Value);	
	else if (!stricmp(Header, "Content-Type")) rsp->Content_Type = strdup(Value);	
	else if (!stricmp(Header, "CSeq")) sscanf(Value, "%d", &rsp->CSeq);
	else if (!stricmp(Header, "Date")) rsp->Date = strdup(Value);	
	else if (!stricmp(Header, "Expires")) rsp->Expires = strdup(Value);	
	else if (!stricmp(Header, "From")) rsp->From = strdup(Value);	
	else if (!stricmp(Header, "Host")) rsp->Host = strdup(Value);	
	else if (!stricmp(Header, "If-Match")) rsp->If_Match = strdup(Value);	
	else if (!stricmp(Header, "If-Modified-Since")) rsp->If_Modified_Since = strdup(Value);	
	else if (!stricmp(Header, "Last-Modified")) rsp->Last_Modified = strdup(Value);	
	else if (!stricmp(Header, "Location")) rsp->Location = strdup(Value);	
	else if (!stricmp(Header, "Proxy-Authenticate")) rsp->Proxy_Authenticate = strdup(Value);	
	else if (!stricmp(Header, "Proxy-Require")) rsp->Proxy_Require = strdup(Value);	
	else if (!stricmp(Header, "Public")) rsp->Public = strdup(Value);	
	else if (!stricmp(Header, "Referer")) rsp->Referer = strdup(Value);	
	else if (!stricmp(Header, "Require")) rsp->Require = strdup(Value);	
	else if (!stricmp(Header, "Retry-After")) rsp->Retry_After = strdup(Value);	
	else if (!stricmp(Header, "Scale")) sscanf(Value, "%f", &rsp->Scale);
	else if (!stricmp(Header, "Server")) rsp->Server = strdup(Value);	
	else if (!stricmp(Header, "Speed")) sscanf(Value, "%f", &rsp->Speed);
	else if (!stricmp(Header, "Timestamp")) rsp->Timestamp = strdup(Value);	
	else if (!stricmp(Header, "Unsupported")) rsp->Unsupported = strdup(Value);	
	else if (!stricmp(Header, "User-Agent")) rsp->User_Agent = strdup(Value);	
	else if (!stricmp(Header, "Vary")) rsp->Vary = strdup(Value);	
	else if (!stricmp(Header, "Via")) rsp->Vary = strdup(Value);	
	else if (!stricmp(Header, "WWW_Authenticate")) rsp->Vary = strdup(Value);	
	else if (!stricmp(Header, "Transport")) {
		LinePos = 0;
		while (1) {
			LinePos = SP_GetComponent(Value, LinePos, "\r\n", LineBuffer, 400);
			if (LinePos <= 0) return;
			trans = ParseTransport(Value);
			if (trans) ChainAddEntry(rsp->Transports, trans);
		}
	}
	//Session
	else if (!stricmp(Header, "Session")) {
		LinePos = SP_GetComponent(Value, 0, ";\r\n", LineBuffer, 400);
		rsp->Session = strdup(LineBuffer);
		//get timeout if any
		if (Value[LinePos] == ';') {
			LinePos += 1;
			LinePos = SP_GetComponent(Value, LinePos, ";\r\n", LineBuffer, 400);
			//default
			rsp->SessionTimeOut = 60;
			sscanf(LineBuffer, "timeout=%d", &rsp->SessionTimeOut);
		}
	}

	//Range
	else if (!stricmp(Header, "Range")) rsp->Range = RTSP_ParseRange(Value);
	//RTP-Info
	else if (!stricmp(Header, "RTP-Info")) {
		LinePos = 0;
		while (1) {
			LinePos = SP_GetComponent(Value, LinePos, ",\r\n", LineBuffer, 400);
			if (LinePos <= 0) return;

			info = malloc(sizeof(RTP_Info));
			memset(info, 0, sizeof(RTP_Info));

			Pos = 0;
			while (1) {	
				Pos = SP_GetComponent(LineBuffer, Pos, " ;", buf, 100);
				if (Pos <= 0) break;
				if (strstr(buf, "=")) {
					nPos = SP_GetComponent(buf, 0, "=", param_name, 100);
					nPos += 1;
					nPos = SP_GetComponent(buf, nPos, "", param_val, 100);
				} else {
					strcpy(param_name, buf);
				}
				if (!stricmp(param_name, "url")) info->url = strdup(param_val);
				else if (!stricmp(param_name, "seq")) sscanf(param_val, "%d", &info->seq);
				else if (!stricmp(param_name, "rtptime")) {
					sscanf(param_val, "%i", &s_val);
					info->rtp_time = (s_val>0) ? s_val : 0;
				}
				else if (!stricmp(param_name, "ssrc")) {
					sscanf(param_val, "%i", &s_val);
					info->ssrc = (s_val>0) ? s_val : 0;
				}
			}
			ChainAddEntry(rsp->RTP_Infos, info);
		}
	}
	//check for extended attributes
	else if (!strnicmp(Header, "x-", 2)) {
		x_Att = malloc(sizeof(X_Attribute));
		x_Att->Name = strdup(Header+2);
		x_Att->Value = NULL;
		if (Value && strlen(Value)) x_Att->Value = strdup(Value);
		ChainAddEntry(rsp->Xtensions, x_Att);
	}
	//unknown field - skip it
}



//parse all fields in the header
M4Err RTSP_ParseResponseHeader(RTSPSession *sess, RTSPResponse *rsp, u32 BodyStart)
{
	unsigned char LineBuffer[1024];
	unsigned char ValBuf[400];
	unsigned char *buffer;
	s32 Pos, ret;
	u32 Size;

	Size = sess->CurrentSize - sess->CurrentPos;
	buffer = sess->TCPBuffer + sess->CurrentPos;

	//parse first line
	ret = SP_GetOneLine(buffer, 0, Size, LineBuffer, 1024);
	if (ret < 0) return M4SignalingFailure;
	//RTSP/1.0
	Pos = SP_GetComponent(LineBuffer, 0, " \t\r\n", ValBuf, 400);
	if (Pos <= 0) return M4SignalingFailure;
	if (strcmp(ValBuf, RTSP_VERSION)) return M4ServiceError;
	//CODE
	Pos = SP_GetComponent(LineBuffer, Pos, " \t\r\n", ValBuf, 400);
	if (Pos <= 0) return M4SignalingFailure;
	rsp->ResponseCode = atoi(ValBuf);
	//string info
	Pos = SP_GetComponent(LineBuffer, Pos, "\t\r\n", ValBuf, 400);
	if (Pos > 0) rsp->ResponseInfo = strdup(ValBuf);

	return RTSP_ParseHeaderLines(buffer + ret, Size - ret, BodyStart, NULL, rsp);
}



u32 IsRTSPMessage(char *buffer)
{
	if (!buffer) return 0;
	if (buffer[0]=='$') return 0;

	if (!strncmp(buffer, "RTSP", 4)) return 1;
	if (!strncmp(buffer, "GET_PARAMETER", strlen("GET_PARAMETER"))) return 1;
	if (!strncmp(buffer, "ANNOUNCE", strlen("ANNOUNCE"))) return 1;
	if (!strncmp(buffer, "SET_PARAMETER", strlen("SET_PARAMETER"))) return 1;
	if (!strncmp(buffer, "REDIRECT", strlen("REDIRECT"))) return 1;
	if (!strncmp(buffer, "OPTIONS", strlen("OPTIONS"))) return 1;
	return 0;
}


M4Err RTSP_GetResponse(LPRTSPSESSION sess, RTSPResponse *rsp)
{
	M4Err e;
	u32 BodyStart, size;
	
	if (!sess || !rsp) return M4BadParam;
	RTSP_ResetResponse(rsp);


	//LOCK
	MX_P(sess->mx);


	e = RTSP_CheckConnection(sess);
	if (e) goto exit;

	//push data in our queue
	e = RTSP_FillTCPBuffer(sess);
	if (e) goto exit;

	//this is interleaved data
	if (!IsRTSPMessage(sess->TCPBuffer+sess->CurrentPos) ) {
		RTSP_ReadTCP(sess);
		e = M4NetworkEmpty;
		goto exit;
	}
	e = RTSP_ReadReply(sess);
	if (e) goto exit;

	//get the reply
	RTSP_GetBodyInfo(sess, &BodyStart, &size);
	e = RTSP_ParseResponseHeader(sess, rsp, BodyStart);

	//copy the body if any
	if (!e && rsp->Content_Length) {
		rsp->body = malloc(sizeof(char) * (rsp->Content_Length));
		memcpy(rsp->body, sess->TCPBuffer+sess->CurrentPos + BodyStart, rsp->Content_Length);
	}	

	if (sess->rtsp_log) fprintf(sess->rtsp_log, "\n/*RTSP Get Response*/\n\n%s\n", sess->TCPBuffer+sess->CurrentPos);
	
	//reset TCP buffer
	sess->CurrentPos += BodyStart + rsp->Content_Length;

	if (e) goto exit;
	
	//update RTSP aggreagation info
	if (sess->NbPending) sess->NbPending -= 1;

	if (sess->RTSP_State == RSM_Waiting) sess->RTSP_State = RSM_Init;
	//control, and everything is recieved
	else if (sess->RTSP_State == RSM_WaitForControl) {
		if (!sess->NbPending) sess->RTSP_State = RSM_Init;
	}
	//this is a late reply to an aggregated control - signal nothing
	if (!strcmp(sess->RTSPLastRequest, "RESET") && sess->CSeq > rsp->CSeq) {
		e = M4NetworkEmpty;
		goto exit;
	}

	//reset last request
	if (sess->RTSP_State == RSM_Init) strcpy(sess->RTSPLastRequest, "");

	//check the CSeq is in the right range. The server should ALWAYS reply in sequence
	//to an aggreagated sequence of requests
	//if we have reseted the connection (due to an APP error) return empty
	if (sess->CSeq > rsp->CSeq + sess->NbPending) {
		MX_V(sess->mx);
		return RTSP_GetResponse(sess, rsp);
	}

	if (sess->CSeq != rsp->CSeq + sess->NbPending) {
		e = M4SignalingFailure;
		goto exit;
	}

	//now extract / check sessionID if specified
	if (rsp->Session) {
		if (!sess->SessionID) 
			sess->SessionID = strdup(rsp->Session);
		else if (strcmp(sess->SessionID, rsp->Session)) {
			e = M4SignalingFailure;
			goto exit;
		}
	}
	//destroy sessionID if needed - real doesn't close the connection when destroying
	//session
	if (!strcmp(sess->RTSPLastRequest, RTSP_TEARDOWN)) {
		free(sess->SessionID);
		sess->SessionID = NULL;
	}

	if (rsp->Connection && !stricmp(rsp->Connection, "Close")) {
		RTSP_ResetSession(sess, 0);
		//destroy the socket
		if (sess->connection) SK_Delete(sess->connection);
		sess->connection = NULL;
		
		//destroy the http tunnel if any
		if (sess->HasTunnel && sess->http) {
			SK_Delete(sess->http);
			sess->http = NULL;
		}
	}	
	
exit:
	MX_V(sess->mx);
	return e;
}




M4Err RTSP_WriteResponse(RTSPSession *sess, RTSPResponse *rsp, 
						 unsigned char **out_buffer, u32 *out_size)
{
	u32 i, cur_pos, size;
	char *buffer, temp[50];
	RTSPTransport *trans;
	X_Attribute *att;
	RTP_Info *info;


	*out_buffer = NULL;

	size = RTSP_WRITE_STEPALLOC;
	buffer = malloc(size);
	cur_pos = 0;

	//RTSP line
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, RTSP_VERSION);
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, " ");
	RTSP_WRITE_INT(buffer, size, cur_pos, rsp->ResponseCode, 0);
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, " ");
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, RTSP_NC_TO_String(rsp->ResponseCode));
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");

	//all headers
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept", rsp->Accept);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept-Encoding", rsp->Accept_Encoding);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept-Language", rsp->Accept_Language);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Allow", rsp->Allow);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Authorization", rsp->Authorization);
	if (rsp->Bandwidth) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Bandwidth: ");	
		RTSP_WRITE_INT(buffer, size, cur_pos, rsp->Bandwidth, 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	if (rsp->Blocksize) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Blocksize: ");
		RTSP_WRITE_INT(buffer, size, cur_pos, rsp->Blocksize, 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Cache-Control", rsp->Cache_Control);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Conference", rsp->Conference);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Connection", rsp->Connection);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Content-Base", rsp->Content_Base);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Content-Encoding", rsp->Content_Encoding);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Content-Language", rsp->Content_Language);
	//if we have a body write the content length
	if (rsp->body) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Content-Length: ");
		RTSP_WRITE_INT(buffer, size, cur_pos, strlen(rsp->body), 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Content-Location", rsp->Content_Location);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Content-Type", rsp->Content_Type);
	//write the CSeq - use the RESPONSE CSeq
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "CSeq: ");
	RTSP_WRITE_INT(buffer, size, cur_pos, rsp->CSeq, 0);
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	

	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Date", rsp->Date);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Expires", rsp->Expires);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "From", rsp->From);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Host", rsp->Host);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "If-Match", rsp->If_Match);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "If-Modified-Since", rsp->If_Modified_Since);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Last-Modified", rsp->Last_Modified);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Location", rsp->Location);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Proxy-Authenticate", rsp->Proxy_Authenticate);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Proxy-Require", rsp->Proxy_Require);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Public", rsp->Public);

	//Range, only NPT
	if (rsp->Range && !rsp->Range->UseSMPTE) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Range: npt:");
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, rsp->Range->start);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "-");	
		if (rsp->Range->end > rsp->Range->start) {
			RTSP_WRITE_FLOAT(buffer, size, cur_pos, rsp->Range->end);
		}
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}

	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Referer", rsp->Referer);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Require", rsp->Require);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Retry-After", rsp->Retry_After);

	//RTP Infos
	if (ChainGetCount(rsp->RTP_Infos)) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "RTP_Info: ");

		for (i=0; i<ChainGetCount(rsp->RTP_Infos); i++) {
			//line separator for headers
			if (i) RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n ,");
			info = ChainGetEntry(rsp->RTP_Infos, i);
			
			if (info->url) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "url=");
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, info->url);
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";");
			}
			RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "seq=");
			RTSP_WRITE_INT(buffer, size, cur_pos, info->seq, 0);
			RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";rtptime=");
			RTSP_WRITE_INT(buffer, size, cur_pos, info->rtp_time, 0);
		}
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");
	}
	
	if (rsp->Scale != 0.0) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Scale: ");
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, rsp->Scale);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Server", rsp->Server);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Session", rsp->Session);
	if (rsp->Speed != 0.0) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Scale: ");
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, rsp->Speed);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Timestamp", rsp->Timestamp);

	//transport info
	if (ChainGetCount(rsp->Transports)) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Transport: ");
		for (i=0; i<ChainGetCount(rsp->Transports); i++) {
			//line separator for headers
			if (i) RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n ,");
			trans = ChainGetEntry(rsp->Transports, i);

			//then write the structure
			RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, trans->Profile);
			RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, (trans->IsUnicast ? ";unicast" : ";multicast"));
			if (trans->destination) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";destination=");
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, trans->destination);
			}
			if (trans->source) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";source=");
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, trans->source);
			}
			if (trans->IsRecord) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";mode=RECORD");
				if (trans->Append) RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";append");
			}
			if (trans->IsInterleaved) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";interleaved=");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->rtpID, 0);
				if (trans->rtcpID != trans->rtpID) {
					RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "-");
					RTSP_WRITE_INT(buffer, size, cur_pos, trans->rtcpID, 0);
				}
			}
			//multicast specific
			if (!trans->IsUnicast) {
				if (trans->MulticastLayers) {
					RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";layers=");
					RTSP_WRITE_INT(buffer, size, cur_pos, trans->MulticastLayers, 0);
				}
				if (trans->TTL) {
					RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";ttl=");
					RTSP_WRITE_INT(buffer, size, cur_pos, trans->TTL, 0);
				}
			}
			if (trans->port_first) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, trans->IsUnicast ? ";server_port=" : ";port=");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->port_first, 0);
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "-");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->port_last, 0);
			}
			if (trans->IsUnicast && trans->client_port_first) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";client_port=");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->client_port_first, 0);
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "-");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->client_port_last, 0);
			}
			if (trans->SSRC) {
				RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, ";ssrc=");
				RTSP_WRITE_INT(buffer, size, cur_pos, trans->SSRC, 0);
			}
		}
		//done with transport
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");
	}

	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Unsupported", rsp->Unsupported);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "User-Agent", rsp->User_Agent);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Vary", rsp->Vary);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Via", rsp->Via);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "WWW-Authenticate", rsp->WWW_Authenticate);

	//eXtensions
	for (i=0; i<ChainGetCount(rsp->Xtensions); i++) {
		att = ChainGetEntry(rsp->Xtensions, i);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "x-");
		RTSP_WRITE_HEADER(buffer, size, cur_pos, att->Name, att->Value);	
	}
	//end of header
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");
	//then body
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, rsp->body);

	*out_buffer = buffer;
	*out_size = strlen(buffer);
	return M4OK;
}


M4Err RTSP_SendResponse(LPRTSPSESSION sess, RTSPResponse *rsp)
{
	u32 size;
	char *buffer;
	M4Err e;

	if (!sess || !rsp || !rsp->CSeq) return M4BadParam;

	//check we're not sending something greater than the current CSeq
	if (rsp->CSeq > sess->CSeq) return M4BadParam;

	e = RTSP_WriteResponse(sess, rsp, (unsigned char **) &buffer, &size);
	if (e) goto exit;

	//send buffer
	e = RTSP_Send(sess, buffer, size);
	if (e) return e;
//	printf("RTSP Send Response\n\n%s\n\n", buffer);

exit:
	if (buffer) free(buffer);
	return e;
}

