# 09-09-23  Michele Tavella <michele.tavella@epfl.ch>
AM_CFLAGS = -I$(top_srcdir)/src

noinst_PROGRAMS = colortest \
		   develtest 


develtest_SOURCES = colortest.c
develtest_LDADD = $(top_builddir)/src/libdrawtk.la

colortest_SOURCES = colortest.c
colortest_LDADD = $(top_builddir)/src/libdrawtk.la

LDADD = $(top_builddir)/src/libdrawtk.la
