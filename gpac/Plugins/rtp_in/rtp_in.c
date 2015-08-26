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

static void RT_LoadPrefs(NetClientPlugin *plug, RTPClient *rtp)
{
	char *sOpt;

	sOpt = PMI_GetOpt(plug, "Streaming", "DefaultPort");
	if (sOpt) {
		rtp->default_port = atoi(sOpt);
	} else {
		rtp->default_port = 554;
	}
	if ((rtp->default_port == 80) || (rtp->default_port == 8080))
		PMI_SetOpt(plug, "Streaming", "RTPoverRTSP", "yes");
	
	sOpt = PMI_GetOpt(plug, "Streaming", "RTPoverRTSP");
	if (sOpt && !stricmp(sOpt, "yes")) {
		rtp->rtp_mode = 1;
		/*HTTP tunnel*/
		if ((rtp->default_port==80) || (rtp->default_port==8080)) rtp->rtp_mode = 2;
	} else {
		rtp->rtp_mode = 0;
	}


	/*
		get heneral network config for UDP
	*/
	/*if UDP not available don't try it*/
	sOpt = PMI_GetOpt(plug, "Network", "UDPNotAvailable");
	if (!rtp->rtp_mode && sOpt && !stricmp(sOpt, "yes")) rtp->rtp_mode = 1;
	
	if (!rtp->rtp_mode) {
		sOpt = PMI_GetOpt(plug, "Network", "UDPTimeout");
		if (sOpt ) {
			rtp->udp_time_out = atoi(sOpt);
		} else {
			rtp->udp_time_out = 10000;
		}
	}

	sOpt = PMI_GetOpt(plug, "Network", "BufferLength");
	if (sOpt) {
		rtp->buffer_len_ms = atoi(sOpt);
	} else {
		rtp->buffer_len_ms = 1000;
	}
	rtp->rebuffer_len_ms = 0;
	sOpt = PMI_GetOpt(plug, "Network", "RebufferLength");
	if (sOpt) rtp->rebuffer_len_ms = atoi(sOpt);
	
	rtp->reorder_size = 30;
	if (!rtp->rtp_mode) {
		sOpt = PMI_GetOpt(plug, "Streaming", "ReorderSize");
		if (sOpt) rtp->reorder_size = atoi(sOpt);
	}

	sOpt = PMI_GetOpt(plug, "Streaming", "RTSPTimeout");
	if (sOpt ) {
		rtp->time_out = atoi(sOpt);
	} else {
		rtp->time_out = 30000;
	}

	/*packet drop emulation*/
	sOpt = PMI_GetOpt(plug, "Streaming", "FirstPacketDrop");
	if (sOpt) {
		rtp->first_packet_drop = atoi(sOpt);
	} else {
		rtp->first_packet_drop = 0;
	}
	sOpt = PMI_GetOpt(plug, "Streaming", "PacketDropFrequency");
	if (sOpt) {
		rtp->frequency_drop = atoi(sOpt);
	} else {
		rtp->frequency_drop = 0;
	}
	
	sOpt = PMI_GetOpt(plug, "Streaming", "LogFile");
	if (sOpt && strlen(sOpt)) {
		if (!stricmp(sOpt, "stdout"))
			rtp->logs = stdout;
		else
			rtp->logs = fopen(sOpt, "wt");
	} else {
		rtp->logs = NULL;
	}

	rtp->handle_announce = 0;
	rtp->bandwidth = 10000000;
}

static void RP_close_service_thread(RTPClient *rtp)
{
	if (rtp->rtsp_session) 
		RP_RemoveSession(rtp->rtsp_session, 1);

	rtp->rtsp_session = NULL;
	
	if (rtp->session_iod) free(rtp->session_iod);
	rtp->session_iod = NULL;
	rtp->session_iod_len = 0;

	if (rtp->sdp_temp) {
		free(rtp->sdp_temp->remote_url);
		free(rtp->sdp_temp);
	}
	rtp->sdp_temp = NULL;
	if (rtp->od_au) free(rtp->od_au);
	rtp->od_au = NULL;
	rtp->od_au_size = 0;

	if (rtp->do_exit==1) NM_OnDisconnect(rtp->service, NULL, M4OK);
	rtp->run_client = 0;
}

u32 RP_Thread(void *param)
{
	u32 i, nb_inter;
	NetworkCommand com;
	RTPStream *ch;
	RTPClient *rtp = param;

	com.command_type = CHAN_BUFFER_QUERY;
	while (rtp->run_client) {
		MX_P(rtp->mx);

		/*fecth data on udp*/
		nb_inter = 0;
		for (i=0; i<ChainGetCount(rtp->channels); i++) {
			ch = ChainGetEntry(rtp->channels, i);
			if (ch->end_of_stream || (ch->status!=RTP_Running) ) continue;
			/*for interleaved channels don't read too fast, query the buffer occupancy*/
			if (ch->is_interleaved) {
				com.base.on_channel = ch->channel;
				NM_OnCommand(rtp->service, &com, M4OK);
				/*if no buffering, use a default value (3 sec of data should do it)*/
				if (!com.buffer.max) com.buffer.max = 3000;
				if (com.buffer.occupancy <= com.buffer.max) nb_inter++;
			} else {
				RP_ReadStream(ch);
			}
		}
		MX_V(rtp->mx);
		
		/*and process commands / flush TCP*/
		if (rtp->rtsp_session) RP_ProcessCommands(rtp->rtsp_session, nb_inter);

		if (rtp->do_exit) RP_close_service_thread(rtp);

		Sleep(1);
	}

	if (rtp->dnload) NM_CloseDownloader(rtp->dnload);
	rtp->dnload = NULL;

	rtp->client_exit = 1;
	return 0;
}


static Bool RP_CanHandleURL(NetClientPlugin *plug, const char *url)
{
	char *sExt = strrchr(url, '.');

	if (sExt && NM_CheckExtension(plug, "application/sdp", "sdp", "OnDemand Media/Multicast Session", sExt)) return 1;

	/*local */
	if (strstr(url, "data:application/sdp")) return 1;
	/*embedded data*/
	if (strstr(url, "data:application/mpeg4-od-au;base64") || 
		strstr(url, "data:application/mpeg4-bifs-au;base64") ||
		strstr(url, "data:application/mpeg4-es-au;base64")) return 1;

	/*we need rtsp/tcp , rtsp/udp or direct RTP sender (no control)*/
	if (!strnicmp(url, "rtsp://", 7) || !strnicmp(url, "rtspu://", 8) || !strnicmp(url, "rtp://", 6)) return 1;
	/*we don't check extensions*/
	return 0;
}

static M4Err RP_ConnectService(NetClientPlugin *plug, LPNETSERVICE serv, const char *url)
{
	RTPSession *sess;
	RTPClient *priv = plug->priv;

	/*store user address*/
	priv->service = serv;

	if (priv->dnload) NM_CloseDownloader(priv->dnload);
	priv->dnload = NULL;


	/*load preferences*/
	RT_LoadPrefs(plug, priv);

	/*start thread*/
	priv->run_client = 1;
	TH_Run(priv->th, RP_Thread, priv);

	/*local or remote SDP*/
	if (strstr(url, "data:application/sdp") || (strnicmp(url, "rtsp", 4) && strstr(url, ".sdp")) ) {
		RP_FetchSDP(plug, (char *) url, NULL);
	}
	/*rtsp and rtsp over udp*/
	else if (!strnicmp(url, "rtsp://", 7) || !strnicmp(url, "rtspu://", 8)) {
		sess = RP_NewSession(priv, (char *) url);
		if (!sess) {
			NM_OnConnect(serv, NULL, M4UnsupportedURL);
		} else {
			RP_Describe(sess, 0, NULL);
		}
	}
	/*direct RTP (no control) or embedded data - this means the service is attached to a single channel (no IOD)
	reply right away*/
	else {
		NM_OnConnect(serv, NULL, M4OK);
	}
	return M4OK;
}

static M4Err RP_CloseService(NetClientPlugin *plug, Bool immediate_shutdown)
{
	RTPClient *priv = plug->priv;
	priv->do_exit = 1;
	return M4OK;
}

static M4Err RP_Get_MPEG4_IOD(NetClientPlugin *plug, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size)
{
	M4Err e;
	RTPClient *priv = plug->priv;

	if ((expect_type!=NM_OD_UNDEF) && (expect_type!=NM_OD_SCENE)) {
		/*ignore the SDP IOD and regenerate one*/
		if (priv->session_iod) free(priv->session_iod);
		priv->session_iod = NULL;
		priv->session_iod_len = 0;
		e = RP_EmulateIOD(priv, expect_type, sub_url);
		if (e) return e;
	}

	if (!priv->session_iod || !priv->session_iod_len) 
		return M4ServiceError;
	*raw_iod = malloc(sizeof(char)*priv->session_iod_len);
	memcpy(*raw_iod, priv->session_iod, priv->session_iod_len);
	*raw_iod_size = priv->session_iod_len;
	return M4OK;
}

static M4Err RP_ConnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel, const char *url, Bool upstream)
{
	u32 ESID;
	RTPStream *ch;
	RTPSession *sess;
	char *es_url;
	RTPClient *priv = plug->priv;
	if (upstream) return M4NotSupported;


	ch = RP_FindChannel(priv, channel, 0, (char *) url, 0);
	if (ch && (ch->status != RTP_Disconnected) ) return M4ServiceError;

	es_url = NULL;
	sess = NULL;
	if (strstr(url, "ES_ID=")) {
		sscanf(url, "ES_ID=%d", &ESID);
		if (ESID==priv->od_es_id) {
			priv->od_ch = channel;
			NM_OnConnect(priv->service, channel, M4OK);
			return M4OK;
		} else {
			/*first case: simple URL (same namespace)*/
			ch = RP_FindChannel(priv, NULL, ESID, NULL, 0);
			/*this should not happen, the sdp must describe all streams in the service*/
			if (!ch) return M4ServiceError;
			
			/*assign app channel*/
			ch->channel = channel;
			sess = ch->rtsp;
		}
	}
	/*rtsp url - create a session if needed*/
	else if (!strnicmp(url, "rtsp://", 7) || !strnicmp(url, "rtspu://", 8)) {
		sess = RP_CheckSession(priv, (char *) url);
		if (!sess) sess = RP_NewSession(priv, (char *) url);
		es_url = (char *) url;
	}
	/*data: url*/
	else if (strstr(url, "data:application/mpeg4-od-au;base64") 
		|| strstr(url, "data:application/mpeg4-bifs-au;base64")
		|| strstr(url, "data:application/mpeg4-es-au;base64")
		) {
		
		ch = malloc(sizeof(RTPStream));
		memset(ch, 0, sizeof(RTPStream));
		ch->control = strdup(url);
		ch->owner = priv;
		ch->channel = channel;
		ch->status = RTP_Connected;
		/*register*/
		ChainAddEntry(priv->channels, ch);
		RP_ConfirmChannelConnect(ch, M4OK);

		return M4OK;
	}
	
	/*send a DESCRIBE (not a setup) on the channel. If the channel is already created then the
	describe is skipped and a SETUP is sent directly, otherwise the channel is first described then setup*/

	if (!sess) return M4InvalidURL;
	RP_Describe(sess, es_url, channel);

	return M4OK;
}


static M4Err RP_DisconnectChannel(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	RTPStream *ch;
	RTPClient *priv = plug->priv;

	if (priv->od_ch==channel) {
		priv->od_ch = NULL;
		NM_OnDisconnect(priv->service, channel, M4OK);
		return M4OK;
	}
	ch = RP_FindChannel(priv, channel, 0, NULL, 0);
	if (!ch) return M4ChannelNotFound;

	MX_P(priv->mx);
	/*disconnect stream BUT DO NOT DELETE IT since we don't store SDP*/
	RP_DisconnectStream(ch);
	MX_V(priv->mx);
	NM_OnDisconnect(priv->service, channel, M4OK);
	return M4OK;
}

static M4Err RP_ServiceCommand(NetClientPlugin *plug, NetworkCommand *com)
{
	RTPStream *ch;
	RTPClient *priv = plug->priv;

	/*ignore commands other than channels one*/
	if (!com->base.on_channel) return M4OK;

	if (priv->od_ch==com->base.on_channel) {
		switch (com->command_type) {
		case CHAN_SET_PULL: return M4OK;
		case CHAN_BUFFER: com->buffer.max = com->buffer.min = 0; return M4OK;
		case CHAN_DURATION: com->duration.duration = 0; return M4OK;
		case CHAN_PLAY:
			priv->od_fetched = 0;
			priv->od_start_time = (u32) (com->play.start_range*1000);
			return M4OK;
		case CHAN_STOP:
			priv->od_start_time = 0;
			break;
		}
		return M4OK;
	}

	ch = RP_FindChannel(priv, com->base.on_channel, 0, NULL, 0);
	if (!ch) return M4ChannelNotFound;

	switch (com->command_type) {
	case CHAN_SET_PULL:
		if (ch->rtp_ch || ch->rtsp || !ch->control) return M4NotSupported;
		/*embedded channels work in pull mode*/
		if (strstr(ch->control, "data:application/")) return M4OK;
		return M4NotSupported;
	case CHAN_INTERACTIVE:
		/*looks like pure RTP / multicast etc, not interactive*/
		if (!ch->control) return M4NotSupported;
		return M4OK;
	case CHAN_BUFFER:
		if (!(ch->rtp_ch || ch->rtsp || !ch->control)) {
		  com->buffer.max = com->buffer.min = 0;
		} else {
		  com->buffer.max = priv->buffer_len_ms;
		  com->buffer.min = priv->rebuffer_len_ms;
		  if (priv->rebuffer_len_ms>=priv->buffer_len_ms) com->buffer.min = 0;
		}
		return M4OK;
	case CHAN_DURATION:
		com->duration.duration = ch->has_range ? (ch->range_end - ch->range_start) : 0;
		return M4OK;
	/*RTP channel config is done upon connection, once the complete SL mapping is known
	however we must store some info not carried in SDP*/
	case CHAN_CONFIG:
		ch->clock_rate = com->cfg.sl_config.timestampResolution;
		if (!ch->clock_rate) ch->clock_rate = 1000;
		ch->unit_duration = com->cfg.frame_duration;
		return M4OK;

	case CHAN_PLAY:
		/*is this RTSP or direct RTP?*/
		ch->end_of_stream = 0;
		if (ch->rtsp) {
			RP_UserCommand(ch->rtsp, ch, com);
		} else {
			ch->status = RTP_Running;
			if (ch->rtp_ch) {
				ch->check_rtp_time = 1;
				RP_InitStream(ch, 1);
				RTP_SetInfo(ch->rtp_ch, 0, 0, 0);
			} else {
				/*direct channel, store current start*/
				ch->current_start = (Float) com->play.start_range;
				ch->new_au = 1;
			}
		}
		return M4OK;
	case CHAN_STOP:
		/*is this RTSP or direct RTP?*/
		if (ch->rtsp) {
			RP_UserCommand(ch->rtsp, ch, com);
		} else {
			ch->status = RTP_Connected;
		}
		return M4OK;
	case CHAN_SET_SPEED:
	case CHAN_PAUSE:
	case CHAN_RESUME:
		if (!ch->rtsp) return M4ControlNotSupported;
		RP_UserCommand(ch->rtsp, ch, com);
		return M4OK;

	case CHAN_GET_DSI:
		if (ch->sl_map.configSize) {
			com->get_dsi.dsi_len = ch->sl_map.configSize;
			com->get_dsi.dsi = malloc(sizeof(char)*com->get_dsi.dsi_len);
			memcpy(com->get_dsi.dsi, ch->sl_map.config, sizeof(char)*com->get_dsi.dsi_len);
		} else {
			com->get_dsi.dsi = NULL;
			com->get_dsi.dsi_len = 0;
		}
		return M4OK;

	
	case NM_NETSTAT:
		memset(&com->net_stats, 0, sizeof(NetStatCom));
		if (ch->rtp_ch) {
			u32 time;
			Float bps;
			com->net_stats.pck_loss_percentage = RTP_GetLossPercentage(ch->rtp_ch);
			if (ch->is_interleaved) {
				com->net_stats.multiplex_port = RTSP_GetSessionPort(ch->rtsp->session);
				com->net_stats.port = RTP_GetLowInterleavedID(ch->rtp_ch);
				com->net_stats.ctrl_port = RTP_GetHighInterleavedID(ch->rtp_ch);
			} else {
				com->net_stats.multiplex_port = 0;
				RTP_GetPorts(ch->rtp_ch, &com->net_stats.port, &com->net_stats.ctrl_port);
			}
			if (ch->stat_stop_time) {
				time = ch->stat_stop_time - ch->stat_start_time;
			} else {
				time = M4_GetSysClock() - ch->stat_start_time;
			}
			bps = 8.0f * ch->rtp_bytes; bps *= 1000; bps /= time; com->net_stats.bw_down = (u32) bps;
			bps = 8.0f * ch->rtcp_bytes; bps *= 1000; bps /= time; com->net_stats.ctrl_bw_down = (u32) bps;
			bps = 8.0f * RTP_GetRTCPBytesSent(ch->rtp_ch); bps *= 1000; bps /= time; com->net_stats.ctrl_bw_up = (u32) bps;
		}
		return M4OK;
	}
	return M4NotSupported;
}

static M4Err RP_ChannelGetSLP(NetClientPlugin *plug, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data)
{
	char *data;
	RTPStream *ch;
	RTPClient *priv = plug->priv;

	if (priv->od_ch==channel) {
		*sl_compressed = 0;
		memset(out_sl_hdr, 0, sizeof(SLHeader));
		out_sl_hdr->accessUnitEndFlag = 1;
		out_sl_hdr->accessUnitStartFlag = 1;
		out_sl_hdr->compositionTimeStamp = priv->od_start_time;
		out_sl_hdr->compositionTimeStampFlag = 1;
		out_sl_hdr->randomAccessPointFlag = 1;
		if (priv->od_fetched) {
			*out_reception_status = M4EOF;
		} else {
			*out_reception_status = M4OK;
			*is_new_data = 1;
			*out_data_ptr = priv->od_au;
			*out_data_size = priv->od_au_size;
		}
		return M4OK;
	}

	ch = RP_FindChannel(priv, channel, 0, NULL, 0);
	if (!ch) return M4ChannelNotFound;
	if (ch->rtp_ch || ch->rtsp || !ch->control) return M4ServiceError;
	if (ch->status != RTP_Running) return M4ServiceError;
	data = strstr(ch->control, ";base64");
	if (!data) return M4ServiceError;

	if (ch->current_start>=0) {
		*sl_compressed = 0;
		memset(out_sl_hdr, 0, sizeof(SLHeader));
		out_sl_hdr->accessUnitEndFlag = 1;
		out_sl_hdr->accessUnitStartFlag = 1;
		out_sl_hdr->compositionTimeStamp = (u64) (ch->current_start * ch->clock_rate);
		out_sl_hdr->compositionTimeStampFlag = 1;
		out_sl_hdr->randomAccessPointFlag = 1;
		*out_reception_status = M4OK;
		*is_new_data = ch->new_au;

		/*decode data*/
		data = strstr(data, ",");
		data += 1;
		*out_data_size = Base64_dec(data, strlen(data), ch->buffer, RTP_BUFFER_SIZE);
		/*FIXME - currently only support for empty SL header*/
		*out_data_ptr = ch->buffer;
		ch->new_au = 0;
	} else {
		*out_data_ptr = NULL;
		*out_data_size = 0;
		*out_reception_status = M4EOF;
		ch->end_of_stream = 1;
	}
	return M4OK;
}

static M4Err RP_ChannelReleaseSLP(NetClientPlugin *plug, LPNETCHANNEL channel)
{
	RTPStream *ch;
	RTPClient *priv = plug->priv;
	if (priv->od_ch==channel) {
		priv->od_fetched = 1;
		return M4OK;
	}
	ch = RP_FindChannel(priv, channel, 0, NULL, 0);
	if (!ch) return M4ChannelNotFound;
	if (ch->rtp_ch || ch->rtsp || !ch->control) return M4ServiceError;
	if (ch->status != RTP_Running) return M4ServiceError;

	/*this will trigger EOS at next fetch*/
	ch->current_start = -1.0;
	return M4OK;
}

static Bool RP_CanHandleURLInService(NetClientPlugin *plug, const char *url)
{
	RTPSession *sess;
	RTPClient *priv = plug->priv;

	if (strstr(url, "data:application/mpeg4-od-au;base64") 
		|| strstr(url, "data:application/mpeg4-bifs-au;base64")
		|| strstr(url, "data:application/mpeg4-es-au;base64")
		) return 1;

	if (!RP_CanHandleURL(plug, url)) return 0;
	/*if this URL is part of a running session then ok*/
	sess = RP_CheckSession(priv, (char *) url);
	if (sess) return 1;
	return 0;
}


NetClientPlugin *RTP_LoadPlugin()
{
	RTPClient *priv;
	NetClientPlugin *plug = malloc(sizeof(NetClientPlugin));
	memset(plug, 0, sizeof(NetClientPlugin));
	M4_REG_PLUG(plug, M4STREAMINGCLIENT, "GPAC RTP/RTSP Client", "gpac distribution", 0)

	plug->CanHandleURL = RP_CanHandleURL;
	plug->CanHandleURLInService = RP_CanHandleURLInService;
	plug->ConnectService = RP_ConnectService;
	plug->CloseService = RP_CloseService;
	plug->Get_MPEG4_IOD = RP_Get_MPEG4_IOD;
	plug->ConnectChannel = RP_ConnectChannel;
	plug->DisconnectChannel = RP_DisconnectChannel;
	plug->ServiceCommand = RP_ServiceCommand;

	/*PULL mode for embedded streams*/
	plug->ChannelGetSLP = RP_ChannelGetSLP;
	plug->ChannelReleaseSLP = RP_ChannelReleaseSLP;

	priv = malloc(sizeof(RTPClient));
	memset(priv, 0, sizeof(RTPClient));
	priv->channels = NewChain();

	plug->priv = priv;
	
	priv->time_out = 30000;
	priv->buffer_len_ms = 5000;
	priv->rebuffer_len_ms = 500;

	priv->mx = NewMutex();
	priv->th = NewThread();
	
	M4_InitClock();
	return plug;
}

void RTP_Delete(BaseInterface *bi)
{
	RTPClient *priv;
	u32 retry;
	NetClientPlugin *plug = (NetClientPlugin *) bi;
	priv = plug->priv;
	if (priv->run_client) {
		if (!priv->do_exit) priv->do_exit = 2;
		retry = 20;
		while (!priv->client_exit && retry) {
			Sleep(10);
			retry--;
		}
		assert(retry);
	}

	/*delete all streams*/
	while (ChainGetCount(priv->channels)) {
		RTPStream *ch = ChainGetEntry(priv->channels, 0);
		ChainDeleteEntry(priv->channels, 0);
		RP_DeleteStream(ch);
	}

	if (priv->rtsp_session) RP_RemoveSession(priv->rtsp_session, 1);
	if (priv->session_iod) free(priv->session_iod);
	priv->session_iod = NULL;
	priv->session_iod_len = 0;
	if (priv->sdp_temp) {
		free(priv->sdp_temp->remote_url);
		free(priv->sdp_temp);
	}

	if (priv->logs && (priv->logs != stdout) ) fclose(priv->logs);

	DeleteChain(priv->channels);
	TH_Delete(priv->th);
	MX_Delete(priv->mx);
	free(priv);
	free(bi);

	M4_StopClock();
}

Bool QueryInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType) 
{
	if (InterfaceType == M4STREAMINGCLIENT) return RTP_LoadPlugin();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *ptr = (BaseInterface *)ifce;
	switch (ptr->InterfaceType) {
	case M4STREAMINGCLIENT:
		RTP_Delete(ptr);
		break;
	}
}
