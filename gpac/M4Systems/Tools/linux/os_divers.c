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


#define SLEEP_ABS_SELECT		1

void M4_InitClock() {}

void M4_StopClock() {}

u32 M4_GetSysClock()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ( (now.tv_sec)*1000 + (now.tv_usec) / 1000);
}

void linux_sleep(u32 ms)
{
	s32 sel_err;
	struct timeval tv;

#ifndef SLEEP_ABS_SELECT
	u32 prev, now, elapsed;
#endif

#ifdef SLEEP_ABS_SELECT
	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;
#else
	prev = M4_GetSysClock();
#endif

	do {
		errno = 0;

#ifndef SLEEP_ABS_SELECT
		now = M4_GetSysClock();
		elapsed = (now - prev);
		if ( elapsed >= ms ) {
			break;
		}
		prev = now;
		ms -= elapsed;
		tv.tv_sec = ms/1000;
		tv.tv_usec = (ms%1000)*1000;
#endif

		sel_err = select(0, NULL, NULL, NULL, &tv);
	} while ( sel_err && (errno == EINTR) );
}




void M4_DeleteFile(char *fileName)
{
	remove(fileName);
}


#ifndef M4_READ_ONLY
FILE *M4NewTMPFile()
{
	return tmpfile(); 
}
#endif



void M4_GetUTCTimeSince1970(u32 *sec, u32 *msec)
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	*sec = tp.tv_sec;
	*msec = tp.tv_usec*1000;
}

void M4_RandInit(Bool Reset)
{
	if (Reset) {
		srand(1);
	} else {
		srand( (u32) time(NULL) );
	}
}

u32 M4_Rand()
{
	return rand();
}



void M4_GetUserName(char *buf, u32 buf_size)
{
	strcpy(buf, "mpeg4-user");

#if 0
	struct passwd *pw;
	pw = getpwuid(getuid());
	strcpy(buf, "");
	if (pw && pw->pw_name) strcpy(name, pw->pw_name);
#endif

}



/*enumerate directories*/
M4Err DIR_Enum(const char *dir, Bool enum_directory, Bool (*enum_dir_item)(void *cbck, char *item_name, char *item_path), void *cbck)
{
	unsigned char ext[2];
	unsigned char path[M4_MAX_PATH];
	unsigned char filepath[M4_MAX_PATH];
	DIR *the_dir;
	struct dirent* the_file;
	struct stat st;

	if (!dir) return M4BadParam;

	strcpy(path, dir);
	if (path[strlen(path)-1] != M4_PATH_SEPARATOR) {
		ext[0] = M4_PATH_SEPARATOR;
		ext[1] = 0;
		strcat(path, ext);
	}


	the_dir = opendir(path);
	if (the_dir == NULL) {
		return M4IOErr;
	}

	the_file = readdir(the_dir);
	while (the_file) {
		if (!strcmp(the_file->d_name, "..")) goto next;
		if (the_file->d_name[0] == '.') goto next;

		strcpy(filepath, path);
		strcat(filepath, the_file->d_name);

		if (stat( filepath, &st ) != 0) {
			printf("stat err %s\n", filepath);
			goto next;
		}
		if (enum_directory && ( (st.st_mode & S_IFMT) != S_IFDIR)) goto next;
		if (!enum_directory && ((st.st_mode & S_IFMT) == S_IFDIR)) goto next;

		if (enum_dir_item(cbck, the_file->d_name, filepath)) break;

next:
		the_file = readdir(the_dir);
	}
	closedir(the_dir);
	return M4OK;
}


char * my_str_upr(char *str)
{
	u32 i;
	for (i=0; i<strlen(str); i++) {
		str[i] = toupper(str[i]);
	}
	return str;
}
char * my_str_lwr(char *str)
{
	u32 i;
	for (i=0; i<strlen(str); i++) {
		str[i] = tolower(str[i]);
	}
	return str;
}
