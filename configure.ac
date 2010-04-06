# If the library source code has changed at all since the last update, then
# increment revision.
# If any interfaces have been added, removed, or changed since the last update,
# increment current, and set revision to 0.
# If any interfaces have been added since the last public release, then
# increment age.  
# If any interfaces have been removed since the last public release, then set
# age to 0.
m4_define([lib_release],0)
m4_define([lib_current],0)
m4_define([lib_revision],0)
m4_define([lib_age],0)

# Setup Automake
AC_INIT(drawtk,[lib_release.lib_current.lib_age.lib_revision],serafeim.perdikis@epfl.ch)
AC_CONFIG_SRCDIR([src/shapes.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])


# Initialize Automake
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC_C99
LT_INIT([win32-dll])
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)


# Checks for libraries.
AC_SEARCH_LIBS([glEnable], [GL], [], AC_MSG_ERROR([The openGL library cannot be found.]))
AC_SEARCH_LIBS([cos], [m], [], AC_MSG_ERROR([The libm cannot be found.]))
AC_SEARCH_LIBS([SDL_Init], [SDL], [], AC_MSG_ERROR([The SDL library must be installed.]))
AC_SEARCH_LIBS([FreeImage_Load], [freeimage], [], AC_MSG_ERROR([The FreeImage library must be installed.]))

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

##########################################################################
# Windows target support
##########################################################################
case $host in
	*win32* | *mingw32* | *cygwin* | *windows*)
		os_support=win32
		AM_LDFLAGS="$AM_LDFLAGS -no-undefined"
		;;
	*)
		os_support=other
		;;
esac
##########################################################################

AC_SUBST([RELEASE],[lib_release])
AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile doc/Makefile])
AC_OUTPUT
