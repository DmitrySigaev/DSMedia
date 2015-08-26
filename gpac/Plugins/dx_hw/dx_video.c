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


#include "dx_hw.h"

#define DDCONTEXT	DDContext *dd = (DDContext *)dr->opaque;


M4Err DD_SetupHardware(VideoOutput *dr, void *os_handle, Bool no_proc_override, M4GLConfig *cfg)
{
	RECT rc;
	DDCONTEXT
	dd->hWnd = (HWND) os_handle;
	
	DD_SetupWindow(dr);
	/*fatal error*/
	if (!dd->hWnd) return M4IOErr;
	
	GetWindowRect(dd->hWnd, &rc);
	dd->is_3D_out = cfg ? 1 : 0;
	return InitDirectDraw(dr, rc.right - rc.left, rc.bottom - rc.top);
}

static void RestoreWindow(DDContext *dd) 
{
	HWND hWnd = dd->hWnd;
	if (!dd->NeedRestore) goto exit;

	if (!dd->owns_hwnd && dd->switch_res) hWnd = GetParent(dd->hWnd);

	dd->NeedRestore = 0;
	if (dd->is_3D_out) {
		ChangeDisplaySettings(NULL,0);
		SetForegroundWindow(GetDesktopWindow());
		SetForegroundWindow(dd->hWnd);
	} else {
#ifdef USE_DX_3
		IDirectDraw_SetCooperativeLevel(dd->pDD, hWnd, DDSCL_NORMAL);
#else
		IDirectDraw7_SetCooperativeLevel(dd->pDD, hWnd, DDSCL_NORMAL);
#endif

		dd->NeedRestore = 0;
	}

	if (dd->owns_hwnd || dd->is_3D_out) SetWindowLong(dd->hWnd, GWL_STYLE, dd->prev_styles);
		
	if (dd->owns_hwnd) SetWindowText(dd->hWnd, "GPAC DirectDraw Output");

	if (!dd->switch_res || dd->owns_hwnd) {
		SetWindowPos(dd->hWnd, NULL, dd->rcWnd.left, dd->rcWnd.top, dd->rcWnd.right - dd->rcWnd.left, dd->rcWnd.bottom - dd->rcWnd.top, SWP_NOZORDER);
	} else {
		/*post a message to the parent wnd*/
		RECT rc = dd->rcWnd;
		HWND hWnd = GetParent(dd->hWnd);
		if (!hWnd) hWnd = dd->hWnd;
		if (dd->is_3D_out) SetWindowPos(dd->hWnd, NULL, dd->rcChildWnd.left, dd->rcChildWnd.top, dd->rcChildWnd.right - dd->rcChildWnd.left, dd->rcChildWnd.bottom - dd->rcChildWnd.top, SWP_NOZORDER);
		SetWindowPos(hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}

exit:
	/*store client rect*/
	GetWindowRect(dd->hWnd, &dd->rcWnd);
}

void DestroyObjects(DDContext *dd)
{
	RestoreWindow(dd);

	/*remove all surfaces*/
	while (ChainGetCount(dd->surfaces)) {
		DDSurface *pS = ChainGetEntry(dd->surfaces, 0);
		ChainDeleteEntry(dd->surfaces, 0);
		SAFE_DD_RELEASE(pS->pSurface);
		free(pS);
	}

	SAFE_DD_RELEASE(dd->pPrimary);
	SAFE_DD_RELEASE(dd->pBack);
	SAFE_DD_RELEASE(dd->pDD);
	dd->ddraw_init = 0;

	/*delete openGL context*/
	if (dd->gl_HRC) {
		wglDeleteContext(dd->gl_HRC);
		dd->gl_HRC = NULL;
	}
	if (dd->gl_HDC) {
		ReleaseDC(dd->hWnd, dd->gl_HDC);
		dd->gl_HDC = NULL;
	}
}

static void DD_Shutdown(VideoOutput *dr)
{
	DDCONTEXT
	DestroyObjects(dd);

	DD_ShutdownWindow(dr);
}

M4Err DD_SetupOpenGL(VideoOutput *dr) 
{
    PIXELFORMATDESCRIPTOR pfd; 
	M4Event evt;
    s32 pixelformat; 
	DDCONTEXT

	/*already setup*/
	if (dd->gl_HRC) return M4OK;

	dd->gl_HDC = GetDC(dd->hWnd);
	if (!dd->gl_HDC) return M4IOErr;

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.dwLayerMask = PFD_MAIN_PLANE;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = pfd.cDepthBits = 16;
    if ( (pixelformat = ChoosePixelFormat(dd->gl_HDC, &pfd)) == FALSE ) return M4IOErr; 
    if (SetPixelFormat(dd->gl_HDC, pixelformat, &pfd) == FALSE) return M4IOErr; 
	dd->gl_HRC = wglCreateContext(dd->gl_HDC);
	if (!dd->gl_HRC) return M4IOErr;
	if (!wglMakeCurrent(dd->gl_HDC, dd->gl_HRC)) return M4IOErr;
	evt.type = M4E_GL_CHANGED;
	dr->on_event(dr->evt_cbk_hdl, &evt);	
	return M4OK;
}

static M4Err DD_SetFullScreen(VideoOutput *dr, Bool bOn, u32 *outWidth, u32 *outHeight)
{
	M4Err e;
	char *sOpt;
	u32 MaxWidth, MaxHeight;
	DDCONTEXT;

	if (!dd->width ||!dd->height) return M4BadParam;
	if (bOn == dd->fullscreen) return M4OK;
	dd->fullscreen = bOn;

	
	/*whenever changing card display mode relocate fastest YUV format for blit (since it depends
	on the dest pixel format)*/
	dd->yuv_init = 0;
	if (dd->fullscreen) {
		char *sOpt = PMI_GetOpt(dr, "Video", "SwitchResolution");
		if (sOpt && !stricmp(sOpt, "yes")) dd->switch_res = 1;
		/*get current or best fitting mode*/
		if (GetDisplayMode(dd) != M4OK) return M4IOErr;
	}

	MaxWidth = MaxHeight = 0;
	sOpt = PMI_GetOpt(dr, "Video", "MaxResolution");
	if (sOpt) sscanf(sOpt, "%dx%d", &MaxWidth, &MaxHeight);

	dd->is_resizing = 1;

	if (dd->is_3D_out) {
		DEVMODE settings;
		e = M4OK;

		/*recreate the GL context whenever changing display settings, it's safer...*/
		DestroyObjects(dd);
		/*Setup FS*/
		if (dd->fullscreen) {
			/*change display mode*/
			if (dd->switch_res) {
				/*when switching res weird messages are sent to parent -> store current rect and post
				a size/pos message on restore */
				if (!dd->owns_hwnd) {
					HWND hWnd = GetParent(dd->hWnd);
					if (!hWnd) hWnd = dd->hWnd;
					GetWindowRect(hWnd, &dd->rcWnd);
				}
			}

			if ((MaxWidth && (dd->fs_width >= MaxWidth)) || (MaxHeight && (dd->fs_height >= MaxHeight)) ) {
				dd->fs_width = MaxWidth;
				dd->fs_height = MaxHeight;
			}

			/*force size change (we do it whether we own or not the window)*/
			dd->prev_styles = GetWindowLong(dd->hWnd, GWL_STYLE);
			GetWindowRect(dd->hWnd, &dd->rcChildWnd);
			SetWindowLong(dd->hWnd, GWL_STYLE, WS_POPUP);
			SetWindowPos(dd->hWnd, NULL, 0, 0, dd->fs_width, dd->fs_height, SWP_NOZORDER | SWP_SHOWWINDOW);
			SetForegroundWindow(dd->hWnd);

			memset(&settings, 0, sizeof(DEVMODE));
			settings.dmSize = sizeof(DEVMODE);
			settings.dmPelsWidth = dd->fs_width;
			settings.dmPelsHeight = dd->fs_height;
			settings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
			if ( ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL ) {
				fprintf(stdout, "cannot change display settings...\n");
				e = M4IOErr;
			} 

			dd->NeedRestore = 1;
			dd->fs_store_width = dd->fs_width;
			dd->fs_store_height = dd->fs_height;
		}
		if (!e) e = DD_SetupOpenGL(dr);
		
	} else {
		e = InitDirectDraw(dr, dd->width, dd->height);
	}
	
	dd->is_resizing = 0;

	if (bOn) {
		dd->store_width = *outWidth;
		dd->store_height = *outHeight;
		*outWidth = dd->fs_width;
		*outHeight = dd->fs_height;
	} else {
		*outWidth = dd->store_width;
		*outHeight = dd->store_height;
	}
	return e;
}


static M4Err DD_Resize(VideoOutput *dr, u32 width, u32 height)
{
	DDCONTEXT;

	if (dd->is_3D_out) {
		dd->width = width;
		dd->height = height;
		return DD_SetupOpenGL(dr);
	}
	if (!dd->ddraw_init) 
		return InitDirectDraw(dr, width, height);
	else
		return CreateBackBuffer(dr, width, height);
}


static M4Err DD_FlushVideo(VideoOutput *dr, M4Window *dest)
{
	RECT rc;
	HRESULT hr;
	DDCONTEXT;

	if (!dd) return M4BadParam;
	if (dd->is_3D_out) {
		SwapBuffers(dd->gl_HDC);
		return M4OK;
	}
	if (!dd->ddraw_init) return M4BadParam;

	if (dest) {
		POINT pt;
		pt.x = dest->x;
		pt.y = dest->y;
		ClientToScreen(dd->hWnd, &pt);
		dest->x = pt.x;
		dest->y = pt.y;
		MAKERECT(rc, dest);
		hr = IDirectDrawSurface_Blt(dd->pPrimary, &rc, dd->pBack, NULL, DDBLT_WAIT, NULL );
	} else {
		hr = IDirectDrawSurface_Blt(dd->pPrimary, NULL, dd->pBack, NULL, DDBLT_WAIT, NULL );
	}
	if (hr == DDERR_SURFACELOST) {
		IDirectDrawSurface_Restore(dd->pPrimary);
		IDirectDrawSurface_Restore(dd->pBack);
	}
	return FAILED(hr) ? M4IOErr : M4OK;
}



#ifdef USE_DX_3
HRESULT WINAPI EnumDisplayModes( LPDDSURFACEDESC lpDDDesc, LPVOID lpContext)
#else
HRESULT WINAPI EnumDisplayModes( LPDDSURFACEDESC2 lpDDDesc, LPVOID lpContext)
#endif
{
	DDContext *dd = (DDContext *) lpContext;
	
	//check W and H
	if (dd->width <= lpDDDesc->dwWidth  && dd->height <= lpDDDesc->dwHeight
		//check FSW and FSH
		&& dd->fs_width > lpDDDesc->dwWidth && dd->fs_height > lpDDDesc->dwHeight) {

		if (lpDDDesc->dwHeight == 200)
			return DDENUMRET_OK;
		
		dd->fs_width = lpDDDesc->dwWidth;
		dd->fs_height = lpDDDesc->dwHeight;

		return DDENUMRET_CANCEL;
	}
	return DDENUMRET_OK;
}

M4Err GetDisplayMode(DDContext *dd)
{
	if (dd->switch_res) {
		HRESULT hr;
		Bool temp_dd = 0;;
		if (!dd->pDD) {
			LPDIRECTDRAW ddraw;
			DirectDrawCreate(NULL, &ddraw, NULL);
#ifdef USE_DX_3
			IDirectDraw_QueryInterface(ddraw, &IID_IDirectDraw, (LPVOID *)&dd->pDD);
#else
			IDirectDraw_QueryInterface(ddraw, &IID_IDirectDraw7, (LPVOID *)&dd->pDD);
#endif		
			temp_dd = 1;
		}
		//we start with a hugde res and downscale
		dd->fs_width = dd->fs_height = 50000;

#ifdef USE_DX_3
		hr = IDirectDraw_EnumDisplayModes(dd->pDD, 0L, NULL, dd,  (LPDDENUMMODESCALLBACK) EnumDisplayModes);
#else
		hr = IDirectDraw7_EnumDisplayModes(dd->pDD, 0L, NULL, dd,  (LPDDENUMMODESCALLBACK2) EnumDisplayModes);
#endif
		if (temp_dd) SAFE_DD_RELEASE(dd->pDD);
		if (FAILED(hr)) return M4IOErr;
	} else {
		dd->fs_width = GetSystemMetrics(SM_CXSCREEN);
		dd->fs_height = GetSystemMetrics(SM_CYSCREEN);
	}
	return M4OK;
}



static void *NewVideoOutput()
{
	DDContext *pCtx;
	VideoOutput *driv = (VideoOutput *) malloc(sizeof(VideoOutput));
	memset(driv, 0, sizeof(VideoOutput));
	M4_REG_PLUG(driv, M4_VIDEO_OUTPUT_INTERFACE, "DirectX Video Output", "gpac distribution", 0);

	pCtx = malloc(sizeof(DDContext));
	memset(pCtx, 0, sizeof(DDContext));
	pCtx->surfaces = NewChain();
	driv->opaque = pCtx;
	driv->FlushVideo = DD_FlushVideo;
	driv->Resize = DD_Resize;
	driv->SetFullScreen = DD_SetFullScreen;
	driv->SetupHardware = DD_SetupHardware;
	driv->Shutdown = DD_Shutdown;
	driv->PushEvent = DD_PushEvent;
	driv->bHas3DSupport = 1;

	DD_SetupDDraw(driv);

	return (void *)driv;
}

static void DeleteVideoOutput(void *ifce)
{
	VideoOutput *driv = (VideoOutput *) ifce;
	DDContext *dd = (DDContext *)driv->opaque;

	DeleteChain(dd->surfaces);
	free(dd);
	free(driv);
}

/*interface query*/
Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return 1;
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) return 1;
	return 0;
}
/*interface create*/
void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_VIDEO_OUTPUT_INTERFACE) return NewVideoOutput();
	if (InterfaceType == M4_AUDIO_OUTPUT_INTERFACE) return NewAudioOutput();
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
	case M4_AUDIO_OUTPUT_INTERFACE:
		DeleteAudioOutput(ifce);
		break;
	}
}
