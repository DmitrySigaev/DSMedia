/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#ifndef __M4_AUTHORING_H_
#define __M4_AUTHORING_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/m4_isomedia.h>
#include <gpac/m4_ietf.h>
#include <gpac/m4_scenegraph_vrml.h>


/*setup interleaving for storage*/
M4Err MP4T_MakeInterleaved(M4File *mp4file, Float TimeInSec);
/*save file as fragmented movie*/
M4Err MP4T_FragmentMovie(M4File *input, char *output_file, Float MaxFragmentDuration);


/*
	Group and Interleaving priorities

  Group: when interleaving (download or hinted file), file is written in order as follows:
  * Meta-Data

  * lowest groupID tracks data
  * ...
  * highest groupID tracks data
 
  Interleaving priorities: this is only used for hinted files and allows placing
  the hint track right before its media track if needed (current assumption is that
  there is one media track per hint track). The hinter automatically adjust priorities
  for prior A given, the result is:
	  hintTrack = priorA*2
	  mediaTrack = priorA*2 + 1
*/

/*group ID for system tracks in ISMA mode*/
#define ISMA_SYS_GROUP_ID	1
/*group ID for media tracks in ISMA mode*/
#define ISMA_MEDIA_GROUP_ID	2

/*
	offset for group ID for hint tracks in SimpleAV mode when all media data
	is copied to the hint track (no use interleaving hint and original in this case)
	this offset is applied internally by the track hinter. Thus you shouldn't
	specify a GroupID >= OFFSET_HINT_GROUP_ID if you want the lib to perform efficient
	interleaving in any cases (referenced or copied media)
*/
#define OFFSET_HINT_GROUP_ID	0x8000

/*track priorities in group. This is used whether hinted data is copied to 
the hint track or referenced from the media track*/
#define MEDIA_AUDIO_PRIO	1
#define MEDIA_VIDEO_PRIO	2


/*RTP<->SL mapping*/
typedef struct 
{
	/*1 - required options*/

	/*mode, or "" if no mode ("generic" should be used instead)*/
	char mode[30];
	
	/*config of the stream if carried in SDP*/
	char *config;
	u32 configSize;
	/* Stream Type*/
	u8 StreamType;
	/* stream profile and level indication*/
	u8 PL_ID;


	/*2 - optional options*/
	
	/*size of AUs if constant*/
	u32 ConstantSize;
	/*duration of AUs if constant, in RTP timescale*/
	u32 ConstantDuration;

	/* Object Type Indication */
	u8 ObjectTypeIndication;
	/*audio max displacement when interleaving (eg, de-interleaving window buffer max length) in RTP timescale*/
	u32 maxDisplacement;
	/*de-interleaveBufferSize if not recomputable from maxDisplacement*/
	u32 deinterleaveBufferSize;
	
	/*The number of bits on which the AU-size field is encoded in the AU-header*/
	u32 SizeLength;
	/*The number of bits on which the AU-Index is encoded in the first AU-header*/
	u32 IndexLength;
	/*The number of bits on which the AU-Index-delta field is encoded in any non-first AU-header*/
	u32 IndexDeltaLength;

	/*The number of bits on which the DTS-delta field is encoded in the AU-header*/
	u32 DTSDeltaLength;
	/*The number of bits on which the CTS-delta field is encoded in the AU-header*/
	u32 CTSDeltaLength;
	/*random access point flag present*/
	Bool RandomAccessIndication;
	
	/*The number of bits on which the Stream-state field is encoded in the AU-header (systems only)*/
	u32 StreamStateIndication;
	/*The number of bits that is used to encode the auxiliary-data-size field 
	(no normative usage of this section)*/
	u32 AuxiliaryDataSizeLength;

	/*internal stuff*/
	/*len of first AU header in an RTP payload*/
	u32 auh_first_min_len;
	u32 auh_min_len;
} RTPSLMap;
	

/*packetizer config flags - some flags are dynamically re-assigned when detecting multiSL / B-Frames / ...*/
enum
{
	/*if flag set, hinter attempts to detect modes (AAC-hbr, etc..) and adjust itself to media*/
	M4HF_AutoConf =	1,
	/*if flag set, RAP flag is signaled in RTP payload*/
	M4HF_SignalRAP	=	2,
	/*if flag set, AU indexes are signaled in RTP payload*/
	M4HF_SignalIDX	=	4,
	/*if flag set, AU size is signaled in RTP payload*/
	M4HF_SignalSize	=	8,
	/*if flag set, CTS is signaled in RTP payload - DTS is automatically set if needed*/
	M4HF_SignalTS	=	16,
	/*if flag set, several AUs can be concatenated in an RTP buffer - Used by MPEG-4 generic and 3GPP timed text
	this forces TKH_SignalSize to be set for MPEG-4*/
	M4HF_UseMulti	=	32
};

/*creates a new track hinter
	@Path_MTU: network transmission unit size
	@CopyMedia: media data is copied into hint tracks (bigger files, less server load)
	@SL_Mode: cf above
	@PayloadID: RTP payload ID to be used (can be one of the above defined or yours cf RFC 1889)
	@OnProgress, cbk_obj: progress callback 
	@sl: user defined SL. If NULL, the lib computes the lightest SL config for the given file/SL mode
	@InterleaveGroupID, InterleaveGroupPriority: cf above
	@e: return error code if failure
*/
typedef struct _tagM4Hinter *LPMP4RTPHINTER;
LPMP4RTPHINTER NewTrackHinter(M4File *file, u32 TrackNum, 
							u32 Path_MTU, u32 flags, 
							u8 PayloadID, Bool copy_media, 
							void (*OnProgress)(void *cbk_obj, u32 done, u32 total),
							void *cbk_obj,
							Bool hint_interleave,
							SLConfigDescriptor *sl, 
							u32 InterleaveGroupID,
							u8 InterleaveGroupPriority,
							M4Err *e);
/*delete the track hinter*/
void MP4T_DeleteHinter(LPMP4RTPHINTER tkHinter);
/*hints all samples in the media track*/
M4Err MP4T_ProcessTrack(LPMP4RTPHINTER tkHint);
/*finalizes hinting process for the track (setup flags, write SDP for RTP, ...)
	@AddSystemInfo: if non-0, systems info are duplicated in the SDP (decoder cfg, PL IDs ..)
*/
M4Err MP4T_FinalizeHintTrack(LPMP4RTPHINTER tkHint, Bool AddSystemInfo);

/*SDP IOD flag*/
enum
{
	/*no IOD included*/
	SDP_IOD_NONE = 0,
	/*base64 encoding of the regular MPEG-4 IOD*/
	SDP_IOD_REGULAR,
	/*base64 encoding of IOD containing BIFS and OD tracks (one AU only) - this is used for ISMA 1.0 profiles
	note that the "hinted" file will loose all systems info*/
	SDP_IOD_ISMA,
	/*same as ISMA but removes all clock references from IOD*/
	SDP_IOD_ISMA_STRICT,
};

/*finalizes hinting process for the file (setup flags, write SDP for RTP, ...)
	@copyright, description: base content info and copyright
	@IOD_Profile: see above
*/
M4Err MP4T_FinalizeHintMovie(M4File *file, u32 IOD_Profile, char *copyright, char *description);


/*
		Generic packetization tools - used by track hinters and future live tools
*/

/*currently supported payload types*/
enum 
{
	/*not defined*/
	RTP_PAYT_UNKNOWN,
	/*use generic MPEG-4 transport - RFC 3016 and RFC 3640*/
	RTP_PAYT_MPEG4,
	/*use generic MPEG-1/2 transport - RFC 2250*/
	RTP_PAYT_MPEG12,
	/*use H263 transport - RFC 2429*/
	RTP_PAYT_H263,
	/*use AMR transport - RFC 3267*/
	RTP_PAYT_AMR,
	/*use AMR-WB transport - RFC 3267*/
	RTP_PAYT_AMR_WB,
	/*use 3GPP Text transport - no RFC yet, only draft*/
	RTP_PAYT_3GPP_TEXT,
};

/*generic rtp builder (packetizer)*/
typedef struct _rtpbuilder *LPM4RTPBUILDER;

/*creates a new builder
	@hintType: hint media type, one of the above
	@flags: hint flags (cf above)
	@slc: user-given SL config to use. If none specified, default RFC 3016 is used
	@cbk_obj: callback object passed back in functions
	@OnNewPacket: callback function starting new RTP packet
		@header: rtp hreader for new packet - note that RTP header flags are not used until PacketDone is called
	@OnPacketDone: callback function closing current RTP packet
		@header: final rtp hreader for packet
		@payl_hdr, @payl_hdr_size: payload specific header(AMR, MPEG-4 SL, ...)
		@payload, @payload_size: RTP payload data
	@OnDataReference: optional, to call each time data from input buffer is added to current RTP packet
		@payload_size: size of reference data
		@offset_from_orig: start offset in input buffer
	@OnData: optional, to call each time extra data is added to current RTP packet (eg when extra header is not concatenated)
*/
LPM4RTPBUILDER NewRTPBuilder(u32 hintType, 
						SLConfigDescriptor *slc, 
						u32 flags,
						void *cbk_obj, 
						void (*OnNewPacket)(void *cbk, RTPHeader *header),
						void (*OnPacketDone)(void *cbk, RTPHeader *header, char *payl_hdr, u32 payl_hdr_size, char *payload, u32 payload_size),
						void (*OnDataReference)(void *cbk, u32 payload_size, u32 offset_from_orig),
						void (*OnData)(void *cbk, char *data, u32 data_size)
					);

/*destroy builder*/
void M4RTP_DeleteBuilder(LPM4RTPBUILDER builder);

/*
		init the builder
	@PathMTU is the base network unit size (UDP packet size for IP, ..)
	@StreamType: MPEG-4 system stream type - MUST always be provided for payloads format specifying audio streams
	audio and video transports.

			*** all other params are for MultiSL draft ***
	
	  @avgSize: average size of an AU. This is not always known (real-time encoding). 
In this case you should specify a rough compute indicating how many packets could be 
stored per RTP packet. for ex AAC stereo at 44100 k / 64kbps , one AU ~= 380 bytes
so 3 AUs for 1500 MTU is ok - BE CAREFULL: MultiSL adds some SL info on top of the 12
byte RTP header so you should specify a smaller size
The packetizer will ALWAYS make sure there's no pb storing the packets so specifying 
more will result in a slight overhead in the SL mapping but the gain to singleSL 
will still be worth it.
	-Nota: at init, the packetizer can decide to switch to SingleSL if the average size 
specified is too close to the PathMTU

	@maxSize: max size of an AU. If unknown (real-time) set to 0
	@avgTS: average CTS progression (1000/FPS for video)
	@maxDTS: maximum DTS offset in case of bidirectional coding. 
	@interleaved: if set and builder is of audio type, will perform interleaving
	on AUs
	@pref_mode: MPEG-4 generic only, specifies the payload mode - can be NULL (mode generic)
*/

void M4RTP_InitBuilder(LPM4RTPBUILDER builder, u8 PayloadType, u32 PathMTU, 
					   u32 StreamType, u32 PL_ID,
					   u32 avgSize, u32 maxSize, 
					   u32 avgTS, u32 maxDTS,
					   Bool interleave, char *pref_mode);

/*packetize input buffer
@data, @data_size: input buffer
@IsAUEnd: set to one if this buffer is the last of the AU
@FullAUSize: complete access unit size if known, 0 otherwise
@duration: sample duration in rtp timescale (only needed for 3GPP text streams)
@descIndex: sample description index (only needed for 3GPP text streams)
*/
M4Err M4RTP_ProcessData(LPM4RTPBUILDER builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex);

/*returns pointer to SL header associated with builder. EVEN IF builder is not an MPEG-4 one, 
you MUST use and modify the SL header to provide CTS (DTS if needed), packetSeqNum and RandomAccessPoint info*/
SLHeader *M4RTP_GetSLHeader(LPM4RTPBUILDER builder);

/*format the "fmtp: " attribute for the given packetizer. sdpline shall be at least 2000 char*/
void M4RTP_FormatSDP(LPM4RTPBUILDER builder, char *payload_name, char *sdpLine, char *dsi, u32 dsi_size);


/*copy trackNum from input to output, reseting dependancies if desired*/
M4Err MP4T_CopyTrack(M4File *infile, u32 inTrackNum, M4File *outfile, Bool ResetDependancies, Bool AddToIOD);
/*creates a reference movie from an hinted one
@in_file: HINTED input movie
@ref_name: output file name (can be changed later on)
@ref_url: full path to server/service. Ex:
	"rtsp://myserver.com/subfolder/mynews.mp4"
	this implies that in_file has to be copied to the directory matching subfolder
	on the specified server, and UNDER THE NAME mynews.mp4, or a 
service name translation has to run on the server
If the name is not correct the reference file will be useless
note that "in_file" may have a different name at the time the function is called
*/
M4Err MP4T_CreateReferenceMovie(M4File *in_file, char *ref_name, char *ref_url);


/*basic MPEG-4 visual object parser (DSI extraction and timing/framing)*/
typedef struct
{
	u8 VideoPL;
	u8 RAP_stream, objectType, has_shape, enh_layer;
	u16 width, height;

	u16 clock_rate;
	u8 NumBitsTimeIncrement;
	u32 time_increment;
} M4VDecoderSpecificInfo;


typedef struct _tagM4V_Parser *LPM4VPARSER;

LPM4VPARSER NewMPEG4VideoParser(unsigned char *data, u32 data_size);
void DeleteMPEG4VideoParser(LPM4VPARSER m4v);
M4Err M4V_ParseConfig(LPM4VPARSER m4v, M4VDecoderSpecificInfo *dsi);

/*get a frame (can contain GOP). The parser ALWAYS resync on the next object in the bitstream
thus you can seek the bitstream to copy the payload without re-seeking it */
M4Err M4V_GetFrame(LPM4VPARSER m4v, M4VDecoderSpecificInfo dsi, u8 *frame_type, u32 *time_inc, u32 *size, u32 *start, Bool *is_coded);
/*returns current object start in bitstream*/
u32 M4V_GetObjectStartPos(LPM4VPARSER m4v);
/*returns 1 if current object is a valid MPEG-4 Visual object*/
Bool M4V_IsValidObjectType(LPM4VPARSER m4v);
/*returns readable description of profile*/
const char *M4V_VideoProfileName(u8 video_pl);
/*decodes DSI*/
M4Err M4V_GetConfig(char *rawdsi, u32 rawdsi_size, M4VDecoderSpecificInfo *dsi);
/*rewrites PL code in DSI*/
void M4V_RewritePL(unsigned char *data, u32 dataLen, u8 PL);

/*MP3 tools*/
u8 MP3_GetNumChannels(u32 hdr);
u16 MP3_GetSamplingRate(u32 hdr);
u16 MP3_GetSamplesPerFrame(u32 hdr);
u16 MP3_GetBitRate(u32 hdr);
u8 MP3_GetObjectTypeIndication(u32 hdr);
u8 MP3_GetLayer(u32 hdr);
u8 MP3_GetVersion(u32 hdr);
const char *MP3_GetVersionName(u32 hdr);
u16 MP3_GetFrameSize(u32 hdr);
u32 MP3_GetNextHeader(FILE* in);
u32 MP3_GetNextHeaderMem(char *buffer, u32 size, u32 *pos);

/*vorbis tools*/
typedef struct
{
	u32 sample_rate, channels, version;
	s32 max_r, avg_r, low_r;
	u32 min_block, max_block;

	/*do not touch, parser private*/
	Bool is_init;
	u32 modebits;
	Bool mode_flag[64];
} VorbisParser;

/*call with vorbis header packets - you MUST initialize the structure to 0 before!!
returns 1 if success, 0 if error.*/
Bool VorbisParseHeader(VorbisParser *vp, char *data, u32 data_len);
/*returns 0 if init error or not a vorbis frame, otherwise returns the number of audio samples
in this frame*/
u32 VorbisCheckFrame(VorbisParser *vp, char *data, u32 data_length);


enum M4A_ObjectTypes
{
    M4A_AAC_Main = 1,
    M4A_AAC_LC = 2,
    M4A_AAC_SBR = 3,
    M4A_AAC_LTP = 4,
    M4A_SBR = 5,
    M4A_AAC_Scalable = 6,
    M4A_TwinVQ = 7,
    M4A_CELP = 8, 
    M4A_HVXC = 9,
    M4A_TTSI = 12,
    M4A_Main_synthetic = 13,
    M4A_Wavetable_Synthesis = 14,
    M4A_General_MIDI = 15,
    M4A_Algorithmic_Synthesis_Audio_FX = 16,
    M4A_ER_AAC_LC = 17,
    M4A_ER_AAC_LTP = 19,
    M4A_ER_AAC_scalable = 20,
    M4A_ER_TwinVQ = 21,
    M4A_ER_BSAC = 22,
    M4A_ER_AAC_LD = 23,
    M4A_ER_CELP = 24,
    M4A_ER_HVXC = 25,
    M4A_ER_HILN = 26,
    M4A_ER_Parametric = 27
};

static const u32 m4a_sample_rates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 
	16000, 12000, 11025, 8000, 7350, 0, 0, 0
};

/*get Audio type from dsi. return audio codec type:*/
typedef struct
{
	u32 nb_chan;
	u32 base_object_type, base_sr, base_sr_index;
	/*SBR*/
	Bool has_sbr;
	u32 sbr_object_type, sbr_sr, sbr_sr_index;
	/*PL indication: this is just a hint computed from object type and SBR flag, may be wrong...*/
	u8 audioPL;
} M4ADecoderSpecificInfo;

M4Err M4A_GetConfig(char *dsi, u32 dsi_size, M4ADecoderSpecificInfo *cfg);

const char *M4A_GetObjectTypeName(u32 objectType);
const char *M4A_AudioProfileName(u8 audio_pl);


typedef struct
{
	u32 bitrate;
	u32 sample_rate;
	u32 framesize;
	u32 channels;
} AC3Header;

Bool AC3_ParseHeader(u8 *buffer, u32 buffer_size, u32 *pos, AC3Header *out_hdr);

/*
		ISMA config
*/

static const u32 ISMA_VIDEO_OD_ID = 20;
static const u32 ISMA_AUDIO_OD_ID = 10;

static const u32 ISMA_VIDEO_ES_ID = 201;
static const u32 ISMA_AUDIO_ES_ID = 101;

static const char ISMA_BIFS_CONFIG[] = {0x00, 0x00, 0x60 };

/*ISMA audio*/
static const u8 ISMA_BIFS_AUDIO[] = 
{
	0xC0, 0x10, 0x12, 0x81, 0x30, 0x2A, 0x05, 0x7C
};
/*ISMA video*/
static const u8 ISMA_BIFS_VIDEO[] = 
{
	0xC0, 0x10, 0x12, 0x60, 0x42, 0x82, 0x28, 0x29,
	0xD0, 0x4F, 0x00
};
/*ISMA audio-video*/
static const u8 ISMA_BIFS_AV[] = 
{
	0xC0, 0x10, 0x12, 0x81, 0x30, 0x2A, 0x05, 0x72, 
	0x60, 0x42, 0x82, 0x28, 0x29, 0xD0, 0x4F, 0x00
};

/*image only - uses same visual OD ID as video*/
static const u8 ISMA_BIFS_IMAGE[] = 
{
	0xC0, 0x11, 0xA4, 0xCD, 0x53, 0x6A, 0x0A, 0x44, 
	0x13, 0x00
};

/*ISMA audio-video*/
static const u8 ISMA_BIFS_AI[] = 
{
	0xC0, 0x11, 0xA5, 0x02, 0x60, 0x54, 0x0A, 0xE4,
	0xCD, 0x53, 0x6A, 0x0A, 0x44, 0x13, 0x00
};

/*make the file ISMA compliant: creates ISMA BIFS / OD tracks if needed, and update audio/video IDs
the file should not contain more than one audio and one video track
@keepImage: if set, generates system info if image is found - only used for image imports
@LogMsg: redirection for message or NULL for stdout
*/
M4Err MP4T_MakeISMA(M4File *mp4file, Bool keepImage, Bool no_ocr, void (*LogMsg)(const char *szMsg));

/*make the file 3GP compliant: the file should not contain more than one audio and one video track
@LogMsg: redirection for message or NULL for stdout
*/
M4Err MP4T_Make3GPP(M4File *mp4file, void (*LogMsg)(const char *szMsg));

/*returns TRUE if the encoded data fits in an ESD url - streamType os the systems stream type*/
Bool MP4T_CanEmbbedData(char *data, u32 data_size, u32 streamType);

/*creates (if needed) an ESDescriptor for the given track - THIS IS RESERVED for local playback
only, since the OTI used when emulated (GPAC_QT_CODECS_OTI) is not standard...*/
ESDescriptor *MP4T_MapESDescriptor(M4File *mp4, u32 track);
/*updates PL indication in IOD - very very basic, only usefull for MPEG-4 visual...*/
void MP4T_CheckMediaProfile(M4File *file, u32 track);

/*			
			track importers

	All these can import a file into a dedicated track. If esd is NULL the track is blindly added 
	otherwise it is added with the requested ESID if non-0, otherwise the new trackID is stored in ESID
	if use_data_ref is set, data is only referenced in the file
	if duration is not 0, only the first duration seconds are imported
	NOTE: if an ESD is specified, its decoderSpecificInfo is also updated
*/
/*track importer flags*/
enum
{
	M4TI_USE_DATAREF = 1,
	/*for AVI video: imports at constant FPS (eg imports N-Vops due to encoder drops)*/
	M4TI_NO_FRAME_DROP = 1<<1,
	/*for AAC audio: forces SBR mode (cannot be signaled through ADTS format*/
	M4TI_FORCE_SBR = 1<<2,
	
	/*when set by user during import, will abort*/
	M4TI_DO_ABORT = 1<<31
};

/*track dumper*/
typedef struct __track_import
{
	M4File *dest;
	/*media to import:
		MP4/ISO media: trackID
		AVI files: 0: all tracks, 1: video track, 2->any: audio track(s)
		MPEG files (only single AV supported for now): 0: all tracks, 1: video track, 2: audio track
	*/
	u32 trackID;
	/*media source - selects importer type based on extension*/
	char *in_name;
	/*progress notif - if NULL dumps to stdout*/
	void (*import_progress)(struct __track_import*, u32 cur_sample, u32 sample_count);
	/*messages notif - if NULL dumps to stdout*/
	void (*import_message)(struct __track_import*, M4Err e, const char *message);
	/*import duration if any*/
	u32 duration;
	/*importer flags*/
	u32 flags;
	/*optional ESD*/
	ESDescriptor *esd;
	
	/*opaque user handle*/
	void *user_data;

	/*for MP4 import only*/
	M4File *orig;

} M4TrackImporter;

M4Err MP4T_ImportMedia(M4TrackImporter *importer);

/*track dumper types*/
enum
{
	/*0: -> native format (JPG, PNG, MP3, raw aac or raw cmp) if supported*/
	DUMP_TRACK_NATIVE = 1,
	/*1: -> NHNT format (any media)*/
	DUMP_TRACK_NHNT,
	/*2: -> AVI (video tracks only)*/
	DUMP_TRACK_AVI,
	/*3: -> MP4 (all except OD)*/
	DUMP_TRACK_MP4
};
/*track dumper*/
typedef struct __track_dumper
{
	M4File *file;
	u32 trackID;
	/*out name WITHOUT extension*/
	char *out_name;
	/*progress notif - if NULL dumps to stdout*/
	void (*dump_progress)(struct __track_dumper*, u32 cur_sample, u32 sample_count);
	/*messages notif - if NULL dumps to stdout*/
	void (*dump_message)(struct __track_dumper*, M4Err e, const char *message);
	/*dump type*/
	u32 dump_type;
	/*for MP4 dumping (to add for AVI...): appends track if file already exists*/
	Bool merge_tracks;

	/*opaque user handle*/
	void *user_data;

	/*set by user*/
	Bool do_abort;
} M4TrackDumper;

/*if error returns same value as error signaled in message*/
M4Err MP4T_DumpTrack(M4TrackDumper *dump);

/*gets image size (bs must contain the whole image) - OTI is image type (JPEG=0x6C, PNG=0x6D)*/
void MP4T_GetImageSize(BitStream *bs, u8 OTI, u32 *width, u32 *height);


/*
		Memory scene management

*/

/*generic systems access unit context*/
typedef struct
{	
	/*AU timing in TimeStampResolution*/
	u32 timing;
	/*timing in sec - used if timing isn't set*/
	Double timing_sec;
	/*random access indication - may be overriden by encoder*/
	Bool is_rap;
	/*opaque command list per stream type*/
	Chain *commands;

	/*pointer to owner stream*/
	struct _stream_context *owner;
} M4AUContext;

/*generic stream context*/
typedef struct _stream_context
{
	/*ESID of stream, or 0 if unknown in which case it is automatically updated at encode stage*/
	u16 ESID;
	/*stream type - used as a hint, the encoder(s) may override it*/
	u8 streamType;
	u8 objectType;
	u32 timeScale;
	Chain *AUs;

	/*last stream AU time, when playing the context directly*/
	u32 last_au_time;
} M4StreamContext;

/*generic presentation context*/
typedef struct 
{
	/*the one and only scene graph used by the scene manager.*/
	LPSCENEGRAPH scene_graph;

	/*all systems streams used in presentation*/
	Chain *streams;
	/*(initial) object descriptor if any - if not set the encoder will generate it*/
	ObjectDescriptor *root_od;

	/*scene resolution*/
	u32 scene_width, scene_height;
	Bool is_pixel_metrics;

	/*BIFS encoding - these is needed for:
	- protos in protos which define subscene graph, hence seperate namespace, but are coded with the same IDs
	- route insertions which are not tracked by the scene graph
	we could do this by hand (counting protos & route insert) but let's be lazy
	*/
	u32 max_node_id, max_route_id, max_proto_id;
} M4SceneManager;

/*scene manager constructor - @scene_graph: scene graph used by the manager. */
M4SceneManager *NewSceneManager(LPSCENEGRAPH scene_graph);
/*scene manager destructor - does not destroy the attached scene graph*/
void M4SM_Delete(M4SceneManager *ctx);
/*retrive or create a stream context in the presentation context
WARNING: if a stream with the same streamType and no ESID already exists in the context, 
it is assigned the requested ES_ID - this is needed to solve base layer*/
M4StreamContext *M4SM_NewStream(M4SceneManager *ctx, u16 ES_ID, u8 streamType, u8 objectType);
/*removes and destroy stream context from presentation context*/
void M4SM_RemoveStream(M4SceneManager *ctx, M4StreamContext *sc);
/*create a new AU context in the given stream context*/
M4AUContext *M4SM_NewAU(M4StreamContext *stream, u32 timing, Double time_ms, Bool isRap);



/*SWF to MPEG-4 flags*/
enum
{
	/*all data in dictionary is in first frame*/
	M4SWF_StaticDictionary = 1,
	/*remove all text*/
	M4SWF_NoText = (1<<1),
	/*remove embedded fonts (force device font usage)*/
	M4SWF_NoFonts = (1<<2),
	/*forces XCurve2D which supports quadratic bezier*/
	M4SWF_UseXCurve = (1<<3),
	/*forces line remove*/
	M4SWF_NoStrike = (1<<4),
	/*forces XLineProperties (supports scalable lines)*/
	M4SWF_UseXLineProps = (1<<5),
	/*forces gradient remove (using center color) */
	M4SWF_NoGradient = (1<<6),
	/*use a dedicated BIFS stream to control display list. This allows positioning in the movie
	(jump to frame, etc..) as well as looping from inside the movie (set by default)*/
	M4SWF_SplitTimeline = (1<<7),
	/*when using SplitTimeline, this flag will prevent generating an AnimationStream in the scene (this is used
	by direct playback only)*/
	M4SWF_NoAnimationStream = (1<<8),
};

enum
{
	/*if set, always load MPEG-4 nodes, otherwise X3D versions are used for vrml/x3d*/
	M4CL_MPEG4_STRICT = 1,
	/*signal loading is done for playback:	
		scrips will be queued in their parent command for later loading
		SFTime (MPEG-4 only) fields will be handled correctly when inserting/creating nodes based on AU timing
	*/
	M4CL_FOR_PLAYBACK = 2
};

/*loader types*/
enum
{	
	M4CL_BT = 1, /*BT loader*/
	M4CL_VRML, /*VRML97 loader*/
	M4CL_X3DV, /*X3D VRML loader*/
	M4CL_XMTA, /*XMT-A loader*/
	M4CL_X3D, /*X3D XML loader*/
	M4CL_SWF, /*SWF->MPEG-4 converter*/
	M4CL_MP4 /*MP4 memory loader*/
};

typedef struct _m4_context_loader
{
	/*scene graph worked on - may be NULL if ctx is present*/
	LPSCENEGRAPH scene_graph;
	/*context manager to load (MUST BE RESETED BEFORE) - may be NULL for loaders not using commands, 
	in which case the graph will be directly updated*/
	M4SceneManager *ctx;
	/*file to import except IsoMedia files*/
	const char *fileName;
	/*IsoMedia file to import (we need to be able to load from an opened file for scene stats)*/
	M4File *isom;
	/*message reporter*/
	void (*OnMessage)(void *cbk, char *msg, M4Err e);
	/*user callback*/
	void *cbk;
	/*user callback - can be frames, bytes etc.. depending on file being loaded*/
	void (*OnProgress)(void *cbk, u32 done, u32 to_do);
	/*swf import flags*/
	u32 swf_import_flags;
	/*swf flatten limit: angle limit below which 2 lines are considered as aligned, 
	in which case the lines are merged as one. If 0, no flattening happens*/
	Float swf_flatten_limit;
	/*swf extraction path: if set, swf media (mp3, jpeg) are extracted to this path. If not set
	media are extracted to original file directory*/
	const char *localPath;

	/*loader flags*/
	u32 flags;

	/*private to loader*/
	void *loader_priv;
	/*loader type, one of the above value. If not set, detected based on file extension*/
	u32 type;
} M4ContextLoader;

/*initializes the context loader - this will load any IOD and the first frame of the main scene*/
M4Err M4SM_LoaderInit(M4ContextLoader *load);
/*terminates the context loader*/
void M4SM_LoaderDone(M4ContextLoader *load);
/*completely loads context*/
M4Err M4SM_LoaderRun(M4ContextLoader *load);

/*translates SRT source into BIFS command stream source
	@src: ESDescriptor of new stream (MUST be created before to store TS resolution)
	@mux: MuxInfoDescriptor of src stream - shall contain a valid file, and at least the textNode member set
*/
M4Err M4SM_ImportSRT_BIFS(M4SceneManager *ctx, ESDescriptor *src, MuxInfoDescriptor *mux);


enum
{
	/*BT*/
	M4SM_DUMP_BT = 0,
	/*XMT-A*/
	M4SM_DUMP_XMTA,
	/*VRML Text (WRL)*/
	M4SM_DUMP_VRML,
	/*X3D Text (x3dv)*/
	M4SM_DUMP_X3D_VRML,
	/*X3D XML*/
	M4SM_DUMP_X3D_XML,
	/*automatic selection of MPEG4 vs X3D, text mode*/
	M4SM_DUMP_AUTO_TXT,
	/*automatic selection of MPEG4 vs X3D, xml mode*/
	M4SM_DUMP_AUTO_XML
};

/*dumps scene context to BT or XMT
@rad_name: file name & loc without extension - if NULL dump will happen in stdout
@dump_mode: one of the above*/
M4Err M4SM_DumpToText(M4SceneManager *ctx, char *rad_name, u32 dump_mode);


/*encoding flags*/
enum
{
	/*if flag set, DEF names are encoded*/
	M4SM_EncodeNames =	1,
	/*if flag set, RAP are generated inband rather than as sync shadow*/
	M4SM_RAPInBand = 2,
};

/*
encodes scene context into @mp4.
if @log is set, generates BIFS encoder log file
if @mediaSource is set, any unknown stream in the scene will be looked for in @mediaSource (MP4 only)
@flags: encoding flags
@rap_freq: delay between 2 RAP in ms. If 0 RAPs are not forced - BIFS only
*/
M4Err M4SM_EncodeFile(M4SceneManager *ctx, M4File *mp4, char *logFile, char *mediaSource, u32 flags, u32 rap_freq);


/*Dumping tools*/
typedef struct _scenedump *LPSCENEDUMPER;
/*create a scene dumper 
@graph: scene graph being dumped
@rad_name: file radical (NULL for stdout) - if not NULL MUST BE M4_MAX_PATH length
@indent_char: indent format
@XMLDump: if set, dumps in XML format otherwise regular text
returns NULL if can't create a file
*/
LPSCENEDUMPER NewSceneDumper(LPSCENEGRAPH graph, char *rad_name, char indent_char, Bool XMLDump);
void DeleteSceneDumper(LPSCENEDUMPER bd);

/*dumps commands list
@indent: indent to use
@skip_replace_scene_header: if set and dumping in BT mode, the initial REPLACE SCENE header is skipped
*/
M4Err SD_DumpCommandList(LPSCENEDUMPER sdump, Chain *comList, u32 indent, Bool skip_first_replace);

/*dumps complete graph - 
@skip_proto: proto declarations are skipped
@skip_routes: routes are not dumped
*/
M4Err SD_DumpGraph(LPSCENEDUMPER sdump, Bool skip_proto, Bool skip_routes);

/*stat object - to refine :)*/

/*store nodes or proto stats*/
typedef struct
{
	/*node type or protoID*/
	u32 tag;
	const char *name;
	/*number of created nodes*/
	u32 nb_created;
	/*number of used nodes*/
	u32 nb_used;
	/*number of used nodes*/
	u32 nb_del;
} NodeStats;

typedef struct _scenestat
{
	Chain *node_stats;
	Chain *proto_stats;
	
	/*ranges of all SFVec2fs for points only (MFVec2fs)*/
	SFVec2f max_2d, min_2d;
	/*number of parsed 2D points*/
	u32 count_2d;
	/*number of deleted 2D points*/
	u32 rem_2d;

	/*ranges of all SFVec3fs for points only (MFVec3fs)*/
	SFVec3f max_3d, min_3d;
	u32 count_3d;
	/*number of deleted 3D points*/
	u32 rem_3d;

	u32 count_float, rem_float;
	u32 count_color, rem_color;
	/*all SFVec2f other than MFVec2fs elements*/
	u32 count_2f;
	/*all SFVec3f other than MFVec3fs elements*/
	u32 count_3f;
} M4SceneStatistics;

typedef struct _statman *LPSTATMAN;

/*creates new stat handler*/
LPSTATMAN M4SM_NewStatisitics();
/*deletes stat handler*/
void M4SM_DeleteStatisitics(LPSTATMAN stat);
/*reset statistics*/
void M4SM_ResetStatisitics(LPSTATMAN stat);

/*get statistics - do NOT modify the returned structure*/
M4SceneStatistics *M4SM_GetStatisitics(LPSTATMAN stat);

/*produces stat report for a complete graph*/
M4Err M4SM_GetGraphStatistics(LPSTATMAN stat, LPSCENEGRAPH sg);

/*produces stat report for the full scene*/
M4Err M4SM_GetStatistics(LPSTATMAN stat, M4SceneManager *sm);

/*produces stat report for the given command*/
M4Err M4SM_GetCommandStatistics(LPSTATMAN stat, SGCommand *com);


#ifdef __cplusplus
}
#endif


#endif 

