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

#include "plugin.h"


LPPLUGMAN NewPluginManager(const unsigned char *directory, LPINIFILE config)
{
	LPPLUGMAN tmp;
	if (!directory || !strlen(directory) || (strlen(directory) > M4_MAX_PATH)) return NULL;

	tmp = malloc(sizeof(PlugMan));
	if (!tmp) return NULL;
	memset(tmp, sizeof(PlugMan), 0);
	strcpy(tmp->dir, directory);

	/*remove the final delimiter*/
	if (tmp->dir[strlen(tmp->dir)-1] == M4_PATH_SEPARATOR) tmp->dir[strlen(tmp->dir)-1] = 0;

	tmp->plug_list = NewChain();
	if (!tmp->plug_list) {
		free(tmp);
		return NULL;
	}
	tmp->cfg = config;
	PM_RefreshPlugins(tmp);
	return tmp;
}

void PM_Delete(LPPLUGMAN pm)
{
	PlugInst *inst;

	/*unload all plugins*/
	while (ChainGetCount(pm->plug_list)) {
		inst = ChainGetEntry(pm->plug_list, 0);
		PM_FreePlugin(inst);
		ChainDeleteEntry(pm->plug_list, 0);
	}
	DeleteChain(pm->plug_list);
	free(pm);
}

Bool is_plugin_loaded(LPPLUGMAN pm, unsigned char *filename) 
{
	u32 i;
	PlugInst *inst;
	for (i=0; i<ChainGetCount(pm->plug_list); i++) {
		inst = ChainGetEntry(pm->plug_list, i);
		if (!strcmp(inst->szName, filename)) return 1;
	}
	return 0;
}

u32 PM_GetPluginsCount(LPPLUGMAN pm)
{
	if (!pm) return 0;
	return ChainGetCount(pm->plug_list);
}


Bool PM_LoadInterface(LPPLUGMAN pm, u32 whichplug, u32 InterfaceFamily, void **interface_obj)
{
	PlugInst *inst;
	BaseInterface *ifce;

	if (!pm) return 0;
	inst = ChainGetEntry(pm->plug_list, whichplug);
	if (!inst) return 0;

	if (! inst->query_func(InterfaceFamily) ) return 0;
	ifce = (BaseInterface *) inst->load_func(InterfaceFamily);
	if (!ifce) return 0;

	/*sanity check*/
	if (!ifce->plugin_name || (ifce->InterfaceType != InterfaceFamily)) {
		inst->destroy_func(ifce);
		return 0;
	}
	ChainAddEntry(inst->interfaces, ifce);
	/*keep track of parent*/
	ifce->HPLUG = inst;
	*interface_obj = ifce;
	return 1;
}


Bool PM_LoadInterfaceByName(LPPLUGMAN pm, const char *plug_name, u32 InterfaceFamily, void **interface_obj)
{
	u32 i;
	BaseInterface *ifce;
	for (i=0; i<ChainGetCount(pm->plug_list); i++) {
		if (PM_LoadInterface(pm, i, InterfaceFamily, (void **) &ifce)) {
			/*check by driver name*/
			if (ifce->plugin_name && !stricmp(ifce->plugin_name, plug_name)) {
				*interface_obj = ifce;
				return 1;
			}
			/*check by file name*/
			if (!stricmp(((PlugInst *)ifce->HPLUG)->szName, plug_name)) {
				*interface_obj = ifce;
				return 1;
			}
			PM_ShutdownInterface(ifce);
		}
	}
	return 0;
}

M4Err PM_ShutdownInterface(void *interface_obj)
{
	BaseInterface *ifce;
	PlugInst *par;
	u32 i;
	if (!interface_obj) return M4BadParam;
	ifce = (BaseInterface *)interface_obj;
	par = ifce->HPLUG;

	if (!par || !ifce->InterfaceType) return M4BadParam;

	i = ChainFindEntry(par->plugman->plug_list, par);
	if (i<0) return M4BadParam;

	i = ChainFindEntry(par->interfaces, interface_obj);
	if (i<0) return M4BadParam;
	ChainDeleteEntry(par->interfaces, (u32) i);
	par->destroy_func(interface_obj);
	return M4OK;
}

/* return the plugin index given its file name*/
Bool PM_GetPluginIndex(LPPLUGMAN pm, const char *plugName, u32 *index)
{
	u32 i;
	PlugInst *inst;
	for (i=0; i<ChainGetCount(pm->plug_list); i++) {
		inst = ChainGetEntry(pm->plug_list, i);
		if (!strcmp(inst->szName, plugName)) {
			*index = i;
			return 1;
		}
	}
	return 0;
}

u32 PMI_GetKeyCount(void *interface_obj, const char *secName)
{
	BaseInterface *ifce = (BaseInterface *) interface_obj;
	if (!ifce || !ifce->HPLUG) return 0;
	return IF_GetKeyCount(((PlugInst *)ifce->HPLUG)->plugman->cfg, secName);
}

const char *PMI_GetKeyName(void *interface_obj, const char *secName, u32 keyIndex)
{
	BaseInterface *ifce = (BaseInterface *) interface_obj;
	if (!ifce || !ifce->HPLUG) return NULL;
	return IF_GetKeyName(((PlugInst *)ifce->HPLUG)->plugman->cfg, secName, keyIndex);
}

char *PMI_GetOpt(void *interface_obj, const char *secName, const char *keyName)
{
	BaseInterface *ifce = (BaseInterface *) interface_obj;
	if (!ifce || !ifce->HPLUG) return NULL;
	return IF_GetKey(((PlugInst *)ifce->HPLUG)->plugman->cfg, secName, keyName);
}

M4Err PMI_SetOpt(void *interface_obj, const char *secName, const char *keyName, const char *keyValue)
{
	BaseInterface *ifce = (BaseInterface *) interface_obj;
	if (!ifce || !ifce->HPLUG) return M4BadParam;
	return IF_SetKey(((PlugInst *)ifce->HPLUG)->plugman->cfg, secName, keyName, keyValue);
}

const char *PM_GetFileName(LPPLUGMAN pm, u32 i)
{
	PlugInst *inst = ChainGetEntry(pm->plug_list, i);
	if (!inst) return NULL;
	return inst->szName;
}

u32 PMI_GetPluginsCount(BaseInterface *parent)
{
	PlugInst*pi=parent ? parent->HPLUG : NULL;
	if (!parent || !pi || !pi->plugman) return 0;

	return PM_GetPluginsCount(pi->plugman);
}

Bool PMI_LoadInterface(BaseInterface *parent, u32 whichplug, u32 InterfaceFamily, void **interface_obj)
{
	PlugInst*pi=parent ? parent->HPLUG : NULL;
	if (!parent || !pi || !pi->plugman) return 0;

	return PM_LoadInterface(pi->plugman, whichplug, InterfaceFamily, interface_obj);
}

Bool PMI_LoadInterfaceByName(BaseInterface *parent, const char *plug_name, u32 InterfaceFamily, void **interface_obj)
{
	PlugInst*pi=parent ? parent->HPLUG : NULL;
	if (!parent || !pi || !pi->plugman) return 0;

	return PM_LoadInterfaceByName(pi->plugman, plug_name, InterfaceFamily, interface_obj);
}

