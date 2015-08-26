/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / plugins interfaces
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


#ifndef M4_GRAPHICS_H
#define M4_GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <m4_tools.h>

#ifndef M_PI
#define M_PI					3.14159265358979323846f
#endif
	
/* 3D matrix object used by the renderer in Column-major format*/
typedef struct
{
	Float m[16];
} M4Matrix;


/*2D point used by the renderer*/
typedef struct
{
	Float x, y;
} M4Point2D;


/*
		NOTE ON RECTANGLE USAGE:
	when M4IRect and M4Rect structure are not used in texture space, the y parameter is not the top of 
	the rectangle (visually), but the greater y value, regardless of coords flipping or not. This allows the tile 
	engine to work without knowledge of the coord systems. Therefore:
		- for centered coord systems (BIFS-like), x: left, y= top = visual top
		- for windowed coord systems (all others), x: left, y= top = visual bottom

		Special care must therefore be taken when using clippers
	In texture space, x and y are always relative to the texture origin (0, 0), and always positives.
*/

/*rect used by the renderer*/
typedef struct
{
	Float x, y;
	Float width, height;
} M4Rect;

/*integer rect object used by renderer*/
typedef struct
{
	s32 x, y;
	s32 width, height;
} M4IRect;

/* matrix object used by the renderer - transformation x,y in X, Y is:
	X = m[0]*x + m[1]*y + m[2];
	Y = m[3]*x + m[4]*y + m[5];
*/
typedef struct
{
	Float m[6];
} M4Matrix2D;

/*color matrix object - can be used by any stencil type and performs complete
color space transformation (shift, rotate, skew, add)*/
typedef struct
{
/*the matrix coefs are in rgba order eg the color RGBA is multiplied by:
	R'		m0	m1	m2	m3	m4			R
	G'		m5	m6	m7	m8	m9			G
	B'	=	m10	m11	m12	m13	m14		x	B
	A'		m15	m16	m17	m18	m19			A
	0		0	0	0	0	1			0
	Coeficients are in intensity scale (range [0, 1])
*/
	Float m[20];
	/*internal flag to speed up things when matrix is identity READ ONLY - DO NOT MODIFY*/
	u32 identity;
} M4ColorMatrix;



/* base color manip - color is of the form 0xAARRGGBB*/
typedef u32 M4Color;

#define MAKEARGB(a, r, g, b) (a<<24 | r <<16 | g<<8 | b)

/*opaque handler for all stencils*/
typedef void * LPM4STENCIL;

/*stencil types*/
enum M4StencilType
{
	/*solid color stencil*/
	M4StencilSolid = 0,
	/*linear color gradient stencil*/
	M4StencilLinearGradient,
	/*radial color gradient stencil*/
	M4StencilRadialGradient,
	/*vertex color gradient stencil*/
	M4StencilVertexGradient,
	/*texture stencil*/
	M4StencilTexture,
};

/*stencil alignment*/
enum M4LineAlignment
{	
	/*line is centered around virtual path (-width/2, +width/2)*/
	M4LineCentered = 0,
	/*line is inside virtual path*/
	M4LineInside,
	/*line is outside virtual path*/
	M4LineOutside,
};

/*line cap*/
enum M4LineCap
{
	/*end of line is flat*/
	M4LineCapFlat = 0,
	/*end of line is round*/
	M4LineCapRound,
	/*end of line is square*/
	M4LineCapButt,
	/*end of line is square*/
	M4LineCapTriangle,
};

/*line join - match XLineProperties values*/
enum M4LineJoin
{
	/*miter join*/
	M4LineJoinMiter = 0,
	/*round join*/
	M4LineJoinRound,
	/*bevel join*/
	M4LineJoinBevel,
};

/*dash styles*/
enum M4LineDash
{
	/*no dash - default*/
	M4StrikePlain = 0,
	M4StrikeDash,
	M4StrikeDot,
	M4StrikeDashDot,
	M4StrikeDashDashDot,
	M4StrikeDashDotDot,
	M4StrikeCustom,
};

/*when dash style is custom, structure indicates dash pattern - DO NOT CHANGE THIS STRUCTURE IT IS TYPECASTED TO 
MFFLOAT*/
typedef struct _dash_settings
{
	u32 num_dash;
	Float *dashes;
} M4DashSettings;

typedef struct _pen_settings
{
	Float width;
	enum M4LineDash dash;
	enum M4LineAlignment align;
	enum M4LineCap cap;
	enum M4LineJoin join;
	Float miterLimit;
	u32 is_scalable;
	Float dash_offset;
	/*valid only when dash style is M4StrikeCustom*/
	M4DashSettings *dash_set;
} M4PenSettings;


/*gradient filling modes*/
enum M4GradientSpreadMode
{
	/*edge colors are repeated until path is filled*/
	M4GradientModePad,
	/*pattern is inversed each time it's repeated*/
	M4GradientModeSpread,
	/*pattern is repeated to fill path*/
	M4GradientModeRepeat
};


/*texture modes*/
enum M4TextureTiling
{
	/*texture is tiled across the path - this is the default mode if not specified*/
	M4TextureModeTile = 0,
	/*tiling with X-fliping at each repeat*/
	M4TextureModeTileFlipX,
	/*tiling with Y-fliping at each repeat*/
	M4TextureModeTileFlipY,
	/*tiling with X and Y-fliping at each repeat*/
	M4TextureModeTileFlipXY
};

/*filter levels for texturing - up to the graphics engine but the following levels are used by
the client*/
enum M4TextureFilterLevel
{
	/*high speed mapping (ex, no filtering applied)*/
	M4TextureFilterHighSpeed,
	/*compromise between speed and quality (ex, filter to nearest pixel)*/
	M4TextureFilterInter,
	/*high quality mapping (ex, bi-linear/bi-cubic interpolation)*/
	M4TextureFilterHighQuality
};



/* rasterizer antialiasing depending on the graphics engine*/
enum M4RasterLevel
{
	/*raster should use fastest mode possible (eg, no antialiasing)*/
	M4RasterHighSpeed,
	/*raster should use fast mode and good quality if possible*/
	M4RasterInter,
	/*raster should use antialiasing*/
	M4RasterHighQuality
};

/* 2D path object */
typedef struct _path *LPM4PATH;

/*path filling rules - by default path uses odd/even */
enum M4PathFillMode
{
	/*odd-even rule */
	M4PathFillOddEven = 0,
	/*zero-nonzero rule*/
	M4PathFillZeroNonZero,
};

/*creates new path*/
LPM4PATH m4_new_path();
/*deletes path*/
void m4_path_delete(LPM4PATH _this);
/*resets path*/
M4Err m4_path_reset(LPM4PATH _this);
/*clones path*/
LPM4PATH m4_path_clone(LPM4PATH _this);
/*adds moveTo*/
M4Err m4_path_add_move_to(LPM4PATH _this, Float x, Float y);
/*adds lineTo*/
M4Err m4_path_add_line_to(LPM4PATH _this, Float x, Float y);
/*adds centered ellipse*/
M4Err m4_path_add_ellipse(LPM4PATH _this, Float a_axis, Float b_axis);
/*adds rectangle centered on cx, cy*/
M4Err m4_path_add_rectangle(LPM4PATH _this, Float cx, Float cy, Float w, Float h);
/*adds path (concatenates)*/
M4Err m4_path_add_path(LPM4PATH _this, LPM4PATH _with_path, u32 joinPath);
/*adds quadratic (conic) bezier - flatening is done using path fineness*/
M4Err m4_path_add_quadratic_to(LPM4PATH _this, Float c_x, Float c_y, Float x, Float y);
/*adds cubic bezier - flatening is done using path fineness*/
M4Err m4_path_add_cubic_to(LPM4PATH _this, Float c1_x, Float c1_y, Float c2_x, Float c2_y, Float x, Float y);
/*adds N-order bezier - flatening is done using path fineness*/
M4Err m4_path_add_bezier(LPM4PATH _this, M4Point2D *pts, u32 nbPoints);
/*add elliptic arc from start to end with given focal points - this function doesn't check
whether the final point is or not on the ellipse computed from current point, fa and fb
@cw: indicates if the arc shall be swept clockwise or counter-clockwise*/
M4Err m4_path_add_arc_to(LPM4PATH _this, Float end_x, Float end_y, Float fa_x, Float fa_y, Float fb_x, Float fb_y, Bool sweep_clockwise);
/*adds conterclockwise arc from start_angle to end_angle.
close_type: 0=not closed, 1=chord closed, 2=pie(center) closed
*/
M4Err m4_path_add_arc(LPM4PATH _this, Float radius, Float start_angle, Float end_angle, u32 close_type);
/*closes path (add line to last moveTo)*/
M4Err m4_path_close(LPM4PATH _this);
/*gets bounding rectangle*/
M4Err m4_path_get_bounds(LPM4PATH _this, M4Rect *rc);
/*returns path outline*/
LPM4PATH m4_path_get_outline(LPM4PATH _this, M4PenSettings pen);
/*sets path fineness*/
void m4_path_set_fineness(LPM4PATH _this, Float fineness);
/*gets path fineness*/
Float m4_path_get_fineness(LPM4PATH _this);
/*sets path fill mode*/
void m4_path_set_fill_mode(LPM4PATH _this, u32 fillMode);
/*returns path fill mode*/
u32 m4_path_get_fill_mode(LPM4PATH _this);
/*returns number of subpath (a subpath is a set of connected lines)*/
u32 m4_path_get_subpath_count(LPM4PATH _this);
/*returns number of points in subpath*/
u32 m4_subpath_get_points_count(LPM4PATH _this, u32 sub_idx);
/*returns points in subpath - do NOT touch the points*/
M4Point2D *m4_subpath_get_points(LPM4PATH _this, u32 sub_idx);
/*returns true if point is over path (uses fillMode)*/
Bool m4_path_point_over(LPM4PATH _this, Float x, Float y);

enum
{
	/*complex/unknown*/
	M4_PolyComplex,
	/*counter-clockwise convex*/
	M4_PolyConvexCCW,
	/*clockwise convex*/
	M4_PolyConvexCW,
	/*convex line (degenerated path - all segments aligned)*/
	M4_PolyConvexLine
};
/*get convexity info for this 2D polygon*/
u32 polygon2D_check_convexity(M4Point2D *pts, u32 nb_pts);


/*visual surface handler*/
typedef void *LPM4SURFACE;

/*interface name for video driver*/
#define M4_GRAPHICS_2D_INTERFACE		FOUR_CHAR_INT('M','G','2','D')

/*graphics driver*/
typedef struct _graphics_driver
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	LPM4STENCIL (*new_stencil) (struct _graphics_driver *, enum M4StencilType type);
	/*common destructor for all stencils*/
	void (*delete_stencil) (LPM4STENCIL _this);
	/*set stencil transformation matrix*/
	M4Err (*stencil_set_matrix) (LPM4STENCIL _this, M4Matrix2D *mat);
	/*solid brush - set brush color*/
	M4Err (*set_brush_color) (LPM4STENCIL _this, M4Color c);
	/*gradient brushes*/
	/*sets gradient repeat mode - return M4NotSupported if driver doesn't support this to let the app compute repeat patterns
	this may be called before the gradient is setup*/
	M4Err (*set_gradient_mode) (LPM4STENCIL _this, enum M4GradientSpreadMode mode);
	/*set linear gradient.  line is defined by start and end, and you can give interpolation colors at specified positions*/
	M4Err (*set_linear_gradient) (LPM4STENCIL _this, Float start_x, Float start_y, Float end_x, Float end_y, M4Color start_col, M4Color end_col);
	/*radial gradient brush center point, focal point and radius - colors can only be set through set_interpolation */
	M4Err (*set_radial_gradient) (LPM4STENCIL _this, Float cx, Float cy, Float fx, Float fy, Float x_radius, Float y_radius);
	/*radial and linear gradient (not used with vertex) - set color interpolation at given points, 
		@pos[i]: distance from (center for radial, start for linear) expressed between 0 and 1 (1 being the gradient bounds)
		@col[i]: associated color
	NOTE 1: the colors at 0 and 1.0 MUST be provided
	NOTE 2: colors shall be fed in order from 0 to 1
	NOTE 3: this overrides the colors provided for linear gradient
	*/
	M4Err (*set_gradient_interpolation) (LPM4STENCIL _this, Float *pos, M4Color *col, u32 count);

	/*vertex gradient : set limit path */
	M4Err (*set_vertex_path) (LPM4STENCIL _this, LPM4PATH path);
	/*set the center of the gradient*/
	M4Err (*set_vertex_center) (LPM4STENCIL _this, Float cx, Float cy, u32 color);
	/*set the center of the gradient*/
	M4Err (*set_vertex_colors) (LPM4STENCIL _this, u32 *colors, u32 nbCol);
	
	
	/*set stencil texture
		@pixels: texture data, from top to bottom
		@width, @height: texture size
		@stride: texture horizontal pitch (bytes to skip to get to next row)
		@pixelFormat: texture pixel format as defined in file m4_tools.h
		@destination_format_hint: this is the current pixel format of the destination surface, and is given
		as a hint in case the texture needs to be converted by the stencil
		@no_copy: if set, specifies the texture data shall not be cached by the plugin (eg it must be able
		to directly modify the given memory
	NOTE: this stencil acts as a data wrapper, the pixel data is not required to be locally copied
	data is not required to be available for texturing until the stencil is used in a draw operation
	*/
	M4Err (*stencil_set_texture) (LPM4STENCIL _this, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat, enum M4PixelFormats destination_format_hint, Bool no_copy);
	/*creates internal texture - pixel data is owned by texture brush - set to NULL if not supported - this is used to 
	cope with engines that don't support random strides (ex: Gdiplus needs stride to be a multiple of 4) 
	if not set the renderer will create its own mem texture and pass it through set_texture - pixel format shall 
	be respected as far as Alpha is concerned (eg alpha info shall be kept and used in blit) */
	M4Err (*stencil_create_texture) (LPM4STENCIL _this, u32 width, u32 height, enum M4PixelFormats pixelFormat);
	/*signals the texture has been modified (internal texture only)*/
	void (*stencil_texture_modified) (LPM4STENCIL _this);

	/*sets texture view area - if rect is NULL the full texture is used during mapping*/
	M4Err (*set_texture_view) (LPM4STENCIL _this, M4Rect *rc);
	/*sets alpha blending level for texture - the alpha channel shall be combined with the color matrix if any*/
	M4Err (*set_texture_alpha) (LPM4STENCIL _this, u8 alpha);
	/*sets texture tile mode*/
	M4Err (*set_texture_repeat_mode) (LPM4STENCIL _this, enum M4TextureTiling mode);
	/*sets texture filtering mode*/
	M4Err (*set_texture_filter) (LPM4STENCIL _this, enum M4TextureFilterLevel filter_mode);
	/*set stencil color matrix - texture stencils only*/
	M4Err (*stencil_set_color_matrix) (LPM4STENCIL _this, M4ColorMatrix *cmat);
	/*reset stencil color matrix - texture stencils only*/
	M4Err (*stencil_reset_color_matrix) (LPM4STENCIL _this);
	/*get texture pixel*/
	M4Err (*stencil_get_pixel) (LPM4STENCIL _this, u32 x, u32 y, M4Color *col);
	/*set texture pixel*/
	M4Err (*stencil_set_pixel) (LPM4STENCIL _this, u32 x, u32 y, M4Color col);

	/*creates surface object*/
	/* @center_coords: true indicates mathematical-like coord system, 
					   false indicates computer-like coord system */
	LPM4SURFACE (*new_surface) (struct _graphics_driver *, Bool center_coords);
	/* delete surface object */
	void (*delete_surface) (LPM4SURFACE _this);

	/* attach surface object to device object (Win32: HDC) width and height are target surface size*/
	M4Err (*attach_surface_to_device) (LPM4SURFACE _this, void *os_handle, u32 width, u32 height);
	/* attach surface object to stencil object*/
	M4Err (*attach_surface_to_texture) (LPM4SURFACE _this, LPM4STENCIL sten);
	/* attach surface object to memory buffer if supported
		@pixels: texture data
		@width, @height: texture size
		@stride: texture horizontal pitch (bytes to skip to get to next row)
		@pixelFormat: texture pixel format
	*/
	M4Err (*attach_surface_to_buffer) (LPM4SURFACE _this, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat);

	/* detach surface object */
	void (*detach_surface) (LPM4SURFACE _this);

	/*sets rasterizer precision */
	M4Err (*surface_set_raster_level) (LPM4SURFACE _this, enum M4RasterLevel RasterSetting);
	/* set the given matrix as the current transformations for all drawn paths
	if NULL reset the current transformation */
	M4Err (*surface_set_matrix) (LPM4SURFACE _this, M4Matrix2D *mat);
	/* set the given rectangle as a clipper - nothing will be drawn outside this clipper
	if the clipper is NULL then no clipper is set
	NB: the clipper is not affected by the surface matrix and is given in pixels
	CF ABOVE NOTE ON CLIPPERS*/
	M4Err (*surface_set_clipper) (LPM4SURFACE _this, M4IRect *rc);

	/*sets the given path as the current one for drawing - the surface transform is NEVER changed between
	setting the path and filling, only the clipper may change*/
	M4Err (*surface_set_path) (LPM4SURFACE _this, LPM4PATH path);
	/*fills the current path using the given stencil - can be called several times with the same current path*/
	M4Err (*surface_fill) (LPM4SURFACE _this, LPM4STENCIL stencil);

	/*flushes to surface*/
	M4Err (*surface_flush) (LPM4SURFACE _this);

	/*clears given pixel rect on the surface with the given color - REQUIRED
	the given rect is formatted as a clipper - CF ABOVE NOTE ON CLIPPERS*/
	M4Err (*surface_clear)(LPM4SURFACE _this, M4IRect *rc, M4Color col);

/*private:*/
	void *internal;
} Graphics2DDriver;


#define M4_FONT_RASTER_INTERFACE		FOUR_CHAR_INT('M','F','N','T')

typedef struct _font_raster
{
	/* interface declaration*/
	M4_DECL_PLUGIN_INTERFACE

	/*inits font engine.*/
	M4Err (*init_font_engine)(struct _font_raster*dr);
	/*shutdown font engine*/
	M4Err (*shutdown_font_engine)(struct _font_raster*dr);

	/*set active font . @styles indicates font styles (PLAIN, BOLD, ITALIC, 
	BOLDITALIC and UNDERLINED, STRIKEOUT)*/
	M4Err (*set_font)(struct _font_raster*dr, const char *fontName, const char *styles);
	/*set active font pixel size*/
	M4Err (*set_font_size)(struct _font_raster*dr, Float pixel_size);
	/*gets font metrics*/
	M4Err (*get_font_metrics)(struct _font_raster*dr, Float *ascent, Float *descent, Float *lineSpacing);
	/*gets size of the given string (wide char)*/
	M4Err (*get_text_size)(struct _font_raster*dr, const unsigned short *string, Float *width, Float *height);

	/*add text to path - graphics driver may be changed at any time, therefore the font engine shall not 
	cache graphics data
		@path: target path
		x and y scaling: string display length control by the renderer, to apply to the text string
		left and top: top-left corner where to place the string (top alignment)
		ascent: offset between @top and baseline - may be needed by some fonts engine
		bounds: output bounds of the added text including white space
		flipText: true = BIFS, false = SVG
	*/
	M4Err (*add_text_to_path)(struct _font_raster*dr, LPM4PATH path, Bool flipText,
					const unsigned short* string, Float left, Float top, Float x_scaling, Float y_scaling, 
					Float ascent, M4Rect *bounds);
/*private*/
	void *priv;
} FontRaster;


#ifdef __cplusplus
}
#endif


#endif	/*M4_GRAPHICS_H*/

