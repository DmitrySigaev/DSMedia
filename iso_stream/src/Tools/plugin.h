/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
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

#include <m4_tools.h>

#ifndef PLUGIN_H
#define PLUGIN_H

/* interface api*/
typedef Bool (*QueryInterface) (u32 InterfaceType);
typedef void * (*LoadInterface) (u32 InterfaceType);
typedef void (*ShutdownInterface) (void *interface_obj);


typedef struct _tagPlugInst
{
	struct _tagPlugMan *plugman;
	Chain *interfaces;
	void *lib_handle;
	unsigned char szName[M4_MAX_PATH];

	QueryInterface query_func;
	LoadInterface load_func;
	ShutdownInterface destroy_func;
} PlugInst;


typedef struct _tagPlugMan
{
	/*location of the plugins*/
	unsigned char dir[M4_MAX_PATH];
	Chain *plug_list;
	LPINIFILE cfg;
} PlugMan;

/*returns 1 if a plugin with the same filename is already loaded*/
Bool is_plugin_loaded(LPPLUGMAN pm, unsigned char *filename);

/*these 2 are OS specific*/
void PM_FreePlugin(PlugInst *inst);
u32 PM_RefreshPlugins(LPPLUGMAN pm);

#endif

