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

/*
	Notes about the mixer:
	1- spatialization is out of scope for the mixer (eg that's the sound node responsability)
	2- mixing is performed by resampling input source & deinterleaving its channels into dedicated buffer.
	We could directly deinterleave in the main mixer ouput buffer, but this would prevent any future 
	gain correction.
*/
typedef struct
{
	AudioInterface *src;

	s32 *ch_buf[MAX_CHANNELS];
	/*resampled buffer*/
	u32 buffer_size;

	u32 bytes_per_sec;
	
	Bool has_prev;
	s32 last_channels[MAX_CHANNELS];

	u32 in_bytes_used, out_samples_written, out_samples_to_write;

	Float speed;
	Float pan[6];
} MixerInput;

struct __audiomix
{
	/*src*/
	Chain *sources;
	/*output config*/
	u32 sample_rate;
	u32 nb_channels;
	u32 bits_per_sample;
	u32 channel_cfg;
	M4Mutex *mx;
	/*if set forces stereo/mono*/
	Bool force_channel_out;
	/*set to true by mixer when detecting an audio config change*/
	Bool must_reconfig;
	Bool isEmpty;
	/*set to non null if this outputs directly to the driver, in which case audio formats have to be checked*/
	struct _audio_render *ar;

	s32 *output;
	u32 output_size;
};

AudioMixer *NewAudioMixer(struct _audio_render *ar)
{
	AudioMixer *am;
	am = (AudioMixer *) malloc(sizeof(AudioMixer));
	if (!am) return NULL;
	memset(am, 0, sizeof(AudioMixer));
	am->mx = NewMutex();
	am->sources = NewChain();
	am->isEmpty = 1;
	am->ar = ar;
	am->sample_rate = 44100;
	am->bits_per_sample = 16;
	am->nb_channels = 2;
	am->output = NULL;
	am->output_size = 0;
	return am;
}

void DeleteAudioMixer(AudioMixer *am)
{
	DeleteChain(am->sources);
	MX_Delete(am->mx);
	if (am->output) free(am->output);
	free(am);
}

void AM_RemoveAllInputs(AudioMixer *am)
{
	u32 j;
	if (am->isEmpty) return;
	AM_Lock(am, 1);
	while (ChainGetCount(am->sources)) {
		MixerInput *in = ChainGetEntry(am->sources, 0);
		ChainDeleteEntry(am->sources, 0);
		for (j=0; j<MAX_CHANNELS; j++) {
			if (in->ch_buf[j]) free(in->ch_buf[j]);
		}
		free(in);
	}
	AM_Lock(am, 0);
}

Bool AM_IsSourcePresent(AudioMixer *am, AudioInterface *ifce)
{
	u32 i;
	for (i=0; i<ChainGetCount(am->sources); i++) {
		MixerInput *in = ChainGetEntry(am->sources, i);
		if (in->src == ifce) return 1;
	}
	return 0;
}
u32 AM_GetSourceCount(AudioMixer *am)
{
	return ChainGetCount(am->sources);
}

void AM_ForceChannelCount(AudioMixer *am, u32 num_channels)
{
	am->force_channel_out = 1;
	am->nb_channels = num_channels;
}

u32 AM_GetBlockAlign(AudioMixer *am)
{
	return am->nb_channels*am->bits_per_sample/8;
}

void AM_Lock(AudioMixer *am, Bool lockIt)
{
	if (lockIt)
		MX_P(am->mx);
	else
		MX_V(am->mx);
}


void AM_AddSource(AudioMixer *am, AudioInterface *src)
{
	MixerInput *in;
	if (AM_IsSourcePresent(am, src)) return;
	AM_Lock(am, 1);
	SAFEALLOC(in, sizeof(MixerInput));
	in->src = src;
	ChainAddEntry(am->sources, in);
	am->must_reconfig = 1;
	am->isEmpty = 0;
	AM_Lock(am, 0);
}

void AM_RemoveSource(AudioMixer *am, AudioInterface *src)
{
	u32 i, j;
	if (am->isEmpty) return;
	AM_Lock(am, 1);
	for (i=0; i<ChainGetCount(am->sources); i++) {
		MixerInput *in = ChainGetEntry(am->sources, i);
		if (in->src != src) continue;
		ChainDeleteEntry(am->sources, i);
		for (j=0; j<MAX_CHANNELS; j++) {
			if (in->ch_buf[j]) free(in->ch_buf[j]);
		}
		free(in);
		break;
	}
	am->isEmpty = ChainGetCount(am->sources);
	/*we don't ask for reconfig when removing a node*/
	AM_Lock(am, 0);
}


static u32 get_best_samplerate(AudioMixer *am, u32 obj_samplerate, u32 outCH, u32 outBPS)
{
	if (!am->ar) return obj_samplerate;
	if (!am->ar->audio_out || !am->ar->audio_out->QueryOutputSampleRate) return obj_samplerate;
	return am->ar->audio_out->QueryOutputSampleRate(am->ar->audio_out, obj_samplerate, outCH, outBPS);
}

void AM_GetMixerConfig(AudioMixer *am, u32 *outSR, u32 *outCH, u32 *outBPS, u32 *outChCfg)
{
	(*outBPS) = am->bits_per_sample;
	(*outCH) = am->nb_channels;
	(*outSR) = am->sample_rate;
	(*outChCfg) = am->channel_cfg;
}

void AM_SetMixerConfig(AudioMixer *am, u32 outSR, u32 outCH, u32 outBPS, u32 outChCfg)
{
	if ((am->bits_per_sample == outBPS) && (am->nb_channels == outCH) 
		&& (am->sample_rate==outSR) && (am->channel_cfg == outChCfg)) return;

	AM_Lock(am, 1);
	am->bits_per_sample = outBPS;
	if (!am->force_channel_out) am->nb_channels = outCH;
	am->sample_rate = get_best_samplerate(am, outSR, outCH, outBPS);
	if (outCH>2) am->channel_cfg = outChCfg;
	else if (outCH==2) am->channel_cfg = CHANNEL_FRONT_LEFT | CHANNEL_FRONT_RIGHT;
	else am->channel_cfg = CHANNEL_FRONT_LEFT;
	/*if main mixer recfg output*/
	if (am->ar)	am->ar->need_reconfig = 1;
	AM_Lock(am, 0);
}

Bool AM_Reconfig(AudioMixer *am)
{
	u32 i, count, numInit, max_sample_rate, max_channels, max_bps, cfg_changed, ch_cfg;
	if (am->isEmpty || !am->must_reconfig) return 0;

	AM_Lock(am, 1);
	numInit = 0;
	max_sample_rate = am->sample_rate;
	max_channels = am->nb_channels;
	max_bps = am->bits_per_sample;
	cfg_changed = 0;
	ch_cfg = 0;


	count = ChainGetCount(am->sources);
	for (i=0; i<count; i++) {
		Bool has_cfg;
		MixerInput *in = (MixerInput *) ChainGetEntry(am->sources, i);
		has_cfg = in->src->GetConfig(in->src);
		if (in->bytes_per_sec && has_cfg) {
			numInit++;
			continue;
		}
		if (!has_cfg) continue;
		/*update out cfg*/
		if ((count==1) && (max_sample_rate != in->src->sr)) {
			cfg_changed = 1;
			max_sample_rate = in->src->sr;
		} else if (max_sample_rate<in->src->sr) {
			cfg_changed = 1;
			max_sample_rate = in->src->sr;
		}
		if ((count==1) && (max_bps!=in->src->bps)) {
			cfg_changed = 1;
			max_bps = in->src->bps;
		} else if (max_bps<in->src->bps) {
			cfg_changed = 1;
			max_bps = in->src->bps;
		}
		if (!am->force_channel_out) {
			if ((count==1) && (max_channels!=in->src->chan)) {
				cfg_changed = 1;
				max_channels = in->src->chan;
				if (in->src->chan>2) ch_cfg |= in->src->ch_cfg;
			} else if (max_channels < in->src->chan) {
				cfg_changed = 1;
				max_channels = in->src->chan;
				if (in->src->chan>2) ch_cfg |= in->src->ch_cfg;
			}
		}

		numInit++;
		in->bytes_per_sec = in->src->sr * in->src->chan * in->src->bps / 8;
		/*cfg has changed, we must reconfig everything*/
		if (cfg_changed) continue;
		in->has_prev = 0;
		memset(&in->last_channels, 0, sizeof(s16)*MAX_CHANNELS);
	}
	
	if (cfg_changed) {
		if (max_channels>2) {
			if (ch_cfg != am->channel_cfg) {
				/*recompute num channel based on all input channels*/
				max_channels = 0;
				if (ch_cfg & CHANNEL_FRONT_LEFT) max_channels ++;
				if (ch_cfg & CHANNEL_FRONT_RIGHT) max_channels ++;
				if (ch_cfg & CHANNEL_FRONT_CENTER) max_channels ++;
				if (ch_cfg & CHANNEL_LFE) max_channels ++;
				if (ch_cfg & CHANNEL_BACK_LEFT) max_channels ++;
				if (ch_cfg & CHANNEL_BACK_RIGHT) max_channels ++;
				if (ch_cfg & CHANNEL_BACK_CENTER) max_channels ++;
				if (ch_cfg & CHANNEL_SIDE_LEFT) max_channels ++;
				if (ch_cfg & CHANNEL_SIDE_RIGHT) max_channels ++;
			}
		} else {
			ch_cfg = CHANNEL_FRONT_LEFT;
			if (max_channels==2) ch_cfg |= CHANNEL_FRONT_RIGHT;
		}
		AM_SetMixerConfig(am, max_sample_rate, max_channels, max_bps, ch_cfg);
	}
	
	if (numInit == count) am->must_reconfig = 0;

	AM_Lock(am, 0);
	return cfg_changed;
}

static M4INLINE u32 get_channel_out_pos(u32 in_ch, u32 out_cfg)
{
	u32 i, cfg, pos;
	pos = 0;
	for (i=0; i<9; i++) {
		cfg = 1<<(i);
		if (out_cfg & cfg) {
			if (cfg == in_ch) return pos;
			pos++;
		}
	}
	return MAX_CHANNELS;
}

/*this is crude, we'd need a matrix or something*/
static M4INLINE void am_map_channels(s32 *inChan, u32 nb_in, u32 in_cfg, u32 nb_out, u32 out_cfg)
{
	u32 i;
	if (nb_in==1) {
		/*mono to stereo*/
		if (nb_out==2) {
			inChan[1] = inChan[0];
		}
		else if (nb_out>2) {
			/*if center channel use it (we assume we always have stereo channels)*/
			if (out_cfg & CHANNEL_FRONT_CENTER) {
				inChan[2] = inChan[0];
				inChan[0] = 0;
				for (i=3; i<nb_out; i++) inChan[i] = 0;
			} else {
				/*mono to stereo*/
				inChan[1] = inChan[0];
				for (i=2; i<nb_out; i++) inChan[i] = 0;
			}
		}
	} else if (nb_in==2) {
		if (nb_out==1) {
			inChan[0] = (inChan[0]+inChan[1])/2;
		} else {
			for (i=2; i<nb_out; i++) inChan[i] = 0;
		}
	}
	/*same output than input channels, nothing to reorder*/

	/*more output than input channels*/
	else if (nb_in<nb_out) {
		s32 bckup[MAX_CHANNELS];
		u32 pos;
		u32 cfg = in_cfg;
		u32 ch = 0;
		memcpy(bckup, inChan, sizeof(s32)*nb_in);
		for (i=0; i<nb_in; i++) {
			/*get first in channel*/
			while (! (cfg & 1)) {
				ch++;
				cfg>>=1;
				/*done*/
				if (ch==10) return;
			}
			pos = get_channel_out_pos((1<<ch), out_cfg);
			assert(pos != MAX_CHANNELS);
			inChan[pos] = bckup[i];
			ch++;
			cfg>>=1;
		}
		for (i=nb_in; i<nb_out; i++) inChan[i] = 0;
	}
	/*less output than input channels (eg sound card doesn't support requested format*/
	else if (nb_in>nb_out) {
		s32 bckup[MAX_CHANNELS];
		u32 pos;
		u32 cfg = in_cfg;
		u32 ch = 0;
		memcpy(bckup, inChan, sizeof(s32)*nb_in);
		for (i=0; i<nb_in; i++) {
			/*get first in channel*/
			while (! (cfg & 1)) {
				ch++;
				cfg>>=1;
				/*done*/
				if (ch==10) return;
			}
			pos = get_channel_out_pos( (1<<ch), out_cfg);
			/*this channel is present in output, copy over*/
			if (pos < MAX_CHANNELS) {
				inChan[pos] = bckup[i];
			} else {
				/*map to stereo (we assume that the driver cannot handle ANY multichannel cfg)*/
				switch (1<<ch) {
				case CHANNEL_FRONT_CENTER:
				case CHANNEL_LFE:
				case CHANNEL_BACK_CENTER:
					inChan[0] += bckup[i]/2;
					inChan[1] += bckup[i]/2;
					break;
				case CHANNEL_BACK_LEFT:
				case CHANNEL_SIDE_LEFT:
					inChan[0] += bckup[i];
					break;
				case CHANNEL_BACK_RIGHT:
				case CHANNEL_SIDE_RIGHT:
					inChan[1] += bckup[i];
					break;
				}
			}
			ch++;
			cfg>>=1;
		}
	}
}


static void AM_GetInputData(AudioMixer *am, MixerInput *in, u32 audio_delay)
{
	u32 i, j, in_ch, out_ch, prev, next, src_samp, ratio, src_size;
	Bool use_prev;
	s16 *in_s16;
	s8 *in_s8;
	s32 frac, inChan[MAX_CHANNELS], inChanNext[MAX_CHANNELS];

	in_s8 = (s8 *) in->src->FetchFrame(in->src->callback, &src_size, audio_delay);
	if (!in_s8) {
		in->has_prev = 0;
		/*done, stop fill*/
		in->out_samples_to_write = 0;
		return;
	}

	ratio = (u32) (255*in->src->sr * in->speed / am->sample_rate);
	src_samp = (u32) (src_size * 8 / in->src->bps / in->src->chan);
	in_ch = in->src->chan;
	out_ch = am->nb_channels;
	if (in->src->bps == 8) {
		in_s16 = NULL;
	} else {
		in_s16 = (s16 *) in_s8;
		in_s8 = NULL;
	}

	/*just in case, if only 1 sample available in src, copy over and discard frame since we cannot 
	interpolate audio*/
	if (src_samp==1) {
		in->has_prev = 1;
		for (j=0; j<in_ch; j++) in->last_channels[j] = in_s16 ? in_s16[j] : in_s8[j];
		in->in_bytes_used = src_size;
		return;
	}

	/*while space to fill and input data, convert*/
	use_prev = in->has_prev;
	i = 0;
	prev = 0;
	while (1) {
		prev = (u32) (i*ratio) / 255;
		if (prev>=src_samp) break;

		next = prev+1;
		frac = (i*ratio) - 255*prev;
		if (frac && (next==src_samp)) break;
		if (use_prev && prev) use_prev = 0;

		if (in_s16) {
			for (j=0; j<in_ch; j++) {
				inChan[j] = use_prev ? in->last_channels[j] : in_s16[in_ch*prev + j];
				inChanNext[j] = in_s16[in_ch*next + j];
				inChan[j] = (frac*inChanNext[j] + (255-frac)*inChan[j]) / 255;
			}
		} else {
			for (j=0; j<in_ch; j++) {
				inChan[j] = use_prev ? in->last_channels[j] : in_s8[in_ch*prev + j];
				inChanNext[j] = in_s8[in_ch*next + j];
				inChan[j] = (frac*inChanNext[j] + (255-frac)*inChan[j]) / 255;
			}
		}

		am_map_channels(inChan, in_ch, in->src->ch_cfg, out_ch, am->channel_cfg);

		for (j=0; j<out_ch ; j++) {
			* (in->ch_buf[j] + in->out_samples_written) = (s32) (inChan[j] * in->pan[j]);
		}
		in->out_samples_written ++;
		if (in->out_samples_written == in->out_samples_to_write) break;
		i++;
	}
	if (ratio==255) {
		in->has_prev = 0;
		if (next==src_samp) {
			in->in_bytes_used = src_size;
		} else {
			in->in_bytes_used = MIN(src_size, prev*in->src->bps * in->src->chan / 8);
		}
	} else {
		in->has_prev = 1;
		if (next==src_samp) {
			for (j=0; j<in_ch; j++) in->last_channels[j] = inChanNext[j];
			in->in_bytes_used = src_size;
		} else {
			in->in_bytes_used = prev*in->src->bps * in->src->chan / 8;
			if (in->in_bytes_used>src_size) {
				in->in_bytes_used = src_size;
				for (j=0; j<in_ch; j++) in->last_channels[j] = inChanNext[j];
			} else {
				for (j=0; j<in_ch; j++) in->last_channels[j] = in_s16 ? in_s16[in_ch*prev + j] : in_s8[in_ch*prev + j];
			}
		}
	}
	/*cf below, make sure we call release*/
	in->in_bytes_used += 1;
}


u32 AM_GetMix(AudioMixer *am, void *buffer, u32 buffer_size)
{
	MixerInput *in, *single_source;
	Float pan[6];
	Bool is_muted;
	u32 i, j, count, size, in_size, nb_samples, delay, nb_written;
	s32 *out_mix, nb_act_src;
	char *data, *ptr;

	/*the config has changed we don't write to output since settings change*/
	if (AM_Reconfig(am)) return 0;

	AM_Lock(am, 1);
	count = ChainGetCount(am->sources);
	if (!count) {
		memset(buffer, 0, buffer_size);
		AM_Lock(am, 0);
		return 0;
	}
	delay = 0;
	if (am->ar && am->ar->resync_clocks) delay = am->ar->audio_delay;

	single_source = NULL;
	if (count!=1) goto do_mix;
	if (am->force_channel_out) goto do_mix;
	single_source = (MixerInput *) ChainGetEntry(am->sources, 0);
	/*if cfg changed or unknown return*/
	if (!single_source->src->GetConfig(single_source->src)) {
		AM_Reconfig(am);
		memset(buffer, 0, buffer_size);
		AM_Lock(am, 0);
		return 0;
	}
	/*this happens if input SR cannot be mapped to output audio hardware*/
	if (single_source->src->sr != am->sample_rate) goto do_mix;
	/*note we don't check output cfg: if the number of channel is the same then the channel cfg is the 
	same*/
	if (single_source->src->chan != am->nb_channels) goto do_mix;
	if (single_source->src->GetSpeed(single_source->src->callback)!=1.0) goto do_mix;
	if (single_source->src->GetChannelVolume(single_source->src->callback, pan)) goto do_mix;


single_source_mix:

	ptr = (char *)buffer;
	in_size = buffer_size;
	is_muted = single_source->src->IsMuted(single_source->src->callback);

	while (buffer_size) {
		data = single_source->src->FetchFrame(single_source->src->callback, &size, delay);
		if (!data || !size) break;
		/*don't copy more than possible*/
		if (size > buffer_size) size = buffer_size;
		if (is_muted) {
			memset(ptr, 0, size);
		} else {
			memcpy(ptr, data, size);
		}
		buffer_size -= size;
		ptr += size;
		single_source->src->ReleaseFrame(single_source->src->callback, size);
		delay = 0;
	}

	/*not completely filled*/
	if (buffer_size) {
		//if (!data) fprintf(stdout, "NOT ENOUGH INPUT DATA %d remain\n", buffer_size);
		memset(ptr, 0, buffer_size);
	}

	AM_Lock(am, 0);
	return (in_size - buffer_size);

do_mix:
	nb_act_src = 0;
	nb_samples = buffer_size / (am->nb_channels * am->bits_per_sample / 8);

	/*step 1, cfg*/
	if (am->output_size<buffer_size) {
		if (am->output) free(am->output);
		am->output = malloc(sizeof(s32) * buffer_size);
		am->output_size = buffer_size;
	}

	single_source = NULL;
	for (i=0; i<count; i++) {
		in = ChainGetEntry(am->sources, i);
		if (in->buffer_size<buffer_size) { 
			for (j=0; j<MAX_CHANNELS; j++) {
				if (in->ch_buf[j]) free(in->ch_buf[j]); 
				SAFEALLOC(in->ch_buf[j], sizeof(u32) * buffer_size);
			}
			in->buffer_size = buffer_size; 
		}
		in->speed = in->src->GetSpeed(in->src->callback);
		if (in->speed<0) in->speed *= -1;
		
		in->out_samples_written = 0;
		in->in_bytes_used = 0;

		/*if cfg unknown or changed (AudioBuffer child...) reconfig*/
		if (!in->src->GetConfig(in->src)) {
			nb_act_src = 0;
			AM_Reconfig(am);
			break;
		} else if (in->speed==0) {
			in->out_samples_to_write = 0;
		} else {
			assert(in->src->sr);
			in->out_samples_to_write = nb_samples;
			if (in->src->IsMuted(in->src->callback)) {
				memset(in->pan, 0, sizeof(Float)*6);
			} else {
				if (!in->src->GetChannelVolume(in->src->callback, in->pan)) {
					/*track first active source with same cfg as mixer*/
					if (!single_source && (in->src->sr == am->sample_rate) 
						&& (in->src->chan == am->nb_channels) && (in->speed == 1.0) 
					) 
						single_source = in;
				}
			}
			nb_act_src ++;
		}
	}
	if (!nb_act_src) {
		memset(buffer, 0, sizeof(char)*buffer_size);
		AM_Lock(am, 0);
		return 0;
	}

	/*if only one active source in native format, process as single source (direct copy)
	this is needed because mediaControl on an audio object doesn't deactivate it (eg the audio
	object is still present in the mixer). this opt is typically usefull for language selection
	content (cf mp4menu)*/
	if ((nb_act_src==1) && single_source) goto single_source_mix;

	/*step 2, fill all buffers*/
	while (nb_act_src) {
		u32 nb_to_fill = 0;
		/*fill*/
		for (i=0; i<count; i++) {
			in = ChainGetEntry(am->sources, i);
			if (in->out_samples_to_write>in->out_samples_written) {
				AM_GetInputData(am, in, in->out_samples_written ? 0 : delay);
				if (in->out_samples_to_write>in->out_samples_written) nb_to_fill++;
			}
		}
		/*release - this is done in 2 steps in case 2 audio object use the same source...*/
		for (i=0; i<count; i++) {
			in = ChainGetEntry(am->sources, i);
			if (in->in_bytes_used) in->src->ReleaseFrame(in->src->callback, in->in_bytes_used-1);
			in->in_bytes_used = 0;
		}
		if (!nb_to_fill) break;
	}
	/*step 3, mix the final buffer*/
	memset(am->output, 0, sizeof(s32) * buffer_size);

	nb_written = 0;
	for (i=0; i<count; i++) {
		out_mix = am->output;
		in = ChainGetEntry(am->sources, i);
		if (!in->out_samples_to_write) continue;
		/*only write what has been filled in the source buffer (may be less than output size)*/
		for (j=0; j<in->out_samples_written; j++) {
			u32 k;
			for (k=0; k<am->nb_channels; k++) {
				(*out_mix) += * (in->ch_buf[k] + j);
				out_mix += 1;
			}
			if (nb_written < in->out_samples_written) nb_written = in->out_samples_written;
		}
	}

	out_mix = am->output;
	if (am->bits_per_sample==16) {
		s16 *out_s16 = (s16 *) buffer;
		for (i=0; i<nb_written; i++) {
			for (j=0; j<am->nb_channels; j++) {
				s32 samp = (*out_mix / nb_act_src);
				if (samp > M4_SHORT_MAX) samp = M4_SHORT_MAX;
				else if (samp < M4_SHORT_MIN) samp = M4_SHORT_MIN;
				(*out_s16) = samp;
				out_s16 += 1;
				out_mix += 1;
			}
		}
	} else {
		s8 *out_s8 = (s8 *) buffer;
		for (i=0; i<nb_written; i++) {
			for (j=0; j<am->nb_channels; j++) {
				s32 samp = (*out_mix / nb_act_src);
				if (samp > 127) samp = 127;
				else if (samp < -128) samp = -128;
				(*out_s8) = samp;
				out_s8 += 1;
				out_mix += 1;
			}
		}
	}

	nb_written *= am->nb_channels*am->bits_per_sample/8;
	AM_Lock(am, 0);
	return nb_written;
}


