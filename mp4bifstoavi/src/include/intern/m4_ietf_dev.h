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

#ifndef	_M4_IETF_DEV_H_
#define _M4_IETF_DEV_H_

#include <gpac/m4_ietf.h>

/*
			RTP intern
*/

typedef struct
{
	/*version of the packet. Must be 2*/
	u8 Version;
	/*padding bits at the end of the payload*/
	u8 Padding;
	/*number of reports*/
	u8 Count;
	/*payload type of RTCP pck*/
	u8 PayloadType;
	/*The length of this RTCP packet in 32-bit words minus one including the header and any padding*/
	u16 Length;
	/*sync source identifier*/
	u32 SSRC;
} RTCPHeader;	


typedef struct __PRO_item
{
	struct __PRO_item *next;
	u32 pck_seq_num;
	void *pck;
	u32 size;
} PRO_item;

typedef struct __PO
{
	struct __PRO_item *in;
	u32 head_seqnum;
	u32 Count;
	u32 MaxCount;
	u32 IsInit;
	u32 MaxDelay, LastTime;
} RTPReorder;

/* creates new RTP reorderer
	@MaxCount: forces automatic packet flush. 0 means no flush
	@MaxDelay: is the max time in ms the queue will wait for a missing packet
*/
RTPReorder *NewPckReoreder(u32 MaxCount, u32 MaxDelay);
void PO_Delete(RTPReorder *po);
/*reset the Queue*/
void PO_Reset(RTPReorder *po);

/*Adds a packet to the queue. Packet Data is memcopied*/
M4Err PO_AddPacket(RTPReorder *po, void *pck, u32 pck_size, u32 pck_seqnum);
/*gets the output of the queue. Packet Data IS YOURS to delete*/
void *PO_GetPacket(RTPReorder *po, u32 *pck_size);


/*the RTP channel with both RTP and RTCP sockets and buffers
each channel is identified by a control string given in RTSP Describe
this control string is used with Darwin
*/
struct _tagRTPChannel
{
	/*global transport info for the session*/
	RTSPTransport net_info;
	
	/*RTP CHANNEL*/
	M4Socket *rtp;
	/*RTCP CHANNEL*/
	M4Socket *rtcp;
	
	/*RTP Packet reordering. Turned on/off during initialization. The library forces a 200 ms
	max latency at the reordering queue*/
	RTPReorder *po;

	/*RTCP report times*/
	u32 last_report_time;
	u32 next_report_time;
	
	/*the seq number of the first packet as signaled by the server if any, or first
	RTP SN recieved (RTP multicast)*/
	u32 rtp_first_SN;
	/*the TS of the associated first packet as signaled by the server if any, or first
	RTP TS recieved (RTP multicast)*/
	u32 rtp_time;
	/*NPT from the rtp_time*/
	u32 CurrentTime;
	/*num loops of pck sn*/
	u32 num_sn_loops;
	/*some mapping info - we should support # payloads*/
	char PayloadName[40];
	u8 PayloadType;
	u32 TimeScale;

	/*static buffer for RTP sending*/
	char *send_buffer;
	u32 send_buffer_size;
	u32 pck_sent_since_last_sr;
	u32 last_pck_ts;
	u32 last_pck_ntp_sec, last_pck_ntp_frac;
	u32 num_pck_sent, num_payload_bytes;

	/*RTCP info*/
	char *s_name, *s_email, *s_location, *s_phone, *s_tool, *s_note, *s_priv;
	s8 first_rtp_pck;
	s8 first_SR;
	u32 SSRC;
	u32 SenderSSRC;

	u32 last_pck_sn;

	char *CName;

	u32 rtcp_bytes_sent;
	/*total pck rcv*/
	u32 tot_num_pck_rcv, tot_num_pck_expected;
	/*stats since last SR*/
	u32 last_num_pck_rcv, last_num_pck_expected, last_num_pck_loss;
	/*jitter compute*/
	u32 Jitter;
	s32 last_deviance;	
	/*NTP of last SR*/
	u32 last_SR_NTP_sec, last_SR_NTP_frac;
	/*RTP time at last SR as indicated in SR*/
	u32 last_SR_rtp_time;
	/*payload info*/
	u32 total_pck, total_bytes;

	FILE *rtp_log;
};

/*gets UTC in the channel RTP timescale*/
u32 RTP_ChannelTime(RTPChannel *ch);
/*gets time in 1/65536 seconds (for reports)*/
u32 RTP_GetReportTime();
/*updates the time for the next report (SR, RR)*/
void RTCP_GetNextReportTime(RTPChannel *ch);


/*
			RTSP intern
*/

#define RTSP_DEFAULT_BUFFER		2048
#define RTSP_VERSION		"RTSP/1.0"

/*macros for RTSP command and response formmating*/
#define RTSP_WRITE_STEPALLOC	250

#define RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, str)		\
		if (str) {		\
			if (strlen(str)+pos >= buf_size) {	\
				buf_size += RTSP_WRITE_STEPALLOC;	\
				buf = realloc(buf, buf_size);		\
			}	\
			strcpy(buf+pos, str);		\
			pos += strlen(str);		\
		}

#define RTSP_WRITE_HEADER(buf, buf_size, pos, type, str)		\
	if (str) {		\
		RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, type);		\
		RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, ": ");		\
		RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, str);		\
		RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, "\r\n");		\
	}

#define RTSP_WRITE_INT(buf, buf_size, pos, d, sig)		\
	if (sig) { \
		sprintf(temp, "%d", d);		\
	} else { \
		sprintf(temp, "%u", d);		\
	}	\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, temp);

#define RTSP_WRITE_FLOAT(buf, buf_size, pos, d)		\
	sprintf(temp, "%.4f", d);		\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, temp);

/*default packet size, but resize on the fly if needed*/
#define RTSP_PCK_SIZE			6000
#define RTSP_TCP_BUF_SIZE		0x10000ul


typedef struct
{
	u8 rtpID;
	u8 rtcpID;
	void *ch_ptr;
} TCPChan;

/**************************************
		RTSP Session
***************************************/
typedef struct _tagRTSPSess
{
	/*service name (extracted from URL) ex: news/latenight.mp4, vod.mp4 ...*/
	char *Service;	
	/*server name (extracted from URL)*/
	char *Server;
	/*server port (extracted from URL)*/
	u16 Port;

	/*if RTSP is on UDP*/
	u8 ConnectionType;
	/*TCP interleaving ID*/
	u8 InterID;
	/*http tunnel*/
	Bool HasTunnel;
	M4Socket *http;
	char HTTP_Cookie[30];
	u32 CookieRadLen;

	/*RTSP CHANNEL*/
	M4Socket *connection;
	u32 SockBufferSize;
	/*needs connection*/
	u32 NeedConnection;

	/*the RTSP sequence number*/
	u32 CSeq;
	/*this is for aggregated request in order to check SeqNum*/
	u32 NbPending;

	/*RTSP sessionID, arbitrary length, alpha-numeric*/
	char *SessionID;

	/*RTSP STATE machine*/
	u32 RTSP_State;
	char RTSPLastRequest[40];

	/*current buffer from TCP if any*/
	char TCPBuffer[RTSP_TCP_BUF_SIZE];
	u32 CurrentSize, CurrentPos;

	/*RTSP interleaving*/
	M4Err (*RTSP_SignalData)(LPRTSPSESSION sess, void *chan, char *buffer, u32 bufferSize, Bool IsRTCP);
	
	/*buffer for pck reconstruction*/
	char *rtsp_pck_buf;
	u32 rtsp_pck_size;
	u32 pck_start, payloadSize;

	/*all RTP channels in an interleaved RTP on RTSP session*/
	Chain *TCPChannels;
	/*thread-safe, full duplex library for PLAY and RECORD*/
	M4Mutex *mx;

	FILE *rtsp_log;
} RTSPSession;

RTSPSession *RTSP_NewSession(char *sURL, u16 DefaultPort);
/*unpack the URL in [RAD]server:port/service[/ctrlID] and remove [ctrlID] for RAD = rtsp or rtspu*/
M4Err RTSP_UnpackURL(char *sURL, char *Server, u16 *Port, char *Service, Bool *useTCP);
/*check connection status*/
M4Err RTSP_CheckConnection(RTSPSession *sess);
/*send data on RTSP*/
M4Err RTSP_Send(RTSPSession *sess, unsigned char *buffer, u32 Size);

/* 
			Common RTSP tools
*/

/*locate body-start and body size in response/commands*/
void RTSP_GetBodyInfo(RTSPSession *sess, u32 *body_start, u32 *body_size);
/*read TCP until a full command/response is recieved*/
M4Err RTSP_ReadReply(LPRTSPSESSION sess);
/*fill the TCP buffer*/
M4Err RTSP_FillTCPBuffer(RTSPSession *sess);
/*force a fill on TCP buffer - used for de-interleaving and TCP-fragmented RTSP messages*/
M4Err RTSP_RefillBuffer(RTSPSession *sess);
/*parses a transport string and returns a transport structure*/
RTSPTransport *ParseTransport(char *buffer);
/*parsing of header for com and rsp*/
M4Err RTSP_ParseHeaderLines(char *buffer, u32 BufferSize, u32 BodyStart, RTSPCommand *com, RTSPResponse *rsp);
void RTSP_SetCommandVal(RTSPCommand *com, char *Header, char *Value);
void RTSP_SetResponseVal(RTSPResponse *rsp, char *Header, char *Value);
/*deinterleave a data packet*/
M4Err RTSP_DeInterleave(RTSPSession *sess);
/*start session through HTTP tunnel (QTSS)*/
M4Err RTSP_HTTPStart(RTSPSession *sess, char *UserAgent);



#endif

