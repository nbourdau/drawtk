lib_LTLIBRARIES = libdrawtk.la
include_HEADERS = drawtk.h dtk_colors.h dtk_event.h dtk_time.h

libdrawtk_la_SOURCES = drawtk.h dtk_event.h		\
			 shapes.c shapes.h		\
			 create_shape.c			\
			 texmanager.h texmanager.c	\
			 imagetex.c fonttex.h fonttex.c	\
			 window.h window.c events.c	\
			 dtk_colors.h colors.c		\
			 dtk_time.h time.c

libdrawtk_la_LIBADD = $(LTLIBOBJS)

libdrawtk_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined \
			 -version-info $(CURRENT):$(REVISION):$(AGE) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = drawtk.pc

