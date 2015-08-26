/***
*time.h - definitions/declarations for time routines
*
*       Copyright (c) 1985-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file has declarations of time routines and defines
*       the structure returned by the localtime and gmtime routines and
*       used by asctime.
*       [ANSI/System V]
*
*       [Public]
*
****/

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TIME
#define _INC_TIME

#if     !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifdef  _MSC_VER
/*
 * Currently, all MS C compilers for Win32 platforms default to 8 byte
 * alignment.
 */
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef  __cplusplus
extern "C" {
#endif



/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif

#ifndef _MAC
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif  /* ndef _MAC */

/* Define the implementation defined time type */

#ifndef _TIME_T_DEFINED
typedef long time_t;        /* time value */
#define _TIME_T_DEFINED     /* avoid multiple def's of time_t */
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif


/* Define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#ifndef _TM_DEFINED
struct tm {
        int tm_sec;     /* seconds after the minute - [0,59] */
        int tm_min;     /* minutes after the hour - [0,59] */
        int tm_hour;    /* hours since midnight - [0,23] */
        int tm_mday;    /* day of the month - [1,31] */
        int tm_mon;     /* months since January - [0,11] */
        int tm_year;    /* years since 1900 */
        int tm_wday;    /* days since Sunday - [0,6] */
        int tm_yday;    /* days since January 1 - [0,365] */
        int tm_isdst;   /* daylight savings time flag */
        };
#define _TM_DEFINED
#endif


/* Clock ticks macro - ANSI version */

#define CLOCKS_PER_SEC  1000


/* Extern declarations for the global variables used by the ctime family of
 * routines.
 */

/* non-zero if daylight savings time is used */
_CRTIMP extern int _daylight;

/* offset for Daylight Saving Time */
_CRTIMP extern long _dstbias;

/* difference in seconds between GMT and local time */
_CRTIMP extern long _timezone;

/* standard/daylight savings time zone names */
_CRTIMP extern char * _tzname[2];


/* Function prototypes */

_CRTIMP char * __cdecl asctime(const struct tm *);
_CRTIMP char * __cdecl ctime(const time_t *);
_CRTIMP clock_t __cdecl clock(void);
_CRTIMP double __cdecl difftime(time_t, time_t);
_CRTIMP struct tm * __cdecl gmtime(const time_t *);
_CRTIMP struct tm * __cdecl localtime(const time_t *);
_CRTIMP time_t __cdecl mktime(struct tm *);
_CRTIMP size_t __cdecl strftime(char *, size_t, const char *,
        const struct tm *);
_CRTIMP char * __cdecl _strdate(char *);
_CRTIMP char * __cdecl _strtime(char *);
_CRTIMP time_t __cdecl time(time_t *);

#ifdef  _POSIX_
_CRTIMP void __cdecl tzset(void);
#else
_CRTIMP void __cdecl _tzset(void);
#endif


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _MAC
#ifndef _WTIME_DEFINED

/* wide function prototypes, also declared in wchar.h */
 
_CRTIMP wchar_t * __cdecl _wasctime(const struct tm *);
_CRTIMP wchar_t * __cdecl _wctime(const time_t *);
_CRTIMP size_t __cdecl wcsftime(wchar_t *, size_t, const wchar_t *,
        const struct tm *);
_CRTIMP wchar_t * __cdecl _wstrdate(wchar_t *);
_CRTIMP wchar_t * __cdecl _wstrtime(wchar_t *);

#define _WTIME_DEFINED
#endif
#endif  /* ndef _MAC */


#if     !__STDC__ || defined(_POSIX_)

/* Non-ANSI names for compatibility */

#define CLK_TCK  CLOCKS_PER_SEC

_CRTIMP extern int daylight;
_CRTIMP extern long timezone;
_CRTIMP extern char * tzname[2];

_CRTIMP void __cdecl tzset(void);

#endif  /* __STDC__ */


#ifdef  __cplusplus
}
#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_TIME */
