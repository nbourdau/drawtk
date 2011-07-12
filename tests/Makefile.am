AM_CFLAGS = -I$(top_srcdir)/src
EXTRA_DIST=navy.png test.ttf

#check_PROGRAMS = test1 test-events test-gst test-gst-tcp test-gst-udp test-gst-file test-gst-dtk

check_PROGRAMS = test-gst-dtk

test1_LDADD = $(top_builddir)/src/libdrawtk.la
test_events_LDADD = $(top_builddir)/src/libdrawtk.la
test_gst_LDADD = $(top_builddir)/src/libdrawtk.la
test_gst_tcp_LDADD = $(top_builddir)/src/libdrawtk.la
test_gst_udp_LDADD = $(top_builddir)/src/libdrawtk.la
test_gst_file_LDADD = $(top_builddir)/src/libdrawtk.la
test_gst_dtk_LDADD = $(top_builddir)/src/libdrawtk.la

TESTS = test-gst-dtk
