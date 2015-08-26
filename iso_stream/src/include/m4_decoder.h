/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / plugins interfaces
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



#ifndef __M4_DECODER_H_
#define __M4_DECODER_H_


#include <m4_tools.h>

#ifdef __cplusplus
extern "C" {
#endif

/*multimedia processing levels*/
enum
{
	/*normal, full processing*/
	MM_LEVEL_NORMAL,
	/*codec is late, should scale down processing*/
	MM_LEVEL_LATE,
	/*codec is very late, should turn off post-processing, even drop*/
	MM_LEVEL_VERY_LATE,
	/*input frames are already late before decoding*/
	MM_LEVEL_DROP,
	/*this is a special level indicating that a seek is happening (decode but no dispatch)
	it is set dynamically*/
	MM_LEVEL_SEEK
};

/*channel cfg flags - DECODERS MUST OUTPUT STEREO/MULTICHANNEL IN THIS ORDER*/
enum
{
	CHANNEL_UNKNOWN = 0,
	CHANNEL_FRONT_LEFT = (1),
	CHANNEL_FRONT_RIGHT = (1<<1),
	CHANNEL_FRONT_CENTER = (1<<2),
	CHANNEL_LFE = (1<<3),
	CHANNEL_BACK_LEFT = (1<<4),
	CHANNEL_BACK_RIGHT = (1<<5),
	CHANNEL_BACK_CENTER = (1<<6),
	CHANNEL_SIDE_LEFT = (1<<7),
	CHANNEL_SIDE_RIGHT = (1<<8)
};

/*the structure for capabilities*/
typedef struct 
{
	/*cap code cf below*/
	u16 CapCode;
	union {
		u32 valueINT;
		Float valueFT;
	} cap;
} CapObject;


/*
			all codecs capabilities
*/

enum
{
	/*size of a single composition unit */
	CAP_OUTPUTSIZE =	0x01,
	/*resilency: if packets are lost within an AU, resilience means the AU won't be discarded and the codec
	will try to decode */
	CAP_HASRESILIENCE,
	/*critical level of composition memory - if below, media management for the object */
	CAP_BUFFER_MIN,
	/*maximum size in CU of composition memory */
	CAP_BUFFER_MAX,
	/*flags that all AUs should be discarded till next RAP (needed in case RAPs are not carried by the transport
	protocol */
	CAP_WAIT_RAP,
	/*number of padding bytes needed - if the decoder needs padding input cannot be pulled and data is duplicated*/
	CAP_PADDING_BYTES,
	/*codecs can be threaded at will - by default a single thread is used for all decoders and priority is handled
	by the app, but a codec can configure itself to run in a dedicated thread*/
	CAP_CODEC_WANTSTHREAD,

	/*video width and height and horizontal pitch (in YV12 we assume half Y pitch for U and V planes) */
	CAP_WIDTH,
	CAP_HEIGHT,
	CAP_STRIDE,
	CAP_FPS,
	/*video color mode - color modes are defined in m4_tools.h*/
	CAP_COLORMODE,
	/*post-processing options (use to turn them on/off)*/
	CAP_VID_POSTPROC,
	/*query-only, indicates whether the video is shape-coded or not
	if yes, shape offset in main frame has to be retrieved for each frame*/
	CAP_VID_SHAPE,
	/*isgnal decoder performs frame re-ordering in temporal scalability*/
	CAP_VID_REORDER,
	
	/*Audio sample rate*/
	CAP_SAMPLERATE,
	/*Audio num channels*/
	CAP_NBCHANNELS,
	/*Audio bps*/
	CAP_BITSPERSAMPLE,
	/*audio frame format*/
	CAP_CHANNEL_CONFIG,
	/*this is only used for audio in case transport mapping relies on sampleRate (RTP)
	gets the CU duration in samplerate unit (type: int) */
	CAP_CU_DURATION,

	/*This is only called on scene decoders to signal that potential overlay scene should be 
	showed (cap.valueINT=1) or hidden (cap.valueINT=0). Currently only used with SetCap*/
	CAP_SHOW_EXTRASCENE,
	/*This is only called on scene decoders, GetCap only. If the decoder may continue modifying the scene once the last AU is recieved,
	it must set cap.valueINT to 1 (typically, text stream decoder will hold the scene for a given duration
	after the last AU). Otherwise the decoder will be stoped and ask to remove any extra scene being displayed*/
	CAP_MEDIA_NOT_OVER,
};


/*the video post-processing types (flags) */
enum
{
	CAP_VID_NONE = 0,
	CAP_VID_DEBLOCK = (1),
	CAP_VID_DERING = (1<<2),

	/*for scalable*/
	CAP_VID_BASEONLY = (1<<5),
};


	/* Generic interface used by both media decoders and scene decoders
@AttachStream:
	Add a Stream to the codec. If DependsOnESID is NULL, the stream is a base layer
	UpStream means that the decoder should send feedback on this channel. 
	WARNING: Feedback format is not standardized by MPEG
	the same API is used for both encoder and decoder (decSpecInfo is ignored
	for an encoder) 
@DetachStream:
	Remove stream
@GetCapabilities:
	Get the desired capability given its code
@SetCapabilities
	Set the desired capability given its code if possible
	if the codec does not support the request capability, return M4NotSupported
@CanHandleStream
	Can plugin handle this codec? Return 0 if No and !0 otherwise
	decoderSpecificInfo is provided for MPEG4 audio/visual where a bunch of codecs are defined 
	with same objectType
@GetDecoderName
	returns codec name - only called once the stream is successfully attached
@privateStack
	user defined.
*/

#define BASE_CODEC_INTERFACE(IFCE_NAME)		\
	M4_DECL_PLUGIN_INTERFACE	\
	M4Err (*AttachStream)(IFCE_NAME, u16 ES_ID, unsigned char *decSpecInfo, u32 decSpecInfoSize, u16 DependsOnES_ID, u32 objectTypeIndication, Bool UpStream);\
	M4Err (*DetachStream)(IFCE_NAME, u16 ES_ID);\
	M4Err (*GetCapabilities)(IFCE_NAME, CapObject *capability);\
	M4Err (*SetCapabilities)(IFCE_NAME, CapObject capability);\
	Bool (*CanHandleStream)(IFCE_NAME, u32 StreamType, u32 ObjectType, unsigned char *decSpecInfo, u32 decSpecInfoSize, u32 PL);\
	const char *(*GetName)(IFCE_NAME);\
	void *privateStack;	\


typedef struct _basedecoder
{
	BASE_CODEC_INTERFACE(struct _basedecoder *)
} BaseDecoder;

/*media decoder interface name*/
#define M4MEDIADECODERINTERFACE		FOUR_CHAR_INT('M', 'D', 'E', 'C')

/*the media plugin interface. A media plugin MUST be implemented in synchronous mode as time 
and resources management is done by the terminal*/
typedef struct _mediadecoder
{
	BASE_CODEC_INTERFACE(struct _basedecoder *)

	/*Process the media data in inAU. 
	@inBuffer, inBufferLength: encoded input data (complete framing of encoded data)
	@ES_ID: stream this data belongs too (scalable object)
	@outBuffer, outBufferLength: allocated data for decoding - if outBufferLength is not enough
		you must set the size in outBufferLength and M4BufferTooSmall 

	@PaddingBits is the padding at the end of the buffer (some codecs need this info)
	@mmlevel: speed indicator for the decoding - cf above for values*/
	M4Err (*ProcessData)(struct _mediadecoder *, 
			unsigned char *inBuffer, u32 inBufferLength,
			u16 ES_ID,
			unsigned char *outBuffer, u32 *outBufferLength,
			u8 PaddingBits, u32 mmlevel);
} MediaDecoder;




/*extensions for undefined codecs - this allows demuxers and codecs to talk the same language*/

/*this is the OTI (user-priv) used for all undefined codec using MP4/QT 4CC codes*/
#define GPAC_QT_CODECS_OTI				0x80

/*The decoder specific info for all unknown decoders - it is always carried encoded

	u32 codec_four_cc: the codec 4CC reg code
	- for audio - 
	u32 sample_rate: sampling rate or 0 if unknown
	u32 nb_channels: num channels or 0 if unknown
	u32 nb_bits_per_sample: nb bits or 0 if unknown
	u32 num_samples: num audio samples per frame or 0 if unknown

  	- for video - 
	u32 width: video width or 0 if unknown;
	u32 height: video height or 0 if unknown;

	- till end of DSI bitstream-
	char *data: per-codec extensions 
*/


typedef struct _inline_scene *LPINLINESCENE;

/*scene decoder interface name*/
#define M4SCENEDECODERINTERFACE		FOUR_CHAR_INT('S', 'D', 'E', 'C')

typedef struct _scenedecoder
{
	BASE_CODEC_INTERFACE(struct _basedecoder *)
	
	/*attaches scene to the decoder - a scene may be attached to several decoders of several types
	(BIFS or others scene dec, ressource decoders (OD), etc. 
	is: inline scene owning graph (and not just graph), defined in intern/m4_esm_dev.h. With inline scene
	the complete terminal is exposed so there's pretty much everything doable in a scene decoder
	@is_scene_root: set to true if this decoder is the root of the scene, false otherwise (either another decoder
	or a re-entrant call, cf below)
	This is called once upon creation of the decoder (several times if re-entrant)
	*/
	M4Err (*AttachScene)(struct _scenedecoder *, LPINLINESCENE is, Bool is_scene_root);
	/*releases scene. If decoder manages nodes / resources in the scene, THESE MUST BE DESTROYED. 
	May be NULL if decoder doesn't manage nodes but only create them (like BIFS, OD)
	This is called each time the scene is about to be reseted (eg, seek and destroy)
	*/
	M4Err (*ReleaseScene)(struct _scenedecoder *);
	/*Process the scene data in inAU. 
	@inBuffer, inBufferLength: encoded input data (complete framing of encoded data)
	@ES_ID: stream this data belongs too (scalable object)
	@AU_Time: specifies the current AU time. This is usually unused, however is needed for decoder
	handling the scene graph without input data (cf below). In this case the buffer passed is always NULL and the AU
	time caries the time of the scene (or of the stream object attached to the scene decoder, cf below)
	@mmlevel: speed indicator for the decoding - cf above for values*/
	M4Err (*ProcessData)(struct _scenedecoder *, unsigned char *inBuffer, u32 inBufferLength,
					u16 ES_ID, u32 AU_Time, u32 mmlevel);

} SceneDecoder;


/*this streamtype (user-priv) is reserved for streams only used to create a scene decoder handling the service 
by itself, as is the case of the BT/VRML reader and can be used by many other things. 
The decoderSpecificInfo carried is simply the local filename of the service (remote files are first entirelly fetched).
The inBufferLength param for decoders using these streams is the stream clock in ms (no input data is given)
There is a dummy plugin available generating this stream and taking care of proper clock init in case 
of seeking
*this is a reentrant streamtype: if any media object with this streamtype also exist in the scene, they will be 
attached to the scene decoder (except when a new inline scene is detected, in which case a new decoder will 
be created). This allows for animation/sprite usage along with the systems timing/stream management

the objectTypeIndication used for these streams are
		0x00	-	 Forbidden
		0x01	-	 VRML/BT/XMT/SWF loader (eg MP4Box context loading)
		0x02	-	 SVG loader
*/
#define M4ST_PRIVATE_SCENE	0x20

/*object type indication for static OD
	this is used when scene information is not available (or not trustable :), in which case the IOD will 
	only contain the OD stream with this OTI. 
	This OD stream shall send one access unit with all available ressources in the service. Note that it may
	still act as a regular OD stream in case ressources are updated on the fly (broadcast radio/tv for ex).
	The scenegraph wll be regenerated at each OD AU, based on all available objects.
	Using this OTI will enable user stream selection (if provided in GUI) which is otherwise disabled.
	In this mode all clock references are ignored and all streams synchronized on the OD stream.
*/
#define GPAC_STATIC_OD_OTI		0x80


#ifdef __cplusplus
}
#endif

#endif	/*__M4_DECODER_H_*/

