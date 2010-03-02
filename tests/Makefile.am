AM_CFLAGS = -I$(top_srcdir)/src
EXTRA_DIST=navy.png font.png

check_PROGRAMS = test1 test-events

test1_LDADD = $(top_builddir)/src/libfeedback.la
test_events_LDADD = $(top_builddir)/src/libfeedback.la

TESTS = test1 test-events
