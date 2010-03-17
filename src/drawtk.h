#ifndef FEEDBACK_H
#define FEEDBACK_H

#ifdef __cplusplus
extern "C" {
#endif 


// Handle to window
typedef struct dtk_window* dtk_hwnd;

typedef int (*EventHandlerProc)(dtk_hwnd wnd, unsigned int type, const void* data);

// Window functions
dtk_hwnd dtk_create_window(unsigned int width, unsigned int height, unsigned int x, unsigned int y, unsigned int bpp, const char* caption);
void dtk_make_current_window(dtk_hwnd wnd);
void dtk_clear_screen(dtk_hwnd wnd);
void dtk_update_screen(dtk_hwnd wnd);
void dtk_close(dtk_hwnd wnd);
void dtk_bgcolor(float* bgcolor);
void dtk_set_event_handler(dtk_hwnd wnd, EventHandlerProc handler);
int dtk_process_events(dtk_hwnd wnd);

// Image functions
typedef struct dtk_texture* dtk_htex;
dtk_htex dtk_load_image(const char* filename, unsigned int mipmap_maxlevel);


// Handle to shape structure
typedef struct dtk_shape* dtk_hshape;

// Shape creation functions
dtk_hshape dtk_create_rectangle_2p(dtk_hshape shp, float p1_x, float p1_y, float p2_x, float p2_y, int isfull, const float* color);
dtk_hshape dtk_create_rectangle_hw(dtk_hshape shp, float cx, float cy, float height, float width, int isfull, const float* color);
dtk_hshape dtk_create_triangle(dtk_hshape shp, float x1, float y1, float x2, float y2, float x3, float y3, int isfull, const float* color);
dtk_hshape dtk_create_circle(dtk_hshape shp, float cx, float cy, float r, int isfull, const float* color, unsigned int numpoints);
dtk_hshape dtk_create_shape(dtk_hshape shp, unsigned int ind_num, const float* vertex_array, int isFull, const float* color);
dtk_hshape dtk_create_line(dtk_hshape shp, float x1, float y1, float x2, float y2, const float *color);
dtk_hshape dtk_create_arrow(dtk_hshape shp, float cx, float cy, float width, float height, int isfull, const float* color);
dtk_hshape dtk_create_cross(dtk_hshape shp, float cx, float cy, float width, const float* color);
dtk_hshape dtk_create_image(dtk_hshape shp, float x, float y, float width, float height, const float* color, dtk_htex image);
dtk_hshape dtk_create_string(dtk_hshape shp, const char* str_text, float size,  float x, float y, const float* color, const char* filepath);
dtk_hshape dtk_create_composite_shape(const dtk_hshape* shp_array, unsigned int num_shp);

// Shape displacement functions
void dtk_move_shape(dtk_hshape shp, float x, float y);
void dtk_relmove_shape(dtk_hshape shp, float dx, float dy);
void dtk_rotate_shape(dtk_hshape shp, float deg);
void dtk_relrotate_shape(dtk_hshape shp, float ddeg);

// Draw a shape
void dtk_draw_shape(const dtk_hshape shp);

void dtk_bind_shape_to_window(dtk_hshape shp, const dtk_hwnd window);

// Destroy shape
void dtk_destroy_shape(dtk_hshape shp);


typedef struct dtk_keyevent* dtk_keyboard;
typedef struct dtk_mouseevent* dtk_mouse;
int dtk_poll_event(dtk_hwnd wnd, unsigned int* type,
		dtk_keyboard keyevt, dtk_mouse mouseevt);

#ifdef __cplusplus
}
#endif

#endif
