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


void RP_ConfirmChannelConnect(RTPStream *ch, M4Err e)
{
	NetworkCommand com;
	
	/*in case the channel has been disconnected while SETUP was issued&processed. We also could
	clean up the command stack*/
	if (!ch->channel) return;

	NM_OnConnect(ch->owner->service, ch->channel, e);
	if (e != M4OK || !ch->rtp_ch) return;

	/*success, overwrite SL config*/
	memset(&com, 0, sizeof(NetworkCommand));
	com.command_type = CHAN_RECONFIG;
	com.base.on_channel = ch->channel;

	com.cfg.sl_config.tag = SLConfigDescriptor_Tag;
	
	com.cfg.sl_config.AULength = ch->sl_map.ConstantSize;
	if (ch->sl_map.ConstantDuration) {
		com.cfg.sl_config.CUDuration = com.cfg.sl_config.AUDuration = ch->sl_map.ConstantDuration;
	} else {
		com.cfg.sl_config.CUDuration = com.cfg.sl_config.AUDuration = ch->unit_duration;
	}
	com.cfg.sl_config.AUSeqNumLength = ch->sl_map.StreamStateIndication;

	/*RTP SN is on 16 bits*/
	com.cfg.sl_config.packetSeqNumLength = 0;
	/*RTP TS is on 32 bits*/
	com.cfg.sl_config.timestampLength = 32;
	ch->clock_rate = RTP_GetTimeScale(ch->rtp_ch);
	com.cfg.sl_config.timeScale = com.cfg.sl_config.timestampResolution = ch->clock_rate;
	com.cfg.sl_config.useTimestampsFlag = 1;

	/*we override these flags because we emulate the flags through the marker bit */
	com.cfg.sl_config.useAccessUnitEndFlag = com.cfg.sl_config.useAccessUnitStartFlag = 1;
	com.cfg.sl_config.useRandomAccessPointFlag = ch->sl_map.RandomAccessIndication;
	/*this is not true however it is needed for RFC3016, otherwise the RAP is never signaled*/
	com.cfg.sl_config.useRandomAccessUnitsOnlyFlag = ch->sl_map.RandomAccessIndication ? 0 : 1;

	/*reconfig*/
	NM_OnCommand(ch->owner->service, &com, M4OK);
}


M4Err RP_InitStream(RTPStream *ch, Bool ResetOnly)
{
	ch->new_au = 1;
	/*3gp reassembler cleanup*/
	if (ch->ttu1) DeleteBitStream(ch->ttu1);
	ch->ttu1 = NULL;

	if (!ResetOnly) {
		memset(&ch->sl_hdr, 0, sizeof(SLHeader));
		return RTP_Initialize(ch->rtp_ch, RTP_BUFFER_SIZE, 0, 0, ch->owner->reorder_size, 200);
	}
	//just reset the sockets
	RTP_ResetBuffers(ch->rtp_ch);
	return M4OK;
}

void RP_DisconnectStream(RTPStream *ch)
{
	if (ch->rtsp && !ch->idle && (ch->status == RTP_Running)) {
		RP_Teardown(ch->rtsp);
	}
	ch->status = RTP_Disconnected;
	ch->connected = 0;
	ch->channel = NULL;
}

void RP_DeleteStream(RTPStream *ch)
{
	if (ch->rtsp) {
		if ((ch->status == RTP_Running)) {
			RP_Teardown(ch->rtsp);
			ch->status = RTP_Disconnected;
		}
		RP_RemoveStream(ch->owner, ch);
	} else {
		RP_FindChannel(ch->owner, ch->channel, 0, NULL, 1);
	}

	if (ch->sl_map.config) free(ch->sl_map.config);
	if (ch->rtp_ch) RTP_DeleteChannel(ch->rtp_ch);
	if (ch->control) free(ch->control);
	if (ch->ttu1) DeleteBitStream(ch->ttu1);
	free(ch);
}


RTPStream *RP_NewStream(RTPClient *rtp, SDPMedia *media, SDPInfo *sdp, RTPStream *input_stream)
{
	RTSPRange *range;
	RTPStream *tmp;
	SDP_RTPMap *map;
	u32 i, ESID, rtp_format;
	Float Start, End;
	X_Attribute *att;
	char *ctrl;
	SDPConnection *conn;
	RTSPTransport trans;

	//extract all relevant info from the SDPMedia
	Start = 0.0;
	End = -1.0;
	ESID = 0;
	ctrl = NULL;
	range = NULL;
	for (i=0; i<ChainGetCount(media->Attributes); i++) {
		att = ChainGetEntry(media->Attributes, i);
		if (!stricmp(att->Name, "control")) ctrl = att->Value;
		else if (!stricmp(att->Name, "mpeg4-esid") && att->Value) ESID = atoi(att->Value);
		else if (!stricmp(att->Name, "range") && !range) range = RTSP_ParseRange(att->Value);
	}

	if (range) {
		Start = range->start;
		End = range->end;
		RTSP_DeleteRange(range);
	}

	/*check connection*/
	conn = sdp->c_connection;
	if (!conn) conn = ChainGetEntry(media->Connections, 0);

	if (!conn) {
		/*RTSP RFC recommends an empty "c= " line but some server don't send it. Use session info (o=)*/
		if (!sdp->o_net_type || !sdp->o_add_type || 
			strcmp(sdp->o_net_type, "IN") || strcmp(sdp->o_add_type, "IP4")) return NULL;
	} else {
		if (strcmp(conn->net_type, "IN") || strcmp(conn->add_type, "IP4")) return NULL;
	}
	/*do we support transport*/
	if (strcmp(media->Profile, "RTP/AVP") && strcmp(media->Profile, "RTP/AVP/TCP")) return NULL; 
	/*check RTP map. For now we only support 1 RTPMap*/
	if (media->fmt_list || (ChainGetCount(media->RTPMaps) > 1)) return NULL;

	/*check payload type*/
	map = ChainGetEntry(media->RTPMaps, 0);

	rtp_format = payt_get_type(rtp, map, media);
	if (!rtp_format) return NULL;


	/*this is an ESD-URL setup, we likely have namespace conflicts so overwrite given ES_ID
	by the app one (client side), but keep control (server side) if provided*/
	if (input_stream) {
		ESID = input_stream->ES_ID;
		if (!ctrl) ctrl = input_stream->control;
		tmp = input_stream;
	} else {
		tmp = RP_FindChannel(rtp, NULL, ESID, NULL, 0);
		if (tmp) return NULL;

		tmp = malloc(sizeof(RTPStream));
		memset(tmp, 0, sizeof(RTPStream));
		tmp->owner = rtp;
	}
	tmp->rtptype = rtp_format;

	/*create an RTP channel*/
	tmp->rtp_ch = RTP_NewChannel();
	if (ctrl) tmp->control = strdup(ctrl);
	tmp->ES_ID = ESID;
	RTP_SetLog(tmp->rtp_ch, rtp->logs);


	memset(&trans, 0, sizeof(RTSPTransport));
	if (conn && SK_IsMulticastAddress(conn->host)) {
		trans.source = conn->host;
		trans.TTL = conn->TTL;
		trans.port_first = media->PortNumber;
		trans.port_last = media->PortNumber + 1;
		trans.Profile = media->Profile;
	} else {
		trans.source = conn? conn->host : sdp->o_address;
		trans.IsUnicast = 1;
		trans.client_port_first = media->PortNumber;
		trans.client_port_last = media->PortNumber + 1;
		if (rtp->rtp_mode) {
			trans.Profile = RTSP_PROFILE_RTP_AVP_TCP;
		} else {
			trans.Profile = media->Profile;
		}
	}
	RTP_SetupTransport(tmp->rtp_ch, &trans, NULL);

	//setup our RTP map
	payt_setup(tmp, map, media);

//	tmp->status = NM_Disconnected;

	tmp->range_start = Start;
	tmp->range_end = End;
	if (End != -1.0) tmp->has_range = 1;
	return tmp;
}




void RP_ProcessRTP(RTPStream *ch, char *pck, u32 size)
{
	NetworkCommand com;
	M4Err e;
	RTPHeader hdr;
	u32 PayloadStart;
	Float ft;

	ch->rtp_bytes += size;

	/*first decode RTP*/
	e = RTP_DecodePacket(ch->rtp_ch, pck, size, &hdr, &PayloadStart);

	/*corrupted or NULL data*/
	if (e || (PayloadStart >= size)) {
		//NM_OnSLPRecieved(ch->owner->service, ch->channel, NULL, 0, NULL, M4CorruptedData);
		return;
	}

	/*if we must notify some timing, do it now. If the channel has no range, this should NEVER be called*/
	if (ch->check_rtp_time /*&& RTP_IsActive(ch->rtp_ch)*/) {

		ft = RTP_GetCurrentTime(ch->rtp_ch);

		/*this is the first packet on the channel (no PAUSE)*/
		if (ch->check_rtp_time == 1) {
			/*Note: in a SEEK with RTSP, the rtp-info time given by the server is 
			the rtp time of the desired range. But the server may (and should) send from
			the previous I frame on video, so the time of the first rtp packet after
			a SEEK can actually be less than CurrentStart. We don't drop these
			packets in order to see the maximum video. We could drop it, this would mean 
			wait for next RAP...*/

			memset(&com, 0, sizeof(com));
			com.command_type = CHAN_MAP_TIME;
			com.base.on_channel = ch->channel;
			com.map_time.media_time = ch->current_start + ft;
			com.map_time.timestamp = hdr.TimeStamp;
			com.map_time.reset_buffers = 1;
			NM_OnCommand(ch->owner->service, &com, M4OK);
		}
		/*this is RESUME on channel, filter packet based on time (darwin seems to send
		couple of packet before)
		do not fetch if we're below 10 ms or <0, because this means we already have
		this packet - as the PAUSE is issued with the RTP currentTime*/
		else if (ft <= 0.021) {
			return;
		}
		ch->check_rtp_time = 0;
	}
	switch (ch->rtptype) {
	case RTP_PAYT_MPEG4:
		RP_ParsePayloadMPEG4(ch, &hdr, pck + PayloadStart, size - PayloadStart);
		break;
	case RTP_PAYT_MPEG12:
		RP_ParsePayloadMPEG12(ch, &hdr, pck + PayloadStart, size - PayloadStart);
		break;
	case RTP_PAYT_AMR:
	case RTP_PAYT_AMR_WB:
		RP_ParsePayloadAMR(ch, &hdr, pck + PayloadStart, size - PayloadStart);
		break;
	case RTP_PAYT_H263:
		RP_ParsePayloadH263(ch, &hdr, pck + PayloadStart, size - PayloadStart);
		break;
	case RTP_PAYT_3GPP_TEXT:
		RP_ParsePayloadText(ch, &hdr, pck + PayloadStart, size - PayloadStart);
		break;
	}

	/*last check: signal EOS if we're close to end range in case the server do not send RTCP BYE*/
	if (ch->has_range && !ch->end_of_stream) {
		/*also check last CTS*/
		Float ts = (Float) ((u32) ch->sl_hdr.compositionTimeStamp - hdr.TimeStamp);
		ts /= ch->clock_rate;
		if (ABSDIFF(ch->range_end, (ts + ch->current_start + RTP_GetCurrentTime(ch->rtp_ch)) ) < 0.2) {
			ch->end_of_stream = 1;
			ch->stat_stop_time = M4_GetSysClock();
			NM_OnSLPRecieved(ch->owner->service, ch->channel, NULL, 0, NULL, M4EOF);
		}
	}
}

void RP_ProcessRTCP(RTPStream *ch, char *pck, u32 size)
{
	M4Err e;

	ch->rtcp_bytes += size;

	e = RTCP_DecodePacket(ch->rtp_ch, pck, size);
	
	if (e == M4EOF) {
		ch->end_of_stream = 1;
		ch->stat_stop_time = M4_GetSysClock();
		NM_OnSLPRecieved(ch->owner->service, ch->channel, NULL, 0, NULL, M4EOF);
	}
}

M4Err RP_DataOnTCP(LPRTSPSESSION sess, void *cbk, char *buffer, u32 bufferSize, Bool IsRTCP)
{
	RTPStream *ch = (RTPStream *) cbk;
	if (!ch) return M4OK;
	if (IsRTCP) {
		RP_ProcessRTCP(ch, buffer, bufferSize);
	} else {
		RP_ProcessRTP(ch, buffer, bufferSize);
	}
	return M4OK;
}


static M4Err SendTCPData(void *par, char *pck, u32 pck_size)
{
	return M4OK;
}


void RP_ReadStream(RTPStream *ch)
{
	u32 size, tot_size;

	if (!ch->rtp_ch) return;

	/*NOTE: A weird bug on windows wrt to select(): if both RTP and RTCP are in the same loop
	there is a hudge packet drop on RTP. We therefore split RTP and RTCP reading, this is not a big
	deal as the RTCP traffic is far less than RTP, and we should never have more than one RTCP
	packet reading per RTP reading loop
	NOTE2: a better implementation would be to use select() to get woken up...
	*/

	tot_size = 0;
	while (1) {
		size = RTP_ReadChannel(ch->rtp_ch, ch->buffer, RTP_BUFFER_SIZE);
		if (!size) break;
		tot_size += size;
		RP_ProcessRTP(ch, ch->buffer, size);
	}

	while (1) {
		size = RTCP_ReadChannel(ch->rtp_ch, ch->buffer, RTP_BUFFER_SIZE);
		if (!size) break;
		tot_size += size;
		RP_ProcessRTCP(ch, ch->buffer, size);
	}

	/*and send the report*/
	RTCP_SendReport(ch->rtp_ch, SendTCPData, ch);
	
	if (tot_size) ch->owner->udp_time_out = 0;

	/*detect timeout*/
	if (ch->owner->udp_time_out) {
		if (!ch->last_udp_time) {
			ch->last_udp_time = M4_GetSysClock();
		} else {
			u32 diff = M4_GetSysClock() - ch->last_udp_time;
			if (diff >= ch->owner->udp_time_out) {
				char szMessage[1024];
				sprintf(szMessage, "No data recieved in %d ms", diff);
				NM_OnMessage(ch->owner->service, M4UDPTimeOut, szMessage);
				ch->status = RTP_Unavailable;
			}
		}
	}
}

