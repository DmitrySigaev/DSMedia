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
#include "smil_stacks.h"

#ifndef M4_DISABLE_SMIL

static void SMIL_findInterval(SMIL_AnimationStack *stack)
{
	if (stack->status == SMIL_STATUS_STARTUP) {

		stack->simple_duration = -1; /* negative values mean indefinite */
		stack->active_duration = -1;
		stack->is_active_duration_clamped_to_min = 0;
		stack->begin = -1;
		stack->end = -1;

		/* Step 1: determine a value for begin */
		/* if no begin is specified, the default timing is equivalent to an offset value of "0".*/
		if (ChainGetCount(*stack->begins)) {
			/* begin values should be sorted first */
			/* assumption there is only one value */
			stack->begin = ((SMIL_BeginOrEndValue *)ChainGetEntry(*stack->begins, 0))->clock_value;
		} else {
			stack->begin = 0;
		}

		/* Step 2: determine a value for end */
		if (ChainGetCount(*stack->ends)) {
			/* end values should be sorted first */
			/* assumption: if there is a value, there is only one value */
			stack->end = ((SMIL_BeginOrEndValue *)ChainGetEntry(*stack->ends, 0))->clock_value;
		} 
		stack->status = SMIL_STATUS_WAITING_TO_BEGIN;
	}	
}

static void SMIL_calcActiveDur(SMIL_AnimationStack *stack)
{
	Bool clamp_active_duration = 1;

	/* Step 3: determine the simple duration */
	if (stack->dur->type != SMILMinMaxDurRepeatDur_clock_value) {
		/* simple_duration is not defined, so stays indefinite */
		/* we can ignore repeatCount to compute active_duration */

		/* Step 2: determine the active duration */
		if (stack->repeatDur->type != SMILMinMaxDurRepeatDur_clock_value) {
			/* active_duration stays indefinite */
		} else {
			stack->active_duration = stack->repeatDur->clock_value;
		}
	} else {
		/* simple_duration is defined */
		stack->simple_duration = stack->dur->clock_value;

		/* Step 2: determine the active duration */
		if (*(stack->repeatCount) < 0) {
			/* use repeatDur only to determine the active duration */
			if (stack->repeatDur->type != SMILMinMaxDurRepeatDur_clock_value) {
				/* active_duration stays indefinite */
			} else {
				stack->active_duration = stack->repeatDur->clock_value;
			}
		} else {
			if (stack->repeatDur->type != SMILMinMaxDurRepeatDur_clock_value) {
				/* use repeatCount only to determine the active duration */
				stack->active_duration = *(stack->repeatCount) * stack->simple_duration;
			} else {
				/* use repeatCount and repeatDur to determine the active duration */
				stack->active_duration = MIN(stack->repeatDur->clock_value, *(stack->repeatCount) * stack->simple_duration);
			}
		}
	}

	/* Step 4: clamp the active duration with min and max */
	if (stack->max->type == SMILMinMaxDurRepeatDur_clock_value && stack->min->type == SMILMinMaxDurRepeatDur_clock_value) {
		if (stack->max->clock_value < stack->min->clock_value) clamp_active_duration = 0;
	}
	if (clamp_active_duration) {
		if (stack->min->type == SMILMinMaxDurRepeatDur_clock_value) {
			if (stack->active_duration >= 0 && stack->active_duration <= stack->min->clock_value) {
				stack->is_active_duration_clamped_to_min = 1;
				stack->active_duration = stack->min->clock_value;
			}
		}
		if (stack->max->type == SMILMinMaxDurRepeatDur_clock_value) {
			stack->active_duration = MIN(stack->max->clock_value, stack->active_duration);
		}
	}

	/* Step 5: if end is defined, clamp active duration to end-begin */
	if (stack->end >= 0) {
		stack->active_duration = MIN(stack->active_duration, (stack->end - stack->begin));
	}
}

void *SMIL_GetLastSpecifiedValue(SMIL_AnimationStack *stack)
{
	void *value;
	u32 nbValues = ChainGetCount(stack->values->values);
	if (nbValues) {
		/* Ignore from/to/by*/
		value = ChainGetEntry(stack->values->values, nbValues - 1);
	} else {
		/* Use from/to/by*/
		if (stack->to->datatype != 0) {
			value = stack->to->value;
		}
	}
	return value;
}

static void SMIL_AnimWithValues(SMIL_AnimationStack *stack, Double sceneTime) 
{
	u32 i;
	u32 nbValues;
	Double activeTime;
	Double simpleTime;
	Float interval_duration;

	nbValues = ChainGetCount(stack->values->values);
	interval_duration = (Float)stack->simple_duration/(nbValues-1);

	activeTime = sceneTime - stack->begin;
	stack->nb_iterations = (u32)floor(activeTime / stack->simple_duration);
	simpleTime = activeTime - stack->simple_duration * stack->nb_iterations;
	
	/* to be sure clamp cycleTime */
	simpleTime = MAX(0, simpleTime);
	simpleTime = MIN(stack->simple_duration, simpleTime);

	switch (*stack->calcMode) {
	case SMILCalcMode_discrete:
		if (interval_duration != -1) {
			for (i=0; i<nbValues; i++) {
				if (simpleTime < (i+1)*interval_duration) break; 
			}
			stack->SetDiscreteValueAndAccumulate(stack, ChainGetEntry(stack->values->values, i));
		} else {
			stack->SetDiscreteValueAndAccumulate(stack, ChainGetEntry(stack->values->values, 0));
		}
		break;
	case SMILCalcMode_linear:
		if (interval_duration != -1) {
			for (i=0; i<nbValues; i++) {
				if (simpleTime < (i+1)*interval_duration) break; 
			}
			stack->InterpolateAndAccumulate(stack, simpleTime/interval_duration - i,
							ChainGetEntry(stack->values->values, i),
							ChainGetEntry(stack->values->values, i+1));
		} else {
			stack->SetDiscreteValueAndAccumulate(stack, ChainGetEntry(stack->values->values, 0));
		}
		break;
	case SMILCalcMode_paced:
		break;
	case SMILCalcMode_spline:
		break;
	}
}

static void SMIL_AnimWithFromToBy(SMIL_AnimationStack *stack, Double sceneTime) 
{
	Double activeTime;
	Double simpleTime;

	activeTime = sceneTime - stack->begin;
	stack->nb_iterations = (u32)floor(activeTime / stack->simple_duration);
	simpleTime = activeTime - stack->simple_duration * stack->nb_iterations;
	
	/* to be sure clamp cycleTime */
	simpleTime = MAX(0, simpleTime);
	simpleTime = MIN(stack->simple_duration, simpleTime);

	/* if to is specified, ignore by */
	if (stack->to->datatype != 0) {
		/* use to */
		if (stack->from->datatype != 0) {
			/* from-to animation */
			switch (*stack->calcMode) {
			case SMILCalcMode_discrete:
				stack->InterpolateAndAccumulate(stack, (simpleTime<=stack->simple_duration/2?0:1),
												stack->from->value, stack->to->value);
				break;
			case SMILCalcMode_linear:
				stack->InterpolateAndAccumulate(stack, simpleTime/stack->simple_duration,
												stack->from->value, stack->to->value);
				break;
			case SMILCalcMode_paced:
				break;
			case SMILCalcMode_spline:
				break;
			}
		} else {
			/* to animation*/
			/* warning: additional constraints */
		}
	} else {
		/* use by : this may be supported only with attributes supporting addition*/
		if (stack->from->datatype != 0) {
			/* from-by animation */
		} else {
			if (stack->by->datatype != 0) {
				/* by animation */
				/* warning: additional constraints */
			} else {
				/* no animation */
				return;
			}
		}
	}
}

void SMIL_Update_Animation(TimeNode *timenode)
{
	Double sceneTime = 0;
	SMIL_AnimationStack *stack = Node_GetPrivate((SFNode *)timenode->obj);

	if (stack->status == SMIL_STATUS_DONE) return;

	if (stack->status == SMIL_STATUS_STARTUP) {
		stack->SaveInitValue(stack);
		SMIL_findInterval(stack);
		SMIL_calcActiveDur(stack);
		stack->status = SMIL_STATUS_WAITING_TO_BEGIN;
	}

	sceneTime = Node_GetSceneTime(timenode->obj);

	if (stack->status == SMIL_STATUS_WAITING_TO_BEGIN) {
		if (sceneTime >= stack->begin) stack->status = SMIL_STATUS_ACTIVE;
		else return;
	}

	if (stack->status == SMIL_STATUS_ACTIVE && 
		stack->active_duration >= 0 && 
		sceneTime >= (stack->begin + stack->active_duration)) {
		stack->status = SMIL_STATUS_POST_ACTIVE;
	}

	if (stack->status == SMIL_STATUS_POST_ACTIVE) {
		/* consider fill = freeze or remove */
		if (*stack->freeze == SMILFill_freeze) {
			stack->RestoreValue(stack, 0);
		} else {
			stack->RestoreValue(stack, 1);
		}
		stack->status = SMIL_STATUS_DONE;
	} else {
		u32 nbValues = ChainGetCount(stack->values->values);
		if (nbValues) {
			/* Ignore from/to/by*/
			SMIL_AnimWithValues(stack, sceneTime);
		} else {
			/* Use from/to/by*/
			SMIL_AnimWithFromToBy(stack, sceneTime);
		}
	}
	SG_NodeChanged((SFNode *)stack->target_element, NULL);
}

static void SMIL_Destroy_AnimationStack(SFNode *node)
{
	SMIL_AnimationStack *stack = Node_GetPrivate(node);
	if (stack->time_handle.is_registered) {
		SR_UnregisterTimeNode(stack->compositor, &stack->time_handle);
	}
	if (stack->init_value) free(stack->init_value);
	if (stack->last_anim_value) free(stack->last_anim_value);
	free(stack);
}

SMIL_AnimationStack *SMIL_Init_AnimationStack(Render2D *sr, SFNode *node, void (*UpdateTimeNode)(TimeNode *))
{
	SMIL_AnimationStack *stack;
	SAFEALLOC(stack, sizeof(SMIL_AnimationStack))
	stack->time_handle.UpdateTimeNode = UpdateTimeNode;
	stack->time_handle.obj = node;
	stack->compositor = sr->compositor;
	stack->status = SMIL_STATUS_STARTUP;

	Node_SetPrivate(node, stack);
	Node_SetPreDestroyFunction(node, SMIL_Destroy_AnimationStack);	
	SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
	return stack;
}

void SMIL_Modified_Animation(SFNode *node)
{
	SMIL_AnimationStack *stack = (SMIL_AnimationStack *) Node_GetPrivate(node);
	if (!stack) return;

	SMIL_Update_Animation(&stack->time_handle);

	stack->time_handle.needs_unregister = 0;
	if (!stack->time_handle.is_registered) {
		SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
	}
}

#endif // M4_DISABLE_SMIL


