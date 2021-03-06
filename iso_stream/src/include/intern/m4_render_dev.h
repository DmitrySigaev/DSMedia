/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2004 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Rendering sub-project
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

#ifndef _M4_RENDER_DEV_H_
#define _M4_RENDER_DEV_H_


/*include scene graph API*/
#include <m4_render.h>
/*include nodes*/
#include <m4_scenegraph.h>
/*bridge between the rendering engine and the systems media engine*/
#include <intern/m4_esm_dev.h>
/*some tools for both 2D and 3D*/
#include <intern/m4_render_tools.h>

/*FPS computed on this number of frame*/
#define FPS_COMPUTE_SIZE	30
/*N-bezier are decomposed in this number of segments (2 and 3-order bezier are flattened through subdivisions)*/
#define DEFAULT_CURVE_RESOLUTION	60


/*used with GL only*/
typedef struct 
{
	Bool multisample;
	Bool bgra_texture;
	Bool abgr_texture;
	Bool npot_texture;
	Bool rect_texture;
} HardwareCaps;

typedef struct scene_renderer
{
	/*the main user*/
	M4User *client;
	/*the terminal running the graph - only used for InputSensor*/
	MPEG4CLIENT term;
	/*audio renderer*/
	struct _audio_render *audio_renderer;
	/*renderer plugin (cf below)*/
	struct visual_render_plugin *visual_renderer;
	/*video out*/
	VideoOutput *video_out;
	/*graphics driver - now also used by 3D renderer for text texturing & gradients generation*/
	Graphics2DDriver *g_hw;

	/*visual rendering thread if used*/
	M4Thread *VisualThread;
	/*0: not init, 1: running, 2: exit requested, 3: done*/
	u32 video_th_state;
	M4Mutex *mx;

	/*the scene graph*/
	LPSCENEGRAPH scene;
	/*extra scene graphs (OSD, etc), always registered in draw order. That's the plugin responsability
	to draw them*/
	Chain *extra_scenes;

	
	/*all time nodes registered*/
	Chain *time_nodes;
	/*all textures (texture handlers)*/
	Chain *textures;

	/*event queue*/
	Chain *events;
	M4Mutex *ev_mx;

	/*simulation frame rate*/
	Double frame_rate;
	u32 frame_duration;
	u32 frame_time[FPS_COMPUTE_SIZE];
	u32 current_frame;

	u32 scene_width, scene_height;
	Bool has_size_info;
	/*screen size*/
	u32 width, height;
	Bool fullscreen;
	/*!! paused will not stop display (this enables pausing a VRML world and still examining it)*/
	Bool paused, step_mode;
	Bool draw_next_frame;
	/*freeze_display prevents any screen updates - needed when output driver uses direct video memory access*/
	Bool is_hidden, freeze_display;

	/*count number of initialized sensors*/
	u32 interaction_sensors;

	/*set whenever 3D HW ctx changes (need to rebuild dlists/textures if any used)*/
	Bool reset_graphics;

	/*font engine*/
	FontRaster *font_engine;

	/*options*/
	u32 aspect_ratio, antiAlias, base_curve_resolution;
	Bool high_speed, stress_mode, use_texture_text;

	/*key modif*/
	u32 key_states;
	u32 interaction_level;

	/*size override when no size info is present
		flags:	1: size override is requested (cfg)
				2: size override has been applied
	*/
	u32 override_size_flags;

	/*0: nothing, 1: override size msg, 2: fullscreen req, 3: aspect ratio recompute needed, 4: user size change (window is already resized), 5: size change from scene config*/
	u32 msg_type;
	/*for size*/
	u32 new_width, new_height;

	/*specific hw caps (openGL for now)*/
	HardwareCaps hw_caps;

	/*unit box (1.0 size) and unit sphere (1.0 radius)*/
	u32 draw_bvol;
} SceneRenderer;

/*macro setting up rendering stack with following member @owner: node pointer and @compositor: compositor*/
#define	traversable_setup(_object, _owner, _compositor)	\
		_object->owner = _owner;		\
		_object->compositor = _compositor; \


/*base stack for timed nodes (nodes that activate themselves at given times)
	@UpdateTimeNode: shall be setup by the node handler and is called once per simulation frame
	@is_registerd: all handlers indicate store their state if wanted (provided for conveniency but 
not inspected by the renderer)
	@needs_unregister: all handlers indicate they can be removed from the list of active time nodes
in order to save time. THIS IS INSPECTED by the renderer at each simulation tick after calling UpdateTimeNode
and if set, the node is removed right away from the list
*/
typedef struct _time_node
{
	void (*UpdateTimeNode)(struct _time_node *);
	Bool is_registered, needs_unregister;
	SFNode *obj;
} TimeNode;

void SR_RegisterTimeNode(LPSCENERENDER sr, TimeNode *tn);
void SR_UnregisterTimeNode(LPSCENERENDER sr, TimeNode *tn);


/*texturing - Movietexture, ImageTexture and PixelTextures are handled by main renderer*/
typedef void *LPHWTEXTURE;

enum
{
	TX_REPEAT_S = (1<<0),
	TX_REPEAT_T = (1<<1)
};

typedef struct _texture_handler
{
	SFNode *owner;
	SceneRenderer *compositor;
	/*driver texture object*/
	LPHWTEXTURE hwtx;
	/*media stream*/
	MediaObject *stream;
	/*texture is open (for DEF/USE)*/
	Bool is_open;
	/*this is needed in case the Url is changed - note that media nodes cannot point to different
	URLs (when they could in VRML), the MF is only holding media segment descriptions*/
	MFURL current_url;
	/*to override by each texture node*/
	void (*update_texture_fcnt)(struct _texture_handler *txh);
	/*needs_release if a visual frame is grabbed (not used by plugins)*/
	Bool needs_release;
	/*stream_finished: indicates stream is over (not used by plugins)*/
	Bool stream_finished;
	/*needs_refresh: indicates texture content has been changed - needed by plugins performing tile drawing*/
	Bool needs_refresh;
	/*needed to discard same frame fetch*/
	u32 last_frame_time;
	/*active display in the texture (0, 0 == top, left)*/
	M4Rect active_window;
	/*texture is transparent*/		
	Bool transparent;
	/*flags for user - the repeatS and repeatT are set upon creation, the rest is NEVER touched by renderer*/
	u32 flags;
	/*gradients are relative to the object bounds, therefore a gradient is not the same if used on 2 different
	objects - since we don't want to build an offscreen texture for the gradient, gradients have to be updated 
	at each draw - the matrix shall be updated to the gradient transformation in the local system
	MUST be set for gradient textures*/
	void (*compute_gradient_matrix)(struct _texture_handler *txh, M4Rect *bounds, M4Matrix2D *mat);

	/*image data for natural media*/
	char *data;
	u32 width, height, stride, pixelformat, frame_size;
	/*if set texture has been transformed by MatteTexture -> disable blit*/
	Bool has_cmat;
} TextureHandler;

/*setup texturing object*/
void texture_setup(TextureHandler *hdl, SceneRenderer *sr, SFNode *owner);
/*destroy texturing object*/
void texture_destroy(TextureHandler *txh);

/*return texture handle for built-in textures (movieTexture, ImageTexture and PixelTexture)*/
TextureHandler *texture_get_handler(SFNode *n);

/*these ones are needed by plugins only for Background(2D) handling*/

/*returns 1 if url changed from current one*/
Bool texture_check_url_change(TextureHandler *txh, MFURL *url);
/*starts associated object*/
M4Err texture_play(TextureHandler *txh, MFURL *url);
/*stops associated object*/
void texture_stop(TextureHandler *txh);
/*restarts associated object - DO NOT CALL stop/start*/
void texture_restart(TextureHandler *txh);
/*common routine for all video texture: fetches a frame and update the 2D texture object */
void texture_update_frame(TextureHandler *txh, Bool disable_resync);
/*release video memory if needed*/
void texture_release_stream(TextureHandler *txh);


/*user interaction event storage - only mouse and keys are send to plugins*/
typedef struct
{
	u32 event_type;
	M4EventMouse mouse;
	M4EventKey key;
} M4UserEvent;

/*
		Renderer Plugin: this is a very basic interface allowing both 2D and 3D renderers
		to be loaded at run-time
*/

/*interface name for video output*/
#define M4_RENDERER_INTERFACE	FOUR_CHAR_INT('R','E','N','D') 

typedef struct visual_render_plugin VisualRenderer;

struct visual_render_plugin
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*load renderer - a pointer to the compositor is passed for later access to generic rendering stuff*/
	M4Err (*LoadRenderer)(VisualRenderer *vr, LPSCENERENDER compositor);
	/*unloads renderer*/
	void (*UnloadRenderer)(VisualRenderer *vr);

	/*the node private stack creation function. Note that some nodes are handled at a higher level thus will 
	never get to the renderer.*/
	void (*NodeInit)(VisualRenderer *vr, SFNode *node);
	/*signals the given node has been modified. If the plugin returns FALSE, the node will be marked as dirty
	and all its parents marked wityh DIRTY_CHILD flag*/
	Bool (*NodeChanged)(VisualRenderer *vr, SFNode *node);

	/*signals that output size has been changed so that the plugin may update output scene size (2D mainly)*/
	M4Err (*RecomputeAR)(VisualRenderer *vr);
	/*signals the scene graph has been deconnected: all vars related to the scene shall be reseted & memory cleanup done*/
	void (*SceneReset)(VisualRenderer *vr);
	/*draw the scene
		1 - the first subtree to be rendered SHALL BE the main scene attached to compositor
		2 - there may be other graphs to draw (such as subtitles, etc,) not related to the main scene in extra_scenes list.
		3 - it is the plugin responsability to flush the video driver
		!!! The scene may be NULL, in which case the screen shall be cleared
	*/
	void (*DrawScene)(VisualRenderer *vr);
	/*execute given event.
	for mouse events, x and y are in BIFS fashion (eg, from x in [-screen_width, screen_width] and y in [-screen_height, screen_height]) 
	return 1 if event matches a pick in the scene, 0 otherwise (this avoids performing shortcuts when user
	clicks on object...*/
	Bool (*ExecuteEvent)(VisualRenderer *vr, M4UserEvent *event);
	/*signals the hw driver has been reseted to reload cfg*/	
	void (*GraphicsReset)(VisualRenderer *vr);
	/*render inline scene*/
	void (*RenderInline)(VisualRenderer *vr, SFNode *inline_root, void *rs);
	/*get viewpoints/viewports for main scene - idx is 1-based, or 0 to retrieve by viewpoint name.
	if idx is greater than number of viewpoints return M4EOF*/
	M4Err (*GetViewpoint)(VisualRenderer *vr, u32 viewpoint_idx, const char **outName, Bool *is_bound);
	/*set viewpoints/viewports for main scene given its name - idx is 1-based, or 0 to retrieve by viewpoint name
	if only one viewpoint is present in the scene, this will bind/unbind it*/
	M4Err (*SetViewpoint)(VisualRenderer *vr, u32 viewpoint_idx, const char *viewpoint_name);

	/*natural texture handling (image & video)*/

	/*allocates hw texture*/
	M4Err (*AllocTexture)(TextureHandler *hdl);
	/*releases hw texture*/
	void (*ReleaseTexture)(TextureHandler *hdl);
	/*push texture data*/
	M4Err (*SetTextureData)(TextureHandler *hdl);
	/*signal HW reset in case the driver needs to reload texture*/
	void (*TextureHWReset)(TextureHandler *hdl);

	/*set/get option*/
	M4Err (*SetOption)(VisualRenderer *vr, u32 option, u32 value);
	u32 (*GetOption)(VisualRenderer *vr, u32 option);

	/*get/release video memory - READ ONLY*/
	M4Err (*GetScreenBuffer)(VisualRenderer *vr, M4VideoSurface *framebuffer);
	/*releases screen buffer and unlocks graph*/
	M4Err (*ReleaseScreenBuffer)(VisualRenderer *vr, M4VideoSurface *framebuffer);

	/*set to true if needs an OpenGL context - MUST be set when interface is created (before LoadRenderer)*/
	Bool bNeedsGL;

	/*user private*/
	void *user_priv;
};


/*
	Audio mixer
*/

/*max number of channels we support in mixer*/
#define MAX_CHANNELS	6

/*the audio object as used by the mixer. All audio nodes need to implement this interface*/
typedef struct _audiointerface
{
	/*fetch audio data for a given audio delay (~soundcard drift) - if delay is 0 sync should not be performed 
	(eg intermediate mix) */
	char *(*FetchFrame) (void *callback, u32 *size, u32 audio_delay_ms);
	/*release a number of bytes in the indicated frame (ts)*/
	void (*ReleaseFrame) (void *callback, u32 nb_bytes);
	/*get media speed*/
	Float (*GetSpeed)(void *callback);
	/*gets volume for each channel - vol = Float[6]. returns 1 if volume shall be changed (!= 1.0)*/
	Bool (*GetChannelVolume)(void *callback, Float *vol);
	/*returns 1 if muted*/
	Bool (*IsMuted)(void *callback);
	/*user callback*/
	void *callback;
	/*returns 0 if config is not known yet, otherwise updates member var below and return TRUE.
	You may return 0 to force parent user invalidation*/
	Bool (*GetConfig)(struct _audiointerface *ai);
	/*cfg*/
	u32 chan, bps, sr, ch_cfg;
} AudioInterface;

typedef struct __audiomix AudioMixer;

/*create mixer - ar is NULL for any sub-mixers, or points to the main audio renderer (mixer outputs to sound driver)*/
AudioMixer *NewAudioMixer(struct _audio_render *ar);
void DeleteAudioMixer(AudioMixer *am);
void AM_RemoveAllInputs(AudioMixer *am);
void AM_AddSource(AudioMixer *am, AudioInterface *src);
void AM_RemoveSource(AudioMixer *am, AudioInterface *src);
void AM_Lock(AudioMixer *am, Bool lockIt);
/*mix inputs in buffer, return number of bytes written to output*/
u32 AM_GetMix(AudioMixer *am, void *buffer, u32 buffer_size);
/*reconfig all sources if needed - returns TRUE if main audio config changed
NOTE: this is called at each AM_GetMix by the mixer. To call externally for audio hardware
reconfiguration only*/
Bool AM_Reconfig(AudioMixer *am);
/*retrieves mixer cfg*/
void AM_GetMixerConfig(AudioMixer *am, u32 *outSR, u32 *outCH, u32 *outBPS, u32 *outChCfg);
/*called by audio renderer in case the hardware used a different setup than requested*/
void AM_SetMixerConfig(AudioMixer *am, u32 outSR, u32 outCH, u32 outBPS, u32 ch_cfg);
Bool AM_IsSourcePresent(AudioMixer *am, AudioInterface *ifce);
u32 AM_GetSourceCount(AudioMixer *am);
void AM_ForceChannelCount(AudioMixer *am, u32 num_channels);
u32 AM_GetBlockAlign(AudioMixer *am);

/*the audio renderer*/
typedef struct _audio_render
{
	AudioOutput *audio_out;
	/*startup time (the audio renderer is used when present as the system clock)*/
	u32 startTime;
	/*frozen time counter if set*/
	Bool Frozen;
	u32 FreezeTime;

	/*final output*/
	AudioMixer *mixer;
	/*client*/
	M4User *client;

	/*audio thread*/
	M4Thread *th;
	/*thread state: 0: not intit, 1: running, 2: waiting for stop, 3: done*/
	u32 audio_th_state;

	Bool resync_clocks;
	Bool force_cfg;
	u32 num_buffers, num_buffer_per_sec;
	
	Bool need_reconfig;
	u32 priority;

	u32 audio_delay;
} AudioRenderer, *LPAUDIORENDERER;

/*creates audio renderer*/
AudioRenderer *LoadAudioRenderer(M4User *client);
/*deletes audio renderer*/
void AR_Delete(AudioRenderer *ar);
/*pauses audio renderer*/
void AR_Pause(AudioRenderer *ar, Bool DoFreeze, Bool step_mode);
/*set volume and pan*/
void AR_SetVolume(AudioRenderer *ar, u32 Volume);
void AR_SetPan(AudioRenderer *ar, u32 Balance);
/*set audio priority*/
void AR_SetPriority(AudioRenderer *ar, u32 priority);
/*gets time in msec - this is the only clock used by the whole ESM system - depends on the audio driver*/
u32 AR_GetTime(AudioRenderer *ar);
/*reset all input nodes*/
void AR_ResetSources(AudioRenderer *ar);
/*add audio node*/
void AR_AddSourceNode(AudioRenderer *ar, AudioInterface *source);
/*remove audio node*/
void AR_RemoveSourceNode(AudioRenderer *ar, AudioInterface *source);
/*reconfig audio hardware if needed*/
void AR_Reconfig(AudioRenderer *ar);
u32 AR_GetAudioDelay(AudioRenderer *ar);

/*the sound node interface for intensity & spatialization*/
typedef struct _soundinterface
{
	/*gets volume for each channel - vol = Float[6]. returns 1 if volume shall be changed (!= 1.0)
	if NULL channels are always at full intensity*/
	Bool (*GetChannelVolume)(SFNode *owner, Float *vol);
	/*get sound priority (0: min, 255: max) - used by mixer to determine*/
	u8 (*GetPriority) (SFNode *owner);
	/*node owning the structure*/
	SFNode *owner;
} SoundInterface;

/*audio common to AudioClip and AudioSource*/
typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	AudioInterface input_ifce;
	/*can be NULL if the audio node generates its output from other input*/
	MediaObject *stream;
	/*object speed and intensity*/
	Float speed, intensity;
	Bool stream_finished;
	Bool need_release;
	MFURL url;
	Bool is_open, is_muted;
	Bool register_with_renderer, register_with_parent;

	SoundInterface *snd;
} AudioInput;
/*setup interface with audio renderer - overwrite any functions needed after setup EXCEPT callback object*/
void setup_audio_input(AudioInput *ai, LPSCENERENDER sr, SFNode *node);
/*open audio object*/
M4Err open_audio(AudioInput *ai, MFURL *url);
/*closes audio object*/
void stop_audio(AudioInput *ai);
/*restarts audio object (cf note in MediaObj)*/
void restart_audio(AudioInput *ai);

Bool audio_check_url_changed(AudioInput *ai, MFURL *url);

/*base grouping audio node (nodes with several audio sources as children)*/
#define AUDIO_GROUP_NODE	\
	AudioInput output;		\
	void (*add_source)(struct _audio_group *_this, AudioInput *src);	\

typedef struct _audio_group
{
	AUDIO_GROUP_NODE
} AudioGroup;


enum
{
	/*part of a switched-off subtree (needed for audio)*/
	TF_SWITCHED_OFF = (1<<1),
};

/*base class for the traversing context: this is needed so that audio renderer can work without knowledge of
the used graphics driver. All traversing contexts must derive from this one
rend_flag (needed for audio): one of the above*/
#define AUDIO_EFFECT_CLASS	\
	struct _audio_group *audio_parent;	\
	SoundInterface *sound_holder;	\
	u32 trav_flags;	\

typedef struct 
{
	AUDIO_EFFECT_CLASS	
} BaseRenderEffect;


/*register audio node with parent audio renderer (mixer or main renderer)*/
void audio_register_node(AudioInput *ai, BaseRenderEffect *eff);
void audio_unregister_node(AudioInput *ai);

#endif

