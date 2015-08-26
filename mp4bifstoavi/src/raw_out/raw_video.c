/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / DirectX audio and video render plugin
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


#if defined(RAW_OUT_STATIC_BUILD)
#include "raw_video.h"
#endif /* defined(RAW_OUT_STATIC_BUILD) */
#include "raw_out.h"

#define RAW_OUT_PIXEL_FORMAT		M4PF_RGB_24

#define RAWCTX	RawContext *rc = (RawContext *)dr->opaque

static void raw_resize(VideoOutput *dr, u32 w, u32 h)
{
	RAWCTX;

	if (rc->pixels) free(rc->pixels);
	rc->width = w;
	rc->height = h;
	rc->pixels = malloc(sizeof(char) * 3 * w * h);
}

static RawSurface *raw_get_surface(VideoOutput *dr, u32 id)
{
	u32 i;
	RawSurface *s;
	RAWCTX;
	for (i=0; i<ChainGetCount(rc->surfaces); i++) {
		s = ChainGetEntry(rc->surfaces, i);
		if (s->id == id) return s;
	}
	return NULL;
}

M4Err RAW_SetupHardware(VideoOutput *dr, void *os_handle, Bool no_proc_override, M4GLConfig *cfg)
{
	if (cfg) return M4NotSupported;
	raw_resize(dr, 100, 100);
	return M4OK;
}


static void RAW_Shutdown(VideoOutput *dr)
{
	RAWCTX;

	while (ChainGetCount(rc->surfaces)) {
		RawSurface *s = ChainGetEntry(rc->surfaces, 0);
		ChainDeleteEntry(rc->surfaces, 0);
		if (s->pixels) free(s->pixels);
		free(s);
	}
	if (rc->pixels) free(rc->pixels);
	rc->pixels = NULL;
}


static M4Err RAW_SetFullScreen(VideoOutput *dr, Bool bOn, u32 *outWidth, u32 *outHeight)
{
	return M4NotSupported;
}

static M4Err RAW_FlushVideo(VideoOutput *dr, M4Window *dest)
{
	return M4OK;
}

static M4Err RAW_LockSurface(VideoOutput *dr, u32 surface_id, M4VideoSurface *vi)
{
	RawSurface *s;
	RAWCTX;

	if (!surface_id) {
		vi->height = rc->height;
		vi->width = rc->width;
		vi->video_buffer = rc->pixels;
	} else {
		s = raw_get_surface(dr, surface_id);
		if (!s) return M4BadParam;
		vi->height = s->height;
		vi->width = s->width;
		vi->video_buffer = s->pixels;
	}
	vi->pitch = 3 * vi->width;
	vi->os_handle = NULL;
	vi->pixel_format = RAW_OUT_PIXEL_FORMAT;
	return M4OK;
}

static M4Err RAW_UnlockSurface(VideoOutput *dr, u32 surface_id)
{
	return M4OK;
}

static M4Err RAW_ClearSurface(VideoOutput *dr, u32 surface_id, M4Window *src, u32 color)
{
	M4Err e;
	u32 i, j, endx, endy;
	u8 r, g, b;
	M4VideoSurface s;

	r = (color>>16) & 0xFF;
	g = (color>>8) & 0xFF;
	b = (color) & 0xFF;

	e = RAW_LockSurface(dr, surface_id, &s);
	if (e) return e;

	i = j = 0;
	endx = s.width;
	endy = s.height;
	if (src) {
		j = src->y;
		i = src->x;
		endx = i + src->w;
		endy = j + src->h;
	}
	if (endx > s.width) endx = s.width;
	if (endy > s.height) endy = s.height;

	for (; j<endy; j++) {
		for (; i<endy; i+=3) {
			s.video_buffer[j * s.pitch + i] = r;
			s.video_buffer[j * s.pitch + i + 1] = g;
			s.video_buffer[j * s.pitch + i + 2] = b;
		}
	}
	return M4OK;
}

static M4Err RAW_Clear(VideoOutput *dr, u32 color)
{
	return RAW_ClearSurface(dr, 0, NULL, color);
}

static void *RAW_GetContext(VideoOutput *dr, u32 surface_id)
{
	return NULL;
}

static void RAW_ReleaseContext(VideoOutput *dr, u32 surface_id, void *context)
{
}


static M4Err RAW_GetPixelFormat(VideoOutput *dr, u32 surfaceID, u32 *pixel_format)
{
	*pixel_format = RAW_OUT_PIXEL_FORMAT;
	return M4OK;
}

static M4Err RAW_Blit(VideoOutput *dr, u32 src_id, u32 dst_id, M4Window *src, M4Window *dst)
{
	return M4NotSupported;
}

static M4Err RAW_CreateSurface(VideoOutput *dr, u32 width, u32 height, u32 pixel_format, u32 *surfaceID)
{
	RawSurface *s;
	RAWCTX;

	s = malloc(sizeof(RawSurface));
	s->height = height;
	s->width = width;
	s->id = (u32) s;
	s->pixels = malloc(sizeof(char) * 3 * width * height);
	ChainAddEntry(rc->surfaces, s);
	*surfaceID = s->id;
	return M4OK;
}


/*deletes video surface by id*/
static M4Err RAW_DeleteSurface(VideoOutput *dr, u32 surface_id)
{
	u32 i;
	RawSurface *s;
	RAWCTX;

	if (!surface_id) return M4BadParam;

	for (i=0; i<ChainGetCount(rc->surfaces); i++) {
		s = ChainGetEntry(rc->surfaces, i);
		if (s->id == surface_id) {
			ChainDeleteEntry(rc->surfaces, i);
			if (s->pixels) free(s->pixels);
			free(s);
			return M4OK;
		}
	}
	return M4OK;
}

Bool RAW_IsSurfaceValid(VideoOutput *dr, u32 surface_id)
{
	if (surface_id) return (raw_get_surface(dr, surface_id)==NULL) ? 0 : 1;
	return 1;
}


static M4Err RAW_ResizeSurface(VideoOutput *dr, u32 surface_id, u32 width, u32 height)
{
	RawSurface *s;

	if (!surface_id) {
		raw_resize(dr, width, height);
		return M4OK;
	}
	s = raw_get_surface(dr, surface_id);
	if (!s) return M4BadParam;

	if (s->pixels) free(s->pixels);
	s->width = width;
	s->height = height;
	s->pixels = malloc(sizeof(char) * 3 * width * height);
	return M4OK;
}


static M4Err RAW_Resize(VideoOutput *dr, u32 width, u32 height)
{
	return RAW_ResizeSurface(dr, 0, width, height);
}

static M4Err RAW_PushEvent(VideoOutput *dr, M4Event *evt)
{
	if (evt->type == M4E_WINDOWSIZE) {
		return RAW_ResizeSurface(dr, 0, evt->size.width, evt->size.height);
	}
	return M4OK;
}

static void *NewVideoOutput()
{
	RawContext *pCtx;
	VideoOutput *driv = (VideoOutput *) malloc(sizeof(VideoOutput));
	memset(driv, 0, sizeof(VideoOutput));
	M4_REG_PLUG(driv, M4_VIDEO_OUTPUT_INTERFACE, "Raw Video Output", "gpac distribution", 0)

	pCtx = malloc(sizeof(RawContext));
	memset(pCtx, 0, sizeof(RawContext));
	pCtx->surfaces = NewChain();
	
	driv->opaque = pCtx;
	/*alpha and keying to do*/
	driv->bHasAlpha = 0;
	driv->bHasKeying = 0;
	driv->bHasYUV = 0;

	driv->Blit = RAW_Blit;
	driv->Clear = RAW_Clear;
	driv->CreateSurface = RAW_CreateSurface;
	driv->DeleteSurface = RAW_DeleteSurface;
	driv->FlushVideo = RAW_FlushVideo;
	driv->GetContext = RAW_GetContext;
	driv->GetPixelFormat = RAW_GetPixelFormat;
	driv->LockSurface = RAW_LockSurface;
	driv->ReleaseContext = RAW_ReleaseContext;
	driv->IsSurfaceValid = RAW_IsSurfaceValid;
	driv->Resize = RAW_Resize;
	driv->SetFullScreen = RAW_SetFullScreen;
	driv->SetupHardware = RAW_SetupHardware;
	driv->Shutdown = RAW_Shutdown;
	driv->UnlockSurface = RAW_UnlockSurface;
	driv->ResizeSurface	= RAW_ResizeSurface;

	driv->PushEvent = RAW_PushEvent;
	return (void *)driv;
}

static void DeleteVideoOutput(void *ifce)
{
	RawContext *rc;
	VideoOutput *driv = (VideoOutput *) ifce;

	RAW_Shutdown(driv);
	rc = (RawContext *)driv->opaque;
	DeleteChain(rc->surfaces);
	free(rc);
	free(driv);
}

#if !defined(RAW_OUT_STATIC_BUILD)

/*interface query*/
Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return 1;
	return 0;
}
/*interface create*/
void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return NewVideoOutput();
	return NULL;
}
/*interface destroy*/
void ShutdownInterface(void *ifce)
{
	VideoOutput *dd = (VideoOutput *)ifce;
	switch (dd->InterfaceType) {
	case M4_VIDEO_OUTPUT_INTERFACE:
		DeleteVideoOutput(dd);
		break;
	}
}
#else  /* !defined(RAW_OUT_STATIC_BUILD) */
/*interface query*/
Bool RO_QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return 1;
	return 0;
}
/*interface create*/
void *RO_LoadInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return NewVideoOutput();
	return NULL;
}
/*interface destroy*/
void RO_ShutdownInterface(void *ifce)
{
	VideoOutput *dd = (VideoOutput *)ifce;
	switch (dd->InterfaceType) {
	case M4_VIDEO_OUTPUT_INTERFACE:
		DeleteVideoOutput(dd);
		break;
	}
}
#endif /* defined(RAW_OUT_STATIC_BUILD) */
