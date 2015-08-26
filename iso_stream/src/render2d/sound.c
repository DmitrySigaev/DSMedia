/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / 2D rendering plugin
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


#include "stacks2d.h"


typedef struct
{
	SoundInterface snd_ifce;
	SFVec2f pos;
} Sound2DStack;

static void DestroySound2D(SFNode *node)
{
	Sound2DStack *st = (Sound2DStack *)Node_GetPrivate(node);
	free(st);
}

/*sound2D wraper - spacialization is not supported yet*/
static void RenderSound2D(SFNode *node, void *rs)
{
	RenderEffect2D *eff = (RenderEffect2D*) rs;
	M_Sound2D *snd = (M_Sound2D *)node;
	Sound2DStack *st = (Sound2DStack *)Node_GetPrivate(node);

	if (!snd->source) return;

	/*this implies no DEF/USE for real location...*/
	st->pos = snd->location;
	mx2d_apply_point(&eff->transform, &st->pos);

	eff->sound_holder = &st->snd_ifce;
	Node_Render((SFNode *) snd->source, eff);
	eff->sound_holder = NULL;
}
static Bool SND2D_GetChannelVolume(SFNode *node, Float *vol)
{
	vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = vol[5] = ((M_Sound2D *)node)->intensity;
	return (vol[0]==1.0f) ? 0 : 1;
}
static u8 SND2D_GetPriority(SFNode *node)
{
	return 255;
}

void R2D_InitSound2D(Render2D *sr, SFNode *node)
{
	Sound2DStack *snd;
	SAFEALLOC(snd, sizeof(Sound2DStack));
	snd->snd_ifce.GetChannelVolume = SND2D_GetChannelVolume;
	snd->snd_ifce.GetPriority = SND2D_GetPriority;
	snd->snd_ifce.owner = node;
	Node_SetPrivate(node, snd);
	Node_SetRenderFunction(node, RenderSound2D);
	Node_SetPreDestroyFunction(node, DestroySound2D);
}
