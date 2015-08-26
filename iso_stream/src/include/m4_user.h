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



#ifndef M4_USER_H
#define M4_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_tools.h>

/*the MPEG-4 terminal*/
typedef struct _m4_client * MPEG4CLIENT;


/*
		minimal event system handling both WM messages (from video driver) and terminal messages
			inspired from SDL nice event handling
*/

/*events*/
enum {
	/*Mouse move*/
	M4E_MOUSEMOVE = 0,
	/*mouse buttons - note that VRML sensors only deal with events <= M4E_LEFTUP*/
	M4E_LEFTDOWN,
	M4E_LEFTUP,
	M4E_MIDDLEDOWN,
	M4E_MIDDLEUP,
	M4E_RIGHTDOWN,
	M4E_RIGHTUP,
	/*wheel moves*/
	M4E_MOUSEWHEEL,
	/*key actions*/
	M4E_CHAR,	/*valid UTF-16 translated char*/
	M4E_VKEYDOWN,
	M4E_VKEYUP,
	M4E_KEYDOWN,
	M4E_KEYUP,
	/*window events*/
	M4E_WINDOWSIZE,	/*size has changed - indicate new w & h in .x end .y fields of event*/
	M4E_NEEDRESIZE,	/*size must be changed - this is needed for video out not supported multithreading - indicate new w & h in .x end .y fields of event. This event may also be pushed by the terminal to indicate resize*/
	M4E_SHOWHIDE,	/*window show/hide (minimized or other)*/
	M4E_SET_CURSOR,	/*set mouse cursor*/
	M4E_SET_STYLE,	/*set window style*/
	M4E_SET_CAPTION,	/*set window caption*/
	M4E_REFRESH, /*window needs repaint (whenever needed, eg restore, hide->show, background refresh, paint)*/
	M4E_QUIT,	/*window is being closed*/
	M4E_GL_CHANGED,	/*GL context has been changed*/
	/*terminal events*/
	M4E_CONNECT,	/*signal URL is connected*/
	M4E_DURATION,	/*signal duration of presentation*/
	/*signal size of the scene client display (if indicated in scene) upon connection
	if scene size hasn't changed (seeking or other) this event is not sent*/
	M4E_SCENESIZE,	
	M4E_NAVIGATE, /*indicates the user app should load or jump to the given URL.*/
	M4E_MESSAGE, /*message from the MPEG-4 terminal*/
	M4E_VIEWPOINTS,	/*indicates viewpoint list has changed - no struct associated*/
	M4E_STREAMLIST,	/*indicates stream list has changed - no struct associated - only used when no scene info is present*/
};

/*virtual keys - codes respect MPEG4/VRML KeySensor ones*/
enum
{
	M4VK_NONE = 0,
	/*key-sensor codes start*/
	M4VK_F1,
	M4VK_F2,
	M4VK_F3,
	M4VK_F4,
	M4VK_F5,
	M4VK_F6,
	M4VK_F7,
	M4VK_F8,
	M4VK_F9,
	M4VK_F10,
	M4VK_F11,
	M4VK_F12,
	M4VK_HOME,
	M4VK_END,
	M4VK_PRIOR,
	M4VK_NEXT,
	M4VK_UP,
	M4VK_DOWN,
	M4VK_LEFT,
	M4VK_RIGHT,
	/*key-sensor codes end*/
	M4VK_RETURN,
	M4VK_ESCAPE,
	M4VK_SHIFT,
	M4VK_CONTROL,
	M4VK_MENU,
};

/*key modifiers state - set by terminal (not set by video driver)*/
enum
{
	M4KM_SHIFT = (1),
	M4KM_CTRL = (1<<2),
	M4KM_ALT = (1<<3)
};

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_MOUSEMOVE, M4E_MOUSEWHEEL, M4E_LEFTDOWN, M4E_LEFTUP, M4E_MIDDLEDOWN, M4E_MIDDLEUP, M4E_RIGHTDOWN, M4E_RIGHTUP*/
	u8 type;
	/*mouse location in BIFS-like coordinates (window center is 0,0, increasing Y from bottom to top) */
	s32 x, y;
	/*wheel position (wheel current delta / wheel absolute delta) for M4E_MouseWheel*/
	Float wheel_pos;
	/*key modifier*/
	u32 key_states;
} M4EventMouse;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_VK* and M4E_K**/
	u8 type;
	/*above virtual key code*/
	u32 m4_vk_code;
	/* key virtual code (matching ASCI) */
	u32 virtual_code;
	/*key modifier*/
	u32 key_states;
} M4EventKey;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_CHAR*/
	u8 type;
	/*above virtual key code*/
	u32 unicode_char;
} M4EventChar;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_WINDOWSIZE, M4E_SCENESIZE*/
	u8 type;
	/*width and height*/
	u16 width, height;
} M4EventSize;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_SHOWHIDE*/
	u8 type;
	/*set if is visible*/
	Bool is_visible;
} M4EventShow;


/*sensor signaling*/
enum
{
	M4CursorNormal = 0x00,
	M4CursorAnchor, 
	M4CursorTouch,
	/*discSensor, cylinderSensor, sphereSensor*/
	M4CursorRotate, 
	/*proximitySensor & proximitySensor2D*/
	M4CursorProximity, 
	/*planeSensor & planeSensor2D*/
	M4CursorPlane,
	/*collision*/
	M4CursorCollide, 
	M4CursorHide, 
};

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_SET_CURSOR*/
	u8 type;
	/*set if is visible*/
	u32 cursor_type;
} M4EventCursor;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_SET_STYLE*/
	u8 type;
	/*window style flags - NOT USED YET*/
	u32 window_style;
} M4EventStyle;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_SET_CAPTION*/
	u8 type;
	/*window style flags - NOT USED YET*/
	const char *caption;
} M4EventCaption;

/*duration may be signaled several times: it may change when setting up streams
event proc return value: ignored*/
typedef struct
{
	/*M4E_DURATION*/
	u8 type;
	/*duration in seconds*/
	Double duration;
	/*is seeking supported for service*/
	Bool can_seek;
} M4EventDuration;

/*event proc return value: 0 if URL not supported, 1 if accepted (it is the user responsability to load the url)
YOU SHALL NOT DIRECTLY OPEN THE NEW URL IN THE EVENT PROC, THIS WOULD DEADLOCK THE TERMINAL
*/
typedef struct
{
	/*M4E_NAVIGATE*/
	u8 type;
	/*new url to open / data to handle*/
	const char *to_url;
	/*parameters (cf vrml spec:) )*/
	u32 param_count;
	const char **parameters;
} M4EventNavigate;


/*event proc return value: ignored*/
typedef struct
{
	/*M4E_MESSAGE*/
	u8 type;
	/*name of service issuing the message*/
	const char *service;
	/*message*/
	const char *message;
	/*error if any*/
	M4Err error;
} M4EventMessage;

/*event proc return value: ignored*/
typedef struct
{
	/*M4E_CONNECT*/
	u8 type;
	/*sent upon connection/deconnection completion. if any error, it is signaled through message event*/
	Bool is_connected;
} M4EventConnect;


typedef union
{
	u8 type;
	M4EventMouse mouse;
	M4EventKey key;
	M4EventChar character;
	M4EventSize size;
	M4EventShow show;
	M4EventDuration duration;
	M4EventNavigate navigate;
	M4EventMessage message;
	M4EventConnect connect;
	M4EventCaption caption;
	M4EventStyle style;
	M4EventCursor cursor;
} M4Event;
	
/*user object for all callbacks*/
typedef struct 
{
	/*user defined callback for all functions - cannot be NULL*/
	void *opaque;
	/*the event proc. Return value depend on the event type, usually 0
	cannot be NULL if os_window_handler is specified and dont_override_window_proc is set
	may be NULL otherwise*/
	Bool (*EventProc)(void *opaque, M4Event *event);

	/*config file of client - cannot be NULL*/
	LPINIFILE config;
	/*plugins manager - cannot be NULL - owned by the user (to allow selection of plugin directory)*/
	LPPLUGMAN plugins;
	/*optional os window handler (currently only HWND on win32/winCE) 
	if not set the video outut plugin will create and manage the display window.*/
	void *os_window_handler;
	/*only used with os_window_handler (win32/CE). 
	If not set the window proc will be overriden and the video driver will take care of window messages
	If set the window proc will be untouched and the app will have to process the window messages
	*/
	Bool dont_override_window_proc;
} M4User;


/*AspectRatio Type */
enum
{
	M4_AR_Keep = 0, /*keep AR*/
	M4_AR_16_9, /*keep 16/9*/
	M4_AR_4_3, /*keep 4/3*/
	M4_AR_None /*none (all rendering area used)*/
};

/*AntiAlias settings*/
enum
{
	M4_AL_None = 0, /*no antialiasing*/
	M4_AL_Text, /*only text has antialiasing*/
	M4_AL_All /*full antialiasing*/
};

/*interact level settings*/
enum
{	
	/*regular interactions enabled (touch sensors, etc)*/
	M4_InteractNormal = 1,
	/*InputSensor interactions enabled*/
	M4_InteractInputSensor = 2,
	/*zoom/pan interactions enabled (mouse + ctrl/shift)*/
	M4_InteractZoomPan = 4,
	/*interactions from WindowManager enabled (built-in shortcuts). 
	If not set all events are forwarded to the app, otherwise only events not handled are*/
	M4_InteractWM = 8
};

/*BoundingVolume settings*/
enum
{
	M4_Bounds_None = 0, /*doesn't draw bounding volume*/
	M4_Bounds_Box, /*draw object bounding box / rect*/
	M4_Bounds_AABB	/*draw object AABB tree (3D only) */
};

/*Wireframe settings*/
enum
{
	M4_WireNone = 0, /*draw solid volumes*/
	M4_WireOnly, /*draw only wireframe*/
	M4_WireOnSolid /*draw wireframe on solid object*/
};


/*navigation type*/
enum 
{
	/*navigation is disabled by content and cannot be forced by user*/
	M4_NoNavigation,
	/*2D navigation modes only can be used*/
	M4_Navigation2D,
	/*3D navigation modes only can be used*/
	M4_Navigation3D
};

/*navigation modes - non-VRML ones are simply blaxxun contact ones*/
enum
{
	/*no navigation*/
	M4_NavNone = 0,
	/*3D navigation modes*/
	/*walk navigation*/
	M4_NavWalk,
	/*fly navigation*/
	M4_NavFly,
	/*pan navigation*/
	M4_NavPan,
	/*game navigation*/
	M4_NavGame,
	/*slide navigation, for 2D and 3D*/
	M4_NavSlide,
	/*all modes below disable collision detection & gravity in 3D*/
	/*examine navigation, for 2D and 3D */
	M4_NavExamine,
	/*orbit navigation - 3D only*/
	M4_NavOrbit,
};

/*collision flags*/
enum
{
	/*no collision*/
	M4_CollideNone,
	/*regular collision*/
	M4_Collide,
	/*collision with camera displacement*/
	M4_CollideDisp,
};

/*high-level options*/
enum
{
	/*common options between 2D and 3D renderers*/
	/*set/get antialias flag (value: one of the AntiAlias enum) - may be ignored with 3D renderer depending on graphic cards*/
	M4O_Antialias  =0,
	/*set/get fast mode (value: boolean) */
	M4O_HighSpeed,
	/*set/get fullscreen flag (value: boolean) */
	M4O_Fullscreen,
	/*reset top-level transform to original (value: boolean)*/
	M4O_OriginalView,
	/*overrides BIFS size info for simple AV - this is not recommended since
	it will resize the window to the size of the biggest texture (thus some elements
	may be lost)*/
	M4O_OverrideSize,
	/*set (no get) audio volume (value is intensity between 0 and 100) */
	M4O_AudioVolume,
	/*set (no get) audio pan (value is pan between 0 (all left) and 100(all right) )*/
	M4O_AudioPan,
	/*get javascript flag (no set, depends on compil) - value: boolean, true if JS enabled in build*/
	M4O_HasScript,
	/*get slectable stream flag (no set) - value: boolean, true if audio/video/subtitle stream selection is 
	possible with content (if an MPEG-4 scene description is not present). Use regular OD browsing to get streams*/
	M4O_HasSelectableStreams,
	/*set/get control interaction, OR'ed combination of interaction flags*/
	M4O_InteractLevel,
	/*set display window visible / get show/hide state*/
	M4O_Visible,
	/*set freeze display on/off / get freeze state freeze_display prevents any screen updates 
	needed when output driver uses direct video memory access*/
	M4O_FreezeDisplay,
	/*get isOver flag: if true the file can be restarted, otherwise it should not
	this is used to check is there are several timelines, timesensors or interactions, in which case 
	the file could be running for an undetermined period.
	Note that nothing prevents the user app to restart such a file*/
	M4O_IsOver,
	/*set/get aspect ratio (value: one of AspectRatio enum) */
	M4O_AspectRatio,
	/*send a force redraw message (SetOption only): all graphics info (display list, vectorial path) is 
	recomputed, and textures are reloaded in HW*/
	M4O_ForceRedraw,
	/*set/get stress mode (value: boolean) - in stress mode a M4O_ForceRedraw is emulated at each frame*/
	M4O_StressMode,
	/*get/set bounding volume drawing (value: one of the above option)*/
	M4O_BoundingVolume,
	/*get/set texture text option - when enabled and usable (that depends on content), text is first rendered 
	to a texture and only the texture is drawn, rather than drawing all the text each time (CPU intensive)*/
	M4O_UseTextureText,
	/*fake option, reload config file (set only), including drivers. Plugins configs are not reloaded*/
	M4O_ReloadConfig,
	/*get: returns whether the content enable navigation and if it's 2D or 3D.
	set: reset viewpoint (whatever value is given)*/
	M4O_NavigationType,
	/*get current navigation mode - set navigation mode if allowed by content - this is not a resident
	option (eg not stored in cfg)*/
	M4O_NavigationMode,

	/*
			2D renderer only options
	*/
	/*set/get direct rendering flag. In direct rendering, the screen is entirely redrawn at each frame
	value: boolean
	*/
	M4O_DirectRender,
	/*set/get scalable zoom (value: boolean)*/
	M4O_ScalableZoom,
	/*set/get YUV acceleration (value: boolean) */
	M4O_YUVHardware,
	/*get (set not supported yet) hardware YUV format (value: YUV 4CC) */
	M4O_YUVFormat,
	
	/*		3D ONLY OPTIONS		*/
	/*set/get raster outline flag (value: boolean) - when set, no vectorial outlining is done, only 
	openGL raster outline*/
	M4O_RasterOutlines,
	/*set/get pow2 emulation flag (value: boolean) - when set, video textures with non power of 2 dimensions
	are emulated as pow2 by expanding the video buffer (image is not scaled). Otherwise the entire image
	is rescaled. This flag does not affect image textures, which are always rescaled*/
	M4O_EmulatePOW2,
	/*get/set polygon antialiasing flag (value: boolean) (may be ugly with some cards)*/
	M4O_PolygonAA,
	/*get/set wireframe flag (value: cf above) (may be ugly with some cards)*/
	M4O_Wireframe,
	/*disable backface culling*/
	M4O_NoBackCull,
	/*get/set RECT Ext flag (value: boolean) - when set, GL rectangular texture extension is not used 
	(but NPO2 texturing is if available)*/
	M4O_NoRectExt,
	/*get/set bitmap draw mode. If set, bitmap doesn't use texturing but direct video copy*/
	M4O_BitmapCopyPixels,
	/*set/get headlight (value: boolean)*/
	M4O_Headlight,
	/*set/get collision (value: cf above)*/
	M4O_Collision,
	/*set/get gravity*/
	M4O_Gravity,

	M4O_UNDEFINED
};

/*macro for event forwarding*/
#define M4USER_SENDEVENT(_user, _evt)	if (_user->EventProc) _user->EventProc(_user->opaque, _evt)

/*macro for message event format/send*/
#define M4USER_MESSAGE(_user, _serv, _msg, _e)	\
	{	\
		M4Event evt;	\
		if (_user->EventProc) {	\
			evt.type = M4E_MESSAGE;	\
			evt.message.service = _serv;	\
			evt.message.message = _msg;	\
			evt.message.error = _e;	\
			_user->EventProc(_user->opaque, &evt);	\
		}	\
	}

/*macro for size event format/send*/
#define M4USER_SETSIZE(_user, _w, _h)	\
	{	\
		M4Event evt;	\
		if (_user->EventProc) {	\
			evt.type = M4E_SCENESIZE;	\
			evt.size.width = _w;	\
			evt.size.height = _h;	\
			_user->EventProc(_user->opaque, &evt);	\
		}	\
	}



#ifdef __cplusplus
}
#endif

#endif

