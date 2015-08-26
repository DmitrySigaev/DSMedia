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


#ifndef	M4_IETF_H_
#define M4_IETF_H_



#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/m4_tools.h>


/****************************************************************************

				RTSP VERSION 1.0 LIBRARY EXPORTS

****************************************************************************/

#define RTSP_VERSION		"RTSP/1.0"


/*
 *		RTSP NOTIF CODES
 */
enum
{
	NC_RTSP_Continue		=		100,
	NC_RTSP_OK				=		200,
	NC_RTSP_Created			=		201,
	NC_RTSP_Low_on_Storage_Space	=	250,

	NC_RTSP_Multiple_Choice	=	300,
	NC_RTSP_Moved_Permanently	=	301,
	NC_RTSP_Moved_Temporarily	=	302,
	NC_RTSP_See_Other	=	303,
	NC_RTSP_Use_Proxy	=	305,

	NC_RTSP_Bad_Request	=	400,
	NC_RTSP_Unauthorized	=	401,
	NC_RTSP_Payment_Required	=	402,
	NC_RTSP_Forbidden	=	403,
	NC_RTSP_Not_Found	=	404,
	NC_RTSP_Method_Not_Allowed	=	405,
	NC_RTSP_Not_Acceptable	=	406,
	NC_RTSP_Proxy_Authentication_Required	=	407,
	NC_RTSP_Request_Timeout	=	408,
	NC_RTSP_Gone	=	410,
	NC_RTSP_Length_Required	=	411,
	NC_RTSP_Precondition_Failed	=	412,
	NC_RTSP_Request_Entity_Too_Large	=	413,
	NC_RTSP_Request_URI_Too_Long	=	414,
	NC_RTSP_Unsupported_Media_Type	=	415,

	NC_RTSP_Invalid_parameter	=	451,
	NC_RTSP_Illegal_Conference_Identifier	=	452,
	NC_RTSP_Not_Enough_Bandwidth	=	453,
	NC_RTSP_Session_Not_Found	=	454,
	NC_RTSP_Method_Not_Valid_In_This_State	=	455,
	NC_RTSP_Header_Field_Not_Valid	=	456,
	NC_RTSP_Invalid_Range	=	457,
	NC_RTSP_Parameter_Is_ReadOnly	=	458,
	NC_RTSP_Aggregate_Operation_Not_Allowed	=	459,
	NC_RTSP_Only_Aggregate_Operation_Allowed	=	460,
	NC_RTSP_Unsupported_Transport	=	461,
	NC_RTSP_Destination_Unreachable	=	462,
	
	NC_RTSP_Internal_Server_Error	=	500,
	NC_RTSP_Not_Implemented	=	501,
	NC_RTSP_Bad_Gateway	=	502,
	NC_RTSP_Service_Unavailable	=	503,
	NC_RTSP_Gateway_Timeout	=	504,
	NC_RTSP_RTSP_Version_Not_Supported	=	505,

	NC_RTSP_Option_not_support	=	551,
};

const char *RTSP_NC_TO_String(u32 ErrCode);

/*
		Common structures between commands and responses
*/

/*
	RTSP Range information - RTSP Session level only (though this is almost the same
	format as an SDP range, this is not used in the SDP lib as "a=range" is not part of SDP
	but part of RTSP
*/
typedef struct {
	/* start and end range. If end<=start, the range is open (from start to unknown) */
	Float start, end;
	/* use SMPTE range (Start and End specify the number of frames) (currently not supported) */
	u32 UseSMPTE;
	/* framerate for SMPTE range */
	Float FPS;
} RTSPRange;

/*
parses a Range line and returns range header structure. can be used for RTSP extension of SDP
NB: Only support for npt for now
*/
RTSPRange *RTSP_ParseRange(char *range_buf);

RTSPRange *RTSP_NewRange();
void RTSP_DeleteRange(RTSPRange *range);

/*
			Transport structure 
		contains all network info for RTSP sessions (ports, uni/multi-cast, ...)
*/

/*
	Transport Profiles as defined in RFC 2326
*/
#define RTSP_PROFILE_RTP_AVP			"RTP/AVP"
#define RTSP_PROFILE_RTP_AVP_TCP		"RTP/AVP/TCP"
#define RTSP_PROFILE_UDP				"udp"


typedef struct
{
	/* set to 1 if unicast */
	Bool IsUnicast;
	/* for multicast */
	char *destination;
	/* for redirections internal to servers */
	char *source;
	/*IsRecord is usually 0 (PLAY) . If set, Append specify that the stream should
	be concatenated to existing resources */
	Bool IsRecord, Append;
	/* in case transport is on TCP/RTSP, If only 1 ID is specified, it is stored in rtpID (this
	is not RTP interleaving) */
	Bool IsInterleaved, rtpID, rtcpID;
	/* Multicast specific */
	u32 MulticastLayers;
	u8 TTL;
	/*RTP specific*/

	/*port for multicast*/
	/*server port in unicast - RTP implies low is even , and last is low+1*/
	u16 port_first, port_last;
	/*client port in unicast - RTP implies low is even , and last is low+1*/
	u16 client_port_first, client_port_last;
	u32 SSRC;

	/*Transport protocol. In this version we only support RTP/AVP, the following flag tells 
	us if this is RTP/AVP/TCP or RTP/AVP (default)*/
	char *Profile;
} RTSPTransport;


RTSPTransport *RTSP_DuplicateTransport(RTSPTransport *original);
void RTSP_DeleteTransport(RTSPTransport *transp);



/*
	structure containing the rtpmap information
*/
typedef struct
{
	/*dynamic payload type of this map*/
	u32 PayloadType;
	/*registered payload name of this map*/
	char *payload_name;
	/*RTP clock rate (TS resolution) of this map*/
	u32 ClockRate;
	/*optional parameters for audio, specifying number of channels. Unused for other media types.*/
	u32 AudioChannels;
} SDP_RTPMap;


/*
				RTSP Command
		the RTSP Response is sent by a client / recieved by a server
	text Allocation is done by the lib when parsing a command, and
	is automatically freed when calling reset / delete. Therefore you must
	set/allocate the fields yourself when writing a command (client)

*/

/*ALL RTSP METHODS - all other methods will be ignored*/
#define RTSP_DESCRIBE		"DESCRIBE"
#define RTSP_SETUP			"SETUP"
#define RTSP_PLAY			"PLAY"
#define RTSP_PAUSE			"PAUSE"
#define RTSP_RECORD			"RECORD"
#define RTSP_TEARDOWN		"TEARDOWN"
#define RTSP_GET_PARAMETER	"GET_PARAMETER"
#define RTSP_SET_PARAMETER	"SET_PARAMETER"
#define RTSP_OPTIONS		"OPTIONS"
#define RTSP_ANNOUNCE		"ANNOUNCE"
#define RTSP_REDIRECT		"REDIRECT"


typedef struct
{
	char *Accept;
	char *Accept_Encoding;
	char *Accept_Language;
	char *Authorization;
	u32 Bandwidth;
	u32 Blocksize;
	char *Cache_Control;
	char *Conference;
	char *Connection;
	u32 Content_Length;
	u32 CSeq;
	char *From;
	char *Proxy_Authorization;
	char *Proxy_Require;
	RTSPRange *Range;
	char *Referer;
	Float Scale;
	char *Session;
	Float Speed;
	/*nota : RTSP allows several configurations for a single channel (multicast and 
	unicast , ...). Usually only 1*/
	Chain *Transports;
	char *User_Agent;

	/*type of the command, one of the described above*/
	char *method;
	
	/*Header extensions*/
	Chain *Xtensions;

	/*body of the command, size is Content-Length (auto computed when sent). It is not 
	terminated by a NULL char*/
	char *body;

	/*
			Specify ControlString if your request targets
		a specific media stream in the service. If null, the service name only will be used
		for control (for ex, both A and V streams in a single file)
		If the request is RTSP_OPTIONS, you must provide a control string containing the options 
		you want to query
	*/
	char *ControlString;

	/*user data: this is never touched by the lib, its intend is to help stacking
	RTSP commands in your app*/
	void *user_data;


	/*
		Server side Extensions
	*/

	/*full URL of the command. Not used at client side, as the URL is ALWAYS relative
	to the server / service of the RTSP session 
	On the server side however redirections are up to the server, so we cannot decide for it	*/
	char *service_name;
	/*RTSP status code of the command as parsed. One of the above RTSP StatusCode*/
	u32 StatusCode;
} RTSPCommand, *LPRTSPCOMMAND;


RTSPCommand *RTSP_NewCommand();
void RTSP_DeleteCommand(RTSPCommand *com);
void RTSP_ResetCommand(RTSPCommand *com);



/*
				RTSP Response
		the RTSP Response is recieved by a client / sent by a server
	text Allocation is done by the lib when parsing a response, and
	is automatically freed when calling reset / delete. Therefore you must
	allocate the fields yourself when writing a response (server)

*/

/*
	RTP-Info for RTP channels. There may be several RTP-Infos in one response
	based on the server implementation (DSS/QTSS begaves this way)
*/
typedef struct
{
	/*control string of the channel*/
	char *url;
	/*seq num for asociated rtp_time*/
	u32 seq;
	/*rtp TimeStamp corresponding to the Range start specified in the PLAY request*/
	u32 rtp_time;
	/*ssrc of sender if known, 0 otherwise*/
	u32 ssrc;
} RTP_Info;



/*
	All attributes x-ZZZZ are considered as extensions attributes. If no "x-" is found 
	the attributes in the RTSP response is SKIPPED. The "x-" radical is removed in the structure
	when parsing commands / responses
*/
typedef struct
{
	char *Name;
	char *Value;
} X_Attribute;




/*
	RTSP Response
*/
typedef struct _tagRTSPRsp
{
	/* response code*/
	u32 ResponseCode;
	/* comment from the server */
	char *ResponseInfo;
	
	/*	Header Fields	*/
	char *Accept;
	char *Accept_Encoding;
	char *Accept_Language;
	char *Allow;
	char *Authorization;
	u32 Bandwidth;
	u32 Blocksize;
	char *Cache_Control;
	char *Conference;
	char *Connection;
	char *Content_Base;
	char *Content_Encoding;
	char *Content_Language;
	u32 Content_Length;
	char *Content_Location;
	char *Content_Type;
	u32 CSeq;
	char *Date;
	char *Expires;
	char *From;
	char *Host;
	char *If_Match;
	char *If_Modified_Since;
	char *Last_Modified;
	char *Location;
	char *Proxy_Authenticate;
	char *Proxy_Require;
	char *Public;
	RTSPRange *Range;
	char *Referer;
	char *Require;
	char *Retry_After;
	Chain *RTP_Infos;
	Float Scale;
	char *Server;
	char *Session;
	u32 SessionTimeOut;
	Float Speed;
	char *Timestamp;
	/*nota : RTSP allows several configurations for a single channel (multicast and 
	unicast , ...). Usually only 1*/
	Chain *Transports;
	char *Unsupported;
	char *User_Agent;
	char *Vary;
	char *Via;
	char *WWW_Authenticate;

	/*Header extensions*/
	Chain *Xtensions;

	/*body of the response, size is Content-Length (auto computed when sent). It is not 
	terminated by a NULL char when response is parsed but must be null-terminated when 
	response is being sent*/
	char *body;
} RTSPResponse;


RTSPResponse *RTSP_NewResponse();
void RTSP_DeleteResponse(RTSPResponse *rsp);
void RTSP_ResetResponse(RTSPResponse *rsp);



typedef struct _tagRTSPSess *LPRTSPSESSION;

LPRTSPSESSION RTSP_NewSession(char *sURL, u16 DefaultPort);
void RTSP_DeleteSession(LPRTSPSESSION sess);

M4Err RTSP_SetBufferSize(LPRTSPSESSION sess, u32 BufferSize);


/*Reset state machine, invalidate SessionID
NOTE: RFC2326 requires that the session is reseted when all RTP streams
are closed. As this lib doesn't maintain the number of valid streams
you MUST call reset when all your streams are shutdown (either requested through
TEARDOWN or signaled through RTCP BYE packets for RTP, or any other signaling means
for other protocols)
reset connection will destroy the socket - this is isefull in case of timeouts, because
some servers do not restart with the right CSeq...*/
void RTSP_ResetSession(LPRTSPSESSION sess, Bool ResetConnection);

u32 RTSP_IsMySession(LPRTSPSESSION sess, char *url);
char *RTSP_GetSessionID(LPRTSPSESSION sess);
char *RTSP_GetServerName(LPRTSPSESSION sess);
char *RTSP_GetServiceName(LPRTSPSESSION sess);
u16 RTSP_GetSessionPort(LPRTSPSESSION sess);

/*Fetch an RTSP response from the server the RTSPResponse will be reseted before fetch*/
M4Err RTSP_GetResponse(LPRTSPSESSION sess, RTSPResponse *rsp);


/*RTSP State Machine. The only non blocking mode is RSM_WaitForControl*/
enum
{
	/*Initialized (connection might be off, but all structures are in place)
	This is the default state between # requests (aka, DESCRIBE and SETUP
	or SETUP and PLAY ...)*/
	RSM_Init	=	0,
	/*Waiting*/
	RSM_Waiting,
	/*PLAY, PAUSE, RECORD. Aggregation is allowed for the same type, you can send several command
	in a row. However the session will return M4ServiceError if you do not have 
	a valid SessionID in the command
	You cannot issue a SETUP / DESCRIBE while in this state*/
	RSM_WaitForControl,

	/*FATAL ERROR: session is invalidated by server. Call reset and restart from SETUP if needed*/
	RSM_Invalidated
};

u32 RTSP_GetSessionState(LPRTSPSESSION sess);
/*aggregate command state-machine: the PLAY/PAUSE can be aggregated 
(sent before the reply is recieved). This function gets the last command sent*/
char *RTSP_GetLastRequest(LPRTSPSESSION sess);
/*foce a reset in case of pbs*/
void RTSP_ResetAggregation(LPRTSPSESSION sess);

/*
	Send an RTSP request to the server.
*/
M4Err RTSP_SendRequest(LPRTSPSESSION sess, RTSPCommand *com);


M4Err RTSP_SetCallbackOnInterleaving(LPRTSPSESSION sess,
						M4Err (*SignalData)(LPRTSPSESSION sess, void *cbk_ptr, char *buffer, u32 bufferSize, Bool IsRTCP)
				);


M4Err RTSP_ReadTCP(LPRTSPSESSION sess);

M4Err RTSP_RegisterTCPChannel(LPRTSPSESSION sess, void *the_ch, u8 LowInterID, u8 HighInterID);
u32 RTSP_UnregisterTCPChannel(LPRTSPSESSION sess, u8 LowInterID);



/*
			Server side session constructor
	create a new RTSP session from an existing socket in listen state. If no pending connection
	is detected, return NULL
*/
LPRTSPSESSION RTSPS_NewSession(M4Socket *rtsp_listener);

/*fetch an RTSP request. The RTSPCommand will be reseted before fetch*/
M4Err RTSP_GetRequest(LPRTSPSESSION sess, RTSPCommand *com);

/*unpack the URL, check the service name / server. Typically used when a client sends a 
 DESCRIBE || SETUP url RTSP/1.0. Server / service name check must be performed by your app as redirection
or services available are unknown here.*/
M4Err RTSP_LoadServiceName(LPRTSPSESSION sess, char *URL);

/*assign a given sessionID to a session, or let the lib generate a new one
by specifying a NULL session ID*/
M4Err RTSP_AssignSessionID(LPRTSPSESSION sess, char *custom_id);

/*send the RTSP response*/
M4Err RTSP_SendResponse(LPRTSPSESSION sess, RTSPResponse *rsp);

/*gets the IP address of the local host running the session
buffer shall be MAX_IP_NAME_LEN long*/
M4Err RTSP_GetSessionIP(LPRTSPSESSION sess, char *buffer);

/*returns the next available ID for interleaving. It is recommended that you use 2 
consecutive IDs for RTP/RTCP interleaving*/
u8 RTSP_GetNextInterleavedID(LPRTSPSESSION sess);

/*gets the IP address of the connected peer - buffer shall be MAX_IP_NAME_LEN long*/
M4Err RTSP_GetRemoteAddress(LPRTSPSESSION sess, char *buffer);

/*sets log file for RTSP session*/
void RTSP_SetLog(LPRTSPSESSION sess, FILE *log);


/****************************************************************************

					RTP LIBRARY EXPORTS

****************************************************************************/


typedef struct tagRTP_HEADER {
	/*version, must be 2*/
	u8 Version;
	/*padding bits in the payload*/
	u8 Padding;
	/*header extension is defined*/
	u8 Extension;
	/*number of CSRC (<=15)*/
	u8 CSRCCount;
	/*Marker Bit*/
	u8 Marker;
	/*payload type on 7 bits*/
	u8 PayloadType;
	/*packet seq number*/
	u16 SequenceNumber;
	/*packet time stamp*/
	u32 TimeStamp;
	/*sync source identifier*/
	u32 SSRC;
	/*in our basic client, CSRC should always be NULL*/
	u32 CSRC[16];
} RTPHeader;

typedef struct _tagRTPChannel RTPChannel;

RTPChannel *RTP_NewChannel();
void RTP_DeleteChannel(RTPChannel *ch);

/*you configure a server channel through the transport structure, with the same info as a 
client channel, the client_port_* info designing the REMOTE client and port_* designing
your server channel*/
M4Err RTP_SetupTransport(RTPChannel *ch, RTSPTransport *trans_info, char *remote_address);

/*auto-setup of rtp/rtcp transport ports - only effective in unicast, non interleaved cases. 
for multicast port setup MUST be done through the above RTP_SetupTransport function
this will take care of port reuse*/
M4Err RTP_SetupPorts(RTPChannel *ch);

/*init of payload information. only ONE payload per sync source is supported in this
version of the library (a sender cannot switch payload types on a single media)*/
M4Err RTP_SetupPayload(RTPChannel *ch, SDP_RTPMap *map);


/*initialize the RTP channel.

UDPBufferSize: UDP stack buffer size if configurable by OS/ISP - ignored otherwise
NOTE: on WinCE devices, this is not configurable on an app bases but for the whole OS
you must update the device registry with:
	[HKEY_LOCAL_MACHINE\Comm\Afd]
	DgramBuffer=dword:N

	where N is the number of UDP datagrams a socket should be able to buffer. For multimedia
app you should set N as large as possible. The device MUST be reseted for the param to take effect

ReorederingSize: max number of packets to queue for reordering. 0 means no reordering
MaxReorderDelay: max time to wait in ms before releasing first packet in reoderer when only one packet is present.
If 0 and reordering size is specified, defaults to 200 ms (usually enough).
IsSource: if true, the channel is a sender (media data, sender report, Reciever report processing)
if source, you must specify the Path MTU size. The RTP lib won't send any packet bigger than this size
your application shall perform payload size splitting if needed*/
M4Err RTP_Initialize(RTPChannel *ch, u32 UDPBufferSize, Bool IsSource, u32 PathMTU, u32 ReorederingSize, u32 MaxReorderDelay);

/*init the RTP info after a PLAY or PAUSE, rtp_time is the rtp TimeStamp of the RTP packet
with seq_num sequence number. This info is needed to compute the CurrentTime of the RTP channel 
ssrc may not be known if sender hasn't indicated it (use 0 then)*/
M4Err RTP_SetInfo(RTPChannel *ch, u32 seq_num, u32 rtp_time, u32 ssrc);

/*retrieve current RTP time in sec. If rtp_time was unknown (not on demand media) the time is absolute.
Otherwise this is the time in ms elapsed since the last PLAY range start value
Not supported yet if played without RTSP (aka RTCP time not supported)*/
Float RTP_GetCurrentTime(RTPChannel *ch);


void RTP_ResetBuffers(RTPChannel *ch);

/*read any data on UDP only (not valid for TCP). Performs re-ordering if configured for it
returns amount of data read (raw UDP packet size)*/
u32 RTP_ReadChannel(RTPChannel *ch, char *buffer, u32 buffer_size);
u32 RTCP_ReadChannel(RTPChannel *ch, char *buffer, u32 buffer_size);

/*decodes an RTP packet and gets the begining of the RTP payload*/
M4Err RTP_DecodePacket(RTPChannel *ch, char *pck, u32 pck_size, RTPHeader *rtp_hdr, u32 *PayloadStart);

/*decodes an RTCP packet and update timing info, send RR too*/
M4Err RTCP_DecodePacket(RTPChannel *ch, char *pck, u32 pck_size);

/*computes and send Reciever report. If the channel is a TCP channel, you must specify
the callback function. NOTE: many RTP implementation do NOT process RTCP info recieved on TCP...
the lib will decide whether the report shall be sent or not, therefore you should call
this function at regular times*/
M4Err RTCP_SendReport(RTPChannel *ch, 
						M4Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk);

/*send a BYE info (leaving the session)*/
M4Err RTCP_SendBye(RTPChannel *ch,
						M4Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk);


/*send RTP packet*/
M4Err RTP_SendPacket(RTPChannel *ch, RTPHeader *rtp_hdr, char *extra_header, u32 extra_header_size, char *pck, u32 pck_size);
/*sets log file on RTP channel*/
void RTP_SetLog(RTPChannel *ch, FILE *log);

enum
{
	RTCP_INFO_NAME = 0,
	RTCP_INFO_EMAIL,
	RTCP_INFO_PHONE,
	RTCP_INFO_LOCATION,
	RTCP_INFO_TOOL,
	RTCP_INFO_NOTE,
	RTCP_INFO_PRIV
};

/*sets RTCP info sent in RTCP reports. info_string shall NOT exceed 255 chars*/
M4Err RTCP_SetInfo(RTPChannel *ch, u32 InfoCode, char *info_string);

u32 RTP_IsUnicast(RTPChannel *ch);
u32 RTP_IsInterleaved(RTPChannel *ch);
u32 RTP_GetTimeScale(RTPChannel *ch);
u32 RTP_IsActive(RTPChannel *ch);
u8 RTP_GetLowInterleavedID(RTPChannel *ch);
u8 RTP_GetHighInterleavedID(RTPChannel *ch);
RTSPTransport *RTP_GetTransport(RTPChannel *ch);
u32 RTP_GetLocalSSRC(RTPChannel *ch);

Float RTP_GetLossPercentage(RTPChannel *ch);
u32 RTP_GetRTCPBytesSent(RTPChannel *ch);
void RTP_GetPorts(RTPChannel *ch, u16 *rtp_port, u16 *rtcp_port);

/****************************************************************************

					SDP LIBRARY EXPORTS
		
		  Note: SDP is mainly a text protocol with 
	well defined containers. The following structures are used to write / read
	SDP informations, and the library also provides consistency checking

  When reading SDP, all text items/structures are allocated by the lib, and you
  must call SDP_Reset(SDPInfo *sdp) or SDP_Delete(SDPInfo *sdp) to release the memory

  When writing the SDP from a SDPInfo, the output buffer is allocated by the library, 
  and you must release it yourself

  Some quick constructors are available for SDPConnection and SDPMedia in order to set up
  some specific parameters to their default value

  An extra function SDP_CheckInfo(SDPInfo *sdp) is provided for compliency check 
  with RFC2327: all requested fields are checked as well as conflicting information forbidden
  in RFC 2327
****************************************************************************/

/*
	Structure for bandwidth info
*/
typedef struct
{
	/*"CT", "AS" are defined. Private extensions must be "X-*" ( * "are recommended to be short")*/
	char *name;
	/*in kBitsPerSec*/
	u32 value;
} SDPBandwidth;

/*
	Structure for Time info
*/
/*we do not support more than ... time offsets / zone adjustment
if more are needed, RFC recommends to use several entries rather than a big*/
#define SDP_MAX_TIMEOFFSET	10

typedef struct
{
	/*NPT time in sec*/
	u32 StartTime;
	/*if 0, session is unbound. NPT time in sec*/
	u32 StopTime;
	/*if 0 session is not repeated. Expressed in sec.
	Session is signaled repeated every repeatInterval*/
	u32 RepeatInterval;
	/*active duration of the session in sec*/
	u32 ActiveDuration;

	/*time offsets to use with repeat. Specify a non-regular repeat time from the Start time*/
	u32 OffsetFromStart[SDP_MAX_TIMEOFFSET];
	/*Number of offsets*/
	u32 NbRepeatOffsets;

	/*EX of repeat:
	a session happens 3 times a week, on mon 1PM, thu 3PM and fri 10AM
	1- StartTime should be NPT for the session on the very first monday, StopTime
	the end of this session
	2- the repeatInterval should be 1 week, ActiveDuration the length of the session
	3- 3 offsets: 0 (for monday) (3*24+2)*3600 for thu and (4*24-3) for fri
	*/


	/*timezone adjustments, to cope with #timezones, daylight saving countries and co ...
	Ex: adjTime = [2882844526 2898848070] adjOffset=[-1h 0]
	[0]: at 2882844526 the time base by which the session's repeat times are calculated 
	is shifted back by 1 hour
	[1]: at time 2898848070 the session's original time base is restored
	*/

	/*Adjustment time at which the corresponding time offset is to be applied to the 
	session time line (time used to compute the "repeat session"). 
	All Expressed in NPT*/
	u32 AdjustmentTime[SDP_MAX_TIMEOFFSET];
	/* Offset with the session time line, ALWAYS ABSOLUTE OFFSET TO the specified StartTime*/
	s32 AdjustmentOffset[SDP_MAX_TIMEOFFSET];
	/*Number of offsets.*/
	u32 NbZoneOffsets;
} SDPTiming;


typedef struct
{
	/*only "IN" currently defined*/
	char *net_type;
	/*only "IP4" currently defined*/
	char *add_type;
	/*doted IP address*/
	char *host;
	/*TTL - MUST BE PRESENT if IP is multicast - -1 otherwise*/
	s32 TTL;
	/*multiple address counts - ONLY in media descriptions if needed. This
	is used for content scaling, when # quality of the same media are multicasted on
	# IP addresses*/
	u32 add_count;
} SDPConnection;

/*
	FMTP: description of dynamic payload types. This is opaque at the SDP level.
	Each attributes is assumed to be formatted as <param_name=param_val; ...>
	If not the case the attribute will have an empty value string and only the
	parameter name.
*/
typedef struct
{
	/*payload type of the format described*/
	u32 PayloadType;
	/*list of X_Attribute elements. The Value field may be NULL*/
	Chain *Attributes;
} SDP_FMTP;

typedef struct
{
	/*m=
	0: application - 1:video - 2: audio - 3: text - 4:data - 5: control*/
	u32 Type;
	/*Port Number - For transports based on UDP, the value should be in the range 1024 
	to 65535 inclusive. For RTP compliance it should be an even number*/
	u32 PortNumber;
	/*number of ports described. If >= 2, the next media(s) in the SDP will be configured
	to use the next tuple (for RTP). If 0 or 1, ignored
	Note: this is used for scalable media: PortNumber indicates the port of the base 
	media and NumPorts the ports||total number of the upper layers*/
	u32 NumPorts;
	/*currently ony "RTP/AVP" and "udp" defined*/
	char *Profile;

	/*list of SDPConnection's. A media can have several connection in case of scalable content*/
	Chain *Connections;

	/*RTPMaps contains a list SDPRTPMaps*/
	Chain *RTPMaps;

	/*FMTP contains a list of FMTP structures*/
	Chain *FMTP;
	
	/*for RTP this is PayloadType, but can be opaque (string) depending on the app.
	Formated as XX WW QQ FF
	When reading the SDP, the payloads defined in RTPMap are removed from this list
	When writing the SDP for RTP, you should only specify static payload types here,
	as dynamic ones are stored in RTPMaps and automatically written*/
	char *fmt_list;

	/*all attributes not defined in RFC 2327 for the media*/
	Chain *Attributes;

	/*Other SDP attributes for media desc*/

	/*k=
	method is 'clear' (key follows), 'base64' (key in base64), 'uri' (key is the URI) 
	or 'prompt' (key not included)*/
	char *k_method, *k_key;

	Chain *Bandwidths;

	/*0 if not present*/
	u32 PacketTime;
	/*0: none - 1: recv, 2: send, 3 both*/
	u32 SendRecieve;
	char *orientation, *sdplang, *lang;
	/*for video only, 0.0 if not present*/
	Float FrameRate;
	/*between 0 and 10, -1 if not present*/
	s32 Quality;
} SDPMedia;

typedef struct
{
	/*v=*/
	u32 Version;
	/*o=*/
	char *o_username, *o_session_id, *o_version, *o_address;
	/*"IN" for Net, "IP4" or "IP6" for address are currently valid*/
	char *o_net_type, *o_add_type;

	/*s=*/
	char *s_session_name;
	/*i=*/
	char *i_description;
	/*u=*/
	char *u_uri;
	/*e=*/
	char *e_email;
	/*p=*/
	char *p_phone;
	/*c= either 1 or 0 SDPConnection */
	SDPConnection *c_connection;	
	/*b=*/
	Chain *b_bandwidth;
	/*All time info (t, r, z)*/
	Chain *Timing;
	/*k=
	method is 'clear' (key follows), 'base64' (key in base64), 'uri' (key is the URI) 
	or 'prompt' (key not included)*/
	char *k_method, *k_key;
	/*all possible attributes (a=), session level*/
	char *a_cat, *a_keywds, *a_tool;
	/*0: none, 1: recv, 2: send, 3 both*/
	u32 a_SendRecieve;
	/*should be `broadcast', `meeting', `moderated', `test' or `H332'*/
	char *a_type;
	char *a_charset;
	char *a_sdplang, *a_lang;

	/*all attributes not defined in RFC 2327 for the presentation*/
	Chain *Attributes;
	
	/*list of media in the SDP*/
	Chain *media_desc;
} SDPInfo;


/*
  Memory Consideration: the destructors free all non-NULL string. You should therefore 
  be carefull while (de-)assigning the strings. The function SDP_Parse() performs a complete 
  reset of the SDPInfo
*/
/*constructor*/
SDPInfo *NewSDPInfo();
/*destructor*/
void SDP_Delete(SDPInfo *sdp);
/*reset all structures (destroys substructure too)*/
void SDP_Reset(SDPInfo *sdp);
/*Parses a memory SDP buffer*/
M4Err SDP_Parse(SDPInfo *sdp, char *sdp_text, u32 text_size);
/*check the consistency of the SDPInfo*/
M4Err SDP_CheckInfo(SDPInfo *sdp);
/*write the SDP to a new buffer and returns it. Automatically checks the SDP before calling*/
M4Err SDP_Write(SDPInfo *sdp, char **out_str_buf);


/*
	Const/dest for SDPMedia
*/
SDPMedia *SDP_NewMedia();
void SDPMedia_Delete(SDPMedia *media);

/*
	Const/dest for SDPConnection
*/
SDPConnection *SDP_NewConnection();
void SDP_DeleteConnection(SDPConnection *conn);

/*
	Const/dest for SDP FMTP
*/
SDP_FMTP *SDP_NewFMTP();
void SDP_DeleteFMTP(SDP_FMTP *fmtp);


#ifdef __cplusplus
}
#endif

#endif		/*M4_IETF_H_*/

