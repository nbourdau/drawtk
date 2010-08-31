EXTRA_DIST=exports.sym

lib_LTLIBRARIES = libdrawtk.la
include_HEADERS = drawtk.h palette.h dtk_event.h

libdrawtk_la_SOURCES = drawtk.h dtk_event.h		\
			 shapes.c shapes.h		\
			 create_shape.c			\
			 texmanager.h texmanager.c	\
			 imagetex.c fonttex.h fonttex.c	\
			 window.h window.c		\
			 palette.h palette.c

libdrawtk_la_LDFLAGS = $(AM_CFLAGS) \
			 -export-symbols $(srcdir)/exports.sym  \
			 -version-info $(CURRENT):$(REVISION):$(AGE) 
