#ifndef LASER_STACKS_H
#define LASER_STACKS_H

#include "../render2d.h"

#ifdef M4_USE_LASeR

#include "../../../M4Systems/LASeR/m4_laser_dev.h"
#include "../stacks2d.h"

/* Basic Graphics stuff */

void LASeR_InitTransform(Render2D *sr, SFNode *node); 
void LASeR_InitShape(Render2D *sr, SFNode *node); 
void LASeR_InitText(Render2D *sr, SFNode *node); 
void LASeR_InitUse(Render2D *sr, SFNode *node); 

/* Animation nodes */

typedef struct
{
	TimeNode time_handle;
	u32 num_cycles;
	LPSCENERENDER compositor;
} LASeR_AnimateStack;

void LASeR_InitAnimateTransform(Render2D *sr, SFNode *node); 
void LASeR_AnimateTransformModified(SFNode *node);

void LASeR_InitAnimateColor(Render2D *sr, SFNode *node); 
void LASeR_AnimateColorModified(SFNode *node);

void LASeR_InitAnimateActivate(Render2D *sr, SFNode *node); 
void LASeR_AnimateColorModified(SFNode *node);

/* Media nodes */

void LASeR_InitVideo(Render2D *sr, SFNode *node); 
void LASeR_InitAudio(Render2D *sr, SFNode *node); 
void LASeR_InitBitmap(Render2D *sr, SFNode *node); 

/* Interaction nodes */

void LASeR_InitAction(Render2D *sr, SFNode *node); 
void LASeR_InitConditional(Render2D *sr, SFNode *node); 

void LASeR_InitTextInput(Render2D *sr, SFNode *node); 
void LASeR_InitCursor(Render2D *sr, SFNode *node); 


#endif	//M4_USE_LASeR

#endif
