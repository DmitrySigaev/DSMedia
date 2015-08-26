#include "laser_stacks.h"

#ifdef M4_USE_LASeR

static u8 LASeR_UpdateGenericAnimate(TimeNode *timenode, Float *fraction, s32 *prevKeyIndex, s32 *nextKeyIndex)
{
	Double currentTime, cycleTime;
	LASeRAnimateTransform *an = (LASeRAnimateTransform *)timenode->obj;
	LASeR_AnimateStack *stack = (LASeR_AnimateStack *) Node_GetPrivate(timenode->obj);
	
	Double currentDuration = 0;
	Double totalDuration = 0;
	u32 i;

	*prevKeyIndex = -1;
	*nextKeyIndex = -1;

	currentTime = Node_GetSceneTime(timenode->obj);

	if (an->startTime == -1) return 0;
	if (currentTime < an->startTime) return 0;
	if (an->startTime == -17 || an->startTime == 0) an->startTime = currentTime;

	for (i=0; i<an->nbKeys; i++) {
		totalDuration += an->durations[i]/1000.0;
	}

	currentTime -= an->startTime;

	cycleTime = currentTime - totalDuration*stack->num_cycles;

	*prevKeyIndex = 0;
	currentDuration = an->durations[*prevKeyIndex];
	for (i=1; i<an->nbKeys; i++) {
		if (an->durations[i]/1000.0 + currentDuration >= cycleTime) {
			*nextKeyIndex = i;
			break;
		} else {
			currentDuration += an->durations[i]/1000.0;
			*prevKeyIndex = i;
			if (i == an->nbKeys -1 && an->repeat) {
				*nextKeyIndex = *prevKeyIndex;
				stack->num_cycles++;
			}
		}
	}
	if (*nextKeyIndex == -1) {
		an->startTime = -1;
		return 0;
	}
	*fraction = (Float)(cycleTime - currentDuration)/(an->durations[*nextKeyIndex]/1000.0);
	return 1;
}


static
void LASeR_UpdateAnimateTransform(TimeNode *timenode)
{
	Float newFraction = 0;
	LASeRAnimateTransform *at = (LASeRAnimateTransform *)timenode->obj;
	s32 prevTKeyIndex, nextTKeyIndex;
	LASeRTransformKey *prevTKey, *nextTKey;
	LASeRTransform *targetnode = NULL;

	if (!LASeR_UpdateGenericAnimate(timenode, &newFraction, &prevTKeyIndex, &nextTKeyIndex)) return;

	targetnode = (LASeRTransform *)SG_FindNode(Node_GetParentGraph((SFNode *)at), at->target);
	if (!targetnode) return;

	nextTKey = &(at->keys[nextTKeyIndex]);
	prevTKey = &(at->keys[prevTKeyIndex]);
	if (nextTKey == NULL || prevTKey == NULL) return;

	if (nextTKey->hasMatrix && prevTKey->hasMatrix) {
		LASeRMatrix newM;

		newM.xx = prevTKey->m.xx*(1-newFraction) + nextTKey->m.xx*newFraction;
		newM.xy = prevTKey->m.xy*(1-newFraction) + nextTKey->m.xy*newFraction;
		newM.yx = prevTKey->m.yx*(1-newFraction) + nextTKey->m.yx*newFraction;
		newM.yy = prevTKey->m.yy*(1-newFraction) + nextTKey->m.yy*newFraction;
		newM.xz = prevTKey->m.xz*(1-newFraction) + nextTKey->m.xz*newFraction;
		newM.yz = prevTKey->m.yz*(1-newFraction) + nextTKey->m.yz*newFraction;

		memcpy(&(targetnode->m), &(newM), sizeof(LASeRMatrix));
		SG_NodeChanged((SFNode *)targetnode, NULL);
	}
}

static void LASeR_UpdateAnimateColor(TimeNode *timenode)
{
	Float newFraction = 0;
	LASeRAnimateColor *ac = (LASeRAnimateColor *)timenode->obj;
	s32 prevCKeyIndex, nextCKeyIndex;
	LASeRColorKey *prevCKey, *nextCKey;
	LASeRColorKey *targetnode = NULL;

	if (!LASeR_UpdateGenericAnimate(timenode, &newFraction, &prevCKeyIndex, &nextCKeyIndex)) return;

	targetnode = (SFNode *)SG_FindNode(Node_GetParentGraph((SFNode *)ac), ac->target);
	if (!targetnode) return;

	nextCKey = &(ac->keys[nextCKeyIndex]);
	prevCKey = &(ac->keys[prevCKeyIndex]);
	if (nextCKey == NULL || prevCKey == NULL) return;

	{
		LASeRColor newC;
		
		newC.r = prevCKey->color.r*(1-newFraction) + nextCKey->color.r*newFraction;
		newC.g = prevCKey->color.g*(1-newFraction) + nextCKey->color.g*newFraction;
		newC.b = prevCKey->color.b*(1-newFraction) + nextCKey->color.b*newFraction;

		if (Node_GetTag(targetnode) == TAG_LASeRShape) {
			memcpy(&(((LASeRShape *)targetnode)->fill), &newC, sizeof(LASeRColor));
		}

		SG_NodeChanged((SFNode *)targetnode, NULL);
	}
}

static void LASeR_UpdateAnimateActivate(TimeNode *timenode)
{
}

static void LASeR_DestroyGenericAnimate(SFNode *node)
{
	LASeR_AnimateStack *stack = Node_GetPrivate(node);
	if (stack->time_handle.is_registered) {
		SR_UnregisterTimeNode(stack->compositor, &stack->time_handle);
	}
	free(stack);
}

static void LASeR_InitGenericAnimate(Render2D *sr, SFNode *node, void (*UpdateTimeNode)(TimeNode *))
{
	LASeR_AnimateStack *animate_stack;
	SAFEALLOC(animate_stack, sizeof(LASeR_AnimateStack))
	animate_stack->time_handle.UpdateTimeNode = UpdateTimeNode;
	animate_stack->time_handle.obj = node;
	animate_stack->num_cycles = 0;
	animate_stack->compositor = sr->compositor;

	Node_SetPrivate(node, animate_stack);
	Node_SetPreDestroyFunction(node, LASeR_DestroyGenericAnimate);	
	SR_RegisterTimeNode(animate_stack->compositor, &animate_stack->time_handle);
}

void LASeR_InitAnimateTransform(Render2D *sr, SFNode *node)
{
	LASeR_InitGenericAnimate(sr, node, LASeR_UpdateAnimateTransform);
}

void LASeR_AnimateTransformModified(SFNode *node)
{
	LASeR_AnimateStack *stack = (LASeR_AnimateStack *) Node_GetPrivate(node);
	if (!stack) return;

	LASeR_UpdateAnimateTransform(&stack->time_handle);

	stack->time_handle.needs_unregister = 0;
	if (!stack->time_handle.is_registered) {
		SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
	}
}

void LASeR_InitAnimateColor(Render2D *sr, SFNode *node)
{
	LASeR_InitGenericAnimate(sr, node, LASeR_UpdateAnimateColor);
}

void LASeR_AnimateColorModified(SFNode *node)
{
	LASeR_AnimateStack *stack = (LASeR_AnimateStack *) Node_GetPrivate(node);
	if (!stack) return;

	LASeR_UpdateAnimateColor(&stack->time_handle);

	stack->time_handle.needs_unregister = 0;
	if (!stack->time_handle.is_registered) {
		SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
	}
}

void LASeR_InitAnimateActivate(Render2D *sr, SFNode *node)
{
	LASeR_InitGenericAnimate(sr, node, LASeR_UpdateAnimateActivate);
}

void LASeR_AnimateActivateModified(SFNode *node)
{
	LASeR_AnimateStack *stack = (LASeR_AnimateStack *) Node_GetPrivate(node);
	if (!stack) return;

	LASeR_UpdateAnimateActivate(&stack->time_handle);

	stack->time_handle.needs_unregister = 0;
	if (!stack->time_handle.is_registered) {
		SR_RegisterTimeNode(stack->compositor, &stack->time_handle);
	}

}

#endif

