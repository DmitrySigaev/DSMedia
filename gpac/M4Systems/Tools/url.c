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

#include <gpac/m4_tools.h>


/* the length of the URL separator ("://" || "|//") */
#define URL_SEP_LENGTH	3


/*resolve the protocol type, for a std URL: http:// or ftp:// ...*/
u32 URL_GetProtocolType(const char *pathName)
{
	char *begin;
	if (!pathName) return URL_TYPE_ANY;

	if ((pathName[0] == '/') 
		|| (pathName[1] == ':') 
		|| ((pathName[0] == ':') && (pathName[1] == ':'))
		) return URL_TYPE_FILE;

	begin = strstr(pathName, "://");
	if (!begin) begin = strstr(pathName, "|//"); 
	if (!begin) return URL_TYPE_RELATIVE;
	if (!strnicmp(pathName, "file", 4)) return URL_TYPE_FILE;
	return URL_TYPE_ANY;
}

char *URL_GetAbsoluteFilePath(const char *pathName, const char *parentPath)
{
	char *outPath;
	u32 prot_type;

	prot_type = URL_GetProtocolType(pathName);

	/*abs path name*/
	if (prot_type == URL_TYPE_FILE) {
		u32 offset = URL_SEP_LENGTH + 4;
		if (!strstr(pathName, "://") && !strstr(pathName, "|//")) return strdup(pathName);
		/*not sure if "file:///C:\..." is std, but let's handle it anyway*/
		if (strstr(pathName, ":///") || strstr(pathName, "|///")) {
			if (pathName[offset+2]==':') offset += 1;
		}
		outPath = (char *) malloc(strlen(pathName) - offset + 1);
		strcpy(outPath, pathName + offset);
		return outPath;
	}
	if (prot_type==URL_TYPE_ANY) return NULL;
	if (!parentPath) return strdup(pathName);

	/*try with the parent URL*/
	prot_type = URL_GetProtocolType(parentPath);
	/*if abs parent path concatenate*/
	if (prot_type == URL_TYPE_FILE) return URL_Concatenate(parentPath, pathName);
	if (prot_type != URL_TYPE_RELATIVE) return NULL;
	/*if we are here, parentPath is also relative... return the original PathName*/
	return strdup(pathName);
}


char *URL_Concatenate(const char *parentName, const char *pathName)
{
	u32 pathSepCount, i, prot_type;
	char psep;
	char *outPath, *name;
	char tmp[M4_MAX_PATH];

	if (!pathName || !parentName) return NULL;

	if ( (strlen(parentName) > M4_MAX_PATH) || (strlen(pathName) > M4_MAX_PATH) ) return NULL;

	prot_type = URL_GetProtocolType(pathName);
	if (prot_type != URL_TYPE_RELATIVE) return strdup(pathName);
	/*we need abs path for parent*/
	prot_type = URL_GetProtocolType(parentName);
	if (prot_type == URL_TYPE_RELATIVE) return strdup(pathName);

	pathSepCount = 0;
	name = NULL;
	if (pathName[0] == '.') {
		for (i = 0; i< strlen(pathName) - 2; i++) {
			/*current dir*/
			if ( (pathName[i] == '.') 
				&& ( (pathName[i+1] == M4_PATH_SEPARATOR) || (pathName[i+1] == '/') ) )
				continue;
			/*parent dir*/
			if ( (pathName[i] == '.') && (pathName[i+1] == '.') 
				&& ( (pathName[i+2] == M4_PATH_SEPARATOR) || (pathName[i+2] == '/') )
				) {
				pathSepCount ++;
				i+=2;
			} else {
				name = (char *) &pathName[i];
				break;
			}
		}
	}
	if (!name) name = (char *) pathName;

	strcpy(tmp, parentName);
	for (i = strlen(parentName); i > 0; i--) {
		//break our path at each separator
		if ((parentName[i-1] == M4_PATH_SEPARATOR) || (parentName[i-1] == '/'))  {
			tmp[i-1] = 0;
			if (!pathSepCount) break;
			pathSepCount--;
		}
	}
	//if i==0, the parent path was relative, just return the pathName
	if (!i) return strdup(pathName);

	psep = (prot_type == URL_TYPE_FILE) ? M4_PATH_SEPARATOR : '/';

	outPath = (char *) malloc(strlen(tmp) + strlen(name) + 2);
	sprintf(outPath, "%s%c%s", tmp, psep, name);

	/*cleanup paths sep for win32*/
	if ((prot_type == URL_TYPE_FILE) && (M4_PATH_SEPARATOR != '/')) {
		for (i = 0; i<strlen(outPath); i++) 
			if (outPath[i]=='/') outPath[i] = M4_PATH_SEPARATOR;
	}
	return outPath;
}
