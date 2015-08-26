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

#include <intern/m4_esm_dev.h>

Clock *NewClock(M4Client *term)
{
	Clock *tmp = malloc(sizeof(Clock));
	if (!tmp) return NULL;
	memset(tmp, 0, sizeof(Clock));
	tmp->mx = NewMutex();
	tmp->term = term;
	tmp->speed = 1.0;
	return tmp;
}

void DeleteClock(Clock *ck)
{
	MX_Delete(ck->mx);
	free(ck);
}

Clock *CK_FindClock(Chain *Clocks, u16 clockID, u16 ES_ID)
{
	u32 i;
	Clock *tmp;
	for (i = 0; i< ChainGetCount(Clocks); i++) {
		tmp = ChainGetEntry(Clocks, i);
		//first check the clock ID
		if (tmp->clockID == clockID) return tmp;
		//then check the ES ID
		if (ES_ID && (tmp->clockID == ES_ID)) return tmp;
	}
	//no clocks found...
	return NULL;
}

Clock *CK_LookForClockDep(struct _inline_scene *is, u16 clockID)
{
	u32 i, j;
	/*check in top OD*/
	for (i=0; i<ChainGetCount(is->root_od->channels); i++) {
		Channel *ch = ChainGetEntry(is->root_od->channels, i);
		if (ch->esd->ESID == clockID) return ch->clock;
	}
	/*check in sub ODs*/
	for (j=0; j<ChainGetCount(is->ODlist); j++) {
		ODManager *odm = ChainGetEntry(is->ODlist, j);
		for (i=0; i<ChainGetCount(odm->channels); i++) {
			Channel *ch = ChainGetEntry(odm->channels, i);
			if (ch->esd->ESID == clockID) return ch->clock;
		}
	}
	return NULL;
}

/*remove clocks created due to out-of-order OCR dependencies*/
void CK_ResolveClockDep(Chain *clocks, struct _inline_scene *is, Clock *ck, u16 Clock_ESID)
{
	u32 i, j;

	/*check all channels - if any is using a clock which ID is the clock_ESID then
	this clock shall be removed*/
	for (i=0; i<ChainGetCount(is->root_od->channels); i++) {
		Channel *ch = ChainGetEntry(is->root_od->channels, i);
		if (ch->clock->clockID == Clock_ESID) {
			if (is->scene_codec && is->scene_codec->ck == ch->clock) is->scene_codec->ck = ck;
			if (is->od_codec && is->od_codec->ck == ch->clock) is->od_codec->ck = ck;
			if (is->root_od->oci_codec && is->root_od->oci_codec->ck == ch->clock) is->root_od->oci_codec->ck = ck;
			ch->clock = ck;
			if (ch->esd) ch->esd->OCRESID = ck->clockID;
		}
	}
	for (j=0; j<ChainGetCount(is->ODlist); j++) {
		ODManager *odm = ChainGetEntry(is->ODlist, j);
		for (i=0; i<ChainGetCount(odm->channels); i++) {
			Channel *ch = ChainGetEntry(odm->channels, i);
			if (ch->clock->clockID == Clock_ESID) {
				if (odm->codec && (odm->codec->ck==ch->clock)) odm->codec->ck = ck;
				if (odm->oci_codec && (odm->oci_codec->ck==ch->clock)) odm->oci_codec->ck = ck;
				ch->clock = ck;
				if (ch->esd) ch->esd->OCRESID = ck->clockID;
			}
		}
	}
	/*destroy clock*/
	for (i=0; i<ChainGetCount(clocks); i++) {
		Clock *clock = ChainGetEntry(clocks, i);
		if (clock->clockID == Clock_ESID) {
			ChainDeleteEntry(clocks, i);
			DeleteClock(clock);
			return;
		}
	}
}

Clock *CK_AttachClock(Chain *clocks, struct _inline_scene *is, u16 clockID, u16 ES_ID, s32 hasOCR)
{
	Bool check_dep;
	Clock *tmp = CK_FindClock(clocks, clockID, ES_ID);
	/*ck dep can only be solved if in the main service*/
	check_dep = (is->root_od->net_service->Clocks==clocks) ? 1 : 0;
	/*this partly solves a->b->c*/
	if (!tmp && check_dep) tmp = CK_LookForClockDep(is, clockID);
	if (!tmp) {
		tmp = NewClock(is->root_od->term);
		tmp->clockID = clockID;
		ChainAddEntry(clocks, tmp);
	} else {
		if (tmp->clockID == ES_ID) tmp->clockID = clockID;
		/*this finally solves a->b->c*/
		if (check_dep && (tmp->clockID != ES_ID)) CK_ResolveClockDep(clocks, is, tmp, ES_ID);
	}
	if (hasOCR >= 0) tmp->use_ocr = hasOCR;
	return tmp;
}

void CK_Reset(Clock *ck)
{
	ck->clock_init = 0;
	ck->drift = 0;
	ck->discontinuity_time = 0;
	//do NOT reset buffering flag, because RESET is called only 
	//for the stream owning the clock, and other streams may 
	//have signaled buffering on this clock
	ck->init_time = 0;
	ck->StartTime = 0;
	ck->has_seen_eos = 0;
}

void CK_SetTime(Clock *ck, u32 TS)
{
	if (!ck->clock_init) {
		ck->init_time = TS;
		ck->clock_init = 1;
		/*update starttime and pausetime even in pause mode*/
		ck->PauseTime = ck->StartTime = Term_GetTime(ck->term);
		ck->drift = 0;
	}
	/*TODO: test with pure OCR streams*/
	if (ck->use_ocr) {
		/*just update the drift - we could also apply a drift algo*/
		u32 now = CK_GetTime(ck);
		s32 drift = now - TS;
		ck->drift += drift;
	}
}



void CK_Pause(Clock *ck)
{
	MX_P(ck->mx);
	if (!ck->Paused) ck->PauseTime = Term_GetTime(ck->term);
	ck->Paused += 1;
	MX_V(ck->mx);
}

void CK_Resume(Clock *ck)
{
	MX_P(ck->mx);
	assert(ck->Paused);
	ck->Paused -= 1;
	if (!ck->Paused) 
		ck->StartTime += Term_GetTime(ck->term) - ck->PauseTime;
	MX_V(ck->mx);
}


u32 CK_GetRealTime(Clock *ck)
{
	u32 time;
	if (!ck || !ck->StartTime) return 0;
	time = ck->Paused > 0 ? ck->PauseTime : Term_GetTime(ck->term);
	time = ck->discontinuity_time + ck->init_time + (u32) (ck->speed * (time - ck->StartTime));
	return time;
}

u32 CK_GetTime(Clock *ck)
{
	u32 time = CK_GetRealTime(ck);
	if ((s32) time < ck->drift) return 0;
	return time - ck->drift;
}


Bool CK_IsStarted(Clock *ck)
{
	if (!ck->StartTime || ck->Buffering || ck->Paused) return 0;
	return 1;
}

/*buffering is protected by a mutex because it may be triggered by composition memory (audio or visual threads)*/
void CK_BufferOn(Clock *ck)
{
	MX_P(ck->mx);
	if (!ck->Buffering) CK_Pause(ck);
	ck->Buffering += 1;
	MX_V(ck->mx);
}

void CK_BufferOff(Clock *ck)
{
	MX_P(ck->mx);
	assert(ck->Buffering);
	if (ck->Buffering) {
		ck->Buffering -= 1;
		if (!ck->Buffering) CK_Resume(ck);
	}
	MX_V(ck->mx);
}


void CK_SetSpeed(Clock *ck, Float speed)
{
	u32 time;
	if (speed==ck->speed) return;
	time = Term_GetTime(ck->term);
	/*adjust start time*/
	ck->discontinuity_time = CK_GetTime(ck);
	ck->PauseTime = ck->StartTime = time;
	ck->speed = speed;
}

void CK_AdjustDrift(Clock *ck, s32 ms_drift)
{
	assert(ck);
	ck->drift = ms_drift;
}
