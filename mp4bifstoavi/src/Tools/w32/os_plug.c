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

#include "../plugin.h"
#ifdef STATIC_LIBRARY_INSTEAD_OF_DLL
#include "raw_video.h"
#include "render2d_ifce.h"
#include "m4_load_ifce.h"
#endif /* STATIC_LIBRARY_INSTEAD_OF_DLL */

/*delete all interfaces loaded on plugin object*/
void PM_FreePlugin(PlugInst *inst)
{
	void *objinterface;
	while (ChainGetCount(inst->interfaces)) {
		objinterface = ChainGetEntry(inst->interfaces, 0);
		ChainDeleteEntry(inst->interfaces, 0);
		inst->destroy_func(objinterface);
	}

	if (inst->lib_handle) FreeLibrary(inst->lib_handle);

	DeleteChain(inst->interfaces);
	free(inst);
}

/*refresh all plug info - not we don't check for deleted plugins but since we've open them the OS should forbid delete*/
u32 PM_RefreshPlugins(LPPLUGMAN pm)
{
	QueryInterface query_func;
	LoadInterface load_func;
	ShutdownInterface del_func;
	PlugInst *inst;
	unsigned char path[M4_MAX_PATH];
	unsigned char file[M4_MAX_PATH];

#ifndef STATIC_LIBRARY_INSTEAD_OF_DLL
	WIN32_FIND_DATA FindData;
	HANDLE SearchH;
	HMODULE PluginLib;
#endif /* STATIC_LIBRARY_INSTEAD_OF_DLL */
	
	if (!pm) return 0;
#ifndef STATIC_LIBRARY_INSTEAD_OF_DLL
	sprintf(path, "%s%c*", pm->dir, M4_PATH_SEPARATOR);
	SearchH= FindFirstFile(path, &FindData);
	if (SearchH == INVALID_HANDLE_VALUE) return 0;
	while (SearchH != INVALID_HANDLE_VALUE) {
	
		if (!strstr(FindData.cFileName, ".dll")) goto next;
		/*Load the current file*/
		sprintf(file, "%s%c%s", pm->dir, M4_PATH_SEPARATOR, FindData.cFileName);

		PluginLib = LoadLibrary(file);
		if (!PluginLib) goto next;

		strcpy(file, FindData.cFileName);
		while (file[strlen(file)-1] != '.') file[strlen(file)-1] = 0;
		file[strlen(file)-1] = 0;

		query_func = (QueryInterface) GetProcAddress(PluginLib, "QueryInterface");
		load_func = (LoadInterface) GetProcAddress(PluginLib, "LoadInterface");
		del_func = (ShutdownInterface) GetProcAddress(PluginLib, "ShutdownInterface");

		if (!load_func || !query_func || !del_func) {
			FreeLibrary(PluginLib);
			goto next;
		}
		
		if (is_plugin_loaded(pm, file) ) {
			FreeLibrary(PluginLib);
			goto next;
		}

		inst = malloc(sizeof(PlugInst));
		inst->lib_handle = PluginLib;
		inst->query_func = query_func;
		inst->load_func = load_func;
		inst->destroy_func = del_func;
		inst->interfaces = NewChain();
		inst->plugman = pm;
		strcpy(inst->szName, file);
		ChainAddEntry(pm->plug_list, inst);

next:
		if (!FindNextFile(SearchH, &FindData)) {
			FindClose(SearchH);
			SearchH = INVALID_HANDLE_VALUE;
		}
	}
#else /* STATIC_LIBRARY_INSTEAD_OF_DLL */
#if defined(RAW_OUT_STATIC_BUILD)
		sprintf(file, "%s", "raw_out");
		query_func = (QueryInterface) RO_QueryInterface;
		load_func = (LoadInterface) RO_LoadInterface;
		del_func = (ShutdownInterface) RO_ShutdownInterface;

		inst = malloc(sizeof(PlugInst));
		inst->lib_handle = NULL;
		inst->query_func = query_func;
		inst->load_func = load_func;
		inst->destroy_func = del_func;
		inst->interfaces = NewChain();
		inst->plugman = pm;
		strcpy(inst->szName, file);
		ChainAddEntry(pm->plug_list, inst);
#endif /* defined(RAW_OUT_STATIC_BUILD) */
#if defined(RENDER2D_STATIC_BUILD)
		sprintf(file, "%s", "render2D");
		query_func = (QueryInterface) RD_QueryInterface;
		load_func = (LoadInterface) RD_LoadInterface;
		del_func = (ShutdownInterface) RD_ShutdownInterface;

		inst = malloc(sizeof(PlugInst));
		inst->lib_handle = NULL;
		inst->query_func = query_func;
		inst->load_func = load_func;
		inst->destroy_func = del_func;
		inst->interfaces = NewChain();
		inst->plugman = pm;
		strcpy(inst->szName, file);
		ChainAddEntry(pm->plug_list, inst);
#endif /* defined(RENDER2D_STATIC_BUILD)*/
#if defined(GRAPHICS_2D_STATIC_BUILD) 
		sprintf(file, "%s", "m4_load");
		query_func = (QueryInterface) GI_QueryInterface;
		load_func = (LoadInterface) GI_LoadInterface;
		del_func = (ShutdownInterface) GI_ShutdownInterface;

		inst = malloc(sizeof(PlugInst));
		inst->lib_handle = NULL;
		inst->query_func = query_func;
		inst->load_func = load_func;
		inst->destroy_func = del_func;
		inst->interfaces = NewChain();
		inst->plugman = pm;
		strcpy(inst->szName, file);
		ChainAddEntry(pm->plug_list, inst);
#endif /* defined(GRAPHICS_2D_STATIC_BUILD)  */
#endif /* STATIC_LIBRARY_INSTEAD_OF_DLL */
	return ChainGetCount(pm->plug_list);
}

