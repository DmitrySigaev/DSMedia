/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Rendering sub-project
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



#ifndef COMMON_STACKS_H
#define COMMON_STACKS_H

#include <intern/m4_render_dev.h>

void InitAudioSource(LPSCENERENDER sr, SFNode *node);
void AudioSourceModified(SFNode *node);
void InitAudioClip(LPSCENERENDER sr, SFNode *node);
void AudioClipModified(SFNode *node);
void InitAudioBuffer(LPSCENERENDER sr, SFNode *node);
void AudioBufferModified(SFNode *node);
void InitAnimationStream(SceneRenderer *sr, SFNode *node);
void AnimationStreamModified(SFNode *node);
void InitTimeSensor(SceneRenderer *sr, SFNode *node);
void TimeSensorModified(SFNode *node);
void InitImageTexture(SceneRenderer *sr, SFNode *node);
TextureHandler *it_get_texture(SFNode *node);
void ImageTextureModified(SFNode *node);
void InitMovieTexture(SceneRenderer *sr, SFNode *node);
TextureHandler *mt_get_texture(SFNode *node);
void MovieTextureModified(SFNode *node);
void InitPixelTexture(SceneRenderer *sr, SFNode *node);
TextureHandler *pt_get_texture(SFNode *node);

#endif


