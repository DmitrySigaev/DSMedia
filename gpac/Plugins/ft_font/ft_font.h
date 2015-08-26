/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / FreeType font engine plugin
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


#ifndef __FT_FONT_H
#define __FT_FONT_H

#include <gpac/m4_graphics.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
/*TrueType tables*/
#include FT_TRUETYPE_TABLES_H 


typedef struct
{
	FT_Library library;
	FT_Face active_face;

	char *font_dir;

	Float pixel_size;

	Chain *loaded_fonts;

	/*0: no line, 1: underlined, 2: strikeout*/
	u32 strike_style;

	Bool register_font;

	/*temp storage for enum - may be NULL*/
	const char *tmp_font_name;
	const char *tmp_font_style;
	/*default fonts*/
	char font_serif[1024];
	char font_sans[1024];
	char font_fixed[1024];
} FTBuilder;


#endif
