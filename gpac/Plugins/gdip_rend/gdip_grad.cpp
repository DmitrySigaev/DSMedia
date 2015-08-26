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

#include "gdip_priv.h"


LPM4STENCIL gp_new_stencil(Graphics2DDriver *, enum M4StencilType type)
{
	struct _stencil *sten;
	
	switch (type) {
	case M4StencilSolid:
	case M4StencilLinearGradient:
	case M4StencilRadialGradient:
	case M4StencilVertexGradient:
	case M4StencilTexture:
		break;
	default:
		return NULL;
	}
	GP_SAFEALLOC(sten, struct _stencil);
	sten->type = type;
	sten->alpha = 255;
	return (LPM4STENCIL) sten;
}

static
void gp_delete_stencil(LPM4STENCIL _this)
{
	GPSTEN();
	if (_sten->pSolid) GdipDeleteBrush(_sten->pSolid);
	if (_sten->pTexture) GdipDeleteBrush(_sten->pTexture);
	if (_sten->pLinear) GdipDeleteBrush(_sten->pLinear);
	if (_sten->pRadial) GdipDeleteBrush(_sten->pRadial);
	if (_sten->circle) GdipDeletePath(_sten->circle);
	if (_sten->pMat) GdipDeleteMatrix(_sten->pMat);
	if (_sten->pLinearMat) GdipDeleteMatrix(_sten->pLinearMat);
	if (_sten->pBitmap) GdipDisposeImage(_sten->pBitmap);
	if (_sten->conv_buf) free(_sten->conv_buf);

	if (_sten->cols) delete [] _sten->cols;
	if (_sten->pos) delete [] _sten->pos;

	free(_sten);
}
static
M4Err gp_stencil_set_matrix(LPM4STENCIL _this, M4Matrix2D *mat)
{
	GPSTEN();
	GPMATRIX();
	if (_sten->pMat) GdipDeleteMatrix(_sten->pMat);
	_sten->pMat = _mat;
	return M4OK;
}

static
M4Err gp_set_brush_color(LPM4STENCIL _this, M4Color c)
{
	GPSTEN();
	CHECK_RET(M4StencilSolid);
	if (!_sten->pSolid) 
		GdipCreateSolidFill(c, &_sten->pSolid);
	else
		GdipSetSolidFillColor(_sten->pSolid, c);

	return M4OK;
}


static
M4Err gp_set_gradient_mode(LPM4STENCIL _this, enum M4GradientSpreadMode mode)
{
	GPSTEN();
	CHECK2_RET(M4StencilLinearGradient, M4StencilRadialGradient);
	_sten->spread = mode;
	_sten->needs_rebuild = 1;
	return M4OK;
}

static 
M4Err gp_set_linear_gradient (LPM4STENCIL _this, Float start_x, Float start_y, Float end_x, Float end_y, M4Color start_col, M4Color end_col)
{
	GPSTEN();
	CHECK_RET(M4StencilLinearGradient);
	if (_sten->pLinear) GdipDeleteBrush(_sten->pLinear);

	_sten->start.X = start_x;
	_sten->start.Y = start_y;
	_sten->end.X = end_x;
	_sten->end.Y = end_y;

	GdipCreateLineBrush(&_sten->start, &_sten->end, start_col, end_col, WrapModeTile, &_sten->pLinear);
	if (!_sten->pLinearMat) GdipCreateMatrix(&_sten->pLinearMat);
	GdipGetLineTransform(_sten->pLinear, _sten->pLinearMat);
	_sten->needs_rebuild = 1;
	return M4OK;
}

void gp_recompute_line_gradient(LPM4STENCIL _this)
{
	GpPointF start, end;
	u32 i, k;
	REAL w, h;
	GPSTEN();

	if (!_sten->needs_rebuild) return;
	_sten->needs_rebuild = 0;

	if (_sten->pLinear) GdipDeleteBrush(_sten->pLinear);
	GdipCreateLineBrush(&_sten->start, &_sten->end, 0xFFFF0000, 0xFFFF00FF, WrapModeTile, &_sten->pLinear);
	switch (_sten->spread) {
	case M4GradientModePad:
		break;
	case M4GradientModeSpread:
		GdipSetLineWrapMode(_sten->pLinear, WrapModeTileFlipXY);
		GdipSetLinePresetBlend(_sten->pLinear, (ARGB *) _sten->cols, _sten->pos, _sten->num_pos);
		return;
	case M4GradientModeRepeat:
		GdipSetLineWrapMode(_sten->pLinear, WrapModeTile);
		GdipSetLinePresetBlend(_sten->pLinear, (ARGB *) _sten->cols, _sten->pos, _sten->num_pos);
		return;
	}
	/*currently gdiplus doesn't support padded mode on gradients, so update the line gradient by 
	using a line 3 times longer*/
	w = _sten->end.X - _sten->start.X;
	h = _sten->end.Y - _sten->start.Y;
	start.X = _sten->start.X - w;
	start.Y = _sten->start.Y - h;
	end.X = _sten->end.X + w;
	end.Y = _sten->end.Y + h;
	GdipCreateLineBrush(&start, &end, 0xFFFF0000, 0xFFFF00FF, WrapModeTile, &_sten->pLinear);
	ARGB *cols = new ARGB[_sten->num_pos+2];
	REAL *pos = new REAL[_sten->num_pos+2];

	k=0;
	for (i=0; i<_sten->num_pos; i++) {
		cols[i+k] = _sten->cols[i];
		pos[i+k] = (1 + _sten->pos[i])/3;

		if (!i) {
			pos[1] = pos[0];
			cols[1] = cols[0];
			k=1;
			pos[0] = 0;
		}
	}
	pos[_sten->num_pos+1] = 1.0;
	cols[_sten->num_pos+1] = cols[_sten->num_pos];

	/*since depending on gradient transform the padding is likely to be not big enough, use flipXY to assure that in most
	cases the x3 dilatation is enough*/
	GdipSetLineWrapMode(_sten->pLinear, WrapModeTileFlipXY);
	GdipSetLinePresetBlend(_sten->pLinear, cols, pos, 2+_sten->num_pos);
	
	delete [] cols;
	delete [] pos;
}


/*GDIplus is completely bugged here, we MUST build the gradient in local coord system and apply translation
after, otherwise performances are just horrible*/
void gp_recompute_radial_gradient(LPM4STENCIL _this)
{
	s32 repeat, k;
	u32 i;
	GpPointF pt;
	GpMatrix *mat;
	GPSTEN();


	if (!_sten->needs_rebuild) return;
	_sten->needs_rebuild = 0;


	if (_sten->pRadial) {
		GdipDeleteBrush(_sten->pRadial);
		_sten->pRadial = NULL;
	}
	if (_sten->pSolid) {
		GdipDeleteBrush(_sten->pSolid);
		_sten->pSolid = NULL;
	}
	if (_sten->circle) {
		GdipDeletePath(_sten->circle);
		_sten->circle = NULL;
	}

	GdipCreatePath(FillModeAlternate, &_sten->circle);
	/*get number of repeats*/
	if (_sten->spread == M4GradientModePad) {


		GdipAddPathEllipse(_sten->circle, - _sten->radius.X, -_sten->radius.Y, 
								2*_sten->radius.X, 2*_sten->radius.Y);
	
		GdipCreatePathGradientFromPath(_sten->circle, &_sten->pRadial);

		ARGB *blends = new ARGB[_sten->num_pos + 1];

		/*radial blend pos are from bounds to center in gdiplus*/
		blends[0] = _sten->cols[_sten->num_pos - 1];
		for (i=0; i<_sten->num_pos;i++) {
			blends[i+1] = _sten->cols[_sten->num_pos - i - 1];
		}
	
		REAL *pos = new REAL[_sten->num_pos + 1];
		pos[0] = 0;
		for (i=0; i<_sten->num_pos;i++) {
			pos[i+1] = _sten->pos[i];
		}

		GdipSetPathGradientPresetBlend(_sten->pRadial, blends, pos, _sten->num_pos + 1);
		delete [] blends;
		delete [] pos;

		/*set focal*/
		pt = _sten->focal;
		pt.X -= _sten->center.X;
		pt.Y -= _sten->center.Y;
		GdipSetPathGradientCenterPoint(_sten->pRadial, &pt);	

		/*set transform*/
		GdipCreateMatrix(&mat);
		GdipTranslateMatrix(mat, _sten->center.X, _sten->center.Y, MatrixOrderAppend);
		if (_sten->pMat) GdipMultiplyMatrix(mat, _sten->pMat, MatrixOrderAppend);
		GdipSetTextureTransform((GpTexture*)_sten->pRadial, mat);
		GdipDeleteMatrix(mat);

		/*create back brush*/
		GdipCreateSolidFill(_sten->cols[_sten->num_pos - 1], &_sten->pSolid);
		GdipResetPath(_sten->circle);
		GdipAddPathEllipse(_sten->circle, - _sten->radius.X + _sten->center.X, -_sten->radius.Y + _sten->center.Y, 
								2*_sten->radius.X, 2*_sten->radius.Y);

	} else {
		repeat = 10;

		GdipAddPathEllipse(_sten->circle, - repeat * _sten->radius.X, - repeat*_sten->radius.Y, 
								2*repeat*_sten->radius.X,  2*repeat*_sten->radius.Y);

		GdipCreatePathGradientFromPath(_sten->circle, &_sten->pRadial);
		GdipDeletePath(_sten->circle);
		_sten->circle = NULL;

		ARGB *blends = new ARGB[_sten->num_pos*repeat];
		REAL *pos = new REAL[_sten->num_pos*repeat];

		if (_sten->spread == M4GradientModeRepeat) {
			for (k=0; k<repeat; k++) {
				for (i=0; i<_sten->num_pos; i++) {
					blends[k*_sten->num_pos + i] = _sten->cols[_sten->num_pos - i - 1];
					pos[k*_sten->num_pos + i] = (k + _sten->pos[i]) / repeat;
				}
			}
		} else {
			for (k=0; k<repeat; k++) {
				for (i=0; i<_sten->num_pos; i++) {
					u32 index = (k%2) ? (_sten->num_pos-i-1) : i;
					blends[k*_sten->num_pos + i] = _sten->cols[index];
					if (k%2) {
						pos[k*_sten->num_pos + i] = (k + (1 - _sten->pos[index]) ) / repeat;
					} else {
						pos[k*_sten->num_pos + i] = ( k + _sten->pos[i] ) / repeat;
					}
				}
			}
		}
		GdipSetPathGradientPresetBlend(_sten->pRadial, blends, pos, _sten->num_pos*repeat);
		delete [] pos;
		delete [] blends;


		/*set focal*/
		pt = _sten->focal;
		pt.X -= (1 - repeat) * (_sten->focal.X - _sten->center.X) + _sten->center.X;
		pt.Y -= (1 - repeat) * (_sten->focal.Y - _sten->center.Y) + _sten->center.Y;
		GdipSetPathGradientCenterPoint(_sten->pRadial, &pt);	

		/*set transform*/
		GdipCreateMatrix(&mat);
		GdipTranslateMatrix(mat, (1 - repeat) * (_sten->focal.X - _sten->center.X) + _sten->center.X,
								(1 - repeat) * (_sten->focal.Y - _sten->center.Y) + _sten->center.Y, 
								MatrixOrderAppend);
		if (_sten->pMat) GdipMultiplyMatrix(mat, _sten->pMat, MatrixOrderAppend);
		GdipSetTextureTransform((GpTexture*)_sten->pRadial, mat);
		GdipDeleteMatrix(mat);

		GdipSetPathGradientWrapMode(_sten->pRadial, WrapModeTileFlipXY);
	}
}

static 
M4Err gp_set_radial_gradient(LPM4STENCIL _this, Float cx, Float cy, Float fx, Float fy, Float x_radius, Float y_radius)
{
	GPSTEN();
	CHECK_RET(M4StencilRadialGradient);

	/*store focal info*/
	_sten->radius.X = x_radius;
	_sten->radius.Y = y_radius;
	_sten->center.X = cx;
	_sten->center.Y = cy;
	_sten->focal.X = fx;
	_sten->focal.Y = fy;
	_sten->needs_rebuild = 1;
	return M4OK;
}

static
M4Err gp_set_gradient_interpolation(LPM4STENCIL _this, Float *pos, M4Color *col, u32 count)
{
	GPSTEN();

	if (_sten->cols) delete [] _sten->cols;
	if (_sten->pos) delete [] _sten->pos;

	/*handle padding internally*/
	_sten->cols = new ARGB[count];
	_sten->pos = new REAL[count];
	memcpy(_sten->pos, pos, sizeof(REAL)*count);
	memcpy(_sten->cols, col, sizeof(ARGB)*count);
	_sten->num_pos = count;
	_sten->needs_rebuild = 1;
	return M4OK;
}


static 
M4Err gp_set_vertex_path(LPM4STENCIL _this, LPM4PATH path)
{
	GPSTEN();
	GpPath *p;
	CHECK_RET(M4StencilVertexGradient);
	p = gdip_create_path(path);
	if (_sten->pRadial) GdipDeleteBrush(_sten->pRadial);
	GdipCreatePathGradientFromPath(p, &_sten->pRadial);
	GdipDeletePath(p);
	return M4OK;
}

static 
M4Err gp_set_vertex_center (LPM4STENCIL _this, Float cx, Float cy, u32 color)
{
	GpStatus ret;
	GPSTEN();
	CHECK_RET(M4StencilVertexGradient);

	if (!_sten->pRadial) return M4BadParam;
	_sten->center.X = cx;
	_sten->center.Y = cy;

	ret = GdipSetPathGradientCenterPoint(_sten->pRadial, &_sten->center);
	ret = GdipSetPathGradientCenterColor(_sten->pRadial, (ARGB) color);
	return M4OK;
}

static
M4Err gp_set_vertex_colors (LPM4STENCIL _this, u32 *colors, u32 nbCol)
{
	int col = nbCol;
	GPSTEN();
	CHECK_RET(M4StencilVertexGradient);

	GpStatus ret;
	if (!_sten->pRadial) return M4BadParam;
	ret = GdipSetPathGradientSurroundColorsWithCount(_sten->pRadial, (ARGB *) colors, &col);
	return M4OK;
}


void gp_init_driver_grad(Graphics2DDriver *driver)
{
	driver->new_stencil = gp_new_stencil;
	driver->delete_stencil = gp_delete_stencil;
	driver->stencil_set_matrix = gp_stencil_set_matrix;
	driver->set_brush_color = gp_set_brush_color;
	driver->set_gradient_mode = gp_set_gradient_mode;
	driver->set_linear_gradient = gp_set_linear_gradient;
	driver->set_radial_gradient = gp_set_radial_gradient;
	driver->set_gradient_interpolation = gp_set_gradient_interpolation;
	driver->set_vertex_path = gp_set_vertex_path;
	driver->set_vertex_center = gp_set_vertex_center;
	driver->set_vertex_colors = gp_set_vertex_colors;
}