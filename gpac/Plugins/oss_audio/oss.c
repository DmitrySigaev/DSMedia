/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / linux_oss audio render plugin
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



/*
 * notes about linux_audio_oss-module, dec. 2003, zefir
 *
 * (based on oss programer's guide v1.11)
 * 
 * buffering:
 * oss-driver has already implemented efficient buffering-routines
 * -> no need to implement own buffering-schemes
 *  for performance reasons audio-data is fed in chunks of size depending
 *  on pcm-parameters and returned from the oss-driver after setup.
 *  audio-data is written to the device without resynchronisation
 *  -> GetAudioDelay always returns 0, there is no need for GotoSleep
 *
 *  mixer:
 *  following the guideline mixer-functions should be implemented in external
 *  programs to maintain the source soundcard-independent
 *  -> no mixing-functionality implemented so far
 */

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "oss.h"

#define OSSCTX()	OSSContext *ctx = (OSSContext *)dr->opaque;


static M4Err OSS_SetupHardware(AudioOutput*dr, void *os_handle, u32 num_buffers, u32 num_buffer_per_sec)
{
	int audio;
	OSSCTX();
	if((audio=open(OSS_AUDIO_DEVICE,O_WRONLY))==-1)
		return M4NotSupported;
	ctx->audio_device=audio;
	return M4OK;
}

static void OSS_Shutdown(AudioOutput*dr)
{
	OSSCTX();
	ioctl(ctx->audio_device,SNDCTL_DSP_RESET);
	close(ctx->audio_device);
	if (ctx->wav_buf) 
		free(ctx->wav_buf);
	ctx->wav_buf = NULL;
}


static M4Err OSS_ConfigureOutput(AudioOutput*dr, u32 SampleRate, u32 NbChannels, u32 nbBitsPerSample, u32 FPS)
{
	int i, format;
	OSSCTX();

	if (!ctx) 
		return M4BadParam;
	/* reset and reopen audio-device */
	ioctl(ctx->audio_device,SNDCTL_DSP_RESET);
	close(ctx->audio_device);
	ctx->audio_device=open(OSS_AUDIO_DEVICE,O_WRONLY);
	i=NbChannels;
	if(ioctl(ctx->audio_device, SNDCTL_DSP_CHANNELS,&i)==-1)
		printf("CHANNELS-error!\n");
	format=	nbBitsPerSample==16 ? AFMT_S16_LE : 
		nbBitsPerSample== 8 ? AFMT_S8 : 0;
	if(!format)
		printf("Error: invalid format!\n");
	else
		if(ioctl(ctx->audio_device, SNDCTL_DSP_SETFMT,&format)==-1)
			printf("SETFMT-error!\n");
	i=SampleRate;
	if(ioctl(ctx->audio_device, SNDCTL_DSP_SPEED,&i)==-1)
		printf("SPEED-error!\n");
	ioctl(ctx->audio_device,SNDCTL_DSP_GETBLKSIZE,&i);
	ctx->buf_size = i;
	ctx->wav_buf = realloc(ctx->wav_buf, ctx->buf_size*sizeof(char));
	if(!ctx->wav_buf)
		return M4OutOfMem;
	memset(ctx->wav_buf, 0, ctx->buf_size*sizeof(char));
	return M4OK;
}

static void OSS_LockAndWrite(AudioOutput*dr)
{
	OSSCTX();
	dr->FillBuffer(dr->audio_renderer, ctx->wav_buf, ctx->buf_size);
	write(ctx->audio_device, ctx->wav_buf, ctx->buf_size);
}

static void OSS_GotoSleep(AudioOutput*dr) {}
static void OSS_Freeze(AudioOutput*dr, Bool DoFreeze) {}
static void OSS_SetVolume(AudioOutput*dr, u32 Volume) {}
static void OSS_SetPan(AudioOutput*dr, u32 Pan) {}
static void OSS_SetPriority(AudioOutput*dr, u32 Priority) {}
static u32 OSS_GetAudioDelay(AudioOutput*dr) { return 0; }

/*
 * to get the best matching samplerate the oss-device can be set up
 * with the desired sr. if not supported the returned value contains the
 * best matching sr.
 *
 * todo: supported samplerate could depend on nb_channels and format
 */
static u32 OSS_QueryOutputSampleRate(AudioOutput*dr, u32 desired_sr, u32 NbChannels, u32 nbBitsPerSample)
{
	int i;
	OSSCTX();
	/* reset and reopen audio-device */
	ioctl(ctx->audio_device,SNDCTL_DSP_RESET);
	close(ctx->audio_device);
	ctx->audio_device=open(OSS_AUDIO_DEVICE,O_WRONLY);
	i=desired_sr;
	if(ioctl(ctx->audio_device, SNDCTL_DSP_SPEED,&i)==-1)
		return 0;
	return i;
}

void *NewOSSRender()
{
	OSSContext *ctx;
	AudioOutput*driv;
	ctx = malloc(sizeof(OSSContext));
	if(!ctx)
		return NULL;
	memset(ctx, 0, sizeof(OSSContext));
	driv = malloc(sizeof(AudioDriver));
	if(!driv)
	{
		free(ctx);
		ctx=NULL;
		return NULL;
	}
	memset(driv, 0, sizeof(AudioDriver));
	driv->opaque = ctx;
	driv->SelfThreaded = 0;
	driv->SetupHardware = OSS_SetupHardware;
	driv->Shutdown = OSS_Shutdown;
	driv->ConfigureOutput = OSS_ConfigureOutput;
	driv->GetAudioDelay = OSS_GetAudioDelay;
	driv->SetVolume = OSS_SetVolume;
	driv->SetPan = OSS_SetPan;
	driv->Freeze = OSS_Freeze;
	driv->SetPriority = OSS_SetPriority;
	driv->QueryOutputSampleRate = OSS_QueryOutputSampleRate;
	driv->GotoSleep = OSS_GotoSleep;
	driv->LockAndWrite = OSS_LockAndWrite;

	M4_REG_PLUG(driv, M4_AUDIO_OUTPUT_INTERFACE, "Linux OSS Audio Output", "gpac distribution (zefir k.)", 0);
	return driv;
}

void DeleteOSSRender(void *ifce)
{
	AudioOutput*dr = (AudioOutput*) ifce;
	OSSContext *ctx = (OSSContext *)dr->opaque;
	free(ctx);
	free(dr);
}


/*
 * ********************************************************************
 * interface
 */
Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) 
		return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) 
		return NewOSSRender();
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	AudioOutput*dr = (AudioOutput*) ifce;
	if(dr->InterfaceType==M4_AUDIO_OUTPUT_INTERFACE)
		DeleteOSSRender(dr);
}
