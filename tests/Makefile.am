AM_CFLAGS = -I$(top_srcdir)/src
EXTRA_DIST=navy.png font.png

check_PROGRAMS = test1 test-events

test1_LDADD = $(top_builddir)/src/libdrawtk.la $(LTLIBOBJS)
test_events_LDADD = $(top_builddir)/src/libdrawtk.la $(LTLIBOBJS)

TESTS = test1 test-events
