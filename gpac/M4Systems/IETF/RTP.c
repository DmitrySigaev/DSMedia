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

#define MAX_RTP_SN	0x10000


RTPChannel *RTP_NewChannel()
{
	RTPChannel *tmp;
	tmp = malloc(sizeof(RTPChannel));
	memset(tmp, 0, sizeof(RTPChannel));

	tmp->first_rtp_pck = 1;
	tmp->first_SR = 1;
	tmp->SSRC = M4_Rand();
	
	return tmp;
}

void RTP_DeleteChannel(RTPChannel *ch)
{
	if (!ch) return;
	if (ch->rtp) SK_Delete(ch->rtp);
	if (ch->rtcp) SK_Delete(ch->rtcp);
	if (ch->net_info.source) free(ch->net_info.source);
	if (ch->net_info.destination) free(ch->net_info.destination);
	if (ch->net_info.Profile) free(ch->net_info.Profile);
	if (ch->po) PO_Delete(ch->po);
	if (ch->send_buffer) free(ch->send_buffer);

	if (ch->CName) free(ch->CName);
	if (ch->s_name) free(ch->s_name);
	if (ch->s_email) free(ch->s_email);
	if (ch->s_location) free(ch->s_location);
	if (ch->s_phone) free(ch->s_phone);
	if (ch->s_tool) free(ch->s_tool);
	if (ch->s_note) free(ch->s_note);
	if (ch->s_priv) free(ch->s_priv);
	free(ch);
}



M4Err RTP_SetupTransport(RTPChannel *ch, RTSPTransport *trans_info, char *remote_address)
{
	if (!ch || !trans_info) return M4BadParam;
	//assert we have at least ONE source ID
	if (!trans_info->source && !remote_address) return M4InvalidPeerAddress;

	if (ch->net_info.destination) free(ch->net_info.destination);
	if (ch->net_info.Profile) free(ch->net_info.Profile);
	if (ch->net_info.source) free(ch->net_info.source);
	memcpy(&ch->net_info, trans_info, sizeof(RTSPTransport));

	if (trans_info->destination) 
		ch->net_info.destination = strdup(trans_info->destination);

	if (trans_info->Profile) 
		ch->net_info.Profile = strdup(trans_info->Profile);

	if (trans_info->source) {
		ch->net_info.source = strdup(trans_info->source);
	} else {
		ch->net_info.source = strdup(remote_address);
	}
	//check we REALLY have unicast or multicast
	if (SK_IsMulticastAddress(ch->net_info.source) && ch->net_info.IsUnicast) return M4ServiceError;
	return M4OK;
}


void RTP_ResetBuffers(RTPChannel *ch)
{
	if (ch->rtp) SK_Reset(ch->rtp);
	if (ch->rtcp) SK_Reset(ch->rtcp);
	if (ch->po) PO_Reset(ch->po);
	/*also reset ssrc*/
	ch->SenderSSRC = 0;
	ch->first_SR = 1;
}


M4Err RTP_SetInfo(RTPChannel *ch, u32 seq_num, u32 rtp_time, u32 ssrc)
{
	if (!ch) return M4BadParam;
	ch->rtp_time = rtp_time;
	ch->last_pck_sn = 0;
	ch->rtp_first_SN = seq_num;
	ch->num_sn_loops = 0;
	//reset RTCP
	ch->first_rtp_pck = 1;
	ch->first_SR = 1;
	ch->SenderSSRC = ssrc;
	ch->last_num_pck_rcv = ch->last_num_pck_expected = ch->last_num_pck_loss = ch->tot_num_pck_rcv = ch->tot_num_pck_expected = ch->rtcp_bytes_sent = 0;
	return M4OK;
}

M4Err RTP_Initialize(RTPChannel *ch, u32 UDPBufferSize, Bool IsSource, u32 PathMTU, u32 ReorederingSize, u32 MaxReorderDelay)
{
	M4Err e;

	if (IsSource && !PathMTU) return M4BadParam;

	if (ch->rtp) SK_Delete(ch->rtp);
	if (ch->rtcp) SK_Delete(ch->rtcp);
	if (ch->po) PO_Delete(ch->po);

	ch->CurrentTime = 0;
	ch->rtp_time = 0;

	//create sockets for RTP/AVP profile only
	if (ch->net_info.Profile && 
		( !stricmp(ch->net_info.Profile, RTSP_PROFILE_RTP_AVP) || !stricmp(ch->net_info.Profile, "RTP/AVP/UDP"))
		) {
		//destination MUST be specified for unicast
		if (IsSource && ch->net_info.IsUnicast && !ch->net_info.destination) return M4BadParam;

		//
		//	RTP
		//
		ch->rtp = NewSocket(SK_TYPE_UDP);
		if (!ch->rtp) return M4NetworkFailure;
		if (ch->net_info.IsUnicast) {
			//if client, bind and connect the socket
			if (!IsSource) {
				e = SK_Bind(ch->rtp, ch->net_info.client_port_first, 1);
				if (e) return e;
				e = SK_Connect(ch->rtp, ch->net_info.source, ch->net_info.port_first);
				if (e) return e;
			}
			//else bind and set remote destination
			else {
				e = SK_Bind(ch->rtp, ch->net_info.port_first, 1);
				if (e) return e;
				e = SK_SetRemoteAddress(ch->rtp, ch->net_info.destination);
				if (e) return e;
				e = SK_SetRemotePort(ch->rtp, ch->net_info.client_port_first);
				if (e) return e;
			}
		} else {
			//Bind to multicast (auto-join the group). 
			//we do not bind the socket if this is a source-only channel because some servers
			//don't like that on local loop ...
			e = SK_MulticastSetup(ch->rtp, ch->net_info.source, ch->net_info.port_first, ch->net_info.TTL, (IsSource==2));
			if (e) return e;
		
			//destination is used for multicast interface addressing - TO DO

		}
		if (UDPBufferSize) SK_SetBufferSize(ch->rtp, IsSource, UDPBufferSize);

		if (IsSource) {
			if (ch->send_buffer) free(ch->send_buffer);
			ch->send_buffer = malloc(sizeof(char) * PathMTU);
			ch->send_buffer_size = PathMTU;
		}
		

		//create re-ordering queue for UDP only, and recieve
		if (ReorederingSize && !IsSource) {
			if (!MaxReorderDelay) MaxReorderDelay = 200;
			ch->po = NewPckReoreder(ReorederingSize, MaxReorderDelay);
		}

		//
		//	RTCP
		//
		ch->rtcp = NewSocket(SK_TYPE_UDP);
		if (!ch->rtcp) return M4NetworkFailure;
		if (ch->net_info.IsUnicast) {
			if (!IsSource) {
				e = SK_Bind(ch->rtcp, ch->net_info.client_port_last, 1);
				if (e) return e;
				e = SK_Connect(ch->rtcp, ch->net_info.source, ch->net_info.port_last);
				if (e) return e;
			} else {
				e = SK_Bind(ch->rtp, ch->net_info.port_last, 1);
				if (e) return e;
				e = SK_SetRemoteAddress(ch->rtp, ch->net_info.destination);
				if (e) return e;
				e = SK_SetRemotePort(ch->rtp, ch->net_info.client_port_last);
				if (e) return e;
			}
		} else {
			//Bind to multicast (auto-join the group)
			e = SK_MulticastSetup(ch->rtcp, ch->net_info.source, ch->net_info.port_last, ch->net_info.TTL, (IsSource==2));
			if (e) return e;
			//destination is used for multicast interface addressing - TO DO
		}
	}
		
	//format CNAME if not done yet
	if (!ch->CName) {
		//this is the real CName setup
		if (!ch->rtp) {
			ch->CName = strdup("mpeg4rtp");
		} else {
			char name[MAX_IP_NAME_LEN];
			s32 start;
			M4_GetUserName(name, 1024);
			if (strlen(name)) strcat(name, "@");
			start = strlen(name);
			//get host IP or loopback if error
			if (SK_GetSocketIP(ch->rtp, name+start) != M4OK) strcpy(name+start, "127.0.0.1");
			ch->CName = strdup(name);
		}
	}
	
	return M4OK;
}

//get the UTC time expressed in RTP timescale
u32 RTP_ChannelTime(RTPChannel *ch)
{
	u32 sec, frac;
	Double t;

	M4_GetNTP(&sec, &frac);
	t = (frac >> 4);
	t /= 0x10000;
	t += sec;
	t *= ch->TimeScale;
	return (u32) t;
}

u32 RTP_GetReportTime()
{
	u32 sec, frac;
	Double t;

	M4_GetNTP(&sec, &frac);
	t = (frac>>4);
	t /= 0x10000;
	t += sec;
	//in units of 1/65536 seconds
	t *= 65536;
	return (u32) t;
}


void RTCP_GetNextReportTime(RTPChannel *ch)
{
	Double d;

	//offset between .5 and 1.5 sec ...
	d = 0.5 + ((Double) M4_Rand()) / ((Double) RAND_MAX);
	//of a minimal 5sec interval expressed in 1/65536 of a sec
	d = 5.0 * d * 65536;
	//we should estimate bandwidth sharing too, but as we only support one sender ...
	ch->next_report_time = RTP_GetReportTime() + (u32) d;
}


u32 RTP_ReadChannel(RTPChannel *ch, char *buffer, u32 buffer_size)
{
	M4Err e;
	u32 seq_num, res;
	char *pck;

	//only if the socket exist (otherwise RTSP interleaved channel)
	if (!ch || !ch->rtp) return 0;

	e = SK_Receive(ch->rtp, buffer, buffer_size, 0, &res);
	if (!res || e || (res < 12)) res = 0;

	//add the packet to our Queue if any
	if (ch->po) {
		if (res) {
			seq_num = ((buffer[2] << 8) & 0xFF00) | (buffer[3] & 0xFF);
			PO_AddPacket(ch->po, (void *) buffer, res, seq_num);
		}

		//pck queue may need to be flushed
		pck = PO_GetPacket(ch->po, &res);
		if (pck) {
			memcpy(buffer, pck, res);
			free(pck);
		}
	}
	return res;
}


M4Err RTP_DecodePacket(RTPChannel *ch, char *pck, u32 pck_size, RTPHeader *rtp_hdr, u32 *PayloadStart)
{
	M4Err e;
	s32 Deviance, delta;
	u32 CurrSeq, LastSeq;
	u32 lost, low16;

	if (!rtp_hdr) return M4BadParam;
	e = M4OK;

	//we need to uncompress the RTP header
	rtp_hdr->Version = (pck[0] & 0xC0 ) >> 6;
	if (rtp_hdr->Version != 2) return M4NotSupported;

	rtp_hdr->Padding = ( pck[0] & 0x20 ) >> 5;
	rtp_hdr->Extension = ( pck[0] & 0x10 ) >> 4;
	rtp_hdr->CSRCCount = pck[0] & 0x0F;
	rtp_hdr->Marker = ( pck[1] & 0x80 ) >> 7;
	rtp_hdr->PayloadType = pck[1] & 0x7F;

	//we don't support multiple CSRC now. Only one source (the server) is allowed
	if (rtp_hdr->CSRCCount) return M4NotSupported;

	//SeqNum
	rtp_hdr->SequenceNumber = ((pck[2] << 8) & 0xFF00) | (pck[3] & 0xFF);

	//TS
	rtp_hdr->TimeStamp = (u32) ((pck[4]<<24) &0xFF000000) | ((pck[5]<<16) & 0xFF0000) | ((pck[6]<<8) & 0xFF00) | ((pck[7]) & 0xFF);

	//SSRC
	rtp_hdr->SSRC = ((pck[8]<<24) &0xFF000000) | ((pck[9]<<16) & 0xFF0000) | ((pck[10]<<8) & 0xFF00) | ((pck[11]) & 0xFF);

	//first we only work with one payload type...
	if (rtp_hdr->PayloadType != ch->PayloadType) return M4NotSupported;

	//update RTP time if we didn't get the info
	if (!ch->rtp_time) {
		ch->rtp_time = rtp_hdr->TimeStamp;
		ch->rtp_first_SN = rtp_hdr->SequenceNumber;
		ch->num_sn_loops = 0;
	}

	if (ch->rtp_log && !ch->first_rtp_pck && ch->SenderSSRC && (ch->SenderSSRC != rtp_hdr->SSRC) ) {
		//fprintf(ch->rtp_log, "SSRC mismatch: %d vs %d\n", rtp_hdr->SSRC, ch->SenderSSRC);
		return M4NetworkEmpty;
	}

	//process RTP / update the RTCP 
	if (ch->first_rtp_pck ) {
		ch->first_rtp_pck = 0;
		ch->last_pck_sn = (u32) rtp_hdr->SequenceNumber;
		ch->last_deviance = RTP_ChannelTime(ch) - rtp_hdr->TimeStamp;
		ch->Jitter = 0;
	} else {
		//this is a loop in SN - add it
		if ( (ch->last_pck_sn + 1> rtp_hdr->SequenceNumber) 
			&& (rtp_hdr->SequenceNumber >= ch->last_pck_sn + MAX_RTP_SN/2)) {
			ch->num_sn_loops += 1;
		}

		//LOG packet drop
		if (ch->rtp_log && ch->last_pck_sn + 1 != rtp_hdr->SequenceNumber) 
			fprintf(ch->rtp_log, "RTP Pck Loss %d -> %d\n", ch->last_pck_sn, rtp_hdr->SequenceNumber);
		
		//RTP specs annexe A.8
		Deviance = RTP_ChannelTime(ch) - rtp_hdr->TimeStamp;
		delta = ch->last_deviance = Deviance;
		ch->last_deviance = Deviance;

		if (delta < 0) delta = -delta;
		ch->Jitter += delta - ( (ch->Jitter + 8) >> 4);

		LastSeq = ch->last_pck_sn;
		CurrSeq = (u32) rtp_hdr->SequenceNumber;
		//next sequential pck
		if ( ( (LastSeq + 1) & 0xffff ) == CurrSeq ) {	
			ch->last_num_pck_rcv += 1;
			ch->last_num_pck_expected += 1;
		}
		//repeated pck
		else if ( (LastSeq & 0xffff ) == CurrSeq ) {
			ch->last_num_pck_rcv += 1;
		}
		//drop pck
		else {
			//pck Loss
			low16 = LastSeq & 0xffff;
			if ( CurrSeq > low16 )
				lost = CurrSeq - low16;
			else
				lost = 0xffff - low16 + CurrSeq + 1;

			ch->last_num_pck_expected += lost;
			ch->last_num_pck_rcv += 1;
			ch->last_num_pck_loss += lost;
		}
		ch->last_pck_sn = CurrSeq;
	}
	//we work with no CSRC so payload offset is always 12
	*PayloadStart = 12;

	//store the time
	ch->CurrentTime = rtp_hdr->TimeStamp;
	return e;
}


Float RTP_GetCurrentTime(RTPChannel *ch)
{
	Double ret;
	if (!ch) return 0.0;
	ret = (Double) ch->CurrentTime;
	ret -= (Double) ch->rtp_time;
	ret /= ch->TimeScale;
	return (Float) ret;
}






M4Err RTP_SendPacket(RTPChannel *ch, RTPHeader *rtp_hdr, char *extra_header, u32 extra_header_size, char *pck, u32 pck_size)
{
	M4Err e;
	u32 Start, i;
	BitStream *bs;

	if (!ch || !rtp_hdr 
		|| !ch->send_buffer 
		|| !pck 
		|| (rtp_hdr->CSRCCount && !rtp_hdr->CSRC) 
		|| (rtp_hdr->CSRCCount > 15)) return M4BadParam;
	
	if (12 + extra_header_size + pck_size + 4*rtp_hdr->CSRCCount > ch->send_buffer_size) return M4PacketTooBig; 

	//we don't support multiple CSRC now. Only one source (the server) is allowed
	if (rtp_hdr->CSRCCount) return M4NotSupported;

	bs = NewBitStream(ch->send_buffer, ch->send_buffer_size, BS_WRITE);
	
	//write header
	BS_WriteInt(bs, rtp_hdr->Version, 2);
	BS_WriteInt(bs, rtp_hdr->Padding, 1);
	BS_WriteInt(bs, rtp_hdr->Extension, 1);
	BS_WriteInt(bs, rtp_hdr->CSRCCount, 4);
	BS_WriteInt(bs, rtp_hdr->Marker, 1);
	BS_WriteInt(bs, rtp_hdr->PayloadType, 7);
	BS_WriteInt(bs, rtp_hdr->SequenceNumber, 16);
	BS_WriteInt(bs, rtp_hdr->TimeStamp, 32);
	BS_WriteInt(bs, rtp_hdr->SSRC, 32);

	for (i=0; i<rtp_hdr->CSRCCount; i++) {
		BS_WriteInt(bs, rtp_hdr->CSRC[i], 32);
	}
	//nb: RTP header is always aligned
	Start = (u32) BS_GetPosition(bs);
	DeleteBitStream(bs);

	//extra header
	if (extra_header_size) {
		memcpy(ch->send_buffer + Start, extra_header, extra_header_size);
		Start += extra_header_size;
	}
	//payload
	memcpy(ch->send_buffer + Start, pck, pck_size);
	e = SK_SendTo(ch->rtp, ch->send_buffer, Start + pck_size, NULL, 0);
	if (e) return e;

	//Update RTCP for sender reports
	ch->pck_sent_since_last_sr += 1;
	if (ch->first_SR) {
		//get a new report time
		RTCP_GetNextReportTime(ch);
		ch->num_payload_bytes = 0;
		ch->num_pck_sent = 0;
		ch->first_SR = 0;
	}

	ch->num_payload_bytes += pck_size + extra_header_size;
	ch->num_pck_sent += 1;
	//store timing
	ch->last_pck_ts = rtp_hdr->TimeStamp;
	M4_GetNTP(&ch->last_pck_ntp_sec, &ch->last_pck_ntp_frac);
	return M4OK;
}

u32 RTP_IsUnicast(RTPChannel *ch)
{
	if (!ch) return 0;
	return ch->net_info.IsUnicast;
}

u32 RTP_IsInterleaved(RTPChannel *ch)
{
	if (!ch || !ch->net_info.Profile) return 0;
	return ch->net_info.IsInterleaved;
}

u32 RTP_GetTimeScale(RTPChannel *ch)
{
	if (!ch || !ch->TimeScale) return 0;
	return ch->TimeScale;
}

u32 RTP_IsActive(RTPChannel *ch)
{
	if (!ch) return 0;
	if (!ch->rtp_first_SN && !ch->rtp_time) return 0;
	return 1;
}

u8 RTP_GetLowInterleavedID(RTPChannel *ch)
{
	if (!ch || !ch->net_info.IsInterleaved) return 0;
	return ch->net_info.rtpID;
}

u8 RTP_GetHighInterleavedID(RTPChannel *ch)
{
	if (!ch || !ch->net_info.IsInterleaved) return 0;
	return ch->net_info.rtcpID;
}


#define RTP_DEFAULT_FIRSTPORT		7040

static u16 NextAvailablePort = RTP_DEFAULT_FIRSTPORT;

M4Err RTP_SetupPorts(RTPChannel *ch)
{
	u32 retry;
	u16 p = NextAvailablePort;
	M4Socket *sock;
	if (!ch) return M4BadParam;

	if (ch->net_info.client_port_first) return M4OK;

	sock = NewSocket(SK_TYPE_UDP);
	if (!sock) return M4IOErr;

	/*should be way enough (more than 100 rtp streams open on the machine)*/
	retry = 100;
	while (1) {
		/*try to bind without reuse. If fails this means the port is used on the machine, don't reuse it*/
		M4Err e = SK_Bind(sock, p, 0);
		if (e==M4OK) break;
		if (e!=M4ConnectionFailed) {
			SK_Delete(sock);
			return M4NetworkFailure;
		}
		p+=2;
	}
	SK_Delete(sock);
	ch->net_info.client_port_first = p;
	ch->net_info.client_port_last = p + 1;
	NextAvailablePort = p + 2;
	return M4OK;
}


M4Err RTP_SetupPayload(RTPChannel *ch, SDP_RTPMap *map)
{
	if (!ch || !map) return M4BadParam;
	ch->PayloadType = map->PayloadType;
	strcpy(ch->PayloadName, map->payload_name ? map->payload_name : "");
	ch->TimeScale = map->ClockRate;
	return M4OK;
}

RTSPTransport *RTP_GetTransport(RTPChannel *ch)
{
	if (!ch) return NULL;
	return &ch->net_info;
}

u32 RTP_GetLocalSSRC(RTPChannel *ch)
{
	if (!ch) return 0;
	return ch->SSRC;
}


void RTP_SetLog(RTPChannel *ch, FILE *log)
{
	if (ch) ch->rtp_log = log;
}

Float RTP_GetLossPercentage(RTPChannel *ch)
{
	if (!ch->tot_num_pck_expected) return 0.0f;
	return 100.0f - (100.0f * ch->tot_num_pck_rcv) / ch->tot_num_pck_expected;
}

u32 RTP_GetRTCPBytesSent(RTPChannel *ch)
{
	return ch->rtcp_bytes_sent;
}

void RTP_GetPorts(RTPChannel *ch, u16 *rtp_port, u16 *rtcp_port)
{
	*rtp_port = ch->net_info.client_port_first;
	*rtcp_port = ch->net_info.client_port_last;
}

