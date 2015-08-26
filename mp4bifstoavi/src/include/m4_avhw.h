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


/*

		Note on video driver: this is not a graphics driver, the only thing requested from this driver
	is accessing video memory and performing stretch of YUV and RGB on the backbuffer (bitmap node)
	the graphics driver is a different entity that performs 2D rasterization

*/

#ifndef _M4_AVHW_H
#define _M4_AVHW_H

#ifdef __cplusplus
extern "C" {
#endif

/*include event system*/
#include <m4_user.h>

/*
		Video hardware output plugin
*/


/*color modes are in m4_tools.h*/

typedef struct
{
	u32 width, height, pitch;
	u32 pixel_format;
	/*pointer to video memory (top, left)*/
	unsigned char *video_buffer;
	/*os specific handler if needed*/
	void *os_handle;
} M4VideoSurface;

/*window for blitting - all coords are x, y at top-left, x+w, y+h at bottom-right */
typedef struct
{
	u32 x, y;
	u32 w, h;
} M4Window;




/*opengl configuration*/
typedef struct
{
	Bool double_buffered;
} M4GLConfig;

/*interface name for video output*/
#define M4_VIDEO_OUTPUT_INTERFACE	FOUR_CHAR_INT('M','V','I','D') 

/*
			video output interface

	the vidoe output may run in 2 modes: 2D and 3D.
	** the 2D video output works by creating surfaces on the video mem board - the app refers to the surfaces 
	by their IDs, and access them through the M4VideoSurface handler. 
	Surface index 0 is reserved for the main video memory (if double (or n) buffering is performed by the driver 
	the surface 0 is the backbuffer (or current flip buffer) )
	Overlay surfaces are not supported yet due to lack of time and also no profile for simple AV in MPEG4

	** the 3D video output only handles window management and openGL contexts setup.
	The context shall be setup in Resize and SetFullScreen calls which are always happening in the main 
	rendering thread. This will take care of openGL context issues with multithreading
*/
typedef struct _video_out
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*setup system - if os_handle is NULL the driver shall create the output display (common case)
	the other case is currently only used by child windows on win32 and winCE
	@no_proc_override: when set and a os_handle is passed, the plugin shall not try to
	override the window proc
	if cfg is specified, the output is 3D, otherwise 2D*/
	M4Err (*SetupHardware)(struct _video_out *vout, void *os_handle, Bool no_proc_override, M4GLConfig *cfg);
	/*shutdown system */
	void (*Shutdown) (struct _video_out *vout);
	/*resize main video*/
	M4Err (*Resize) (struct _video_out *vout, u32 newWidth, u32 newHeight);
	/*set full screen - screen resolution shall be reported in screen_width and screen_height when turning 
	on FS, otherwise retored window size shall be reported - the screen mode to select shall be the smallest 
	one bigger than current output size - the driver may destroy all extra surfaces created when 
	switching to fullscreen*/
	M4Err (*SetFullScreen) (struct _video_out *vout, Bool bFullScreenOn, u32 *screen_width, u32 *screen_height);

	/*flush video: the video shall be presented to screen 
	the destination area to update is in client display coordinates (0,0) being top-left, (w,h) bottom-right
	it shall be ugnored when using 3D output (buffer flip only)*/
	M4Err (*FlushVideo) (struct _video_out *vout, M4Window *dest);

	/*window events: only set cursor, set title, set style and set visible used*/
	M4Err (*PushEvent)(struct _video_out *vout, M4Event *event);

	/*pass events to user (assigned before setup)*/
	void *evt_cbk_hdl;
	void (*on_event)(void *hdl, M4Event *event);

	/*driver private*/
	void *opaque;

	Bool bHas3DSupport;
	/*
			All the following are 2D specific and are NEVER called in 3D mode
	*/
	/*clears screen with specified color*/
	M4Err (*Clear) (struct _video_out *vout, u32 color);
	/*creates a offscreen video surface and setup surface id - pixel format MUST be respected except for YUV
	formats, where the hardware is free to choose the fastest format for blit*/
	M4Err (*CreateSurface) (struct _video_out *vout, u32 width, u32 height, u32 pixel_format, u32 *surfaceID);
	/*deletes video surface by id*/
	M4Err (*DeleteSurface) (struct _video_out *vout, u32 surface_id);
	/*lock video mem*/
	M4Err (*LockSurface)(struct _video_out *vout, u32 surface_id, M4VideoSurface *video_info);
	/*unlock video mem*/
	M4Err (*UnlockSurface)(struct _video_out *vout, u32 surface_id);
	/*checks if the surface is valid - this is used to discard surfaces when changing video mode (fullscreen)*/
	Bool (*IsSurfaceValid) (struct _video_out *vout, u32 surface_id);
	/*resize surface - the resulting surface can still be larger than what requested*/
	M4Err (*ResizeSurface) (struct _video_out *vout, u32 surface_id, u32 width, u32 height);

	/*lock video mem through OS context (HDC, ...)*/
	void *(*GetContext)(struct _video_out *vout, u32 surface_id);
	/*unlock video mem through OS context (HDC, ...)*/
	void (*ReleaseContext)(struct _video_out *vout, u32 surface_id, void *context);

	/*blit operations - windows are provided in surface coordinate*/

	/*blit surface src to surface dest - if a window is not specified, the full surface is used _ can be NULL*/
	M4Err (*Blit)(struct _video_out *vout, u32 src_id, u32 dst_id, M4Window *src, M4Window *dst);
	/*blit surface src to surface dest with keying for src surface - if a window is not specified, the full surface is used - can be NULL*/
	M4Err (*BlitKey)(struct _video_out *vout, u32 color_key, u32 src_id, u32 dst_id, M4Window *src, M4Window *dst);
	/*blit surface src to surface dest with alpha - if a window is not specified, the full surface is used - can be NULL*/
	M4Err (*BlitAlpha)(struct _video_out *vout, u32 alpha, u32 src_id, u32 dst_id, M4Window *src, M4Window *dst);

	/*returns pixel format of the surface - if surfaceID is 0, the main video memory format is requested*/
	M4Err (*GetPixelFormat) (struct _video_out *vout, u32 surfaceID, u32 *pixel_format);

	/*set to true if hardware supports color keying*/
	Bool bHasKeying;
	/*set to true if hardware supports color keying with stretching*/
	Bool bHasKeyingStretch;
	/*set to true if hardware supports texture blending*/
	Bool bHasAlpha;
	/*set to true if hardware supports texture blending with stretching*/
	Bool bHasAlphaStretch;
	/*set to true if YV12 input can be blited - this may be changed dynamically whenever a YUV surface is 
	resized or card main format changes*/
	Bool bHasYUV;
} VideoOutput;



/*
	Audio hardware output plugin
*/

/*interface name for audio driver*/
#define M4_AUDIO_OUTPUT_INTERFACE		FOUR_CHAR_INT('M','A','U','D')

/*interface returned on query interface*/
typedef struct _audiooutput
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*setup system 
		Win32: os_handle is HWND

	if num_buffer is set, the audio driver should work with num_buffers of 1000/num_buffers_per_sec ms length each
	if not set the driver is free to decide what to do
	*/
	M4Err (*SetupHardware) (struct _audiooutput *aout, void *os_handle, u32 num_buffers, u32 num_buffers_per_sec);

	/*shutdown system */
	void (*Shutdown) (struct _audiooutput *aout);

	/*query output frequency available - if the requested sampleRate is not available, the driver shall return the best 
	possible sampleRate able to handle NbChannels and NbBitsPerSample - if it doesn't handle the NbChannels
	the internal mixer will do it
	*/
	u32 (*QueryOutputSampleRate)(struct _audiooutput *aout, u32 desired_samplerate, u32 NbChannels, u32 nbBitsPerSample);

	/*set output config - if audio is not running, driver must start it
	*SampleRate, *NbChannels, *nbBitsPerSample: 
		input: desired value
		output: final values
	channel_cfg is the channels output cfg, eg set of flags as specified in m4_decoder.h
	*/
	M4Err (*ConfigureOutput) (struct _audiooutput *aout, u32 *SampleRate, u32 *NbChannels, u32 *nbBitsPerSample, u32 channel_cfg);

	/*returns total buffer size used in ms. This is needed to compute the min size of audio decoders output*/
	u32 (*GetTotalBufferTime)(struct _audiooutput *aout);

	/*returns audio delay in ms, eg time delay until written audio data is outputed by the sound card
	This function is only called after ConfigureOuput*/ 
	u32 (*GetAudioDelay)(struct _audiooutput *aout);

	/*set output volume(between 0 and 100) */
	void (*SetVolume) (struct _audiooutput *aout, u32 Volume);
	/*set balance (between 0 and 100, 0=full left, 100=full right)*/
	void (*SetPan) (struct _audiooutput *aout, u32 pan);
	/*freezes soundcard flow - must not be NULL for self threaded*/
	void (*Pause) (struct _audiooutput *aout, Bool DoFreeze);
	/*specifies whether the driver relies on the app to feed data or runs standalone*/
	Bool SelfThreaded;

	/*if not using private thread, this should perform sleep & fill of HW buffer
		the audio render loop in this case is: while (run) {driver->WriteAudio(); if (reconf) Reconfig();}
	the driver must therefore give back the hand to the renderer as often as possible - the usual way is:
		Sleep untill hw data can be written
		write HW data
		return
	*/
	void (*WriteAudio)(struct _audiooutput *aout);

	/*if using private thread the following MUST be provided*/
	void (*SetPriority)(struct _audiooutput *aout, u32 priority);

	/*your private data handler - should be allocated when creating the interface object*/
	void *opaque;
	
	/*these are assigned by the audio renderer once plugin is loaded*/
	
	/*fills the buffer with audio data*/
	void (*FillBuffer) (void *audio_renderer, char *buffer, u32 buffer_size);
	void *audio_renderer;

} AudioOutput;


#ifdef __cplusplus
}
#endif


#endif	/*_M4_AVHW_H*/

