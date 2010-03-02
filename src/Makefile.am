lib_LTLIBRARIES = libfeedback.la

libfeedback_includedir = $(includedir)/libfeedback
libfeedback_include_HEADERS = feedback.h palette.h fb_event.h

libfeedback_la_SOURCES = feedback.h fb_event.h\
						 shapes.c\
						 texmanager.h texmanager.c\
						 window.h window.c\
						 palette.h palette.c

libfeedback_la_LDFLAGS = $(AM_CFLAGS) \
						 -version-info $(CURRENT):$(REVISION):$(AGE) \
						 -release $(RELEASE) 
libfeedback_la_LIBADD = -lm -lGL
