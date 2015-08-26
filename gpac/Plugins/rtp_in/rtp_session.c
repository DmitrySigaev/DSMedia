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

void RP_SendFailure(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	char sMsg[1000];
	sprintf(sMsg, "Cannot send %s", com->method);
	NM_OnMessage(sess->owner->service, e, sMsg);
}

void RP_ProcessResponse(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	if (!strcmp(com->method, RTSP_DESCRIBE)) 
		RP_ProcessDescribe(sess, com, e);
	else if (!strcmp(com->method, RTSP_SETUP)) 
		RP_ProcessSetup(sess, com, e);
	else if (!strcmp(com->method, RTSP_TEARDOWN)) 
		RP_ProcessTeardown(sess, com, e);
	else if (!strcmp(com->method, RTSP_PLAY) || !strcmp(com->method, RTSP_PAUSE)) 
		RP_ProcessUserCommand(sess, com, e);
}

/*access to command list is protected bymutex, BUT ONLY ACCESS - this way we're sure that command queueing
from app will not deadlock if we're waiting for the app to release any mutex (don't forget play request may 
come on stream N while we're processing stream P setup)*/
static RTSPCommand *RP_GetCommand(RTPSession *sess)
{
	RTSPCommand *com;
	MX_P(sess->owner->mx);
	com = ChainGetEntry(sess->rtsp_commands, 0);
	MX_V(sess->owner->mx);
	return com;
}

static void RP_RemoveCommand(RTPSession *sess)
{
	MX_P(sess->owner->mx);
	ChainDeleteEntry(sess->rtsp_commands, 0);
	MX_V(sess->owner->mx);
}

void RP_ProcessCommands(RTPSession *sess, Bool read_tcp)
{
	RTSPCommand *com;
	M4Err e;
	u32 time;

	com = RP_GetCommand(sess);

	/*if asked or command to send, flushout TCP - TODO: check what's going on with ANNOUNCE*/
	if ((com && !sess->wait_for_reply) || read_tcp) {
		while (1) {
			e = RTSP_ReadTCP(sess->session);
			if (e) break;
		}
	}

	/*handle response or announce*/
	if ( (com && sess->wait_for_reply) || (!com && sess->owner->handle_announce)) {
		e = RTSP_GetResponse(sess->session, sess->rtsp_rsp);
		if (e!= M4NetworkEmpty) {
			RP_ProcessResponse(sess, com, e);
			RP_RemoveCommand(sess);
			RTSP_DeleteCommand(com);
			sess->wait_for_reply = 0;
			sess->command_time = 0;
		} else {
			/*evaluate timeout*/
			time = M4_GetSysClock() - sess->command_time;
			/*don't waste time waiting for teardown ACK, half a sec is enough. If server is not replying
			in time it is likely to never reply (happens with RTP over RTSP) -> kill session 
			and create new one*/
			if (!strcmp(com->method, RTSP_TEARDOWN) && (time>=500) ) time = sess->owner->time_out;
			//signal what's going on
			if (time >= sess->owner->time_out) {
				if (!strcmp(com->method, RTSP_TEARDOWN)) RTSP_ResetSession(sess->session, 1);

				RP_ProcessResponse(sess, com, M4NetworkFailure);
				RP_RemoveCommand(sess);
				RTSP_DeleteCommand(com);
				sess->wait_for_reply = 0;
				sess->command_time = 0;
				RTSP_ResetAggregation(sess->session);
			}
		}
		return;
	}

	if (!com) return;

	/*send command - check RTSP session state first*/
	switch (RTSP_GetSessionState(sess->session)) {
	case RSM_Waiting:
	case RSM_WaitForControl:
		return;
	case RSM_Invalidated:
		RP_SendFailure(sess, com, M4NetworkFailure);
		RP_RemoveCommand(sess);
		RTSP_DeleteCommand(com);
		sess->wait_for_reply = 0;
		sess->command_time = 0;
		return;
	}
	/*process*/
	com->User_Agent = RTSP_CLIENTNAME;
	com->Accept_Language = RTSP_LANGUAGE;
	com->Session = RTSP_GetSessionID(sess->session);

	e = M4OK;
	/*preprocess describe before sending (always the ESD url thing)*/
	if (!strcmp(com->method, RTSP_DESCRIBE)) {
		com->Session = NULL;
		if (!RP_PreprocessDescribe(sess, com)) {
			e = M4BadParam;
			goto exit;
		}
	}
	/*preprocess play/stop/pause before sending (aggregation)*/
	if (!strcmp(com->method, RTSP_PLAY) 
		|| !strcmp(com->method, RTSP_PAUSE)
		|| !strcmp(com->method, RTSP_TEARDOWN)) {
		//command is skipped
		if (!RP_PreprocessUserCom(sess, com)) {
			e = M4BadParam;
			goto exit;
		}
	}
	e = RTSP_SendRequest(sess->session, com);
	if (e) {
		RP_SendFailure(sess, com, e);
		RP_ProcessResponse(sess, com, e);
	} else {
		sess->command_time = M4_GetSysClock();
		sess->wait_for_reply = 1;
	}

exit:
	/*reset static strings*/
	com->User_Agent = NULL;
	com->Accept_Language = NULL;
	com->Session = NULL;
	/*remove command*/
	if (e) {
		RP_RemoveCommand(sess);
		RTSP_DeleteCommand(com);
		sess->wait_for_reply = 0;
		sess->command_time = 0;
	}
}


/*locate channel - if requested remove from session*/
RTPStream *RP_FindChannel(RTPClient *rtp, LPNETCHANNEL ch, u32 ES_ID, char *es_control, Bool remove_stream)
{
	u32 i;
	RTPStream *st;

	for (i=0; i<ChainGetCount(rtp->channels); i++) {
		st = ChainGetEntry(rtp->channels, i);
		if (ch && (st->channel==ch)) goto found;
		if (ES_ID && (st->ES_ID==ES_ID)) goto found;
		if (es_control && strstr(es_control, st->control)) goto found;
	}
	return NULL;

found:
	if (remove_stream) ChainDeleteEntry(rtp->channels, i);
	return st;
}

/*locate session by control*/
RTPSession *RP_CheckSession(RTPClient *rtp, char *control)
{
	if (!control || !rtp->rtsp_session) return NULL;
	if (RTSP_IsMySession(rtp->rtsp_session->session, control)) return rtp->rtsp_session;
	return NULL;
}

RTPSession *RP_NewSession(RTPClient *rtp, char *session_control)
{
	char *szCtrl, *szExt;
	RTPSession *tmp;
	LPRTSPSESSION rtsp;

	if (rtp->rtsp_session) return NULL;

	/*little fix: some servers don't understand DESCRIBE URL/trackID=, so remove the trackID...*/
	szCtrl = strdup(session_control);
	szExt = strrchr(szCtrl, '.');
	if (szExt) {
		szExt = strchr(szExt, '/');
		if (szExt) {
			if (!strnicmp(szExt+1, "trackID=", 8) || !strnicmp(szExt+1, "ESID=", 5) || !strnicmp(szExt+1, "ES_ID=", 6)) szExt[0] = 0;
		}
	}

	rtsp = RTSP_NewSession(szCtrl, rtp->default_port);
	free(szCtrl);

	if (!rtsp) return NULL;

	RTSP_SetLog(rtsp, rtp->logs);

	tmp = malloc(sizeof(RTPSession));
	memset(tmp, 0, sizeof(RTPSession));
	tmp->owner = rtp;
	tmp->session = rtsp;

	if (rtp->rtp_mode) {
		RTSP_SetBufferSize(rtsp, RTSP_TCP_BUFFER_SIZE);
	} else {
		RTSP_SetBufferSize(rtsp, RTSP_BUFFER_SIZE);
	}
	rtp->rtsp_session = tmp;

	tmp->rtsp_commands = NewChain();

	tmp->rtsp_rsp = RTSP_NewResponse();	

	return tmp;
}

M4Err RP_AddStream(RTPClient *rtp, RTPStream *stream, char *session_control)
{
	Bool has_aggregated_control;
	char *service_name, *ctrl;
	RTPSession *in_session = rtp->rtsp_session;

	has_aggregated_control = 0;
	if (session_control) {
		if (!strcmp(session_control, "*")) session_control = NULL;
		if (session_control) has_aggregated_control = 1;
	}

	/*regular setup in an established session (RTSP DESCRIBE)*/
	if (in_session) {
		if (in_session->has_aggregated_control) 
			in_session->has_aggregated_control = has_aggregated_control;
		stream->rtsp = in_session;
		ChainAddEntry(rtp->channels, stream);
		return M4OK;
	}

	/*setup through SDP with control - assume this is RTSP and try to create a session*/
	if (stream->control) {
		/*stream control is relative to main session*/
		if (strnicmp(stream->control, "rtsp://", 7) && strnicmp(stream->control, "rtspu://", 7)) {
			/*we need session control*/
			if (!session_control) return M4InvalidPeerAddress;
			/*locate session by control*/
			if (!in_session) in_session = RP_CheckSession(rtp, session_control);
			/*none found, try to create one*/
			if (!in_session) in_session = RP_NewSession(rtp, session_control);
			/*cannot add an RTSP session for this channel, check if multicast*/
			if (!in_session && RTP_IsUnicast(stream->rtp_ch) ) return M4InvalidPeerAddress;
		}
		/*stream control is absolute*/
		else {
			in_session = RP_CheckSession(rtp, stream->control);
			if (!in_session) in_session = RP_CheckSession(rtp, session_control);
			if (!in_session) {
				if (session_control && strstr(stream->control, session_control))
					in_session = RP_NewSession(rtp, session_control);
				else
					in_session = RP_NewSession(rtp, stream->control);
				if (!in_session) return M4InvalidPeerAddress;
			}
			/*remove session control part from channel control*/
			service_name = RTSP_GetServiceName(in_session->session);
			ctrl = strstr(stream->control, service_name);
			if (ctrl && (strlen(ctrl) != strlen(service_name)) ) {
				ctrl += strlen(service_name) + 1;
				service_name = strdup(ctrl);
				free(stream->control);
				stream->control = service_name;
			}
		}
	}
	/*no control specified, assume this is multicast*/
	else {
		in_session = NULL;
	}

	if (in_session) in_session->has_aggregated_control = has_aggregated_control;
	stream->rtsp = in_session;
	ChainAddEntry(rtp->channels, stream);
	return M4OK;
}


void RP_RemoveStream(RTPClient *rtp, RTPStream *ch)
{
	u32 i;
	MX_P(rtp->mx);
	for (i=0; i<ChainGetCount(rtp->channels); i++) {
		if (ChainGetEntry(rtp->channels, i) == ch) {
			ChainDeleteEntry(rtp->channels, i);
			break;
		}
	}
	MX_V(rtp->mx);
}

void RP_ResetSession(RTPSession *sess, M4Err e)
{
	LPRTSPCOMMAND com;
	u32 first = 1;

	//destroy command list
	while (ChainGetCount(sess->rtsp_commands)) {
		com = ChainGetEntry(sess->rtsp_commands, 0);
		ChainDeleteEntry(sess->rtsp_commands, 0);
		//this destroys stacks if any
//		RP_SendFailure(sess, com, first ? e : M4OK);
		RTSP_DeleteCommand(com);
		first = 0;
	}
	/*reset session state*/	
	RTSP_ResetSession(sess->session, 1);
	sess->wait_for_reply = 0;
}


void RP_RemoveSession(RTPSession *sess, Bool immediate_shutdown)
{
	/*shutdown session*/
	RP_Teardown(sess);
	/*wait for ack*/
	if (!immediate_shutdown ) {
		while (ChainGetCount(sess->rtsp_commands)) 
			Sleep(10);
	}

	RP_ResetSession(sess, M4OK);

	DeleteChain(sess->rtsp_commands);
	RTSP_DeleteResponse(sess->rtsp_rsp);
	RTSP_DeleteSession(sess->session);
	if (sess->control) free(sess->control);
	free(sess);
}


