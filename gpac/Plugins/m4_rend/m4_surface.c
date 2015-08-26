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
 *	Most of this module is a C code rewrite of the charcoal lib (mupdf)
 *	written by Tor Andersson <tor.andersson@dsek.lth.se> published under the GPL license
 *		
 */


#include "m4_priv.h"

static void get_surface_world_matrix(M4Surface *_this, M4Matrix2D *mat)
{
	mx2d_init(*mat);
	if (_this->center_coords) {
		mx2d_add_scale(mat, 1, -1);
		mx2d_add_translation(mat, (Float) _this->width / 2, (Float) _this->height / 2);
	}
}

LPM4SURFACE m4_new_surface(Graphics2DDriver *dr, Bool center_coords)
{
	M4Surface *_this;
	SAFEALLOC(_this, sizeof(M4Surface));
	if (_this) {
		_this->center_coords = center_coords;
		_this->texture_filter = M4_TEXTURE_FILTER_DEFAULT;
		_this->ftparams.target = NULL;
		_this->ftparams.source = &_this->ftoutline;
		_this->ftparams.user = _this;
		my_ft_grays_raster.raster_new(_this->ftrast_mem, &_this->ftraster);
		my_ft_grays_raster.raster_reset(_this->ftraster, _this->raster_pool, M4_FT_POOL_SIZE);
	}
	return _this;
}

void m4_delete_surface(LPM4SURFACE surf)
{
	M4Surface *_this = (M4Surface *) surf;
	if (_this->contours) free(_this->contours);
	if (_this->tags) free(_this->tags);
	if (_this->points) free(_this->points);
	if (_this->stencil_pix_run) free(_this->stencil_pix_run);
	my_ft_grays_raster.raster_done(_this->ftraster);
	free(_this);
}

M4Err m4_surface_set_matrix(LPM4SURFACE surf, M4Matrix2D *mat)
{
	M4Matrix2D tmp;
	M4Surface *_this = (M4Surface *) surf;
	if (!_this) return M4BadParam;

	get_surface_world_matrix(_this, &_this->mat);
	if (!mat) return M4OK;

	mx2d_init(tmp);
	mx2d_add_matrix(&tmp, mat);
	mx2d_add_matrix(&tmp, &_this->mat);
	mx2d_copy(_this->mat, tmp);
	return M4OK;
}


M4Err m4_attach_surface_to_device(LPM4SURFACE _this, void *os_handle, u32 width, u32 height)
{
	return M4NotSupported;
}

M4Err m4_attach_surface_to_buffer(LPM4SURFACE surf, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat)
{
	u32 BPP;
	M4Surface *_this = (M4Surface *)surf;
	if (!_this || !pixels || (pixelFormat>M4PF_YUVA)) return M4BadParam;

	switch (pixelFormat) {
	case M4PF_RGB_555:
	case M4PF_RGB_565:
		BPP = 2;
		break;
	case M4PF_BGR_24:
	case M4PF_RGB_24:
		BPP = 3;
		break;
	case M4PF_RGB_32:
	case M4PF_ARGB:
		BPP = 4;
		break;
	case M4PF_BGR_32:
	default:
		return M4NotSupported;
	}
	_this->stride = stride;
	_this->width = width;
	if (_this->stencil_pix_run) free(_this->stencil_pix_run);
	SAFEALLOC(_this->stencil_pix_run , sizeof(u32) * (width+2));
	_this->height = height;
	_this->pixels = pixels;
	_this->pixelFormat = pixelFormat;
	_this->BPP = BPP;
	m4_surface_set_matrix(_this, NULL);
	return M4OK;
}


M4Err m4_attach_surface_to_texture(LPM4SURFACE surf, LPM4STENCIL sten)
{
	u32 BPP;
	M4Surface *_this = (M4Surface *)surf;
	M4Texture *tx = (M4Texture *) sten;;
	if (!_this || (tx->type != M4StencilTexture)) return M4BadParam;

	switch (tx->pixel_format) {
	case M4PF_GREYSCALE:
		BPP = 1;
		break;
	case M4PF_ALPHAGREY:
	case M4PF_RGB_555:
	case M4PF_RGB_565:
		BPP = 2;
		break;
	case M4PF_BGR_24:
	case M4PF_RGB_24:
		BPP = 3;
		break;
	case M4PF_RGB_32:
	case M4PF_ARGB:
		BPP = 4;
		break;
	default:
		return M4NotSupported;
	}
	_this->stride = tx->stride;
	if (_this->stencil_pix_run) free(_this->stencil_pix_run);
	SAFEALLOC(_this->stencil_pix_run , sizeof(u32) * (tx->width+2));

	_this->width = tx->width;
	_this->height = tx->height;
	_this->pixels = tx->pixels;
	_this->pixelFormat = tx->pixel_format;
	_this->BPP = BPP;
	m4_surface_set_matrix(_this, NULL);
	return M4OK;
}


M4Err m4_surface_clear(LPM4SURFACE surf, M4IRect *rc, u32 color)
{
	M4IRect clear;

	M4Surface *_this = (M4Surface *)surf;
	if (!_this) return M4BadParam;

	if (rc) {	
		s32 _x, _y;
		if (_this->center_coords) {
			_x = rc->x + _this->width / 2;
			_y = _this->height / 2 - rc->y;
		} else {
			_x = rc->x;
			_y = rc->y - rc->height;
		}

		clear.width = (u32) rc->width;
		if (_x>=0) {
			clear.x = (u32) _x;
		} else {
			if ( (s32) clear.width + _x < 0) return M4BadParam;
			clear.width += _x;
			clear.x = 0;
		}
		clear.height = (u32) rc->height;
		if (_y>=0) {
			clear.y = _y;
		} else {
			if ( (s32) clear.height + _y < 0) return M4BadParam;
			clear.height += _y;
			clear.y = 0;
		}
	} else {
		clear.x = clear.y = 0;
		clear.width = _this->width;
		clear.height = _this->height;
	}
	
	switch (_this->pixelFormat) {
	case M4PF_ARGB:
	case M4PF_RGB_32:
		return m4_surface_clear_argb(surf, clear, color);
	case M4PF_BGR_24:
		return m4_surface_clear_rgb(surf, clear, color);
	case M4PF_RGB_24:
		return m4_surface_clear_bgr(surf, clear, color);
	case M4PF_RGB_565:
		return m4_surface_clear_565(surf, clear, color);
	case M4PF_RGB_555:
		return m4_surface_clear_555(surf, clear, color);
	default:
		return M4BadParam;
	}
}

void m4_detach_surface(LPM4SURFACE surf)
{
}

M4Err m4_surface_set_raster_level(LPM4SURFACE surf , enum M4RasterLevel RasterSetting)
{
	M4Surface *_this = (M4Surface *) surf;
	if (!_this) return M4BadParam;
	switch (RasterSetting) {
	case M4RasterHighQuality:
		_this->AALevel = 1;/*don't draw pixels with 0 alpha...*/
		_this->texture_filter = M4TextureFilterHighQuality;
		break;
	case M4RasterInter:
		_this->AALevel = 90;
		_this->texture_filter = M4TextureFilterHighQuality;
		break;
	case M4RasterHighSpeed:
	default:
		_this->AALevel = 180;
		_this->texture_filter = M4TextureFilterHighSpeed;
		break;
	}
	return M4OK;
}


M4Err m4_surface_set_clipper(LPM4SURFACE surf, M4IRect *rc)
{
	M4Surface *_this = (M4Surface *)surf;
	if (!_this) return M4BadParam;
	if (rc) {
		_this->clipper = *rc;
		_this->useClipper = 1;
		/*clipper was given in BIFS like coords, we work with bottom-min for rect, (0,0) top-left of surface*/
		if (_this->center_coords) {
			_this->clipper.x += _this->width / 2;
			_this->clipper.y = _this->height / 2 - rc->y;
		} else {
			_this->clipper.y -= rc->height;
		}

		if (_this->clipper.x <=0) {
			if (_this->clipper.x + (s32) _this->clipper.width < 0) return M4BadParam;
			_this->clipper.width += _this->clipper.x;
			_this->clipper.x = 0;
		}
		if (_this->clipper.y <=0) {
			if (_this->clipper.y + (s32) _this->clipper.height < 0) return M4BadParam;
			_this->clipper.height += _this->clipper.y;
			_this->clipper.y = 0;
		}
	} else {
		_this->useClipper = 0;
	}
	return M4OK;
}


static M4Err m4_resizecontours(M4Surface *surf, u32 num)
{
	if (surf->contourlen < num) {
		if (surf->contours != NULL) free(surf->contours);
		surf->contours = malloc(sizeof (s16) * num);
		if (surf->contours == NULL) {
			surf->contourlen = 0;
			return M4OutOfMem;
		}
		surf->contourlen = num;
	}
	return M4OK;
}

static M4Err m4_resizepoints(M4Surface *surf, u32 num)
{
	if (surf->pointlen < num) {
		if (surf->points != NULL) free(surf->points);
		surf->points = NULL;

		if (surf->tags != NULL) free(surf->tags);
		surf->tags = NULL;

		surf->points = malloc(sizeof (FT_Vector) * num);
		if (surf->points == NULL) {
			surf->pointlen = 0;
			return M4OutOfMem;
		}
		surf->tags = malloc(sizeof(s8) * num);
		if (surf->tags == NULL) {
			surf->pointlen = 0;
			return M4OutOfMem;
		}
		surf->pointlen = num;
	}
	return M4OK;
}


static Bool setup_ft_callbacks(M4Surface *surf)
{
	u32 col, a;
	Bool use_const = 1;

	if (surf->sten->type == M4StencilSolid) {
		col = surf->fill_col = ((M4Brush *)surf->sten)->color;
		a = M4C_A(surf->fill_col);
	} else {
		col = a = 0;
		use_const = 0;
	}

	switch (surf->pixelFormat) {
	case M4PF_ARGB:
		if (use_const) {
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_argb_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_argb_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_argb_fill_var;
		}
		break;

	case M4PF_RGB_32:
		if (use_const) {
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb32_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb32_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb32_fill_var;
		}
		break;
	case M4PF_RGB_24:
		if (use_const) {
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_rgb_fill_var;
		}
		break;
	case M4PF_BGR_24:
		if (use_const) {
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_bgr_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_bgr_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_bgr_fill_var;
		}
		break;
	case M4PF_RGB_565:
		if (use_const) {
			surf->fill_565 = M4COL_565(col);
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_565_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_565_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_565_fill_var;
		}
		break;
	case M4PF_RGB_555:
		if (use_const) {
			surf->fill_555 = M4COL_555(col);
			if (!a) return 0;
			if (a!=0xFF) {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_555_fill_const_a;
			} else {
				surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_555_fill_const;
			}
		} else {
			surf->ftparams.gray_spans = (FT_Raster_Span_Func) m4_555_fill_var;
		}
		break;
	}
	return 1;
}




M4Err m4_surface_set_path(LPM4SURFACE surf, LPM4PATH _path)
{
	u32 n, i, j, cnt;
	M4Err e;
	M4Matrix2D mat;
	M4Surface *_this = (M4Surface *)surf;
	if (!surf) return M4BadParam;
	if (!_path) {
		_this->ftoutline.n_points = 0;
		_this->ftoutline.n_contours = 0;
		return M4OK;
	}

	n=0;
	cnt = m4_path_get_subpath_count(_path);
	for (i=0; i<cnt; i++) 
		n += m4_subpath_get_points_count(_path, i);

	/* freetype can't handle bigger paths */
	if (n > 32767) return M4OutOfMem;

	_this->ftoutline.n_points = n;
	_this->ftoutline.n_contours = cnt;

	e = m4_resizepoints(_this, n);
	if (e) return e;
	e = m4_resizecontours(_this, cnt);
	if (e) return e;

	_this->ftoutline.points = _this->points;
	_this->ftoutline.tags = _this->tags;
	_this->ftoutline.contours = _this->contours;

	n = 0;
	/*convert surface mtrix to final FT coord system*/
	mx2d_copy(mat, _this->mat);
	mx2d_add_scale(&mat, 64, 64);

	for (i=0; i<cnt; i++) {
		M4Point2D *pts = m4_subpath_get_points(_path, i);
		u32 len = m4_subpath_get_points_count(_path, i);

		for (j=0; j<len; j++) {
			Float x, y;

			/* transform */
			x = pts[j].x;
			y = pts[j].y;
			/* convert to fixed-point and save */
			_this->points[n].x = (s32) (x * mat.m[0] + y * mat.m[1] + mat.m[2]);
			_this->points[n].y = (s32) (x * mat.m[3] + y * mat.m[4] + mat.m[5]);
			_this->tags[n] = 1;
			n ++;
		}
		_this->contours[i] = n - 1;
	}

	if (m4_path_get_fill_mode(_path) == M4PathFillOddEven) 
		_this->ftoutline.flags = ft_outline_even_odd_fill;
	else
		_this->ftoutline.flags = 0;

	_this->ftparams.flags = ft_raster_flag_direct | ft_raster_flag_clip | ft_raster_flag_aa;
	m4_path_get_bounds(_path, &_this->path_bounds);
	/*invert Y (ft uses min Y)*/
	_this->path_bounds.y -= _this->path_bounds.height;
	return M4OK;
}


M4Err m4_surface_fill(LPM4SURFACE surf, LPM4STENCIL stencil)
{
	M4Rect rc;
	M4Matrix2D mat, st_mat;
	Bool restore_filter;
	M4Stencil *sten = (M4Stencil *)stencil;
	M4Surface *_this = (M4Surface *)surf;

	if (!surf || !stencil) return M4BadParam;
	if (!_this->ftoutline.n_points) return M4OK;

	_this->sten = sten;

	/*setup ft raster calllbacks*/
	if (!setup_ft_callbacks(surf)) return M4OK;

	get_surface_world_matrix(_this, &mat);

	restore_filter = 0;
	/*get path frame for texture convertion */
	if (sten->type != M4StencilSolid) {
		rc = _this->path_bounds;
		mx2d_apply_rect(&mat, &rc);
		rc.x = rc.y = 0;
		m4_set_stencil_path_frame(sten, rc);
		m4_set_stencil_path_matrix(sten, &_this->mat);

		m4_get_stencil_matrix(sten, &st_mat);
		mx2d_init(sten->smat);
		switch (sten->type) {
		case M4StencilTexture:
			if (! ((M4Texture *)sten)->pixels) return M4BadParam;
			if (_this->center_coords) mx2d_add_scale(&sten->smat, 1, -1);
			m4_set_texture_active(sten);
			mx2d_add_matrix(&sten->smat, &st_mat);
			mx2d_add_matrix(&sten->smat, &mat);
			mx2d_inverse(&sten->smat);
			bmp_init(sten);
			if (((M4Texture *)sten)->filter == M4_TEXTURE_FILTER_DEFAULT) {
				restore_filter = 1;
				((M4Texture *)sten)->filter = _this->texture_filter;
			}

			break;
		case M4StencilLinearGradient:
		{
			M4LinearGradient *lin = (M4LinearGradient *)sten;
			mx2d_add_matrix(&sten->smat, &st_mat);
			mx2d_add_matrix(&sten->smat, &mat);
			mx2d_inverse(&sten->smat);
			/*and finalize matrix in gradient coord system*/
			mx2d_add_matrix(&sten->smat, &lin->vecmat);
			mx2d_add_scale(&sten->smat, (1<<M4GRADIENTSCALEBITS), (1<<M4GRADIENTSCALEBITS));

		}
			break;
		case M4StencilRadialGradient:
		{
			M4RadialGradient *rad = (M4RadialGradient*)sten;
			mx2d_copy(sten->smat, st_mat);
			mx2d_add_matrix(&sten->smat, &mat);
			mx2d_inverse(&sten->smat);
			mx2d_add_translation(&sten->smat, -rad->center.x, -rad->center.y);
			mx2d_add_scale(&sten->smat, 1 / rad->radius.x, 1 / rad->radius.y);


			rad->d_f.x = (rad->focus.x - rad->center.x) / rad->radius.x;
			rad->d_f.y = (rad->focus.y - rad->center.y) / rad->radius.y;
			/*init*/
			m4_radial_init(rad);
		}
			break;
		}
	}

	if (_this->useClipper) {
		_this->ftparams.clip_box.xMin = _this->clipper.x;
		_this->ftparams.clip_box.yMin = _this->clipper.y;
		_this->ftparams.clip_box.xMax = (_this->clipper.x + _this->clipper.width);
		_this->ftparams.clip_box.yMax = (_this->clipper.y + _this->clipper.height);
	} else {
		_this->ftparams.clip_box.xMin = 0;
		_this->ftparams.clip_box.yMin = 0;
		_this->ftparams.clip_box.xMax = (_this->width);
		_this->ftparams.clip_box.yMax = (_this->height);
	}

	/*and call the raster*/
	my_ft_grays_raster.raster_render(_this->ftraster, &_this->ftparams);

	/*restore stencil matrix*/
	if (sten->type != M4StencilSolid) {
		mx2d_copy(sten->smat, st_mat);
		if (restore_filter) ((M4Texture *)sten)->filter = M4_TEXTURE_FILTER_DEFAULT;
	}
	_this->sten = 0L;
	return M4OK;
}



M4Err m4_surface_flush(LPM4SURFACE _this)
{
	/*nothing*/
	return M4OK;
}


