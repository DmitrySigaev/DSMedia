#ifndef _CYGWIN_INTTYPES_H
#define _CYGWIN_INTTYPES_H
/* /usr/include/inttypes.h for CYGWIN
 * Copyleft 2001-2002 by Felix Buenemann
 * <atmosfear at users.sourceforge.net>
 *
 * Should be installed into /usr/include
 * as inttypes.h
 */
#if 1
typedef char *  caddr_t;
typedef char int8_t;
typedef unsigned char u_int8_t;
typedef short int16_t;
typedef unsigned short u_int16_t;
typedef int int32_t;
typedef unsigned int u_int32_t;
typedef int32_t register_t;
#   ifdef CONFIG_WIN32 
       typedef signed __int64   int64_t;
       typedef unsigned __int64 u_int64_t;
#   else /* other OS */
        typedef signed long long   int64_t;
        typedef unsigned long long u_int64_t;
#   endif /* other OS */
#endif /* 0/1 */
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
#   ifdef CONFIG_WIN32 
       typedef unsigned __int64 uint64_t;
#   else /* other OS */
        typedef signed long long   int64_t;
        typedef unsigned long long uint64_t;
#   endif /* other OS */
#endif /* _CYGWIN_INTTYPES_H */

