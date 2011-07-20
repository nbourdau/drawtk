lib_LTLIBRARIES = libdrawtk.la
include_HEADERS = drawtk.h dtk_colors.h dtk_event.h dtk_time.h dtk_video.h dtk_video_custom.h
 
libdrawtk_la_SOURCES = drawtk.h dtk_event.h		\
			 shapes.c shapes.h		\
			 create_shape.c			\
			 texmanager.h texmanager.c	\
			 imagetex.c fonttex.h fonttex.c	\
			 window.h window.c events.c	\
			 dtk_colors.h colors.c		\
			 dtk_time.h time.c              \
			 video_structs.h            \
			 video_custom.c dtk_video_custom.h \
			 video.c dtk_video.h

libdrawtk_la_LIBADD = $(LTLIBOBJS)

libdrawtk_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined \
			 -version-info $(CURRENT):$(REVISION):$(AGE) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = drawtk.pc

