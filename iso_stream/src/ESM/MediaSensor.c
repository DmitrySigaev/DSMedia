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


#include "MediaControl.h"

static void DestroyMediaSensor(SFNode *node)
{
	MediaSensorStack *st = Node_GetPrivate(node);

	/*unlink from OD*/
	if (st->stream && st->stream->odm) 
		ChainDeleteItem(st->stream->odm->ms_stack, st);

	DeleteChain(st->seg);
	free(st);
}

/*render : setup media sensor and update timing in case of inline scenes*/
void RenderMediaSensor(SFNode *node, void *rs)
{
	Clock *ck;
	MediaSensorStack *st = Node_GetPrivate(node);

	if (!st->stream) st->stream = MO_FindObject(node, &st->sensor->url);
	if (!st->stream) return;
	if (!st->stream->odm) return;
	if (!st->is_init) {
		ChainAddEntry(st->stream->odm->ms_stack, st);
		ODM_InitSegmentDescriptors(st->stream->odm, st->seg, &st->sensor->url);
		st->is_init = 1;
		st->active_seg = 0;
		
	}
	/*media sensor bound to natural media (audio, video) is updated when fetching the stream
	data for rendering.*/

	ck = NULL;
	/*check inline scenes - if the scene is set to restart DON'T MODIFY SENSOR: since we need a 2 render
	passes to restart inline, scene is considered as not running*/
	if (st->stream->odm->subscene && !st->stream->odm->subscene->needs_restart) {
		ck = st->stream->odm->subscene->scene_codec->ck;
		/*since audio may be used alone through an inline scene, we need to refresh the graph*/
		if (st->stream->odm->is_open) Term_InvalidateScene(st->stream->term);
	}
	/*check anim streams*/
	else if (st->stream->odm->codec && (st->stream->odm->codec->type==M4ST_SCENE)) ck = st->stream->odm->codec->ck;
	/*check OCR streams*/
	else if (st->stream->odm->ocr_codec) ck = st->stream->odm->ocr_codec->ck;

	if (ck && CK_IsStarted(ck) ) {
		st->stream->odm->current_time = CK_GetTime(ck);
		MS_UpdateTiming(st->stream->odm);
	}
}

void InitMediaSensor(InlineScene *is, SFNode *node)
{
	MediaSensorStack *st = malloc(sizeof(MediaSensorStack));
	memset(st, 0, sizeof(MediaSensorStack));

	st->parent = is;
	st->sensor = (M_MediaSensor *)node;
	st->seg = NewChain();
	Node_SetRenderFunction(node, RenderMediaSensor);
	Node_SetPreDestroyFunction(node, DestroyMediaSensor);
	Node_SetPrivate(node, st);

}

/*only URL can be changed, so reset and get new URL*/
void MS_Modified(SFNode *node)
{
	MediaSensorStack *st = Node_GetPrivate(node);
	if (!st) return;
	
	while (ChainGetCount(st->seg)) {
		ChainDeleteEntry(st->seg, 0);
	}
	st->stream = MO_FindObject(node, &st->sensor->url);
	st->is_init = 0;
	Term_InvalidateScene(st->parent->root_od->term);
}

void MS_UpdateTiming(ODManager *odm)
{
	SegmentDescriptor *desc;
	u32 i, count, j, ms_count;
	Double time;
	ms_count = ChainGetCount(odm->ms_stack);
	if (!ms_count) return;
	
	time = odm->current_time / 1000.0;
	for (j=0; j<ms_count; j++) {
		MediaSensorStack *media_sens = (MediaSensorStack *)ChainGetEntry(odm->ms_stack, j);
		if (!media_sens->is_init) continue;
		count = ChainGetCount(media_sens->seg);
		if (media_sens->active_seg==count) {
			/*full object controled*/
			if (!count) {
				if (!media_sens->sensor->isActive) {
					media_sens->sensor->isActive = 1;
					Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
					if (odm->subscene) {
						media_sens->sensor->mediaDuration = odm->subscene->duration;
					} else {
						media_sens->sensor->mediaDuration = odm->duration;
					}
					media_sens->sensor->mediaDuration /= 1000;
					Node_OnEventOutSTR((SFNode *) media_sens->sensor, "mediaDuration");
				}
				if (media_sens->sensor->mediaCurrentTime != time) {
					media_sens->sensor->mediaCurrentTime = time;
					Node_OnEventOutSTR((SFNode *) media_sens->sensor, "mediaCurrentTime");
				}
				/*check for end of scene (MediaSensor on inline)*/
				if (odm->subscene && odm->subscene->duration) {
					Clock *ck = ODM_GetMediaClock(odm);
					if (ck->has_seen_eos && media_sens->sensor->isActive && (1000*time>odm->subscene->duration)) {
						media_sens->sensor->isActive = 0;
						Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
					}
				}
				continue;
			}
		}


		/*locate segment*/
		for (i=media_sens->active_seg; i<count; i++) {
			desc = ChainGetEntry(media_sens->seg, i);
			/*not controled*/
			if (desc->startTime > time) {
				if (media_sens->sensor->isActive) {
					media_sens->sensor->isActive = 0;
					Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
				}
				break;
			}
			if (desc->startTime + desc->Duration < time) continue;
			/*segment switch, force activation (isActive TRUE send at each seg)*/
			if (media_sens->active_seg != i) {
				media_sens->active_seg = i;
				media_sens->sensor->isActive = 0;
			}

			if (!media_sens->sensor->isActive) {
				media_sens->sensor->isActive = 1;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
				/*set info*/
				VRML_MF_Reset(& media_sens->sensor->info, FT_MFString);
				VRML_MF_Alloc(& media_sens->sensor->info, FT_MFString, 1);
				media_sens->sensor->info.vals[0] = desc->SegmentName ? strdup(desc->SegmentName) : NULL;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "info");
				/*set duration*/
				media_sens->sensor->mediaDuration = desc->Duration;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "mediaDuration");
				/*set seg start time*/
				media_sens->sensor->streamObjectStartTime = desc->startTime;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "streamObjectStartTime");
			}
			/*set media time - relative to segment start time*/
			time -= desc->startTime;
			if (media_sens->sensor->mediaCurrentTime != time) {
				media_sens->sensor->mediaCurrentTime = time;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "mediaCurrentTime");
			}
			break;
		}
		if (i==count) {
			/*we're after last segment, deactivate*/
			if (media_sens->sensor->isActive) {
				media_sens->sensor->isActive = 0;
				Node_OnEventOutSTR((SFNode *) media_sens->sensor, "isActive");
				media_sens->active_seg = count;
			}
		}
	}
}

void MS_Stop(MediaSensorStack *st)
{
	if (st->sensor->isActive) {
		st->sensor->isActive = 0;
		Node_OnEventOutSTR((SFNode *) st->sensor, "isActive");
	}
	st->active_seg = 0;
}
