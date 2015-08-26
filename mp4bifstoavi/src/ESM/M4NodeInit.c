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


#include <m4_render.h>
#include <intern/m4_esm_dev.h>
#include <m4_x3d_nodes.h>

#include "InputSensor.h"

void Inline_Modified(SFNode *node);
void InitMediaControl(InlineScene *is, SFNode *node);
void MC_Modified(SFNode *node);
void InitMediaSensor(InlineScene *is, SFNode *node);
void MS_Modified(SFNode *node);


void Destroy_WorldInfo(SFNode *node)
{
	InlineScene *is = Node_GetPrivate(node);
	is->world_info = NULL;
}
void Render_WorldInfo(SFNode *node, void *rs)
{
	InlineScene *is = Node_GetPrivate(node);
	is->world_info = (M_WorldInfo *) node;
}

void Term_NodeInit(void *_is, SFNode *node)
{
	InlineScene *is = (InlineScene *)_is;
	if (!node || !is) return;
	
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_Inline: 
	case TAG_X3D_Inline: 
		Node_SetRenderFunction(node, IS_Render); break;
	case TAG_MPEG4_MediaBuffer: break;
	case TAG_MPEG4_MediaControl: InitMediaControl(is, node); break;
	case TAG_MPEG4_MediaSensor: InitMediaSensor(is, node); break;
	case TAG_MPEG4_InputSensor: InitInputSensor(is, node); break;

	/*BIFS nodes, get back to codec, but filter externProtos*/
	case TAG_MPEG4_Conditional: break;
	case TAG_MPEG4_QuantizationParameter: break;
	/*world info is stored at the inline scene level*/
	case TAG_MPEG4_WorldInfo:
	case TAG_X3D_WorldInfo:
		Node_SetPreDestroyFunction(node, Destroy_WorldInfo);
		Node_SetRenderFunction(node, Render_WorldInfo);
		Node_SetPrivate(node, is);
		break;

	case TAG_X3D_KeySensor: InitKeySensor(is, node); break;
	case TAG_X3D_StringSensor: InitStringSensor(is, node); break;

	default: SR_NodeInit(is->root_od->term->renderer, node); break;
	}
}

void Term_NodeModified(void *_is, SFNode *node)
{
	InlineScene *is = (InlineScene *)_is;
	if (!node || !is) return;
	
	switch (Node_GetTag(node)) {
	case TAG_MPEG4_Inline: 
	case TAG_X3D_Inline: 
		Inline_Modified(node); break;
	case TAG_MPEG4_MediaBuffer: break;
	case TAG_MPEG4_MediaControl: MC_Modified(node); break;
	case TAG_MPEG4_MediaSensor: MS_Modified(node); break;
	case TAG_MPEG4_InputSensor: InputSensorModified(node); break;
	case TAG_MPEG4_Conditional: break;
	default: SR_Invalidate(is->root_od->term->renderer, node); break;
	}
}
