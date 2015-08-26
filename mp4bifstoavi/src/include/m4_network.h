/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Network Interfaces
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

#ifndef M4_NETWORK_H
#define M4_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_descriptors.h>

/*handle to service*/
typedef struct _net_service *LPNETSERVICE;

/*handle to channel*/
typedef void *LPNETCHANNEL;

enum
{
	/*channel control, app->plugin. Note that most plugins don't need to handle pause/resume/set_speed*/
	CHAN_PLAY,
	CHAN_STOP,
	CHAN_PAUSE,
	CHAN_RESUME,
	CHAN_SET_SPEED,
	/*channel configuration, app->plugin*/
	CHAN_CONFIG,
	/*channel duration, app<->plugin (in case duration is not known at setup)*/
	CHAN_DURATION,
	/*channel buffer, app->plugin*/
	CHAN_BUFFER,
	/*channel buffer query, app<-plugin*/
	CHAN_BUFFER_QUERY,
	/*retrieves DSI from channel (DSI may be caried by net with a != value than OD), app->plugin*/
	CHAN_GET_DSI,
	/*set media padding for all AUs fetched (pull mode only). 
	If not supported the channel will have to run in push mode. app->plugin*/
	CHAN_SET_PADDING,
	/*sets input channel to pull mode if possible, app->plugin*/
	CHAN_SET_PULL,
	/*query channel capability to pause/resume and seek(play from an arbitrary range)
	a non-interactive channel doesn't have to handle SET_SPEED, PAUSE and RESUME commands but can 
	still work in pull mode*/
	CHAN_INTERACTIVE,
	/*map net time (OTB) to media time (up only) - this is needed by some signaling protocols when the 
	real play range is not the requested one */
	CHAN_MAP_TIME,
	/*reconfiguration of channel comming from network (up only) - this is used to override the SL config
	if it differs from the one specified at config*/
	CHAN_RECONFIG,
	
	/*retrieves network stats for service/channel; app->plugin*/
	NM_NETSTAT,

	/*no service commands define yet*/
	NM_COM_UNKNOWN
};

/*channel command for all commands that don't need params:
CHAN_SET_PULL: plugin shall return M4OK or M4NotSupported
CHAN_INTERACTIVE: plugin shall return M4OK or M4NotSupported
*/
typedef struct
{
	/*command type*/
	u32 command_type;
	/*channel*/
	LPNETCHANNEL on_channel;
} ChannelCom;

/*CHAN_PLAY, CHAN_SET_SPEED*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*params for CHAN_PLAY, ranges in sec - if range is <0, then it is ignored (eg [2, -1] with speed>0 means 2 +oo) */
	Double start_range, end_range;
	/*params for CHAN_PLAY and CHAN_SPEED*/
	Double speed;
} ChanPlayCom;


/*CHAN_CONFIG, CHAN_RECONFIG
channel config may happen as soon as the channel is open, even if the plugin hasn't acknowledge creation
channel config can also be used from network to app, with CHAN_RECONFIG type - only the SL config is then used
*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;

	/*SL config of the stream as delivered in OD (app->channel) or by network (channel->app)*/
	SLConfigDescriptor sl_config;		
	/*stream priority packet drops are more tolerable if low priority - app->channel only*/
	u32 priority;
	/*sync ID: all channels with the same sync ID run on the same timeline, thus the plugin should 
	try to match this - note this may not be possible (typically RTP/RTSP)*/
	u32 sync_id;
	/*audio frame duration and sample rate if any - this is needed by some RTP payload*/
	u32 frame_duration, sample_rate;
} ChanConfigCom;

/*CHAN_BUFFER, CHAN_BUFFER_QUERY*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*the recommended buffering limits in ms - this depends on the plugins preferences and on the service 
	type (multicast, vod, ...) - below buffer_min the stream will pause if possible until buffer_max is reached
	note the app will fill in default values before querying*/
	u32 min, max;
	/*only used with CHAN_BUFFER_QUERY - amount of media in decoding buffer, in ms*/
	u32 occupancy;
} ChanBufferCom;

/*CHAN_DURATION*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*duration in sec*/
	Double duration;
} ChanDurationCom;

/*CHAN_GET_DSI*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*carries specific info for codec - data shall be allocated by service and is freed by user*/
	char *dsi;
	u32 dsi_len;
} ChanGetDSICom;

/*CHAN_SET_PADDING*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	u32 padding_bytes;
} ChanPaddingCom;

/*CHAN_MAP_TIME*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*MediaTime at this timestamp*/
	Double media_time;
	/*TS where mapping is done (in SL TS resolution)*/
	u64 timestamp;
	/*specifies whether decoder input data shall be discarded or only have its timing updated*/
	Bool reset_buffers;
} ChanMapTimeCom;

/*CHAN_NETSTAT
Notes
1: only channels using network must reply. All channels fetching data through a 
file downloader (cf below) shall NOT answer, the app manages downloader bandwidth internally.
2: BANDWIDTH USED BY SIGNALING PROTOCOL IS IGNORED IN GPAC
*/
typedef struct __netcom
{
	u32 command_type;
	/*MAY BE NULL, in which case the plugin must fill in ONLY the control channel part. This
	is not used yet, but could be with a protocol using a single control socket for N media channels.*/
	LPNETCHANNEL on_channel;
	/*percentage of packet loss from network. This cannot be figured out by the app since there is no
	one-to-one mapping between the protocol packets and the final SL packet (cf RTP payloads)*/
	Float pck_loss_percentage;
	/*channel port, control channel port if any (eg RTCP)*/
	u16 port, ctrl_port;
	/*bandwidth used by channel & its control channel if any (both up and down) - expressed in bits per second*/
	u32 bw_up, bw_down, ctrl_bw_down, ctrl_bw_up;
	/*set to 0 if channel is not part of a multiplex. Otherwise set to the multiplex port, and 
	above port info shall be identifiers in the multiplex - note that multiplexing overhead is ignored 
	in GPAC for the current time*/
	u16 multiplex_port;
} NetStatCom;

typedef union __netcommand
{
	u32 command_type;
	ChannelCom base;
	ChanPlayCom play;
	ChanConfigCom cfg;
	ChanBufferCom buffer;
	ChanDurationCom duration;
	ChanGetDSICom get_dsi;
	ChanPaddingCom pad;
	ChanMapTimeCom map_time;
	NetStatCom net_stats;
} NetworkCommand;

/*MPEG-4 OD types. This type provides a hint to network plugins which may have to generate an OD/IOD on the fly
they occur only if objects/services used in the scene are not referenced through OD IDs but direct URL*/
enum
{
	/*OD expected is of undefined type*/
	NM_OD_UNDEF = 0,
	/*OD expected is of SCENE type (eg, BIFS and OD if needed)*/
	NM_OD_SCENE,
	/*OD expected is of BIFS type (anim streams)*/
	NM_OD_BIFS,
	/*OD expected is of VISUAL type*/
	NM_OD_VIDEO,
	/*OD expected is of AUDIO type*/
	NM_OD_AUDIO,
	/*OD expected is of TEXT type (3GPP/MPEG4)*/
	NM_OD_TEXT,
	/*OD expected is of InputSensor type*/
	NM_OD_INTERACT,
};

/*
	network plugins
*/

/*MPEG-4 streaming client interface name*/
#define M4STREAMINGCLIENT			FOUR_CHAR_INT('M', 'S', 'T', 'R')

typedef struct _netinterface
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*retuns 1 if plugin can process this URL, 0 otherwise. This is only called when the file extension/mimeType cannot be
	retrieved in the cfg file, otherwise the mime type/file ext is used to load service. Typically a plugin would 
	register its mime types in this function (cf NM_RegisterMimeType below)
	*/
	Bool (*CanHandleURL)(struct _netinterface *, const char *url);

	/*connects the service to the desired URL - the service handle is used for callbacks. 
	Only one service can be connected to a loaded interface.
	*/
	M4Err (*ConnectService) (struct _netinterface *, LPNETSERVICE serv, const char *url);

	/*disconnects service - the plugin is no longer used after this call - if immediate_shutdown is set the plugin
	shall not attempt to get confirmation from remote side, it will be deleted right away
	
	NOTE: depending on how the client/server exchange is happening, it may happen that the CloseService is called
	in the same context as a reply from your plugin. This can result into deadlocks if you're using threads. 
	You should therefore only try to destroy threads used in the interface shutdown process, which is guarantee
	to be in a different context call.
	*/
	M4Err (*CloseService) (struct _netinterface *, Bool immediate_shutdown);

	/*retrieves MPEG4 iod for accessing this service through an IOD - data is allocated and returned to the caller if OK
	the IOD shall refer to the service attached to the plugin
	@expect_type is a hint in case the service regenerates an IOD. It indicates whether the entry point expected is INLINE, 
	BIFS animation stream, video, audio or input sensor. cf above for types
	@sub_url: indicates fetching of an IOD for a given object in the service.
		Only used for services handling the optional CanHandleURLInService below
		NULL for main service
		service extension for sub-service (cf CanHandleURLInService below). For ex, "rtsp://myserver/file.mp4/ES_ID=3" and "rtsp://myserver/file.mp4/ES_ID=4" 
		or "file.avi#audio" and "file.avi#video".In this case a partial IOD for the desired object is expected
	Note: once a service is acknowledged as connected, the IOD shall be available synchronously
	*/
	M4Err (*Get_MPEG4_IOD) (struct _netinterface *, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size);


	/*sends command to the service / channel - cf command structure*/
	M4Err (*ServiceCommand) (struct _netinterface *, NetworkCommand *com);

	/*data channel setup - url is either
	"ES_ID=ID" where ID is the stream ID in this service
	or a control string depending on the service/stream. The URL is first used to load a plugin able to handle it, 
	so the plugin has no redirection to handle
	*/
	M4Err (*ConnectChannel) (struct _netinterface *, LPNETCHANNEL channel, const char *url, Bool upstream);
	/*teardown of data channel*/
	M4Err (*DisconnectChannel) (struct _netinterface *, LPNETCHANNEL channel);

	/*optional - fetch MPEG4 data from channel - data shall not be duplicated and must be released at ReleaseData
	SL info shall be written to provided header - if the data is a real SL packet the flag sl_compressed shall be 
	set to signal the app this is a full SL pdu (@out_sl_hdr is then ignored)
	set to NULL if not supported
	*/
	M4Err (*ChannelGetSLP) (struct _netinterface *, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, struct tagSLHeader *out_sl_hdr, Bool *sl_compressed, M4Err *out_reception_status, Bool *is_new_data);

	/*optional - release SLP data allocated on channel by the previous call, if any
	set to NULL if not supported*/
	M4Err (*ChannelReleaseSLP) (struct _netinterface *, LPNETCHANNEL channel);

	/*this is needed for plugins to query other plugins, the typical case being 2 ESD URLs pointing to the 
	same media (audio and video streams in an RTSP session). This is always used on loaded plugins but 
	doesn't have to be declared*/
	Bool (*CanHandleURLInService)(struct _netinterface *, const char *url);

/*private*/
	void *priv;
} NetClientPlugin;

/*callback functions - these can be linked with non-GPL plugins*/
/*message from service - error is set if error*/
void NM_OnMessage(LPNETSERVICE service, M4Err error, const char *message);
/*to call on service (if channel is NULL) or channel connect completed*/
void NM_OnConnect(LPNETSERVICE service, LPNETCHANNEL ns, M4Err response);
/*to call on service (if channel is NULL) or channel disconnect completed*/
void NM_OnDisconnect(LPNETSERVICE service, LPNETCHANNEL ns, M4Err response);
/* acknowledgement of service command - service commands handle both services and channels
Most of the time commands are NOT acknowledged, typical acknowledgement are needed for setup and control
with remote servers. 
command can also be triggered from the service (QoS, broadcast announcements)
cf above for command usage
*/
void NM_OnCommand(LPNETSERVICE service, NetworkCommand *com, M4Err response);
/*to call when data packet is recieved. 
@data, data_size: data recieved
@hdr: uncompressed SL header passed with data for stream sync - if not present then data shall be a valid SL packet 
	(header + PDU). Note that using an SLConfig resulting in an empty SLHeader allows sending raw data directly
@reception_status: data reception status. To signal end of stream, set this to M4EOF
*/
void NM_OnSLPRecieved(LPNETSERVICE service, LPNETCHANNEL ns, char *data, u32 data_size, struct tagSLHeader *hdr, M4Err reception_status);
/*returns URL associated with service (so that you don't need to store it)*/
const char *NM_GetServiceURL(LPNETSERVICE service);

/*check if @fileExt extension is supported for given mimeType, and if associated with plugin. If mimeType not registered, register it for given plugin*/
Bool NM_CheckExtension(NetClientPlugin *ifce, const char *mimeType, const char *extList, const char *description, const char *fileExt);
/*register mime types & file extensions - most plugins should only need the check version above*/
void NM_RegisterMimeType(NetClientPlugin *ifce, const char *mimeType, const char *extList, const char *description);


/*downloader status*/
enum
{
	/*setup and waits for connection request*/
	DL_Setup = 0,
	/*waiting for server reply*/
	DL_WaitingForAck,
	/*connection OK*/
	DL_Connected,
	/*data exchange on this downloader*/
	DL_Running,
	/*deconnection OK */
	DL_Disconnected,
	/*dowlnloader session failed or destroyed*/
	DL_Unavailable
};

/*file downloading - can and MUST be used by any plugin (regardless of license) in order not to interfere with net management*/
typedef struct _filedownloader *LPFILEDOWNLOADER;

M4Err NM_FetchFile(LPNETSERVICE service, const char *url, Bool disable_cache, void (*OnState)(void *cbk), void (*OnData)(void *cbk, char *data, u32 data_size), void *cbk, LPFILEDOWNLOADER *out_dnload);
const char *NM_GetCacheFileName(LPFILEDOWNLOADER dnload);
M4Err NM_GetDownloaderStats(LPFILEDOWNLOADER dnload, u32 *total_size, u32 *bytes_done, Float *bytes_per_sec, u32 *net_status);
M4Err NM_FetchData(LPFILEDOWNLOADER dnload, char *buffer, u32 buffer_size, u32 *read_size);
void NM_CloseDownloader(LPFILEDOWNLOADER dnload);




/*file downloader interface name - this shall not be used by plugins, use above functions instead*/
#define M4FILEDOWNLOADER			FOUR_CHAR_INT('M', 'D', 'N', 'L')

typedef struct _netdownloader
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*returns 1 if URL can be handled*/
	Bool (*CanHandleURL)(const char *url);

	/*returns mime type or NULL if URL not available - mime type string shall be freed by caller*/
	char *(*GetMimeType)(struct _netdownloader *, const char *url);

	/*connect to given URL and start fetching data. If disable_cache, data is NEVER cached*/
	M4Err (*Connect) (struct _netdownloader *, const char *url, Bool disable_cache);
	/*closes service*/
	M4Err (*Close) (struct _netdownloader *);
	/*returns local cache file name*/
	const char *(*GetCacheFileName) (struct _netdownloader *);
	
	/*if callback function is null, the downloader is not threaded and the owner can use this to fetch data
	returns error if any, or M4NetworkEmpty if empty or M4OK/M4EOS. Cache must work in the same way 
	regardless of the threading mode
	retuns M4BadParam if downloader is using a cache (the user shall use the cache through regular IO fct)*/
	M4Err (*FetchData)(struct _netdownloader *, char *buffer, u32 buffer_size, u32 *read_size);

	/*stat info updated by plugin*/
	u32 total_size;
	u32 bytes_done;
	Float bytes_per_sec;
	/*current net status*/
	u32 net_status;
	/*error report - M4EOF indicates done*/
	M4Err error;

	/*callback assigned by loader*/
	void *user_cbck;

	/*callback function for data delivery - assigned by loader;, may be NULL*/
	void (*OnData)(struct _netdownloader *, char *data, u32 data_size);

	/*Whenever a plugin needs to communicate a state change, it shall call this with data_size = 0 and update its error/net_status*/
	void (*OnState)(struct _netdownloader *);

	/*plugin private*/
	void *priv;
} NetDownloader;


#ifdef __cplusplus
}
#endif

#endif
