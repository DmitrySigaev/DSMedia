/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / software rasterizer plugin
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
 *
 *	Most of this module is a C code rewrite of the charcoal lib (mupdf)
 *	written by Tor Andersson <tor.andersson@dsek.lth.se> published under the GPL license
 *		
 */


#ifndef _M4PRIV_H_
#define _M4PRIV_H_

#include <gpac/m4_graphics.h>
#include <gpac/intern/m4_render_tools.h>

/*math.h is not included in main config (C++ clash on win32)*/
#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif

/*force standalone compil of ftgrays (freetype rasterizer)*/
#ifndef _STANDALONE_
#define _STANDALONE_
#endif

/*freetype rasterizer*/
#include "ftheader.h"
#include "ftgrays.h"

typedef struct _surface M4Surface;

/*base stencil stack*/
#define M4BASESTENCIL	\
	u32 type;	\
	void (*fill_run)(struct _stencil*p, M4Surface *surf, s32 x, s32 y, u32 count);	\
	M4Matrix2D pmat;					\
	M4Matrix2D smat;					\
	M4Rect frame;					\

typedef struct _stencil
{
	M4BASESTENCIL
} M4Stencil;


/*16 k of pool buffer for FreeType raster - min is 4k*/
#define M4_FT_POOL_SIZE		16318


/*the surface object - currently only ARGB/RGB32, RGB/BGR and RGB555/RGB565 supported*/
struct _surface
{
	/*surface info*/
	char *pixels;
	u32 pixelFormat, BPP;
	u32 width, height, stride;
	Bool center_coords;

	/*color buffer for variable stencils - size of width*/
	u32 *stencil_pix_run;

	/*aliasing flag (on/off only)*/
	u8 AALevel;
	/*default texture filter level*/
	u32 texture_filter;

	u32 useClipper;
	M4IRect clipper;

	M4Rect path_bounds;

	/*complete path transform (including page flipping/recenter)*/
	M4Matrix2D mat;

	/*stencil currently used*/
	M4Stencil *sten;

	/*in solid color mode to speed things*/
	u32 fill_col;
	u32 fill_565;
	u32 fill_555;

	/*FreeType outline (path converted to ft)*/
	FT_Outline ftoutline;
	FT_Raster_Params ftparams;
	u32 pointlen;
	u32 contourlen;
	FT_Vector *points;
	s8 *tags;
	s16 *contours;

	/*FreeType raster and pool size*/
	char raster_pool[M4_FT_POOL_SIZE];
	void *ftrast_mem;
	FT_Raster ftraster;
};

LPM4SURFACE m4_new_surface(Graphics2DDriver *dr, Bool center_coords);
void m4_delete_surface(LPM4SURFACE _this);

M4Err m4_attach_surface_to_device(LPM4SURFACE _this, void *os_handle, u32 width, u32 height);
M4Err m4_attach_surface_to_buffer(LPM4SURFACE _this, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat);
M4Err m4_attach_surface_to_texture(LPM4SURFACE _this, LPM4STENCIL sten);
void m4_detach_surface(LPM4SURFACE _this);

M4Err m4_surface_set_matrix(LPM4SURFACE surf, M4Matrix2D *mat);
M4Err m4_surface_set_raster_level(LPM4SURFACE surf , enum M4RasterLevel RasterSetting);
M4Err m4_surface_set_clipper(LPM4SURFACE surf, M4IRect *rc);
M4Err m4_surface_set_path(LPM4SURFACE surf, LPM4PATH _path);
M4Err m4_surface_fill(LPM4SURFACE surf, LPM4STENCIL stencil);
M4Err m4_surface_strike_path(LPM4SURFACE surf, LPM4PATH _path, LPM4STENCIL stencil, M4PenSettings pen);
M4Err m4_surface_flush(LPM4SURFACE _this);

/*solid color brush*/
typedef struct
{
	M4BASESTENCIL
	M4Color color;
} M4Brush;

/*max number of interpolation points*/
#define M4GRADIENTSLOTS		12

/*(2^M4GRADIENTBITS)-1 colors in gradient. Below 6 bits the gradient is crappy */
#define M4GRADIENTBITS		12

/*scale factor for linear - MUST BE greater than M4GRADIENTBITS*/
#define M4GRADIENTSCALEBITS		20

/*base gradient stencil*/
#define M4GRADIENT	\
		s32	mod;	\
		u32	pre[(1<<M4GRADIENTBITS)];	\
		u32	col[M4GRADIENTSLOTS];		\
		Float pos[M4GRADIENTSLOTS];	\

typedef struct
{
	M4BASESTENCIL
	M4GRADIENT
} M4BaseGradient;

/*linear gradient*/
typedef struct
{
	M4BASESTENCIL
	M4GRADIENT
	
	SFVec2f start, end;
	M4Matrix2D vecmat;
	s32	curp;
	Float pos_ft;
} M4LinearGradient;

/*radial gradient*/
typedef struct
{
	M4BASESTENCIL
	M4GRADIENT
	
	SFVec2f	center, focus;
	SFVec2f radius;

	/*drawing state vars*/
	SFVec2f	cur_p, d_f, d_i;
	Float rad;
} M4RadialGradient;

/*texture stencil - should be reworked, is very slow*/
typedef struct
{
	M4BASESTENCIL
	u32 width, height, stride;
	u32 pixel_format, Bpp;
	char *pixels;

	SFVec2f cur_pt;
	Float cur_y, inc_x, inc_y;

	u32 mod, filter;
	M4ColorMatrix cmat;
	u32 replace_col;
	Bool cmat_is_replace;


	M4Rect viewport;
	u8 alpha;

	/*YUV->RGB local buffer*/
	unsigned char *conv_buf;
	u32 conv_size;
	unsigned char *orig_buf;
	u32 orig_stride, orig_format;
	Bool is_converted;

	Bool owns_texture;

	u32 (*tx_get_pixel)(char *pix);
} M4Texture;

#define M4_TEXTURE_FILTER_DEFAULT	M4TextureFilterHighQuality + 1

/*stencils decl*/
LPM4STENCIL m4_new_stencil(Graphics2DDriver *dr, enum M4StencilType type);
void m4_delete_stencil(LPM4STENCIL st);
M4Err m4_stencil_set_matrix(LPM4STENCIL st, M4Matrix2D *mx);
M4Err m4_set_brush_color(LPM4STENCIL st, M4Color c);
M4Err m4_set_linear_gradient(LPM4STENCIL st, Float start_x, Float start_y, Float end_x, Float end_y, M4Color start_col, M4Color end_col);
M4Err m4_set_radial_gradient(LPM4STENCIL st, Float cx, Float cy, Float fx, Float fy, Float x_radius, Float y_radius);
M4Err m4_set_gradient_interpolation(LPM4STENCIL p, Float *pos, M4Color *col, u32 count);
M4Err m4_set_gradient_mode(LPM4STENCIL p, enum M4GradientSpreadMode mode);

M4Err m4_stencil_set_texture(LPM4STENCIL st, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat, enum M4PixelFormats destination_format_hint, Bool no_copy);
M4Err m4_set_texture_repeat_mode(LPM4STENCIL st, enum M4TextureTiling mode);
M4Err m4_set_texture_filter(LPM4STENCIL st, enum M4TextureFilterLevel filter_mode);
M4Err m4_stencil_set_color_matrix(LPM4STENCIL st, M4ColorMatrix *cmat);
M4Err m4_stencil_reset_color_matrix(LPM4STENCIL st);
Bool m4_stencil_has_color_matrix(LPM4STENCIL st);
M4Err m4_set_texture_alpha(LPM4STENCIL st, u8 alpha);
M4Err m4_set_texture_view(LPM4STENCIL st, M4Rect *rc);
M4Err m4_stencil_create_texture(LPM4STENCIL st, u32 width, u32 height, enum M4PixelFormats pixelFormat);
void m4_stencil_texture_modified(LPM4STENCIL _this);

#define M4C_ARGB(a, r, g, b) ( ( (a) << 24) | ((r) << 16) | ((g) << 8) | (b) )
#define M4C_555(r, g, b) (u16) (((r & 248)<<7) + ((g & 248)<<2)  + (b>>3))
#define M4C_AG(a, g) (u16) ( (a << 8) | g)

#define M4C_565(r, g, b) (u16) (((r & 248)<<8) + ((g & 252)<<3)  + (b>>3))

/*<intern>*/
void m4_set_stencil_path_matrix(M4Stencil * _this, M4Matrix2D *mx);
void m4_set_stencil_path_frame(M4Stencil * _this, M4Rect rc);
Bool m4_get_stencil_matrix(M4Stencil * _this, M4Matrix2D *mat);
void m4_radial_init(M4RadialGradient *_this) ;
void bmp_init(M4Stencil *p);
void m4_set_texture_active(M4Stencil *st);

M4Err m4_stencil_get_pixel(LPM4STENCIL st, u32 x, u32 y, M4Color *col);
M4Err m4_stencil_set_pixel(LPM4STENCIL st, u32 x, u32 y, M4Color col);

/*FT raster callbacks - it's better to have a bit bigger code than using extra 
get/set pixels routines on the surface*/
void m4_argb_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_argb_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_argb_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_argb(LPM4SURFACE surf, M4IRect rc, M4Color col);

void m4_rgb32_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_rgb32_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_rgb32_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_rgb32(LPM4SURFACE surf, M4IRect rc, M4Color col);

void m4_rgb_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_rgb_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_rgb_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_rgb(LPM4SURFACE surf, M4IRect rc, M4Color col);

void m4_bgr_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_bgr_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_bgr_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_bgr(LPM4SURFACE surf, M4IRect rc, M4Color col);

void m4_565_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_565_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_565_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_565(LPM4SURFACE _this, M4IRect rc, M4Color col);

void m4_555_fill_const(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_555_fill_const_a(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
void m4_555_fill_var(s32 y, s32 count, FT_Span *spans, M4Surface *surf);
M4Err m4_surface_clear_555(LPM4SURFACE surf, M4IRect rc, M4Color col);

M4Err m4_surface_clear(LPM4SURFACE surf, M4IRect *rc, u32 color);

/*</intern>*/


#ifdef __cplusplus
}
#endif

#endif 

