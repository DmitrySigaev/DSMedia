/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / DirectX audio and video render plugin
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


#include "dx_hw.h"

#include <process.h>

/*for channel codes*/
#include <gpac/m4_decoder.h>

#if (DIRECTSOUND_VERSION >= 0x0800)
#define USE_WAVE_EX
#endif

#ifdef USE_WAVE_EX
#include <ks.h>
#include <ksmedia.h>
const static GUID  GPAC_KSDATAFORMAT_SUBTYPE_PCM = {0x00000001,0x0000,0x0010,
{0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71}};
#endif



/*
		DirectSound audio output
*/
#define MAX_NUM_BUFFERS		20

typedef struct 
{
	Bool force_config;
	u32 cfg_num_buffers, cfg_num_buffer_per_sec;

	HWND hWnd;
	LPDIRECTSOUND pDS;
	WAVEFORMATEX format;
	IDirectSoundBuffer *pOutput;

	u32 buffer_size, num_audio_buffer, total_audio_buffer_ms;
	
	/*notifs*/
	Bool use_notif;
    u32 frame_state[MAX_NUM_BUFFERS];
    DSBPOSITIONNOTIFY notif_events[MAX_NUM_BUFFERS];
    HANDLE events[MAX_NUM_BUFFERS];
} DSContext;

#define DSCONTEXT()		DSContext *ctx = (DSContext *)dr->opaque;

void DS_WriteAudio(AudioOutput *dr);
void DS_WriteAudio_Notifs(AudioOutput *dr);

static M4Err DS_SetupHardware(AudioOutput *dr, void *os_handle, u32 num_buffers, u32 num_buffer_per_sec)
{
	DWORD flags;
    HRESULT hr;

	DSCONTEXT();
	ctx->hWnd = (HWND) os_handle;
	/*check if we have created a HWND (this requires that video is handled by the DX plugin*/
	if (!ctx->hWnd) ctx->hWnd = DD_GetGlobalHWND();
	/*too bad, use desktop as window*/
	if (!ctx->hWnd) ctx->hWnd = GetDesktopWindow();

	ctx->force_config = (num_buffers && num_buffer_per_sec) ? 1 : 0;
	ctx->cfg_num_buffers = num_buffers;
	ctx->cfg_num_buffer_per_sec = num_buffer_per_sec;
	if (ctx->cfg_num_buffers <= 1) ctx->cfg_num_buffers = 2;

	if ( FAILED( hr = DirectSoundCreate( NULL, &ctx->pDS, NULL ) ) ) return M4IOErr;
	flags = DSSCL_EXCLUSIVE;
	if( FAILED( hr = IDirectSound_SetCooperativeLevel(ctx->pDS, ctx->hWnd, DSSCL_EXCLUSIVE) ) ) {
		SAFE_DS_RELEASE( ctx->pDS ); 
		return M4IOErr;
	}
	return M4OK;
}


void DS_ResetBuffer(DSContext *ctx)
{
    VOID *pLock = NULL;
    DWORD size;

    if( FAILED(IDirectSoundBuffer_Lock(ctx->pOutput, 0, ctx->buffer_size*ctx->num_audio_buffer, &pLock, &size, NULL, NULL, 0 ) ) )
        return;
	memset(pLock, 0, (size_t) size);
	IDirectSoundBuffer_Unlock(ctx->pOutput, pLock, size, NULL, 0L); 
}

void DS_ReleaseBuffer(AudioOutput *dr)
{
	u32 i;
	DSCONTEXT();

	/*stop playing and notif proc*/
	if (ctx->pOutput) IDirectSoundBuffer_Stop(ctx->pOutput);
	SAFE_DS_RELEASE(ctx->pOutput);
	
	/*use notif, shutdown notifier and event watcher*/
	if (ctx->use_notif) {
		for (i=0; i<ctx->num_audio_buffer; i++) CloseHandle(ctx->events[i]);
	}
	ctx->use_notif = 0;
}

static void DS_Shutdown(AudioOutput *dr)
{
	DSCONTEXT();
	DS_ReleaseBuffer(dr);
	SAFE_DS_RELEASE(ctx->pDS ); 
}

/*we assume what was asked is what we got*/
static M4Err DS_ConfigureOutput(AudioOutput *dr, u32 *SampleRate, u32 *NbChannels, u32 *nbBitsPerSample, u32 channel_cfg)
{
    u32 i;
	HRESULT hr; 
	char *sOpt;
	DSBUFFERDESC dsbBufferDesc;
	IDirectSoundNotify *pNotify;
#ifdef USE_WAVE_EX
	WAVEFORMATEXTENSIBLE format_ex;
#endif
	DSCONTEXT();

	DS_ReleaseBuffer(dr);

	ctx->format.nChannels = *NbChannels;
	ctx->format.wBitsPerSample = *nbBitsPerSample;
	ctx->format.nSamplesPerSec = *SampleRate;
	ctx->format.cbSize = sizeof (WAVEFORMATEX);
	ctx->format.wFormatTag = WAVE_FORMAT_PCM;
	ctx->format.nBlockAlign = ctx->format.nChannels * ctx->format.wBitsPerSample / 8;
	ctx->format.nAvgBytesPerSec = ctx->format.nSamplesPerSec * ctx->format.nBlockAlign;

	if (!ctx->force_config) {
		ctx->buffer_size = ctx->format.nBlockAlign * 2048;
		ctx->num_audio_buffer = 8;
	} else {
		ctx->num_audio_buffer = ctx->cfg_num_buffers;
		ctx->buffer_size = ctx->format.nAvgBytesPerSec / ctx->cfg_num_buffer_per_sec;
	}

	/*make sure we're aligned*/
	while (ctx->buffer_size % ctx->format.nBlockAlign) ctx->buffer_size++;

	ctx->use_notif = 1;
	sOpt = PMI_GetOpt(dr, "Audio", "DisableNotification");
	if (sOpt && !stricmp(sOpt, "yes")) ctx->use_notif = 0;

	memset(&dsbBufferDesc, 0, sizeof(DSBUFFERDESC));
	dsbBufferDesc.dwSize = sizeof (DSBUFFERDESC);
	dsbBufferDesc.dwBufferBytes = ctx->buffer_size * ctx->num_audio_buffer;
	dsbBufferDesc.lpwfxFormat = &ctx->format;
	dsbBufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
	if (ctx->use_notif) dsbBufferDesc.dwFlags |= DSBCAPS_CTRLPOSITIONNOTIFY;

#ifdef USE_WAVE_EX
	if (channel_cfg && ctx->format.nChannels>2) {
		memset(&format_ex, 0, sizeof(WAVEFORMATEXTENSIBLE));
		format_ex.Format = ctx->format;
		format_ex.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);
		format_ex.SubFormat = GPAC_KSDATAFORMAT_SUBTYPE_PCM;
		format_ex.Samples.wValidBitsPerSample = *nbBitsPerSample;
		format_ex.dwChannelMask = 0;
		if (channel_cfg & CHANNEL_FRONT_LEFT) format_ex.dwChannelMask |= SPEAKER_FRONT_LEFT;
		if (channel_cfg & CHANNEL_FRONT_RIGHT) format_ex.dwChannelMask |= SPEAKER_FRONT_RIGHT;
		if (channel_cfg & CHANNEL_FRONT_CENTER) format_ex.dwChannelMask |= SPEAKER_FRONT_CENTER;
		if (channel_cfg & CHANNEL_LFE) format_ex.dwChannelMask |= SPEAKER_LOW_FREQUENCY;
		if (channel_cfg & CHANNEL_BACK_LEFT) format_ex.dwChannelMask |= SPEAKER_BACK_LEFT;
		if (channel_cfg & CHANNEL_BACK_RIGHT) format_ex.dwChannelMask |= SPEAKER_BACK_RIGHT;
		if (channel_cfg & CHANNEL_BACK_CENTER) format_ex.dwChannelMask |= SPEAKER_BACK_CENTER;
		if (channel_cfg & CHANNEL_SIDE_LEFT) format_ex.dwChannelMask |= SPEAKER_SIDE_LEFT;
		if (channel_cfg & CHANNEL_SIDE_RIGHT) format_ex.dwChannelMask |= SPEAKER_SIDE_RIGHT;
		dsbBufferDesc.lpwfxFormat = (WAVEFORMATEX *) &format_ex;
	}
#endif


	hr = IDirectSound_CreateSoundBuffer(ctx->pDS, &dsbBufferDesc, &ctx->pOutput, NULL );
	if (FAILED(hr)) {
retry:
		if (ctx->use_notif) PMI_SetOpt(dr, "Audio", "DisableNotification", "yes");
		ctx->use_notif = 0;
		dsbBufferDesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
		hr = IDirectSound_CreateSoundBuffer(ctx->pDS, &dsbBufferDesc, &ctx->pOutput, NULL );
		if (FAILED(hr)) return M4IOErr;
	}

	for (i=0; i<ctx->num_audio_buffer; i++) ctx->frame_state[i] = 0;

	if (ctx->use_notif) {
		hr = IDirectSoundBuffer_QueryInterface(ctx->pOutput, &IID_IDirectSoundNotify , (void **)&pNotify);
		if (hr == S_OK) {
			/*setup the notification positions*/
			for (i=0; i<ctx->num_audio_buffer; i++) {
				ctx->events[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
				ctx->notif_events[i].hEventNotify = ctx->events[i];
				ctx->notif_events[i].dwOffset = ctx->buffer_size * i;
			}

			/*Tell DirectSound when to notify us*/
			hr = IDirectSoundNotify_SetNotificationPositions(pNotify, ctx->num_audio_buffer, ctx->notif_events);

			if (hr != S_OK) {
				IDirectSoundNotify_Release(pNotify);
				for (i=0; i<ctx->num_audio_buffer; i++) CloseHandle(ctx->events[i]);
				SAFE_DS_RELEASE(ctx->pOutput);
				goto retry;
			}

			IDirectSoundNotify_Release(pNotify);
		} else {
			ctx->use_notif = 0;
		}
	}
	if (ctx->use_notif) {
		dr->WriteAudio = DS_WriteAudio_Notifs;
	} else {
		dr->WriteAudio = DS_WriteAudio;
	}

	ctx->total_audio_buffer_ms = 1000 * ctx->buffer_size * ctx->num_audio_buffer / ctx->format.nAvgBytesPerSec;

	/*reset*/
	DS_ResetBuffer(ctx);
	/*play*/
	IDirectSoundBuffer_Play(ctx->pOutput, 0, 0, DSBPLAY_LOOPING );	
	return M4OK;
}

static Bool DS_RestoreBuffer(LPDIRECTSOUNDBUFFER pDSBuffer)
{
	DWORD dwStatus;
    IDirectSoundBuffer_GetStatus(pDSBuffer, &dwStatus );
    if( dwStatus & DSBSTATUS_BUFFERLOST ) {
		fprintf(stdout, "DS buffer lost\n");
		IDirectSoundBuffer_Restore(pDSBuffer);
	    IDirectSoundBuffer_GetStatus(pDSBuffer, &dwStatus);
		if( dwStatus & DSBSTATUS_BUFFERLOST ) return 1;
    }
	return 0;
}



void DS_FillBuffer(AudioOutput *dr, u32 buffer) 
{
	HRESULT hr;
    VOID *pLock;
	u32 pos;
    DWORD size;
	DSCONTEXT();

	/*restoring*/
    if (DS_RestoreBuffer(ctx->pOutput)) {
		fprintf(stdout, "restoring DS buffer\n");
		return;
	}
	
	/*lock and fill from current pos*/
	pos = buffer * ctx->buffer_size;
	pLock = NULL;
    if( FAILED( hr = IDirectSoundBuffer_Lock(ctx->pOutput, pos, ctx->buffer_size,  
			&pLock,  &size, NULL, NULL, 0L ) ) ) {
		fprintf(stdout, "Error locking DS buffer\n");
        return;
	}

	assert(size == ctx->buffer_size);

	dr->FillBuffer(dr->audio_renderer, pLock, size);

	/*update current pos*/
    if( FAILED( hr = IDirectSoundBuffer_Unlock(ctx->pOutput, pLock, size, NULL, 0)) ) {
		fprintf(stdout, "Error unlocking DS buffer\n");
	}
	ctx->frame_state[buffer] = 1;
}


void DS_WriteAudio_Notifs(AudioOutput *dr)
{
	s32 i, inframe, nextframe;
	DSCONTEXT();

	inframe = WaitForMultipleObjects(ctx->num_audio_buffer, ctx->events, 0, INFINITE) - WAIT_OBJECT_0;
	/*reset state*/
	ctx->frame_state[ (inframe + ctx->num_audio_buffer - 1) % ctx->num_audio_buffer] = 0;

	nextframe = (inframe + 1) % ctx->num_audio_buffer;
	for (i=nextframe; (i % ctx->num_audio_buffer) != (u32) inframe; i++) {
		u32 buf = i % ctx->num_audio_buffer;
		if (ctx->frame_state[buf]) continue;
		DS_FillBuffer(dr, buf);
	}
}

void DS_WriteAudio(AudioOutput *dr)
{
    DWORD in_play, cur_play;
	DSCONTEXT();

	/*wait for end of current play buffer*/
	if (IDirectSoundBuffer_GetCurrentPosition(ctx->pOutput, &in_play, NULL) != DS_OK ) {
		fprintf(stdout, "error getting DS buffer poitions\n");
		return;
	}
	in_play = (in_play / ctx->buffer_size);
	while (1) {
		if (IDirectSoundBuffer_GetCurrentPosition(ctx->pOutput, &cur_play, NULL) != DS_OK ) {
			fprintf(stdout, "error getting DS buffer poitions\n");
			return;
		}
		cur_play = (cur_play / ctx->buffer_size);
		if (cur_play == in_play) {
			Sleep(5);
		} else {
			/**/
			ctx->frame_state[in_play] = 0;
			DS_FillBuffer(dr, in_play);
			return;
		}
	}
}

static u32 DS_QueryOutputSampleRate(AudioOutput *dr, u32 desired_samplerate, u32 NbChannels, u32 nbBitsPerSample)
{
	/*all sample rates supported for now ... */
	return desired_samplerate;
}

static void DS_Pause(AudioOutput *dr, Bool DoFreeze)
{
	DSCONTEXT();
	if (DoFreeze) {
		IDirectSoundBuffer_Stop(ctx->pOutput);
	} else {
		IDirectSoundBuffer_Play(ctx->pOutput, 0, 0, DSBPLAY_LOOPING);
	}
}

static void DS_SetVolume(AudioOutput *dr, u32 Volume)
{
	LONG Vol;
	DSCONTEXT();
	if (Volume > 100) Volume = 100;
	Vol = Volume * (DSBVOLUME_MIN - DSBVOLUME_MAX) / 100;
	IDirectSoundBuffer_SetVolume(ctx->pOutput, Vol);
}

static void DS_SetPan(AudioOutput *dr, u32 Pan)
{
	LONG dspan;
	DSCONTEXT();

	if (Pan > 100) Pan = 100;
	if (Pan > 50) {
		dspan = DSBPAN_RIGHT * (Pan - 50) / 50;
	} else if (Pan < 50) {
		dspan = DSBPAN_LEFT * (50 - Pan) / 50;
	} else {
		dspan = 0;
	}
	IDirectSoundBuffer_SetPan(ctx->pOutput, dspan);
}


static void DS_SetPriority(AudioOutput *dr, u32 Priority)
{
}

static u32 DS_GetAudioDelay(AudioOutput *dr)
{
	DSCONTEXT();
	return ctx->total_audio_buffer_ms;
}

static u32 DS_GetTotalBufferTime(AudioOutput *dr)
{
	DSCONTEXT();
	return ctx->total_audio_buffer_ms;
}

void *NewAudioOutput()
{
	HRESULT hr;
	DSContext *ctx;
	AudioOutput *driv;

	if( FAILED( hr = CoInitialize(NULL) ) ) return NULL;

	
	ctx = malloc(sizeof(DSContext));
	memset(ctx, 0, sizeof(DSContext));

	driv = malloc(sizeof(AudioOutput));
	memset(driv, 0, sizeof(AudioOutput));
	M4_REG_PLUG(driv, M4_AUDIO_OUTPUT_INTERFACE, "DirectSound Audio Output", "gpac distribution", 0);

	driv->opaque = ctx;

	driv->SetupHardware = DS_SetupHardware;
	driv->Shutdown = DS_Shutdown;
	driv->ConfigureOutput = DS_ConfigureOutput;
	driv->SetVolume = DS_SetVolume;
	driv->SetPan = DS_SetPan;
	driv->Pause = DS_Pause;
	driv->SetPriority = DS_SetPriority;
	driv->GetAudioDelay = DS_GetAudioDelay;
	driv->GetTotalBufferTime = DS_GetTotalBufferTime;
	driv->WriteAudio = DS_WriteAudio;
	driv->QueryOutputSampleRate = DS_QueryOutputSampleRate;
	/*never threaded*/
	driv->SelfThreaded = 0;
	return driv;
}

void DeleteAudioOutput(void *ifce)
{
	AudioOutput *dr = (AudioOutput *)ifce;
	DSCONTEXT();

	free(ctx);
	free(ifce);
	CoUninitialize();
}

