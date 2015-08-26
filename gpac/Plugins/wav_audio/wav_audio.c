/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / wave audio render plugin
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


#include <gpac/m4_avhw.h>

#define MAX_AUDIO_BUFFER	30

typedef struct 
{
	HWAVEOUT hwo;
	WAVEHDR wav_hdr[MAX_AUDIO_BUFFER];
	WAVEFORMATEX fmt;

	u32 num_buffers;
	u32 buffer_size;

	Bool exit_request;
	HANDLE event;

	u32 vol, pan;
	u32 const_delay;

	Bool force_config;
	u32 cfg_num_buffers, cfg_num_buffer_per_sec;

	char *wav_buf;
} WAVContext;



#if !defined(DISABLE_WAVE_EX) && !defined(_WIN32_WCE)
/*IF YOU CAN'T COMPILE WAVEOUT TRY TO COMMENT THIS - note waveOut & multichannel is likely not to work*/
#define USE_WAVE_EXT
#endif


#ifdef USE_WAVE_EXT

/*for channel codes*/
#include <gpac/m4_decoder.h>

#ifndef WAVE_FORMAT_EXTENSIBLE
#define  WAVE_FORMAT_EXTENSIBLE   0xFFFE
#endif

#ifndef SPEAKER_FRONT_LEFT
#   define SPEAKER_FRONT_LEFT             0x1
#   define SPEAKER_FRONT_RIGHT            0x2
#   define SPEAKER_FRONT_CENTER           0x4
#   define SPEAKER_LOW_FREQUENCY          0x8
#   define SPEAKER_BACK_LEFT              0x10
#   define SPEAKER_BACK_RIGHT             0x20
#   define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#   define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#   define SPEAKER_BACK_CENTER            0x100
#   define SPEAKER_SIDE_LEFT              0x200
#   define SPEAKER_SIDE_RIGHT             0x400
#   define SPEAKER_TOP_CENTER             0x800
#   define SPEAKER_TOP_FRONT_LEFT         0x1000
#   define SPEAKER_TOP_FRONT_CENTER       0x2000
#   define SPEAKER_TOP_FRONT_RIGHT        0x4000
#   define SPEAKER_TOP_BACK_LEFT          0x8000
#   define SPEAKER_TOP_BACK_CENTER        0x10000
#   define SPEAKER_TOP_BACK_RIGHT         0x20000
#   define SPEAKER_RESERVED               0x80000000
#endif

#ifndef _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif

const static GUID  GPAC_KSDATAFORMAT_SUBTYPE_PCM = {0x00000001,0x0000,0x0010, {0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71} };

#pragma message("Using multichannel audio extensions")

#endif



#define WAVCTX()	WAVContext *ctx = (WAVContext *)dr->opaque;

static M4Err WAV_SetupHardware(AudioOutput *dr, void *os_handle, u32 num_buffers, u32 num_buffer_per_sec)
{
	WAVCTX();

	ctx->force_config = (num_buffers && num_buffer_per_sec) ? 1 : 0;
	ctx->cfg_num_buffers = num_buffers;
	if (ctx->cfg_num_buffers <= 1) ctx->cfg_num_buffers = 2;
	ctx->cfg_num_buffer_per_sec = num_buffer_per_sec;
	if (!ctx->force_config) ctx->num_buffers = 6;

	return M4OK;
}

static void CALLBACK WaveProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    u32 i, queued_frames;
	AudioOutput *dr = (AudioOutput *) dwInstance;
	WAVCTX();

	if (uMsg != WOM_DONE) return;
	if (ctx->exit_request) return;
		
	queued_frames = 0;
    for (i=0; i<ctx->num_buffers; i++) {
        if (! (ctx->wav_hdr[i].dwFlags & WHDR_DONE) ) queued_frames++;
    }
    if (queued_frames < ctx->num_buffers / 2)
		SetEvent(ctx->event);
} 


static void close_waveform(AudioOutput *dr)
{
	WAVCTX();

    if (!ctx->event) return;

	ctx->exit_request = 1;
	SetEvent(ctx->event);

	if (ctx->hwo) {
		u32 i;
		Bool not_done;
		MMRESULT res;
		/*wait for all buffers to complete, otherwise this locks waveOutReset*/
		while (1) {
			not_done = 0;
			for (i=0 ; i< ctx->num_buffers; i++) {
				if (! (ctx->wav_hdr[i].dwFlags & WHDR_DONE)) {
					not_done = 1;
					break;
				}
			}
			if (!not_done) break;
			Sleep(60);
		}
		/*waveOutReset gives unpredictable results on PocketPC, so just close right away*/
		while (1) {
			res = waveOutClose(ctx->hwo);
			if (res == MMSYSERR_NOERROR) break;
		}
		ctx->hwo = NULL;
	}
	if (ctx->wav_buf) free(ctx->wav_buf);
	ctx->wav_buf = NULL;

	
    CloseHandle(ctx->event);
	ctx->event = NULL;
	ctx->exit_request = 0;
}

static void WAV_Shutdown(AudioOutput *dr)
{
	close_waveform(dr);
}


/*we assume what was asked is what we got*/
static M4Err WAV_ConfigureOutput(AudioOutput *dr, u32 *SampleRate, u32 *NbChannels, u32 *nbBitsPerSample, u32 channel_cfg)
{
	u32 i, retry;
	HRESULT	hr;
	WAVEFORMATEX *fmt;
#ifdef USE_WAVE_EXT
	WAVEFORMATEXTENSIBLE format_ex;
#endif

	WAVCTX();

	if (!ctx) return M4BadParam;

	/*reset*/
	close_waveform(dr);

#ifndef USE_WAVE_EXT
	if (*NbChannels>2) *NbChannels=2;
#endif

	memset (&ctx->fmt, 0, sizeof(ctx->fmt));
	ctx->fmt.cbSize = sizeof(WAVEFORMATEX);
	ctx->fmt.wFormatTag = WAVE_FORMAT_PCM;
	ctx->fmt.nChannels = *NbChannels;
	ctx->fmt.wBitsPerSample = *nbBitsPerSample;
	ctx->fmt.nSamplesPerSec = *SampleRate;
	ctx->fmt.nBlockAlign = ctx->fmt.wBitsPerSample * ctx->fmt.nChannels / 8;
	ctx->fmt.nAvgBytesPerSec = *SampleRate * ctx->fmt.nBlockAlign; 
	fmt = &ctx->fmt;

#ifdef USE_WAVE_EXT
	if (channel_cfg && ctx->fmt.nChannels>2) {
		memset(&format_ex, 0, sizeof(WAVEFORMATEXTENSIBLE));
		format_ex.Format = ctx->fmt;
		format_ex.Format.cbSize = 22;
		format_ex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		format_ex.SubFormat = GPAC_KSDATAFORMAT_SUBTYPE_PCM;
		format_ex.Samples.wValidBitsPerSample = ctx->fmt.wBitsPerSample;
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
		fmt = (WAVEFORMATEX *) &format_ex;
	}
#endif

	/* Open a waveform device for output using window callback. */ 
	retry = 10;
	while (retry) {		
		hr = waveOutOpen((LPHWAVEOUT)&ctx->hwo, WAVE_MAPPER, &ctx->fmt, (DWORD) WaveProc, (DWORD) dr, 
			CALLBACK_FUNCTION | WAVE_ALLOWSYNC | WAVE_FORMAT_DIRECT
			);

		if (hr == MMSYSERR_NOERROR) break;
		/*couldn't open audio*/
		if (hr != MMSYSERR_ALLOCATED) return M4IOErr;
		Sleep(10);
		retry--;
	}
	if (hr != MMSYSERR_NOERROR) return M4IOErr;

	if (!ctx->force_config) {
		/*one wave buffer size*/
		ctx->buffer_size = 1024 * ctx->fmt.nBlockAlign;
	} else {
		ctx->num_buffers = ctx->cfg_num_buffers;
		ctx->buffer_size = ctx->fmt.nAvgBytesPerSec / ctx->cfg_num_buffer_per_sec;
	}

    ctx->event = CreateEvent( NULL, FALSE, FALSE, NULL);

	/*make sure we're aligned*/
	while (ctx->buffer_size % ctx->fmt.nBlockAlign) ctx->buffer_size++;

	ctx->wav_buf = malloc(ctx->buffer_size*ctx->num_buffers*sizeof(char));
	memset(ctx->wav_buf, 0, ctx->buffer_size*ctx->num_buffers*sizeof(char));

	/*setup wave headers*/
	for (i=0 ; i < ctx->num_buffers; i++) {
		memset(& ctx->wav_hdr[i], 0, sizeof(WAVEHDR));
		ctx->wav_hdr[i].dwBufferLength = ctx->buffer_size; 
		ctx->wav_hdr[i].lpData = & ctx->wav_buf[i*ctx->buffer_size];
		ctx->wav_hdr[i].dwFlags = WHDR_DONE;
		waveOutPrepareHeader(ctx->hwo, &ctx->wav_hdr[i], sizeof(WAVEHDR));
		waveOutWrite(ctx->hwo, &ctx->wav_hdr[i], sizeof(WAVEHDR));
	}
	ctx->const_delay = 1000 * ctx->num_buffers * ctx->buffer_size / ctx->fmt.nAvgBytesPerSec;

	return M4OK;
}

static void WAV_WriteAudio(AudioOutput *dr)
{
	LPWAVEHDR hdr;
	HRESULT hr;
	u32 i, queued_frames;
	WAVCTX();

	if (!ctx->hwo) return;

	WaitForSingleObject(ctx->event, INFINITE);

	i = 0;
	queued_frames = 0;

	for (i=0; i<ctx->num_buffers; i++) {
		if ( (ctx->wav_hdr[i].dwFlags & WHDR_DONE) && ctx->wav_hdr[i].dwUser) {
			waveOutUnprepareHeader(ctx->hwo, &ctx->wav_hdr[i], sizeof(WAVEHDR) );
			ctx->wav_hdr[i].dwUser = 0;
		}
		//if (!(ctx->wav_hdr[i].dwFlags & WHDR_DONE)) queued_frames++;
	}


	if (ctx->exit_request) return;


	for (i=0; i<ctx->num_buffers; i++) {
		/*get buffer*/
		hdr = &ctx->wav_hdr[i];

		if (hdr->dwFlags & WHDR_DONE) {
			hdr->dwBufferLength = ctx->buffer_size;
			/*update delay...*/
			/*fill it*/
			dr->FillBuffer(dr->audio_renderer, hdr->lpData, ctx->buffer_size);

			hdr->dwFlags = 0;
			hdr->dwUser = 1;
		    hr = waveOutPrepareHeader(ctx->hwo, hdr, sizeof(WAVEHDR));
			/*write it*/
			waveOutWrite(ctx->hwo, hdr, sizeof(WAVEHDR));
		}
	}
}

static void WAV_Pause(AudioOutput *dr, Bool DoFreeze)
{
	WAVCTX();
	if (DoFreeze) 
		waveOutPause(ctx->hwo);
	else 
		waveOutRestart(ctx->hwo);
}

static void set_vol_pan(WAVContext *ctx)
{
	WORD rV, lV;
	/*in wave, volume & pan are specified as a DWORD. LOW word is LEFT channel, HIGH is right - iPaq doesn't support that*/
	lV = (WORD) (ctx->vol * ctx->pan / 100);
	rV = (WORD) (ctx->vol * (100 - ctx->pan) / 100);

#ifdef _WIN32_WCE
	waveOutSetVolume(0, MAKELONG(lV, rV) );
#else
	waveOutSetVolume(ctx->hwo, MAKELONG(lV, rV) );
#endif

}

static void WAV_SetVolume(AudioOutput *dr, u32 Volume)
{
	WAVCTX();
	if (Volume > 100) Volume = 100;
	ctx->vol = Volume * 0xFFFF / 100;
	set_vol_pan(ctx);
}

static void WAV_SetPan(AudioOutput *dr, u32 Pan)
{
	WAVCTX();
	if (Pan > 100) Pan = 100;
	ctx->pan = Pan;
	set_vol_pan(ctx);
}

static void WAV_SetPriority(AudioOutput *dr, u32 Priority)
{
	TH_SetPriority(NULL, TH_PRIOR_HIGHEST);
}


static u32 WAV_QueryOutputSampleRate(AudioOutput *dr, u32 desired_samplerate, u32 NbChannels, u32 nbBitsPerSample)
{
	/*iPaq's output frequencies available*/
#ifdef _WIN32_WCE
	switch (desired_samplerate) {
	case 11025:
	case 22050:
		return 22050;
	case 8000:
	case 16000:
	case 32000:
		return 44100;
	case 24000:
	case 48000:
		return 44100;
	case 44100: return 44100;
	default:
		return desired_samplerate;
	}
#else
	return desired_samplerate;
#endif
}

static u32 WAV_GetAudioDelay(AudioOutput *dr)
{
	WAVCTX();
	return ctx->const_delay;
}

static u32 WAV_GetTotalBufferTime(AudioOutput *dr)
{
	WAVCTX();
	return ctx->const_delay;
}


void *NewWAVRender()
{
	WAVContext *ctx;
	AudioOutput *driv;
	ctx = malloc(sizeof(WAVContext));
	memset(ctx, 0, sizeof(WAVContext));
	ctx->num_buffers = 10;
	ctx->pan = 50;
	ctx->vol = 100;
	driv = malloc(sizeof(AudioOutput));
	memset(driv, 0, sizeof(AudioOutput));
	M4_REG_PLUG(driv, M4_AUDIO_OUTPUT_INTERFACE, "Windows MME Output", "gpac distribution", 0)

	driv->opaque = ctx;

	driv->SelfThreaded = 0;
	driv->SetupHardware = WAV_SetupHardware;
	driv->Shutdown = WAV_Shutdown;
	driv->ConfigureOutput = WAV_ConfigureOutput;
	driv->GetAudioDelay = WAV_GetAudioDelay;
	driv->GetTotalBufferTime = WAV_GetTotalBufferTime;
	driv->SetVolume = WAV_SetVolume;
	driv->SetPan = WAV_SetPan;
	driv->Pause = WAV_Pause;
	driv->SetPriority = WAV_SetPriority;
	driv->QueryOutputSampleRate = WAV_QueryOutputSampleRate;
	driv->WriteAudio = WAV_WriteAudio;

	return driv;
}

void DeleteWAVRender(void *ifce)
{
	AudioOutput *dr = (AudioOutput *) ifce;
	WAVContext *ctx = (WAVContext *)dr->opaque;
	free(ctx);
	free(dr);
}

Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) return NewWAVRender();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	AudioOutput *dr = (AudioOutput *) ifce;
	switch (dr->InterfaceType) {
	case M4_AUDIO_OUTPUT_INTERFACE:
		DeleteWAVRender(dr);
		break;
	}
}
