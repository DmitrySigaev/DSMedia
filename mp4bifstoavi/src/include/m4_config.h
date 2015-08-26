/*
 *			GPAC - MPEG-4 Systems C Development Kit
 *
 *			Copyright (c) Jean Le Feuvre 2000-2003 
 *					All rights reserved
 *
 *  This file is part of GPAC / general OS configuration file
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

#ifndef _M4_CONFIG_H
#define _M4_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


/*WIN32 and WinCE config*/
#if defined(WIN32) || defined(_WIN32_WCE)

/*common win32 parts*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

typedef unsigned __int64 u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned int Bool;
typedef __int64 s64;
typedef int s32;
typedef short s16;
typedef char s8;

#if defined(__GNUC__)
#define M4INLINE inline
#else
#define M4INLINE __inline
#endif

#define M4_PATH_SEPARATOR	'\\'
#define M4_MAX_PATH	MAX_PATH

/*WINCE config*/
#if defined(_WIN32_WCE)

#include <winsock.h>

/*winCE read-only (smaller)*/
#ifndef M4_READ_ONLY
#define M4_READ_ONLY
#endif

/*win32 assert*/
void CE_Assert(u32 valid);
#define assert( t )	CE_Assert((unsigned int) (t) )

/*performs wide->char and char->wide conversion on a buffer M4_MAX_PATH long*/
void CE_WideToChar(unsigned short *w_str, char *str);
void CE_CharToWide(char *str, unsigned short *w_str);


#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupr _strupr

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define PTRDIFF(p1, p2, type)	((p1) - (p2))
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef offsetof
#define offsetof(s,m) ((size_t)&(((s*)0)->m))
#endif

#ifndef getenv
#define getenv(a) 0L
#endif

#ifndef strupr
char * my_str_upr(char *str);
#define strupr my_str_upr
#endif

#ifndef strlwr
char * my_str_lwr(char *str);
#define strlwr my_str_lwr
#endif


#define f64_tell(__f)	(u64) ftell(__f)
#define f64_seek(__f, __pos, __whence)	(u64) fseek(__f, (s32) __pos, __whence)

#else	/*END WINCE*/

/*WIN32 not-WinCE*/
#include <time.h>
#include <assert.h>
#include <sys/timeb.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

u64 f64_tell(FILE *f);
u64 f64_seek(FILE *f, s64 pos, s32 whence);

#endif	/*END WIN32 non win-ce*/

#ifndef gettimeofday
#define W32_NEED_GETTIMEOFDAY
int gettimeofday(struct timeval *tp, void *tz);
#endif

#else	/*end WIN32 config*/

/*UNIX likes*/

/*force large file support*/
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <string.h>


#include <inttypes.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <syslog.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <dirent.h>

#ifndef __BEOS__
#include <errno.h>
#else
#include "berrno.h"
#endif

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t Bool;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

#define M4INLINE	inline

void linux_sleep(u32 ms);

/*sorry this was developed under w32 :)*/
#define stricmp		strcasecmp
#define strnicmp	strncasecmp

#ifndef strupr
char * my_str_upr(char *str);
#define strupr my_str_upr
#endif

#ifndef strlwr
char * my_str_lwr(char *str);
#define strlwr my_str_lwr
#endif


#ifdef Sleep
#undef Sleep
#endif
#define Sleep linux_sleep

#define M4_PATH_SEPARATOR	'/'

#ifdef PATH_MAX
#define M4_MAX_PATH	PATH_MAX
#else
/*PATH_MAX not defined*/
#define M4_MAX_PATH	1023
#endif

#define f64_tell(__f)	(u64) ftello64(__f)
#define f64_seek(__f, __pos, __whence)	fseeko64(__f, (off64_t) __pos, __whence)

#ifdef fopen
#undef fopen
#define fopen fopen64
#endif

#endif

/*define what's missing*/
#ifndef NULL
#define NULL 0
#endif



#ifdef __cplusplus
}
#endif

#endif
