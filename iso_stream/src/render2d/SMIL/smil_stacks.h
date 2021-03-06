/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SMIL Rendering sub-project
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
#ifndef _SMIL_STACKS_H
#define _SMIL_STACKS_H

#include "../render2d.h"

#ifndef M4_DISABLE_SMIL

#include "../stacks2d.h"

/* Animation functions */

/* status of an SMIL animated element */ 
enum {
	SMIL_STATUS_STARTUP = 0,
	SMIL_STATUS_WAITING_TO_BEGIN,
	SMIL_STATUS_ACTIVE,
	SMIL_STATUS_END_INTERVAL,
	SMIL_STATUS_POST_ACTIVE,
	SMIL_STATUS_DONE
};

typedef struct _smil_anim_stack
{
	TimeNode time_handle;
	LPSCENERENDER compositor;

	/* SMIL element life-cycle status */
	u8 status;

	/* negative values mean indefinite */
	Double begin, end, simple_duration, active_duration;
	Bool is_active_duration_clamped_to_min;

	u32 nb_iterations;

	void *init_value;
	void *last_anim_value;

	/* animation attributes of the timenode */
	
	/* attributes to identify the target element of an animation: 
	       SVG.Animation.attrib 
	*/
	SFNode *target_element;
	/* attributes to identify the target attribute or property of an animation: 
	       SVG.AnimationAttribute.attrib 
	*/
	SMIL_AttributeName *attributeName; 
	/* attributes to control the timing of the animation 
	       SVG.AnimationTiming.attrib
	*/
	SMIL_BeginOrEndValues *begins; 
	SMIL_MinMaxDurRepeatDurValue *dur; 
	SMIL_BeginOrEndValues *ends; 
	SMIL_MinMaxDurRepeatDurValue *min; 
	SMIL_MinMaxDurRepeatDurValue *max; 
	SMIL_RestartValue *restart; 
	SMIL_RepeatCountValue *repeatCount; 
	SMIL_MinMaxDurRepeatDurValue *repeatDur; 
	SMIL_FreezeValue *freeze; /* fill renamed freeze because of conflict in SVG */
	/* attributes that define animation values over time 
	       SVG.AnimationValue.attrib
	*/
	SMIL_CalcModeValue *calcMode; 
	SMIL_AnimateValues *values; 
	SMIL_KeyTimesValues *keyTimes; 
	SMIL_KeySplinesValues *keySplines; 
	SMIL_AnimateValue *from; 
	SMIL_AnimateValue *to; 
	SMIL_AnimateValue *by; 
	/* attributes to control whether animations are additive 
	       SVG.AnimationAddition.attrib
	*/
	SMIL_AdditiveValue *additive; 
	SMIL_AccumulateValue *accumulate; 

	/* additional attributes for animateMotion*/
	/* */

	/* additional attributes for animateTransform*/
	/* */

	/* generic api for animation */
	void (*SetDiscreteValueAndAccumulate)(struct _smil_anim_stack *stack, void *) ;
	void (*InterpolateAndAccumulate)(struct _smil_anim_stack *stack, Double interpolation_coefficient,
									 void *value1, void *value2);
	void (*SaveInitValue)(struct _smil_anim_stack *stack);
	void (*RestoreValue)(struct _smil_anim_stack *stack, Bool init_or_last);

} SMIL_AnimationStack;

SMIL_AnimationStack *SMIL_Init_AnimationStack(Render2D *sr, SFNode *node, void (*UpdateTimeNode)(TimeNode *));
void SMIL_Update_Animation(TimeNode *timenode);
void SMIL_Modified_Animation(SFNode *node);
void *SMIL_GetLastSpecifiedValue(SMIL_AnimationStack *stack);

#endif // M4_DISABLE_SMIL

#endif // _SMIL_STACKS_H

