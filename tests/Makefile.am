AM_CFLAGS = -I$(top_srcdir)/src
EXTRA_DIST=navy.png test.ogv test.ttf

check_PROGRAMS = test1 test-events test-video test-video-custom

test1_LDADD = $(top_builddir)/src/libdrawtk.la
test_events_LDADD = $(top_builddir)/src/libdrawtk.la
test_video_LDADD = $(top_builddir)/src/libdrawtk.la
test_video_custom_LDADD = $(top_builddir)/src/libdrawtk.la

TESTS = test1 test-events test-video test-video-custom
