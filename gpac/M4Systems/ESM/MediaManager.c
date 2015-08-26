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

//#define NO_MM_LOCK

u32 MM_Loop(void *par);

typedef struct
{
	M4Thread *th;
	/*stop/start access mutex*/
	M4Mutex *mx;
	Bool thread_exit;
	u32 is_threaded;
	u32 dec_wants_threading;
	GenericCodec *dec;
	Bool is_running;
	MediaManager *mm;
} CodecEntry;

MediaManager *NewMediaManager(M4Client *term, u32 threading_mode)
{
	MediaManager *tmp = malloc(sizeof(MediaManager));
	memset(tmp, 0, sizeof(MediaManager));
	tmp->mm_mx = NewMutex();
	tmp->threaded_codecs = NewChain();
	tmp->unthreaded_codecs = NewChain();
	tmp->term = term;

	tmp->interrupt_cycle_ms = 33;
	tmp->threading_mode = threading_mode;
	tmp->th = NewThread();
	tmp->run = 1;
	tmp->exit = 0;
	tmp->priority = TH_PRIOR_NORMAL;
	TH_Run(tmp->th, MM_Loop, tmp);
	return tmp;
}

void MM_Delete(MediaManager *mgr)
{
	mgr->run = 0;
	while (!mgr->exit) Sleep(20);

	assert(! ChainGetCount(mgr->threaded_codecs));
	assert(! ChainGetCount(mgr->unthreaded_codecs));

	DeleteChain(mgr->threaded_codecs);
	DeleteChain(mgr->unthreaded_codecs);
	MX_Delete(mgr->mm_mx);
	TH_Delete(mgr->th);
	free(mgr);
}

static CodecEntry *mm_get_codec(Chain *list, GenericCodec *codec)
{
	u32 i;
	CodecEntry *ce;
	for (i=0; i<ChainGetCount(list); i++) {
		ce = ChainGetEntry(list, i);
		if (ce->dec==codec) return ce;
	}
	return NULL;
}


void MM_AddCodec(MediaManager *mgr, GenericCodec *codec)
{
	u32 i, count;
	CodecEntry *cd;
	CodecEntry *ptr, *next;
	CapObject cap;
	assert(codec);

	/*we need REAL exclusive access when adding a dec*/
#ifndef NO_MM_LOCK
	MX_P(mgr->mm_mx);
#endif

	cd = mm_get_codec(mgr->unthreaded_codecs, codec);
	if (cd) goto exit;

	cd = malloc(sizeof(CodecEntry));
	cd->dec = codec;
	cd->th = NewThread();
	cd->mx = NewMutex();
	cd->mm = mgr;
	cd->is_running = 0;
	cd->thread_exit = 1;

	cap.CapCode = CAP_CODEC_WANTSTHREAD;
	cap.cap.valueINT = 0;
	Codec_GetCap(codec, &cap);
	cd->dec_wants_threading = cap.cap.valueINT;

	switch (mgr->threading_mode) {
	case MM_THREAD_MULTI:
		cd->is_threaded = 1;
		break;
	case MM_THREAD_SINGLE:
		cd->is_threaded = 0;
		break;
	case MM_THREAD_FREE:
	default:
		cd->is_threaded = cd->dec_wants_threading;
		break;
	}
	
	if (cd->is_threaded) {
		ChainAddEntry(mgr->threaded_codecs, cd);
		goto exit;
	}

	//add codec 1- per priority 2- per type, audio being first
	//priorities inherits from Systems (5bits) so range from 0 to 31
	//we sort from MAX to MIN
	count = ChainGetCount(mgr->unthreaded_codecs);
	for (i=0; i<count; i++) {
		ptr = ChainGetEntry(mgr->unthreaded_codecs, i);
		//higher priority, continue
		if (ptr->dec->Priority > codec->Priority) continue;

		//same priority, put audio first
		if (ptr->dec->Priority == codec->Priority) {
			//we insert audio (0x05) before video (0x04)
			if (ptr->dec->type < codec->type) {
				ChainInsertEntry(mgr->unthreaded_codecs, cd, i);
				goto exit;
			}
			//same prior, same type: insert after
			if (ptr->dec->type == codec->type) {
				if (i+1==count) {
					ChainAddEntry(mgr->unthreaded_codecs, cd);
				} else {
					ChainInsertEntry(mgr->unthreaded_codecs, cd, i+1);
				}
				goto exit;
			}
			//we insert video (0x04) after audio (0x05) if next is not audio
			//last one
			if (i+1 == count) {
				ChainAddEntry(mgr->unthreaded_codecs, cd);
				goto exit;
			}
			next = ChainGetEntry(mgr->unthreaded_codecs, i+1);
			//# priority level, insert
			if (next->dec->Priority != codec->Priority) {
				ChainInsertEntry(mgr->unthreaded_codecs, cd, i+1);
				goto exit;
			}
			//same priority level and at least one after : continue
			continue;
		}
		ChainInsertEntry(mgr->unthreaded_codecs, cd, i);
		goto exit;
	}
	//if we got here, nothing in the chain -> add the codec
	ChainAddEntry(mgr->unthreaded_codecs, cd);

exit:
#ifndef NO_MM_LOCK
	MX_V(mgr->mm_mx);
#endif
	return;
}

void MM_RemoveCodec(MediaManager *mgr, GenericCodec *codec)
{
	u32 i;
	CodecEntry *ce;

	/*we need REAL exclusive access when removing a dec*/
#ifndef NO_MM_LOCK
	MX_P(mgr->mm_mx);
#endif

	for (i = 0; i<ChainGetCount(mgr->threaded_codecs); i++) {
		ce = ChainGetEntry(mgr->threaded_codecs, i);
		if (ce->dec == codec) {
			assert(ce->is_threaded);
			ce->is_running = 0;
			while (!ce->thread_exit) Sleep(10);
			TH_Delete(ce->th);
			MX_Delete(ce->mx);
			free(ce);
			ChainDeleteEntry(mgr->threaded_codecs, i);
			goto exit;
		}
	}
	for (i = 0; i<ChainGetCount(mgr->unthreaded_codecs); i++) {
		ce = ChainGetEntry(mgr->unthreaded_codecs, i);
		if (ce->dec == codec) {
			assert (!ce->is_threaded);
			TH_Delete(ce->th);
			MX_Delete(ce->mx);
			free(ce);
			ChainDeleteEntry(mgr->unthreaded_codecs, i);
			break;
		}
	}
exit:
#ifndef NO_MM_LOCK
	MX_V(mgr->mm_mx);
#endif
	return;
}


u32 MM_Loop(void *par)
{
	CodecEntry *ce;
	M4Err e;
	u32 count, current_dec, remain;
	u32 time_taken, time_slice, time_left;

	MediaManager *mgr = (MediaManager *) par;

	current_dec = 0;


	TH_SetPriority(mgr->th, mgr->priority);

	while (mgr->run) {
		Term_HandleServices(mgr->term);
		MX_P(mgr->mm_mx);

		count = ChainGetCount(mgr->unthreaded_codecs);
		time_left = mgr->interrupt_cycle_ms;

		if (!count) {
			MX_V(mgr->mm_mx);
			Sleep(mgr->interrupt_cycle_ms);
			continue;
		}

		if (current_dec >= count) current_dec = 0;
		remain = count;

		/*this is ultra basic a nice scheduling system would be much better*/
		while (remain) {
			ce = ChainGetEntry(mgr->unthreaded_codecs, current_dec);
			if (!ce) break;

			MX_P(ce->mx);
			if (!ce->is_running) {
				MX_V(ce->mx);
				assert(!ce->is_threaded);
				remain--;
				if (!remain) break;
				current_dec = (current_dec + 1) % count;
				continue;
			}
			time_slice = ce->dec->Priority * time_left / mgr->cumulated_priority;
			if (ce->dec->PriorityBoost) time_slice *= 2;
			time_taken = M4_GetSysClock();

			e = Decoder_ProcessData(ce->dec, time_slice);
			MX_V(ce->mx);

			if (e) M4_OnMessage(ce->dec->odm->term, ce->dec->odm->net_service->url, "Decoding Error", e);

			time_taken = M4_GetSysClock() - time_taken;

			if (ce->dec->CB && (ce->dec->CB->UnitCount >= ce->dec->CB->Min)) ce->dec->PriorityBoost = 0;

			remain -= 1;
			if (!remain) break;

			current_dec = (current_dec + 1) % count;

			if (time_left > time_taken) {
				time_left -= time_taken;
			} else {
				break;
			}
		}
		MX_V(mgr->mm_mx);

		if (mgr->term->render_frames) {
			time_taken = M4_GetSysClock();
			SR_RenderFrame(mgr->term->renderer);
			time_taken = M4_GetSysClock() - time_taken;
			if (time_left>time_taken) 
				time_left -= time_taken;
			else
				time_left = 0;
		}

		if (!count) {
			Sleep(mgr->interrupt_cycle_ms);
		} else {
//			while (time_left > mgr->interrupt_cycle_ms) time_left -= mgr->interrupt_cycle_ms;
			Sleep(time_left);
		}
	}
	mgr->exit = 1;
	return 0;
}

void MM_SetInterruptCycleTime(MediaManager *mgr, u32 time)
{
	MX_P(mgr->mm_mx);
	mgr->interrupt_cycle_ms = time;
	MX_V(mgr->mm_mx);
}

u32 RunSingleDec(void *ptr)
{
	M4Err e;
	u32 time_left;
	CodecEntry *ce = (CodecEntry *) ptr;

	while (ce->is_running) {
		time_left = M4_GetSysClock();
		MX_P(ce->mx);
		e = Decoder_ProcessData(ce->dec, ce->mm->interrupt_cycle_ms);
		if (e) M4_OnMessage(ce->dec->odm->term, ce->dec->odm->net_service->url, "Decoding Error", e);
		MX_V(ce->mx);
		time_left = M4_GetSysClock() - time_left;



		/*no priority boost this way for systems codecs, priority is dynamically set by not releasing the 
		graph when late and moving on*/
		if (!ce->dec->CB || (ce->dec->CB->UnitCount >= ce->dec->CB->Min)) 
			ce->dec->PriorityBoost = 0;

		/*while on don't sleep*/
		if (ce->dec->PriorityBoost) continue;

		if (time_left) {
			while (time_left > ce->mm->interrupt_cycle_ms) time_left -= ce->mm->interrupt_cycle_ms;
			Sleep(time_left);
		} else {
			Sleep(ce->mm->interrupt_cycle_ms);
		}
	}
	ce->thread_exit = 1;
	return 0;
}

/*NOTE: when starting/stoping a decoder we only lock the decoder mutex, NOT the media manager. This
avoids deadlocking in case a system codec waits for the scene graph and the renderer requests 
a stop/start on a media*/
void MM_StartCodec(GenericCodec *codec)
{
	CapObject cap;
	CodecEntry *ce;
	MediaManager *mgr = codec->odm->term->mediaman;
	ce = mm_get_codec(mgr->unthreaded_codecs, codec);
	if (!ce) ce = mm_get_codec(mgr->threaded_codecs, codec);
	if (!ce) return;

	/*lock dec*/
	MX_P(ce->mx);

	/*clean decoder memory and wait for RAP*/
	if (codec->CB) CB_Reset(codec->CB);

	cap.CapCode = CAP_WAIT_RAP;
	Codec_SetCap(codec, cap);

	if (codec->decio && codec->decio->InterfaceType == M4SCENEDECODERINTERFACE) {
		cap.CapCode = CAP_SHOW_EXTRASCENE;
		cap.cap.valueINT = 1;
		Codec_SetCap(codec, cap);
	}

	Codec_SetStatus(codec, CODEC_PLAY);

	if (!ce->is_running) {
		if (ce->is_threaded) {
			/*in case the decoder thread hasn't finished yet wait for it*/
			while (!ce->thread_exit) Sleep(10);
			ce->is_running = 1;
			ce->thread_exit = 0;
			TH_Run(ce->th, RunSingleDec, ce);
			TH_SetPriority(ce->th, mgr->priority);
		} else {
			ce->is_running = 1;
			mgr->cumulated_priority += ce->dec->Priority+1;
		}
	}

	/*unlock dec*/
	MX_V(ce->mx);
}

void MM_StopCodec(GenericCodec *codec)
{
	CodecEntry *ce;
	MediaManager *mgr = codec->odm->term->mediaman;
	ce = mm_get_codec(mgr->unthreaded_codecs, codec);
	if (!ce) ce = mm_get_codec(mgr->threaded_codecs, codec);
	if (!ce) return;

	MX_P(ce->mx);
	
	if (codec->decio && codec->decio->InterfaceType == M4SCENEDECODERINTERFACE) {
		CapObject cap;
		cap.CapCode = CAP_SHOW_EXTRASCENE;
		cap.cap.valueINT = 0;
		Codec_SetCap(codec, cap);
	}

	/*set status directly and don't touch CB state*/
	codec->Status = CODEC_STOP;
	if (ce->is_running) {
		ce->is_running = 0;
		if (!ce->is_threaded) {
			mgr->cumulated_priority -= codec->Priority+1;
		}
	}
	MX_V(ce->mx);
}

void MM_SetThreadingMode(MediaManager *mgr, u32 mode)
{
	u32 i;
	
	if (mgr->threading_mode == mode) return;

	/*note we lock global mutex but don't lock any codecs*/
	MX_P(mgr->mm_mx);

	switch (mode) {
	/*moving to no threads*/
	case MM_THREAD_SINGLE:
		while (ChainGetCount(mgr->threaded_codecs)) {
			CodecEntry *ce = ChainGetEntry(mgr->threaded_codecs, 0);
			ChainDeleteEntry(mgr->threaded_codecs, 0);
			ce->is_running = 0;
			while (!ce->thread_exit) Sleep(0);
			mgr->cumulated_priority += ce->dec->Priority+1;
			ce->is_running = 1;
			ce->is_threaded = 0;
			ChainAddEntry(mgr->unthreaded_codecs, ce);
		}
		break;
	/*moving to all threads*/
	case MM_THREAD_MULTI:
		while (ChainGetCount(mgr->unthreaded_codecs)) {
			CodecEntry *ce = ChainGetEntry(mgr->unthreaded_codecs, 0);
			ChainDeleteEntry(mgr->unthreaded_codecs, 0);
			ce->is_running = 0;
			mgr->cumulated_priority -= ce->dec->Priority+1;
			ce->is_running = 1;
			ChainAddEntry(mgr->threaded_codecs, ce);
			ce->thread_exit = 0;
			ce->is_threaded = 1;
			TH_Run(ce->th, RunSingleDec, ce);
			TH_SetPriority(ce->th, mgr->priority);
		}
		break;
	/*moving to free threading*/
	case MM_THREAD_FREE:
	default:
		/*remove all forced-threaded dec*/
		for (i=0; i<ChainGetCount(mgr->threaded_codecs); i++) {
			CodecEntry *ce = ChainGetEntry(mgr->threaded_codecs, i);
			if (ce->dec_wants_threading) continue;
			/*stop it*/
			ce->is_running = 0;
			while (!ce->thread_exit) Sleep(0);
			ce->is_threaded = 0;
			ChainDeleteEntry(mgr->threaded_codecs, i);
			i--;
			/*add to unthreaded list*/
			ce->is_running = 1;
			ChainAddEntry(mgr->unthreaded_codecs, ce);
			mgr->cumulated_priority += ce->dec->Priority+1;
		}
		/*remove all forced unthreaded dec*/
		for (i=0; i<ChainGetCount(mgr->unthreaded_codecs); i++) {
			CodecEntry *ce = ChainGetEntry(mgr->unthreaded_codecs, i);
			if (! ce->dec_wants_threading) continue;
			/*stop it*/
			ce->is_threaded = 1;
			ChainDeleteEntry(mgr->unthreaded_codecs, i);
			i--;
			mgr->cumulated_priority -= ce->dec->Priority+1;

			/*add to unthreaded list*/
			ChainAddEntry(mgr->threaded_codecs, ce);
			ce->is_running = 1;
			ce->thread_exit = 0;
			TH_Run(ce->th, RunSingleDec, ce);
			TH_SetPriority(ce->th, mgr->priority);
		}
	}
	mgr->threading_mode = mode;
	MX_V(mgr->mm_mx);
}

void MM_SetPriority(MediaManager *mgr, s32 Priority)
{
	u32 i;

	MX_P(mgr->mm_mx);

	TH_SetPriority(mgr->th, Priority);

	for (i=0; i<ChainGetCount(mgr->threaded_codecs); i++) {
		CodecEntry *ce = ChainGetEntry(mgr->threaded_codecs, i);
		TH_SetPriority(ce->th, Priority);
	}
	mgr->priority = Priority;

	MX_V(mgr->mm_mx);
}

