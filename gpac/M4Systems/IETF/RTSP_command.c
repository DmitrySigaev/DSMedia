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

RTSPCommand *RTSP_NewCommand()
{
	RTSPCommand *tmp = malloc(sizeof(RTSPCommand));
	memset(tmp, 0, sizeof(RTSPCommand));
	tmp->Xtensions = NewChain();
	tmp->Transports = NewChain();
	return tmp;
}


#define COM_FREE_CLEAN(hdr)		if (com->hdr) free(com->hdr);	\
								com->hdr = NULL;

void RTSP_ResetCommand(RTSPCommand *com)
{
	RTSPTransport *trans;
	X_Attribute *att;
	if (!com) return;

	//free all headers
	COM_FREE_CLEAN(Accept);
	COM_FREE_CLEAN(Accept_Encoding);
	COM_FREE_CLEAN(Accept_Language);
	COM_FREE_CLEAN(Authorization);
	COM_FREE_CLEAN(Cache_Control);
	COM_FREE_CLEAN(Conference);
	COM_FREE_CLEAN(Connection);
	COM_FREE_CLEAN(From);
	COM_FREE_CLEAN(Proxy_Authorization);
	COM_FREE_CLEAN(Proxy_Require);
	COM_FREE_CLEAN(Referer);
	COM_FREE_CLEAN(Session);
	COM_FREE_CLEAN(User_Agent);
	COM_FREE_CLEAN(body);
	COM_FREE_CLEAN(service_name);
	COM_FREE_CLEAN(ControlString);
	COM_FREE_CLEAN(method);

	//this is for server only, set to OK by default
	com->StatusCode = NC_RTSP_OK;
	
	
	com->user_data = NULL;

	com->Bandwidth = com->Blocksize = com->Content_Length = com->CSeq = 0;
	com->Scale = com->Speed = 0.0;
	if (com->Range) free(com->Range);
	com->Range = NULL;

	while (ChainGetCount(com->Transports)) {
		trans = ChainGetEntry(com->Transports, 0);
		ChainDeleteEntry(com->Transports, 0);
		RTSP_DeleteTransport(trans);
	}
	while (ChainGetCount(com->Xtensions)) {
		att = ChainGetEntry(com->Xtensions, 0);
		ChainDeleteEntry(com->Xtensions, 0);
		free(att->Name);
		free(att->Value);
		free(att);
	}
}

void RTSP_DeleteCommand(RTSPCommand *com)
{
	if (!com) return;
	RTSP_ResetCommand(com);
	DeleteChain(com->Xtensions);
	DeleteChain(com->Transports);
	free(com);
}


M4Err RTSP_WriteCommand(RTSPSession *sess, RTSPCommand *com, unsigned char *req_buffer, 
						 unsigned char **out_buffer, u32 *out_size)
{
	u32 i, cur_pos, size;
	char *buffer, temp[50];
	RTSPTransport *trans;
	X_Attribute *att;

	*out_buffer = NULL;

	size = RTSP_WRITE_STEPALLOC;
	buffer = malloc(size);
	cur_pos = 0;

	//request
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, req_buffer);

	//then all headers
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept", com->Accept);
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept-Encoding", com->Accept_Encoding);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Accept-Language", com->Accept_Language);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Authorization", com->Authorization);	
	if (com->Bandwidth) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Bandwidth: ");	
		RTSP_WRITE_INT(buffer, size, cur_pos, com->Bandwidth, 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	if (com->Blocksize) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Blocksize: ");
		RTSP_WRITE_INT(buffer, size, cur_pos, com->Blocksize, 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Cache-Control", com->Cache_Control);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Conference", com->Conference);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Connection", com->Connection);
	//if we have a body write the content length
	if (com->body) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Content-Length: ");
		RTSP_WRITE_INT(buffer, size, cur_pos, strlen(com->body), 0);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	//write the CSeq - use the SESSION CSeq
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "CSeq: ");
	RTSP_WRITE_INT(buffer, size, cur_pos, sess->CSeq, 0);
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	

	RTSP_WRITE_HEADER(buffer, size, cur_pos, "From", com->From);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Proxy-Authorization", com->Proxy_Authorization);	
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Proxy-Require", com->Proxy_Require);	

	//Range, only NPT
	if (com->Range && !com->Range->UseSMPTE) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Range: npt=");
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, com->Range->start);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "-");	
		if (com->Range->end > com->Range->start) {
			RTSP_WRITE_FLOAT(buffer, size, cur_pos, com->Range->end);
		}
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}

	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Referer", com->Referer);	
	if (com->Scale != 0.0) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Scale: ");	
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, com->Scale);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "Session", com->Session);	
	if (com->Speed != 0.0) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Speed: ");	
		RTSP_WRITE_FLOAT(buffer, size, cur_pos, com->Speed);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");	
	}

	//transport info
	if (ChainGetCount(com->Transports)) {
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "Transport: ");
		for (i=0; i<ChainGetCount(com->Transports); i++) {
			//line separator for headers
			if (i) RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n ,");
			trans = ChainGetEntry(com->Transports, i);

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
	RTSP_WRITE_HEADER(buffer, size, cur_pos, "User-Agent", com->User_Agent);	

	//eXtensions
	for (i=0; i<ChainGetCount(com->Xtensions); i++) {
		att = ChainGetEntry(com->Xtensions, i);
		RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "x-");
		RTSP_WRITE_HEADER(buffer, size, cur_pos, att->Name, att->Value);	
	}

	//the end of header
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");
	//then body
	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, com->body);
	//the end of message ? to check, should not be needed...
//	RTSP_WRITE_ALLOC_STR(buffer, size, cur_pos, "\r\n");

	*out_buffer = buffer;
	*out_size = strlen(buffer);
	return M4OK;
}


//format a DESCRIBE, SETUP, PLAY or PAUSE on a session
//YOUR COMMAND MUST BE FORMATTED ACCORDINGLY 
//sCtrl contains a control string if needed, formating the REQUEST as server_url/service_name/sCtrl
M4Err RTSP_SendRequest(RTSPSession *sess, RTSPCommand *com)
{
	M4Err e;
	char *rad, *sCtrl;
	u32 size;
	char buffer[1024], *result, *body;

	if (!com || !com->method) return M4BadParam;

	sCtrl = com->ControlString;

	//NB: OPTIONS is not sent this way
	if (strcmp(com->method, RTSP_DESCRIBE)
		&& strcmp(com->method, RTSP_ANNOUNCE)
		&& strcmp(com->method, RTSP_GET_PARAMETER)
		&& strcmp(com->method, RTSP_SET_PARAMETER)
		&& strcmp(com->method, RTSP_SETUP)
		&& strcmp(com->method, RTSP_PLAY)
		&& strcmp(com->method, RTSP_PAUSE)
		&& strcmp(com->method, RTSP_RECORD)
		&& strcmp(com->method, RTSP_REDIRECT)
		&& strcmp(com->method, RTSP_TEARDOWN)
		&& strcmp(com->method, RTSP_OPTIONS)

		) return M4BadParam;

	//check the state machine
	if (strcmp(com->method, RTSP_PLAY) 
		&& strcmp(com->method, RTSP_PAUSE) 
		&& strcmp(com->method, RTSP_RECORD)
		&& sess->RTSP_State != RSM_Init) 
		return M4ServiceError;

	//aggregation is ONLY for the same request - unclear in RFC2326 ...
	//it is often mentioned "queued requests" at the server, like 3 PLAYS
	//and a PAUSE ....
	
	/*
	else if (sess->RTSP_State == RSM_WaitForControl 
		&& strcmp(com->method, sess->RTSPLastRequest))
		&& strcmp(com->method, RTSP_OPTIONS))

		return M4BadParam;
*/

	//OPTIONS must have a parameter string 
	if (!strcmp(com->method, RTSP_OPTIONS) && !sCtrl) return M4BadParam;


	//update sequence number
	sess->CSeq += 1;
	sess->NbPending += 1;

	if (!strcmp(com->method, RTSP_OPTIONS)) {
		sprintf(buffer, "OPTIONS %s %s\r\n", sCtrl, RTSP_VERSION);
	} else {
		rad = (sess->ConnectionType == SK_TYPE_TCP) ? "rtsp" : "rtspu";
		if (sCtrl) {
			//if both server and service names are included in the control, just
			//use the control
			if (strstr(sCtrl, sess->Server) && strstr(sCtrl, sess->Service)) {
				sprintf(buffer, "%s %s %s\r\n", com->method, sCtrl, RTSP_VERSION);
			}
			//if service is specified in ctrl, do not rewrite it
			else if (strstr(sCtrl, sess->Service)) {
				sprintf(buffer, "%s %s://%s:%d/%s %s\r\n", com->method, rad, sess->Server, sess->Port, sCtrl, RTSP_VERSION);
			}
			//otherwise rewrite full URL
			else {
				sprintf(buffer, "%s %s://%s/%s/%s %s\r\n", com->method, rad, sess->Server, sess->Service, sCtrl, RTSP_VERSION);
//				sprintf(buffer, "%s %s://%s:%d/%s/%s %s\r\n", com->method, rad, sess->Server, sess->Port, sess->Service, sCtrl, RTSP_VERSION);
			}
		} else {
			sprintf(buffer, "%s %s://%s:%d/%s %s\r\n", com->method, rad, sess->Server, sess->Port, sess->Service, RTSP_VERSION);
		}
	}

	//Body on ANNOUNCE, GET_PARAMETER, SET_PARAMETER ONLY
	body = NULL;
	if (strcmp(com->method, RTSP_ANNOUNCE) 
		&& strcmp(com->method, RTSP_GET_PARAMETER) 
		&& strcmp(com->method, RTSP_SET_PARAMETER) 
		) {
		//this is an error, but don't say anything
		if (com->body) {
			body = com->body;
			com->body = NULL;
		}
	}

	result = NULL;
	e = RTSP_WriteCommand(sess, com, buffer, (unsigned char **) &result, &size);
	//restore body if needed
	if (body) com->body = body;
	if (e) goto exit;


	//send buffer
	e = RTSP_Send(sess, result, size);
	if (e) goto exit;


	if (sess->rtsp_log) fprintf(sess->rtsp_log, "\n/*RTSP Send Command*/\n\n%s\n", result);

	//update our state
	if (!strcmp(com->method, RTSP_RECORD)) sess->RTSP_State = RSM_WaitForControl;
	else if (!strcmp(com->method, RTSP_PLAY)) sess->RTSP_State = RSM_WaitForControl;
	else if (!strcmp(com->method, RTSP_PAUSE)) sess->RTSP_State = RSM_WaitForControl;
	else sess->RTSP_State = RSM_Waiting;
	//teardown invalidates the session most of the time, so we force the user to wait for the reply
	//as the reply may indicate a connection-closed
	strcpy(sess->RTSPLastRequest, com->method);

exit:
	if (result) free(result);
	return e;
}


void RTSP_SetCommandVal(RTSPCommand *com, char *Header, char *Value)
{
	char LineBuffer[400];
	s32 LinePos;
	RTSPTransport *trans;
	X_Attribute *x_Att;

	if (!stricmp(Header, "Accept")) com->Accept = strdup(Value);
	else if (!stricmp(Header, "Accept-Encoding")) com->Accept_Encoding = strdup(Value);
	else if (!stricmp(Header, "Accept-Language")) com->Accept_Language = strdup(Value);
	else if (!stricmp(Header, "Authorization")) com->Authorization = strdup(Value);
	else if (!stricmp(Header, "Bandwidth")) sscanf(Value, "%d", &com->Bandwidth);
	else if (!stricmp(Header, "Blocksize")) sscanf(Value, "%d", &com->Blocksize);
	else if (!stricmp(Header, "Cache-Control")) com->Cache_Control = strdup(Value);
	else if (!stricmp(Header, "Conference")) com->Conference = strdup(Value);
	else if (!stricmp(Header, "Connection")) com->Connection = strdup(Value);
	else if (!stricmp(Header, "Content-Length")) sscanf(Value, "%d", &com->Content_Length);
	else if (!stricmp(Header, "CSeq")) sscanf(Value, "%d", &com->CSeq);
	else if (!stricmp(Header, "From")) com->From = strdup(Value);
	else if (!stricmp(Header, "Proxy_Authorization")) com->Proxy_Authorization = strdup(Value);
	else if (!stricmp(Header, "Proxy_Require")) com->Proxy_Require = strdup(Value);
	else if (!stricmp(Header, "Range")) com->Range = RTSP_ParseRange(Value);
	else if (!stricmp(Header, "Referer")) com->Referer = strdup(Value);
	else if (!stricmp(Header, "Scale")) sscanf(Value, "%f", &com->Scale);
	else if (!stricmp(Header, "Session")) com->Session = strdup(Value);
	else if (!stricmp(Header, "Speed")) sscanf(Value, "%f", &com->Speed);
	else if (!stricmp(Header, "User_Agent")) com->User_Agent = strdup(Value);
	//Transports
	else if (!stricmp(Header, "Transport")) {
		LinePos = 0;
		while (1) {
			LinePos = SP_GetComponent(Value, LinePos, "\r\n", LineBuffer, 400);
			if (LinePos <= 0) return;
			trans = ParseTransport(Value);
			if (trans) ChainAddEntry(com->Transports, trans);
		}
	}
	//eXtensions attributes
	else if (!strnicmp(Header, "x-", 2)) {
		x_Att = malloc(sizeof(X_Attribute));
		x_Att->Name = strdup(Header+2);
		x_Att->Value = NULL;
		if (Value && strlen(Value)) x_Att->Value = strdup(Value);
		ChainAddEntry(com->Xtensions, x_Att);
	}
	//the rest is ignored
}

M4Err RTSP_ParseCommandHeader(RTSPSession *sess, RTSPCommand *com, u32 BodyStart)
{
	unsigned char LineBuffer[1024];
	unsigned char ValBuf[1024];
	unsigned char *buffer;
	s32 Pos, ret;
	u32 Size;

	Size = sess->CurrentSize - sess->CurrentPos;
	buffer = sess->TCPBuffer + sess->CurrentPos;

	//by default the command is wrong ;)
	com->StatusCode = NC_RTSP_Bad_Request;

	//parse first line
	ret = SP_GetOneLine(buffer, 0, Size, LineBuffer, 1024);
	if (ret < 0) return M4SignalingFailure;

	//method
	Pos = SP_GetComponent(LineBuffer, 0, " \t\r\n", ValBuf, 1024);
	if (Pos <= 0) return M4OK;
	com->method = strdup(ValBuf);

	//URL
	Pos = SP_GetComponent(LineBuffer, Pos, " \t\r\n", ValBuf, 1024);
	if (Pos <= 0) return M4OK;
	com->service_name = strdup(ValBuf);
	
	//RTSP version
	Pos = SP_GetComponent(LineBuffer, Pos, "\t\r\n", ValBuf, 1024);
	if (Pos <= 0) return M4OK;
	if (strcmp(ValBuf, RTSP_VERSION)) {
		com->StatusCode = NC_RTSP_RTSP_Version_Not_Supported;
		return M4OK;
	}

	com->StatusCode = NC_RTSP_OK;
	
	return RTSP_ParseHeaderLines(buffer + ret, Size - ret, BodyStart, com, NULL);
}


M4Err RTSP_GetRequest(RTSPSession *sess, RTSPCommand *com)
{
	M4Err e;
	u32 BodyStart, size;
	if (!sess || !com) return M4BadParam;

	//reset the command
	RTSP_ResetCommand(com);
	//if no connection, we have sent a "Connection: Close"
	if (!sess->connection) return M4ConnectionClosed;

	//lock
	MX_P(sess->mx);

	//fill TCP buffer
	e = RTSP_FillTCPBuffer(sess);
	if (e) goto exit;
	//this is upcoming, interleaved data
	if (strncmp(sess->TCPBuffer+sess->CurrentPos, "RTSP", 4)) {
		e = M4NetworkEmpty;
		goto exit;
	}
	e = RTSP_ReadReply(sess);
	if (e) goto exit;

	RTSP_GetBodyInfo(sess, &BodyStart, &size);
	e = RTSP_ParseCommandHeader(sess, com, BodyStart);
	//before returning an error we MUST reset the TCP buffer

	//copy the body if any
	if (!e && com->Content_Length) {
		com->body = malloc(sizeof(char) * (com->Content_Length));
		memcpy(com->body, sess->TCPBuffer+sess->CurrentPos + BodyStart, com->Content_Length);
	}	
	//reset TCP buffer
	sess->CurrentPos += BodyStart + com->Content_Length;

	if (!com->CSeq) com->StatusCode = NC_RTSP_Bad_Request;

	if (e || (com->StatusCode != NC_RTSP_OK)) goto exit;

	//NB: there is no "session state" in our lib when acting at the server side, as it depends
	//on the server implementation. We cannot block responses / announcement to be sent
	//dynamically, nor reset the session ourselves as we don't know the details of the session
	//(eg TEARDOWN may keep resources up or not, ...)

	//we also have the same pb for CSeq, as nothing forbids a server to buffer commands (and it 
	//happens during aggregation of PLAY/PAUSE with overlapping ranges)

	//however store the last CSeq in case for client checking
	if (!sess->CSeq) {
		sess->CSeq = com->CSeq;
	}
	//check we're in the right range
	else {
		if (sess->CSeq >= com->CSeq) 
			com->StatusCode = NC_RTSP_Header_Field_Not_Valid;
		else
			sess->CSeq = com->CSeq;
	}

	//
	//if a connection closed is signal, check this is the good session
	// and reset it (the client is no longer connected)
	if (sess->SessionID && com->Session && !strcmp(com->Session, sess->SessionID) 
		&& com->Connection && !stricmp(com->Connection, "Close")) {

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














