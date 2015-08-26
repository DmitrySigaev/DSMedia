/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / GAPI WinCE-iPaq video render plugin
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


#include <windows.h>
#include <aygshell.h>
#include <gx.h>

#include "gapi.h"
#include "ColorTools.h"

#define GAPI_TOOLBAR_H 26

/*color tools*/
#define M4C_A(c) (u8) (c>>24)
#define M4C_R(c) (u8) ( (c>>16) & 0xFF)
#define M4C_G(c) (u8) ( (c>>8) & 0xFF)
#define M4C_B(c) (u8) ( c & 0xFF)

/*color transform macros*/
#define M4COL_565(c) (((M4C_R(c) & 248)<<8) + ((M4C_G(c) & 252)<<3)  + (M4C_B(c)>>3))
#define M4COL_555(c) (((M4C_R(c) & 248)<<7) + ((M4C_G(c) & 248)<<2)  + (M4C_B(c)>>3))

#define GAPICTX(dr)	GAPIPriv *gctx = (GAPIPriv *) dr->opaque;


static VideoOutput *the_video_driver = NULL;

static void GAPI_MapBIFSCorrdinate(DWORD lParam, M4Event *evt)
{
	GAPIPriv *ctx = (GAPIPriv *)the_video_driver->opaque;
	evt->mouse.x = LOWORD(lParam);
	evt->mouse.y = HIWORD(lParam);

	if (ctx->fullscreen) {
		RECT rc;
		s32 tmp = evt->mouse.y;
		GetClientRect(ctx->hWnd, &rc);
		evt->mouse.y = evt->mouse.x;
		evt->mouse.x = rc.bottom - tmp;
		evt->mouse.x -= ctx->bb_width / 2;
		evt->mouse.y = ctx->bb_height / 2 - evt->mouse.y;
	} else {
		evt->mouse.x -= ctx->bb_width / 2;
		evt->mouse.y = (ctx->bb_height ) / 2 - evt->mouse.y;
	}
}

static u32 GAPI_TranslateActionKey(u32 VirtKey) 
{
	switch (VirtKey) {
	case VK_HOME: return M4VK_HOME;
	case VK_END: return M4VK_END;
	case VK_NEXT: return M4VK_PRIOR;
	case VK_PRIOR: return M4VK_NEXT;
	case VK_UP: return M4VK_UP;
	case VK_DOWN: return M4VK_DOWN;
	case VK_LEFT: return M4VK_LEFT;
	case VK_RIGHT: return M4VK_RIGHT;
	case VK_F1: return M4VK_F1;
	case VK_F2: return M4VK_F2;
	case VK_F3: return M4VK_F3;
	case VK_F4: return M4VK_F4;
	case VK_F5: return M4VK_F5;
	case VK_F6: return M4VK_F6;
	case VK_F7: return M4VK_F7;
	case VK_F8: return M4VK_F8;
	case VK_F9: return M4VK_F9;
	case VK_F10: return M4VK_F10;
	case VK_F11: return M4VK_F11;
	case VK_F12: return M4VK_F12;
	case VK_RETURN: return M4VK_RETURN;
	case VK_ESCAPE: return M4VK_ESCAPE;
	case VK_SHIFT: return M4VK_SHIFT;
	case VK_CONTROL: return M4VK_CONTROL;
	case VK_MENU: 
		return M4VK_MENU;
	default: return 0;
	}
}

/*navigation pad keys*/
#define VK_IPAQ_LEFT		0x25
#define VK_IPAQ_UP			0x26
#define VK_IPAQ_RIGHT		0x27
#define VK_IPAQ_DOWN		0x28
/*"enter" key*/
#define VK_IPAQ_START		0x86
/*ipaq keys from left to right*/
#define VK_IPAQ_A			0xC1
#define VK_IPAQ_B			0xC2
#define VK_IPAQ_C			0xC3
#define VK_IPAQ_D			0xC4
/*record button*/
#define VK_IPAQ_E			0xC5

u32 IPAQ_TRANSLATE_KEY(u32 vk)
{
	u32 res = (LOWORD(vk) != 0x5b) ? LOWORD(vk) : vk;
	switch (res) {
	case VK_IPAQ_LEFT: return M4VK_LEFT;
	case VK_IPAQ_UP: return M4VK_UP;
	case VK_IPAQ_RIGHT: return M4VK_RIGHT;
	case VK_IPAQ_DOWN: return M4VK_DOWN;
	case VK_IPAQ_START: return M4VK_RETURN;
	case VK_IPAQ_E: return M4VK_F5;
	case VK_IPAQ_A: return M4VK_F4;
	case VK_IPAQ_B: return M4VK_F3;
	case VK_IPAQ_C: return M4VK_F2;
	case VK_IPAQ_D: return M4VK_F1;
	default: return 0;
	}
}

LRESULT APIENTRY GAPI_WindowProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
	M4Event evt;
	switch (msg) {
	case WM_SIZE:
	{
		GAPIPriv *ctx = (GAPIPriv *)the_video_driver->opaque;
		if (!ctx->is_resizing) {
			ctx->is_resizing = 1;
			evt.type = M4E_WINDOWSIZE;
			evt.size.width = LOWORD(lParam);
			evt.size.height = HIWORD(lParam);
			the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
			ctx->is_resizing = 0;
		}
	}
		break;
	case WM_CLOSE:
	{
		evt.type = M4E_QUIT;
		the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
	}
		return 1;
	case WM_DESTROY:
		PostQuitMessage (0);
		break;

	case WM_ERASEBKGND:
	case WM_PAINT:
	{
		GAPIPriv *ctx = (GAPIPriv *)the_video_driver->opaque;
		evt.type = M4E_REFRESH;
		if (ctx->backbuffer) the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
	}
		break;

	case WM_MOUSEMOVE:
		GAPI_MapBIFSCorrdinate(lParam, &evt);
		evt.type = M4E_MOUSEMOVE;
		the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		GAPI_MapBIFSCorrdinate(lParam, &evt);
		evt.type = M4E_LEFTDOWN;
		the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
		break;
	case WM_LBUTTONUP:
		GAPI_MapBIFSCorrdinate(lParam, &evt);
		evt.type = M4E_LEFTUP;
		the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
		break;

	/*FIXME - there's a bug on alt state (we miss one event)*/
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		evt.key.m4_vk_code = GAPI_TranslateActionKey(wParam);
		evt.key.virtual_code = wParam;
		if (evt.key.m4_vk_code) {
			evt.type = (msg==WM_SYSKEYDOWN) ? M4E_VKEYDOWN : M4E_VKEYUP;
			if (evt.key.m4_vk_code<=M4VK_RIGHT) evt.key.virtual_code = 0;
			the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		/*emulate button actions as vk codes*/
		if (msg==WM_KEYDOWN) {
			if ((evt.key.m4_vk_code = IPAQ_TRANSLATE_KEY(wParam)) != 0) {
				evt.key.virtual_code = wParam;
				evt.type = M4E_VKEYDOWN;
				the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
				break;
			}
		}
		evt.key.m4_vk_code = GAPI_TranslateActionKey(wParam);
		evt.key.virtual_code = wParam;
		if (evt.key.m4_vk_code) {
			evt.type = (msg==WM_KEYDOWN) ? M4E_VKEYDOWN : M4E_VKEYUP;
			if (evt.key.m4_vk_code<=M4VK_RIGHT) evt.key.virtual_code = 0;
			the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
			/*also send a normal key for non-key-sensors*/
			if (evt.key.m4_vk_code>M4VK_RIGHT) goto send_key;
		} else {
send_key:
			evt.type = (msg==WM_KEYDOWN) ? M4E_KEYDOWN : M4E_KEYUP;
			the_video_driver->on_event(the_video_driver->evt_cbk_hdl, &evt);
		}
	}
		break;
	case WM_CHAR:
		evt.type = M4E_CHAR;
		evt.character.unicode_char = wParam;
		break;
	}
	return DefWindowProc (hWnd, msg, wParam, lParam);
}

void DD_WindowThread(void *par)
{
	MSG msg;
	WNDCLASS wc;
	GAPIPriv *ctx = (GAPIPriv *)the_video_driver->opaque;

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.hInstance = GetModuleHandle(_T("gapi.dll"));
	wc.lpfnWndProc = GAPI_WindowProc;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject (BLACK_BRUSH);
	wc.lpszClassName = _T("GPAC DirectDraw Output");
	RegisterClass (&wc);
	
	ctx->hWnd = CreateWindow(_T("GPAC DirectDraw Output"), NULL, WS_POPUP, 0, 0, 120, 100, NULL, NULL, wc.hInstance, NULL);
	if (ctx->hWnd == NULL) {
		ctx->ThreadID = 0;
		ExitThread(1);
	}
	ShowWindow(ctx->hWnd, SW_SHOWNORMAL);

	while (GetMessage (&(msg), NULL, 0, 0)) {
		TranslateMessage (&(msg));
		DispatchMessage (&(msg));
	}
	ctx->ThreadID = 0;
	ExitThread (0);
}


void GAPI_SetupWindow(VideoOutput *dr)
{
	GAPIPriv *ctx = (GAPIPriv *)dr->opaque;
	if (the_video_driver) return;
	the_video_driver = dr;

	if (!ctx->hWnd) {
		ctx->hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) DD_WindowThread, (LPVOID) dr, 0, &(ctx->ThreadID) );
		while (!ctx->hWnd && ctx->hThread) Sleep(10);
		if (!ctx->hThread) return;
		ctx->owns_hwnd = 1;
	} else {
		/*override window proc*/
		SetWindowLong(ctx->hWnd, GWL_WNDPROC, (DWORD) GAPI_WindowProc);
	}
}

void GAPI_ShutdownWindow(VideoOutput *dr)
{
	GAPIPriv *ctx = (GAPIPriv *)dr->opaque;

	if (ctx->owns_hwnd) {
		PostMessage(ctx->hWnd, WM_DESTROY, 0, 0);
		while (ctx->ThreadID) Sleep(10);
		UnregisterClass(_T("GPAC DirectDraw Output"), GetModuleHandle(_T("dx_hw.dll")));
		CloseHandle(ctx->hThread);
		ctx->hThread = NULL;
	}
	ctx->hWnd = NULL;
	the_video_driver = NULL;
}

static M4Window GAPI_RectToWindow(GAPIPriv *gctx, RECT *rc)
{
	M4Window wnd;
	if (!rc) {
		if (gctx->fullscreen) {
			if (gctx->g_prop.ffFormat & kfLandscape) {
				wnd.x = (gctx->g_prop.cbxPitch>0) ? 0 : gctx->screen_h;
				wnd.y = (gctx->g_prop.cbyPitch>0) ? 0 : gctx->screen_w;
			} else {
				wnd.x = (gctx->g_prop.cbxPitch>0) ? 0 : gctx->screen_w;
				wnd.y = (gctx->g_prop.cbyPitch>0) ? 0 : gctx->screen_h;
			}
			wnd.w = gctx->fs_h;
			wnd.h = gctx->fs_w;
		} else {
			if (gctx->g_prop.ffFormat & kfLandscape) {
				wnd.x = (gctx->g_prop.cbxPitch>0) ? 0 : gctx->screen_w;
				wnd.y = (gctx->g_prop.cbyPitch>0) ? 0 : gctx->screen_h;
				wnd.w = gctx->screen_h;
				wnd.h = gctx->screen_w;
			} else {
				wnd.x = (gctx->g_prop.cbxPitch>0) ? 0 : gctx->screen_h;
				wnd.y = (gctx->g_prop.cbyPitch>0) ? 0 : gctx->screen_w;
				wnd.w = gctx->screen_h;
				wnd.h = gctx->screen_w;
			}
		}
	} else {
		if (gctx->g_prop.ffFormat & kfLandscape) {
			wnd.x = gctx->screen_h - rc->bottom;
			if (wnd.x < GAPI_TOOLBAR_H) wnd.x = GAPI_TOOLBAR_H;
			wnd.y = rc->left;
		} else {
			wnd.x = rc->left;
			wnd.y = rc->top;
		}
		wnd.w = rc->right - rc->left;
		wnd.h = rc->bottom - rc->top;
	}
	return wnd;
}


M4Err GAPI_Clear(VideoOutput *dr, u32 color)
{
	u32 i, j, w, h;
	RECT rc;
	s32 x_off, y_off;
	char *ptr;
	unsigned short col, *s_ptr;
	GAPICTX(dr);

	if (!gctx || !gctx->is_init) return M4IOErr;

	MX_P(gctx->mx);
	col = 0;
	switch (gctx->pixel_format) {
	case M4PF_RGB_555:
		col = M4COL_555(color);
		break;
	case M4PF_RGB_565:
		col = M4COL_565(color);
		break;
	}


	::GetWindowRect(GetParent(gctx->hWnd), &rc);
	if (gctx->fullscreen) {
		rc.top = rc.left = 0;
		rc.right = gctx->screen_w;
		rc.bottom = gctx->screen_h;
	}

	if (gctx->g_prop.ffFormat & kfLandscape) {
		x_off = gctx->screen_h - rc.bottom;
		w = rc.bottom - rc.top;
		y_off = rc.left;
		h = rc.right - rc.left;
	} else {
		x_off = rc.left;
		w = rc.right - rc.left;
		y_off = rc.top;
		h = rc.bottom - rc.top;
	}

	ptr = (char *) GXBeginDraw();
	if (!ptr) {
		MX_V(gctx->mx);
		return M4IOErr;
	}
	ptr += gctx->stride_x * y_off + x_off * gctx->stride_y;

	for (i=y_off; i<h; i++) {
		s_ptr = (unsigned short *) ptr;
		if (col) {
			for (j=0; j<w; j++) s_ptr[j] = col;
		} else {
			memset(s_ptr, 0, sizeof(unsigned short) * w);
		}
		ptr += gctx->stride_x;
	}
	GXEndDraw();

	MX_V(gctx->mx);
	return M4OK;
}


static Bool GAPI_InitSurface(VideoOutput *dr, u32 VideoWidth, u32 VideoHeight)
{
	GAPICTX(dr);

	if (!gctx || !VideoWidth || !VideoHeight) return FALSE;

	MX_P(gctx->mx);

	if (!gctx->fullscreen) {
		if (VideoWidth > gctx->screen_w) VideoWidth = gctx->screen_w;
		if (VideoHeight + 2*GAPI_TOOLBAR_H > gctx->screen_h) VideoHeight = gctx->screen_h - 2*GAPI_TOOLBAR_H;
	}

	gctx->g_prop = GXGetDisplayProperties();
	if (gctx->g_prop.ffFormat & kfDirect555) {
		gctx->pixel_format = M4PF_RGB_555;
		gctx->BPP = 2;
		gctx->bitsPP = 15;
	} 
	else if (gctx->g_prop.ffFormat & kfDirect565) {
		gctx->pixel_format = M4PF_RGB_565;
		gctx->BPP = 2;
		gctx->bitsPP = 16;
	}
	else if (gctx->g_prop.ffFormat & kfDirect888) {
		gctx->pixel_format = M4PF_RGB_24;
		gctx->BPP = 3;
		gctx->bitsPP = 24;
	} else {
		MX_V(gctx->mx);
		return 0;
	}
	gctx->screen_w = gctx->g_prop.cxWidth;
	gctx->screen_h = gctx->g_prop.cyHeight;
	if (gctx->g_prop.cbxPitch>0) gctx->stride_x = gctx->g_prop.cbxPitch;
	else gctx->stride_x = -1*gctx->g_prop.cbxPitch;
	if (gctx->g_prop.cbyPitch>0) gctx->stride_y = gctx->g_prop.cbyPitch;
	else gctx->stride_y = -1*gctx->g_prop.cbyPitch;

	if (gctx->backbuffer) free(gctx->backbuffer);
	gctx->bb_size = VideoWidth * VideoHeight * gctx->BPP;
	gctx->backbuffer = (unsigned char *) malloc(sizeof(unsigned char) * gctx->bb_size);
	gctx->bb_width = VideoWidth;
	gctx->bb_height = VideoHeight;
	gctx->bb_pitch = VideoWidth * gctx->BPP;
	gctx->is_init = 1;
	GAPI_Clear(dr, 0);
	MX_V(gctx->mx);
	return 1;
}

M4Err GAPI_SetupHardware(VideoOutput *dr, void *os_handle, Bool no_proc_override, M4GLConfig *cfg)
{
	RECT rc;
	GAPICTX(dr);
	gctx->hWnd = (HWND) os_handle;
	
	if (cfg) return M4NotSupported;
	GAPI_SetupWindow(dr);
	if (!gctx->hWnd || !GXOpenDisplay(gctx->hWnd, 0L)) return M4IOErr;

	gctx->gapi_open = 1;
    GetClientRect(gctx->hWnd, &rc);
	gctx->disp_w = rc.right - rc.left;
	gctx->disp_h = rc.bottom - rc.top;

	if (!GAPI_InitSurface(dr, gctx->disp_w, gctx->disp_h)) return M4IOErr;
	return M4OK;
}

static void GAPI_Shutdown(VideoOutput *dr)
{
	GAPICTX(dr);

	MX_P(gctx->mx);

	if (gctx->backbuffer) free(gctx->backbuffer);
	gctx->backbuffer = NULL;

	while (ChainGetCount(gctx->surfaces)) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, 0);
		ChainDeleteEntry(gctx->surfaces, 0);
		free(ptr->buffer);
		free(ptr);
	}

	if (gctx->gapi_open) {
		GXCloseDisplay();
		gctx->gapi_open = 0;
	}

	GAPI_ShutdownWindow(dr);

	MX_V(gctx->mx);
}

static M4Err GAPI_SetFullScreen(VideoOutput *dr, Bool bOn, u32 *outWidth, u32 *outHeight)
{
	M4Err e;
	GAPICTX(dr);

	if (!gctx || !gctx->is_init) return M4BadParam;
	if (bOn == gctx->fullscreen) return M4OK;

	GAPI_Clear(dr, 0);

	MX_P(gctx->mx);
	GXCloseDisplay();
	e = M4OK;
	if (bOn) {
		if (!GXOpenDisplay(GetParent(gctx->hWnd), GX_FULLSCREEN)) {
			GXOpenDisplay(gctx->hWnd, 0L);
			gctx->fullscreen = 0;
			e = M4IOErr;
		} else {
			gctx->fullscreen = 1;
		}
	} else {
		GXOpenDisplay(gctx->hWnd, 0L);
		gctx->fullscreen = 0;
	}
	MX_V(gctx->mx);

	if (e) return e;
	if (gctx->fullscreen) {
		if (gctx->g_prop.ffFormat & kfLandscape) {
			gctx->fs_w = gctx->screen_h;
			gctx->fs_h = gctx->screen_w;
		} else {
			gctx->fs_w = gctx->screen_w;
			gctx->fs_h = gctx->screen_h;
		}
		*outWidth = gctx->fs_w;
		*outHeight = gctx->fs_h;
	} else {
		*outWidth = gctx->disp_w;
		*outHeight = gctx->disp_h;
	}
	if (!GAPI_InitSurface(dr, *outWidth, *outHeight)) return M4IOErr; 
	GAPI_Clear(dr, 0);
	return M4OK;
}

static M4Err GAPI_Resize(VideoOutput *dr, u32 width, u32 height)
{
	GAPICTX(dr);

	/*store width and height*/
/*	-- this is now moved to PushEvent --
	if (!gctx->fullscreen) {
		if (!gctx->is_resizing) {
			gctx->is_resizing = 1;
			SetWindowPos(gctx->hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
			gctx->is_resizing = 0;
		}
		gctx->disp_w = width;
		gctx->disp_h = height;
	}
*/
	return GAPI_InitSurface(dr, width, height); 
}



static M4Err GAPI_FlushVideo(VideoOutput *dr, M4Window *dest)
{
	unsigned char *ptr;
	GAPICTX(dr);

	if (!gctx || !gctx->is_init) return M4BadParam;
	if (dest && (!dest->w || !dest->h)) return M4OK;
	if (gctx->is_resizing) return M4OK;

	MX_P(gctx->mx);

	/*get a pointer to video memory*/
	ptr = (unsigned char *) GXBeginDraw();
	if (!ptr) {
		MX_V(gctx->mx);
		return M4IOErr;
	}
	
	/*full screen, do NOT rotate*/
	if (gctx->fullscreen) {
		ptr += gctx->stride_x * dest->y + dest->x * gctx->stride_y;
		gctx->dst_blt.x = dest->x;
		gctx->dst_blt.y = dest->y;
		gctx->dst_blt.w = dest->w;
		gctx->dst_blt.h = dest->h;

		if (gctx->dst_blt.x + gctx->dst_blt.w > gctx->fs_w) gctx->dst_blt.w = gctx->fs_w - gctx->dst_blt.x;
		if (gctx->dst_blt.y + gctx->dst_blt.h > gctx->fs_h) gctx->dst_blt.h = gctx->fs_h - gctx->dst_blt.y;

		StretchBits(ptr, gctx->bitsPP, gctx->dst_blt.w, gctx->dst_blt.h, gctx->stride_x,
				/*src*/
				gctx->backbuffer, gctx->bitsPP, gctx->bb_width, gctx->bb_height, gctx->bb_pitch,
				0);
	} else {
		RECT rc;
		
		GetWindowRect(gctx->hWnd, &rc);
		gctx->dst_blt = GAPI_RectToWindow(gctx, &rc);

		gctx->dst_blt.x += dest->y;
		gctx->dst_blt.y += dest->x;
		gctx->dst_blt.w = dest->w;
		gctx->dst_blt.h = dest->h;

		if ((gctx->dst_blt.w > gctx->bb_width) || (gctx->dst_blt.h > gctx->bb_height)) {
			gctx->dst_blt.w = gctx->bb_width;
			gctx->dst_blt.h = gctx->bb_height;
		}
		if (gctx->g_prop.ffFormat & kfLandscape) {
			ptr += gctx->stride_x * gctx->dst_blt.y + gctx->dst_blt.x * gctx->stride_y;

			RotateBits(ptr, 
						gctx->bitsPP, gctx->dst_blt.w, gctx->dst_blt.h, gctx->stride_x,
						/*src & src bpp*/
						gctx->backbuffer, gctx->bitsPP, gctx->bb_width, gctx->bb_height, gctx->bb_pitch,
						/*90 deg rotation*/
						SB_ROT90);
		} else {
			ptr += gctx->stride_x * gctx->dst_blt.y + gctx->dst_blt.x * gctx->stride_y;

			StretchBits(ptr, 
						gctx->bitsPP, gctx->dst_blt.w, gctx->dst_blt.h, gctx->stride_x,
						/*src & src bpp*/
						gctx->backbuffer, gctx->bitsPP, gctx->bb_width, gctx->bb_height, gctx->bb_pitch,
						0);
		}
	}

	GXEndDraw();
	MX_V(gctx->mx);
	return M4OK;
}

static M4Err GAPI_LockSurface(VideoOutput *dr, u32 surface_id, M4VideoSurface *vi)
{
	u32 i;
	GAPICTX(dr);

	if (!surface_id) {
		vi->width = gctx->bb_width;
		vi->height = gctx->bb_height;
		vi->pitch = gctx->bb_pitch;
		vi->pixel_format = gctx->pixel_format;
		vi->os_handle = NULL;
		vi->video_buffer = gctx->backbuffer;
		return M4OK;
	}
	/*check surfaces*/
	for (i=0; i<ChainGetCount(gctx->surfaces); i++) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, i);
		if (ptr->ID == surface_id) {
			vi->width = ptr->width;
			vi->height = ptr->height;
			vi->pitch = ptr->pitch;
			vi->pixel_format = ptr->pixel_format;
			vi->os_handle = NULL;
			vi->video_buffer = ptr->buffer;
			return M4OK;
		}
	}
	return M4BadParam;
}

static M4Err GAPI_UnlockSurface(VideoOutput *dr, u32 surface_id)
{
	/*no special things to do on unlock*/
	return M4OK;
}

static M4Err GAPI_GetPixelFormat(VideoOutput *dr, u32 surfaceID, u32 *pixel_format)
{
	u32 i;
	GAPICTX(dr);

	if (!surfaceID) {
		*pixel_format = gctx->pixel_format;
		return M4OK;
	}
	/*check surfaces*/
	for (i=0; i<ChainGetCount(gctx->surfaces); i++) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, i);
		if (ptr->ID == surfaceID) {
			*pixel_format = ptr->pixel_format;
			return M4OK;
		}
	}
	return M4BadParam;
}

static M4Err GAPI_Blit(VideoOutput *dr, u32 src_id, u32 dst_id, M4Window *src, M4Window *dst)
{
	M4VideoSurface vs;
	M4Err e;
	void *pdst, *psrc;
	
	GAPICTX(dr);

	/*NOT SUPPORTED*/
	if (dst_id) return 0;

	e = GAPI_LockSurface(dr, src_id, &vs);
	if (e) return e;

	switch (vs.pixel_format) {
	case M4PF_RGB_555:
		pdst = gctx->backbuffer + dst->y * gctx->bb_pitch + dst->x * gctx->BPP;
		psrc = vs.video_buffer + src->y * vs.pitch + src->x * 2;
		StretchBits(pdst, gctx->bitsPP, dst->w, dst->h, gctx->bb_pitch, 
					psrc, 15, src->w, src->h, vs.pitch, 0);
		break;
	case M4PF_RGB_565:
		pdst = gctx->backbuffer + dst->y * gctx->bb_pitch + dst->x * gctx->BPP;
		psrc = vs.video_buffer + src->y * vs.pitch + src->x * 2;
		StretchBits(pdst, gctx->bitsPP, dst->w, dst->h, gctx->bb_pitch, 
					psrc, 16, src->w, src->h, vs.pitch, 0);
		break;
	case M4PF_RGB_24:
		pdst = gctx->backbuffer + dst->y * gctx->bb_pitch + dst->x * gctx->BPP;
		psrc = vs.video_buffer + src->y * vs.pitch + src->x * 3;
		StretchBits(pdst, gctx->bitsPP, dst->w, dst->h, gctx->bb_pitch, 
					psrc, 24, src->w, src->h, vs.pitch, 0);
		break;
	case M4PF_RGB_32:
		pdst = gctx->backbuffer + dst->y * gctx->bb_pitch + dst->x * gctx->BPP;
		psrc = vs.video_buffer + src->y * vs.pitch + src->x * 4;
		StretchBits(pdst, gctx->bitsPP, dst->w, dst->h, gctx->bb_pitch, 
					psrc, 32, src->w, src->h, vs.pitch, 0);
		break;
	}
	return M4OK;
}

static M4Err GAPI_CreateSurface(VideoOutput *dr, u32 width, u32 height, u32 pixel_format, u32 *surfaceID)
{
	u32 size;
	GAPISurface *surf;
	GAPICTX(dr);

	switch (pixel_format) {
	case M4PF_RGB_555:
	case M4PF_RGB_565:
	case M4PF_RGB_24:
	case M4PF_RGB_32:
		break;
	default:
		return M4NotSupported;
	}
	
	surf = (GAPISurface *) malloc(sizeof(GAPISurface));
	surf->width = width;
	surf->height = height;
	surf->ID = (u32) surf;
	surf->pixel_format = pixel_format;

	size = width * height;
	switch (pixel_format) {
	case M4PF_RGB_555:
		size *= 2;
		surf->pitch = width * 2;
		break;
	case M4PF_RGB_565:
		size *= 2;
		surf->pitch = width * 2;
		break;
	case M4PF_RGB_24:
		size *= 3;
		surf->pitch = width * 3;
		break;
	case M4PF_RGB_32:
		size *= 4;
		surf->pitch = width * 4;
		break;
	}
	surf->buffer = (unsigned char *)malloc(sizeof(unsigned char) * size);
	ChainAddEntry(gctx->surfaces, surf);

	*surfaceID = surf->ID;

	return M4OK;
}


/*deletes video surface by id*/
static M4Err GAPI_DeleteSurface(VideoOutput *dr, u32 surfaceID)
{
	u32 i;
	GAPICTX(dr);
	if (!surfaceID) return M4BadParam;

	/*check surfaces*/
	for (i=0; i<ChainGetCount(gctx->surfaces); i++) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, i);
		if (ptr->ID == surfaceID) {
			if (ptr->buffer) free(ptr->buffer);
			free(ptr);
			ChainDeleteEntry(gctx->surfaces, i);
			return M4OK;
		}
	}
	return M4BadParam;
}


Bool GAPI_IsSurfaceValid(VideoOutput *dr, u32 surface_id)
{
	u32 i;
	GAPICTX(dr);
	if (!surface_id) {
		if (!gctx->is_init) return 0;
		return 1;
	}
	/*check surfaces*/
	for (i=0; i<ChainGetCount(gctx->surfaces); i++) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, i);
		if (ptr->ID == surface_id) return 1;
	}
	return 0;
}

static M4Err GAPI_ResizeSurface(VideoOutput *dr, u32 surface_id, u32 width, u32 height)
{
	u32 i;
	GAPICTX(dr);
	if (!surface_id) return M4BadParam;

	/*check surfaces*/
	for (i=0; i<ChainGetCount(gctx->surfaces); i++) {
		GAPISurface *ptr = (GAPISurface *) ChainGetEntry(gctx->surfaces, i);
		if (ptr->ID == surface_id) {
			if ( (ptr->height>=height) && (ptr->width>=width)) return M4OK;
			if (ptr->buffer) free(ptr->buffer);
			ptr->pitch = ptr->pitch * width / ptr->width;
			ptr->width = width;
			ptr->height = height;
			ptr->buffer = (unsigned char *) malloc(sizeof(unsigned char) * ptr->height * ptr->pitch);
			return M4OK;
		}
	}
	return M4BadParam;
}

static M4Err GAPI_PushEvent(VideoOutput *dr, M4Event *evt)
{
	GAPICTX(dr);
	switch (evt->type) {
	case M4E_SHOWHIDE:
		if (gctx->hWnd) ShowWindow(gctx->hWnd, evt->show.is_visible ? SW_SHOW : SW_HIDE);
		break;
	case M4E_NEEDRESIZE:
		gctx->is_resizing = 1;
		SetWindowPos(gctx->hWnd, NULL, 0, 0, evt->size.width, evt->size.height, SWP_NOZORDER | SWP_NOMOVE);
		gctx->is_resizing = 0;
		gctx->disp_w = evt->size.width;
		gctx->disp_h = evt->size.height;
		break;
	}
	return M4OK;
}


static void *NewVideoOutput()
{
	GAPIPriv *priv;
	VideoOutput *driv = (VideoOutput *) malloc(sizeof(VideoOutput));
	memset(driv, 0, sizeof(VideoOutput));
	M4_REG_PLUG(driv, M4_VIDEO_OUTPUT_INTERFACE, "GAPI Video Output", "gpac distribution", 0)

	priv = (GAPIPriv *) malloc(sizeof(GAPIPriv));
	memset(priv, 0, sizeof(GAPIPriv));
	priv->mx = NewMutex();
	priv->surfaces = NewChain();
	driv->opaque = priv;

	/*alpha and keying to do*/
	driv->bHasAlpha = 0;
	driv->bHasKeying = 0;
	driv->bHasYUV = 0;

	driv->Blit = GAPI_Blit;
	driv->Clear = GAPI_Clear;
	driv->CreateSurface = GAPI_CreateSurface;
	driv->DeleteSurface = GAPI_DeleteSurface;
	driv->FlushVideo = GAPI_FlushVideo;
	driv->GetPixelFormat = GAPI_GetPixelFormat;
	driv->LockSurface = GAPI_LockSurface;
	driv->IsSurfaceValid = GAPI_IsSurfaceValid;
	driv->Resize = GAPI_Resize;
	driv->SetFullScreen = GAPI_SetFullScreen;
	driv->SetupHardware = GAPI_SetupHardware;
	driv->Shutdown = GAPI_Shutdown;
	driv->UnlockSurface = GAPI_UnlockSurface;
	driv->ResizeSurface	= GAPI_ResizeSurface;
	driv->PushEvent = GAPI_PushEvent;
	return (void *)driv;
}

static void DeleteVideoOutput(void *ifce)
{
	VideoOutput *driv = (VideoOutput *) ifce;
	GAPICTX(driv);
	GAPI_Shutdown(driv);
	MX_Delete(gctx->mx);
	DeleteChain(gctx->surfaces);
	free(gctx);
	free(driv);
}

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

