#ifndef FEEDBACK_H
#define FEEDBACK_H

#ifdef __cplusplus
extern "C" {
#endif 


// Handle to feedback window
typedef struct fb_window* fb_hwnd;

typedef int (*EventHandlerProc)(fb_hwnd wnd, unsigned int type, const void* data);

// Window functions
fb_hwnd fb_create_window(unsigned int width, unsigned int height, unsigned int x, unsigned int y, unsigned int bpp, const char* caption);
void fb_make_current_window(fb_hwnd wnd);
void fb_clear_screen(fb_hwnd wnd);
void fb_update_screen(fb_hwnd wnd);
void fb_close(fb_hwnd wnd);
void fb_bgcolor(float* bgcolor);
void fb_set_event_handler(fb_hwnd wnd, EventHandlerProc handler);
int fb_process_events(fb_hwnd wnd);

// Image functions
typedef struct fb_texture* fb_htex;
fb_htex fb_load_image(const char* filename, unsigned int mipmap_maxlevel);


// Handle to shape structure
typedef struct fb_shape* fb_hshape;

// Shape creation functions
fb_hshape fb_create_rectangle_2p(fb_hshape shp, float p1_x, float p1_y, float p2_x, float p2_y, int isfull, const float* color);
fb_hshape fb_create_rectangle_hw(fb_hshape shp, float cx, float cy, float height, float width, int isfull, const float* color);
fb_hshape fb_create_triangle(fb_hshape shp, float x1, float y1, float x2, float y2, float x3, float y3, int isfull, const float* color);
fb_hshape fb_create_circle(fb_hshape shp, float cx, float cy, float r, int isfull, const float* color, unsigned int numpoints);
fb_hshape fb_create_shape(fb_hshape shp, unsigned int ind_num, const float* vertex_array, int isFull, const float* color);
fb_hshape fb_create_line(fb_hshape shp, float x1, float y1, float x2, float y2, const float *color);
fb_hshape fb_create_arrow(fb_hshape shp, float cx, float cy, float width, float height, int isfull, const float* color);
fb_hshape fb_create_cross(fb_hshape shp, float cx, float cy, float width, const float* color);
fb_hshape fb_create_image(fb_hshape shp, float x, float y, float width, float height, const float* color, fb_htex image);
fb_hshape fb_create_string(fb_hshape shp, const char* str_text, float size,  float x, float y, const float* color, const char* filepath);
fb_hshape fb_create_composite_shape(const fb_hshape* shp_array, unsigned int num_shp);

// Shape displacement functions
void fb_move_shape(fb_hshape shp, float x, float y);
void fb_relmove_shape(fb_hshape shp, float dx, float dy);
void fb_rotate_shape(fb_hshape shp, float deg);
void fb_relrotate_shape(fb_hshape shp, float ddeg);

// Draw a shape
void fb_draw_shape(const fb_hshape shp);

void fb_bind_shape_to_window(fb_hshape shp, const fb_hwnd window);

// Destroy shape
void fb_destroy_shape(fb_hshape shp);


typedef struct fb_keyevent* fb_keyboard;
typedef struct fb_mouseevent* fb_mouse;
int fb_poll_event(fb_hwnd wnd, unsigned int* type,
		fb_keyboard keyevt, fb_mouse mouseevt);

#ifdef __cplusplus
}
#endif

#endif
