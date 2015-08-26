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

	if (inst->lib_handle) dlclose(inst->lib_handle);

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
	unsigned char file[M4_MAX_PATH];

	DIR *the_dir;
	struct dirent* the_file;
	s32 _flags;
	struct stat st;
	void *PluginLib;
	
	if (!pm) return 0;

	the_dir = opendir(pm->dir);
	if (the_dir == NULL) return 0;

	the_file = readdir(the_dir);
	while (the_file) {

		/*Load the current file*/
		sprintf(file, "%s%c", pm->dir, M4_PATH_SEPARATOR);

		if (!strcmp(the_file->d_name, "..")) goto next;
		if (the_file->d_name[0] == '.') goto next;

		strcat(file, the_file->d_name);
		/*filter directories*/
		if (stat(file, &st ) != 0) goto next;
		if ( (st.st_mode & S_IFMT) == S_IFDIR) goto next;

#ifdef RTLD_GLOBAL
		_flags =RTLD_LAZY | RTLD_GLOBAL;
#else
		_flags =RTLD_LAZY;
#endif
	
		PluginLib = dlopen(file, _flags);
		if (!PluginLib) goto next;

		query_func = (QueryInterface) dlsym(PluginLib, "QueryInterface");		
		load_func = (LoadInterface) dlsym(PluginLib, "LoadInterface");		
		del_func = (ShutdownInterface) dlsym(PluginLib, "ShutdownInterface");		

		if (!load_func || !query_func || !del_func) {
			dlclose(PluginLib);
			goto next;
		}

		if (is_plugin_loaded(pm, file) ) {
			dlclose(PluginLib);
			goto next;
		}
		inst = malloc(sizeof(PlugInst));
		inst->lib_handle = PluginLib;
		inst->query_func = query_func;
		inst->load_func = load_func;
		inst->destroy_func = del_func;
		inst->interfaces = NewChain();
		inst->plugman = pm;
		strcpy(inst->szName, the_file->d_name);
		ChainAddEntry(pm->plug_list, inst);

next:
		the_file = readdir(the_dir);
	}

	closedir(the_dir);

	return ChainGetCount(pm->plug_list);
}

