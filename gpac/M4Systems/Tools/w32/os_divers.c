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

static u32 clock_init = 0;

/*TODO: add queryperformancetimer stuff for win32*/
void M4_InitClock()
{
	if (!clock_init) timeBeginPeriod(1);
	clock_init += 1;
}

void M4_StopClock()
{
	if (clock_init > 0) {
		clock_init --;
		if (!clock_init) timeEndPeriod(1);
	}
}

u32 M4_GetSysClock()
{
	return timeGetTime();
}

void M4_DeleteFile(char *fileName)
{
	DeleteFile(fileName);
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


#ifndef M4_READ_ONLY
FILE *M4NewTMPFile()
{
	return tmpfile();
}
#endif

u64 f64_tell(FILE *fp)
{
	fpos_t pos;
	if (fgetpos(fp, &pos))
		return (u64) -1;
	else
		return ((u64) pos);
}

u64 f64_seek(FILE *fp, s64 offset, s32 whence)
{
	fpos_t pos;
	if (whence == SEEK_CUR) {
		fgetpos(fp, &pos);
		pos += (fpos_t) offset;
	} 
	else if (whence == SEEK_END) 
		pos = (fpos_t) (_filelengthi64(fileno(fp)) + offset);
	else if (whence == SEEK_SET)
		pos = (fpos_t) offset;
	return fsetpos(fp, &pos);
}


void M4_GetUTCTimeSince1970(u32 *sec, u32 *msec)
{
	struct _timeb	tb;
	_ftime( &tb );
	*sec = tb.time;
	*msec = tb.millitm;

}

void M4_GetUserName(char *buf, u32 buf_size)
{
	strcpy(buf, "mpeg4-user");

#if 0

	s32 len;
	char *t;

	strcpy(buf, "");
	len = 1024;
	GetUserName(buf, &len);
	if (!len) {
		t = getenv("USER");
		if (t) strcpy(buf, t);
	}
#endif
}

#ifdef W32_NEED_GETTIMEOFDAY
s32 gettimeofday(struct timeval *tp, void *tz)
{
	struct _timeb timebuffer;   

	_ftime( &timebuffer );
	tp->tv_sec  = timebuffer.time;
	tp->tv_usec = timebuffer.millitm * 1000;
	return 0;
}
#endif


/*enumerate directories*/
M4Err DIR_Enum(const char *dir, Bool enum_directory, Bool (*enum_dir_item)(void *cbck, char *item_name, char *item_path), void *cbck)
{
	unsigned char path[M4_MAX_PATH], item_path[M4_MAX_PATH];
	WIN32_FIND_DATA FindData;
	HANDLE SearchH;

	if (!dir) return M4BadParam;
	if (dir[strlen(dir) - 1] == M4_PATH_SEPARATOR) {
		sprintf(path, "%s*", dir);
	} else {
		sprintf(path, "%s%c*", dir, M4_PATH_SEPARATOR);
	}

	SearchH= FindFirstFile(path, &FindData);
	if (SearchH == INVALID_HANDLE_VALUE) return M4IOErr;

	path[strlen(path)-1] = 0;

	while (SearchH != INVALID_HANDLE_VALUE) {
		if (!strcmp(FindData.cFileName, ".")) goto next;
		if (!strcmp(FindData.cFileName, "..")) goto next;

		if (!enum_directory && (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) goto next;
		if (enum_directory && !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) goto next;

		strcpy(item_path, path);
		strcat(item_path, FindData.cFileName);
		if (enum_dir_item(cbck, FindData.cFileName, item_path)) {
			FindClose(SearchH);
			break;
		}

next:
		if (!FindNextFile(SearchH, &FindData)) {
			FindClose(SearchH);
			break;
		}
	}
	return M4OK;
}
