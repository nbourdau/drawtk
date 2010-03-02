#ifndef WINDOW_H
#define WINDOW_H

#include "texmanager.h"

struct fb_window;
struct fb_texture_manager* get_texmanager(struct fb_window* wnd);

extern struct fb_window* current_window;

#endif
