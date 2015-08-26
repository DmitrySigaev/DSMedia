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

#include <gpac/intern/m4_render_dev.h>

#define MIN_RESYNC_TIME		1000

static char *AI_FetchFrame(void *callback, u32 *size, u32 audio_delay_ms)
{
	u32 obj_time;
	s32 drift;
	AudioInput *ai = (AudioInput *) callback;
	/*even if the stream is signaled as finished we must check it, because it may have been restarted by a mediaControl*/
	if (!ai->stream) return NULL;
	
	ai->need_release = MO_FetchFrame(ai->stream, 0, &ai->stream_finished);
	/*invalidate scene on end of stream to refresh audio graph*/
	if (ai->stream_finished) SR_Invalidate(ai->compositor, NULL);

	/*no more data or not enough data, reset syncro drift*/
	if (!ai->need_release) {
		MO_AdjustClock(ai->stream, 0);
		return NULL;
	}
	*size = ai->stream->current_size;

	MO_GetObjectTime(ai->stream, &obj_time);
	obj_time += audio_delay_ms;
	drift = obj_time - ai->stream->current_ts;

	/*too early (silence insertions), don't render*/
	if (drift + (s32) audio_delay_ms + MIN_RESYNC_TIME < 0) {
		//fprintf(stdout, "audio too early %d\n", drift + audio_delay_ms + MIN_RESYNC_TIME);
		ai->need_release = 0;
		MO_ReleaseFrame(ai->stream, 0, 0);
		return NULL;
	}
	/*adjust drift*/
	if (audio_delay_ms) {
		//fprintf(stdout, "Audio clock: delay %d - obj time %d - CTS %d - adjust drift %d\n", audio_delay_ms, obj_time - audio_delay_ms, ai->stream->current_ts, drift);
		MO_AdjustClock(ai->stream, drift);
	}

	return ai->stream->current_frame;
}

static void AI_ReleaseFrame(void *callback, u32 nb_bytes)
{
	AudioInput *ai = (AudioInput *) callback;
	if (!ai->stream) return;
	MO_ReleaseFrame(ai->stream, nb_bytes, 1);
	ai->need_release = 0;
}

static Float AI_GetSpeed(void *callback)
{
	AudioInput *ai = (AudioInput *) callback;
	return MO_GetSpeed(ai->stream, ai->speed);
}

static Bool AI_GetChannelVolume(void *callback, Float *vol)
{
	AudioInput *ai = (AudioInput *) callback;
	if (ai->snd && ai->snd->GetChannelVolume) {
		return ai->snd->GetChannelVolume(ai->snd->owner, vol);
	} else {
		vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = vol[5] = 1.0f;
		return 0;
	}
}

static Bool AI_IsMuted(void *callback)
{
	AudioInput *ai = (AudioInput *) callback;
	if (!ai->stream) return 1;
	if (ai->is_muted) return 1;
	return MO_IsMuted(ai->stream);
}

static Bool AI_GetConfig(AudioInterface *aifc)
{
	AudioInput *ai = (AudioInput *) aifc->callback;
	if (!ai->stream) return 0;
	/*watchout for object reuse*/
	if (aifc->sr && (ai->stream->mo_flags & MO_IS_INIT)) return 1;

	aifc->sr = ai->stream->sample_rate;
	aifc->chan = ai->stream->num_channels;
	aifc->bps = ai->stream->bits_per_sample;
	aifc->ch_cfg = ai->stream->channel_config;
	if (aifc->sr * aifc->chan * aifc->bps && ((aifc->chan<=2) || aifc->ch_cfg))  {
		ai->stream->mo_flags |= MO_IS_INIT;
		return 1;
	}
	ai->stream->mo_flags &= ~MO_IS_INIT;
	return 0;
}

void setup_audio_input(AudioInput *ai, LPSCENERENDER sr, SFNode *node)
{
	memset(ai, 0, sizeof(AudioInput));
	ai->owner = node;
	ai->compositor = sr;
	ai->stream = NULL;
	/*setup io interface*/
	ai->input_ifce.FetchFrame = AI_FetchFrame;
	ai->input_ifce.ReleaseFrame = AI_ReleaseFrame;
	ai->input_ifce.GetConfig = AI_GetConfig;
	ai->input_ifce.GetChannelVolume = AI_GetChannelVolume;
	ai->input_ifce.GetSpeed = AI_GetSpeed;
	ai->input_ifce.IsMuted = AI_IsMuted;
	ai->input_ifce.callback = ai;

	ai->speed = 1.0;
}



M4Err open_audio(AudioInput *ai, MFURL *url)
{
	if (ai->is_open) return M4BadParam;

	/*get media object*/
	ai->stream = MO_FindObject(ai->owner, url);
	/*bad URL*/
	if (!ai->stream) return M4UnsupportedURL;

	/*store url*/
	VRML_FieldCopy(&ai->url, url, FT_MFURL);

	/*request play*/
	MO_Play(ai->stream);

	ai->stream_finished = 0;
	ai->is_open = 1;
	ai->stream->mo_flags = 0;
	return M4OK;
}

void stop_audio(AudioInput *ai)
{
	if (!ai->is_open) return;
	
	/*we must make sure audio mixer is not using the stream otherwise we may leave it dirty (with unrelease frame)*/
	AM_Lock(ai->compositor->audio_renderer->mixer, 1);

	assert(!ai->need_release);

	MO_Stop(ai->stream);
	VRML_MF_Reset(&ai->url, FT_MFURL);
	ai->is_open = 0;
	ai->stream = NULL;

	AM_Lock(ai->compositor->audio_renderer->mixer, 0);

}

void restart_audio(AudioInput *ai)
{
	if (!ai->is_open) return;
	if (ai->need_release) MO_ReleaseFrame(ai->stream, ai->stream->current_size, 1);
	ai->need_release = 0;
	ai->stream_finished = 0;
	MO_Restart(ai->stream);
}

Bool audio_check_url_changed(AudioInput *ai, MFURL *url)
{
	if (!ai->stream) return url->count;
	return MO_UrlChanged(ai->stream, url);
}

void audio_register_node(AudioInput *ai, BaseRenderEffect *eff)
{
	/*check interface is valid*/
	if (!ai->input_ifce.FetchFrame
		|| !ai->input_ifce.GetChannelVolume
		|| !ai->input_ifce.GetConfig
		|| !ai->input_ifce.GetSpeed
		|| !ai->input_ifce.IsMuted
		|| !ai->input_ifce.ReleaseFrame
		) return;

	if (eff->audio_parent) {
		/*this assume only one parent may use an audio node*/
		if (ai->register_with_parent) return;
		if (ai->register_with_renderer) {
			AR_RemoveSourceNode(ai->compositor->audio_renderer, &ai->input_ifce);
			ai->register_with_renderer = 0;
		}
		eff->audio_parent->add_source(eff->audio_parent, ai);
		ai->register_with_parent = 1;
		ai->snd = eff->sound_holder;
	} else if (!ai->register_with_renderer) {
		
		if (ai->register_with_parent) {
			ai->register_with_parent = 0;
			/*if used in a parent audio group, do a complete traverse to rebuild the group*/
			SR_Invalidate(ai->compositor, NULL);
		}

		AR_AddSourceNode(ai->compositor->audio_renderer, &ai->input_ifce);
		ai->register_with_renderer = 1;
		ai->snd = eff->sound_holder;
	}
}

void audio_unregister_node(AudioInput *ai)
{
	if (ai->register_with_renderer) {
		ai->register_with_renderer = 0;
		AR_RemoveSourceNode(ai->compositor->audio_renderer, &ai->input_ifce);
	} else {
		/*if used in a parent audio group, do a complete traverse to rebuild the group*/
		SR_Invalidate(ai->compositor, NULL);
	}
}


