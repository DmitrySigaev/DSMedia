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

struct _graphics
{
	GpGraphics *graph;
	GpMatrix *mat;
	GpPath *clip;
	GpPath *current;
	u32 w, h;
	Bool center_coords;

	/*offscreen buffer handling*/
	GpBitmap *pBitmap;
};

GpPath *gdip_create_path(LPM4PATH _this)
{
	GpPath *p;
	u32 fmode;
	u32 j, i, count, nb_pts;
	M4Point2D *pts;
	if (!_this) return NULL;
	count = m4_path_get_subpath_count(_this);
	if (!count) return NULL;
	GdipCreatePath(FillModeAlternate, &p);
	
	fmode = m4_path_get_fill_mode(_this);
	GdipSetPathFillMode(p, (fmode==M4PathFillZeroNonZero) ? FillModeWinding : FillModeAlternate);

	for (i=0; i<count; i++) {
		nb_pts = m4_subpath_get_points_count(_this, i);
		pts = m4_subpath_get_points(_this, i);
		GdipStartPathFigure(p);
		for (j=1; j<nb_pts; j++) {
			GdipAddPathLine(p, pts[j-1].x, pts[j-1].y, pts[j].x, pts[j].y);
		}
		GdipClosePathFigure(p);
	}
	return p;
}

#define GPGRAPH() struct _graphics *_graph = (struct _graphics *)_this;

static
LPM4SURFACE gp_new_surface(Graphics2DDriver *, Bool center_coords)
{
	struct _graphics *graph;
	GP_SAFEALLOC(graph, struct _graphics);
	graph->center_coords = center_coords;
	return graph;
}

static
void gp_delete_surface(LPM4SURFACE _this)
{
	GPGRAPH();
	free(_graph);
}

/*should give the best results with the clippers*/
#define GDIP_PIXEL_MODE PixelOffsetModeHighQuality

static
M4Err gp_attach_surface_to_device(LPM4SURFACE _this, void *os_handle, u32 width, u32 height)
{
	GpMatrix *mat;
	HDC handle = (HDC) os_handle;
	GPGRAPH();
	if (!_graph || !handle) return M4BadParam;
	if (_graph->graph) return M4BadParam;
	GdipCreateFromHDC(handle, &_graph->graph);

	GdipCreateMatrix(&mat);
	if (	_graph->center_coords) {
		GdipScaleMatrix(mat, 1.0, -1.0, MatrixOrderAppend);
		GdipTranslateMatrix(mat, (Float) width/2, (Float) height/2, MatrixOrderAppend);
	}
	GdipSetWorldTransform(_graph->graph, mat);
	GdipDeleteMatrix(mat);
	_graph->w = width;
	_graph->h = height;
	GdipSetPixelOffsetMode(_graph->graph, GDIP_PIXEL_MODE);
	return M4OK;
}
static
M4Err gp_attach_surface_to_texture(LPM4SURFACE _this, LPM4STENCIL sten)
{
	GpMatrix *mat;
	struct _stencil *_sten = (struct _stencil *)sten;
	GPGRAPH();
	if (!_graph || !_sten || !_sten->pBitmap) return M4BadParam;

	GdipGetImageGraphicsContext(_sten->pBitmap, &_graph->graph);

	GdipCreateMatrix(&mat);
	GdipScaleMatrix(mat, 1.0, -1.0, MatrixOrderAppend);
	GdipTranslateMatrix(mat, (Float) _sten->width/2, (Float) _sten->height/2, MatrixOrderAppend);
	GdipSetWorldTransform(_graph->graph, mat);
	GdipDeleteMatrix(mat);
	_graph->w = _sten->width;
	_graph->h = _sten->height;
	GdipSetPixelOffsetMode(_graph->graph, GDIP_PIXEL_MODE);
	return M4OK;
}
static
M4Err gp_attach_surface_to_buffer(LPM4SURFACE _this, unsigned char *pixels, u32 width, u32 height, u32 stride, enum M4PixelFormats pixelFormat)
{
	GpMatrix *mat;
	u32 pFormat;
	GPGRAPH();

	if (stride%4) return M4NotSupported;

	switch (pixelFormat) {
	case M4PF_ALPHAGREY:
		pFormat = PixelFormat16bppGrayScale;
		break;
	case M4PF_RGB_555:
		pFormat = PixelFormat16bppRGB555;
		break;
	case M4PF_RGB_565:
		pFormat = PixelFormat16bppRGB565;
		break;
	case M4PF_RGB_24:
		pFormat = PixelFormat24bppRGB;
		break;
	case M4PF_RGB_32:
		pFormat = PixelFormat32bppRGB;
		break;
	case M4PF_ARGB:
		pFormat = PixelFormat32bppARGB;
		break;
	default:
		return M4NotSupported;
	}
	GdipCreateBitmapFromScan0(width, height, stride, pFormat, pixels, &_graph->pBitmap);
	GdipGetImageGraphicsContext(_graph->pBitmap, &_graph->graph);
	
	GdipCreateMatrix(&mat);
	GdipScaleMatrix(mat, 1.0, -1.0, MatrixOrderAppend);
	GdipTranslateMatrix(mat, (Float) width/2, (Float) height/2, MatrixOrderAppend);
	GdipSetWorldTransform(_graph->graph, mat);
	GdipDeleteMatrix(mat);
	GdipSetPixelOffsetMode(_graph->graph, GDIP_PIXEL_MODE);
	return M4OK;
}

static
void gp_detach_surface(LPM4SURFACE _this)
{
	GPGRAPH();
	if (_graph->graph) GdipDeleteGraphics(_graph->graph);
	_graph->graph = NULL;
	if (_graph->clip) GdipDeletePath(_graph->clip);
	_graph->clip = NULL;
	if (_graph->pBitmap) GdipDisposeImage(_graph->pBitmap);
	_graph->pBitmap = NULL;
	if (_graph->current) GdipDeletePath(_graph->current);
	_graph->current = NULL;
}


static
M4Err gp_surface_set_raster_level(LPM4SURFACE _this, enum M4RasterLevel RasterSetting)
{
	GPGRAPH();
	switch (RasterSetting) {
	case M4RasterHighSpeed:
		GdipSetSmoothingMode(_graph->graph, SmoothingModeHighSpeed);
		GdipSetCompositingQuality(_graph->graph, CompositingQualityHighSpeed);
		break;
	case M4RasterInter:
		GdipSetSmoothingMode(_graph->graph, SmoothingModeDefault);
		GdipSetCompositingQuality(_graph->graph, CompositingQualityDefault);
		break;
	case M4RasterHighQuality:
		GdipSetSmoothingMode(_graph->graph, SmoothingModeHighQuality);
		GdipSetCompositingQuality(_graph->graph, CompositingQualityDefault);
		/*THIS IS HORRIBLY SLOW DON'T EVEN THINK ABOUT IT*/
		/*GdipSetCompositingQuality(_graph->graph, CompositingQualityHighQuality);*/
		break;
	}
	return M4OK;
}
static
M4Err gp_surface_set_matrix(LPM4SURFACE _this, M4Matrix2D * mat)
{
	GPGRAPH();
	if (_graph->mat) GdipDeleteMatrix(_graph->mat);
	
	_graph->mat = mat_m4_to_gdip(mat);
	return M4OK;
}
static
M4Err gp_surface_set_clipper(LPM4SURFACE _this, M4IRect *rc)
{
	GPGRAPH();
	if (_graph->clip) GdipDeletePath(_graph->clip);
	_graph->clip = 0L;
	if (!rc) return M4OK;

	GdipCreatePath(FillModeAlternate, &_graph->clip);
	/*luckily enough this maps well for both flipped and unflipped coords (cf notes on clipper in m4_graphics.h)*/
	GdipAddPathRectangleI(_graph->clip, rc->x, rc->y - rc->height, rc->width, rc->height);
	return M4OK;
}

static
GpBrush *gp_get_brush(struct _stencil *_sten)
{
	if (_sten->pSolid) return _sten->pSolid;
	if (_sten->pLinear) return _sten->pLinear;
	if (_sten->pRadial) return _sten->pRadial;
	if (_sten->pTexture) return _sten->pTexture;
	return NULL;
}

static GpPath *gp_setup_path(struct _graphics *_this, LPM4PATH path)
{
	GpPath *tr = gdip_create_path(path);
	/*append current matrix*/
	if (_this->mat) GdipTransformPath(tr, _this->mat);
	return tr;
}

static
M4Err gp_surface_set_path(LPM4SURFACE _this, LPM4PATH path)
{
	struct _storepath *_path;
	GPGRAPH();
	if (!_graph) return M4BadParam;
	if (_graph->current) GdipDeletePath(_graph->current);
	_graph->current = NULL;
	if (!path) return M4OK;
	
	_path = (struct _storepath *)path;
	_graph->current = gp_setup_path(_graph, path);
	return M4OK;
}

//#define NODRAW

static
M4Err gp_surface_fill(LPM4SURFACE _this, LPM4STENCIL stencil)
{
	GpStatus ret;
	GpMatrix *newmat;
	struct _stencil *_sten;
	GPGRAPH();
	if (!_this) return M4BadParam;
	if (!_graph->current) return M4OK;
	_sten = (struct _stencil *)stencil; assert(_sten);

#ifdef NODRAW
	return M4OK;
#endif


	if (_graph->clip) GdipSetClipPath(_graph->graph, _graph->clip, CombineModeReplace);

	switch (_sten->type) {
	case M4StencilSolid:
		assert(_sten->pSolid);
		GdipFillPath(_graph->graph, _sten->pSolid, _graph->current);
		break;
	case M4StencilLinearGradient:
		if (_sten->pMat) {
			/*rebuild gradient*/
			gp_recompute_line_gradient(_sten);

			GdipResetTextureTransform((GpTexture*)_sten->pLinear);
			if (_sten->pMat) {
				GdipCloneMatrix(_sten->pMat, &newmat);
			} else {
				GdipCreateMatrix(&newmat);
			}
			GdipMultiplyMatrix(newmat, _sten->pLinearMat, MatrixOrderPrepend);
			GdipSetTextureTransform((GpTexture*)_sten->pLinear, newmat);
			GdipDeleteMatrix(newmat);
		}
		GdipFillPath(_graph->graph, _sten->pLinear, _graph->current);
		break;
	case M4StencilRadialGradient:
		/*build gradient*/
		gp_recompute_radial_gradient(_sten);

		GdipSetCompositingQuality(_graph->graph, CompositingQualityHighSpeed);
		GdipSetInterpolationMode(_graph->graph, InterpolationModeLowQuality);
		GdipSetSmoothingMode(_graph->graph, SmoothingModeHighSpeed);
		
		/*check if we need to draw solid background (GDIplus doesn't implement padded mode on path gradients)*/
		if (_sten->pSolid) {
			GpPath *tr;
			GdipClonePath(_sten->circle, &tr);
			GdipTransformPath(tr, _sten->pMat);
			GdipSetClipPath(_graph->graph, tr, CombineModeExclude);
			GdipFillPath(_graph->graph, _sten->pSolid, _graph->current);
			GdipDeletePath(tr);
			GdipResetClip(_graph->graph);
			if (_graph->clip) GdipSetClipPath(_graph->graph, _graph->clip, CombineModeReplace);
		}
		GdipFillPath(_graph->graph, _sten->pRadial, _graph->current);
		break;
	case M4StencilVertexGradient:
		assert(_sten->pRadial);
		if (_sten->pMat) GdipSetTextureTransform((GpTexture*)_sten->pRadial, _sten->pMat);
		ret = GdipFillPath(_graph->graph, _sten->pRadial, _graph->current);
		break;
	case M4StencilTexture:
		gp_load_texture(_sten);
		if (_sten->pTexture) {
			GpMatrix *newmat;
			GdipResetTextureTransform((GpTexture*)_sten->pTexture);
			if (_sten->pMat) {
				GdipCloneMatrix(_sten->pMat, &newmat);
			} else {
				GdipCreateMatrix(&newmat);
			}
			/*gdip flip*/
			if (_graph->center_coords) GdipScaleMatrix(newmat, 1, -1, MatrixOrderPrepend);	
			GdipSetTextureTransform((GpTexture*)_sten->pTexture, newmat);
			GdipDeleteMatrix(newmat);

			GdipSetInterpolationMode(_graph->graph, (_sten->tFilter==M4TextureFilterHighQuality) ? InterpolationModeHighQuality : InterpolationModeLowQuality);

			GdipFillPath(_graph->graph, _sten->pTexture, _graph->current);
		}
		break;
	}
	return M4OK;
}


static
M4Err gp_surface_flush(LPM4SURFACE _this)
{
	GPGRAPH();
	GdipFlush(_graph->graph, FlushIntentionSync);
	return M4OK;
}

static
M4Err gp_surface_clear(LPM4SURFACE _this, M4IRect *rc, u32 color)
{
	GpPath *path;
	GPGRAPH();

	GdipCreatePath(FillModeAlternate, &path);
	if (rc) {
		/*luckily enough this maps well for both flipped and unflipped coords (cf notes on clipper in m4_graphics.h)*/
		GdipAddPathRectangleI(path, rc->x, rc->y - rc->height, rc->width, rc->height);
	} else {
		if (_graph->center_coords) {
			GdipAddPathRectangleI(path, -1 * (s32)_graph->w / 2, -1 * (s32)_graph->h / 2, _graph->w, _graph->h);
		} else {
			GdipAddPathRectangleI(path, 0, 0, _graph->w, _graph->h);
		}
	}
	/*we MUST use clear otherwise ARGB surfaces are not cleared correctly*/
	GdipSetClipPath(_graph->graph, path, CombineModeReplace);
	GdipGraphicsClear(_graph->graph, color);
	GdipDeletePath(path);
	return M4OK;
}

void gp_init_driver_surface(Graphics2DDriver *driver)
{
	driver->new_surface = gp_new_surface;
	driver->delete_surface = gp_delete_surface;
	driver->attach_surface_to_device = gp_attach_surface_to_device;
	driver->attach_surface_to_texture = gp_attach_surface_to_texture;
	driver->attach_surface_to_buffer = gp_attach_surface_to_buffer;
	driver->detach_surface = gp_detach_surface;
	driver->surface_set_raster_level = gp_surface_set_raster_level;
	driver->surface_set_matrix = gp_surface_set_matrix;
	driver->surface_set_clipper = gp_surface_set_clipper;
	driver->surface_set_path = gp_surface_set_path;
	driver->surface_fill = gp_surface_fill;
	driver->surface_flush = gp_surface_flush;
	driver->surface_clear = gp_surface_clear;
}


Graphics2DDriver *gdip_LoadRenderer()
{
	GdiplusStartupInput startupInput;
	Graphics2DDriver *driver;
	struct _gp_context *ctx;
	GP_SAFEALLOC(ctx, struct _gp_context);
	GP_SAFEALLOC(driver, Graphics2DDriver);
	GdiplusStartup(&ctx->gdiToken, &startupInput, NULL);
	driver->internal = ctx;
	M4_REG_PLUG(driver, M4_GRAPHICS_2D_INTERFACE, "GDIplus 2D Raster", "gpac distribution", 0)
	gp_init_driver_texture(driver);
	gp_init_driver_surface(driver);
	gp_init_driver_grad(driver);
	return driver;
}

void gdip_ShutdownRenderer(Graphics2DDriver *driver)
{
	struct _gp_context *ctx = (struct _gp_context *)driver->internal;

	GdiplusShutdown(ctx->gdiToken);
	free(driver->internal);
	free(driver);
}

Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_GRAPHICS_2D_INTERFACE) return 1;
	if (InterfaceType == M4_FONT_RASTER_INTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType==M4_GRAPHICS_2D_INTERFACE) {
		return gdip_LoadRenderer();
	}
	else if (InterfaceType==M4_FONT_RASTER_INTERFACE) {
		return gdip_new_font_driver();
	}
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	BaseInterface *dr = (BaseInterface *)ifce;
	if (dr->InterfaceType == M4_GRAPHICS_2D_INTERFACE) {
		gdip_ShutdownRenderer((Graphics2DDriver *)dr);
	}
	else if (dr->InterfaceType == M4_FONT_RASTER_INTERFACE) {
		gdip_delete_font_driver((FontRaster *)dr);
	}
}

