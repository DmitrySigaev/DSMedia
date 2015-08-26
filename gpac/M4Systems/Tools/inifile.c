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

#define MAX_SECTION_NAME		500
#define MAX_INI_LINE			2046

typedef struct
{
	char name[MAX_SECTION_NAME];
	char value[MAX_INI_LINE];
} IniKey;

typedef struct
{
	char section_name[MAX_SECTION_NAME];
	Chain *keys;
} IniSection;

struct _iniFile
{
	char *fileName;
	char *filePath;
	Chain *sections;
	Bool hasChanged;
} INIFILE;

LPINIFILE NewIniFile(const char *filePath, const char* file_name)
{
	IniSection *p;
	IniKey *k;
	u32 i;
	FILE *file;
	char *ret;
	char fileName[M4_MAX_PATH];
	char line[MAX_INI_LINE];
	LPINIFILE tmp;

	if (filePath) {
		if (filePath[strlen(filePath)-1] == M4_PATH_SEPARATOR) {
			strcpy(fileName,filePath);
			strcat(fileName, file_name);
		} else {
			sprintf(fileName, "%s%c%s", filePath, M4_PATH_SEPARATOR, file_name);
		}
	} else {
		strcpy(fileName,file_name);
	}
	file = fopen(fileName, "rt");
	if (!file) return NULL;

	tmp = malloc(sizeof(INIFILE));
	memset(tmp, 0, sizeof(INIFILE));

	tmp->filePath = malloc(sizeof(char) * (strlen(filePath)+1));
	strcpy(tmp->filePath, filePath ? filePath : "");
	tmp->fileName = malloc(sizeof(char) * (strlen(fileName)+1));
	strcpy(tmp->fileName, fileName);
	tmp->sections = NewChain();

	//load the file
	p = NULL;

	while (!feof(file)) {
		ret = fgets(line, MAX_INI_LINE, file);

		if (!ret) continue;
		if (!strlen(line)) continue;
		if (line[0] == '#') continue;

		//get rid of the end of line stuff
		while ((strlen(line) > 0) && ((line[strlen(line)-1] == '\n') || (line[strlen(line)-1] == '\r')) )
			line[strlen(line)-1] = 0;

		
		//new section
		if (line[0] == '[') {
			p = malloc(sizeof(IniSection));
			p->keys = NewChain();
			strcpy(p->section_name, line + 1);
			p->section_name[strlen(line) - 2] = 0;
			while (p->section_name[strlen(p->section_name) - 1] == ']' || p->section_name[strlen(p->section_name) - 1] == ' ') p->section_name[strlen(p->section_name) - 1] = 0;
			ChainAddEntry(tmp->sections, p);
		}
		else if (strlen(line) && (strchr(line, '=') != NULL) ) {
			if (!p) {
				DeleteChain(tmp->sections);
				free(tmp->fileName);
				free(tmp->filePath);
				free(tmp);
				fclose(file);
				return NULL;
			}
			k = malloc(sizeof(IniKey));
			i = 0;
			while (line[i] != '=') {
				k->name[i] = line[i];
				i++;
			}
			k->name[i] = 0;
			while (k->name[strlen(k->name) - 1] == ' ') k->name[strlen(k->name) - 1] = 0;
			i++;
			while (line[i] == ' ') i++;
			strcpy(k->value, line + i);
			while (k->value[strlen(k->value) - 1] == ' ') k->value[strlen(k->value) - 1] = 0;
			ChainAddEntry(p->keys, k);
		}
	}
	fclose(file);
	return tmp;
}

void DelSection(IniSection *ptr)
{
	IniKey *k;
	if (!ptr) return;

	while (ChainGetCount(ptr->keys)) {
		k = ChainGetEntry(ptr->keys, 0);
		free(k);
		ChainDeleteEntry(ptr->keys, 0);
	}
	DeleteChain(ptr->keys);
	free(ptr);
}


M4Err WriteIniFile(LPINIFILE iniFile)
{
	u32 i, j;
	IniSection *sec;
	IniKey *key;
	FILE *file;

	if (!iniFile->hasChanged) return M4OK;

	file = fopen(iniFile->fileName, "wt");
	if (!file) return M4IOErr;

	for (i=0; i<ChainGetCount(iniFile->sections); i++) {
		sec = ChainGetEntry(iniFile->sections, i);
		fprintf(file, "[%s]\n", sec->section_name);
		for (j=0; j<ChainGetCount(sec->keys); j++) {
			key = ChainGetEntry(sec->keys, j);
			fprintf(file, "%s=%s\n", key->name, key->value);
		}
		//end of section
		fprintf(file, "\n");
	}
	fclose(file);
	return M4OK;
}

void IF_Delete(LPINIFILE iniFile)
{
	IniSection *p;
	if (!iniFile) return;

	WriteIniFile(iniFile);
	while (ChainGetCount(iniFile->sections)) {
		p = ChainGetEntry(iniFile->sections, 0);
		DelSection(p);
		ChainDeleteEntry(iniFile->sections, 0);
	}
	DeleteChain(iniFile->sections);
	free(iniFile->fileName);
	free(iniFile->filePath);
	free(iniFile);
}

char *IF_GetPath(LPINIFILE iniFile)
{
	return iniFile->filePath;
}


char *IF_GetKey(LPINIFILE iniFile, const char *secName, const char *keyName)
{
	u32 i;
	IniSection *sec;
	IniKey *key;

	for (i=0; i<ChainGetCount(iniFile->sections); i++) {
		sec = ChainGetEntry(iniFile->sections, i);
		if (!strcmp(secName, sec->section_name)) goto get_key;
	}
	return NULL;

get_key:

	for (i=0; i<ChainGetCount(sec->keys); i++) {
		key = ChainGetEntry(sec->keys, i);
		if (!strcmp(key->name, keyName)) return key->value;
	}
	return NULL;
}



M4Err IF_SetKey(LPINIFILE iniFile, const char *secName, const char *keyName, const char *keyValue)
{
	u32 i;
	IniSection *sec;
	IniKey *key;

	if (!iniFile) return M4BadParam;

	for (i=0; i<ChainGetCount(iniFile->sections); i++) {
		sec = ChainGetEntry(iniFile->sections, i);
		if (!strcmp(secName, sec->section_name)) goto get_key;
	}
	//need a new key
	sec = malloc(sizeof(IniSection));
	strcpy(sec->section_name, secName);
	sec->keys = NewChain();
	iniFile->hasChanged = 1;
	ChainAddEntry(iniFile->sections, sec);

get_key:

	for (i=0; i<ChainGetCount(sec->keys); i++) {
		key = ChainGetEntry(sec->keys, i);
		if (!strcmp(key->name, keyName)) goto set_value;
	}
	//need a new key
	key = malloc(sizeof(IniKey));
	strcpy(key->name, keyName);
	strcpy(key->value, "");
	iniFile->hasChanged = 1;
	ChainAddEntry(sec->keys, key);

set_value:
	//same value, don't update
	if (!strcmp(key->value, keyValue)) return M4OK;

	strcpy(key->value, keyValue);
	iniFile->hasChanged = 1;
	return M4OK;
}

u32 IF_GetSectionCount(LPINIFILE iniFile)
{
	return ChainGetCount(iniFile->sections);
}
const char *IF_GetSectionName(LPINIFILE iniFile, u32 secIndex)
{
	IniSection *is = ChainGetEntry(iniFile->sections, secIndex);
	if (!is) return NULL;
	return is->section_name;
}
u32 IF_GetKeyCount(LPINIFILE iniFile, const char *secName)
{
	u32 i;
	IniSection *sec;
	for (i=0; i<ChainGetCount(iniFile->sections); i++) {
		sec = ChainGetEntry(iniFile->sections, i);
		if (!strcmp(secName, sec->section_name)) return ChainGetCount(sec->keys);
	}
	return 0;
}

const char *IF_GetKeyName(LPINIFILE iniFile, const char *secName, u32 keyIndex)
{
	u32 i;
	IniSection *sec;
	for (i=0; i<ChainGetCount(iniFile->sections); i++) {
		sec = ChainGetEntry(iniFile->sections, i);
		if (!strcmp(secName, sec->section_name)) {
			IniKey *key = ChainGetEntry(sec->keys, keyIndex);
			return key ? key->name : NULL;
		}
	}
	return NULL;
}

