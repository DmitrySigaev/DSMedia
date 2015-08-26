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


void AR_FillBuffer(void *ar, char *buffer, u32 buffer_size);
u32 AR_MainLoop(void *ar);

M4Err AR_SetupAudioFormat(AudioRenderer *ar, AudioOutput *dr)
{
	M4Err e;
	u32 freq, nb_bits, nb_chan, BPS, ch_cfg;
	AM_GetMixerConfig(ar->mixer, &freq, &nb_chan, &nb_bits, &ch_cfg);

	/*we try to set the FPS so that SR*2/FPS is an integer, and FPS the smallest int >= 30 fps */
	BPS = freq * nb_chan * nb_bits / 8;

	e = dr->ConfigureOutput(dr, &freq, &nb_chan, &nb_bits, ch_cfg);
	if (e) {
		if (nb_chan>2) {
			nb_chan=2;
			e = dr->ConfigureOutput(dr, &freq, &nb_chan, &nb_bits, ch_cfg);
		}
		if (e) return e;
	}

	AM_SetMixerConfig(ar->mixer, freq, nb_chan, nb_bits, ch_cfg);
	ar->audio_delay = ar->audio_out->GetAudioDelay(ar->audio_out);
	return M4OK;
}

AudioRenderer *LoadAudioRenderer(M4User *client)
{
	char *sOpt;
	u32 i, count;
	M4Err e;
	AudioRenderer *ar;
	ar = (AudioRenderer *) malloc(sizeof(AudioRenderer));
	memset(ar, 0, sizeof(AudioRenderer));

	ar->force_cfg = 0;
	ar->num_buffers = ar->num_buffer_per_sec = 0;
	sOpt = IF_GetKey(client->config, "Audio", "ForceConfig");
	if (sOpt && !stricmp(sOpt, "yes")) {
		ar->force_cfg = 1;
		sOpt = IF_GetKey(client->config, "Audio", "NumBuffers");
		ar->num_buffers = sOpt ? atoi(sOpt) : 6;
		sOpt = IF_GetKey(client->config, "Audio", "BuffersPerSecond");
		ar->num_buffer_per_sec = sOpt ? atoi(sOpt) : 15;
	}

	ar->resync_clocks = 1;
	sOpt = IF_GetKey(client->config, "Audio", "NoResync");
	if (sOpt && !stricmp(sOpt, "yes")) ar->resync_clocks = 0;
	
	ar->mixer = NewAudioMixer(ar);
	ar->client = client;

	/*get a prefered renderer*/
	sOpt = IF_GetKey(client->config, "Audio", "DriverName");
	if (sOpt) {
		if (!PM_LoadInterfaceByName(client->plugins, sOpt, M4_AUDIO_OUTPUT_INTERFACE, (void **) &ar->audio_out)) {
			ar->audio_out = NULL;
			sOpt = NULL;
		}
	}
	if (!ar->audio_out) {
		count = PM_GetPluginsCount(ar->client->plugins);
		for (i=0; i<count; i++) {
			if (!PM_LoadInterface(ar->client->plugins, i, M4_AUDIO_OUTPUT_INTERFACE, (void **) &ar->audio_out)) continue;

			/*check that's a valid audio renderer*/
			if (ar->audio_out->SelfThreaded) {
				if (ar->audio_out->SetPriority) break;
			} else {
				if (ar->audio_out->WriteAudio) break;
			}
			PM_ShutdownInterface(ar->audio_out);
			ar->audio_out = NULL;
		}
	}

	/*if not init we run with a NULL audio renderer*/
	if (ar->audio_out) {
		ar->audio_out->FillBuffer = AR_FillBuffer;
		ar->audio_out->audio_renderer = ar;
		e = ar->audio_out->SetupHardware(ar->audio_out, ar->client->os_window_handler, ar->num_buffers, ar->num_buffer_per_sec);
		if (e==M4OK) e = AR_SetupAudioFormat(ar, ar->audio_out);
		if (e != M4OK) {
			PM_ShutdownInterface(ar->audio_out);
			ar->audio_out = NULL;
		} else {
			/*remember the plugin we use*/
			IF_SetKey(client->config, "Audio", "DriverName", ar->audio_out->plugin_name);
			if (!ar->audio_out->SelfThreaded) {
				ar->th = NewThread();
				ar->audio_th_state = 1;
				TH_Run(ar->th, AR_MainLoop, ar);
			}
			ar->audio_out->SetPriority(ar->audio_out, TH_PRIOR_REALTIME);
		}
	}
	if (!ar->audio_out) IF_SetKey(client->config, "Audio", "DriverName", "No Audio Output Available");

	M4_InitClock();
	/*init renderer timer*/
	ar->startTime = M4_GetSysClock();
	return ar;
}

void AR_Delete(AudioRenderer *ar)
{
	if (!ar) return;

	/*resume if paused (might cause deadlock otherwise)*/
	if (ar->Frozen) AR_Pause(ar, 0, 0);
	/*stop and shutdown*/
	if (ar->audio_out) {
		/*kill audio thread*/
		if (!ar->audio_out->SelfThreaded) {
			ar->audio_th_state = 2;
			while (ar->audio_th_state != 3) Sleep(10);
			TH_Delete(ar->th);
		}
		/*lock access before shutdown and emulate a reconfig (avoids mixer lock from self-threaded plugins)*/
		ar->need_reconfig = 1;
		AM_Lock(ar->mixer, 1);
		ar->audio_out->Shutdown(ar->audio_out);
		PM_ShutdownInterface(ar->audio_out);
		AM_Lock(ar->mixer, 0);
	}


	DeleteAudioMixer(ar->mixer);
	free(ar);

	/*shutdown OS timer system*/
	M4_StopClock();
}


void AR_ResetSources(AudioRenderer *ar)
{
	AM_RemoveAllInputs(ar->mixer);
}

void AR_FreezeIntern(AudioRenderer *ar, Bool DoFreeze, Bool for_reconfig)
{
	AM_Lock(ar->mixer, 1);
	if (DoFreeze) {
		if (!ar->Frozen) {
			ar->FreezeTime = M4_GetSysClock();
			if (!for_reconfig && ar->audio_out && ar->audio_out->Pause) ar->audio_out->Pause(ar->audio_out, 1);
			ar->Frozen = 1;
		}
	} else {
		if (ar->Frozen) {
			if (!for_reconfig && ar->audio_out && ar->audio_out->Pause) ar->audio_out->Pause(ar->audio_out, 0);
			ar->Frozen = 0;
			ar->startTime += M4_GetSysClock() - ar->FreezeTime;
		}
	}
	AM_Lock(ar->mixer, 0);
}

void AR_Pause(AudioRenderer *ar, Bool DoFreeze, Bool step_mode)
{
	AR_FreezeIntern(ar, DoFreeze, 0);
}

void AR_SetVolume(AudioRenderer *ar, u32 Volume)
{
	AM_Lock(ar->mixer, 1);
	if (ar && ar->audio_out) ar->audio_out->SetVolume(ar->audio_out, Volume);
	AM_Lock(ar->mixer, 0);
}
void AR_SetPan(AudioRenderer *ar, u32 Balance)
{
	AM_Lock(ar->mixer, 1);
	if (ar && ar->audio_out) ar->audio_out->SetPan(ar->audio_out, Balance);
	AM_Lock(ar->mixer, 0);
}


void AR_AddSourceNode(AudioRenderer *ar, AudioInterface *source)
{
	Bool recfg;
	if (!ar) return;
	/*lock mixer*/
	AM_Lock(ar->mixer, 1);
	AM_AddSource(ar->mixer, source);
	/*if changed reconfig*/
	recfg = AM_Reconfig(ar->mixer);
	if (!ar->need_reconfig) ar->need_reconfig = recfg;

	/*unlock mixer*/
	AM_Lock(ar->mixer, 0);
}

void AR_RemoveSourceNode(AudioRenderer *ar, AudioInterface *source)
{
	if (ar) AM_RemoveSource(ar->mixer, source);
}


void AR_SetPriority(AudioRenderer *ar, u32 priority)
{
	ar->priority = priority;
	if (ar->audio_out && ar->audio_out->SelfThreaded) {
		ar->audio_out->SetPriority(ar->audio_out, priority);
	} else {
		TH_SetPriority(ar->th, priority);
	}
}

u32 AR_MainLoop(void *p)
{
	AudioRenderer *ar = (AudioRenderer *) p;

	ar->audio_th_state = 1;
	while (ar->audio_th_state == 1) {
		AM_Lock(ar->mixer, 1);
		if (ar->Frozen) {
			AM_Lock(ar->mixer, 0);
			Sleep(10);
		} else {
			ar->audio_out->WriteAudio(ar->audio_out);
			AM_Lock(ar->mixer, 0);
		}
	}
	ar->audio_th_state = 3;
	return 0;
}

void AR_FillBuffer(void *ptr, char *buffer, u32 buffer_size)
{
	AudioRenderer *ar = (AudioRenderer *) ptr;
	if (!ar->need_reconfig) AM_GetMix(ar->mixer, buffer, buffer_size);
}

void AR_Reconfig(AudioRenderer *ar)
{
	if (!ar->need_reconfig || !ar->audio_out) return;
	/*lock mixer*/
	AM_Lock(ar->mixer, 1);

	AR_FreezeIntern(ar, 1, 1);

	AR_SetupAudioFormat(ar, ar->audio_out);

	AR_FreezeIntern(ar, 0, 1);
	
	ar->need_reconfig = 0;
	/*unlock mixer*/
	AM_Lock(ar->mixer, 0);
}

u32 AR_GetAudioDelay(AudioRenderer *ar)
{
	return ar->audio_out->GetAudioDelay(ar->audio_out);
}


u32 AR_GetTime(AudioRenderer *ar)
{
	if (ar->Frozen) return ar->FreezeTime - ar->startTime;
	return M4_GetSysClock() - ar->startTime;
}
