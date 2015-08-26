/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#ifndef _M4_SWF_DEV_H_
#define _M4_SWF_DEV_H_

#include <gpac/m4_author.h>
#include <gpac/m4_graphics.h>


#define SWF_COLOR_SCALE				(1/256.0f)
#define SWF_TWIP_SCALE				(1/20.0f)
#define SWF_TEXT_SCALE				(1/1024.0f)


enum
{
	SWF_SND_UNCOMP = 0,
	SWF_SND_ADPCM,
	SWF_SND_MP3
};

typedef struct
{
	u32 ID;
	u8 format;
	/*0: 5.5k - 1: 11k - 2: 22k - 3: 44k*/
	u8 sound_rate;
	u8 bits_per_sample;
	Bool stereo;
	u16 sample_count;
	u32 frame_delay_ms;

	/*IO*/
	FILE *output;
	char *szFileName;

	/*set when sound is setup (OD inserted)*/
	Bool is_setup;
} SWFSound;

typedef struct _swf_reader
{
	M4ContextLoader *load;
	FILE *input;

	char *localPath;
	/*file header*/
	u8 sig[3];
	u8 version;
	u32 length;
	u32 frame_rate;
	u32 frame_count;
	Float width, height;

	Bool compressed, has_interact;
	
	u32 flags;
	u32 max_depth;

	Chain *display_list;

	/*all fonts*/
	Chain *fonts;
	/*all simple appearances (no texture)*/
	Chain *apps;

	/*all sounds*/
	Chain *sounds;

	/*the one and only sound stream for current timeline*/
	SWFSound *sound_stream;

	/*bit reader*/
	BitStream *bs;
	
	/*current tag*/
	u32 tag, size;

	/*current BIFS stream*/
	M4StreamContext *bifs_es;
	M4AUContext *bifs_au;

	/*for sound insert*/
	SFNode *root;

	/*current OD AU*/
	M4StreamContext *od_es;
	M4AUContext *od_au;

	u32 current_frame;
	M4Err ioerr;	

	/*when creating sprites: 
		1- all BIFS AUs in sprites are random access
		2- depth is ignored in Sprites
	*/
	u32 current_sprite_id;

	/*the parser can decide to remove nearly aligned pppoints in lineTo sequences*/
	/*flatten limit - 0 means no flattening*/
	Float flat_limit;
	/*number of points removed*/
	u32 flatten_points;

	u16 prev_od_id, prev_es_id;
} SWFReader;

SFNode *SWF_NewNode(SWFReader *read, u32 tag);

void swf_report(SWFReader *read, M4Err e, char *format, ...);

typedef struct
{
	Float x, y;
	Float w, h;
} SWFRec;

typedef struct
{
	/*0: not defined, otherwise index of shape*/
	u32 nbType;
	/*0: moveTo, 1: lineTo, 2: quad curveTo*/
	u32 *types;
	SFVec2f *pts;
	u32 nbPts;
} SWFPath;

typedef struct
{
	u32 type;
	u32 solid_col;
	u32 nbGrad;
	u32 *grad_col;
	u8 *grad_ratio;
	M4Matrix2D mat;
	u32 img_id;
	Float width;

	SWFPath *path;
} SWFShapeRec;


typedef struct
{
	Chain *fill_left, *fill_right, *lines;
	u32 ID;
} SWFShape;

/*converts SWF shape to BIFS shape*/
SFNode *SWFShapeToBIFS(SWFReader *read, SWFShape *shape);

/*returns new appearance or USE any existing one (base fill/strike only)*/
SFNode *SWF_GetAppearance(SWFReader *read, SFNode *parent, u32 fill_col, Float line_width, u32 l_col);

/*display list item (one per layer only)*/
typedef struct
{
	M4Matrix2D mat;
	M4ColorMatrix cmat;
	u32 depth;
	SFNode *n;
} DispShape;

/*SWF font object*/
typedef struct
{
	u32 fontID;
	u32 nbGlyphs;
	Chain *glyphs;

	/*the following may all be overridden by a DefineFontInfo*/

	/*index -> glyph code*/
	u16 *glyph_codes;
	/*index -> glyph advance*/
	s16 *glyph_adv;

	/*font flags (SWF 3.0)*/
	Bool has_layout;
	Bool has_shiftJIS;
	Bool is_unicode, is_ansi;
	Bool is_bold, is_italic;
	s16 ascent, descent, leading;

	/*font familly*/
	char *fontName;
} SWFFont;

/*chunk of text with the same aspect (font, col)*/
typedef struct
{
	u32 fontID;
	u32 col;
	/*font size scaling (so that glyph coords * fontHeight is in TWIPs) */
	Float fontHeight;
	/*origin point in local metrics*/
	Float orig_x, orig_y;

	u32 nbGlyphs;
	u32 *indexes;
	Float *dx;
} SWFGlyphRec;

typedef struct
{
	M4Matrix2D mat;
	Chain *text;
} SWFText;

SFNode *SWFTextToBIFS(SWFReader *read, SWFText *text);

SWFFont *SWF_FindFont(SWFReader *read, u32 fontID);

/*insert node in dictionary*/
M4Err SWF_InsertNode(SWFReader *read, SFNode *n);

SFNode *SWF_GetBIFSMatrix(SWFReader *read, M4Matrix2D *mat);
SFNode *SWF_GetBIFSColorMatrix(SWFReader *read, M4ColorMatrix *cmat);

enum
{
	SWF_END = 0,
	SWF_SHOWFRAME = 1,
	SWF_DEFINESHAPE = 2,
	SWF_FREECHARACTER = 3,
	SWF_PLACEOBJECT = 4,
	SWF_REMOVEOBJECT = 5,
	SWF_DEFINEBITS = 6,
	SWF_DEFINEBITSJPEG = 6,
	SWF_DEFINEBUTTON = 7,
	SWF_JPEGTABLES = 8,
	SWF_SETBACKGROUNDCOLOR = 9,
	SWF_DEFINEFONT = 10,
	SWF_DEFINETEXT = 11,
	SWF_DOACTION = 12,
	SWF_DEFINEFONTINFO = 13,
	SWF_DEFINESOUND = 14,
	SWF_STARTSOUND = 15,
	SWF_DEFINEBUTTONSOUND = 17,
	SWF_SOUNDSTREAMHEAD = 18,
	SWF_SOUNDSTREAMBLOCK = 19,
	SWF_DEFINEBITSLOSSLESS = 20,
	SWF_DEFINEBITSJPEG2 = 21,
	SWF_DEFINESHAPE2 = 22,
	SWF_DEFINEBUTTONCXFORM = 23,
	SWF_PROTECT = 24,
	SWF_PLACEOBJECT2 = 26,
	SWF_REMOVEOBJECT2 = 28,
	SWF_DEFINESHAPE3 = 32,
	SWF_DEFINETEXT2 = 33,
	SWF_DEFINEBUTTON2 = 34,
	SWF_DEFINEBITSJPEG3 = 35,
	SWF_DEFINEBITSLOSSLESS2 = 36,
	SWF_DEFINEEDITTEXT = 37,
	SWF_DEFINEMOVIE = 38,
	SWF_DEFINESPRITE = 39,
	SWF_NAMECHARACTER = 40,
	SWF_SERIALNUMBER = 41,
	SWF_GENERATORTEXT = 42,
	SWF_FRAMELABEL = 43,
	SWF_SOUNDSTREAMHEAD2 = 45,
	SWF_DEFINEMORPHSHAPE = 46,
	SWF_DEFINEFONT2 = 48,
	SWF_TEMPLATECOMMAND = 49,
	SWF_GENERATOR3 = 51,
	SWF_EXTERNALFONT = 52,
	SWF_EXPORTASSETS = 56,
	SWF_IMPORTASSETS	= 57,
	SWF_ENABLEDEBUGGER = 58,
	SWF_MX0 = 59,
	SWF_MX1 = 60,
	SWF_MX2 = 61,
	SWF_MX3 = 62,
	SWF_MX4 = 63,
	SWF_REFLEX = 777
};

u32 swf_read_int(SWFReader *read, u32 nbBits);
u32 swf_align(SWFReader *read);
void swf_skip_data(SWFReader *read, u32 size);
void swf_get_rec(SWFReader *read, SWFRec *rc);
u32 swf_get_32(SWFReader *read);
u16 swf_get_16(SWFReader *read);
u32 swf_get_matrix(SWFReader *read, M4Matrix2D *mat, Bool rescale);
void swf_get_colormatrix(SWFReader *read, M4ColorMatrix *cmat);
M4Err swf_seek_file_to(SWFReader *read, u32 size);
u32 swf_get_file_pos(SWFReader *read);


M4Err swf_define_button(SWFReader *read, u32 revision);


#endif /*_M4_SWF_DEV_H_*/
