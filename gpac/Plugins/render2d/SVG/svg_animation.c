/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Rendering sub-project
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
#include "svg_stacks.h"

#ifndef M4_DISABLE_SVG

static void SVG_Accumulate(SMIL_AnimationStack *stack, void *value) 
{
	void *last_specified_value = SMIL_GetLastSpecifiedValue(stack);
	switch(stack->attributeName->fieldType) {
	case SVG_Paint_datatype:
		{
			SVG_Paint *savedPaint = (SVG_Paint *)stack->last_anim_value;
			SVG_Paint *targetPaint = (SVG_Paint *)stack->attributeName->far_ptr;
			SVG_SVGColor *computedColor = &(((SVG_Paint *)value)->color);
			if (*stack->accumulate == SMILAccumulateValue_none) {
				targetPaint->color.red = computedColor->red;
				targetPaint->color.green = computedColor->green;
				targetPaint->color.blue = computedColor->blue;
			} else {
				targetPaint->color.red = MAX(0.0f , MIN(1.0f, computedColor->red + stack->nb_iterations*((SVG_Paint *)last_specified_value)->color.red));
				targetPaint->color.green = MAX(0.0f , MIN(1.0f, computedColor->green + stack->nb_iterations*((SVG_Paint *)last_specified_value)->color.green));
				targetPaint->color.blue = MAX(0.0f , MIN(1.0f, computedColor->blue + stack->nb_iterations*((SVG_Paint *)last_specified_value)->color.blue));
			}
			savedPaint->color.red = targetPaint->color.red;
			savedPaint->color.green = targetPaint->color.green;
			savedPaint->color.blue = targetPaint->color.blue;
		}
	case SVG_StrokeWidthValue_datatype:
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
		{
			SVG_Length *savedLength = (SVG_Length *)stack->last_anim_value;
			SVG_Length *targetLength = (SVG_Length *)stack->attributeName->far_ptr;
			SVG_Length *length = (SVG_Length *)value;
			if (*stack->accumulate == SMILAccumulateValue_none) {
				targetLength->number = length->number;
			} else {
				targetLength->number = length->number + stack->nb_iterations*((SVG_Length *)last_specified_value)->number;
			}
			savedLength->number = targetLength->number;
		}
		break;
	}
}

static void SVG_InterpolateAndAccumulate(SMIL_AnimationStack *stack, Double interpolation_coefficient, 
						   void *value1, void *value2)
{
	Float interpFraction = (Float)interpolation_coefficient;
	switch(stack->attributeName->fieldType) {
	case SVG_Paint_datatype:
		{
			SVG_Paint toPaint;
			SVG_SVGColor *colorA = &(((SVG_Paint *)value1)->color);
			SVG_SVGColor *colorB = &(((SVG_Paint *)value2)->color);
			toPaint.color.red = colorA->red*(1-interpFraction) + colorB->red*interpFraction;
			toPaint.color.green = colorA->green*(1-interpFraction) + colorB->green*interpFraction;
			toPaint.color.blue = colorA->blue*(1-interpFraction) + colorB->blue*interpFraction;
			SVG_Accumulate(stack, &toPaint);
		}
		break;
	case SVG_StrokeWidthValue_datatype:
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
		{
			SVG_Length toLength;
			toLength.number = ((SVG_Length *)value1)->number*(1-interpFraction) + ((SVG_Length *)value2)->number*interpFraction;
			SVG_Accumulate(stack, &toLength);
		}
		break;
	}
}


static void SVG_SetDiscreteValueAndAccumulate(SMIL_AnimationStack *stack, void *value) 
{
	SVG_Accumulate(stack, value);
}

static void SVG_SaveInitValue(SMIL_AnimationStack *stack)
{
	/* allocating memory to backup last anim value
	   and saving init value */
	switch(stack->attributeName->fieldType) {
	case SVG_Paint_datatype:
		SAFEALLOC(stack->last_anim_value, sizeof(SVG_Paint))
		SAFEALLOC(stack->init_value, sizeof(SVG_Paint))
		memcpy(stack->init_value, stack->attributeName->far_ptr, sizeof(SVG_Paint));
		break;
	case SVG_StrokeWidthValue_datatype:
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
		/* allocating memory to backup last anim value */
		SAFEALLOC(stack->last_anim_value, sizeof(SVG_Length))
		/* saving init value */
		SAFEALLOC(stack->init_value, sizeof(SVG_Length))
		memcpy(stack->init_value, stack->attributeName->far_ptr, sizeof(SVG_Length));
		break;
	}
}

static void SVG_RestoreValue(SMIL_AnimationStack *stack, Bool init_or_last)
{
	void *value = (init_or_last?stack->init_value:stack->last_anim_value);
	switch(stack->attributeName->fieldType) {
	case SVG_Paint_datatype:
		((SVG_Paint *)stack->attributeName->far_ptr)->color.red = ((SVG_Paint *)value)->color.red;
		((SVG_Paint *)stack->attributeName->far_ptr)->color.green = ((SVG_Paint *)value)->color.green;
		((SVG_Paint *)stack->attributeName->far_ptr)->color.blue = ((SVG_Paint *)value)->color.blue;
		break;
	}
//	free(stack->init_value);
}

static void SVG_Init_SMILAnimationStackAPI(SMIL_AnimationStack *stack)
{
	stack->SetDiscreteValueAndAccumulate = SVG_SetDiscreteValueAndAccumulate;
	stack->InterpolateAndAccumulate = SVG_InterpolateAndAccumulate;
	stack->SaveInitValue = SVG_SaveInitValue;
	stack->RestoreValue = SVG_RestoreValue;
}


void SVG_Init_set(Render2D *sr, SFNode *node)
{
	SVGsetElement *set = (SVGsetElement *)node;
	SMIL_AnimationStack *stack = SMIL_Init_AnimationStack(sr, node, SMIL_Update_Animation);
	
	SVG_Init_SMILAnimationStackAPI(stack);

	stack->target_element = (SFNode*)set->xlink_href.target_element;
	stack->attributeName = &(set->attributeName); 
	/*stack->attributeType = &(set->attributeType); */
	stack->begins = &(set->begin); 
	stack->dur = &(set->dur); 
	stack->ends = &(set->end); 
	stack->repeatCount = &(set->repeatCount); 
	stack->repeatDur = &(set->repeatDur); 
	stack->restart = &(set->restart); 
	stack->min = &(set->min); 
	stack->max = &(set->max); 
	stack->freeze = &(set->freeze); 
	stack->to = &(set->to); 
}

void SVG_Init_animate(Render2D *sr, SFNode *node)
{
	SVGanimateElement *animate = (SVGanimateElement *)node;
	SMIL_AnimationStack *stack = SMIL_Init_AnimationStack(sr, node, SMIL_Update_Animation);
	
	SVG_Init_SMILAnimationStackAPI(stack);

	stack->target_element = (SFNode*)animate->xlink_href.target_element;
	stack->attributeName = &(animate->attributeName); 
	/*stack->attributeType = &(animate->attributeType); */
	stack->begins = &(animate->begin); 
	stack->dur = &(animate->dur); 
	stack->ends = &(animate->end); 
	stack->repeatCount = &(animate->repeatCount); 
	stack->repeatDur = &(animate->repeatDur); 
	stack->restart = &(animate->restart); 
	stack->min = &(animate->min); 
	stack->max = &(animate->max); 
	stack->freeze = &(animate->freeze); 
	stack->calcMode = &(animate->calcMode); 
	stack->values = &(animate->values);
	stack->keyTimes = &(animate->keyTimes);
	stack->keySplines = &(animate->keySplines);
	stack->from = &(animate->from);
	stack->to = &(animate->to); 
	stack->by = &(animate->by); 
	stack->additive = &(animate->additive); 
	stack->accumulate = &(animate->accumulate); 
}

void SVG_Init_animateColor(Render2D *sr, SFNode *node)
{
	SVGanimateColorElement *ac = (SVGanimateColorElement *)node;
	SMIL_AnimationStack *stack = SMIL_Init_AnimationStack(sr, node, SMIL_Update_Animation);
	
	SVG_Init_SMILAnimationStackAPI(stack);

	stack->target_element = (SFNode*)ac->xlink_href.target_element;
	stack->attributeName = &(ac->attributeName); 
	/*stack->attributeType = &(ac->attributeType); */
	stack->begins = &(ac->begin); 
	stack->dur = &(ac->dur); 
	stack->ends = &(ac->end); 
	stack->repeatCount = &(ac->repeatCount); 
	stack->repeatDur = &(ac->repeatDur); 
	stack->restart = &(ac->restart); 
	stack->min = &(ac->min); 
	stack->max = &(ac->max); 
	stack->freeze = &(ac->freeze); 
	stack->calcMode = &(ac->calcMode); 
	stack->values = &(ac->values);
	stack->keyTimes = &(ac->keyTimes);
	stack->keySplines = &(ac->keySplines);
	stack->from = &(ac->from);
	stack->to = &(ac->to); 
	stack->by = &(ac->by); 
	stack->additive = &(ac->additive); 
	stack->accumulate = &(ac->accumulate); 
}

void SVG_Init_animateTransform(Render2D *sr, SFNode *node)
{
	SVGanimateTransformElement *at = (SVGanimateTransformElement *)node;
	SMIL_AnimationStack *stack = SMIL_Init_AnimationStack(sr, node, SMIL_Update_Animation);
	
	SVG_Init_SMILAnimationStackAPI(stack);

	stack->target_element = (SFNode*)at->xlink_href.target_element;
	stack->attributeName = &(at->attributeName); 
	/*stack->attributeType = &(at->attributeType); */
	stack->begins = &(at->begin); 
	stack->dur = &(at->dur); 
	stack->ends = &(at->end); 
	stack->repeatCount = &(at->repeatCount); 
	stack->repeatDur = &(at->repeatDur); 
	stack->restart = &(at->restart); 
	stack->min = &(at->min); 
	stack->max = &(at->max); 
	stack->freeze = &(at->freeze); 
	stack->calcMode = &(at->calcMode); 
	stack->values = &(at->values);
	stack->keyTimes = &(at->keyTimes);
	stack->keySplines = &(at->keySplines);
	stack->from = &(at->from);
	stack->to = &(at->to); 
	stack->by = &(at->by); 
	stack->additive = &(at->additive); 
	stack->accumulate = &(at->accumulate); 
}

void SVG_Init_animateMotion(Render2D *sr, SFNode *node)
{
	SVGanimateMotionElement *am = (SVGanimateMotionElement *)node;
	SMIL_AnimationStack *stack = SMIL_Init_AnimationStack(sr, node, SMIL_Update_Animation);
	
	SVG_Init_SMILAnimationStackAPI(stack);

	stack->target_element = (SFNode*)am->xlink_href.target_element;

	stack->begins = &(am->begin); 
	stack->dur = &(am->dur); 
	stack->ends = &(am->end); 
	stack->repeatCount = &(am->repeatCount); 
	stack->repeatDur = &(am->repeatDur); 
	stack->restart = &(am->restart); 
	stack->min = &(am->min); 
	stack->max = &(am->max); 
	stack->freeze = &(am->freeze); 
	stack->calcMode = &(am->calcMode); 
	stack->values = &(am->values);
	stack->keyTimes = &(am->keyTimes);
	stack->keySplines = &(am->keySplines);
	stack->from = &(am->from);
	stack->to = &(am->to); 
	stack->by = &(am->by); 
	stack->additive = &(am->additive); 
	stack->accumulate = &(am->accumulate); 
}

#endif
