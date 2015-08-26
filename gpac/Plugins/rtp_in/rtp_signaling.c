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


Bool channel_is_valid(RTPClient *rtp, RTPStream *ch)
{
	u32 i;
	for (i=0; i<ChainGetCount(rtp->channels); i++) {
		if (ChainGetEntry(rtp->channels, i) == ch) return 1;
	}
	return 0;
}

void RP_StopChannel(RTPStream *ch)
{
	if (!ch || !ch->rtsp) return;

	ch->idle = 0;
	ch->status = RTP_Disconnected;
	//remove interleaved
	if (RTP_IsInterleaved(ch->rtp_ch)) {
		RTSP_UnregisterTCPChannel(ch->rtsp->session, RTP_GetLowInterleavedID(ch->rtp_ch));
	}
}

/*this prevent sending teardown on session with running channels*/
Bool RP_SessionActive(RTPStream *ch)
{
	u32 i, count, idle;
	idle = count = 0;
	for (i=0; i<ChainGetCount(ch->owner->channels); i++) {
		RTPStream *ach = ChainGetEntry(ch->owner->channels, i);
		if (ach->rtsp != ch->rtsp) continue;
		/*count only active channels*/
		if (ach->status == RTP_Running) continue;
		count++;
		if (ach->idle) idle++;
	}
	return (count==idle) ? 0 : 1;
}


/*
 						channel setup functions
																*/

void RP_Setup(RTPStream *ch)
{
	LPRTSPCOMMAND com;
	RTSPTransport *trans;

	com = RTSP_NewCommand();	
	com->method = strdup(RTSP_SETUP);

	//setup ports if unicast non interleaved
	if (RTP_IsUnicast(ch->rtp_ch) && !ch->owner->rtp_mode && !RTP_IsInterleaved(ch->rtp_ch) )
		RTP_SetupPorts(ch->rtp_ch);

	trans = RTSP_DuplicateTransport(RTP_GetTransport(ch->rtp_ch));
	if (trans->source) {
		free(trans->source);
		trans->source = NULL;
	}
	/*some servers get confused when trying to resetup on the same remote ports, so reset info*/
	trans->port_first = trans->port_last = 0;
	trans->SSRC = 0;


	/*turn off interleaving in case of re-setup, some servers don't like it (we still signal it
	through RTP/AVP/TCP profile so it's OK)*/
	trans->IsInterleaved = 0;
	ChainAddEntry(com->Transports, trans);
	if (strlen(ch->control)) com->ControlString = strdup(ch->control);

	com->user_data = ch;
	ch->status = RTP_WaitingForAck;

	MX_P(ch->owner->mx);
	ChainAddEntry(ch->rtsp->rtsp_commands, com);
	MX_V(ch->owner->mx);
}

/*filter setup if no session (rtp only)*/
M4Err RP_SetupChannel(RTPStream *ch, ChannelDescribe *ch_desc)
{
	M4Err resp;

	/*assign ES_ID of the channel*/
	if (ch_desc && !ch->ES_ID && ch_desc->ES_ID) ch->ES_ID = ch_desc->ES_ID;

	ch->status = RTP_Setup;

	/*assign channel handle if not done*/
	if (ch_desc && ch->channel) {
		assert(ch->channel == ch_desc->channel);
	} else if (!ch->channel) {
		assert(ch_desc);
		assert(ch_desc->channel);
		ch->channel = ch_desc->channel;
	}

	/*no session , setup for pure rtp*/
	if (!ch->rtsp) {
		ch->connected = 1;
		/*init rtp*/
		resp = RP_InitStream(ch, 0),
		/*send confirmation to user*/
		RP_ConfirmChannelConnect(ch, resp);
	} else {
		RP_Setup(ch);
	}
	return M4OK;
}

void RP_ProcessSetup(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	RTPStream *ch;
	u32 i;
	RTSPTransport *trans;
	
	
	ch = com->user_data;
	if (e) goto exit;

	switch (sess->rtsp_rsp->ResponseCode) {
	case NC_RTSP_OK:
		break;
	case NC_RTSP_Not_Found:
		e = M4ChannelNotFound;
		goto exit;
	default:
		e = M4ServiceError;
		goto exit;
	}
	e = M4ServiceError;
	if (!ch) goto exit;

	/*transport setup: break at the first correct transport */
	for (i=0; i<ChainGetCount(sess->rtsp_rsp->Transports); i++) {
		trans = ChainGetEntry(sess->rtsp_rsp->Transports, 0);
		e = RTP_SetupTransport(ch->rtp_ch, trans, RTSP_GetServerName(sess->session));
		if (!e) break;
	}
	if (e) goto exit;

	e = RP_InitStream(ch, 0);
	if (e) goto exit;
	ch->status = RTP_Connected;

	//in case this is TCP channel, setup callbacks
	ch->is_interleaved = RTP_IsInterleaved(ch->rtp_ch);
	if (ch->is_interleaved) {
		RTSP_SetCallbackOnInterleaving(sess->session, RP_DataOnTCP);
	}

exit:
	/*confirm only on first connect, otherwise this is a re-SETUP of the rtsp session, not the channel*/
	if (! ch->connected) {
		ch->connected = 1;
		RP_ConfirmChannelConnect(ch, e);
	}
	com->user_data = NULL;
}



/*
 						session/channel describe functions
																*/
/*filter describe commands in case of ESD URLs*/
Bool RP_PreprocessDescribe(RTPSession *sess, RTSPCommand *com)
{
	RTPStream *ch;
	ChannelDescribe *ch_desc;
	/*not a channel describe*/
	if (!com->user_data) {
		NM_OnMessage(sess->owner->service, M4OK, "Connecting...");
		return 1;
	}

	ch_desc = (ChannelDescribe *)com->user_data;
	ch = RP_FindChannel(sess->owner, NULL, ch_desc->ES_ID, ch_desc->esd_url, 0);
	if (!ch) return 1;

	/*channel has been described already, skip describe and send setup directly*/
	RP_SetupChannel(ch, ch_desc);
	
	if (ch_desc->esd_url) free(ch_desc->esd_url);
	free(ch_desc);
	return 0;
}

/*process describe reply*/
void RP_ProcessDescribe(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	RTPStream *ch;
	ChannelDescribe *ch_desc;

	ch = NULL;
	ch_desc = com->user_data;
	if (e) goto exit;

	switch (sess->rtsp_rsp->ResponseCode) {
	//TODO handle all 3xx codes  (redirections)
	case NC_RTSP_Multiple_Choice:
		e = ch_desc ? M4ChannelNotFound : M4URLNotFound;
		goto exit;
	case NC_RTSP_Not_Found:
		e = M4URLNotFound;
		goto exit;
	case NC_RTSP_OK:
		break;
	default:
		//we should have a basic error code mapping here
		e = M4ServiceError;
		goto exit;
	}

	ch = NULL;
	if (ch_desc) {
		ch = RP_FindChannel(sess->owner, ch_desc->channel, ch_desc->ES_ID, ch_desc->esd_url, 0);
	} else {
		NM_OnMessage(sess->owner->service, M4OK, "Connected");
	}

	/*error on loading SDP is done internally*/
	RP_LoadSDP(sess->owner, sess->rtsp_rsp->body, sess->rtsp_rsp->Content_Length, ch);

	if (!ch_desc) goto exit;
	if (!ch) {
		e = M4ChannelNotFound;
		goto exit;
	}
	e = RP_SetupChannel(ch, ch_desc);

exit:
	if (e) {
		if (!ch_desc) {
			NM_OnConnect(sess->owner->service, NULL, e);
		} else if (ch) {
			RP_ConfirmChannelConnect(ch, e);
		} else {
			NM_OnConnect(sess->owner->service, ch_desc->channel, e);
		}
	}
	if (ch_desc) free(ch_desc);
	com->user_data = NULL;
}

/*send describe*/
void RP_Describe(RTPSession *sess, char *esd_url, LPNETCHANNEL channel)
{
	RTPStream *ch;
	ChannelDescribe *ch_desc;
	RTSPCommand *com;

	/*locate the channel by URL - if we have one, this means the channel is already described
	this happens when 2 ESD with URL use the same RTSP service - skip describe and send setup*/
	if (esd_url || channel) {
		ch = RP_FindChannel(sess->owner, channel, 0, esd_url, 0);
		if (ch) {
			if (!ch->channel) ch->channel = channel;
			ch_desc = malloc(sizeof(ChannelDescribe));
			ch_desc->esd_url = esd_url ? strdup(esd_url) : NULL;
			ch_desc->channel = channel;
			RP_SetupChannel(ch, ch_desc);
			
			if (esd_url) free(ch_desc->esd_url);
			free(ch_desc);
			return;
		}
		/*channel not found, send describe on service*/
	}

	/*send describe*/
	com = RTSP_NewCommand();
	com->method = strdup(RTSP_DESCRIBE);

	if (channel || esd_url) {
		com->Accept = strdup("application/sdp");
		com->ControlString = esd_url ? strdup(esd_url) : NULL;

		ch_desc = malloc(sizeof(ChannelDescribe));
		ch_desc->esd_url = esd_url ? strdup(esd_url) : NULL;
		ch_desc->channel = channel;
		
		com->user_data = ch_desc;
	} else {
		//always accept both SDP and IOD
		com->Accept = strdup("application/sdp, application/mpeg4-iod");
//		com->Accept = strdup("application/sdp");
	}
	com->Bandwidth = sess->owner->bandwidth;

	MX_P(sess->owner->mx);
	ChainAddEntry(sess->rtsp_commands, com);
	MX_V(sess->owner->mx);
}

/*
 						channel control functions
																*/
/*remove command if session is using aggregated control*/
Bool RP_PreprocessUserCom(RTPSession *sess, RTSPCommand *com)
{
	ChannelControl *ch_ctrl;
	RTPStream *ch;
	M4Err e;
	Bool skip_it;

	ch_ctrl = com->user_data;
	if (!ch_ctrl) return 1;
	ch = ch_ctrl->ch;
	
	if (!channel_is_valid(sess->owner, ch)) {
		free(ch_ctrl);
		com->user_data = NULL;
		return 0;
	}

	assert(ch->rtsp == sess);
	assert(ch->channel==ch_ctrl->com.base.on_channel);

	skip_it = 0;
	if (!com->Session) {
		/*re-SETUP failed*/
		if (!strcmp(com->method, RTSP_PLAY) || !strcmp(com->method, RTSP_PAUSE)) {
			e = M4ServiceError;
			goto err_exit;
		}
		/*this is a stop, no need for SessionID just skip*/
		skip_it = 1;
	}
	/*check if aggregation discards this command*/
	if (ch->idle || skip_it || (sess->has_aggregated_control && ch->skip_next_command)) {
		ch->skip_next_command = 0;
		ch->idle = 0;
		NM_OnCommand(sess->owner->service, &ch_ctrl->com, M4OK);
		free(ch_ctrl);
		com->user_data = NULL;
		return 0;
	}
	return 1;

err_exit:
	RTSP_ResetAggregation(ch->rtsp->session);
	ch->status = RTP_Disconnected;
	ch->check_rtp_time = 0;
	NM_OnCommand(sess->owner->service, &ch_ctrl->com, e);
	free(ch_ctrl);
	com->user_data = NULL;
	return 0;
}

static void SkipCommandOnSession(RTPStream *ch)
{
	u32 i;
	if (!ch || ch->skip_next_command || !ch->rtsp->has_aggregated_control) return;
	for (i=0; i<ChainGetCount(ch->owner->channels); i++) {
		RTPStream *a_ch = ChainGetEntry(ch->owner->channels, i);
		if (a_ch->idle || (ch == a_ch) || (a_ch->rtsp != ch->rtsp) ) continue;
		a_ch->skip_next_command = 1;
	}
}


void RP_ProcessUserCommand(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	ChannelControl *ch_ctrl;
	RTPStream *ch, *agg_ch;
	u32 i;
	RTP_Info *info;


	ch_ctrl = com->user_data;
	ch = ch_ctrl->ch;

	if (!channel_is_valid(sess->owner, ch)) {
		free(ch_ctrl);
		com->user_data = NULL;
		return;
	}

	assert(ch->channel==ch_ctrl->com.base.on_channel);

	/*some consistency checking: on interleaved sessions, some servers do NOT reply to the 
	teardown. If our command is STOP just skip the error notif*/
	if (e) {
		if (!strcmp(com->method, RTSP_TEARDOWN)) {
			goto process_reply;
		} else {
			goto err_exit;
		}
	}

	switch (sess->rtsp_rsp->ResponseCode) {
	//handle all 3xx codes  (redirections)
	case NC_RTSP_Method_Not_Allowed:
		e = M4NotSupported;
		goto err_exit;
	case NC_RTSP_OK:
		break;
	default:
		//we should have a basic error code mapping here
		e = M4ServiceError;
		goto err_exit;
	}

process_reply:

	NM_OnCommand(sess->owner->service, &ch_ctrl->com, M4OK);

	if ( (ch_ctrl->com.command_type==CHAN_PLAY) 
		|| (ch_ctrl->com.command_type==CHAN_SET_SPEED)
		|| (ch_ctrl->com.command_type==CHAN_RESUME) ) {

		//auto-detect any aggregated control if not done yet
		if (ChainGetCount(sess->rtsp_rsp->RTP_Infos) > 1) {
			sess->has_aggregated_control = 1;
		}

		//process all RTP infos
		for (i=0;i<ChainGetCount(sess->rtsp_rsp->RTP_Infos); i++) {
			info = ChainGetEntry(sess->rtsp_rsp->RTP_Infos, i);
			agg_ch = RP_FindChannel(sess->owner, NULL, 0, info->url, 0);

			if (!agg_ch || (agg_ch->rtsp != sess) ) continue;
			
			/*if play/seeking we must send update RTP/NPT link*/
			if (ch_ctrl->com.command_type != CHAN_RESUME) {
				agg_ch->check_rtp_time = 1;
			}
			/*this is used to discard RTP packets re-sent on resume*/
			else {
				agg_ch->check_rtp_time = 2;
			}
			/* reset the buffers */
			RP_InitStream(agg_ch, 1);

			RTP_SetInfo(agg_ch->rtp_ch, info->seq, info->rtp_time, info->ssrc);
			agg_ch->status = RTP_Running;

			/*skip next play command on this channel if aggregated control*/
			if (ch!=agg_ch && ch->rtsp->has_aggregated_control) agg_ch->skip_next_command = 1;


			if (RTP_IsInterleaved(agg_ch->rtp_ch)) {
				RTSP_RegisterTCPChannel(sess->session, 
								agg_ch, 
								RTP_GetLowInterleavedID(agg_ch->rtp_ch), 
								RTP_GetHighInterleavedID(agg_ch->rtp_ch));
			}
		}
		/*no rtp info (just in case), no time mapped - set to 0 and specify we're not interactive*/
		if (!i) {
			ch->current_start = 0.0;
			ch->check_rtp_time = 1;
			RP_InitStream(ch, 1);
			ch->status = RTP_Running;
			if (RTP_IsInterleaved(ch->rtp_ch)) {
				RTSP_RegisterTCPChannel(sess->session, 
								ch, RTP_GetLowInterleavedID(ch->rtp_ch), RTP_GetHighInterleavedID(ch->rtp_ch));
			}
		}
		ch->skip_next_command = 0;
	} else if (ch_ctrl->com.command_type == CHAN_PAUSE) {
		SkipCommandOnSession(ch);
		ch->skip_next_command = 0;
	} else if (ch_ctrl->com.command_type == CHAN_STOP) {
		assert(0);
	}
	free(ch_ctrl);
	com->user_data = NULL;
	return;


err_exit:
	ch->status = RTP_Disconnected;
	NM_OnCommand(sess->owner->service, &ch_ctrl->com, e);
	RTSP_ResetAggregation(ch->rtsp->session);
	ch->check_rtp_time = 0;
	free(ch_ctrl);
	com->user_data = NULL;
}

static void RP_FlushAndTearDown(RTPSession *sess)
{
	RTSPCommand *com;
	MX_P(sess->owner->mx);

	while (ChainGetCount(sess->rtsp_commands)) {
		com = ChainGetEntry(sess->rtsp_commands, 0);
		ChainDeleteEntry(sess->rtsp_commands, 0);
		RTSP_DeleteCommand(com);
	}
	if (sess->wait_for_reply) {
		M4Err e;
		while (1) {
			e = RTSP_GetResponse(sess->session, sess->rtsp_rsp);
			if (e!= M4NetworkEmpty) break;
		}
		sess->wait_for_reply = 0;
	}
	/*no private stack on teardown - shutdown now*/
	com	= RTSP_NewCommand();
	com->method = strdup(RTSP_TEARDOWN);
	ChainAddEntry(sess->rtsp_commands, com);
	MX_V(sess->owner->mx);
}



void RP_UserCommand(RTPSession *sess, RTPStream *ch, NetworkCommand *command)
{
	RTPStream *a_ch;
	ChannelControl *ch_ctrl;
	u32 i;
	RTSPCommand *com;
	RTSPRange *range;

	assert(ch->rtsp==sess);
	
	/*we may need to re-setup stream/session*/
	if ( (command->command_type==CHAN_PLAY) || (command->command_type==CHAN_RESUME) || (command->command_type==CHAN_PAUSE)) {
		if (ch->status == RTP_Disconnected) {
			if (sess->has_aggregated_control) {
				for (i=0; i<ChainGetCount(sess->owner->channels); i++) {
					a_ch = ChainGetEntry(sess->owner->channels, i);
					if (a_ch->rtsp != sess) continue;
					RP_Setup(a_ch);
				}
			} else {
				RP_Setup(ch);
			}
		}
	}
	
	com	= RTSP_NewCommand();
	range = NULL;

	if ( (command->command_type==CHAN_PLAY) || (command->command_type==CHAN_RESUME) ) {

		range = RTSP_NewRange();
		range->start = ch->range_start;
		range->end = ch->range_end;
		
		com->method = strdup(RTSP_PLAY);
		
		/*specify pause range on resume - this is not mandatory but most servers need it*/
		if (command->command_type==CHAN_RESUME) {
			range->start = ch->current_start;

			ch->stat_start_time -= ch->stat_stop_time;
			ch->stat_start_time += M4_GetSysClock();
			ch->stat_stop_time = 0;
		} else {
			range->start = ch->range_start;
			if (command->play.start_range>=0) range->start += (Float) command->play.start_range;
			range->end = ch->range_start;
			if (command->play.end_range >=0) {
				range->end += (Float) command->play.end_range;
				if (range->end > ch->range_end) range->end = ch->range_end;
			}

			ch->stat_start_time = M4_GetSysClock();
			ch->stat_stop_time = 0;
		}
		/*if aggregated the command is sent once, so store info at session level*/
		if (ch->skip_next_command) {
			ch->current_start = ch->rtsp->last_range;
		} else {
			ch->rtsp->last_range = range->start;
			ch->current_start = range->start;
		}
		/*some RTSP servers don't accept Range=npt:0.0- (for ex, broadcast only...), so skip it if:
		- a range was given in initial describe
		- the command is not a RESUME
		*/
		if (!ch->has_range && (command->command_type != CHAN_RESUME) ) {
			RTSP_DeleteRange(range);
			com->Range = NULL;
		} else {
			com->Range = range;
		}

		if (strlen(ch->control)) com->ControlString = strdup(ch->control);

	} else if (command->command_type==CHAN_PAUSE) {
		range = RTSP_NewRange();
		range->start = ch->range_start;
		range->end = ch->range_end;
		com->method = strdup(RTSP_PAUSE);
		/*update current time*/
		ch->current_start += RTP_GetCurrentTime(ch->rtp_ch);
		range->start = ch->current_start;
		range->end = -1.0;
		com->Range = range;

		ch->stat_stop_time = M4_GetSysClock();

	}
	//nb: we could actually send a PAUSE in order to keep the session alive
	//but let's be nice to the server
	else if (command->command_type==CHAN_STOP) {
		ch->current_start = 0;
		ch->idle = 1;

		ch->stat_stop_time = M4_GetSysClock();

		/*last stream running*/
		if (!RP_SessionActive(ch)) {
			ch->idle = 0;
			RP_StopChannel(ch);
			SkipCommandOnSession(ch);
			ch->skip_next_command = 0;
			RTSP_DeleteCommand(com);
			RP_FlushAndTearDown(sess);
		} else {
			ch->skip_next_command = 0;
			if (com) RTSP_DeleteCommand(com);
		}
		return;
	} else {
		NM_OnCommand(sess->owner->service, command, M4ControlNotSupported);
		RTSP_DeleteCommand(com);
		return;
	}

	ch_ctrl = malloc(sizeof(ChannelControl));
	ch_ctrl->ch = ch;
	memcpy(&ch_ctrl->com, command, sizeof(NetworkCommand));
	com->user_data = ch_ctrl;

	MX_P(sess->owner->mx);
	ChainAddEntry(sess->rtsp_commands, com);
	MX_V(sess->owner->mx);

	return;
}


/*
 						session/channel teardown functions
																*/
void RP_ProcessTeardown(RTPSession *sess, RTSPCommand *com, M4Err e)
{
	/*this is a disconnect (channel or service) - nothing to do, status is updated
	before sending the command*/
}

void RP_Teardown(RTPSession *sess)
{
	LPRTSPCOMMAND com;

	if (!RTSP_GetSessionID(sess->session)) return;
	
	com = RTSP_NewCommand();
	com->method = strdup(RTSP_TEARDOWN);
	MX_P(sess->owner->mx);
	ChainAddEntry(sess->rtsp_commands, com);
	MX_V(sess->owner->mx);
}

