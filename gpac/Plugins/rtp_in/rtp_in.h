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

#ifndef RTP_IN_H
#define RTP_IN_H

/*plugin interface*/
#include <gpac/m4_network.h>
/*IETF lib*/
#include <gpac/m4_ietf.h>
/*RTP to SL map and payload types*/
#include <gpac/m4_author.h>


#define RTP_BUFFER_SIZE			0x100000ul
#define RTSP_BUFFER_SIZE		5000
#define RTSP_TCP_BUFFER_SIZE    0x100000ul
#define RTSP_CLIENTNAME		"GPAC " M4_VERSION " RTSP Client"
#define RTSP_LANGUAGE		"English"


/*the rtsp/rtp client*/
typedef struct
{
	/*the service we're responsible for*/
	LPNETSERVICE service;

	/*the one and only IOD*/
	char *session_iod;
	u32 session_iod_len;

	/*current rtsp session if any*/
	struct _rtp_session *rtsp_session;

	Chain *channels;

	/*sdp downloader*/
	LPFILEDOWNLOADER dnload;
	/*initial sdp download if any (temp storage)*/
	struct _sdp_fetch *sdp_temp;


	M4Mutex *mx;
	M4Thread *th;
	Bool run_client, client_exit, do_exit;


	/*RTSP config*/
	/*transport mode. 0 is udp, 1 is tcp, 2 is tcp with HTTP tunnel*/
	u32 rtp_mode;
	/*default RTSP port*/
	u16 default_port;
	/*net bandwidth (connection speed) in bits per sec*/
	u32 bandwidth;
	/*if set ANNOUNCE (sent by server) will be handled*/
	Bool handle_announce;
	/*signaling timeout in msec*/
	u32 time_out;
	/*udp timeout in msec*/
	u32 udp_time_out;
	/*use rtp reordering*/
	u32 reorder_size;
	/*amount of buffering in ms*/
	u32 buffer_len_ms;
	/*rebuffer low limit in ms - if the amount of buffering is less than this, rebuffering will never occur*/
	u32 rebuffer_len_ms;

	/*packet drop emulation*/
	u32 first_packet_drop;
	u32 frequency_drop;

	/*OD data for static OD stream - we don't use the usual base64 encoding in OD ESD URL since data may be much
	larger that what ESD.URL supports (255 bytes)*/
	u32 od_au_size, od_es_id, state;
	Bool od_fetched;
	char *od_au;
	LPNETCHANNEL od_ch;
	u32 od_start_time;

	/*logs*/
	FILE *logs;
} RTPClient;

/*rtsp session*/
typedef struct _rtp_session
{
	/*owner*/
	RTPClient *owner;

	/*RTSP session object*/
	LPRTSPSESSION session;
	
	/*session control string*/
	char *control;

	/*response object*/
	RTSPResponse *rtsp_rsp;

	Bool has_aggregated_control;
	Float last_range;

	Bool wait_for_reply;
	u32 command_time;
	Chain *rtsp_commands;
} RTPSession;

/*creates new RTSP session handler*/
RTPSession *RP_NewSession(RTPClient *rtp, char *session_control);
/*disconnects and destroy RTSP session handler - if immediate_shutdown do not wait for response*/
void RP_RemoveSession(RTPSession *sess, Bool immediate_shutdown);
/*check session by control string*/
RTPSession *RP_CheckSession(RTPClient *rtp, char *control);

void RP_ProcessCommands(RTPSession *sess, Bool read_tcp);

/*RTP channel state*/
enum
{
	/*channel is setup and waits for connection request*/
	RTP_Setup = 0,
	/*waiting for server reply*/
	RTP_WaitingForAck,
	/*connection OK*/
	RTP_Connected,
	/*data exchange on this service/channel*/
	RTP_Running,
	/*deconnection OK - a download channel can automatically disconnect when download is done*/
	RTP_Disconnected,
	/*service/channel is not (no longer) available/found and should be removed*/
	RTP_Unavailable
};
/*rtp channel*/
typedef struct
{
	/*plugin*/
	RTPClient *owner;
	/*payloat type*/
	u32 rtptype;

	/*control session (may be null)*/
	RTPSession *rtsp;

	/*logical app channel*/
	LPNETCHANNEL channel;
	u32 status;

	/*RTP channel*/
	RTPChannel *rtp_ch;
	
	u32 ES_ID;
	char *control;

	/*depacketizer config*/
	RTPSLMap sl_map;
	SLHeader sl_hdr;

	/*rtp recieve buffer*/
	char buffer[RTP_BUFFER_SIZE];
	/*set at seek stages to resync app NPT to RTP time*/
	u32 check_rtp_time;

	/*can we control the stream ?*/
	Bool has_range;
	Float range_start, range_end;

	/*RTSP control aggregation state*/
	Bool skip_next_command;

	/*current start time in npt (for pause/resume)*/
	Float current_start;
	u32 clock_rate;
	/*needed for DTS/CTS compute with some payloads type*/
	u32 unit_duration;

	Bool new_au;
	/*internal flag, indicating whether channel creation has been acknowledged or not
	this is needed to filter real channel_connect calls from RTSP re-setup (after STOP) ones*/
	Bool connected;

	/*is RTP over RTSP*/
	Bool is_interleaved;
	Bool end_of_stream;

	/*stream is setup but not playing...*/
	Bool idle;

	/*AMR config*/
	Bool octet_align;
	/*3GP text reassembler state*/
	u8 nb_txt_frag, cur_txt_frag, sidx, txt_len, nb_mod_frag;
	u32 au_dur;
	BitStream *ttu1;

	/*UDP detection*/
	u32 last_udp_time;

	/*stats*/
	u32 rtp_bytes, rtcp_bytes, stat_start_time, stat_stop_time;
} RTPStream;

/*creates new RTP stream from SDP info*/
RTPStream *RP_NewStream(RTPClient *rtp, SDPMedia *media, SDPInfo *sdp, RTPStream *input_stream);
/*destroys RTP stream */
void RP_DeleteStream(RTPStream *ch);
/*resets stream state and inits RTP sockets if ResetOnly is false*/
M4Err RP_InitStream(RTPStream *ch, Bool ResetOnly);
/*disconnect stream but keeps its config alive*/
void RP_DisconnectStream(RTPStream *ch);

/*RTSP -> RTP de-interleaving callback*/
M4Err RP_DataOnTCP(LPRTSPSESSION sess, void *cbck, char *buffer, u32 bufferSize, Bool IsRTCP);
/*send confirmation of connection - if no error, also setup SL based on payload*/
void RP_ConfirmChannelConnect(RTPStream *ch, M4Err e);

/*fetch sdp file - stream is the RTP channel this sdp describes, or NULL if session sdp*/
void RP_FetchSDP(NetClientPlugin *plug, char *url, RTPStream *stream);

/*locate RTP stream by channel or ES_ID or control*/
RTPStream *RP_FindChannel(RTPClient *rtp, LPNETCHANNEL ch, u32 ES_ID, char *es_control, Bool remove_stream);
/*adds channel to session identified by session_control. If no session exists, the session is created if needed*/
M4Err RP_AddStream(RTPClient *rtp, RTPStream *stream, char *session_control);
/*removes stream from session*/
void RP_RemoveStream(RTPClient *rtp, RTPStream *ch);
/*reads input socket and process*/
void RP_ReadStream(RTPStream *ch);

/*parse RTP payload for MPEG4*/
void RP_ParsePayloadMPEG4(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size);
/*parse RTP payload for MPEG12*/
void RP_ParsePayloadMPEG12(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size);
/*parse RTP payload for AMR*/
void RP_ParsePayloadAMR(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size);
/*parse RTP payload for H263+*/
void RP_ParsePayloadH263(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size);
/*parse RTP payload for 3GPP Text*/
void RP_ParsePayloadText(RTPStream *ch, RTPHeader *hdr, char *payload, u32 size);

/*load SDP and setup described media in SDP. If stream is null this is the root
SDP and IOD will be extracted, otherwise this a channel SDP*/
void RP_LoadSDP(RTPClient *rtp, char *sdp, u32 sdp_len, RTPStream *stream);

/*returns 1 if payload type is supported*/
u32 payt_get_type(RTPClient *rtp, SDP_RTPMap *map, SDPMedia *media);
/*setup payload type*/
void payt_setup(RTPStream *st, SDP_RTPMap *map, SDPMedia *media);


/*RTSP signaling is handled by stacking commands and processing them
in the main session thread. Each RTSP command has an associated private stack as follows*/

/*describe stack for single channel (not for session)*/
typedef struct
{
	u32 ES_ID;
	LPNETCHANNEL channel;
	char *esd_url;
} ChannelDescribe;

typedef struct
{
	RTPStream *ch;
	NetworkCommand com;
} ChannelControl;

/*RTSP signaling */
Bool RP_PreprocessDescribe(RTPSession *sess, RTSPCommand *com);
void RP_ProcessDescribe(RTPSession *sess, RTSPCommand *com, M4Err e);
void RP_ProcessSetup(RTPSession *sess, RTSPCommand *com, M4Err e);
void RP_ProcessTeardown(RTPSession *sess, RTSPCommand *com, M4Err e);
Bool RP_PreprocessUserCom(RTPSession *sess, RTSPCommand *com);
void RP_ProcessUserCommand(RTPSession *sess, RTSPCommand *com, M4Err e);

/*send describe - if esd_url is given, this is a describe on es*/
void RP_Describe(RTPSession *sess, char *esd_url, LPNETCHANNEL channel);
/*send setup for stream*/
void RP_Setup(RTPStream *ch);
/*filter setup if no session (rtp only), otherwise setup channel - ch_desc may be NULL
if channel association is already done*/
M4Err RP_SetupChannel(RTPStream *ch, ChannelDescribe *ch_desc);
/*send command for stream - handles aggregation*/
void RP_UserCommand(RTPSession *sess, RTPStream *ch, NetworkCommand *command);
/*disconnect the session - currently no servers allow for channel disconnection only
thus the full session is shut down ...*/
void RP_Teardown(RTPSession *sess);

/*emulate IOD*/
M4Err RP_EmulateIOD(RTPClient *rtp, u32 expect_type, const char *sub_url);



/*sdp file downloader*/
typedef struct _sdp_fetch
{
	RTPClient *client;
	/*when loading a channel from SDP*/
	RTPStream *chan;

	char *remote_url;
} SDPFetch;

#endif


