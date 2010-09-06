# - If the library source code has changed at all since the last update,
# then increment revision.
# - If any interfaces have been added, removed, or changed since the last
# update, increment current, and set revision to 0.
# - If any interfaces have been added since the last public release, then
# increment age.
# - If any interfaces have been removed since the last public release, then
# set age to 0.

m4_define([lib_current],0)
m4_define([lib_revision],0)
m4_define([lib_age],0)


# Setup Automake
AC_INIT(drawtk,[lib_current.lib_age.lib_revision],nicolas.bourdaud@epfl.ch)
AC_CONFIG_SRCDIR([src/shapes.c])
AC_CONFIG_LIBOBJ_DIR([lib])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([subdir-objects no-dist-gzip dist-bzip2 color-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CC_C99
LT_INIT([win32-dll disable-static])
AC_SUBST(LIBTOOL_DEPS)
AC_CHECK_FT2


AC_DEF_API_EXPORT_ATTRS

# Checks for libraries.
AC_SEARCH_LIBS([pthread_create], [pthread posix4], 
               [], AC_MSG_ERROR([The pthread library has not been found]))
AC_SEARCH_LIBS([cos], [m], [], AC_MSG_ERROR([The libm cannot be found.]))
AC_SEARCH_LIBS([SDL_Init], [SDL], [], AC_MSG_ERROR([The SDL library must be installed.]))
AC_SEARCH_LIBS_FNARG([FreeImage_Load], [freeimage],
        [0,0,0], [#include <FreeImage.h>],
        [], AC_MSG_ERROR([The FreeImage library must be installed.]))
AC_SEARCH_LIBS_FNARG([glEnable], [GL opengl32], 
        [0], [#include <SDL/SDL_opengl.h>],
	[], AC_MSG_ERROR([The openGL library cannot be found.]))


# Replacement checks
AC_SEARCH_LIBS([clock_gettime], [rt posix4])
AC_SEARCH_LIBS([clock_nanosleep], [rt posix4])
AC_CHECK_TYPES([struct timespec, clockid_t])
AC_CHECK_DECLS([clock_gettime, clock_nanosleep],,,[#include <time.h>])
AC_CHECK_FUNCS([nanosleep gettimeofday ftime _ftime])
AC_CHECK_FUNC_FNARG([GetSystemTimeAsFileTime], [0], [#include <windows.h>])
AC_REPLACE_FUNCS([clock_gettime clock_nanosleep])


AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])

AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tests/Makefile doc/Makefile])
AC_OUTPUT

