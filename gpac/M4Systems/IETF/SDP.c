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



#include <gpac/m4_ietf.h>

#define SDP_WRITE_STEPALLOC		2048


SDP_FMTP *SDP_NewFMTP()
{
	SDP_FMTP *tmp = malloc(sizeof(SDP_FMTP));
	tmp->PayloadType = 0;
	tmp->Attributes = NewChain();
	return tmp;
}

void SDP_DeleteFMTP(SDP_FMTP *fmtp)
{
	X_Attribute *att;
	if (!fmtp) return;
	while (ChainGetCount(fmtp->Attributes)) {
		att = ChainGetEntry(fmtp->Attributes, 0);
		ChainDeleteEntry(fmtp->Attributes, 0);
		if (att->Name) free(att->Name);
		if (att->Value) free(att->Value);
		free(att);
	}
	DeleteChain(fmtp->Attributes);
	free(fmtp);
}

SDP_FMTP *SDP_GetFMTPForPayload(SDPMedia *media, u32 PayloadType)
{
	SDP_FMTP *tmp;
	u32 i;
	if (!media) return NULL;
	for (i=0; i<ChainGetCount(media->FMTP); i++) {
		tmp = ChainGetEntry(media->FMTP, i);
		if (tmp->PayloadType == PayloadType) return tmp;
	}
	return NULL;
}

void SDP_ParseAttribute(SDPInfo *sdp, char *buffer, SDPMedia *media)
{
	s32 pos;
	u32 PayT;
	char comp[3000];
	SDP_RTPMap *map;
	SDP_FMTP *fmtp;
	X_Attribute *att;

	pos = SP_GetComponent(buffer, 0, " :\t\r\n", comp, 3000);

	if (!strcmp(comp, "cat")) {
		if (media) return;
		pos = SP_GetComponent(buffer, pos, ":\t\r\n", comp, 3000);
		sdp->a_cat = strdup(comp);
		return;
	}
	if (!strcmp(comp, "keywds")) {
		if (media) return;
		pos = SP_GetComponent(buffer, pos, ":\t\r\n", comp, 3000);
		sdp->a_keywds = strdup(comp);
		return;
	}
	if (!strcmp(comp, "tool")) {
		if (media) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		sdp->a_tool = strdup(comp);
		return;
	}

	if (!strcmp(comp, "ptime")) {
		if (!media) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		media->PacketTime = atoi(comp);
		return;
	}
	if (!strcmp(comp, "recvonly")) {
		if (!media) {
			sdp->a_SendRecieve = 1;
		} else {
			media->SendRecieve = 1;
		}
		return;
	}
	if (!strcmp(comp, "sendonly")) {
		if (!media) {
			sdp->a_SendRecieve = 2;
		} else {
			media->SendRecieve = 2;
		}
		return;
	}
	if (!strcmp(comp, "sendrecv")) {
		if (!media) {
			sdp->a_SendRecieve = 3;
		} else {
			media->SendRecieve = 3;
		}
		return;
	}
	if (!strcmp(comp, "orient")) {
		if (!media || media->Type) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		media->orientation = strdup(comp);
		return;
	}
	if (!strcmp(comp, "type")) {
		if (media) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		sdp->a_type = strdup(comp);
		return;
	}
	if (!strcmp(comp, "charset")) {
		if (media) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		sdp->a_charset = strdup(comp);
		return;
	}
	if (!strcmp(comp, "sdplang")) {
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		if (media) {
			media->sdplang = strdup(comp);
		} else {
			sdp->a_sdplang = strdup(comp);
		}
		return;
	}
	if (!strcmp(comp, "lang")) {
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		if (media) {
			media->lang = strdup(comp);
		} else {
			sdp->a_lang = strdup(comp);
		}
		return;
	}
	if (!strcmp(comp, "framerate")) {
		//only for video
		if (!media || (media->Type != 1)) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		media->FrameRate = (Float) atof(comp);
		return;
	}
	if (!strcmp(comp, "quality")) {
		if (!media) return;
		pos = SP_GetComponent(buffer, pos, ":\r\n", comp, 3000);
		media->Quality = atoi(comp);
		return;
	}
	if (!strcmp(comp, "rtpmap")) {
		if (!media) return;
		map = malloc(sizeof(SDP_RTPMap));
		pos = SP_GetComponent(buffer, pos, ": \r\n", comp, 3000);
		map->PayloadType = atoi(comp);
		pos = SP_GetComponent(buffer, pos, " /\r\n", comp, 3000);
		map->payload_name = strdup(comp);
		pos = SP_GetComponent(buffer, pos, " /\r\n", comp, 3000);
		map->ClockRate = atoi(comp);
		pos = SP_GetComponent(buffer, pos, " /\r\n", comp, 3000);
		map->AudioChannels = (pos > 0) ? atoi(comp) : 0;
		ChainAddEntry(media->RTPMaps, map);
		return;
	}
	//FMTP
	if (!strcmp(comp, "fmtp")) {
		if (!media) return;
		pos = SP_GetComponent(buffer, pos, ": \r\n", comp, 3000);
		PayT = atoi(comp);
		fmtp = SDP_GetFMTPForPayload(media, PayT);
		if (!fmtp) {
			fmtp = SDP_NewFMTP();
			fmtp->PayloadType = PayT;
			ChainAddEntry(media->FMTP, fmtp);
		}
		while (1) {
			pos = SP_GetComponent(buffer, pos, "; =\r\n", comp, 3000);
			if (pos <= 0) break;
			att = malloc(sizeof(X_Attribute));
			att->Name = strdup(comp);
			att->Value = NULL;
			pos ++;
			pos = SP_GetComponent(buffer, pos, ";\r\n", comp, 3000);
			if (pos > 0) att->Value = strdup(comp);
			ChainAddEntry(fmtp->Attributes, att);
		}
		return;
	}
	//the rest cannot be discarded that way as it may be application-specific
	//so keep it.
	//a= <attribute> || <attribute>:<value>
	//we add <attribute> <value> in case ...
	pos = SP_GetComponent(buffer, 0, " :\r\n", comp, 3000);
	att = malloc(sizeof(X_Attribute));
	att->Name = strdup(comp);
	att->Value = NULL;
	pos += 1;
	if (buffer[pos] == ' ') pos += 1;
	pos = SP_GetComponent(buffer, pos, "\r\n", comp, 3000);
	if (pos > 0) att->Value = strdup(comp);

	if (media) {
		ChainAddEntry(media->Attributes, att);
	} else {
		ChainAddEntry(sdp->Attributes, att);
	}
}



#define SDPM_DESTROY(p) if (media->p) free(media->p)
void SDPMedia_Delete(SDPMedia *media)
{
	SDPBandwidth *bw;
	SDP_RTPMap *map;
	SDPConnection *conn;
	SDP_FMTP *fmtp;
	X_Attribute *att;
	if (!media) return;

	while (ChainGetCount(media->FMTP)) {
		fmtp = ChainGetEntry(media->FMTP, 0);
		ChainDeleteEntry(media->FMTP, 0);
		SDP_DeleteFMTP(fmtp);
	}
	DeleteChain(media->FMTP);

	while (ChainGetCount(media->Attributes)) {
		att = ChainGetEntry(media->Attributes, 0);
		ChainDeleteEntry(media->Attributes, 0);
		if (att->Name) free(att->Name);
		if (att->Value) free(att->Value);
		free(att);
	}
	DeleteChain(media->Attributes);

	while (ChainGetCount(media->RTPMaps)) {
		map = ChainGetEntry(media->RTPMaps, 0);
		free(map->payload_name);
		free(map);
		ChainDeleteEntry(media->RTPMaps, 0);
	}
	DeleteChain(media->RTPMaps);
	
	while (ChainGetCount(media->Connections)) {
		conn = ChainGetEntry(media->Connections, 0);
		ChainDeleteEntry(media->Connections, 0);
		SDP_DeleteConnection(conn);
	}
	DeleteChain(media->Connections);

	while (ChainGetCount(media->Bandwidths)) {
		bw = ChainGetEntry(media->Bandwidths, 0);
		ChainDeleteEntry(media->Bandwidths, 0);
		if (bw->name) free(bw->name);
		free(bw);
	}
	DeleteChain(media->Bandwidths);

	SDPM_DESTROY(orientation);
	SDPM_DESTROY(sdplang);
	SDPM_DESTROY(lang);
	SDPM_DESTROY(Profile);
	SDPM_DESTROY(fmt_list);
	SDPM_DESTROY(k_method);
	SDPM_DESTROY(k_key);
	free(media);
}


SDPConnection *SDP_NewConnection()
{
	SDPConnection *conn = malloc(sizeof(SDPConnection));
	memset(conn, 0, sizeof(SDPConnection));
	conn->TTL = -1;
	return conn;
}

void SDP_DeleteConnection(SDPConnection *conn)
{
	if (conn->add_type) free(conn->add_type);
	if (conn->host) free(conn->host);
	if (conn->net_type) free(conn->net_type);
	free(conn);
}

SDPMedia *SDP_NewMedia()
{
	SDPMedia *tmp = malloc(sizeof(SDPMedia));
	memset(tmp, 0, sizeof(SDPMedia));
	tmp->FMTP = NewChain();
	tmp->RTPMaps = NewChain();
	tmp->Attributes = NewChain();
	tmp->Connections = NewChain();
	tmp->Bandwidths = NewChain();
	tmp->Quality = -1;
	return tmp;
}

SDPInfo *NewSDPInfo()
{
	SDPInfo *sdp = malloc(sizeof(SDPInfo));
	memset(sdp, 0, sizeof(SDPInfo));
	sdp->b_bandwidth = NewChain();
	sdp->media_desc = NewChain();
	sdp->Attributes = NewChain();
	sdp->Timing = NewChain();
	return sdp;
}

#define SDP_DESTROY(p) if (sdp->p)	\
					free(sdp->p);	\
					sdp->p = NULL;


void SDP_Reset(SDPInfo *sdp)
{
	SDPBandwidth *bw;
	SDPMedia *media;
	SDPTiming *timing;
	X_Attribute *att;

	if (!sdp) return;
	
	while (ChainGetCount(sdp->media_desc)) {
		media = ChainGetEntry(sdp->media_desc, 0);
		ChainDeleteEntry(sdp->media_desc, 0);
		SDPMedia_Delete(media);
	}
	while (ChainGetCount(sdp->Attributes)) {
		att = ChainGetEntry(sdp->Attributes, 0);
		ChainDeleteEntry(sdp->Attributes, 0);
		if (att->Name) free(att->Name);
		if (att->Value) free(att->Value);
		free(att);
	}
	while (ChainGetCount(sdp->b_bandwidth)) {
		bw = ChainGetEntry(sdp->b_bandwidth, 0);
		ChainDeleteEntry(sdp->b_bandwidth, 0);
		if (bw->name) free(bw->name);
		free(bw);
	}
	while (ChainGetCount(sdp->Timing)) {
		timing = ChainGetEntry(sdp->Timing, 0);
		ChainDeleteEntry(sdp->Timing, 0);
		free(timing);
	}

	//then delete all info ...
	SDP_DESTROY(o_username);
	SDP_DESTROY(o_session_id);
	SDP_DESTROY(o_version);
	SDP_DESTROY(o_address);
	SDP_DESTROY(o_net_type);
	SDP_DESTROY(o_add_type);
	SDP_DESTROY(s_session_name);
	SDP_DESTROY(i_description);
	SDP_DESTROY(u_uri);
	SDP_DESTROY(e_email);
	SDP_DESTROY(p_phone);
	SDP_DESTROY(k_method);
	SDP_DESTROY(k_key);
	SDP_DESTROY(a_cat);
	SDP_DESTROY(a_keywds);
	SDP_DESTROY(a_tool);
	SDP_DESTROY(a_type);
	SDP_DESTROY(a_charset);
	SDP_DESTROY(a_sdplang);
	SDP_DESTROY(a_lang);

	if (sdp->c_connection) {
		SDP_DeleteConnection(sdp->c_connection);
		sdp->c_connection = NULL;
	}
	sdp->a_SendRecieve = 0;
}

void SDP_Delete(SDPInfo *sdp)
{	
	if (!sdp) return;
	SDP_Reset(sdp);
	DeleteChain(sdp->media_desc);
	DeleteChain(sdp->Attributes);
	DeleteChain(sdp->b_bandwidth);
	DeleteChain(sdp->Timing);
	free(sdp);
}


Bool SDP_IsDynamicPayload(SDPMedia *media, char *payt)
{
	u32 i;
	SDP_RTPMap *map;
	char buf[10];
	for (i=0;i<ChainGetCount(media->RTPMaps); i++) {
		map = ChainGetEntry(media->RTPMaps, i);
		sprintf(buf, "%d", map->PayloadType);
		if (!strcmp(payt, buf)) return 1;
	}
	return 0;
}

//translate h || m || d in sec. Fractions are not allowed with this writing 
s32 SDP_MakeSeconds(char *buf)
{
	s32 sign;
	char num[30], *test;
	sign = 1;
	if (buf[0] == '-') {
		sign = -1;
		buf += 1;
	}
	memset(num, 0, 30);
	test = strstr(buf, "d");
	if (test) {
		strncpy(num, buf, strlen(buf)-strlen(test));
		return (atoi(num)*sign*86400);
	}
	test = strstr(buf, "h");
	if (test) {
		strncpy(num, buf, strlen(buf)-strlen(test));
		return (atoi(num)*sign*3600);
	}
	test = strstr(buf, "m");
	if (test) {
		strncpy(num, buf, strlen(buf)-strlen(test));
		return (atoi(num)*sign*60);
	}
	return (atoi(buf) * sign);
}


M4Err SDP_Parse(SDPInfo *sdp, char *sdp_text, u32 text_size)
{
	SDPBandwidth *bw;
	SDPConnection *conn;
	SDPMedia *media;
	SDPTiming *timing;
	u32 i;
	s32 pos, LinePos;
	char LineBuf[3000], comp[3000];

	media = NULL;
	timing = NULL;

	if (!sdp) return M4BadParam;

	//Clean SDP info
	SDP_Reset(sdp);

	LinePos = 0;
	while (1) {
		LinePos = SP_GetOneLine(sdp_text, LinePos, text_size, LineBuf, 3000);
		if (LinePos <= 0) break;
		if (!strcmp(LineBuf, "\r\n") || !strcmp(LineBuf, "\n") || !strcmp(LineBuf, "\r")) continue;


		switch (LineBuf[0]) {
		case 'v':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->Version = atoi(comp);
			break;
		case 'o':
			pos = SP_GetComponent(LineBuf, 2, " \t\r\n", comp, 3000);
			sdp->o_username = strdup(comp);
			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			sdp->o_session_id = strdup(comp);
			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			sdp->o_version = strdup(comp);
			
			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			sdp->o_net_type = strdup(comp);

			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			sdp->o_add_type = strdup(comp);

			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			sdp->o_address = strdup(comp);
			break;
		case 's':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->s_session_name = strdup(comp);
			break;
		case 'i':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->i_description = strdup(comp);
			break;
		case 'u':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->u_uri = strdup(comp);
			break;
		case 'e':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->e_email = strdup(comp);
			break;
		case 'p':
			pos = SP_GetComponent(LineBuf, 2, "\t\r\n", comp, 3000);
			sdp->p_phone = strdup(comp);
			break;
		case 'c':
			//if at session level, only 1 is allowed for all SDP
			if (sdp->c_connection) break;
			
			conn = SDP_NewConnection();

			pos = SP_GetComponent(LineBuf, 2, " \t\r\n", comp, 3000);
			conn->net_type = strdup(comp);
			
			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			conn->add_type = strdup(comp);

			pos = SP_GetComponent(LineBuf, pos, " /\r\n", comp, 3000);
			conn->host = strdup(comp);
			if (SK_IsMulticastAddress(conn->host)) {
				//a valid SDP will have TTL if address is multicast
				pos = SP_GetComponent(LineBuf, pos, "/\r\n", comp, 3000);
				if (pos <= 0) {
					SDP_DeleteConnection(conn);
					break;
				}
				conn->TTL = atoi(comp);
				//multiple address indication is only valid for media
				pos = SP_GetComponent(LineBuf, pos, "/\r\n", comp, 3000);
				if (pos > 0) {
					if (!media) {
						SDP_DeleteConnection(conn);
						break;
					}
					conn->add_count = atoi(comp);
				}
			}
			if (!media) 
				sdp->c_connection = conn;
			else 
				ChainAddEntry(media->Connections, conn);

			break;
		case 'b':
			pos = SP_GetComponent(LineBuf, 2, ":\r\n", comp, 3000);
			if (strcmp(comp, "CT") && strcmp(comp, "AS") && (comp[0] != 'X')) break;

			bw = malloc(sizeof(SDPBandwidth));
			bw->name = strdup(comp);
			pos = SP_GetComponent(LineBuf, pos, ":\r\n", comp, 3000);
			bw->value = atoi(comp);
			if (media) {
				ChainAddEntry(media->Bandwidths, bw);
			} else {
				ChainAddEntry(sdp->b_bandwidth, bw);
			}
			break;

		case 't':
			if (media) break;
			//create a new time structure for each entry
			timing = malloc(sizeof(SDPTiming));
			memset(timing, 0, sizeof(SDPTiming));
			pos = SP_GetComponent(LineBuf, 2, " \t\r\n", comp, 3000);
			timing->StartTime = atoi(comp);
			pos = SP_GetComponent(LineBuf, pos, "\r\n", comp, 3000);
			timing->StopTime = atoi(comp);
			ChainAddEntry(sdp->Timing, timing);
			break;
		case 'r':
			if (media) break;
			pos = SP_GetComponent(LineBuf, 2, " \t\r\n", comp, 3000);
			timing->RepeatInterval = SDP_MakeSeconds(comp);
			pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
			timing->ActiveDuration = SDP_MakeSeconds(comp);
			while (1) {
				pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
				if (pos <= 0) break;
				timing->OffsetFromStart[timing->NbRepeatOffsets] = SDP_MakeSeconds(comp);
				timing->NbRepeatOffsets += 1;
			}
			break;
		case 'z':
			if (media) break;
			pos = 2;
			while (1) {
				pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
				if (pos <= 0) break;
				timing->AdjustmentTime[timing->NbZoneOffsets] = atoi(comp);
				pos = SP_GetComponent(LineBuf, pos, " \t\r\n", comp, 3000);
				timing->AdjustmentOffset[timing->NbZoneOffsets] = SDP_MakeSeconds(comp);
				timing->NbZoneOffsets += 1;
			}
			break;
		case 'k':
			pos = SP_GetComponent(LineBuf, 2, ":\t\r\n", comp, 3000);
			if (media) {
				media->k_method = strdup(comp);
			} else {
				sdp->k_method = strdup(comp);
			}
			pos = SP_GetComponent(LineBuf, pos, ":\r\n", comp, 3000);
			if (pos > 0) {
				if (media) {
					media->k_key = strdup(comp);
				} else {
					sdp->k_key = strdup(comp);
				}
			}
			break;
		case 'a':
			SDP_ParseAttribute(sdp, LineBuf+2, media);
			break;
		case 'm':
			pos = SP_GetComponent(LineBuf, 2, " \t\r\n", comp, 3000);
			if (strcmp(comp, "audio") 
				&& strcmp(comp, "data") 
				&& strcmp(comp, "control") 
				&& strcmp(comp, "video") 
				&& strcmp(comp, "text") 
				&& strcmp(comp, "application")) {
				return M4ServiceError;
			}
			media = SDP_NewMedia();
			//media type
			if (!strcmp(comp, "video")) media->Type = 1;
			else if (!strcmp(comp, "audio")) media->Type = 2;
			else if (!strcmp(comp, "text")) media->Type = 3;
			else if (!strcmp(comp, "data")) media->Type = 4;
			else if (!strcmp(comp, "control")) media->Type = 5;
			else media->Type = 0;
			//port numbers			
			SP_GetComponent(LineBuf, pos, " ", comp, 3000);
			if (!strstr(comp, "/")) {
				pos = SP_GetComponent(LineBuf, pos, " \r\n", comp, 3000);
				media->PortNumber = atoi(comp);
				media->NumPorts = 0;
			} else {
				pos = SP_GetComponent(LineBuf, pos, " /\r\n", comp, 3000);
				media->PortNumber = atoi(comp);
				pos = SP_GetComponent(LineBuf, pos, " \r\n", comp, 3000);
				media->NumPorts = atoi(comp);
			}
			//transport Profile
			pos = SP_GetComponent(LineBuf, pos, " \r\n", comp, 3000);
			media->Profile = strdup(comp);
			pos = SP_GetComponent(LineBuf, pos, " \r\n", comp, 3000);
			media->fmt_list = strdup(comp);

			ChainAddEntry(sdp->media_desc, media);
			break;
		}
	}
	//finally rewrite the fmt_list for all media, and remove dynamic payloads 
	//from the list
	for (i=0; i<ChainGetCount(sdp->media_desc); i++) {
		media = ChainGetEntry(sdp->media_desc, i);
		pos = 0;
		LinePos = 1;
		strcpy(LineBuf, "");
		while (1) {
			if (!media->fmt_list) break;
			pos = SP_GetComponent(media->fmt_list, pos, " ", comp, 3000);
			if (pos <= 0) break;
			if (!SDP_IsDynamicPayload(media, comp)) {
				if (!LinePos) {
					strcat(LineBuf, " ");
				} else {
					LinePos = 0;
				}
				strcat(LineBuf, comp);
			}
			free(media->fmt_list);
			media->fmt_list = NULL;
			if (strlen(LineBuf)) {
				media->fmt_list = strdup(LineBuf);
			}
		}
	}
	return M4OK;
}



M4Err SDP_CheckConnection(SDPConnection *conn)
{
	if (!conn) return M4BadParam;
	if (!conn->host || !conn->add_type || !conn->net_type) return M4SignalingFailure;
	if (SK_IsMulticastAddress(conn->host)) {
		if (conn->TTL < 0 || conn->TTL > 255) return M4SignalingFailure;
	} else {
		conn->TTL = -1;
		conn->add_count = 0;
	}
	return M4OK;
}

//return M4BadParam if invalid structure, M4SignalingFailure if bad formatting
//or M4OK
M4Err SDP_CheckInfo(SDPInfo *sdp)
{
	M4Err e;
	u32 i, j;
	SDPMedia *media;
	SDPConnection *conn;
	SDP_RTPMap *map;
	Bool HasGlobalConnection, HasSeveralPorts;

	if (!sdp || !sdp->media_desc || !sdp->Attributes) return M4BadParam;
	//we force at least one media per SDP
	if (!ChainGetCount(sdp->media_desc)) return M4SignalingFailure;

	//normative fields
	//o=
	if (!sdp->o_add_type || !sdp->o_address || !sdp->o_username || !sdp->o_session_id || !sdp->o_version) 
		return M4SignalingFailure;
	//s=
	if (!sdp->s_session_name) return M4SignalingFailure;
	//t=
//	if () return M4SignalingFailure;
	//c=
	if (sdp->c_connection) { 
		e = SDP_CheckConnection(sdp->c_connection);
		if (e) return e;
		//multiple addresses are only for media desc
		if (sdp->c_connection->add_count >= 2) return M4SignalingFailure;
		HasGlobalConnection = 1;
	} else {
		HasGlobalConnection = 0;
	}

	//then check all media
	for (i=0; i<ChainGetCount(sdp->media_desc); i++) {
		media = ChainGetEntry(sdp->media_desc, i);
		HasSeveralPorts = 0;

		//m= : force non-null port, profile and fmt_list
		if (!media->PortNumber || !media->Profile) return M4SignalingFailure;
		if (media->NumPorts) HasSeveralPorts = 1;

		//no connections specified - THIS IS AN ERROR IN SDP BUT NOT IN ALL RTSP SESSIONS...
//		if (!HasGlobalConnection && !ChainGetCount(media->Connections)) return M4SignalingFailure;
		//too many connections specified
		if (HasGlobalConnection && ChainGetCount(media->Connections)) return M4SignalingFailure;

		//check all connections, and make sure we don't have multiple addresses 
		//and multiple ports at the same time
		if (ChainGetCount(media->Connections)>1 && HasSeveralPorts) return M4SignalingFailure;
		for (j=0; j<ChainGetCount(media->Connections); j++) {
			conn = ChainGetEntry(media->Connections, j);
			e = SDP_CheckConnection(conn);
			if (e) return e;
			if ((conn->add_count >= 2) && HasSeveralPorts) return M4SignalingFailure; 
		}
		//RTPMaps. 0 is tolerated, but if some are specified check them
		for (j=0; j<ChainGetCount(media->RTPMaps); j++) {
			map = ChainGetEntry(media->RTPMaps, j);
			//RFC2327 is not clear here, but we assume the PayloadType should be a DYN one
			//however this depends on the profile (RTP/AVP or others) so don't check it
			//ClockRate SHALL NOT be NULL
			if (!map->payload_name || !map->ClockRate) return M4SignalingFailure;
		}
	}
	//Encryption: nothing tells wether the scope of the global key is eclusive or not.
	//we accept a global key + keys per media entry, assuming that the media key primes
	//on the global key


	return M4OK;
}




#define SDP_WRITE_ALLOC_STR(str, space)		\
		if (str) {		\
			if (strlen(str)+pos + (space ? 1 : 0) >= buf_size) {	\
				buf_size += SDP_WRITE_STEPALLOC;	\
				buf = realloc(buf, buf_size);		\
			}	\
			strcpy(buf+pos, str);		\
			pos += strlen(str);		\
			if (space) {			\
				strcat(buf+pos, " ");	\
				pos += 1;		\
			}		\
		}

#define SDP_WRITE_ALLOC_INT(d, spa, sig)		\
	if (sig) { \
		sprintf(temp, "%d", d);		\
	} else { \
		sprintf(temp, "%u", d);		\
	}	\
	SDP_WRITE_ALLOC_STR(temp, spa);

#define SDP_WRITE_ALLOC_FLOAT(d, spa)		\
	sprintf(temp, "%.2f", d);		\
	SDP_WRITE_ALLOC_STR(temp, spa);

#define TEST_SDP_WRITE_SINGLE(type, str, sep)		\
	if (str) {		\
		SDP_WRITE_ALLOC_STR(type, 0);		\
		if (sep) SDP_WRITE_ALLOC_STR(":", 0);		\
		SDP_WRITE_ALLOC_STR(str, 0);		\
		SDP_WRITE_ALLOC_STR("\r\n", 0);		\
	}


#define SDP_WRITE_CONN(conn)		\
	if (conn) {			\
		SDP_WRITE_ALLOC_STR("c=", 0);	\
		SDP_WRITE_ALLOC_STR(conn->net_type, 1);		\
		SDP_WRITE_ALLOC_STR(conn->add_type, 1);		\
		SDP_WRITE_ALLOC_STR(conn->host, 0);			\
		if (SK_IsMulticastAddress(conn->host)) {	\
			SDP_WRITE_ALLOC_STR("/", 0);			\
			SDP_WRITE_ALLOC_INT(conn->TTL, 0, 0);		\
			if (conn->add_count >= 2) {		\
				SDP_WRITE_ALLOC_STR("/", 0);		\
				SDP_WRITE_ALLOC_INT(conn->add_count, 0, 0);	\
			}		\
		}	\
		SDP_WRITE_ALLOC_STR("\r\n", 0);		\
	}

M4Err SDP_Write(SDPInfo *sdp, char **out_str_buf)
{
	char *buf;
	SDP_FMTP *fmtp;
	char temp[50];
	SDPMedia *media;
	SDPBandwidth *bw;
	u32 buf_size, pos, i, j, k;
	SDP_RTPMap *map;
	SDPConnection *conn;
	M4Err e;
	SDPTiming *timing;
	X_Attribute *att;

	e = SDP_CheckInfo(sdp);
	if (e) return e;

	buf = malloc(SDP_WRITE_STEPALLOC);
	buf_size = SDP_WRITE_STEPALLOC;
	pos = 0;

	//v
	SDP_WRITE_ALLOC_STR("v=", 0);
	SDP_WRITE_ALLOC_INT(sdp->Version, 0, 0);
	SDP_WRITE_ALLOC_STR("\r\n", 0);
	//o
	SDP_WRITE_ALLOC_STR("o=", 0);
	SDP_WRITE_ALLOC_STR(sdp->o_username, 1);
	SDP_WRITE_ALLOC_STR(sdp->o_session_id, 1);
	SDP_WRITE_ALLOC_STR(sdp->o_version, 1);
	SDP_WRITE_ALLOC_STR(sdp->o_net_type, 1);
	SDP_WRITE_ALLOC_STR(sdp->o_add_type, 1);
	SDP_WRITE_ALLOC_STR(sdp->o_address, 0);
	SDP_WRITE_ALLOC_STR("\r\n", 0);
	//s
	TEST_SDP_WRITE_SINGLE("s=", sdp->s_session_name, 0);
	//i
	TEST_SDP_WRITE_SINGLE("i=", sdp->i_description, 0);
	//u
	TEST_SDP_WRITE_SINGLE("u=", sdp->u_uri, 0);
	//e
	TEST_SDP_WRITE_SINGLE("e=", sdp->e_email, 0);
	//p
	TEST_SDP_WRITE_SINGLE("p=", sdp->p_phone, 0);
	//c
	SDP_WRITE_CONN(sdp->c_connection);
	//b
	for (i=0; i<ChainGetCount(sdp->b_bandwidth); i++) {
		bw = ChainGetEntry(sdp->b_bandwidth, i);
		SDP_WRITE_ALLOC_STR("b=", 0);
		SDP_WRITE_ALLOC_STR(bw->name, 0);
		SDP_WRITE_ALLOC_STR(":", 0);
		SDP_WRITE_ALLOC_INT(bw->value, 0, 0);
		SDP_WRITE_ALLOC_STR("\r\n", 0);
	}
	//t+r+z
	for (i=0; i<ChainGetCount(sdp->Timing); i++) {
		timing = ChainGetEntry(sdp->Timing, i);
		if (timing->NbRepeatOffsets > SDP_MAX_TIMEOFFSET) timing->NbRepeatOffsets = SDP_MAX_TIMEOFFSET;
		if (timing->NbZoneOffsets > SDP_MAX_TIMEOFFSET) timing->NbZoneOffsets = SDP_MAX_TIMEOFFSET;
		//t
		SDP_WRITE_ALLOC_STR("t=", 0);
		SDP_WRITE_ALLOC_INT(timing->StartTime, 1, 0);
		SDP_WRITE_ALLOC_INT(timing->StopTime, 0, 0);
		SDP_WRITE_ALLOC_STR("\r\n", 0);
		if (timing->NbRepeatOffsets) {
			SDP_WRITE_ALLOC_STR("r=", 0);
			SDP_WRITE_ALLOC_INT(timing->RepeatInterval, 1, 0);
			SDP_WRITE_ALLOC_INT(timing->ActiveDuration, 0, 0);
			for (j=0; j<timing->NbRepeatOffsets; j++) {
				SDP_WRITE_ALLOC_STR(" ", 0);
				SDP_WRITE_ALLOC_INT(timing->OffsetFromStart[j], 0, 0);
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		if (timing->NbZoneOffsets) {
			SDP_WRITE_ALLOC_STR("z=", 0);
			for (j=0; j<timing->NbZoneOffsets; j++) {
				SDP_WRITE_ALLOC_INT(timing->AdjustmentTime[j], 1, 0);
				if (j+1 == timing->NbRepeatOffsets) {
					SDP_WRITE_ALLOC_INT(timing->AdjustmentOffset[j], 0, 1);
				} else {
					SDP_WRITE_ALLOC_INT(timing->AdjustmentOffset[j], 1, 1);
				}
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
	}
	//k
	if (sdp->k_method) {
		SDP_WRITE_ALLOC_STR("k=", 0);
		SDP_WRITE_ALLOC_STR(sdp->k_method, 0);
		if (sdp->k_key) {
			SDP_WRITE_ALLOC_STR(":", 0);
			SDP_WRITE_ALLOC_STR(sdp->k_key, 0);
		}
		SDP_WRITE_ALLOC_STR("\r\n", 0);
	}
	//a=cat
	TEST_SDP_WRITE_SINGLE("a=cat", sdp->a_cat, 1);
	//a=keywds
	TEST_SDP_WRITE_SINGLE("a=keywds", sdp->a_keywds, 1);
	//a=tool
	TEST_SDP_WRITE_SINGLE("a=tool", sdp->a_tool, 1);
	//a=SendRecv
	switch (sdp->a_SendRecieve) {
	case 1:
		TEST_SDP_WRITE_SINGLE("a=", "recvonly", 0);
		break;
	case 2:
		TEST_SDP_WRITE_SINGLE("a=", "sendonly", 0);
		break;
	case 3:
		TEST_SDP_WRITE_SINGLE("a=", "sendrecv", 0);
		break;
	default:
		break;
	}
	//a=type
	TEST_SDP_WRITE_SINGLE("a=type", sdp->a_type, 1);
	//a=charset
	TEST_SDP_WRITE_SINGLE("a=charset", sdp->a_charset, 1);
	//a=sdplang
	TEST_SDP_WRITE_SINGLE("a=sdplang", sdp->a_sdplang, 1);
	//a=lang
	TEST_SDP_WRITE_SINGLE("a=lang", sdp->a_lang, 1);

	//the rest
	for (i=0; i<ChainGetCount(sdp->Attributes); i++) {
		att = ChainGetEntry(sdp->Attributes, i);
		SDP_WRITE_ALLOC_STR("a=", 0);
		SDP_WRITE_ALLOC_STR(att->Name, 0);
		if (att->Value) {
			SDP_WRITE_ALLOC_STR(":", 0);
			SDP_WRITE_ALLOC_STR(att->Value, 0);
		}
		SDP_WRITE_ALLOC_STR("\r\n", 0);
	}

	//now write media specific
	for (i=0; i<ChainGetCount(sdp->media_desc); i++) {
		media = ChainGetEntry(sdp->media_desc, i);

		//m=
		SDP_WRITE_ALLOC_STR("m=", 0);
		switch (media->Type) {
		case 1:
			SDP_WRITE_ALLOC_STR("video", 1);
			break;
		case 2:
			SDP_WRITE_ALLOC_STR("audio", 1);
			break;
		case 3:
			SDP_WRITE_ALLOC_STR("data", 1);
			break;
		case 4:
			SDP_WRITE_ALLOC_STR("control", 1);
			break;
		default:
			SDP_WRITE_ALLOC_STR("application", 1);
			break;
		}
		SDP_WRITE_ALLOC_INT(media->PortNumber, 0, 0);
		if (media->NumPorts >= 2) {
			SDP_WRITE_ALLOC_STR("/", 0);
			SDP_WRITE_ALLOC_INT(media->NumPorts, 1, 0);
		} else {
			SDP_WRITE_ALLOC_STR(" ", 0);
		}
		SDP_WRITE_ALLOC_STR(media->Profile, 1);
		SDP_WRITE_ALLOC_STR(media->fmt_list, 0);

		for (j=0;j<ChainGetCount(media->RTPMaps); j++) {
			map = ChainGetEntry(media->RTPMaps, j);
			SDP_WRITE_ALLOC_STR(" ", 0);
			SDP_WRITE_ALLOC_INT(map->PayloadType, 0, 0);
		}
		SDP_WRITE_ALLOC_STR("\r\n", 0);

		//c=
		for (j=0; j<ChainGetCount(media->Connections); j++) {
			conn = ChainGetEntry(media->Connections, j);
			SDP_WRITE_CONN(conn);
		}

		//k=
		if (media->k_method) {
			SDP_WRITE_ALLOC_STR("k=", 0);
			SDP_WRITE_ALLOC_STR(media->k_method, 0);
			if (media->k_key) {
				SDP_WRITE_ALLOC_STR(":", 0);
				SDP_WRITE_ALLOC_STR(media->k_key, 0);
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		//b
		for (j=0; j<ChainGetCount(media->Bandwidths); j++) {
			bw = ChainGetEntry(media->Bandwidths, j);
			SDP_WRITE_ALLOC_STR("b=", 0);
			SDP_WRITE_ALLOC_STR(bw->name, 0);
			SDP_WRITE_ALLOC_STR(":", 0);
			SDP_WRITE_ALLOC_INT(bw->value, 0, 0);
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}

		//a=rtpmap
		for (j=0; j<ChainGetCount(media->RTPMaps); j++) {
			map = ChainGetEntry(media->RTPMaps, j);

			SDP_WRITE_ALLOC_STR("a=rtpmap", 0);
			SDP_WRITE_ALLOC_STR(":", 0);
			SDP_WRITE_ALLOC_INT(map->PayloadType, 1, 0);
			SDP_WRITE_ALLOC_STR(map->payload_name, 0);
			SDP_WRITE_ALLOC_STR("/", 0);
			SDP_WRITE_ALLOC_INT(map->ClockRate, 0, 0);
			if (map->AudioChannels > 1) {
				SDP_WRITE_ALLOC_STR("/", 0);
				SDP_WRITE_ALLOC_INT(map->AudioChannels, 0, 0);
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		//a=fmtp
		for (j=0; j<ChainGetCount(media->FMTP); j++) {
			fmtp = ChainGetEntry(media->FMTP, j);
			SDP_WRITE_ALLOC_STR("a=fmtp:", 0);
			SDP_WRITE_ALLOC_INT(fmtp->PayloadType, 1 , 0);
			for (k=0; k<ChainGetCount(fmtp->Attributes); k++) {
				if (k) SDP_WRITE_ALLOC_STR(";", 0);

				att = ChainGetEntry(fmtp->Attributes, k);
				SDP_WRITE_ALLOC_STR(att->Name, 0);
				if (att->Value) {
					SDP_WRITE_ALLOC_STR("=", 0);
					SDP_WRITE_ALLOC_STR(att->Value, 0);
				}
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		//a=ptime
		if (media->PacketTime) {
			SDP_WRITE_ALLOC_STR("a=ptime:", 0);
			SDP_WRITE_ALLOC_INT(media->PacketTime, 0, 0);
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		//a=FrameRate
		if (media->Type == 1 && media->FrameRate) {
			SDP_WRITE_ALLOC_STR("a=framerate:", 0);
			SDP_WRITE_ALLOC_FLOAT(media->FrameRate, 0);
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
		//a=SendRecv
		switch (media->SendRecieve) {
		case 1:
			TEST_SDP_WRITE_SINGLE("a=", "recvonly", 0);
			break;
		case 2:
			TEST_SDP_WRITE_SINGLE("a=", "sendonly", 0);
			break;
		case 3:
			TEST_SDP_WRITE_SINGLE("a=", "sendrecv", 0);
			break;
		default:
			break;
		}
		//a=orient
		TEST_SDP_WRITE_SINGLE("a=orient", media->orientation, 1);
		//a=sdplang
		TEST_SDP_WRITE_SINGLE("a=sdplang", media->sdplang, 1);
		//a=lang
		TEST_SDP_WRITE_SINGLE("a=lang", media->lang, 1);
		//a=quality
		if (media->Quality >= 0) {
			SDP_WRITE_ALLOC_STR("a=quality:", 0);
			SDP_WRITE_ALLOC_INT(media->Quality, 0, 0);
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}		
		//the rest
		for (j=0; j<ChainGetCount(media->Attributes); j++) {
			att = ChainGetEntry(media->Attributes, j);
			SDP_WRITE_ALLOC_STR("a=", 0);
			SDP_WRITE_ALLOC_STR(att->Name, 0);
			if (att->Value) {
				SDP_WRITE_ALLOC_STR(":", 0);
				SDP_WRITE_ALLOC_STR(att->Value, 0);
			}
			SDP_WRITE_ALLOC_STR("\r\n", 0);
		}
	}

	//finally realloc
	//finall NULL char
	pos += 1;
	buf = realloc(buf, pos);
	*out_str_buf = buf;
	return M4OK;
}	

