dist_man_MANS = dtk_move_shape.3 dtk_relmove_shape.3			\
		dtk_rotate_shape.3 dtk_relrotate_shape.3		\
		dtk_draw_shape.3					\
		dtk_create_shape.3 dtk_create_composite_shape.3 	\
		dtk_create_complex_shape.3				\
		dtk_create_rectangle_2p.3 dtk_create_rectangle_hw.3	\
		dtk_create_triangle.3 dtk_create_circle.3		\
		dtk_create_line.3 dtk_create_arrow.3			\
		dtk_create_cross.3 dtk_create_image.3			\
		dtk_create_string.3					\
		dtk_destroy_shape.3					\
		dtk_load_image.3 dtk_destroy_texture.3			\
		dtk_texture_getsize.3					\
		dtk_load_font.3 dtk_destroy_font.3			\
		dtk_create_window.3 dtk_close.3				\
		dtk_make_current_window.3 dtk_window_getsize.3		\
		dtk_update_screen.3 dtk_clear_screen.3 dtk_bgcolor.3	\
		dtk_process_events.3 dtk_set_event_handler.3		\
		dtk_get_color.3						\
		dtk_gettime.3 dtk_nanosleep.3				\
		dtk_addtime.3 dtk_difftime_s.3				\
		dtk_difftime_ms.3 dtk_difftime_us.3 dtk_difftime_ns.3
		
nobase_dist_doc_DATA = examples/Makefile		\
                       examples/bounce.c

AM_CPPFLAGS = $(CPPFLAGS) -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/libdrawtk.la

# Verify at least that the example compiles
check_PROGRAMS = bounce
bounce_SOURCES = examples/bounce.c


