dnl Process this file with autoconf to produce a configure script

dnl ------------------------------------------------
dnl Initialization and Versioning
dnl ------------------------------------------------

AC_INIT(libtheora,[1.0alpha3])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([lib/dct.c])
AM_INIT_AUTOMAKE

dnl Library versioning

V_LIB_CURRENT=0
V_LIB_REVISION=0
V_LIB_AGE=0
AC_SUBST(V_LIB_CURRENT)
AC_SUBST(V_LIB_REVISION)
AC_SUBST(V_LIB_AGE)

dnl --------------------------------------------------  
dnl Check for programs
dnl --------------------------------------------------  

dnl save $CFLAGS since AC_PROG_CC likes to insert "-g -O2"
dnl if $CFLAGS is blank
cflags_save="$CFLAGS"
AC_PROG_CC
AC_PROG_CPP
CFLAGS="$cflags_save"

dnl no shared linking for us
AM_DISABLE_SHARED
AM_ENABLE_STATIC
AM_PROG_LIBTOOL

dnl --------------------------------------------------
dnl Set build flags based on environment
dnl --------------------------------------------------

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

dnl Set some target options

cflags_save="$CFLAGS"
ldflags_save="$LDFLAGS"
if test -z "$GCC"; then
        case $host in 
        *)
                DEBUG="-g"
                CFLAGS="-O"
                PROFILE="-g -p" ;;
        esac
else

        case $host in 
        *)
                DEBUG="-g -W -D__NO_MATH_INLINES"
                CFLAGS="-O2 -Wall"
                PROFILE="-W -pg -g -O2 -fno-inline-functions";;
        esac
fi
CFLAGS="$CFLAGS $cflags_save"
LDFLAGS="$LDFLAGS $ldflags_save"

dnl --------------------------------------------------
dnl Check for headers
dnl --------------------------------------------------

dnl none here

dnl --------------------------------------------------
dnl Check for typedefs, structures, etc
dnl --------------------------------------------------

dnl none

dnl --------------------------------------------------
dnl Check for library functions
dnl --------------------------------------------------

dnl substitue the included getopt if the system doesn't support long options
AC_CHECK_FUNC(getopt_long, [GETOPT_OBJS=''], [GETOPT_OBJS='getopt.$(OBJEXT) getopt1.$(OBJEXT)'])
AC_SUBST(GETOPT_OBJS)

XIPH_PATH_OGG(, AC_MSG_ERROR([
    libogg is required to build this package!
    please see http://www.xiph.org/ for how to
    obtain a copy.
]))
CFLAGS="$CFLAGS $OGG_CFLAGS"
LIBS="$LIBS $OGG_LIBS"
AC_CHECK_FUNC(oggpackB_read, , [
  AC_MSG_ERROR([newer libogg version (>1.0) required])
])

AM_PATH_SDL(,[
	HAVE_SDL=yes
	SDL_LIBS=`$SDL_CONFIG --libs`
],AC_MSG_WARN([*** Unable to find SDL -- Not compiling example players ***]))

AC_CHECK_HEADERS([sys/soundcard.h soundcard.h machine/soundcard.h],[
  HAVE_OSS=yes
  break
],AC_MSG_WARN([OSS audio support not found -- not compiling player_example]))

if test x$HAVE_SDL = xyes -a x$HAVE_OSS = xyes; then
  BUILDABLE_EXAMPLES="$BUILDABLE_EXAMPLES player_example"
fi
AC_SUBST(BUILDABLE_EXAMPLES)

dnl --------------------------------------------------
dnl Do substitutions
dnl --------------------------------------------------

LIBS="$LIBS"

AC_SUBST(LIBS)
AC_SUBST(DEBUG)
AC_SUBST(PROFILE)

AC_CONFIG_FILES([
  Makefile lib/Makefile
  include/Makefile include/theora/Makefile
  examples/Makefile doc/Makefile
  debian/Makefile
])
AC_OUTPUT
