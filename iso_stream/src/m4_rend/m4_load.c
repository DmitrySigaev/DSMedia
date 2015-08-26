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
 */

#if defined(GRAPHICS_2D_STATIC_BUILD) 
#include "m4_load_ifce.h"
#endif /* defined(GRAPHICS_2D_STATIC_BUILD)  */
#include "m4_priv.h"

/*we don't need any private context*/
Graphics2DDriver *M4_LoadRenderer()
{
	Graphics2DDriver *dr = malloc(sizeof(Graphics2DDriver));
	memset(dr, 0, sizeof(Graphics2DDriver));
	M4_REG_PLUG(dr, M4_GRAPHICS_2D_INTERFACE, "GPAC 2D Raster", "gpac distribution", 0)


	dr->new_stencil = m4_new_stencil;
	dr->delete_stencil = m4_delete_stencil;
	dr->stencil_set_matrix = m4_stencil_set_matrix;
	dr->set_brush_color = m4_set_brush_color;
	dr->set_gradient_mode = m4_set_gradient_mode;
	dr->set_linear_gradient = m4_set_linear_gradient;
	dr->set_radial_gradient = m4_set_radial_gradient;
	dr->set_gradient_interpolation = m4_set_gradient_interpolation;
	dr->stencil_set_texture = m4_stencil_set_texture;
	dr->set_texture_repeat_mode = m4_set_texture_repeat_mode;
	dr->set_texture_filter = m4_set_texture_filter;
	dr->stencil_set_color_matrix = m4_stencil_set_color_matrix;
	dr->stencil_reset_color_matrix = m4_stencil_reset_color_matrix;
	dr->set_texture_view = m4_set_texture_view;
	dr->set_texture_alpha = m4_set_texture_alpha;
	dr->stencil_create_texture = m4_stencil_create_texture;
	dr->stencil_texture_modified = m4_stencil_texture_modified;
	dr->stencil_set_pixel = m4_stencil_set_pixel;
	dr->stencil_get_pixel = m4_stencil_get_pixel;

	dr->new_surface = m4_new_surface;
	dr->delete_surface = m4_delete_surface;
	dr->attach_surface_to_device = NULL;
	dr->attach_surface_to_texture = m4_attach_surface_to_texture;
	dr->attach_surface_to_buffer = m4_attach_surface_to_buffer;
	dr->detach_surface = m4_detach_surface;
	dr->surface_set_raster_level = m4_surface_set_raster_level;
	dr->surface_set_matrix = m4_surface_set_matrix;
	dr->surface_set_clipper = m4_surface_set_clipper;
	dr->surface_set_path = m4_surface_set_path;
	dr->surface_fill = m4_surface_fill;
	dr->surface_flush = m4_surface_flush;
	dr->surface_clear = m4_surface_clear;
	return dr;
}

void M4_ShutdownRenderer(Graphics2DDriver *dr)
{
	free(dr);
}

#if !defined(GRAPHICS_2D_STATIC_BUILD)

Bool QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_GRAPHICS_2D_INTERFACE) return 1;
	return 0;
}

void *LoadInterface(u32 InterfaceType)
{
	if (InterfaceType==M4_GRAPHICS_2D_INTERFACE) {
		return M4_LoadRenderer();
	}
	return NULL;
}

void ShutdownInterface(void *ifce)
{
	Graphics2DDriver *dr = (Graphics2DDriver *)ifce;
	if (dr->InterfaceType == M4_GRAPHICS_2D_INTERFACE) {
		M4_ShutdownRenderer(dr);
	}
}
#else /* !defined(GRAPHICS_2D_STATIC_BUILD)*/

Bool GI_QueryInterface(u32 InterfaceType)
{
	if (InterfaceType == M4_GRAPHICS_2D_INTERFACE) return 1;
	return 0;
}

void *GI_LoadInterface(u32 InterfaceType)
{
	if (InterfaceType==M4_GRAPHICS_2D_INTERFACE) {
		return M4_LoadRenderer();
	}
	return NULL;
}

void GI_ShutdownInterface(void *ifce)
{
	Graphics2DDriver *dr = (Graphics2DDriver *)ifce;
	if (dr->InterfaceType == M4_GRAPHICS_2D_INTERFACE) {
		M4_ShutdownRenderer(dr);
	}
}

#endif/* !defined(GRAPHICS_2D_STATIC_BUILD)*/

