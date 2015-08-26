/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / MP4 reader plugin
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


#include "mp4_io.h"
#include <gpac/m4_author.h>


void m4_reset_reader(M4Channel *ch)
{
	memset(&ch->current_slh, 0, sizeof(SLHeader));
	ch->last_state = M4OK;
	if (ch->sample) M4_DeleteSample(&ch->sample);
	ch->sample = NULL;
	ch->sample_num = 0;
	ch->speed = 1.0;
	ch->start = ch->end = 0;
	ch->to_init = 1;
	ch->is_playing = 0;
}


static void init_reader(M4Channel *ch)
{
	u32 ivar;

	ch->current_slh.accessUnitEndFlag = 1;
	ch->current_slh.accessUnitStartFlag = 1;
	ch->current_slh.AU_sequenceNumber = 1;
	ch->current_slh.compositionTimeStampFlag = 1;
	ch->current_slh.decodingTimeStampFlag = 1;
	ch->current_slh.packetSequenceNumber = 1;
	ch->current_slh.randomAccessPointFlag = 0;

	assert(ch->sample==NULL);

	if (ch->is_ocr) {
		assert(!ch->sample);
		ch->sample = M4_NewSample();
		ch->sample->IsRAP = 1;
		ch->sample->DTS = ch->start;
		ch->last_state=M4OK;
	} else {
		/*take care of seeking out of the track range*/
		if (ch->duration<ch->start) {
			ch->last_state = M4_GetSampleForMovieTime(ch->owner->mov, ch->track, ch->duration, &ivar, M4_SearchSyncBackward, &ch->sample, &ch->sample_num);
		} else {
			ch->last_state = M4_GetSampleForMovieTime(ch->owner->mov, ch->track, ch->start, &ivar, M4_SearchSyncBackward, &ch->sample, &ch->sample_num);
		}
	}

	/*no sample means we're not in the track range - stop*/
	if (!ch->sample) {
		/*incomplete file - check if we're still downloading or not*/
		if (M4_GetBytesMissing(ch->owner->mov, ch->track)) {
			u32 net_status;
			NM_GetDownloaderStats(ch->owner->dnload, NULL, NULL, NULL, &net_status);
			if (net_status == DL_Running) {
				ch->last_state = M4OK;
				return;
			}
			ch->last_state = M4UncompleteFile;
		} else if (ch->sample_num) {
			ch->last_state = M4EOF;
		}
	} else {
		ch->sample_time = ch->sample->DTS;
	}
	ch->to_init = 0;
	ch->current_slh.decodingTimeStamp = ch->start;
	ch->current_slh.compositionTimeStamp = ch->start;
	ch->current_slh.randomAccessPointFlag = ch->sample ? ch->sample->IsRAP : 0;

}

void m4_reader_get_sample(M4Channel *ch)
{
	M4Err e;
	u32 ivar;
	if (ch->sample) return;

	/*that's our fake OD stream*/
	if (ch->FAKE_ESID) {
		ch->current_slh.accessUnitEndFlag = 1;
		ch->current_slh.accessUnitStartFlag = 1;
		ch->current_slh.AU_sequenceNumber = 1;
		ch->current_slh.compositionTimeStampFlag = 1;
		ch->current_slh.decodingTimeStampFlag = 1;
		ch->current_slh.packetSequenceNumber = 1;
		ch->current_slh.randomAccessPointFlag = 0;
		if (ch->last_state == M4EOF) return;

		ch->sample = M4_NewSample();
		ch->sample->dataLength = ch->owner->od_au_size;
		ch->sample->data = ch->owner->od_au;
		ch->sample->IsRAP = 1;
		ch->current_slh.decodingTimeStamp = ch->start;
		ch->current_slh.compositionTimeStamp = ch->start;
		return;
	}

	if (ch->to_init) {
		init_reader(ch);
	} else if (ch->has_edit_list) {
		e = M4_GetSampleForMovieTime(ch->owner->mov, ch->track, ch->sample_time + 1, &ivar, M4_SearchForward, &ch->sample, &ch->sample_num);
		if (ch->sample) ch->sample_time = ch->sample->DTS;
	} else {
		ch->sample = M4_GetSample(ch->owner->mov, ch->track, ch->sample_num, &ivar);
		/*if sync shadow skip*/
		if (ch->sample && (ch->sample->IsRAP==2)) {
			M4_DeleteSample(&ch->sample);
			ch->sample_num++;
			ch->sample = M4_GetSample(ch->owner->mov, ch->track, ch->sample_num, &ivar);
		}
	}
	if (!ch->sample) {
		/*incomplete file - check if we're still downloading or not*/
		if (M4_GetBytesMissing(ch->owner->mov, ch->track)) {
			u32 net_status;
			NM_GetDownloaderStats(ch->owner->dnload, NULL, NULL, NULL, &net_status);
			if (net_status == DL_Running) {
				ch->last_state = M4OK;
			} else {
				ch->last_state = M4UncompleteFile;
			}
		} else if (!ch->sample_num || (ch->sample_num > M4_GetSampleCount(ch->owner->mov, ch->track))) {
			ch->last_state = M4EOF;
		}
		return;
	}
	ch->last_state = M4OK;
	ch->current_slh.accessUnitLength = ch->sample->dataLength;
	/*still seeking or not ?*/
	if (ch->start <= ch->sample->DTS + ch->sample->CTS_Offset) {
		ch->current_slh.decodingTimeStamp = ch->sample->DTS;
		ch->current_slh.compositionTimeStamp = ch->sample->DTS + ch->sample->CTS_Offset;
	} else {
		ch->current_slh.decodingTimeStamp = ch->start;
		ch->current_slh.compositionTimeStamp = ch->start;
	}
	ch->current_slh.randomAccessPointFlag = ch->sample->IsRAP;

	if (ch->end && (ch->end < ch->sample->DTS + ch->sample->CTS_Offset)) {
		ch->last_state = M4EOF;
	}
}

void m4_reader_release_sample(M4Channel *ch)
{

	if (ch->FAKE_ESID) {
		/*DON'T DESTROY OD, it will be needed for later seek...*/
		ch->last_state = M4EOF;
		ch->sample->data = NULL;
		ch->sample->dataLength = 0;
	}
	/*this is to handle edit list*/
	if (ch->sample->data) ch->sample_num++;

	if (ch->sample) M4_DeleteSample(&ch->sample);
	ch->sample = NULL;
	ch->current_slh.AU_sequenceNumber++;
	ch->current_slh.packetSequenceNumber++;
}



