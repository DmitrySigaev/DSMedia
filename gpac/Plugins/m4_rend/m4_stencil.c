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
 *		
 */

#include "m4_priv.h"

M4Stencil *m4_solid_brush();
M4Stencil *m4_texture_brush();
M4Stencil *m4_linear_gradient_brush();
M4Stencil *m4_radial_gradient_brush();



M4Color color_interpolate(u32 a, u32 b, u8 pos) 
{ 
	u32 ca = ((a>>24)     )*(u32)(0xFF-pos)+((b>>24)     )*(u32)pos;
	u32 cr = ((a>>16)&0xFF)*(u32)(0xFF-pos)+((b>>16)&0xFF)*(u32)pos;
	u32 cg = ((a>> 8)&0xFF)*(u32)(0xFF-pos)+((b>> 8)&0xFF)*(u32)pos;
	u32 cb = ((a    )&0xFF)*(u32)(0xFF-pos)+((b    )&0xFF)*(u32)pos;
	return	(((ca+(ca>>8)+1)>>8)<<24)|
			(((cr+(cr>>8)+1)>>8)<<16)|
			(((cg+(cg>>8)+1)>>8)<< 8)|
			(((cb+(cb>>8)+1)>>8)    );
}




/*
	Generic gradient tools
*/

#define M4GRADIENTBUFFERSIZE	(1<<M4GRADIENTBITS)
#define M4GRADIENTMAXINTPOS		M4GRADIENTBUFFERSIZE - 1

static void gradient_update(M4BaseGradient *_this)
{
	s32 i, c, start, end, diff;
	Float maxPos = M4GRADIENTMAXINTPOS;

	if (_this->pos[0]>=0) {
		if(_this->pos[0]>0) {
			end = (s32) ( _this->pos[0] * maxPos );
			for (i=0; i<= end;i++) {
				_this->pre[i] = _this->col[0];
			}
		}
		for (c=0; c<M4GRADIENTSLOTS;c++) {
			if (_this->pos[c]>=0) {
				if (_this->pos[c+1]>=0) {
					start = (s32) (_this->pos[c] * maxPos);
					end  = (s32) (_this->pos[c+1] * maxPos);
					diff  = end-start;
	
					if (diff) {
						for (i=start;i<=end;i++) {
							_this->pre[i] = color_interpolate(_this->col[c], _this->col[c+1],
								(u8) ( ( (i-start) * 255) / diff) );
						}
					}
				} else {
					start = (s32) (_this->pos[c+0] * maxPos);
					for(i=start;i<=M4GRADIENTMAXINTPOS;i++) {
						_this->pre[i] = _this->col[c];
					}
				}
			}
		}
	}
}

static u32 gradient_get_color(M4BaseGradient *_this, s32 pos) 
{
	s32 max_pos = 1 << M4GRADIENTBITS;
	while (pos < 0) pos += max_pos;

	switch (_this->mod) {
	case M4GradientModeSpread:
		return _this->pre[(pos & max_pos) ?  M4GRADIENTMAXINTPOS - (pos % max_pos) :  pos % max_pos];

	case M4GradientModeRepeat:
		return _this->pre[pos % max_pos];

	case M4GradientModePad:
	default:
		return _this->pre[ MIN(M4GRADIENTMAXINTPOS, MAX((s32) 0, pos))];
	}
}

M4Err m4_set_gradient_interpolation(LPM4STENCIL p, Float *pos, M4Color *col, u32 count)
{
	M4BaseGradient *_this = (M4BaseGradient *) p;
	if ( (_this->type != M4StencilLinearGradient) && (_this->type != M4StencilRadialGradient) ) return M4BadParam;

	if (count>=M4GRADIENTSLOTS-1) return M4OutOfMem;
	memcpy(_this->col, col, sizeof(M4Color) * count);
	memcpy(_this->pos, pos, sizeof(Float) * count);
	_this->col[count] = 0;
	_this->pos[count] = -1;
	gradient_update(_this);
	return M4OK;
}

M4Err m4_set_gradient_mode(LPM4STENCIL p, enum M4GradientSpreadMode mode)
{
	M4BaseGradient *_this = (M4BaseGradient *) p;
	if ( (_this->type != M4StencilLinearGradient) && (_this->type != M4StencilRadialGradient) ) return M4BadParam;
	_this->mod = mode;
	return M4OK;
}

/*
	Generic stencil
*/

LPM4STENCIL m4_new_stencil(Graphics2DDriver *dr, enum M4StencilType type)
{
	M4Stencil *st;
	switch (type) {
	case M4StencilSolid:
		st = m4_solid_brush();
		break;
	case M4StencilLinearGradient:
		st = m4_linear_gradient_brush();
		break;
	case M4StencilRadialGradient:
		st = m4_radial_gradient_brush();
		break;
	case M4StencilTexture:
		st = m4_texture_brush();
		break;
	default:
		return 0L;
	}
	if (st) {
		mx2d_init(st->pmat);
		mx2d_init(st->smat);
	}
	return st;
}

void m4_delete_stencil(LPM4STENCIL st)
{
	M4Stencil *_this = (M4Stencil *) st;
	switch(_this->type) {
	case M4StencilSolid:
	case M4StencilLinearGradient:
	case M4StencilRadialGradient:
		free(_this);
		return;
	case M4StencilTexture:
	{
		M4Texture *tx = (M4Texture *)_this;
		/*destroy conversion buffer if any*/
		if ( tx->conv_buf) free( tx->conv_buf );
		/*destroy local texture iof any*/
		if (tx->owns_texture && tx->pixels) free(tx->pixels);
		free(_this);
	}
		return;
	}
}

/*internal*/
void m4_set_stencil_path_matrix(M4Stencil * _this, M4Matrix2D *mx)
{
	if (!_this->type || (_this->type > M4StencilTexture) ) return;
	mx2d_copy(_this->pmat, *mx);
	mx2d_inverse(&_this->pmat);
}
/*internal*/
void m4_set_stencil_path_frame(M4Stencil * _this, M4Rect rc)
{
	if (!_this->type || _this->type>M4StencilTexture) return;
	_this->frame = rc;
}

/*internal*/
Bool m4_get_stencil_matrix(M4Stencil * _this, M4Matrix2D *mat)
{
	if (!mat || _this->type>M4StencilTexture) return 0;
	mx2d_copy(*mat, _this->smat);
	return 1;
}


M4Err m4_stencil_set_matrix(LPM4STENCIL st, M4Matrix2D *mx)
{
	M4Stencil *_this = (M4Stencil *)st;
	if (!_this || _this->type>M4StencilTexture) return M4BadParam;
	if (mx) {
		mx2d_copy(_this->smat, *mx);
	} else {
		mx2d_init(_this->smat);
	}
	return M4OK;
}


/*
	Solid color stencil
*/

M4Stencil *m4_solid_brush()
{
	M4Brush *tmp;
	SAFEALLOC(tmp, sizeof(M4Brush));
	if (!tmp) return 0L;
	tmp->fill_run = NULL;
	tmp->color = 0xFF000000;
	tmp->type = M4StencilSolid;
	return (M4Stencil *) tmp;
}

M4Err m4_set_brush_color(LPM4STENCIL st, M4Color c)
{
	M4Brush *_this = (M4Brush *) st;
	if (!_this  || (_this ->type != M4StencilSolid) ) return M4BadParam;
	_this->color = c;
	return M4OK;
}


/*
	linear gradient stencil
*/

static void lgb_fill_run(M4Stencil *p, M4Surface *surf, s32 x, s32 y, u32 count) 
{
	s32 val;
	u32 *data = surf->stencil_pix_run;
	u32 shifter = (M4GRADIENTSCALEBITS - M4GRADIENTBITS);
	M4LinearGradient *_this = (M4LinearGradient *) p;

	val = (s32) (x * _this->smat.m[0] + y * _this->smat.m[1] + _this->smat.m[2]);
	while (count) {
		*data++ = gradient_get_color((M4BaseGradient *)_this, (val >> shifter) ); 
		val += (s32) _this->smat.m[0];
		count--;
	}
}

M4Err m4_set_linear_gradient(LPM4STENCIL st, Float start_x, Float start_y, Float end_x, Float end_y, M4Color start_col, M4Color end_col)
{
	M4Matrix2D mtx;
	SFVec2f s;
	Float f;
	M4LinearGradient *_this = (M4LinearGradient *) st;
	if (_this->type != M4StencilLinearGradient) return M4BadParam;

	_this->start.x = start_x;
	_this->start.y = start_y;
	_this->end.x = end_x;
	_this->end.y = end_y;
	s.x = end_x - start_x;
	s.y = end_y - start_y;
	f = (Float) sqrt(s.x * s.x + s.y * s.y);
	if (f) f = 1 / f;

	mx2d_init(mtx);
	mtx.m[2] = - _this->start.x;
	mtx.m[5] = - _this->start.y;
	_this->vecmat = mtx;

	mx2d_init(mtx);
	mx2d_add_rotation(&mtx, 0, 0, -1 *  (Float) atan2(s.y, s.x) );
	mx2d_add_matrix(&_this->vecmat, &mtx);

	mx2d_init(mtx);
	mx2d_add_scale(&mtx, f, f);
	mx2d_add_matrix(&_this->vecmat, &mtx);

	_this->col[0] = start_col;
	_this->col[1] = end_col;
	_this->col[2] = 0;
	_this->pos[0] = 0;
	_this->pos[1] = 1;
	_this->pos[2] = -1;
	gradient_update((M4BaseGradient *) _this);
	return M4OK;
}

M4Stencil *m4_linear_gradient_brush()
{
	s32 i;
	M4LinearGradient *tmp;
	SAFEALLOC(tmp, sizeof(M4LinearGradient));
	if (!tmp) return 0L;
	mx2d_init(tmp->vecmat);
	tmp->fill_run = lgb_fill_run;
	tmp->type = M4StencilLinearGradient;
	for(i=0; i<M4GRADIENTSLOTS; i++) tmp->pos[i]=-1;

	m4_set_linear_gradient(tmp, 0, 0, 1, 0, 0xFFFFFFFF, 0xFFFFFFFF);
	return (M4Stencil *) tmp;
}


/*
	radial gradient stencil
*/

static void rg_fill_run(M4Stencil *p, M4Surface *surf, s32 _x, s32 _y, u32 count) 
{
	Float x, y, dx, dy, b, val;
	s32 pos;
	u32 *data = surf->stencil_pix_run;
	M4RadialGradient *_this = (M4RadialGradient *) p;

	x = (Float) _x;
	y = (Float) _y;
	mx2d_apply_coords(&_this->smat, &x, &y);

	dx = x - _this->d_f.x;
	dy = y - _this->d_f.y;
	while (count) {
		b = _this->rad * ( dx * _this->d_f.x + dy * _this->d_f.y);
		val = b * b + _this->rad * ( dx * dx + dy * dy);
		pos = (s32) ( M4GRADIENTBUFFERSIZE * ( b + (Float) sqrt(val)) );
		*data++ = gradient_get_color((M4BaseGradient *)_this, pos);
		dx += _this->d_i.x;
		dy += _this->d_i.y;
		count--;
	}
}

void m4_radial_init(M4RadialGradient *_this) 
{
	SFVec2f p0, p1;
	p0.x = p0.y = p1.y = 0;
	p1.x = 1;

	mx2d_apply_point(&_this->smat, &p0);
	mx2d_apply_point(&_this->smat, &p1);
	_this->d_i.x = p1.x - p0.x;
	_this->d_i.y = p1.y - p0.y;

	_this->rad = 1 - _this->d_f.x * _this->d_f.x - _this->d_f.y * _this->d_f.y;
	if (_this->rad) {
		_this->rad = 1 / _this->rad;
	} else {
		_this->rad = M4GRADIENTBUFFERSIZE;
	}
}

M4Stencil *m4_radial_gradient_brush()
{
	s32 i;
	M4RadialGradient *tmp;
	SAFEALLOC(tmp, sizeof(M4RadialGradient));
	if (!tmp) return 0L;

	tmp->fill_run = rg_fill_run;
	tmp->type = M4StencilRadialGradient;
	for(i=0; i<M4GRADIENTSLOTS; i++) tmp->pos[i]=-1;

	tmp->center.x = tmp->center.y = 0.5;
	tmp->focus = tmp->center;
	tmp->radius = tmp->center;
	return (M4Stencil *) tmp;
}


M4Err m4_set_radial_gradient(LPM4STENCIL st, Float cx, Float cy, Float fx, Float fy, Float x_radius, Float y_radius)
{
	M4RadialGradient *_this = (M4RadialGradient *) st;
	if (_this->type != M4StencilRadialGradient) return M4BadParam;

	_this->center.x = cx;
	_this->center.y = cy;
	_this->focus.x = fx;
	_this->focus.y = fy;
	_this->radius.x = x_radius;
	_this->radius.y = y_radius;
	return M4OK;
}

/*
	Texture stencil 
*/
static s32
mul255(s32 a, s32 b)
{
	return ((a+1) * b) >> 8;
}

/*thx to charcoal for the bilinear filter*/
static u32 M4_LERP(u32 c0, u32 c1, u8 t)
{
	s32 a0, r0, g0, b0;
	s32 a1, r1, g1, b1;
	s32 a2, r2, g2, b2;

	a0 = M4C_A(c0);
	r0 = M4C_R(c0);
	g0 = M4C_G(c0);
	b0 = M4C_B(c0);
	a1 = M4C_A(c1);
	r1 = M4C_R(c1);
	g1 = M4C_G(c1);
	b1 = M4C_B(c1);

	a2 = a0 + mul255(t, (a1 - a0));
	r2 = r0 + mul255(t, (r1 - r0));
	g2 = g0 + mul255(t, (g1 - g0));
	b2 = b0 + mul255(t, (b1 - b0));
	return (a2<<24) | (r2<<16) | (g2<<8) | b2;
}


static void bmp_fill_run(M4Stencil *p, M4Surface *surf, s32 _x, s32 _y, u32 count) 
{
	s32 cx, cy;
	u32 x0, y0, pix, replace_col;
	Bool has_alpha, has_replace_cmat, has_cmat;
	Float x, y;
	u32 *data = surf->stencil_pix_run;
	M4Texture *_this = (M4Texture *) p;

	x = (Float) _x;
	y = (Float) _y;

	/* reverse to texture coords*/
	mx2d_apply_coords(&_this->smat, &x, &y);
	/* and move in absolute coords*/
	while (x<0) x += _this->width;
	while (y<0) y += _this->height;

	y0 = (s32) (y);
	has_alpha = (_this->alpha != 255) ? 1 : 0;
	has_replace_cmat = _this->cmat_is_replace ? 1 : 0;
	has_cmat = _this->cmat.identity ? 0 : 1;
	replace_col = _this->replace_col;

	while (count) {
		if (_this->inc_y) {
			cx = (s32) floor(x);
			x0 = (cx) % _this->width;
			cy = (s32) floor(y);
			y0 = (cy) % _this->height;
			y += _this->inc_y;
		} else {
			x0 = (s32) (x);
			x0 = (x0) % _this->width;
		}
		x += _this->inc_x;
		pix = _this->tx_get_pixel(_this->pixels + _this->stride*y0 + _this->Bpp*x0);

	
		/*bilinear filtering*/
		if (0 && (_this->filter==M4TextureFilterHighQuality)) {
			u32 p00, p01, p10, p11, x1, y1;
			u8 tx, ty;

			x1 = (cx+1) % _this->width;
			y1 = (cy+1) % _this->height;
			p00 = pix;
			p01 = _this->tx_get_pixel(_this->pixels + _this->stride*y0 + _this->Bpp*x1);
			p10 = _this->tx_get_pixel(_this->pixels + _this->stride*y1 + _this->Bpp*x0);
			p11 = _this->tx_get_pixel(_this->pixels + _this->stride*y1 + _this->Bpp*x1);

			tx = (u8) floor(x*255/_this->width);
			ty = (u8) floor(y*255/_this->height);

			p00 = M4_LERP(p00, p01, tx);
			p10 = M4_LERP(p10, p11, tx);
			pix = M4_LERP(p00, p10, ty);
		}

		if (has_alpha) {
			cx = ((M4C_A(pix) + 1) * _this->alpha) >> 8;
			pix = ( ((cx<<24) & 0xFF000000) ) | (pix & 0x00FFFFFF);
		}
		if (has_replace_cmat) {
			u32 __a;
			__a = M4C_A(pix);
			__a = (u32) (_this->cmat.m[18] * __a);
			pix = ((__a<<24) | (replace_col & 0x00FFFFFF));
		} else if (has_cmat) {
			pix = cmat_apply(&_this->cmat, pix);
		}
		*data++ = pix;
		count--;
	}
}


/*just a little faster...*/
static void bmp_fill_run_straight(M4Stencil *p, M4Surface *surf, s32 _x, s32 _y, u32 count) 
{
	u32 x0, y0, pix;
	u32 __a;
	Float x, y;
	char *pix_line;
	u32 *data = surf->stencil_pix_run;
	M4Texture *_this = (M4Texture *) p;

	x = _this->smat.m[0]*_x + _this->smat.m[2];
	y = _this->smat.m[4]*_y + _this->smat.m[5];

	/* and move in absolute coords*/
	while (x<0) x += _this->width;
	while (y<0) y += _this->height;

	y0 = (s32) (y);
	pix_line = _this->pixels + _this->stride*y0;

	while (count) {
		x0 = (s32) (x);
		x0 = (x0) % _this->width;
		x += _this->inc_x;
		pix = _this->tx_get_pixel(pix_line + _this->Bpp*x0);
		
		if (_this->replace_col) {
			__a = M4C_A(pix);
			pix = ((__a<<24) | (_this->replace_col & 0x00FFFFFF));
		}
		*data++ = pix;
		count--;
	}
}

void bmp_init(M4Stencil *p) 
{
	SFVec2f p0, p1;
	M4Texture *_this = (M4Texture *) p;

	p0.x = p0.y = p1.y = 0;
	p1.x = 1;
	mx2d_apply_point(&_this->smat, &p0);
	mx2d_apply_point(&_this->smat, &p1);
	_this->inc_x = p1.x - p0.x;
	_this->inc_y = p1.y - p0.y;

	_this->replace_col = 0;
	_this->cmat_is_replace = 0;
	if (!_this->cmat.identity
		&& !_this->cmat.m[0] && !_this->cmat.m[1] && !_this->cmat.m[2] && !_this->cmat.m[3]
		&& !_this->cmat.m[5] && !_this->cmat.m[6] && !_this->cmat.m[7] && !_this->cmat.m[8]
		&& !_this->cmat.m[10] && !_this->cmat.m[11] && !_this->cmat.m[12] && !_this->cmat.m[13]
		&& !_this->cmat.m[15] && !_this->cmat.m[16] && !_this->cmat.m[17] && !_this->cmat.m[19]) {
		_this->cmat_is_replace = 1;
		_this->replace_col = MAKE_ARGB_FLOAT(_this->cmat.m[18], _this->cmat.m[4], _this->cmat.m[9], _this->cmat.m[14]);
	}

	if ((_this->alpha == 255) && !_this->smat.m[1] && !_this->smat.m[3] && (_this->cmat.identity || _this->cmat_is_replace)) {
		_this->fill_run = bmp_fill_run_straight;
	} else {
		_this->fill_run = bmp_fill_run;
	}
}


M4Stencil *m4_texture_brush()
{
	M4Texture *tmp;
	SAFEALLOC(tmp, sizeof(M4Texture));
	if (!tmp) return 0L;

	tmp->fill_run = bmp_fill_run;
	tmp->type = M4StencilTexture;
	/*default is using the surface settings*/
	tmp->filter = M4_TEXTURE_FILTER_DEFAULT;
	tmp->mod = M4TextureModeTile;
	cmat_init(&tmp->cmat);
	tmp->alpha = 255;
	return (M4Stencil *) tmp;
}


/*by casting to u32 the input ARGB (BGRA on little endian) we get back to 0xAARRGGBB format on all machines*/
u32 get_pix_argb(char *pix) {return *(u32 *) pix;}
u32 get_pix_rgba(char *pix) { return M4C_ARGB(*(pix+3) & 0xFF, *pix & 0xFF, *(pix+1) & 0xFF, *(pix+2) & 0xFF); }
u32 get_pix_rgb_24(char *pix) { return M4C_ARGB(0xFF, *pix & 0xFF, *(pix+1) & 0xFF, *(pix+2) & 0xFF); }
u32 get_pix_bgr_24(char *pix) { return M4C_ARGB(0xFF, *(pix+2) & 0xFF, * (pix+1) & 0xFF, *pix & 0xFF); }
u32 get_pix_555(char *pix) { return M4C_ARGB(0xFF, (u8) ( (*(u16*)pix >> 7) & 0xf8), (u8) ( (*(u16*)pix >> 2) & 0xf8), (u8) ( (*(u16*)pix << 3) & 0xf8) ); }
u32 get_pix_565(char *pix) { u16 val = *(u16*)pix; return M4C_ARGB(0xFF,  (u8) ( (val >> 8) & 0xf8), (u8) ( (val >> 3) & 0xfc),  (u8) ( (val << 3) & 0xf8)	); }
u32 get_pix_grey(char *pix) { u8 val = *pix; return M4C_ARGB(0xFF, val, val, val); }
u32 get_pix_alphagrey(char *pix) { return M4C_ARGB(*(pix+1), *pix, *pix, *pix); }

static void texture_set_callback(M4Texture *_this)
{
	switch (_this->pixel_format) {
	case M4PF_RGBA:
		_this->tx_get_pixel = get_pix_rgba;
		return;
	case M4PF_ARGB:
	case M4PF_RGB_32:
		_this->tx_get_pixel = get_pix_argb;
		return;
	case M4PF_RGB_24:
		_this->tx_get_pixel = get_pix_rgb_24;
		return;
	case M4PF_BGR_24:
		_this->tx_get_pixel = get_pix_bgr_24;
		return;
	case M4PF_RGB_555:
		_this->tx_get_pixel = get_pix_555;
		return;
	case M4PF_RGB_565:
		_this->tx_get_pixel = get_pix_565;
		return;
	case M4PF_GREYSCALE:
		_this->tx_get_pixel = get_pix_grey;
		return;
	case M4PF_ALPHAGREY:
		_this->tx_get_pixel = get_pix_alphagrey;
		return;
	}
}

M4Err m4_stencil_set_texture(LPM4STENCIL st, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat, enum M4PixelFormats destination_format_hint, Bool no_copy)
{
	M4Texture *_this = (M4Texture *) st;
	if (!_this || (_this->type != M4StencilTexture) || !pixels || !width || !height || !stride || _this->owns_texture)
		return M4BadParam;

	_this->pixels = 0L;
	_this->is_converted = 1;

	switch (pixelFormat) {
	case M4PF_ARGB:
	case M4PF_RGBA:
		_this->Bpp = 4;
		break;
	case M4PF_RGB_24:
	case M4PF_BGR_24:
		_this->Bpp = 3;
		break;
	case M4PF_RGB_555:
	case M4PF_RGB_565:
	case M4PF_ALPHAGREY:
		_this->Bpp = 2;
		break;
	case M4PF_GREYSCALE:
		_this->Bpp = 1;
		break;
	case M4PF_YV12:
	case M4PF_IYUV:
	case M4PF_I420:
		_this->orig_format = M4PF_YV12;
		_this->orig_buf = pixels;
		_this->orig_stride = stride;
		_this->is_converted = 0;
		break;
	case M4PF_YUVA:
		_this->orig_format = M4PF_YUVA;
		_this->orig_buf = pixels;
		_this->orig_stride = stride;
		_this->is_converted = 0;
		break;
	default:
		/*the rest is not supported (eg BGR32)*/
		return M4NotSupported;
	}
	_this->pixel_format = pixelFormat;
	_this->width = width;
	_this->height = height;
	_this->stride = stride;
	_this->pixels = (char *) pixels;
	texture_set_callback(_this);
	return M4OK;
}


M4Err m4_set_texture_repeat_mode(LPM4STENCIL st, enum M4TextureTiling mode)
{
	M4Texture *_this = (M4Texture *) st;
	if (!_this || (_this->type != M4StencilTexture)) return M4BadParam;
	_this->mod = mode;
	return M4OK;
}

M4Err m4_set_texture_filter(LPM4STENCIL st, enum M4TextureFilterLevel filter_mode)
{
	M4Texture *_this = (M4Texture *) st;
	if (!_this || (_this->type != M4StencilTexture)) return M4BadParam;
	_this->filter = filter_mode;
	return M4OK;
}

M4Err m4_stencil_set_color_matrix(LPM4STENCIL st, M4ColorMatrix *cmat)
{
	M4Texture *_this = (M4Texture *)st;
	if (!_this || !cmat || (_this->type!=M4StencilTexture)) return M4BadParam;
	cmat_copy(&_this->cmat, cmat);
	return M4OK;
}

M4Err m4_stencil_reset_color_matrix(LPM4STENCIL st)
{
	M4Texture *_this = (M4Texture *)st;
	if (!_this || (_this->type!=M4StencilTexture)) return M4BadParam;
	cmat_init(&_this->cmat);
	return M4OK;
}

M4Err m4_set_texture_alpha(LPM4STENCIL st, u8 alpha)
{
	M4Texture *_this = (M4Texture *)st;
	if (!_this || (_this->type!=M4StencilTexture)) return M4BadParam;
	_this->alpha = alpha;
	return M4OK;
}

M4Err m4_set_texture_view(LPM4STENCIL st, M4Rect *rc)
{
	M4Texture *_this = (M4Texture *)st;
	if (!_this || (_this->type!=M4StencilTexture)) return M4BadParam;
	if (!rc) {
		_this->viewport.x = _this->viewport.y = 0;
		_this->viewport.width = (Float) _this->width;
		_this->viewport.height = (Float) _this->height;
	} else {
		_this->viewport = *rc;
	}
	return M4OK;
}

M4Err m4_stencil_get_pixel(LPM4STENCIL st, u32 x, u32 y, M4Color *col)
{
	M4Texture *_this = (M4Texture *)st;
	if (!_this || (_this->type!=M4StencilTexture)) return M4BadParam;
	m4_set_texture_active(st);

	if (x>=_this->width || y>=_this->height) return M4BadParam;
	*col = _this->tx_get_pixel(_this->pixels + _this->stride*y + _this->Bpp*x);
	return M4OK;
}

M4Err m4_stencil_set_pixel(LPM4STENCIL st, u32 x, u32 y, M4Color col)
{
	char *pix;
	M4Texture *_this = (M4Texture *)st;
	if (!_this || (_this->type!=M4StencilTexture)) return M4BadParam;
	m4_set_texture_active(st);

	if (x>=_this->width || y>=_this->height) return M4BadParam;
	pix = _this->pixels + _this->stride*y + _this->Bpp*x;
	switch (_this->pixel_format) {
	case M4PF_ARGB:
		*(u32 *) (pix) = col;
		break;
	case M4PF_RGB_24:
		pix[0] = M4C_R(col); pix[1] = M4C_G(col); pix[2] = M4C_B(col);
		break;
	case M4PF_BGR_24:
		pix[2] = M4C_R(col); pix[1] = M4C_G(col); pix[0] = M4C_B(col);
		break;
	}
	return M4OK;
}

/*internal*/
void m4_set_texture_active(M4Stencil *st)
{
	unsigned char *y, *u, *v, *a;
	M4Texture *_this = (M4Texture *)st;
	if (_this->is_converted) return;

	/*perform YUV->RGB*/

	if (_this->orig_format == M4PF_YV12) {
		_this->Bpp = 3;
	} else {
		_this->Bpp = 4;
	}
	if (_this->Bpp * _this->width * _this->height > _this->conv_size) {
		if (_this->conv_buf) free(_this->conv_buf);
		_this->conv_size = _this->Bpp * _this->width * _this->height;
		_this->conv_buf = (unsigned char *) malloc(sizeof(unsigned char)*_this->conv_size);
	}
	y = _this->orig_buf;
	u = y + _this->orig_stride*_this->height;
	v = y + 5 * _this->orig_stride*_this->height / 4;

	if (_this->Bpp==4) {
		a = y + 3 * _this->orig_stride*_this->height / 2;
		yuva2rgb_32(_this->conv_buf, 4*_this->width, y, u, v, a, _this->orig_stride, _this->orig_stride/2, _this->width, _this->height);
		_this->pixel_format = M4PF_ARGB;
	} else {
		yuv2rgb_24(_this->conv_buf, 3*_this->width, y, u, v, _this->orig_stride, _this->orig_stride/2, _this->width, _this->height);
		_this->pixel_format = M4PF_BGR_24;
	}
	_this->is_converted = 1;
	_this->pixels = (char *) _this->conv_buf;
	_this->stride = _this->Bpp * _this->width;
	texture_set_callback(_this);
}

M4Err m4_stencil_create_texture(LPM4STENCIL st, u32 width, u32 height, enum M4PixelFormats pixelFormat)
{
	M4Texture *_this = 	(M4Texture *)st;
	if (_this->orig_buf) return M4BadParam;
	_this->pixels = 0L;
	_this->is_converted = 1;

	switch (pixelFormat) {
	case M4PF_ARGB:
	case M4PF_RGBA:
	case M4PF_RGB_32:
		_this->Bpp = 4;
		break;
	case M4PF_RGB_24:
	case M4PF_BGR_24:
		_this->Bpp = 3;
		break;
	case M4PF_RGB_555:
	case M4PF_RGB_565:
	case M4PF_ALPHAGREY:
		_this->Bpp = 2;
		break;
	case M4PF_GREYSCALE:
		_this->Bpp = 1;
		break;
	default:
		return M4NotSupported;
	}
	_this->pixel_format = pixelFormat;
	_this->width = width;
	_this->height = height;
	_this->stride = width*_this->Bpp;

	if (_this->pixels) free(_this->pixels);
	SAFEALLOC(_this->pixels , sizeof(char) * _this->stride * _this->height);
	_this->owns_texture = 1;
	texture_set_callback(_this);
	return M4OK;
}

void m4_stencil_texture_modified(LPM4STENCIL _this)
{
	return;
}
