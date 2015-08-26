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

#include "common_stacks.h"

#include <m4_x3d_nodes.h>

void SR_NodeInit(SceneRenderer *sr, SFNode *node)
{
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_AnimationStream: InitAnimationStream(sr, node); break;
	case TAG_MPEG4_AudioBuffer: InitAudioBuffer(sr, node); break;
	case TAG_MPEG4_AudioSource: InitAudioSource(sr, node); break;
	case TAG_MPEG4_AudioClip: case TAG_X3D_AudioClip: InitAudioClip(sr, node); break;
	case TAG_MPEG4_TimeSensor: case TAG_X3D_TimeSensor: InitTimeSensor(sr, node); break;
	case TAG_MPEG4_ImageTexture: case TAG_X3D_ImageTexture: InitImageTexture(sr, node); break;
	case TAG_MPEG4_PixelTexture: case TAG_X3D_PixelTexture: InitPixelTexture(sr, node); break;
	case TAG_MPEG4_MovieTexture: case TAG_X3D_MovieTexture: InitMovieTexture(sr, node); break;
	default:
		sr->visual_renderer->NodeInit(sr->visual_renderer, node);
		break;
	}
}

void SR_Invalidate(SceneRenderer *sr, SFNode *byObj)
{

	if (!byObj) {
		sr->draw_next_frame = 1;
		return;
	}
	switch (Node_GetTag(byObj)) {
	case TAG_MPEG4_AnimationStream: AnimationStreamModified(byObj); break;
	case TAG_MPEG4_AudioBuffer: AudioBufferModified(byObj); break;
	case TAG_MPEG4_AudioSource: AudioSourceModified(byObj); break;
	case TAG_MPEG4_AudioClip: case TAG_X3D_AudioClip: AudioClipModified(byObj); break;
	case TAG_MPEG4_TimeSensor: case TAG_X3D_TimeSensor: TimeSensorModified(byObj); break;
	case TAG_MPEG4_ImageTexture: case TAG_X3D_ImageTexture: ImageTextureModified(byObj); break;
	case TAG_MPEG4_MovieTexture: case TAG_X3D_MovieTexture: MovieTextureModified(byObj); break;
	default:
		/*for all nodes, invalidate parent graph - note we do that for sensors as well to force recomputing
		sensor list cached at grouping node level*/
		if (!sr->visual_renderer->NodeChanged(sr->visual_renderer, byObj)) {
			Node_SetDirty(byObj, 1);
			sr->draw_next_frame = 1;
		}
		break;
	}
}

typedef struct
{
	SFNode *owner;
	struct scene_renderer *compositor;
	TimeNode time_handle;
	Double start_time;
	MediaObject *stream;
	MFURL current_url;
} AnimationStreamStack;

static void DestroyAnimationStream(SFNode *node)
{
	AnimationStreamStack *st = (AnimationStreamStack *) Node_GetPrivate(node);

	if (st->time_handle.is_registered) {
		SR_UnregisterTimeNode(st->compositor, &st->time_handle);
	}
	if (st->stream && st->stream->num_open) MO_Stop(st->stream);
	VRML_MF_Reset(&st->current_url, FT_MFURL);
	free(st);
}


static void AS_CheckURL(AnimationStreamStack *stack, M_AnimationStream *as)
{
	if (!stack->stream) {
		VRML_MF_Reset(&stack->current_url, FT_MFURL);
		VRML_FieldCopy(&stack->current_url, &as->url, FT_MFURL);
		stack->stream = MO_FindObject((SFNode *)as, &as->url);
		SR_Invalidate(stack->compositor, NULL);

		/*if changed while playing trigger*/
		if (as->isActive) {
			MO_Play(stack->stream);
			MO_SetSpeed(stack->stream, as->speed);
		}
		return;
	}
	/*check change*/
	if (MO_UrlChanged(stack->stream, &as->url)) {
		VRML_MF_Reset(&stack->current_url, FT_MFURL);
		VRML_FieldCopy(&stack->current_url, &as->url, FT_MFURL);
		/*if changed while playing stop old source*/
		if (as->isActive) MO_Stop(stack->stream);
		stack->stream = MO_FindObject((SFNode *)as, &as->url);
		/*if changed while playing play new source*/
		if (as->isActive) {
			MO_Play(stack->stream);
			MO_SetSpeed(stack->stream, as->speed);
		}
		SR_Invalidate(stack->compositor, NULL);
	}
}

static Float AS_GetSpeed(AnimationStreamStack *stack, M_AnimationStream *as)
{
	return MO_GetSpeed(stack->stream, as->speed);
}
static Bool AS_GetLoop(AnimationStreamStack *stack, M_AnimationStream *as)
{
	return MO_GetLoop(stack->stream, as->loop);
}

static void AS_Activate(AnimationStreamStack *stack, M_AnimationStream *as)
{
	AS_CheckURL(stack, as);
	as->isActive = 1;
	Node_OnEventOutSTR((SFNode*)as, "isActive");

	MO_Play(stack->stream);
	MO_SetSpeed(stack->stream, as->speed);
}

static void AS_Deactivate(AnimationStreamStack *stack, M_AnimationStream *as)
{
	if (as->isActive) {
		as->isActive = 0;
		Node_OnEventOutSTR((SFNode*)as, "isActive");
	}
	if (stack->stream) MO_Stop(stack->stream);
	stack->time_handle.needs_unregister = 1;

}

static void AS_UpdateTime(TimeNode *st)
{
	Double time;
	M_AnimationStream *as = (M_AnimationStream *)st->obj;
	AnimationStreamStack *stack = Node_GetPrivate(st->obj);
	
	/*not active, store start time and speed*/
	if ( ! as->isActive) {
		stack->start_time = as->startTime;
	}
	time = Node_GetSceneTime(st->obj);

	if ((time < stack->start_time) || (stack->start_time < 0)) return;

	if (AS_GetSpeed(stack, as) && as->isActive) {
		//if stoptime is reached (>startTime) deactivate	
		if ((as->stopTime > stack->start_time) && (time >= as->stopTime) ) {
			AS_Deactivate(stack, as);
			return;
		}
		if (MO_IsFinished(stack->stream)) {
			if (AS_GetLoop(stack, as)) {
				MO_Restart(stack->stream);
			} else if (MO_ShouldDeactivate(stack->stream)) {
				AS_Deactivate(stack, as);
			}
		}
	}

	/*we're (about to be) active: VRML:
	"A time-dependent node is inactive until its startTime is reached. When time now becomes greater than or 
	equal to startTime, an isActive TRUE event is generated and the time-dependent node becomes active 	*/

	if (! as->isActive) AS_Activate(stack, as);
}


void InitAnimationStream(LPSCENERENDER sr, SFNode *node)
{
	AnimationStreamStack *st = malloc(sizeof(AnimationStreamStack));
	memset(st, 0, sizeof(AnimationStreamStack));
	traversable_setup(st, node, sr);
	st->time_handle.UpdateTimeNode = AS_UpdateTime;
	st->time_handle.obj = node;
	
	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyAnimationStream);
	
	SR_RegisterTimeNode(sr, &st->time_handle);
}



void AnimationStreamModified(SFNode *node)
{
	M_AnimationStream *as = (M_AnimationStream *)node;
	AnimationStreamStack *st = (AnimationStreamStack *) Node_GetPrivate(node);
	if (!st) return;

	/*update state if we're active*/
	if (as->isActive) 
		AS_UpdateTime(&st->time_handle);

	/*check URL change*/
	AS_CheckURL(st, as);

	if (!st->time_handle.is_registered && !st->time_handle.needs_unregister) 
		SR_RegisterTimeNode(st->compositor, &st->time_handle);
	else
		st->time_handle.needs_unregister = 0;
}

typedef struct
{
	TimeNode time_handle;
	Bool store_info;
	Double start_time, cycle_interval;
	u32 num_cycles;
	LPSCENERENDER compositor;
} TimeSensorStack;

static
void DestroyTimeSensor(SFNode *ts)
{
	TimeSensorStack *st = Node_GetPrivate(ts);
	if (st->time_handle.is_registered) {
		SR_UnregisterTimeNode(st->compositor, &st->time_handle);
	}
	free(st);
}


static
void ts_deactivate(TimeSensorStack *stack, M_TimeSensor *ts)
{
	ts->isActive = 0;
	Node_OnEventOutSTR((SFNode *) ts, "isActive");
	assert(stack->time_handle.is_registered);
	stack->time_handle.needs_unregister = 1;
	stack->num_cycles = 0;
}

static
void UpdateTimeSensor(TimeNode *st)
{
	Double currentTime, cycleTime;
	Float newFraction;
	u32 inc;
	M_TimeSensor *TS = (M_TimeSensor *)st->obj;
	TimeSensorStack *stack = Node_GetPrivate(st->obj);

	if (! TS->enabled) {
		if (TS->isActive) {
			TS->cycleTime = Node_GetSceneTime(st->obj);
			Node_OnEventOutSTR(st->obj, "cycleTime");
			ts_deactivate(stack, TS);
		}
		return;
	}

	if (stack->store_info) {
		stack->store_info = 0;
		stack->start_time = TS->startTime;
		stack->cycle_interval = TS->cycleInterval;
	}
	
	currentTime = Node_GetSceneTime(st->obj);
	if (currentTime < stack->start_time) return;
	/*special case: if we're greater than both start and stop time don't activate*/
	else if (!TS->isActive && (TS->stopTime > stack->start_time) && (currentTime >= TS->stopTime)) {
		stack->time_handle.needs_unregister = 1;
		return;
	}

	cycleTime = currentTime - stack->start_time - stack->num_cycles * stack->cycle_interval;
	newFraction = (Float) fmod(cycleTime, stack->cycle_interval);
	newFraction /= (Float) stack->cycle_interval;

	if (TS->isActive) {
		TS->time = currentTime;
		Node_OnEventOutSTR(st->obj, "time");

		/*VRML:
			"f = fmod( (now - startTime) , cycleInterval) / cycleInterval
			if (f == 0.0 && now > startTime) fraction_changed = 1.0
			else fraction_changed = f"
		*/
		if (!newFraction && (currentTime > stack->start_time ) ) newFraction = 1.0;

		/*check for deactivation - if so generate a fraction_changed AS IF NOW WAS EXACTLY STOPTIME*/
		if ((TS->stopTime > stack->start_time) && (currentTime >= TS->stopTime) ) {
			cycleTime = TS->stopTime - stack->start_time - stack->num_cycles * stack->cycle_interval;
			TS->fraction_changed = (Float) fmod(cycleTime, stack->cycle_interval);
			TS->fraction_changed /= (Float) stack->cycle_interval;
			/*cf above*/
			if (TS->fraction_changed < M4_EPSILON_FLOAT) TS->fraction_changed = 1.0;
			Node_OnEventOutSTR(st->obj, "fraction_changed");
			ts_deactivate(stack, TS);
			return;
		}
		if (! TS->loop) {
			if (cycleTime >= stack->cycle_interval) {
				TS->fraction_changed = 1.0;
				Node_OnEventOutSTR(st->obj, "fraction_changed");
				ts_deactivate(stack, TS);
				return;
			}
		}
		TS->fraction_changed = newFraction;
		Node_OnEventOutSTR(st->obj, "fraction_changed");
	}

	/*we're (about to be) active: VRML:
	"A time-dependent node is inactive until its startTime is reached. When time now becomes greater than or 
	equal to startTime, an isActive TRUE event is generated and the time-dependent node becomes active 	*/

	if (!TS->isActive) {
		st->needs_unregister = 0;
		TS->isActive = 1;
		Node_OnEventOutSTR(st->obj, "isActive");
		TS->cycleTime = currentTime;
		Node_OnEventOutSTR(st->obj, "cycleTime");
		TS->fraction_changed = newFraction;
		Node_OnEventOutSTR(st->obj, "fraction_changed");
	}

	//compute cycle time
	if (TS->loop && (cycleTime >= stack->cycle_interval) ) {
		inc = 1 + (u32) ( (cycleTime - stack->cycle_interval ) / stack->cycle_interval );
		stack->num_cycles += inc;
		cycleTime -= inc*stack->cycle_interval;
		TS->cycleTime = currentTime - cycleTime;
		Node_OnEventOutSTR(st->obj, "cycleTime");
	}
}

void InitTimeSensor(LPSCENERENDER sr, SFNode *node)
{
	TimeSensorStack *st = malloc(sizeof(TimeSensorStack));
	memset(st, 0, sizeof(TimeSensorStack));
	st->time_handle.UpdateTimeNode = UpdateTimeSensor;
	st->time_handle.obj = node;
	st->store_info = 1;
	st->compositor = sr;

	Node_SetPrivate(node, st);
	Node_SetPreDestroyFunction(node, DestroyTimeSensor);
	/*time sensor needs to be run only if def'ed, otherwise it doesn't impact scene*/
	if (Node_GetID(node)) SR_RegisterTimeNode(sr, &st->time_handle);
}


void TimeSensorModified(SFNode *t)
{
	M_TimeSensor *ts = (M_TimeSensor *)t;
	TimeSensorStack *stack = (TimeSensorStack *) Node_GetPrivate(t);
	if (!stack) return;

	if (ts->isActive) UpdateTimeSensor(&stack->time_handle);

	if (!ts->isActive) stack->store_info = 1;

	if (ts->enabled) {
		stack->time_handle.needs_unregister = 0;
		if (!stack->time_handle.is_registered) {
			SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
		}
	}
}
