# - If the library source code has changed at all since the last update,
# then increment revision.
# - If any interfaces have been added, removed, or changed since the last
# update, increment current, and set revision to 0.
# - If any interfaces have been added since the last public release, then
# increment age.
# - If any interfaces have been removed since the last public release, then
# set age to 0.

m4_define([lib_current],2)
m4_define([lib_revision],0)
m4_define([lib_age],2)

m4_define([packageversion], [lib_current.lib_revision])

# Setup Automake
AC_INIT(drawtk,[packageversion],nicolas.bourdaud@gmail.com)
AC_CONFIG_SRCDIR([src/shapes.c])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([subdir-objects no-dist-gzip dist-bzip2 color-tests])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT([win32-dll disable-static])
AC_SUBST(LIBTOOL_DEPS)

AC_DEF_API_EXPORT_ATTRS

# Replacement checks
AC_SEARCH_LIBS([clock_gettime], [rt posix4])
AC_SEARCH_LIBS([clock_nanosleep], [rt posix4])
AC_CHECK_TYPES([struct timespec, clockid_t])
AC_CHECK_DECLS([clock_gettime, clock_nanosleep],,,[#include <time.h>])
AC_CHECK_FUNCS([nanosleep gettimeofday ftime _ftime])
AC_CHECK_FUNC_FNARG([GetSystemTimeAsFileTime], [0], [#include <windows.h>])
AC_REPLACE_FUNCS([clock_gettime clock_nanosleep])


# Checks for libraries.
PKG_CHECK_CUSTOM(FREETYPE2, freetype2)
PKG_CHECK_CUSTOM(FONTCONFIG, fontconfig)
PKG_CHECK_CUSTOM(GSTAPP, [gstreamer-app-0.10])

AC_SEARCH_LIBS([pthread_create], [pthread posix4], 
               [], AC_MSG_ERROR([The pthread library has not been found]))
AC_SEARCH_LIBS([cos], [m], [], AC_MSG_ERROR([The libm cannot be found.]))
AC_SEARCH_LIBS([SDL_Init], [SDL2-2.0], [], AC_MSG_ERROR([The SDL2 library must be installed.]))
AC_SEARCH_LIBS_FNARG([FreeImage_Load], [freeimage],
        [0,0,0], [#include <FreeImage.h>],
        [], AC_MSG_ERROR([The FreeImage library must be installed.]))
AC_SEARCH_LIBS_FNARG([glEnable], [GL opengl32], 
        [0], [#include <SDL/SDL_opengl.h>],
	[], AC_MSG_ERROR([The openGL library cannot be found.]))


LIBS="$LIBS $FREETYPE2_LIBS $FONTCONFIG_LIBS -lgstapp-0.10 -lgstreamer-0.10 -lgobject-2.0 -lglib-2.0"
AC_SUBST(AM_CPPFLAGS, "$CPPFLAGS $FREETYPE2_CFLAGS $GSTAPP_CFLAGS $FONTCONFIG_FLAGS")
AC_SUBST(AM_LDFLAGS, "$LDFLAGS $FREETYPE2_LDFLAGS $GSTAPP_LDFLAGS $FONTCONFIG_FLAGS")

AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile src/drawtk.pc tests/Makefile doc/Makefile])
AC_OUTPUT

