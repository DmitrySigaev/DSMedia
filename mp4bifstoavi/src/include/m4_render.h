/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
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

#ifndef M4_RENDER_H
#define M4_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif


/*include scene graph API*/
#include <m4_scenegraph.h>
/*frame buffer definition, M4User and terminal */
#include <m4_avhw.h>

typedef struct scene_renderer *LPSCENERENDER;

/*creates default renderer 
if self_threaded, video renderer uses a dedicated thread, otherwise visual rendering is done by the user
audio renderer always runs in its own thread
term may be NULL, in which case InputSensors won't be enabled
*/
LPSCENERENDER NewSceneRender(M4User *user_interface, Bool self_threaded, Bool no_audio, MPEG4CLIENT term);
void SR_Delete(LPSCENERENDER sr);

/*sets simulation frame rate*/
void SR_SetFrameRate(LPSCENERENDER sr, Double fps);

/*set the root scene graph of the renderer - if NULL remove current and reset simulation time*/
M4Err SR_SetSceneGraph(LPSCENERENDER sr, LPSCENEGRAPH scene_graph);

/*if the renderer doesn't use its own thread for visual, this will perform a render pass*/
M4Err SR_RenderFrame(LPSCENERENDER sr);

/*inits rendering info for the node - shall be called for all nodes the parent system doesn't handle*/
void SR_NodeInit(LPSCENERENDER sr, SFNode *node);

/*notify the given node has been modified. The renderer filters object to decide whether the scene graph has to be 
traversed or not- if object is NULL, this means complete traversing of the graph is requested (use carefully since it
can be a time consuming operation)*/
void SR_Invalidate(LPSCENERENDER sr, SFNode *byObj);

/*pause or resume rendering. If Pause==2 moves to stepMode (only usable when an MPEG4TERM is associated with renderer)*/
void SR_Pause(LPSCENERENDER sr, Bool Pause);

/*return the renderer time - this is the time every time line syncs on*/
u32 SR_GetTime(LPSCENERENDER sr);

/*locks/unlocks the scene rendering - modification of the scene tree shall only happen when scene renderer is locked*/
void SR_Lock(LPSCENERENDER sr, Bool doLock);

/*notify user input - only M4EventMouse and M4EventKey are checked, depending on the renderer used...*/
void SR_UserInput(LPSCENERENDER sr, M4Event *event);

/*maps screen coordinates to bifs 2D coordinates for the current zoom/pan settings
X and Y are point coordinates in the display expressed in BIFS-like fashion (0,0) at center of 
display and Y increasing from bottom to top*/
void SR_MapCoordinates(LPSCENERENDER sr, s32 X, s32 Y, Float *bifsX, Float *bifsY);

/*forces a paint of the window*/
void SR_RefreshWindow(LPSCENERENDER sr);

/*signal the size of the display area has been changed*/
M4Err SR_SizeChanged(LPSCENERENDER sr, u32 NewWidth, u32 NewHeight);

/*set/get user options - options are as defined in M4TermEx*/
M4Err SR_SetOption(LPSCENERENDER sr, u32 type, u32 value);
u32 SR_GetOption(LPSCENERENDER sr, u32 type);

/*returns current FPS
if @absoluteFPS is set, the return value is the absolute framerate, eg NbFrameCount/NbTimeSpent regardless of
whether a frame has been drawn or not, which means the FPS returned can be much greater than the renderer FPS
if @absoluteFPS is not set, the return value is the FPS taking into account not drawn frames (eg, less than or equal to
renderer FPS)
*/
Float SR_GetCurrentFPS(LPSCENERENDER sr, Bool absoluteFPS);


/*user-define management: this is used for instant visual rendering of the scene graph, 
for exporting or authoring tools preview. User is responsible for calling render when desired and shall also maintain
scene timing*/

/*force render tick*/
void SR_Render(LPSCENERENDER sr);
/*gets screen buffer - this locks the scene graph too until released is called*/
M4Err SR_GetScreenBuffer(LPSCENERENDER sr, M4VideoSurface *framebuffer);
/*releases screen buffer and unlocks graph*/
M4Err SR_ReleaseScreenBuffer(LPSCENERENDER sr, M4VideoSurface *framebuffer);

/*renders one frame*/
void SR_SimulationTick(LPSCENERENDER sr);

/*forces graphics cache recompute*/
void SR_ResetGraphics(LPSCENERENDER sr);

/*picks a node (may return NULL) - coords are given in OS client system coordinate, as in UserInput*/
SFNode *SR_PickNode(LPSCENERENDER sr, s32 X, s32 Y);

/*get viewpoints/viewports for main scene - idx is 1-based, and if greater than number of viewpoints return M4EOF*/
M4Err SR_GetViewpoint(LPSCENERENDER sr, u32 viewpoint_idx, const char **outName, Bool *is_bound);
/*set viewpoints/viewports for main scene given its name - idx is 1-based, or 0 to retrieve by viewpoint name
if only one viewpoint is present in the scene, this will bind/unbind it*/
M4Err SR_SetViewpoint(LPSCENERENDER sr, u32 viewpoint_idx, const char *viewpoint_name);

/*render subscene root node. rs is the current traverse stack
this is needed to handle graph metrics changes between scenes...*/
void SR_RenderInline(LPSCENERENDER sr, SFNode *inline_root, void *rs);

/*set outupt size*/
M4Err SR_SetSize(LPSCENERENDER sr, u32 NewWidth, u32 NewHeight);

/*returns total length of audio hardware buffer in ms, 0 if no audio*/
u32 SR_GetAudioBufferLength(LPSCENERENDER sr);

/*add/remove extra scene from renderer (extra scenes are OSDs or any other scene graphs not directly
usable by main scene, like 3GP text streams*/
void SR_RegisterExtraGraph(LPSCENERENDER sr, LPSCENEGRAPH extra_scene, Bool do_remove);

#ifdef __cplusplus
}
#endif

#endif

