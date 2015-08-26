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
#include <gpac/m4_render.h>
#include "MediaMemory.h"
#include "MediaControl.h"
#include "InputSensor.h"

/*update config of object*/
void MO_UpdateCaps(MediaObject *mo);


#define TIME_CHECK		3


M4Err Codec_Load(GenericCodec *codec, ESDescriptor *esd, u32 PL);

GenericCodec *NewDecoder(ODManager *odm, ESDescriptor *base_layer, s32 PL, M4Err *e)
{
	GenericCodec *tmp = malloc(sizeof(GenericCodec));
	if (! tmp) {
		*e = M4OutOfMem;
		return NULL;
	}
	memset(tmp, 0, sizeof(GenericCodec));
	tmp->odm = odm;

	if (PL<0) PL = 0xFF;
	*e = Codec_Load(tmp, base_layer, PL);
	
	if (*e) {
		free(tmp);
		return NULL;
	}
	/*remember codec type*/
	tmp->type = base_layer->decoderConfig->streamType;
	tmp->inChannels = NewChain();	
	tmp->Status = CODEC_STOP;
	
	return tmp;
}

GenericCodec *Codec_UseDecoder(GenericCodec *codec, ODManager *odm)
{
	GenericCodec *tmp;
	if (!codec->decio) return NULL;
	SAFEALLOC(tmp, sizeof(GenericCodec));
	tmp->type = codec->type;
	tmp->inChannels = NewChain();	
	tmp->Status = CODEC_STOP;
	tmp->odm = odm;
	tmp->flags = codec->flags | COD_IS_USE;
	tmp->decio = codec->decio;
	return tmp;
}

M4Err Codec_AddChannel(GenericCodec *codec, Channel *ch)
{
	M4Err e;
	NetworkCommand com;
	Channel *a_ch;
	unsigned char *dsi;
	u32 dsiSize, CUsize, i;
	CapObject cap;
	u32 min, max;


	/*only for valid codecs (eg not OCR)*/
	if (codec->decio) {
		com.get_dsi.dsi = NULL;
		dsi = NULL;
		dsiSize = 0;
		if (ch->esd->decoderConfig->upstream) codec->flags |= COD_HAS_UPSTREAM;
		if (ch->esd->decoderConfig->decoderSpecificInfo) {
			dsi = ch->esd->decoderConfig->decoderSpecificInfo->data;
			dsiSize = ch->esd->decoderConfig->decoderSpecificInfo->dataLength;
		} 
		if (!dsi) {
			com.command_type = CHAN_GET_DSI;
			com.base.on_channel = ch;
			e = NM_ServiceCommand(ch->service, &com);
			if (!e && com.get_dsi.dsi) {
				dsi = com.get_dsi.dsi;
				dsiSize = com.get_dsi.dsi_len;
			}
		}

		e = codec->decio->AttachStream(codec->decio, ch->esd->ESID, 
					dsi, dsiSize, ch->esd->dependsOnESID, ch->esd->decoderConfig->objectTypeIndication, ch->esd->decoderConfig->upstream);

		if (com.get_dsi.dsi) free(com.get_dsi.dsi);

		/*some broken ISMA files don't signal the DSI in the IOD, only in sdp...*/
		if (e) return e;

		/*ask codec for desired output capacity - note this may be 0 if stream is not yet configured*/
		cap.CapCode = CAP_OUTPUTSIZE;
		Codec_GetCap(codec, &cap);
		if (codec->CB && (cap.cap.valueINT != codec->CB->UnitSize)) {
			CB_Delete(codec->CB);
			codec->CB = NULL;
		}
		CUsize = cap.cap.valueINT;

		/*get desired amount of units and minimal fullness (used for scheduling)*/
		switch(codec->type) {
		case M4ST_VISUAL:
		case M4ST_AUDIO:
			cap.CapCode = CAP_BUFFER_MIN;
			Codec_GetCap(codec, &cap);
			min = cap.cap.valueINT;
			cap.CapCode = CAP_BUFFER_MAX;
			Codec_GetCap(codec, &cap);
			max = cap.cap.valueINT;
			break;
		default:
			min = max = 0;
		}
		if ((codec->type==M4ST_AUDIO) && (max<2)) max = 2;

		/*setup CB*/
		if (!codec->CB && max) {
			codec->CB = CB_New(CUsize, max);
			codec->CB->Min = min;
			codec->CB->odm = codec->odm;
		}

		/*check re-ordering*/
		cap.CapCode = CAP_VID_REORDER;
		Codec_GetCap(codec, &cap);
		if (cap.cap.valueINT) codec->is_reordering = 1;
	}

	/*assign the first base layer as the codec clock by default, or current channel clock if no clock set
	Also assign codec priority here*/
	if (!ch->esd->dependsOnESID || !codec->ck) {
		codec->ck = ch->clock;
		codec->Priority = ch->esd->streamPriority;
		/*insert base layer first - note we are sure this is a stream of the same type
		as the codec (other streams - OCI, MPEG7, MPEGJ - are not added that way)*/
		return ChainInsertEntry(codec->inChannels, ch, 0);
	}
	else {
		/*make sure all channels are in order*/
		for (i=0; i<ChainGetCount(codec->inChannels); i++) {
			a_ch = ChainGetEntry(codec->inChannels, i);
			if (ch->esd->dependsOnESID == a_ch->esd->ESID) {
				return ChainInsertEntry(codec->inChannels, ch, i+1);
			}
			if (a_ch->esd->dependsOnESID == ch->esd->ESID) {
				return ChainInsertEntry(codec->inChannels, ch, i);
			}
		}
		/*by default append*/
		return ChainAddEntry(codec->inChannels, ch);
	}
}

Bool Codec_RemoveChannel(GenericCodec *codec, struct _es_channel *ch)
{
	s32 i;
	i = ChainFindEntry(codec->inChannels, ch);
	if (i>=0) {
		if (codec->decio) codec->decio->DetachStream(codec->decio, ch->esd->ESID);
		ChainDeleteEntry(codec->inChannels, (u32) i);
		return 1;
	}
	return 0;
}


static void codec_update_stats(GenericCodec *codec, u32 dataLength, u32 dec_time)
{
	codec->total_dec_time += dec_time;
	codec->nb_dec_frames++;
	if (dec_time>codec->max_dec_time) codec->max_dec_time = dec_time;

	if (dataLength) {
		u32 now = CK_GetTime(codec->ck);
		if (codec->last_stat_start + 1000 <= now) {
			if (!codec->cur_bit_size) {
				codec->last_stat_start = now;
			} else {
				codec->avg_bit_rate = codec->cur_bit_size;
				if (codec->avg_bit_rate > codec->max_bit_rate) codec->max_bit_rate = codec->avg_bit_rate;
				codec->last_stat_start = now;
				codec->cur_bit_size = 0;
			}
		}
		codec->cur_bit_size += 8*dataLength;
	}
}

/*scalable browsing of input channels: find the AU with the lowest DTS on all input channels*/
void Decoder_GetNextAU(GenericCodec *codec, Channel **activeChannel, LPAUBUFFER *nextAU)
{
	Channel *ch;
	LPAUBUFFER AU;
	u32 count, minDTS, i;
	count = ChainGetCount(codec->inChannels);
	*nextAU = NULL;
	*activeChannel = NULL;

	if (!count) return;

	minDTS = (u32) -1;
	/*reverse browsing to make sure we fill enhancement before base layer*/
	for (i=count;i>0;i--) {
		ch = ChainGetEntry(codec->inChannels, i-1);

		if ((codec->type==M4ST_OCR) && ch->IsClockInit) {
			/*check duration - we assume that scalable OCR streams are just pure nonsense...*/
			if (ch->is_pulling && codec->odm->duration) {
				if (CK_GetTime(codec->ck) > codec->odm->duration) 
					Channel_EndOfStream(ch);
			}
			return;
		}

		AU = Channel_GetAU(ch);
		if (!AU) {
			if (! (*activeChannel)) *activeChannel = ch;
			continue;
		}

		/*we use <= to make sure we first fetch data on base layer if same
		DTS (which is possible in spatial scalability)*/
		if (AU->DTS <= minDTS) {
			minDTS = AU->DTS;
			*activeChannel = ch;
			*nextAU = AU;
		}
	}

	/*FIXME - we're breaking sync (couple of frames delay)*/
	if (*nextAU && codec->is_reordering) (*nextAU)->CTS = (*nextAU)->DTS;
}


M4Err SystemCodec_Process(GenericCodec *codec, u32 TimeAvailable)
{
	LPAUBUFFER AU;
	Channel *ch;
	u32 now, obj_time;
	Bool scene_locked, invalidate_scene;
	Bool check_next_unit;
	SceneDecoder *sdec = (SceneDecoder *)codec->decio;
	M4Err e = M4OK;

	scene_locked = 0;
	invalidate_scene = 0;
	
	/*for resync, if needed - the logic behind this is that there is no composition memory on sytems codecs so
	"frame dropping" is done by preventing the renderer from redrawing after an update and decoding following AU
	so that the renderer is always woken up once all late systems AUs are decoded. This flag is overriden when 
	seeking*/
	check_next_unit = codec->odm->term->bifs_can_resync;
	
check_unit:

	/*muting systems codec means we don't decode until mute is off - likely there will be visible however
	there is no other way to decode system AUs without modifying the content, which is what mute is about on visual...*/
	if (codec->Muted) goto exit;

	/*fetch next AU in DTS order for this codec*/
	Decoder_GetNextAU(codec, &ch, &AU);

	/*get the object time*/
	obj_time = CK_GetTime(codec->ck);

	/*no active channel return*/
	if (!AU || !ch) {
		/*if the codec is in EOS state, move to STOP*/
		if (codec->Status == CODEC_EOS) {
			CapObject cap;
			cap.CapCode = CAP_MEDIA_NOT_OVER;
			cap.cap.valueINT = 0;
			sdec->GetCapabilities(codec->decio, &cap);
			if (!cap.cap.valueINT) MM_StopCodec(codec);
		}
		goto exit;
	}
	

	/*clock is not started*/
	if (ch->first_au_fetched && !CK_IsStarted(ch->clock)) goto exit;

	/*check timing based on the input channel and main FPS*/
	if ( (AU->DTS > obj_time + codec->odm->term->half_frame_duration) ) goto exit;

	/*check seeking and update timing - do NOT use the base layer, since BIFS streams may depend on other 
	streams not on the same clock*/
	if (codec->last_unit_cts == AU->CTS ) {
		/*hack for RTSP streaming of systems streams, except InputSensor*/
		if (!ch->is_pulling && (codec->type != M4ST_INTERACT) && (AU->dataLength == codec->prev_au_size)) {
			Channel_DropAU(ch);
			goto check_unit;
		}
		/*seeking for systems is done by not releasing the graph until seek is done*/
		check_next_unit = 1;
	} 
	/*set system stream timing*/
	else {
		codec->last_unit_cts = AU->CTS;
	}

	/*lock scene*/
	if (!scene_locked) {
		Term_LockScene(codec->odm->term, 1);
		scene_locked = 1;
	}

	/*current media time for system objects is the clock time, since the media is likely to have random 
	updates in time*/
	codec->odm->current_time = CK_GetTime(codec->ck);

	now = Term_GetTime(codec->odm->term);
	e = sdec->ProcessData(sdec, (unsigned char *) AU->data, AU->dataLength, ch->esd->ESID, codec->odm->current_time, MM_LEVEL_NORMAL);
	now = Term_GetTime(codec->odm->term) - now;

	codec_update_stats(codec, AU->dataLength, now);
	codec->prev_au_size = AU->dataLength;

	/*destroy this AU*/
	Channel_DropAU(ch);

	if (e) goto exit;
	/*dynamic OD stream, regenerate scene*/
	if (codec->flags & COD_IS_STATIC_OD) IS_RegenerateScene(codec->odm->subscene ? codec->odm->subscene : codec->odm->parentscene);

	/*always force redraw for system codecs*/
	invalidate_scene = 1;

	/*if no release restart*/
	if (check_next_unit) goto check_unit;
	
exit:
	if (scene_locked) Term_LockScene(codec->odm->term, 0);
	if (invalidate_scene) Term_InvalidateScene(codec->odm->term);
	return e;
}



/*special handling of decoders not using ESM*/
M4Err PrivateScene_Process(GenericCodec *codec, u32 TimeAvailable)
{
	Bool resume_clock;
	u32 now;
	Channel *ch;
	SceneDecoder *sdec = (SceneDecoder *)codec->decio;
	M4Err e = M4OK;
	
	/*muting systems codec means we don't decode until mute is off - likely there will be visible however
	there is no other way to decode system AUs without modifying the content, which is what mute is about on visual...*/
	if (codec->Muted) return M4OK;

	if (codec->Status == CODEC_EOS) {
		MM_StopCodec(codec);
		return M4OK;
	}

	ch = ChainGetEntry(codec->inChannels, 0);
	if (!ch) return M4OK;
	resume_clock = 0;
	/*init channel clock*/
	if (!ch->IsClockInit) {
		Channel_InitDummy(ch);
		if (!CK_IsStarted(ch->clock)) return M4OK;
		/*let's be nice to the scene loader (that usually involves quite some parsing), pause clock while
		parsing*/
		CK_Pause(ch->clock);
		codec->last_unit_dts = 0;
	}

	codec->odm->current_time = codec->last_unit_cts = CK_GetTime(codec->ck);

	/*lock scene*/
	Term_LockScene(codec->odm->term, 1);
	now = Term_GetTime(codec->odm->term);
	e = sdec->ProcessData(sdec, NULL, codec->odm->current_time, ch->esd->ESID, codec->odm->current_time, MM_LEVEL_NORMAL);
	now = Term_GetTime(codec->odm->term) - now;
	codec->last_unit_dts ++;
	/*resume on error*/
	if (e && (codec->last_unit_dts<2) ) {
		CK_Resume(ch->clock);
		codec->last_unit_dts = 2;
	}
	/*resume clock on 2nd decode (we assume parsing is done in 2 steps, one for first frame display, one for complete parse)*/
	else if (codec->last_unit_dts==2) {
		CK_Resume(ch->clock);
	}

	codec_update_stats(codec, 0, now);

	Term_LockScene(codec->odm->term, 0);
	Term_InvalidateScene(codec->odm->term);

	if (e==M4EOF) {
		/*first end of stream, evaluate duration*/
		if (!codec->odm->duration) ODM_SetDuration(codec->odm, ch, codec->odm->current_time);
		Channel_EndOfStream(ch);
		return M4OK;
	}
	return e;
}
/*Get a pointer to the next CU buffer*/
static M4INLINE M4Err LockCompositionUnit(GenericCodec *dec, u32 CU_TS, char **outBuffer, u32 *availableSize)
{
	LPCUBUFFER cu;
	*outBuffer = NULL;
	*availableSize = 0;

	if (!dec->CB) return M4BadParam;
	
	cu = CB_LockInput(dec->CB, CU_TS);
	if (!cu ) return M4OutOfMem;
	
	cu->TS = CU_TS;
	*outBuffer = cu->data;
	*availableSize = dec->CB->UnitSize;
	return M4OK;
}


static M4INLINE M4Err UnlockCompositionUnit(GenericCodec *dec, u32 CTS, u32 NbBytes)
{
	/*temporal scalability disabling: if we already rendered this, no point getting further*/
	if (CTS < dec->CB->LastRenderedTS) NbBytes = 0;

	/*unlock the CB*/
	CB_UnlockInput(dec->CB, CTS, NbBytes);
	return M4OK;
}


static M4Err ResizeCompositionBuffer(GenericCodec *dec, u32 NewSize)
{
	if (!dec || !dec->CB) return M4BadParam;
	
	/*update config*/
	MO_UpdateCaps(dec->odm->mo);

	/*bytes per sec not available: either video or audio not configured*/
	if (!dec->bytes_per_sec) {
		if (NewSize && (NewSize != dec->CB->UnitSize) ) CB_ResizeBuffers(dec->CB, NewSize);
	} 
	/*audio: make sure we have enough data in CM to entirely fill the HW audio buffer...*/
	else {
		u32 unit_size, audio_buf_len, unit_count;
		CapObject cap;
		unit_size = NewSize;
		audio_buf_len = SR_GetAudioBufferLength(dec->odm->term->renderer);
		if (audio_buf_len < 400) audio_buf_len = 400;
		cap.CapCode = CAP_BUFFER_MAX;
		Codec_GetCap(dec, &cap);
		unit_count = cap.cap.valueINT;
		/*at least 2 units for dec and render ...*/
		if (unit_count<2) unit_count = 2;
		while (unit_size*unit_count*1000 < dec->bytes_per_sec*audio_buf_len) unit_count++;

		CB_Reinit(dec->CB, unit_size, unit_count);
		dec->CB->Min = unit_size/3;
		if (!dec->CB->Min) dec->CB->Min = 1;
	}
	if ((dec->type==M4ST_VISUAL) && dec->odm->parentscene->is_dynamic_scene) {
		IS_ForceSceneSize(dec->odm->parentscene, dec->odm->mo->width, dec->odm->mo->height);
	}
	return M4OK;
}

M4Err MediaCodec_Process(GenericCodec *codec, u32 TimeAvailable)
{
	LPAUBUFFER AU;
	Channel *ch;
	char *cu_buf;
	u32 cu_buf_size, mmlevel, deltaTS;
	u32 first, entryTime, now, obj_time;
	MediaDecoder *mdec = (MediaDecoder*)codec->decio;
	M4Err e = M4OK;

	/*if video codec muted don't decode
	if audio codec muted we dispatch to keep sync in place*/
	if (codec->Muted && (codec->type==M4ST_VISUAL) ) return M4OK;

	entryTime = Term_GetTime(codec->odm->term);

	/*fetch next AU in DTS order for this codec*/
	Decoder_GetNextAU(codec, &ch, &AU);
	/*no active channel return*/
	if (!AU || !ch) {
		/*if the codec is in EOS state, assume we're done*/
		if (codec->Status == CODEC_EOS) {
			/*if codec is reordering, try to flush it*/
			if (codec->is_reordering) {
				if ( LockCompositionUnit(codec, codec->last_unit_cts+1, &cu_buf, &cu_buf_size) == M4OutOfMem) 
					return M4OK;
				e = mdec->ProcessData(mdec, NULL, 0, 0, cu_buf, &cu_buf_size, 0, 0);
				if (e==M4OK) e = UnlockCompositionUnit(codec, codec->last_unit_cts+1, cu_buf_size);
			}
			MM_StopCodec(codec);
			if (codec->CB) CB_SetEndOfStream(codec->CB);
		}
		/*if no data, and channel not buffering, ABORT CB buffer (data timeout or EOS not detectable)*/
		else if (ch && !ch->BufferOn) 
			CB_AbortBuffering(codec->CB);
		return M4OK;
	}

	/*get the object time*/
	obj_time = CK_GetTime(codec->ck);
	/*Media Time for media codecs is updated in the CB*/

	if (!codec->CB) {
		Channel_DropAU(ch);
		return M4BadParam;
	}
	
	/*try to refill the full buffer*/
	first = 1;
	while (codec->CB->Capacity > codec->CB->UnitCount) {
		/*set media processing level*/
		mmlevel = MM_LEVEL_NORMAL;
		/*SEEK: if the last frame had the same TS, we are seeking. Ask the codec to drop*/
		if (!ch->skip_sl && codec->last_unit_cts && (codec->last_unit_cts == AU->CTS) && !ch->esd->dependsOnESID) {
			mmlevel = MM_LEVEL_SEEK;
		}
		/*only perform drop in normal playback*/
		else if (codec->CB->Status == CB_PLAY) {
			if (!ch->skip_sl && (AU->CTS < obj_time)) {
				/*extremely late, even if we decode the renderer will drop the frame 
				so set the level to drop*/
				mmlevel = MM_LEVEL_DROP;
			}
			/*we are late according to the media manager*/
			else if (codec->PriorityBoost) {
				mmlevel = MM_LEVEL_VERY_LATE;
			}
			/*otherwise we must have an idea of the load in order to set the right level
			use the composition buffer for that, only on the first frame*/
			else if (first) {
				//if the CB is almost empty set to very late
				if (codec->CB->UnitCount <= codec->CB->Min+1) {
					mmlevel = MM_LEVEL_VERY_LATE;
				} else if (codec->CB->UnitCount * 2 <= codec->CB->Capacity) {
					mmlevel = MM_LEVEL_LATE;
				}
				first = 0;
			}
		}

		/*when using temporal scalability make sure we can decode*/
		if (ch->esd->dependsOnESID && (codec->last_unit_dts > AU->DTS)){
//			printf("SCALABLE STREAM DEAD!!\n");
			goto drop;
		}

		if (ch->skip_sl) {
			if (codec->bytes_per_sec) {
				AU->CTS = codec->last_unit_cts + ch->ts_offset + codec->cur_audio_bytes * 1000 / codec->bytes_per_sec;
			} else if (codec->fps) {
				AU->CTS = codec->last_unit_cts + ch->ts_offset + (u32) (codec->cur_video_frames * 1000 / codec->fps);
			}
		}

		if ( LockCompositionUnit(codec, AU->CTS, &cu_buf, &cu_buf_size) == M4OutOfMem) 
			return M4OK;

		now = Term_GetTime(codec->odm->term);

		e = mdec->ProcessData(mdec, AU->data, AU->dataLength, 
			ch->esd->ESID, cu_buf, &cu_buf_size, AU->PaddingBits, mmlevel);
		now = Term_GetTime(codec->odm->term) - now;

		/*input is too small, resize composition memory*/
		switch (e) {
		case M4BufferTooSmall:
			/*release but no dispatch*/
			UnlockCompositionUnit(codec, AU->CTS, 0);
			if (ResizeCompositionBuffer(codec, cu_buf_size)==M4OK) continue;
			break;
		case M4OK:
			/*in seek don't dispatch any output*/
			if (mmlevel == MM_LEVEL_SEEK) cu_buf_size = 0;
			e = UnlockCompositionUnit(codec, AU->CTS, cu_buf_size);
			codec_update_stats(codec, AU->dataLength, now);
			if (ch->skip_sl) {
				if (codec->bytes_per_sec) {
					codec->cur_audio_bytes += cu_buf_size;
					while (codec->cur_audio_bytes>codec->bytes_per_sec) {
						codec->cur_audio_bytes -= codec->bytes_per_sec;
						codec->last_unit_cts += 1000;
					}
				} else if (codec->fps && cu_buf_size) {
					codec->cur_video_frames += 1;
				}
			}
			break;
		/*this happens a lot when using non-MPEG-4 streams (ex: ffmpeg demuxer)*/
		case M4PackedFrames:
			/*in seek don't dispatch any output*/
			if (mmlevel	== MM_LEVEL_SEEK) cu_buf_size = 0;
			e = UnlockCompositionUnit(codec, AU->CTS, cu_buf_size);

			if (ch->skip_sl) {
				if (codec->bytes_per_sec) {
					codec->cur_audio_bytes += cu_buf_size;
				} else if (codec->fps && cu_buf_size) {
					codec->cur_video_frames += 1;
				}
			} else {
				if (codec->bytes_per_sec) {
					deltaTS = cu_buf_size * 1000 / codec->bytes_per_sec;
				} else if (codec->fps && cu_buf_size) {
					deltaTS = (u32) (1000.0f / codec->fps);
				}
				AU->DTS += deltaTS;
				AU->CTS += deltaTS;
			}
			codec_update_stats(codec, 0, now);
			continue;
		default:
			UnlockCompositionUnit(codec, AU->CTS, 0);
			/*error - if the object is in intitial buffering resume it!!*/
			CB_AbortBuffering(codec->CB);
			break;
		}

		codec->last_unit_dts = AU->DTS;
		/*remember base layer timing*/
		if (!ch->esd->dependsOnESID && !ch->skip_sl) codec->last_unit_cts = AU->CTS;

drop:
		Channel_DropAU(ch);
		if (e) return e;

		/*escape from decoding loop only if above critical limit - this is to avoid starvation on audio*/
		if (codec->CB->UnitCount > codec->CB->Min) {
			now = Term_GetTime(codec->odm->term);
			if (now - entryTime >= TimeAvailable - TIME_CHECK) {
				return M4OK;
			}
		}

		Decoder_GetNextAU(codec, &ch, &AU);
		if (!ch || !AU) return M4OK;
	}
	return M4OK;
}



M4Err Decoder_ProcessData(GenericCodec *codec, u32 TimeAvailable)
{
	if (codec->Status == CODEC_STOP) return M4OK;
	codec->Muted = (codec->odm->media_ctrl && codec->odm->media_ctrl->control->mute) ? 1 : 0;

	/*OCR: needed for OCR in pull mode (dummy streams used to sync various sources)*/
	if (codec->type==M4ST_OCR) {
		LPAUBUFFER AU;
		Channel *ch;
		/*fetch next AU on OCR (empty AUs)*/
		Decoder_GetNextAU(codec, &ch, &AU);

		/*no active channel return*/
		if (!AU || !ch) {
			/*if the codec is in EOS state, move to STOP*/
			if (codec->Status == CODEC_EOS) {
				MM_StopCodec(codec);
				/*if a mediacontrol is ruling this OCR*/
				if (codec->odm->media_ctrl && codec->odm->media_ctrl->control->loop) MC_Restart(codec->odm); 
			}
		}
	}
	/*special case here (we tweak a bit the timing)*/
	else if (codec->type==M4ST_PRIVATE_SCENE) {
		return PrivateScene_Process(codec, TimeAvailable);
	} else if (codec->decio->InterfaceType==M4MEDIADECODERINTERFACE) {
		return MediaCodec_Process(codec, TimeAvailable);
	} else if (codec->decio->InterfaceType==M4SCENEDECODERINTERFACE) {
		return SystemCodec_Process(codec, TimeAvailable);
	}
	return M4OK;
}


M4Err Codec_GetCap(GenericCodec *codec, CapObject *cap)
{
	cap->cap.valueINT = 0;
	if (!codec->decio) return M4OK;
	return codec->decio->GetCapabilities(codec->decio, cap);
}

M4Err Codec_SetCap(GenericCodec *codec, CapObject cap)
{
	if (!codec->decio) return M4OK;
	return codec->decio->SetCapabilities(codec->decio, cap);
}


void Codec_SetStatus(GenericCodec *codec, u32 Status)
{
	if (!codec) return;

	if (Status == CODEC_PAUSE) codec->Status = CODEC_STOP;
	else if (Status == CODEC_BUFFER) codec->Status = CODEC_PLAY;
	else if (Status == CODEC_PLAY) {
		codec->last_unit_cts = 0;
		codec->prev_au_size = 0;
		codec->Status = Status;
		codec->last_stat_start = codec->cur_bit_size = codec->max_bit_rate = codec->avg_bit_rate = 0;
		codec->nb_dec_frames = codec->total_dec_time = codec->max_dec_time = 0;
		codec->cur_audio_bytes = codec->cur_video_frames = 0;
	}
	else codec->Status = Status;

	if (!codec->CB) return;
	
	/*notify CB*/
	switch (Status) {
	case CODEC_PLAY:
		CB_SetStatus(codec->CB, CB_PLAY);
		return;
	case CODEC_PAUSE:
		CB_SetStatus(codec->CB, CB_PAUSE);
		return;
	case CODEC_STOP:
		CB_SetStatus(codec->CB, CB_STOP);
		return;
	case CODEC_EOS:
		/*do NOT notify CB yet, wait till last AU is decoded*/
		return;
	case CODEC_BUFFER:
	default:
		return;
	}
}

M4Err Codec_LoadPlugin(GenericCodec *codec, ESDescriptor *esd, u32 PL)
{
	char szPrefDec[500];
	char *sOpt;
	BaseDecoder *ifce;
	u32 i, plugCount;
	u32 ifce_type;
	char *cfg;
	u32 cfg_size;
	M4Client *term = codec->odm->term;


	if (esd->decoderConfig->decoderSpecificInfo) {
		cfg = esd->decoderConfig->decoderSpecificInfo->data;
		cfg_size = esd->decoderConfig->decoderSpecificInfo->dataLength;
	} else {
		cfg = NULL;
		cfg_size = 0;
	}

	ifce_type = M4SCENEDECODERINTERFACE;
	if ((esd->decoderConfig->streamType==M4ST_AUDIO) || (esd->decoderConfig->streamType==M4ST_VISUAL))
		ifce_type = M4MEDIADECODERINTERFACE;

	/*a bit dirty, if FFMPEG is used for demuxer load it for decoder too*/
	if (0 && !stricmp(codec->odm->net_service->ifce->plugin_name, "FFMPEG demuxer")) {
		sOpt = "FFMPEG decoder";
	} else {
		/*use user-defined plugin if any*/
		sOpt = NULL;
		switch (esd->decoderConfig->streamType) {
		case M4ST_VISUAL:
			sOpt = IF_GetKey(term->user->config, "Systems", "DefVideoDec");
			break;
		case M4ST_AUDIO:
			sOpt = IF_GetKey(term->user->config, "Systems", "DefAudioDec");
			break;
		default:
			break;
		}
	}
	
	if (sOpt) {
		if (PM_LoadInterfaceByName(term->user->plugins, sOpt, ifce_type, (void **) &ifce)) {
			if (ifce->CanHandleStream && ifce->CanHandleStream(ifce, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication, cfg, cfg_size, PL) ) {
				codec->decio = ifce;
				return M4OK;
			}
			PM_ShutdownInterface(ifce);		
		}
	}

	/*prefered codec plugin per streamType/objectType from config*/
	sprintf(szPrefDec, "codec_%.2x_%.2x", esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication);
	sOpt = IF_GetKey(term->user->config, "Systems", szPrefDec);
	if (sOpt) {
		if (PM_LoadInterfaceByName(term->user->plugins, sOpt, ifce_type, (void **) &ifce)) {
			if (ifce->CanHandleStream && ifce->CanHandleStream(ifce, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication, cfg, cfg_size, PL) ) {
				codec->decio = ifce;
				return M4OK;
			}
			PM_ShutdownInterface(ifce);		
		}
	}
	/*not found, check all plugins*/
	plugCount = PM_GetPluginsCount(term->user->plugins);
	for (i = 0; i < plugCount ; i++) {
		if (!PM_LoadInterface(term->user->plugins, i, ifce_type, (void **) &ifce)) continue;
		if (ifce->CanHandleStream && ifce->CanHandleStream(ifce, esd->decoderConfig->streamType, esd->decoderConfig->objectTypeIndication, cfg, cfg_size, PL) ) {
			codec->decio = ifce;
			return M4OK;
		}
		PM_ShutdownInterface(ifce);
	}
	return M4CodecNotFound;
}

M4Err Codec_Load(GenericCodec *codec, ESDescriptor *esd, u32 PL)
{
	switch (esd->decoderConfig->streamType) {
	/*OCR has no codec, just a channel*/
	case M4ST_OCR:
		codec->decio = NULL;
		return M4OK;
	/*InteractionStream is currently hardcoded*/
	case M4ST_INTERACT:
		codec->decio = (BaseDecoder *) NewISCodec(PL);
		assert(codec->decio->InterfaceType == M4SCENEDECODERINTERFACE);
		return M4OK;

	/*load decoder plugin*/
	default:
		return Codec_LoadPlugin(codec, esd, PL);
	}
}


void DeleteCodec(GenericCodec *codec)
{
	if (ChainGetCount(codec->inChannels)) return;

	if (!(codec->flags & COD_IS_USE)) {
		switch (codec->type) {
		/*input sensor streams are handled internally for now*/
		case M4ST_INTERACT:
			MX_P(codec->odm->term->net_mx);
			ISDec_Delete(codec->decio);
			ChainDeleteItem(codec->odm->term->input_streams, codec);
			MX_V(codec->odm->term->net_mx);
			break;
		default:
			PM_ShutdownInterface(codec->decio);
			break;
		}
	}
	if (codec->CB) CB_Delete(codec->CB);
	DeleteChain(codec->inChannels);
	free(codec);
}


