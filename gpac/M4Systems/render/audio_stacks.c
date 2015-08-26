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


#include "common_stacks.h"

typedef struct
{
	AudioInput input;
	TimeNode time_handle;
	Double start_time;
	Bool set_duration;
} AudioClipStack;

static void DestroyAudioClip(SFNode *node)
{
	AudioClipStack *st = (AudioClipStack *)Node_GetPrivate(node);
	stop_audio(&st->input);
	audio_unregister_node(&st->input);
	if (st->time_handle.is_registered) {
		SR_UnregisterTimeNode(st->input.compositor, &st->time_handle);
	}
	free(st);
}


static void AC_Activate(AudioClipStack *st, M_AudioClip *ac)
{
	open_audio(&st->input, &ac->url);
	ac->isActive = 1;
	Node_OnEventOutSTR((SFNode *)ac, "isActive");

	MO_SetSpeed(st->input.stream, st->input.speed);
	/*rerender all graph to get parent audio group*/
	SR_Invalidate(st->input.compositor, NULL);
}

static void AC_Deactivate(AudioClipStack *st, M_AudioClip *ac)
{
	stop_audio(&st->input);
	ac->isActive = 0;
	Node_OnEventOutSTR((SFNode *)ac, "isActive");

	st->time_handle.needs_unregister = 1;
}

static void RenderAudioClip(SFNode *node, void *rs)
{
	BaseRenderEffect *eff = (BaseRenderEffect *)rs;
	M_AudioClip *ac = (M_AudioClip *)node;
	AudioClipStack *st = (AudioClipStack *)Node_GetPrivate(node);
	/*check end of stream*/
	if (st->input.stream && st->input.stream_finished) {
		if (MO_GetLoop(st->input.stream, ac->loop)) {
			restart_audio(&st->input);
		} else if (ac->isActive && MO_ShouldDeactivate(st->input.stream)) {
			/*deactivate*/
			AC_Deactivate(st, ac);
		}
	}
	if (ac->isActive) {
		audio_register_node(&st->input, (BaseRenderEffect*)rs);
	}
	if (st->set_duration && st->input.stream) {
		ac->duration_changed = MO_GetDuration(st->input.stream);
		Node_OnEventOutSTR(node, "duration_changed");
		st->set_duration = 0;
	}

	/*store mute flag*/
	st->input.is_muted = (eff->trav_flags & TF_SWITCHED_OFF);
}

static void AC_UpdateTime(TimeNode *tn)
{
	Double time;
	M_AudioClip *ac = (M_AudioClip *)tn->obj;
	AudioClipStack *st = (AudioClipStack *)Node_GetPrivate(tn->obj);

	if (! ac->isActive) {
		st->start_time = ac->startTime;
		st->input.speed = ac->pitch;
	}
	time = Node_GetSceneTime(tn->obj);
	if ((time<st->start_time) || (st->start_time<0)) return;
	
	if (ac->isActive) {
		if ( (ac->stopTime > st->start_time) && (time>=ac->stopTime)) {
			AC_Deactivate(st, ac);
			return;
		}
	}
	if (!ac->isActive) AC_Activate(st, ac);
}


void InitAudioClip(LPSCENERENDER sr, SFNode *node)
{
	AudioClipStack *st = malloc(sizeof(AudioClipStack));
	memset(st, 0, sizeof(AudioClipStack));
	setup_audio_input(&st->input, sr, node);

	st->time_handle.UpdateTimeNode = AC_UpdateTime;
	st->time_handle.obj = node;
	st->set_duration = 1;

	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, RenderAudioClip);
	Node_SetPreDestroyFunction(node, DestroyAudioClip);

	SR_RegisterTimeNode(sr, &st->time_handle);
}


void AudioClipModified(SFNode *node)
{
	M_AudioClip *ac = (M_AudioClip *)node;
	AudioClipStack *st = (AudioClipStack *) Node_GetPrivate(node);
	if (!st) return;

	/*MPEG4 spec is not clear about that , so this is not forbidden*/
	if (st->input.is_open && st->input.is_open) {
		if (audio_check_url_changed(&st->input, &ac->url)) {
			stop_audio(&st->input);
			open_audio(&st->input, &ac->url);
			/*force unregister to resetup audio cfg*/
			audio_unregister_node(&st->input);
			SR_Invalidate(st->input.compositor, NULL);
		}
	}

	//update state if we're active
	if (ac->isActive) {
		AC_UpdateTime(&st->time_handle);
		/*we're no longer active fon't check for reactivation*/
		if (!ac->isActive) return;
	}

	/*make sure we are still registered*/
	if (!st->time_handle.is_registered && !st->time_handle.needs_unregister) 
		SR_RegisterTimeNode(st->input.compositor, &st->time_handle);
	else
		st->time_handle.needs_unregister = 0;
}


typedef struct
{
	AudioInput input;
	TimeNode time_handle;
	Bool is_active;
	Double start_time;
} AudioSourceStack;

static void DestroyAudioSource(SFNode *node)
{
	AudioSourceStack *st = (AudioSourceStack *)Node_GetPrivate(node);
	stop_audio(&st->input);
	audio_unregister_node(&st->input);
	if (st->time_handle.is_registered) {
		SR_UnregisterTimeNode(st->input.compositor, &st->time_handle);
	}
	free(st);
}


static void AS_Activate(AudioSourceStack *st, M_AudioSource *as)
{
	open_audio(&st->input, &as->url);
	st->is_active = 1;
	MO_SetSpeed(st->input.stream, st->input.speed);
	/*rerender all graph to get parent audio group*/
	SR_Invalidate(st->input.compositor, NULL);
}

static void AS_Deactivate(AudioSourceStack *st, M_AudioSource *as)
{
	stop_audio(&st->input);
	st->is_active = 0;
	st->time_handle.needs_unregister = 1;
}

static void RenderAudioSource(SFNode *node, void *rs)
{
	BaseRenderEffect*eff = (BaseRenderEffect*)rs;
	M_AudioSource *as = (M_AudioSource *)node;
	AudioSourceStack *st = (AudioSourceStack *)Node_GetPrivate(node);
	/*check end of stream*/
	if (st->input.stream && st->input.stream_finished) {
		if (MO_GetLoop(st->input.stream, 0)) {
			restart_audio(&st->input);
		} else if (st->is_active && MO_ShouldDeactivate(st->input.stream)) {
			/*deactivate*/
			AS_Deactivate(st, as);
		}
	}
	if (st->is_active) {
		audio_register_node(&st->input, (BaseRenderEffect*)rs);
	}

	/*store mute flag*/
	st->input.is_muted = (eff->trav_flags & TF_SWITCHED_OFF);
}

static void AS_UpdateTime(TimeNode *tn)
{
	Double time;
	M_AudioSource *as = (M_AudioSource *)tn->obj;
	AudioSourceStack *st = (AudioSourceStack *)Node_GetPrivate(tn->obj);

	if (! st->is_active) {
		st->start_time = as->startTime;
		st->input.speed = as->speed;
	}
	time = Node_GetSceneTime(tn->obj);
	if ((time<st->start_time) || (st->start_time<0)) return;
	
	if (st->input.input_ifce.GetSpeed(st->input.input_ifce.callback) && st->is_active) {
		if ( (as->stopTime > st->start_time) && (time>=as->stopTime)) {
			AS_Deactivate(st, as);
			return;
		}
	}
	if (!st->is_active) AS_Activate(st, as);
}


void InitAudioSource(LPSCENERENDER sr, SFNode *node)
{
	AudioSourceStack *st = malloc(sizeof(AudioSourceStack));
	memset(st, 0, sizeof(AudioSourceStack));
	setup_audio_input(&st->input, sr, node);

	st->time_handle.UpdateTimeNode = AS_UpdateTime;
	st->time_handle.obj = node;

	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, RenderAudioSource);
	Node_SetPreDestroyFunction(node, DestroyAudioSource);

	SR_RegisterTimeNode(sr, &st->time_handle);
}


void AudioSourceModified(SFNode *node)
{
	M_AudioSource *as = (M_AudioSource *)node;
	AudioSourceStack *st = (AudioSourceStack *) Node_GetPrivate(node);
	if (!st) return;

	/*MPEG4 spec is not clear about that , so this is not forbidden*/
	if (st->input.is_open&& st->input.is_open) {
		if (audio_check_url_changed(&st->input, &as->url)) {
			stop_audio(&st->input);
			open_audio(&st->input, &as->url);
			/*force unregister to resetup audio cfg*/
			audio_unregister_node(&st->input);
			SR_Invalidate(st->input.compositor, NULL);
		}
	}

	//update state if we're active
	if (st->is_active) {
		AS_UpdateTime(&st->time_handle);
		if (!st->is_active) return;
	}

	/*make sure we are still registered*/
	if (!st->time_handle.is_registered && !st->time_handle.needs_unregister) 
		SR_RegisterTimeNode(st->input.compositor, &st->time_handle);
	else
		st->time_handle.needs_unregister = 0;
}


typedef struct
{
	AUDIO_GROUP_NODE

	TimeNode time_handle;
	Double start_time;
	Bool set_duration;
	/*AudioBuffer mixes its children*/
	AudioMixer *am;
	Bool is_init;
	/*buffer audio data*/
	char *buffer;
	u32 buffer_size;

	Bool done;
	/*read/write position in buffer and associated read time (CTS)*/
	u32 read_pos, write_pos, cur_cts;
	/*list of audio children after a traverse*/
	Chain *new_inputs;
} AudioBufferStack;

static void DestroyAudioBuffer(SFNode *node)
{
	AudioBufferStack *st = (AudioBufferStack *)Node_GetPrivate(node);

	audio_unregister_node(&st->output);
	if (st->time_handle.is_registered) 
		SR_UnregisterTimeNode(st->output.compositor, &st->time_handle);

	DeleteAudioMixer(st->am);
	if (st->buffer) free(st->buffer);
	DeleteChain(st->new_inputs);
	free(st);
}


/*we have no choice but always browsing the children, since a src can be replaced by a new one
without the parent being modified. We just collect the src and check against the current mixer inputs
to reset the mixer or not - the spec is not clear about that btw, shall rebuffering happen if a source is modified or not ...*/
static void RenderAudioBuffer(SFNode *node, void *rs)
{
	u32 count, i, j;
	Bool update_mixer;
	AudioGroup *parent;
	AudioBufferStack *st = (AudioBufferStack *)Node_GetPrivate(node);
	M_AudioBuffer *ab = (M_AudioBuffer *)node;
	BaseRenderEffect*eff = (BaseRenderEffect*) rs;

	parent = eff->audio_parent;
	eff->audio_parent = (AudioGroup *) st;
	count = ChainGetCount(ab->children);
	for (i=0; i<count; i++) {
		SFNode *ptr = ChainGetEntry(ab->children, i);
		Node_Render(ptr, eff);
	}

	AM_Lock(st->am, 1);

	/*if no new inputs don't change mixer config*/
	update_mixer = ChainGetCount(st->new_inputs) ? 1 : 0;
	
	if (AM_GetSourceCount(st->am) == ChainGetCount(st->new_inputs)) {
		update_mixer = 0;
		for (j=0; j<ChainGetCount(st->new_inputs); j++) {
			AudioInput *cur = ChainGetEntry(st->new_inputs, j);
			if (!AM_IsSourcePresent(st->am, &cur->input_ifce)) {
				update_mixer = 1;
				break;
			}
		}
	}

	if (update_mixer) {
		AM_RemoveAllInputs(st->am);
		AM_ForceChannelCount(st->am, ab->numChan);
	}

	while (ChainGetCount(st->new_inputs)) {
		AudioInput *src = ChainGetEntry(st->new_inputs, 0);
		ChainDeleteEntry(st->new_inputs, 0);
		if (update_mixer) AM_AddSource(st->am, &src->input_ifce);
	}

	AM_Lock(st->am, 0);
	eff->audio_parent = parent;

	/*Note the audio buffer is ALWAYS registered untill destroyed since buffer filling shall happen even when inactive*/
	if (!st->output.register_with_parent || !st->output.register_with_renderer) 
		audio_register_node(&st->output, eff);
}



static void AB_Activate(AudioBufferStack *st, M_AudioBuffer *ab)
{
	ab->isActive = 1;
	Node_OnEventOutSTR((SFNode *)ab, "isActive");
	/*rerender all graph to get parent audio group*/
	SR_Invalidate(st->output.compositor, NULL);
	st->done = 0;
	st->read_pos = 0;
}

static void AB_Deactivate(AudioBufferStack *st, M_AudioBuffer *ab)
{
	ab->isActive = 0;
	Node_OnEventOutSTR((SFNode *)ab, "isActive");
	st->time_handle.needs_unregister = 1;
}

static void AB_UpdateTime(TimeNode *tn)
{
	Double time;
	M_AudioBuffer *ab = (M_AudioBuffer *)tn->obj;
	AudioBufferStack *st = (AudioBufferStack *)Node_GetPrivate(tn->obj);

	if (! ab->isActive) {
		st->start_time = ab->startTime;
	}
	time = Node_GetSceneTime(tn->obj);
	if ((time<st->start_time) || (st->start_time<0)) return;
	
	if (ab->isActive) {
		if ( (ab->stopTime > st->start_time) && (time>=ab->stopTime)) {
			AB_Deactivate(st, ab);
			return;
		}
		/*THIS IS NOT NORMATIVE*/
		if ( !ab->loop && st->done) {
			AB_Deactivate(st, ab);
			return;
		}
	}
	if (!ab->isActive) AB_Activate(st, ab);
}




static char *AB_FetchFrame(void *callback, u32 *size, u32 audio_delay_ms)
{
	u32 blockAlign;
	AudioBufferStack *st = (AudioBufferStack *) Node_GetPrivate( ((AudioInput *) callback)->owner);
	M_AudioBuffer *ab = (M_AudioBuffer*)st->output.owner;

	if (!st->is_init) return NULL;
	if (!st->buffer) {
		st->done = 0;
		st->buffer_size = (u32) ceil(ab->length * st->output.input_ifce.bps*st->output.input_ifce.sr*st->output.input_ifce.chan/8);
		blockAlign = AM_GetBlockAlign(st->am);
		/*BLOCK ALIGN*/
		while (st->buffer_size%blockAlign) st->buffer_size++;
		st->buffer = malloc(sizeof(char) * st->buffer_size);
		memset(st->buffer, 0, sizeof(char) * st->buffer_size);
		st->read_pos = st->write_pos = 0;
	}
	if (st->done) return NULL;

	/*even if not active, fill the buffer*/
	if (st->write_pos < st->buffer_size) {
		u32 written;
		while (1) {
			/*just try to completely fill it*/
			written = AM_GetMix(st->am, st->buffer + st->write_pos, st->buffer_size - st->write_pos);
			if (!written) break;
			st->write_pos += written;
			assert(st->write_pos<=st->buffer_size);
		}
	}
	/*not playing*/
	if (! ab->isActive) return NULL;
	*size = st->write_pos - st->read_pos;
	return st->buffer + st->read_pos;
}

static void AB_ReleaseFrame(void *callback, u32 nb_bytes)
{
	AudioBufferStack *st = (AudioBufferStack *) Node_GetPrivate( ((AudioInput *) callback)->owner);
	st->read_pos += nb_bytes;
	assert(st->read_pos<=st->write_pos);
	if (st->read_pos==st->write_pos) {
		if (st->write_pos<st->buffer_size) {
			/*reading faster than buffering - let's still attempt to fill the buffer*/
#if 0
			st->write_pos = st->buffer_size;
			fprintf(stdout, "Warning: AudioBuffer done playing before buffer filling done\n");
#endif
		} else if ( ((M_AudioBuffer*)st->output.owner)->loop) {
			st->read_pos = 0;
		} else {
			st->done = 1;
		}
	}
}


static Float AB_GetSpeed(void *callback)
{
	M_AudioBuffer *ab = (M_AudioBuffer *) ((AudioInput *) callback)->owner;
	return ab->pitch;
}

static Bool AB_GetChannelVolume(void *callback, Float *vol)
{
	AudioInput *ai = (AudioInput *) callback;
	if (ai->snd->GetChannelVolume) {
		return ai->snd->GetChannelVolume(ai->snd->owner, vol);
	} else {
		vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = vol[5] = 1;
		return 0;
	}
}

static Bool AB_IsMuted(void *callback)
{
	/*no mute on AudioBuffer*/
	return 0;
}

static Bool AB_GetConfig(AudioInterface *aifc)
{
	AudioBufferStack *st = (AudioBufferStack *) Node_GetPrivate( ((AudioInput *) aifc->callback)->owner);

	if (AM_Reconfig(st->am)) {
		if (st->buffer) free(st->buffer);
		st->buffer = NULL;
		st->buffer_size = 0;

		AM_GetMixerConfig(st->am, &aifc->sr, &aifc->chan, &aifc->bps, &aifc->ch_cfg);
		st->is_init = (aifc->sr && aifc->chan && aifc->bps) ? 1 : 0;
		/*this will force invalidation*/
		return 0;
	}
	return st->is_init;
}

void AB_AddSource(AudioGroup *_this, AudioInput *src)
{
	AudioBufferStack *st = (AudioBufferStack *)_this;
	if (!src) return;
	/*just collect the input, reconfig is done once all children are rendered*/
	ChainAddEntry(st->new_inputs, src);
}


void setup_audiobufer(AudioInput *ai, LPSCENERENDER sr, SFNode *node)
{
	memset(ai, 0, sizeof(AudioInput));
	ai->owner = node;
	ai->compositor = sr;
	/*NEVER used for audio buffer*/
	ai->stream = NULL;
	/*setup io interface*/
	ai->input_ifce.FetchFrame = AB_FetchFrame;
	ai->input_ifce.ReleaseFrame = AB_ReleaseFrame;
	ai->input_ifce.GetConfig = AB_GetConfig;
	ai->input_ifce.GetChannelVolume = AB_GetChannelVolume;
	ai->input_ifce.GetSpeed = AB_GetSpeed;
	ai->input_ifce.IsMuted = AB_IsMuted;
	ai->input_ifce.callback = ai;
	ai->speed = 1.0;
}

void InitAudioBuffer(LPSCENERENDER sr, SFNode *node)
{
	AudioBufferStack *st = malloc(sizeof(AudioBufferStack));
	memset(st, 0, sizeof(AudioBufferStack));

	/*use our private input*/
	setup_audiobufer(&st->output, sr, node);
	st->add_source = AB_AddSource;

	st->time_handle.UpdateTimeNode = AB_UpdateTime;
	st->time_handle.obj = node;
	st->set_duration = 1;

	st->am = NewAudioMixer(NULL);
	st->new_inputs = NewChain();

	Node_SetPrivate(node, st);
	Node_SetRenderFunction(node, RenderAudioBuffer);
	Node_SetPreDestroyFunction(node, DestroyAudioBuffer);
	SR_RegisterTimeNode(sr, &st->time_handle);
}


void AudioBufferModified(SFNode *node)
{
	M_AudioBuffer *ab = (M_AudioBuffer *)node;
	AudioBufferStack *st = (AudioBufferStack *) Node_GetPrivate(node);
	if (!st) return;

	//update state if we're active
	if (ab->isActive) 
		AB_UpdateTime(&st->time_handle);

	/*make sure we are still registered*/
	if (!st->time_handle.is_registered && !st->time_handle.needs_unregister) 
		SR_RegisterTimeNode(st->output.compositor, &st->time_handle);
	else
		st->time_handle.needs_unregister = 0;
}

