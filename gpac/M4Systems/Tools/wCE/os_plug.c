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

static Bool ce_is_plugin_loaded(LPPLUGMAN pm, unsigned short *filename) 
{
	unsigned char szName[1024];
	u32 i;
	PlugInst *inst;
	CE_WideToChar(filename, szName);
	for (i=0; i<ChainGetCount(pm->plug_list); i++) {
		inst = ChainGetEntry(pm->plug_list, i);
		if (!strcmp(inst->szName, szName)) return 1;
	}
	return 0;
}

/*refresh all plug info - not we don't check for deleted plugins but since we've open them the OS should forbid delete*/
u32 PM_RefreshPlugins(LPPLUGMAN pm)
{
	QueryInterface query_func;
	LoadInterface load_func;
	ShutdownInterface del_func;
	PlugInst *inst;

	unsigned short w_path[M4_MAX_PATH];
	unsigned short w_file[M4_MAX_PATH];

	unsigned char path[M4_MAX_PATH];
	unsigned char file[M4_MAX_PATH];

	WIN32_FIND_DATA FindData;
	HANDLE SearchH;
	HMODULE PluginLib;
	
	if (!pm) return 0;

	sprintf(path, "%s%c*", pm->dir, M4_PATH_SEPARATOR);

	CE_CharToWide(path, w_path);
	SearchH= FindFirstFile(w_path, &FindData);

	if (SearchH == INVALID_HANDLE_VALUE) return 0;
	while (SearchH != INVALID_HANDLE_VALUE) {
	
		/*Load the current file*/
		sprintf(file, "%s%c", pm->dir, M4_PATH_SEPARATOR);

		if (!wcscmp(FindData.cFileName, _T("."))) goto next;
		if (!wcscmp(FindData.cFileName, _T("..") )) goto next;
		if (!wcsncmp(FindData.cFileName, _T("."), 1)) goto next;
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) goto next;

		MultiByteToWideChar(CP_ACP, 0, file, -1, w_file, sizeof(w_file)/sizeof(TCHAR));
		wcscat(w_file, FindData.cFileName);
		if (wcsstr(w_file, _T(".")) ) {
			while (w_file[wcslen(w_file)-1] != '.') w_file[wcslen(w_file)-1] = 0;
			w_file[wcslen(w_file)-1] = 0;
		}

		PluginLib = LoadLibrary(w_file);
		if (!PluginLib) goto next;

		query_func = (QueryInterface) GetProcAddress(PluginLib, _T("QueryInterface"));
		load_func = (LoadInterface) GetProcAddress(PluginLib, _T("LoadInterface"));
		del_func = (ShutdownInterface) GetProcAddress(PluginLib, _T("ShutdownInterface"));

		if (!load_func || !query_func || !del_func) {
			FreeLibrary(PluginLib);
			goto next;
		}
		
		if (ce_is_plugin_loaded(pm, w_file) ) {
			FreeLibrary(PluginLib);
			goto next;
		}

		wcscpy(w_file, FindData.cFileName);
		if (wcsstr(w_file, _T(".")) ) {
			while (w_file[wcslen(w_file)-1] != '.') w_file[wcslen(w_file)-1] = 0;
			w_file[wcslen(w_file)-1] = 0;
		}
		CE_WideToChar(w_file, file);

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

	return ChainGetCount(pm->plug_list);
}

