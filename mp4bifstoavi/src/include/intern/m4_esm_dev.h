/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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



#ifndef __M4_ESM_DEV_H_
#define __M4_ESM_DEV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <m4_terminal.h>
#include <m4_network.h>
#include <m4_descriptors.h>
#include <m4_decoder.h>
/*we need mediaControl & co, so include all MPEG-4 nodes*/
#include <m4_mpeg4_nodes.h>

/*interface between the ESM and the service plugins*/
typedef struct
{
	/*user callback data*/
	void *user_priv;
	/*message from service - error is set if error*/
	void (*on_message)(void *user_priv, LPNETSERVICE service, M4Err error, const char *message);
	/*called on service (if channel is NULL) or channel connect completed*/
	void (*on_connect)(void *user_priv, LPNETSERVICE service, LPNETCHANNEL ns, M4Err response);
	/*called on service (if channel is NULL) or channel disconnect completed*/
	void (*on_disconnect)(void *user_priv, LPNETSERVICE service, LPNETCHANNEL ns, M4Err response);
	/*
		acknowledgement of service command - service commands handle both services and channels
		Most of the time commands are NOT acknowledged, typical acknowledgement are needed for setup and control
		with remote servers
		command can also be triggered from the service (QoS, broadcast announcements)
	*/
	void (*on_command)(void *user_priv, LPNETSERVICE service, NetworkCommand *com, M4Err response);
	/*uncompressed SL header passed with data for stream sync - if not present then data shall
	be a valid SL packet (header + PDU)*/
	void (*on_slp_recieved)(void *user_priv, LPNETSERVICE service, LPNETCHANNEL ns, char *data, u32 data_size, SLHeader *hdr, M4Err reception_status);
} NetUser;

typedef struct _inline_scene InlineScene;
typedef struct _m4_client M4Client;
typedef struct _media_manager MediaManager;
typedef struct _object_clock Clock;
typedef struct _es_channel Channel;
typedef struct _generic_codec GenericCodec;
typedef struct _od_manager ODManager;
typedef struct _mediaobj MediaObject;
typedef struct _net_service NetService;


struct _net_service
{
	/*the plugin handling this service - must be declared first to typecast with FileDownlaoder service upon deletion*/
	NetClientPlugin *ifce;

	/*the terminal*/
	struct _m4_client *term;
	/*service url*/
	char *url;
	/*struct _od_manager owning service, NULL for services created for remote channels*/
	struct _od_manager *owner;
	/*number of attached remote channels ODM (ESD URLs)*/
	u32 nb_ch_users;
	/*number of attached remote ODM (OD URLs)*/
	u32 nb_odm_users;
	
	/*clock objects. Kept at service level since ESID namespace is the service one*/
	Chain *Clocks;
	/*all downloaders objects used in this service*/
	Chain *dnloads;
};


/*opens service - performs URL concatenation if parent service specified*/
LPNETSERVICE NM_NewService(M4Client *term, ODManager *owner, const char *url, LPNETSERVICE parent_service, M4Err *ret_code);
/*destroy service*/
void NM_DestroyService(LPNETSERVICE nets);
/*connect service (net ressource setup)*/
M4Err NM_OpenService(LPNETSERVICE ns);
/*closes service (net ressource shutdown) - a service may be reopened once closed*/
M4Err NM_CloseService(LPNETSERVICE ns);
/*access to the plugin interfaces - cf m4_network.h NetClientPlugin for details*/
M4Err NM_Get_MPEG4_IOD(LPNETSERVICE ns, u32 expect_type, const char *sub_url, char **raw_iod, u32 *raw_iod_size) ;
M4Err NM_ConnectChannel(LPNETSERVICE ns, LPNETCHANNEL channel, const char *url, Bool upstream);
M4Err NM_DisconnectChannel(LPNETSERVICE ns, LPNETCHANNEL channel);
M4Err NM_ServiceCommand(LPNETSERVICE ns, NetworkCommand *com);
M4Err NM_ChannelGetSLP(LPNETSERVICE ns, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, SLHeader *out_sl_hdr, Bool *is_compressed, M4Err *out_reception_status, Bool *is_new_data);
M4Err NM_ChannelReleaseSLP(LPNETSERVICE ns, LPNETCHANNEL channel);
Bool NM_CanHandleURLInService(LPNETSERVICE ns, char *url);


/*
		Inline scene stuff
*/
struct _inline_scene
{
	/*root OD of the subscene, ALWAYS namespace of the parent scene*/
	struct _od_manager *root_od;
	/*scene codec: top level decoder decoding/generating the scene - can be BIFS, VRML parser, etc*/
	struct _generic_codec *scene_codec;
	/*OD codec - specific to MPEG-4, only present at the inline level (media ressources are always scoped here)*/
	struct _generic_codec *od_codec;

	/*struct _od_managers used, namespace of this scene. The chain does not have the root_od
	it only contains OD sent through OD UPDATE in the OD stream(s) attached 
	to this scene. Remote ODs are not added, only there parents are*/
	Chain *ODlist;
	/*list of MOs (links between OD and nodes)*/
	Chain *media_objects;
	/*list of externproto libraries*/
	Chain *extern_protos;
	/*list of nodes using this inline*/
	Chain *inline_nodes;
	/*list of extra scene graphs (text streams, generic OSDs, ...)*/
	Chain *extra_scenes;
	/*inline scene graph*/
	LPSCENEGRAPH graph;
	/*graph state - if not attached, no traversing of inline*/
	Bool graph_attached;
	/*togles inline restart - needed because the restart may be triggered from inside the scene or from
	parent scene, hence 2 render passes must be used*/
	Bool needs_restart;
	/*duration of inline scene*/
	u32 duration;
	M_WorldInfo *world_info;

	Bool is_dynamic_scene;
	/*URLs of current video, audio and subs (we can't store objects since they're destroyed when seeking)*/
	SFURL visual_url, audio_url, text_url;
};

InlineScene *NewInlineScene(InlineScene *parentScene);
void IS_Delete(InlineScene *is);
struct _od_manager *IS_FindODM(InlineScene *is, u16 OD_ID);
void IS_Disconnect(InlineScene *is);
void IS_RemoveOD(InlineScene *is, ODManager *odm);
/*browse all (media) channels and send buffering info to the app*/
void IS_UpdateBufferingInfo(InlineScene *is);
/*the inline rendering function*/
void IS_Render(SFNode *pInline, void *render_stack);
void IS_AttachGraphToRender(InlineScene *is);
struct _mediaobj *IS_GetMediaObject(InlineScene *is, MFURL *url, u32 obj_type_hint);
void IS_SetupOD(InlineScene *is, ODManager *odm);
u32 URL_GetODID(MFURL *url);
/*restarts inline scene - care has to be taken not to remove the scene while it is traversed*/
void IS_Restart(InlineScene *is);
/*updates scene duration based on settings*/
void IS_SetSceneDuration(InlineScene *is);
/*returns struct _od_manager for a given extern proto scene graph*/
struct _od_manager *IS_GetProtoSceneByGraph(void *_is, LPSCENEGRAPH sg);
/*locate media object by ODID (non dynamic ODs) or URL (dynamic ODs)*/
struct _mediaobj *IS_FindObject(InlineScene *is, u16 ODID, char *url);
/*returns scene time in sec - exact meaning of time depends on standard used*/
Double IS_GetSceneTime(void *_is);
/*returns url target view (eg blabla#myview) for the given node (uses node DEF name)*/
Bool IS_IsDefaultView(SFNode *node);
/*compares object URL with another URL - ONLY USE THIS WITH DYNAMIC ODs*/
Bool Inline_IsSameURL(MFURL *obj_url, MFURL *inline_url);
/*register extra scene graph for on-screen display*/
void IS_RegisterExtraScene(InlineScene *is, LPSCENEGRAPH extra_scene, Bool do_remove);
/*forces scene size info (without changing pixel metrics) - this may be needed by plugin using extra graphs (like timedtext)*/
void IS_ForceSceneSize(InlineScene *is, u32 width, u32 height);
/*regenerate a scene graph based on available objects - can only be called for dynamic OD streams*/
void IS_RegenerateScene(InlineScene *is);
/*selects given ODM for dynamic scenes*/
void IS_SelectODM(InlineScene *is, ODManager *odm);
/*restarts dynamic scene from given time: scene graph is not reseted, objects are just restarted
instead of closed and reopened. If a media control is present on inline, from_time is overriden by MC range*/
void IS_RestartDynamicScene(InlineScene *is, u32 from_time);


struct _m4_client
{
	/*callback to user application*/	
	M4User *user;
	/*JavaScript browser API*/
	JSInterface js_ifce;
	/*media scheduler*/
	struct _media_manager *mediaman;
	/*scene renderer*/
	struct scene_renderer *renderer;
	/*top level scene*/
	InlineScene *root_scene;

	/*rendering frame rate (used for systems AU execution)*/
	Double system_fps;
	u32 half_frame_duration;

	/*callback functions for network client plugin*/
	NetUser client_sink;
	/*net services*/
	Chain *net_services;
	/*net services to be destroyed*/
	Chain *net_services_to_remove;
	/*channels waiting for service CONNECT ack to be setup*/
	Chain *channels_pending;
	/*od pending for play*/
	Chain *od_pending;
	/*network lock*/
	M4Mutex *net_mx;
	/*all input stream decoders*/
	Chain *input_streams;
	/*all X3D key/mouse/string sensors*/
	Chain *x3d_sensors;

	/*restart time for main time-line control*/
	u32 restart_time;
	
	/*options (cf config doc)*/
	Bool bifs_can_resync;
	Bool force_single_clock;
	Bool render_frames;

	u32 reload_state;
	char *reload_url;
};


/*error report function*/
void M4_OnMessage(M4Client *app, const char *service, const char *message, M4Err error);
void Term_SetupNetwork(M4Client *app);
/*creates service for given OD / URL*/
void Term_ConnectODManager(M4Client *app, ODManager *odm, char *serviceURL, LPNETSERVICE ParentService);
/*creates service for given channel / URL*/
M4Err Term_ConnectChannelURL(M4Client *app, Channel *ch, char *URL);

/*called by media manager to perform service maintenance:
servive shutdown: this is needed because service handler may be asynchronous
object Play: this is needed to properly handle multiplexed sources (all channels must be connected before play)
service restart
*/
void Term_HandleServices(M4Client *app);
/*close service and queue for delete*/
void Term_CloseService(M4Client *app, LPNETSERVICE service);
/*locks net manager*/
void Term_LockNet(M4Client *app, Bool LockIt);

/*locks scene renderer*/
void Term_LockScene(M4Client *app, Bool LockIt);
/*get scene renderer time*/
u32 Term_GetTime(M4Client *term);
/*forces scene retraversal*/
void Term_InvalidateScene(M4Client *term);

/*callbacks for scene graph library so that all related ESM nodes are properly instanciated*/
void Term_NodeInit(void *_is, SFNode *node);
void Term_NodeModified(void *_is, SFNode *node); 


/*
		Media manager
*/

enum
{
	MM_THREAD_FREE,
	MM_THREAD_SINGLE,
	MM_THREAD_MULTI,
};

struct _media_manager
{
	/*MPEG4 terminal*/
	M4Client *term;
	/*decoding thread*/
	M4Thread *th;
	/*thread priority*/
	s32 priority;
	/*thread exec flags*/
	Bool run, exit;

	/*mutex for decoder access*/
	M4Mutex *mm_mx;

	Chain *unthreaded_codecs;
	Chain *threaded_codecs;
	u32 cumulated_priority;
	u32 interrupt_cycle_ms;
	/*0: up to decoder, 1: single thread, 2: all decoders threaded*/
	u32 threading_mode;
};

MediaManager *NewMediaManager(M4Client *term, u32 threading_mode);
void MM_Delete(MediaManager *mgr);
void MM_ProcessMediaData(void *mgr, s32 timeAvailable);
void MM_AddCodec(MediaManager *mgr, GenericCodec *codec);
void MM_RemoveCodec(MediaManager *mgr, GenericCodec *codec);
void MM_StartCodec(GenericCodec *codec);
void MM_StopCodec(GenericCodec *codec);
void MM_SetThreadingMode(MediaManager *mgr, u32 mode);
void MM_SetPriority(MediaManager *mgr, s32 Priority);


/*clock*/
struct _object_clock 
{
	u16 clockID;	
	M4Client *term;
	M4Mutex *mx;
	/*no_time_ctrl : set if ANY stream running on this clock has no time control capabilities - this avoids applying
	mediaControl and others that would break stream dependencies*/
	Bool use_ocr, clock_init, has_seen_eos, no_time_ctrl;
	u32 init_time, StartTime, PauseTime, Paused;
	/*the number of streams buffering on this clock*/
	u32 Buffering;
	/*associated media control if any*/
	struct _media_control *mc;
	/*for MC only (no FlexTime)*/
	Float speed;
	u32 discontinuity_time;
	s32 drift;
};

/*destroys clock*/
void DeleteClock(Clock *ck);
/*finds a clock by ID or by ES_ID*/
Clock *CK_FindClock(Chain *Clocks, u16 clockID, u16 ES_ID);
/*attach clock returns a new clock or the clock this stream (ES_ID) depends on (OCR_ES_ID)
hasOCR indicates whether the stream being attached carries object clock references
@clocks: list of clocks in ES namespace (service)
@is: inline scene to solve clock dependencies
*/
Clock *CK_AttachClock(Chain *clocks, InlineScene *is, u16 OCR_ES_ID, u16 ES_ID, s32 hasOCR);
/*reset clock (only called by channel owning clock*/
void CK_Reset(Clock *ck);
/*return clock time in ms*/
u32 CK_GetTime(Clock *ck);
/*sets clock time - FIXME: drift updates for OCRs*/
void CK_SetTime(Clock *ck, u32 TS);
/*return clock time in ms without drift adjustment - used by audio objects only*/
u32 CK_GetRealTime(Clock *ck);
/*pause the clock*/
void CK_Pause(Clock *ck);
/*resume the clock*/
void CK_Resume(Clock *ck);
/*returns true if clock started*/
Bool CK_IsStarted(Clock *ck);
/*toggles buffering on (clock is paused at the first stream buffering) */
void CK_BufferOn(Clock *ck);
/*toggles buffering off (clock is paused at the last stream restarting) */
void CK_BufferOff(Clock *ck);
/*set clock speed scaling factor*/
void CK_SetSpeed(Clock *ck, Float speed);
/*set clock drift - used to resync audio*/
void CK_AdjustDrift(Clock *ck, s32 ms_drift);

enum
{
	/*channel is setup and waits for connection request*/
	ES_Setup = 0,
	/*waiting for server reply*/
	ES_WaitingForAck,
	/*connection OK*/
	ES_Connected,
	/*data exchange on this service/channel*/
	ES_Running,
	/*deconnection OK - a download channel can automatically disconnect when download is done*/
	ES_Disconnected,
	/*service/channel is not (no longer) available/found and should be removed*/
	ES_Unavailable
};

/*data channel (elementary stream)*/
struct _es_channel 
{
	/*security check on channel*/
	u32 chan_id;
	/*service this channel belongs to*/
	LPNETSERVICE service;
	/*stream descriptor*/
	ESDescriptor *esd;
	/*parent OD for this stream*/
	struct _od_manager *odm;
	u32 es_state;
	Bool is_pulling;
	u32 media_padding_bytes;
	/*IO mutex*/
	M4Mutex *mx;
	u32 AU_Count;
	/*decoding buffers for push mode*/
	struct _decoding_buffer * AU_buffer_first, * AU_buffer_last;
	/*static decoding buffer for pull mode*/
	struct _decoding_buffer * AU_buffer_pull;
	/*channel buffer flag*/
	Bool BufferOn;
	/*min level to trigger buffering on, max to trigger it off. */
	u32 MinBuffer, MaxBuffer;
	/*amount of buffered media - this is the DTS of the last recieved AU minus the onject clock time, to make sure
	we always have MaxBuffer ms ready for composition when resuming the clock*/
	s32 BufferTime;
	/*last recieved AU time - if exceeding a certain time and buffering is on, buffering is turned off.
	This is needed for streams with very short duration (less than buffer time) and stream with only one AU (BIFS/OD)*/
	u32 last_au_time;
	/*Current reassemnbling buffer - currently packets are NOT reordered, only AUs are*/
	char *buffer;
	u32 len, allocSize;
	/*only for last packet of an AU*/
	u8 padingBits;
	Bool IsEndOfStream;
	/*	SL reassembler	*/
	/*current AU TSs*/
	u32 DTS, CTS;
	/*AU and Packet seq num info*/
	u32 au_sn, pck_sn;
	u32 max_au_sn, max_pck_sn;
	/*the AU length indicated in the SL Header. */
	u32 AULength;
	/*RAP indicator. If set, skip till a RAP is received*/
	Bool NeedRap;
	/*the AU in reception is RAP*/
	Bool IsRap;
	/*signal that next AU is an AU start*/
	Bool NextIsAUStart;
	/*if codec resilient, packet drops are not considered as fatal for AU reconstruction (eg no wait for RAP)*/
	Bool codec_resilient;
	/*when starting a channel, the first AU is ALWAYS fetched when buffering - this forces
	BIFS and OD to be decoded and first frame render, in order to detect media objects that would also need
	buffering - note this doesn't affect the clock, it is still paused if buffering*/
	Bool first_au_fetched;
	/* TimeStamp to Media Time mapping*/
	/*TS (in TSResolution) corresponding to the SeedTime of the decoder. Delivered by net, otherwise 0*/
	u64 seed_ts;
	/*media time offset corresponding to SeedTS. This is needed when the channel doesn't own the clock*/
	u32 ts_offset;
	/*scaling factors to remap to timestamps in milliseconds*/
	Double ts_scale, ocr_scale;
	/*clock driving this stream - currently only CTS is supported (no OCR)*/
	struct _object_clock *clock;
	/*flag for clock init. Only a channel owning the clock will set this flag on clock init*/
	Bool IsClockInit;

	/*duration of last recieved AU if any, 0 if not known (most of the time)*/
	u32 au_duration;
	Bool skip_sl;
};

/*creates a new channel for this stream*/
Channel *NewChannel(ESDescriptor *esd);
/*destroys channel*/
void DeleteChannel(Channel *ch);
/*(un)locks channel*/
void Channel_Lock(Channel *ch, u32 LockIt);
/*setup channel for reception of data*/
M4Err Channel_Start(Channel *ch);
/*stop channel from receiving data*/
M4Err Channel_Stop(Channel *ch);
/*handles reception of an SL PDU*/
void Channel_RecieveSLP(LPNETSERVICE serv, Channel *ch, char *StreamBuf, u32 StreamLength, SLHeader *header, M4Err reception_status);
/*signals end of stream on the channel*/
void Channel_EndOfStream(Channel *ch);
/*if a plugin dispatches raw data*/
void Channel_DistpatchRawData(Channel *ch, char *data, u32 dataLen);
/*fetches first AU available for decoding on this channel*/
struct _decoding_buffer *Channel_GetAU(Channel *ch);
/*drops first AU on this channel*/
void Channel_DropAU(Channel *ch);
/*performs final setup upon connection confirm*/
void Channel_OnConnect(Channel *ch);
/*reconfigure SL for this channel*/
void Channel_ReconfigSL(Channel *ch, SLConfigDescriptor *slc);
/*hack for streaming: whenever a time map (media time <-> TS time) event is recieved on the channel reset decoding buffer
this is needed because all server tested resend packets on already running channel*/
void Channel_TimeMapped(Channel *ch, Bool reset);
/*dummy channels are used by scene decoders which don't use ESM but load directly the scene graph themselves
these channels are ALWAYS pulling ones, and this function will init the channel clock if needed*/
void Channel_InitDummy(Channel *ch);



/*
		decoder stuff
*/
enum
{
	/*stop: the decoder is not playing*/
	CODEC_STOP	=	0,
	/*stop: the decoder is playing*/
	CODEC_PLAY	=	1,
	/*End Of Stream: when the base layer signals it's done, this triggers media-specific
	handling of the CB. 
	For video, the output is kept alive, For audio, the output is reseted (don't want audio loop ;)*/
	CODEC_EOS	=	2,
	/*pause: the decoder is stoped but the CB is kept intact
	THIS IS NOT USED AS A CODEC STATUS, but only for signaling that the CB shouldn't 
	be reseted - the real status of a "paused" decoder is STOP*/
	CODEC_PAUSE	=	3,
	/*Buffer: transition state: the decoder runs (fetch data/decode) but the clock
	is not running (no composition). This is used for rebuffering channels (rtp...)*/
	CODEC_BUFFER =	4
};

enum
{
	COD_HAS_UPSTREAM = 1,
	/*the codec uses the interface from another codec (only used by private scene streams to handle
	any intern sprite/animation streams)*/
	COD_IS_USE = 2,
	/*set for OD codec when static (eg all ressources are static and scene graph is generated on the fly)*/
	COD_IS_STATIC_OD = 4,
};

struct _generic_codec 
{
	/*codec type (streamType from base layer)*/
	u32 type;
	u32 flags;
	/*current decoder interface */
	BaseDecoder *decio;
	/*composition memory for media streams*/
	struct _composition_memory *CB;
	/*input media channles*/
	Chain *inChannels;
	/*a pointer to the OD that owns the decoder.*/
	struct _od_manager *odm;
	u32 Status;
	Bool Muted;
	struct _object_clock *ck;
	/*priority of this media object. This is ALWAYS the base layer priority
	PriorityBoost is set when the CB is under critical limit (for now only audio uses the feature)
	and results in a bigger time slice for the codec. Only on/off value for now*/
	u32 Priority, PriorityBoost;
	/*last processed DTS - sanity check for scalability*/
	u32 last_unit_dts;
	/*last processed CTS on base layer - seeking detection*/
	u32 last_unit_cts;
	/*in case the codec performs temporal re-ordering itself*/
	Bool is_reordering;
	u32 prev_au_size;
	u32 bytes_per_sec;
	Float fps;

	/*statistics*/
	u32 last_stat_start, cur_bit_size;
	u32 avg_bit_rate, max_bit_rate;
	u32 total_dec_time, nb_dec_frames, max_dec_time;

	/*for CTS reconstruction (channels not using SL): we cannot just update timing at each frame, not precise enough 
	since we use ms and not microsec TSs*/
	u32 cur_audio_bytes, cur_video_frames;
};

GenericCodec *NewDecoder(ODManager *odm, ESDescriptor *base_layer, s32 PL, M4Err *e);
void DeleteCodec(GenericCodec *codec);
M4Err Codec_AddChannel(GenericCodec *codec, Channel *ch);
/*returns TRUE if stream was present, false otherwise*/
Bool Codec_RemoveChannel(GenericCodec *codec, Channel *ch);
M4Err Decoder_ProcessData(GenericCodec *codec, u32 TimeAvailable);
M4Err Codec_GetCap(GenericCodec *codec, CapObject *cap);
M4Err Codec_SetCap(GenericCodec *codec, CapObject cap);
void Codec_SetStatus(GenericCodec *codec, u32 Status);
/*returns a new codec using an existing loaded decoder - only used by private scene to handle timelines*/
GenericCodec *Codec_UseDecoder(GenericCodec *codec, ODManager *odm);

/*OD manager*/


/*all inserted ODs have this ODID*/
#define DYNAMIC_OD_ID	1050

struct _od_manager
{
	ObjectDescriptor *OD;
	/*remote od manager*/
	struct _od_manager *remote_OD;
	/*parent od manager for remote od*/
	struct _od_manager *parent_OD;
	/*the service used by this ODM. If the service private data is this ODM, then the service was created for this ODM*/
	LPNETSERVICE net_service;
	/*channels associated with this object (media channels, OCR, IPMP, OCI, etc)*/
	Chain *channels;
	/*sub scene for inline or NULL */
	struct _inline_scene *subscene;
	/*parent scene or NULL for root scene*/
	struct _inline_scene *parentscene;
	/*pointer to terminal*/
	struct _m4_client *term;
	/*object codec (media or BIFS for AnimationStream) attached if any*/
	struct _generic_codec *codec;
	/*OCI codec attached if any*/
	struct _generic_codec *oci_codec;
	/*OCR codec attached if any*/
	struct _generic_codec *ocr_codec;
	/*PLs*/
	s32 Audio_PL, Graphics_PL, OD_PL, Scene_PL, Visual_PL;
	Bool ProfileInlining;
	/*interface with scene rendering*/
	struct _mediaobj *mo;
	/*number of channels with connection not yet acknowledge*/
	u32 pending_channels;
	Bool is_open;
	/*timing as evaluated by the CB*/
	u32 current_time;
	/*full object duration 0 if unknown*/
	u32 duration;
	/*playback end in media time (eg, duration OR end_range if MediaControl)*/
	u32 range_end;
	/*the one and only media control currently attached to this object*/
	struct _media_control *media_ctrl;
	/*the list of media control controling the object*/
	Chain *mc_stack;
	/*the media sensor(s) attached to this object*/
	Chain *ms_stack;
	/*this flag is set if ANY stream of this OD has no time control capabilities*/
	Bool no_time_ctrl;
};

ODManager *NewODManager();
void ODM_Delete(ODManager *ODMan);
/*setup service entry point*/
void ODM_SetupService(ODManager *odm, const char *sub_url);
/*setup OD*/
void ODM_SetupOD(ODManager *odm, LPNETSERVICE parent_serv);
/*removes OD*/
void ODM_RemoveOD(ODManager *odman);
/*setup an ESD*/
M4Err ODM_SetupStream(ODManager *odm, ESDescriptor *esd, LPNETSERVICE service);
/*removes an ESD (this destroys associated channel if any)*/
void ODM_RemoveStream(ODManager *odm, u16 ES_ID);
/*set stream duration - updates object duration accordingly*/
void ODM_SetDuration(ODManager *odm, Channel *, u32 stream_duration);
/*signals end of stream on channels*/
void ODM_EndOfStream(ODManager *odm, Channel *);
/*start OD streams*/
void ODM_Start(ODManager *odm);
/*stop OD streams*/
void ODM_Stop(ODManager *odm, Bool force_close);
/*send PLAY request to network - needed to properly handle multiplexed inputs 
ONLY called by service handler (media manager thread)*/
void ODM_Play(ODManager *odm);

/*returns 1 if this is a segment switch, 0 otherwise - takes care of object restart if segment switch*/
Bool ODM_CheckSegmentSwitch(ODManager *odm);
/*pause object (mediaControl use only)*/
void ODM_Pause(ODManager *odm);
/*resume object (mediaControl use only)*/
void ODM_Resume(ODManager *odm);
/*set object speed*/
void ODM_SetSpeed(ODManager *odm, Float speed);
/*returns the clock of the media stream (video, audio or bifs), NULL otherwise */
struct _object_clock *ODM_GetMediaClock(ODManager *odm);
/*adds segment descriptors targeted by the URL to the list and sort them - the input list must be empty*/
void ODM_InitSegmentDescriptors(ODManager *odm, Chain *list, MFURL *url);
/*returns true if this OD depends on the given clock*/
Bool ODM_SharesClock(ODManager *odm, struct _object_clock *ock);
/*refresh all ODs when an non-interactive stream is found*/
void ODM_RefreshNonInteractives(ODManager *odm);

/*used for delayed channel setup*/
typedef struct 
{
	struct _generic_codec *dec;
	struct _es_channel *ch;	
} ChannelSetup;

/*post-poned channel connect*/
M4Err ODM_SetupChannel(struct _es_channel *ch, struct _generic_codec *dec, M4Err err);


/*
		Media Object

  opaque handler for all natural media objects (audio, video, image) so that scene renderer and systems engine
are ot tied up. 
	NOTE: the media object location relies on the node parent graph (this is to deal with namespaces in OD framework)
therefore it is the task of the media management app to setup clear links between the scene graph and its ressources
(but this is not mandatory, cf URLs in VRML )

	TODO - add interface for shape coding positioning in mediaObject and in the decoder API
*/

enum
{
	/*this is set to 0 by the OD manager whenever a change occur in the media (w/h change, SR change, etc) 
	as a hint for the renderer*/
	MO_IS_INIT = (1<<1),
	/*this is used for 3D/GL rendering to indicate an image has been vertically flipped*/
	MO_IS_FLIP = (1<<2)
};

struct _mediaobj
{
	/*type is as defined in m4_network*/
	u32 type;
	/*one of the above flags*/
	u32 mo_flags;
	/*audio props*/
	u32 sample_rate;
	u32 bits_per_sample;
	u32 num_channels;
	/*cf m4_decoder.h for channel cfg*/
	u32 channel_config;

	/*video props*/
	Float FPS;	/*this is not known by default, but computed on the fly*/
	u32 width, height, stride;
	u32 pixelFormat;
	/*changing at each frame for shape coding, or always 0 for frame coding, indicates 
	X and Y coordinate of object in frame(width, height)*/
	u32 offset_x, offset_y;
	/*shared object info: if 0 a new frame will be checked, otherwise current is returned*/
	u32 num_fetched;
	/*frame presentation time*/
	u32 current_ts;
	/*data frame size*/
	u32 current_size;
	/*pointer to data frame */
	unsigned char *current_frame;

	/* private to ESM*/
	M4Client *term;
	/*media object manager - private to the sync engine*/
	ODManager *odm;
	/*OD ID of the object*/
	u32 OD_ID;
	/*OD URL for object not using MPEG4 OD urls*/
	MFURL URLs;
	/*session join*/
	u32 num_open;
	/*shared object restart handling*/
	u32 num_to_restart, num_restart;
	Float speed;
};
MediaObject *NewMediaObject(M4Client *term);

/*locate media object related to the given node - url designes the object to find - returns NULL if
URL cannot be handled - note that until the mediaObject.isInit member is true, the media object is not valid
(and could actually never be) */
MediaObject *MO_FindObject(SFNode *node, MFURL *url);
/*opens media object*/
void MO_Play(MediaObject *mo);
/*stops media object - video memory is not reset, last frame is kept*/
void MO_Stop(MediaObject *mo);
/*restarts media object - shall be used for all looping media instead of stop/play for mediaControl
to restart appropriated objects*/
void MO_Restart(MediaObject *mo);

/*
	Note on mediaControl: mediaControl is the media management app responsability, therefore
is hidden from the rendering app. Since MediaControl overrides default settings of the node (speed and loop)
you must use the MO_GetSpeed and MO_GetLoop in order to know whether the related field applies or not
*/

/*set speed of media - speed is not always applied, depending on media control settings.
NOTE: audio pitching is the responsability of the rendering app*/
void MO_SetSpeed(MediaObject *mo, Float speed);
/*returns current speed of media - in_speed is the speed of the media as set in the node (MovieTexture, 
AudioClip and AudioSource) - the return value is the real speed of the media as overloaded by mediaControl if any*/
Float MO_GetSpeed(MediaObject *mo, Float in_speed);
/*returns looping flag of media - in_loop is the looping flag of the media as set in the node (MovieTexture, 
AudioClip) - the return value is the real loop flag of the media as overloaded by mediaControl if any*/
Bool MO_GetLoop(MediaObject *mo, Bool in_loop);
/*returns whether the object should be deactivated (stop) or not - this checks object status as well as 
mediaControl status */
Bool MO_ShouldDeactivate(MediaObject *mo);
/*returns media object duration*/
Double MO_GetDuration(MediaObject *mo);
/*checks whether the target object is changed - you MUST use this in order to detect url changes*/
Bool MO_UrlChanged(MediaObject *mo, MFURL *url);
/*fetch media data - returns 1 if new frame (in which case media info of the MediaObject structure are updated)
0 otherwise - eos is set if stream is finished. if resync is specified and no frame is locked the video memory is
resync'ed to its object clock (eg frame droping) 
*/
Bool MO_FetchFrame(MediaObject *mo, Bool resync, Bool *eos);
/*release given amount of media data - nb_bytes is used for audio - if forceDrop is set, the unlocked frame will be 
droped if all bytes are consumed, otherwise it will be droped based on object time - typically, video fetches with the resync
flag set and release without forceDrop, while audio fetches without resync but forces buffer drop*/
void MO_ReleaseFrame(MediaObject *mo, u32 nb_bytes, Bool forceDrop);
/*get media time*/
void MO_GetMediaTime(MediaObject *mo, u32 *media_time, u32 *media_dur);
/*get object clock*/
void MO_GetObjectTime(MediaObject *mo, u32 *obj_time);
/*returns mute flag of media - if muted the media shouldn't be displayed*/
Bool MO_IsMuted(MediaObject *mo);
/*returns end of stream state*/
Bool MO_IsFinished(MediaObject *mo);
/*resyncs clock - only audio objects are allowed to use this*/
void MO_AdjustClock(MediaObject *mo, s32 ms_drift);





/*
	special entry point: specify directly a plugin interface for service input
*/
void M4T_OpenService(MPEG4CLIENT term, NetClientPlugin *service_hdl);


#ifdef __cplusplus
}
#endif


#endif


