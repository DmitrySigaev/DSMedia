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



#ifndef GENERIC_TEXT_H
#define GENERIC_TEXT_H

#include "stacks2d.h"
#include "visualsurface2d.h"

TextLineEntry2D *NewTextLine2D(Render2D *sr);
Bool TextLine2D_TextureIsReady(TextLineEntry2D *tl);
void TextStack2D_clean_paths(TextStack2D *stack);


void text2D_get_ascent_descent(DrawableContext *ctx, Float *a, Float *d);


typedef struct
{
	unsigned short *wcText;
	u32 length;
	Float width, height;
	Float x_scaling, y_scaling;
} TextLine2D;

void Text2D_Draw(DrawableContext *ctx);
Bool Text2D_PointOver(DrawableContext *ctx, Float x, Float y, Bool check_outline);


#endif


