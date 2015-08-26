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



#ifndef M4_TERMINAL_H
#define M4_TERMINAL_H


#ifdef __cplusplus
extern "C" {
#endif

#include <m4_user.h>


/*creates a new terminal for a userApp callback*/
MPEG4CLIENT NewMPEG4Term(M4User *user);

/*delete the app - M4T_Stop is called automatically, you don't have to call it before deleting the app
returns M4IOErr if client couldn't be shutdown normally*/
M4Err M4T_Delete(MPEG4CLIENT term);

/*connects to a URL*/
void M4T_ConnectURL(MPEG4CLIENT term, const char *URL);
/*disconnects the url*/
void M4T_CloseURL(MPEG4CLIENT term);
/*reloads (shutdown/restart) the current url if any. This is the only safe way of restarting a 
presentation from inside the EventProc where doing a close/connect could deadlock*/
void M4T_ReloadURL(MPEG4CLIENT term);
/*restarts url from given time (in ms)*/
void M4T_PlayFromTime(MPEG4CLIENT term, u32 from_time);
/*pause/resume rendering
Pause values: 
	0: resume
	1: pause
	2: step (draws next frame and pause)
*/
void M4T_Pause(MPEG4CLIENT term, u32 DoPause);


/*set option - cf m4_user.h*/
M4Err M4T_SetOption(MPEG4CLIENT term, u32 type, u32 value);
/*get option - cf m4_user.h*/
u32 M4T_GetOption(MPEG4CLIENT term, u32 type);

/*returns current framerate
	if @absoluteFPS is set, the return value is the absolute framerate, eg NbFrameCount/NbTimeSpent regardless of
whether a frame has been drawn or not, which means the FPS returned can be much greater than the renderer FPS
depending on threading modes
	if @absoluteFPS is not set, the return value is the FPS taking into account not drawn frames (eg, less than 
	or equal to renderer FPS)
*/
Float M4T_GetCurrentFPS(MPEG4CLIENT term, Bool absoluteFPS);
/*get main scene current time in sec*/
u32 M4T_GetCurrentTimeInMS(MPEG4CLIENT term);


/*get viewpoints/viewports for main scene - idx is 1-based, and if greater than number of viewpoints return M4EOF*/
M4Err M4T_GetViewpoint(MPEG4CLIENT term, u32 viewpoint_idx, const char **outName, Bool *is_bound);
/*set active viewpoints/viewports for main scene given its name - idx is 1-based, or 0 to set by viewpoint name
if only one viewpoint is present in the scene, this will bind/unbind it*/
M4Err M4T_SetViewpoint(MPEG4CLIENT term, u32 viewpoint_idx, const char *viewpoint_name);

/*adds an object to the scene - only possible if scene has selectable streams (cf M4O_HasSelectableStreams option)*/
M4Err M4T_AddObject(MPEG4CLIENT term, const char *url, Bool auto_play);

/*
		OD browsing and misc exports
*/

#include <m4_descriptors.h>

/*
	OD Browsing API - ALL ITEMS ARE READ-ONLY AND SHALL NOT BE MODIFIED
*/
typedef struct _od_manager *LPODMANAGER;

/*returns top-level OD of the presentation*/
LPODMANAGER M4T_GetRootOD(MPEG4CLIENT term);
/*returns number of sub-ODs in the current root. scene_od must be an inline OD*/
u32 M4T_GetODCount(MPEG4CLIENT term, LPODMANAGER scene_od);
/*returns indexed (0-based) OD manager in the scene*/
LPODMANAGER M4T_GetODManager(MPEG4CLIENT term, LPODMANAGER scene_od, u32 index);
/*returns remote ODManager of this OD if any, NULL otherwise*/
LPODMANAGER M4T_GetRemoteOD(MPEG4CLIENT term, LPODMANAGER odm);
/*returns 0 if not inline, 1 if inline, 2 if externProto scene*/
u32 M4T_IsInlineOD(MPEG4CLIENT term, LPODMANAGER odm);

/*select given object when stream selection is available*/
void M4T_SelectOD(MPEG4CLIENT term, LPODMANAGER odm);

typedef struct
{
	ObjectDescriptor *od;
	Double duration;
	Double current_time;
	/*0: stoped, 1: playing, 2: paused, 3: not setup, 4; setup failed.*/
	u32 status;
	/*if set, the PL flags are valid*/
	Bool has_profiles;
	Bool inline_pl;
	u8 OD_pl; 
	u8 scene_pl;
	u8 audio_pl;
	u8 visual_pl;
	u8 graphics_pl;

	/*name of service plugin*/
	const char *service_handler;
	/*name of service*/
	const char *service_url;
	/*set if the service is owned by this object*/
	Bool owns_service;

	/*stream buffer:
		-2: stream is not playing
		-1: stream has no buffering
		>=0: amount of media data present in buffer, in ms
	*/
	s32 buffer;
	/*number of AUs in DB (cumulated on all input channels)*/
	u32 db_unit_count;
	/*number of CUs in composition memory (if any) and CM capacity*/
	u16 cb_unit_count, cb_max_count;
	/*clock drift in ms of object clock: this is the delay set by the audio renderer to keep AV in sync*/
	s32 clock_drift;
	/*codec name*/
	const char *codec_name;
	/*object type - match streamType (cf m4_descriptors)*/
	u32 od_type;
	/*audio properties*/
	u32 sample_rate, bits_per_sample, num_channels;
	/*video properties (w & h also used for scene codecs)*/
	u32 width, height, pixelFormat;

	/*average birate over last second and max bitrate over one second at decoder input - expressed in bits per sec*/
	u32 avg_bitrate, max_bitrate;
	u32 total_dec_time, max_dec_time, nb_dec_frames;
} ODInfo;

/*fills the ODInfo structure describing the OD manager*/
M4Err M4T_GetODInfo(MPEG4CLIENT term, LPODMANAGER odm, ODInfo *info);
/*gets current downloads info for the service - only use if ODM owns thesrevice, returns 0 otherwise.
	@d_enum: in/out current enum - shall start to 0, incremented at each call. fct returns 0 if no more 
	downloads
	@url: file/data location
	@bytes_done, @total_bytes: file info. total_bytes may be 0 (eg http streaming)
	@bytes_per_sec: guess what
*/
Bool M4T_GetDownloadInfo(MPEG4CLIENT term, LPODMANAGER odm, u32 *d_enum, const char **url, u32 *bytes_done, u32 *total_bytes, Float *bytes_per_sec);
/*same principles as above , struct __netcom is defined in m4_network.h*/
typedef struct __netcom NetStatCommand;
Bool M4T_GetChannelNetInfo(MPEG4CLIENT term, LPODMANAGER odm, u32 *d_enum, u32 *chid, NetStatCommand *netcom, M4Err *ret_code);

/*retrieves world info of the scene @odm belongs to. 
If @odm is or points to an inlined OD the world info of the inlined content is retrieved
If @odm is NULL the world info of the main scene is retrieved
returns NULL if no WorldInfo available
returns world title if available 
@descriptions: any textual descriptions is stored here
  all strings are allocated by term and shall be freed by user
*/
char *M4T_GetWorldInfo(MPEG4CLIENT term, LPODMANAGER scene_od, Chain *descriptions);

/*dumps scene graph in specified file, in BT or XMT format
@rad_name: file radical (NULL for stdout) - if not NULL MUST BE M4_MAX_PATH length
if @skip_proto is set proto declarations are not dumped
If @odm is or points to an inlined OD the inlined scene is dumped
If @odm is NULL the main scene is dumped
*/
M4Err M4T_DumpSceneGraph(MPEG4CLIENT term, char *rad_name, Bool xml_dump, Bool skip_proto, LPODMANAGER odm);


/*refresh window info when window moved (redraws offscrenn to screen without rendering) */
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
void M4T_RefreshWindow(MPEG4CLIENT term);

/*request window size change (window resized)*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
M4Err M4T_SetSize(MPEG4CLIENT term, u32 NewWidth, u32 NewHeight);

/*signal window size change (window resized)*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
M4Err M4T_SizeChanged(MPEG4CLIENT term, u32 NewWidth, u32 NewHeight);

/*post user interaction to terminal*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
void M4T_UserInput(MPEG4CLIENT term, M4Event *event);

/*post extended user mouse interaction to terminal 
	X and Y are point coordinates in the display expressed in BIFS-like fashion (0,0) at center of 
	display and Y increasing from bottom to top
	@xxx_but_down: specifiy whether the mouse button is down(2) or up (1), 0 if unchanged
	@wheel: specifiy current wheel inc (0: unchanged , +1 for one wheel delta forward, -1 for one wheel delta backward)
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
void M4T_MouseInput(MPEG4CLIENT term, M4EventMouse *event);

/*post extended user key interaction to terminal 
	@keyPressed, keyReleased: UTF-8 char code of regular keys
	@actionKeyPressed, actionKeyReleased: UTF-8 char code of action keys (Fxx, up, down, left, right, home, end, page up, page down) - cf spec for values
	@xxxKeyDown: specifiy whether given key is down (2) or up (1), 0 if unchanged
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
void M4T_KeyboardInput(MPEG4CLIENT term, s32 keyPressed, s32 keyReleased, s32 actionKeyPressed, s32 actionKeyReleased, u32 shiftKeyDown, u32 controlKeyDown, u32 altKeyDown);

/*post extended user character interaction to terminal 
	@character: unicode character input
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf m4_user.h)*/
void M4T_StringInput(MPEG4CLIENT term, u32 character);


/*connect URL and seek right away - only needed when reloading the complete player (avoids waiting
for connection and post a seek..)*/
void M4T_ConnectURLWithStartTime(MPEG4CLIENT term, const char *URL, u32 startTime);

#ifdef __cplusplus
}
#endif


#endif
