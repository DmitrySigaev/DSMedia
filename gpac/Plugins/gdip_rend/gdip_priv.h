/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / GDIplus rasterizer plugin
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


#ifndef __GDIP_PRIV_H
#define __GDIP_PRIV_H

#include <gpac/m4_graphics.h>

#ifndef ULONG_PTR 
#define ULONG_PTR DWORD
#endif

#ifndef INT16
typedef s16 INT16;
#endif

#ifndef UINT16
typedef u16 UINT16;
#endif

#define GP_SAFEALLOC(__ptr, __struc) __ptr = (__struc*)malloc(sizeof(__struc)); if (__ptr) memset(__ptr, 0, sizeof(__struc));

/*all GDIPLUS includes for C api*/

struct IDirectDrawSurface7;

#include "GdiplusMem.h"
#include "GdiplusEnums.h"
#include "GdiplusTypes.h"
#include "GdiplusInit.h"
#include "GdiplusPixelFormats.h"
#include "GdiplusColor.h"
#include "GdiplusMetaHeader.h"
#include "GdiplusImaging.h"
#include "GdiplusColorMatrix.h"
#include "GdiplusGpStubs.h"
#include "GdiplusColor.h"
#include "GdiplusFlat.h"

#include <math.h>

#define GD_PI		3.1415926536f

/* default resolution for N-bezier curves*/
#define GDIP_DEFAULT_RESOLUTION		64

struct _gp_context
{
	ULONG_PTR gdiToken;
};


/*struct translators*/

M4INLINE GpMatrix *mat_m4_to_gdip(M4Matrix2D *mat)
{
	GpMatrix *ret;
	if (!mat) return NULL;
	GdipCreateMatrix(&ret);
	GdipSetMatrixElements(ret, mat->m[0], mat->m[3], mat->m[1], mat->m[4], mat->m[2], mat->m[5]);
	return ret;
}
M4INLINE void mat_m4_to_gdip_conv(M4Matrix2D *mat, GpMatrix *_mat)
{
	GdipSetMatrixElements(_mat, mat->m[0], mat->m[3], mat->m[1], mat->m[4], mat->m[2], mat->m[5]);
}
M4INLINE void mat_gdip_to_m4(GpMatrix *matrix, M4Matrix2D *mat)
{
	Float c[6];
	GdipGetMatrixElements(matrix, c);
	mat->m[0] = c[0];
	mat->m[1] = c[2];
	mat->m[2] = c[4];
	mat->m[3] = c[1];
	mat->m[0] = c[3];
	mat->m[0] = c[5];
}

M4INLINE void cmat_m4_to_gdip(M4ColorMatrix *mat, ColorMatrix *matrix)
{
	memset(matrix->m, 0, sizeof(Float)*5*5);
	matrix->m[0][0] = mat->m[0];
	matrix->m[1][0] = mat->m[1];
	matrix->m[2][0] = mat->m[2];
	matrix->m[3][0] = mat->m[3];
	matrix->m[4][0] = mat->m[4];
	matrix->m[0][1] = mat->m[5];
	matrix->m[1][1] = mat->m[6];
	matrix->m[2][1] = mat->m[7];
	matrix->m[3][1] = mat->m[8];
	matrix->m[4][1] = mat->m[9];
	matrix->m[0][2] = mat->m[10];
	matrix->m[1][2] = mat->m[11];
	matrix->m[2][2] = mat->m[12];
	matrix->m[3][2] = mat->m[13];
	matrix->m[4][2] = mat->m[14];
	matrix->m[0][3] = mat->m[15];
	matrix->m[1][3] = mat->m[16];
	matrix->m[2][3] = mat->m[17];
	matrix->m[3][3] = mat->m[18];
	matrix->m[4][3] = mat->m[19];
}

M4INLINE void cmat_gdip_to_m4(ColorMatrix *matrix, M4ColorMatrix *mat)
{
	mat->m[0] = matrix->m[0][0];
	mat->m[1] = matrix->m[1][0];
	mat->m[2] = matrix->m[2][0];
	mat->m[3] = matrix->m[3][0];
	mat->m[4] = matrix->m[4][0];
	mat->m[5] = matrix->m[0][1];
	mat->m[6] = matrix->m[1][1];
	mat->m[7] = matrix->m[2][1];
	mat->m[8] = matrix->m[3][1];
	mat->m[9] = matrix->m[4][1];
	mat->m[10] = matrix->m[0][2];
	mat->m[11] = matrix->m[1][2];
	mat->m[12] = matrix->m[2][2];
	mat->m[13] = matrix->m[3][2];
	mat->m[14] = matrix->m[4][2];
	mat->m[15] = matrix->m[0][3];
	mat->m[16] = matrix->m[1][3];
	mat->m[17] = matrix->m[2][3];
	mat->m[18] = matrix->m[3][3];
	mat->m[19] = matrix->m[4][3];
}

M4INLINE void gp_cmat_reset(ColorMatrix *matrix)
{
	memset(matrix->m, 0, sizeof(Float)*5*5);
	matrix->m[0][0] = matrix->m[1][1] = matrix->m[2][2] = matrix->m[3][3] = matrix->m[4][4] = 1.0;
}

#define GPMATRIX() GpMatrix * _mat = mat_m4_to_gdip(mat);

GpPath *gdip_create_path(LPM4PATH _this);

struct _stencil
{
	enum M4StencilType type;
	enum M4GradientSpreadMode spread;
	enum M4TextureTiling tiling;

	GpSolidFill *pSolid;

	GpMatrix *pMat;

	/*Linear gradient vars*/
	GpLineGradient *pLinear;
	GpMatrix *pLinearMat;
	GpPointF start, end;

	/*Radial gradient vars*/
	GpPathGradient *pRadial;
	GpPointF center, radius, focal;
	GpPath *circle;

	/*interpolation colors storage*/
	REAL *pos;
	ARGB *cols;
	u32 num_pos;
	Bool needs_rebuild;

	/*texture specific*/
	GpTexture *pTexture;
	GpBitmap *pBitmap;
	u32 width, height;
	ColorMatrix cmat;
	Bool has_cmat;
	PixelFormat format;
	/*GDIplus is expecting ABGR when creating a bitmap with GdipCreateBitmapFromScan0.
	Since we don't want to rewrite by hand the full image when loading textures, we
	force R->B switching */
	Bool invert_br;
	enum M4TextureFilterLevel tFilter;

	Bool texture_invalid;
	Bool has_window;
	M4Rect wnd;
	u8 alpha;

	unsigned char *conv_buf;
	u32 conv_size;
	unsigned char *orig_buf;
	u32 orig_stride, orig_format;
	Bool is_converted;
	/*not used yet, we only convert to RGB or ARGB*/
	u32 destination_format;
};
#define GPSTEN() struct _stencil *_sten = (struct _stencil *) _this; assert(_this);
#define CHECK(_type) if (_sten->type!=_type) return;
#define CHECK_RET(_type) if (_sten->type!=_type) return M4BadParam;
#define CHECK2(_t1, _t2) if ((_sten->type!=_t1) && (_sten->type!=_t2)) return;
#define CHECK2_RET(_t1, _t2) if ((_sten->type!=_t1) && (_sten->type!=_t2)) return M4BadParam;

void gp_recompute_line_gradient(LPM4STENCIL _this);
void gp_recompute_radial_gradient(LPM4STENCIL _this);

void gp_load_texture(struct _stencil *sten);

void gp_init_driver_texture(Graphics2DDriver *driver);
void gp_init_driver_common(Graphics2DDriver *driver);
void gp_init_driver_grad(Graphics2DDriver *driver);

typedef struct
{
	ULONG_PTR gdiToken;

	/*text stuff*/
	Float font_size;
	s32 font_style;
	Float whitespace_width;
	Float underscore_width;
	GpFontFamily *font;

	char font_serif[1024];
	char font_sans[1024];
	char font_fixed[1024];
} FontPriv;

FontRaster *gdip_new_font_driver();
void gdip_delete_font_driver(FontRaster *dr);



#endif	//__GDIP_PRIV_H