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


#include <gpac/intern/m4_esm_dev.h>
#include <gpac/m4_x3d_nodes.h>
#include "MediaMemory.h"
#include "MediaControl.h"
#include <gpac/m4_svg_nodes.h>


MediaObject *MO_FindObject(SFNode *node, MFURL *url)
{
	u32 obj_type;
	InlineScene *is;
	LPSCENEGRAPH sg = Node_GetParentGraph(node);
	if (!sg) return NULL;
	is = SG_GetPrivate(sg);
	if (!is) return NULL;

	/*keep track of the kind of object expected if URL is not using OD scheme*/
	switch (Node_GetTag(node)) {
	/*MPEG4 only*/
	case TAG_MPEG4_AudioSource: obj_type = NM_OD_AUDIO; break;
	case TAG_MPEG4_AnimationStream: obj_type = NM_OD_BIFS; break;
	case TAG_MPEG4_InputSensor: obj_type = NM_OD_INTERACT; break;

	/*MPEG4/X3D*/
	case TAG_MPEG4_MovieTexture: case TAG_X3D_MovieTexture: obj_type = NM_OD_VIDEO; break;
	case TAG_MPEG4_Background2D: obj_type = NM_OD_VIDEO; break;
	case TAG_MPEG4_Background: case TAG_X3D_Background: obj_type = NM_OD_VIDEO; break;
	case TAG_MPEG4_ImageTexture: case TAG_X3D_ImageTexture: obj_type = NM_OD_VIDEO; break;
	case TAG_MPEG4_AudioClip: case TAG_X3D_AudioClip: obj_type = NM_OD_AUDIO; break;
	case TAG_MPEG4_Inline: case TAG_X3D_Inline: obj_type = NM_OD_SCENE; break;
	
	/*SVG*/
	case TAG_SVG_image: obj_type = NM_OD_VIDEO; break;
	case TAG_SVG_video: obj_type = NM_OD_VIDEO; break;
	case TAG_SVG_audio: obj_type = NM_OD_AUDIO; break;

	default: obj_type = NM_OD_UNDEF; break;
	}
	return IS_GetMediaObject(is, url, obj_type);
}

MediaObject *NewMediaObject(M4Client *term)
{
	MediaObject *mo;
	mo = (MediaObject *) malloc(sizeof(MediaObject));
	memset(mo, 0, sizeof(MediaObject));
	mo->speed = 1.0;
	mo->URLs.count = 0;
	mo->URLs.vals = NULL;
	mo->term = term;
	return mo;
}


void MO_UpdateCaps(MediaObject *mo)
{
	CapObject cap;
	mo->mo_flags &= ~MO_IS_INIT;

	if (mo->type == NM_OD_VIDEO) {
		cap.CapCode = CAP_WIDTH;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->width = cap.cap.valueINT;
		cap.CapCode = CAP_HEIGHT;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->height = cap.cap.valueINT;
		cap.CapCode = CAP_STRIDE;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->stride = cap.cap.valueINT;
		cap.CapCode = CAP_COLORMODE;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->pixelFormat = cap.cap.valueINT;
		cap.CapCode = CAP_FPS;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->odm->codec->fps = cap.cap.valueFT;
	}
	else if (mo->type == NM_OD_AUDIO) {
		cap.CapCode = CAP_SAMPLERATE;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->sample_rate = cap.cap.valueINT;
		cap.CapCode = CAP_NBCHANNELS;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->num_channels = cap.cap.valueINT;
		cap.CapCode = CAP_BITSPERSAMPLE;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->bits_per_sample = cap.cap.valueINT;
		mo->odm->codec->bytes_per_sec = mo->sample_rate * mo->num_channels * mo->bits_per_sample / 8;
		cap.CapCode = CAP_CHANNEL_CONFIG;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->channel_config = cap.cap.valueINT;
	}
	else if (mo->type==NM_OD_TEXT) {
		cap.CapCode = CAP_WIDTH;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->width = cap.cap.valueINT;
		cap.CapCode = CAP_HEIGHT;
		Codec_GetCap(mo->odm->codec, &cap);
		mo->height = cap.cap.valueINT;
	}
}

Bool MO_FetchFrame(MediaObject *mo, Bool resync, Bool *eos)
{
	Bool ret;
	u32 obj_time;
	LPCUBUFFER CU;
	*eos = 0;

	if (!mo) return 0;

	if (!mo->odm || !mo->odm->codec || !mo->odm->codec->CB) return 0;
	/*check if we need to play*/
	if (mo->num_open && !mo->odm->is_open) {
		ODM_Start(mo->odm);
		return 0;
	}

	ret = 0;
	CB_Lock(mo->odm->codec->CB, 1);

	/*end of stream */
	*eos = CB_IsEndOfStream(mo->odm->codec->CB);
	/*not running*/
	if (!CB_IsRunning(mo->odm->codec->CB)) goto exit;

	/*if frame locked return it*/
	if (mo->num_fetched) {
		*eos = 0;
		ret = 1;
		mo->num_fetched++;
		goto exit;
	}

	/*new frame to fetch, lock*/
	CU = CB_GetOutput(mo->odm->codec->CB);
	/*no output*/
	if (!CU || (CU->RenderedLength == CU->dataLength)) {
		goto exit;
	}

	assert(CU->TS >= mo->odm->codec->CB->LastRenderedTS);
	if (mo->odm->codec->CB->UnitCount==1) resync = 0;

	/*resync*/
	if (resync) {
		obj_time = CK_GetTime(mo->odm->codec->ck);
		while (CU->TS < obj_time) {
			if (!CU->next->dataLength) break;
			/*figure out closest time*/
			if (CU->next->TS > obj_time) {
				/*current frame is closer to object clock than next one, keep it*/
				if ((obj_time - CU->TS) < (CU->next->TS - obj_time) ) break;

			}
			/*discard*/
			CU->RenderedLength = CU->dataLength = 0;
			CB_DropOutput(mo->odm->codec->CB);
			/*get next*/
			CU = CB_GetOutput(mo->odm->codec->CB);
		}
	}
	mo->current_size = CU->dataLength - CU->RenderedLength;
	mo->current_frame = CU->data + CU->RenderedLength;
	mo->current_ts = CU->TS;
	/*also adjust CU time based on consummed bytes in input, since some codecs output very large audio chunks*/
	if (mo->odm->codec->bytes_per_sec) mo->current_ts += CU->RenderedLength * 1000 / mo->odm->codec->bytes_per_sec;

	mo->num_fetched++;
	ret = 1;

/*
	obj_time = CK_GetTime(mo->odm->codec->ck);
	fprintf(stdout, "At OTB %d fetch frame TS %d size %d - %d unit in CB\n", obj_time, mo->current_ts, mo->current_size, mo->odm->codec->CB->UnitCount);
*/
	
exit:

	CB_Lock(mo->odm->codec->CB, 0);
	return ret;
}

void MO_ReleaseFrame(MediaObject *mo, u32 nb_bytes, Bool forceDrop)
{
	u32 obj_time;
	if (!mo || !mo->num_fetched) return;
	assert(mo->odm);
	mo->num_fetched--;
	if (mo->num_fetched) return;

	CB_Lock(mo->odm->codec->CB, 1);

	/*perform a sanity check on TS since the CB may have changed status - this may happen in 
	temporal scalability only*/
	if (mo->odm->codec->CB->output->dataLength ) {
		assert(mo->odm->codec->CB->output->RenderedLength + nb_bytes <= mo->odm->codec->CB->output->dataLength);
		mo->odm->codec->CB->output->RenderedLength += nb_bytes;

		/*discard frame*/
		if (mo->odm->codec->CB->output->RenderedLength == mo->odm->codec->CB->output->dataLength) {
			if (forceDrop) {
				CB_DropOutput(mo->odm->codec->CB);
			} else {
				obj_time = CK_GetTime(mo->odm->codec->ck);
				if (mo->odm->codec->CB->output->next->dataLength) { 
					if (2*obj_time < mo->current_ts + mo->odm->codec->CB->output->next->TS ) {
						mo->odm->codec->CB->output->RenderedLength = 0;
					} else {
						CB_DropOutput(mo->odm->codec->CB);
					}
				} else {
					CB_DropOutput(mo->odm->codec->CB);
				}
			}
		}
	}
	CB_Lock(mo->odm->codec->CB, 0);
}

void MO_GetObjectTime(MediaObject *mo, u32 *obj_time)
{
	if (!mo || !mo->odm || !obj_time) return;

	/*regular media codec...*/
	if (mo->odm->codec) {
		/*get absolute clock (without drift) for audio*/
		if (mo->odm->codec->type==M4ST_AUDIO)  
			*obj_time = CK_GetRealTime(mo->odm->codec->ck);
		else
			*obj_time = CK_GetTime(mo->odm->codec->ck);
	}
	/*BIFS object */
	else if (mo->odm->subscene && mo->odm->subscene->scene_codec) {
		*obj_time = CK_GetTime(mo->odm->subscene->scene_codec->ck);
	} 
	/*unknown / unsupported object*/
	else {
		*obj_time = 0;
	}
}


void MO_Play(MediaObject *mo)
{
	if (!mo) return;

	Term_LockNet(mo->term, 1);
	if (!mo->num_open && mo->odm) {
		ODM_Start(mo->odm);
	} else {
		if (mo->num_to_restart) mo->num_restart--;
		if (!mo->num_restart && (mo->num_to_restart==mo->num_open+1) ) {
			MC_Restart(mo->odm);
			mo->num_to_restart = mo->num_restart = 0;
		}
	}

	mo->num_open++;
	Term_LockNet(mo->term, 0);
}

void MO_Stop(MediaObject *mo)
{
	if (!mo) return;
	assert(mo->num_open);
	mo->num_open--;
	if (!mo->num_open && mo->odm) {
		ODM_Stop(mo->odm, 0);
	} else {
		if (!mo->num_to_restart) {
			mo->num_restart = mo->num_to_restart = mo->num_open + 1;
		}
	}
}

void MO_Restart(MediaObject *mo)
{
	MediaControlStack *ctrl;
	if (!mo) return;
	assert(mo->num_open);
	/*this should not happen (inline scene are restarted internally)*/
	assert(!mo->odm->subscene);

	ctrl = ODM_GetMediaControl(mo->odm);
	
	if (!ctrl) {
		Clock *ck = ODM_GetMediaClock(mo->odm->parentscene->root_od);
		/*don't restart if sharing parent scene clock*/
		if (ODM_SharesClock(mo->odm, ck)) return;
	}
	/*all other cases, call restart to take into account clock references*/
	MC_Restart(mo->odm);
}

Bool MO_UrlChanged(MediaObject *mo, MFURL *url)
{
	if (!mo) return (url ? 1 : 0);
	if (mo->OD_ID != DYNAMIC_OD_ID) return (URL_GetODID(url) != mo->OD_ID) ? 1 : 0;
	if (URL_GetODID(url) != DYNAMIC_OD_ID) return 1;
	return !Inline_IsSameURL(&mo->URLs, url);
}


void MO_SetSpeed(MediaObject *mo, Float speed)
{
	MediaControlStack *ctrl;

	if (!mo) return;
	if (!mo->odm) {
		mo->speed = speed;
		return;
	}
	/*if media control forbidd that*/
	ctrl = ODM_GetMediaControl(mo->odm);
	if (ctrl) return;
	ODM_SetSpeed(mo->odm, speed);
}

Float MO_GetSpeed(MediaObject *mo, Float in_speed)
{
	MediaControlStack *ctrl;
	if (!mo || !mo->odm) return in_speed;
	/*get control*/
	ctrl = ODM_GetMediaControl(mo->odm);
	return ctrl ? ctrl->control->mediaSpeed : in_speed;
}

Bool MO_GetLoop(MediaObject *mo, Bool in_loop)
{
	Clock *ck;
	MediaControlStack *ctrl;
	if (!mo || !mo->odm) return in_loop;
	
	/*get control*/
	ctrl = ODM_GetMediaControl(mo->odm);
	if (ctrl) in_loop = ctrl->control->loop;

	/*otherwise looping is only accepted if not sharing parent scene clock*/
	ck = ODM_GetMediaClock(mo->odm->parentscene->root_od);
	if (ODM_SharesClock(mo->odm, ck)) return 0;
	return in_loop;
}

Double MO_GetDuration(MediaObject *mo)
{
	if (!mo || !mo->odm) return -1.0;
	return ((Double)mo->odm->duration)/1000.0;
}

Bool MO_ShouldDeactivate(MediaObject *mo)
{
	MediaControlStack *ctrl;
	
	if (!mo || !mo->odm) return 0;
	/*not running*/
	if (!mo->odm->is_open) return 0;

	/*get media control and see if object owning control is running*/
	ctrl = ODM_GetMediaControl(mo->odm);
	if (!ctrl) return 1;
	/*if ctrl and ctrl not ruling this mediaObject, deny deactivation*/
	if (ctrl->stream->odm != mo->odm) return 0;
	/*this is currently under discussion in MPEG. for now we deny deactivation as soon as a mediaControl is here*/
	if (ctrl->stream->odm->is_open) return 0;
	/*otherwise allow*/	
	return 1;
}

Bool MO_IsMuted(MediaObject *mo)
{
	if (!mo->odm) return 1;
	return mo->odm->media_ctrl ? mo->odm->media_ctrl->control->mute : 0;
}

Bool MO_IsFinished(MediaObject *mo)
{
	GenericCodec *codec;
	u32 dur;
	if (!mo || !mo->odm) return 0;

	/*for natural media use composition buffer*/
	if (mo->odm->codec && mo->odm->codec->CB) return (mo->odm->codec->CB->Status==CB_STOP) ? 1 : 0;

	/*otherwise check EOS and time*/
	codec = mo->odm->codec;
	dur = mo->odm->duration;
	if (!mo->odm->codec) {
		if (!mo->odm->subscene) return 0;
		codec = mo->odm->subscene->scene_codec;
		dur = mo->odm->subscene->duration;
	}
	if (codec->Status==CODEC_STOP) {
		/*codec is done, check by duration*/
		Clock *ck = ODM_GetMediaClock(mo->odm);
		if (CK_GetTime(ck) > dur) return 1;
	}
	return 0;
}

/*resyncs clock - only audio objects are allowed to use this*/
void MO_AdjustClock(MediaObject *mo, s32 ms_drift)
{
	if (!mo || !mo->odm) return;
	if (!mo->odm->codec || (mo->odm->codec->type != M4ST_AUDIO) ) return;
	CK_AdjustDrift(mo->odm->codec->ck, ms_drift);
}

