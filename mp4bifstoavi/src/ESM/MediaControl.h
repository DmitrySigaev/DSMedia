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



#ifndef __MEDIACONTROL_H
#define __MEDIACONTROL_H

#include <m4_mpeg4_nodes.h>
#include <intern/m4_esm_dev.h>


/*to do: add preroll support*/
typedef struct _media_control
{
	M_MediaControl *control;

	/*store current values to detect changes*/
	Double media_start, media_stop;
	Float media_speed;
	Bool enabled;
	MFURL url;
	
	InlineScene *parent;
	/*stream owner*/
	MediaObject *stream;
	Bool changed;
	Bool is_init;
	Bool paused;
	u32 prev_active;

	/*stream object list (segments)*/
	Chain *seg;
	/*current active segment index (ie, controling the PLAY range of the media)*/
	u32 current_seg;

} MediaControlStack;
void InitMediaControl(InlineScene *is, SFNode *node);
void MC_Modified(SFNode *node);

void MC_GetRange(MediaControlStack *ctrl, Double *start_range, Double *end_range);

/*assign mediaControl for this object*/
void ODM_SetMediaControl(ODManager *odm, struct _media_control *ctrl);
/*get media control ruling the clock the media is running on*/
struct _media_control *ODM_GetMediaControl(ODManager *odm);
/*get mediaControl controling and owned by the OD, or NULL if none*/
struct _media_control *ODM_GetObjectMediaControl(ODManager *odm);
/*removes control from OD context*/
void ODM_RemoveMediaControl(ODManager *odm, struct _media_control *ctrl);
/*switches control (propagates enable=FALSE), returns 1 if control associated with OD has changed to new one*/
Bool ODM_SwitchMediaControl(ODManager *odm, struct _media_control *ctrl);

/*restart object and takes care of media control/clock dependencies*/
void MC_Restart(ODManager *odm);

Bool MC_URLChanged(MFURL *old_url, MFURL *new_url);

typedef struct _media_sensor
{
	M_MediaSensor *sensor;

	InlineScene *parent;

	Chain *seg;
	Bool is_init;
	/*stream owner*/
	MediaObject *stream;

	/*private cache (avoids browsing all sensor*/
	u32 active_seg;
} MediaSensorStack;

void InitMediaSensor(InlineScene *is, SFNode *node);
void MS_Modified(SFNode *node);

void MS_UpdateTiming(ODManager *odm);
void MS_Stop(MediaSensorStack *st);


#endif
