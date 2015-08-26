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


#ifndef _INPUTSENSOR_H_
#define _INPUTSENSOR_H_


#include <m4_decoder.h>
#include <intern/m4_esm_dev.h>

enum
{
	IS_KeySensor = 1,
	IS_StringSensor,
	IS_Mouse,
	IS_HTKSensor,
};

#define M4_HTK_DEMO  0

typedef struct
{
	/*parent scene*/
	InlineScene *scene;
	/*list of attached nodes*/
	Chain *is_nodes;
	/*stream ID*/
	u16 ES_ID;
	/*uncompressed data frame*/
	Chain *ddf;
	u32 type;

	/*string sensor sep char */
	s16 termChar, delChar;
	/*current typed text in UTF-8*/
	short enteredText[5000];
	u32 text_len;
	Bool is_local;

#if M4_HTK_DEMO
	M4Thread *th;
	Bool htk_running;
	char szHTKPath[M4_MAX_PATH];
#endif

} ISPriv;


BaseDecoder *NewISCodec(u32 PL);
void ISDec_Delete(BaseDecoder *plug);
M4Err IS_Configure(BaseDecoder *plug, InlineScene *scene, Bool is_remote);


typedef struct
{
	/*stream context*/
	u16 ES_ID;

	MFURL url;

	Bool registered;
	MediaObject *is_dec;
	M_InputSensor *is;
} ISStack;


void InitInputSensor(InlineScene *is, SFNode *node);
void InputSensorModified(SFNode *n);

void InitKeySensor(InlineScene *is, SFNode *node);


typedef struct
{
	u16 enteredText[5000];
	u32 text_len;
	M4Client *term;
} StringSensorStack;

void InitStringSensor(InlineScene *is, SFNode *node);

#endif

