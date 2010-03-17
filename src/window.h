#ifndef WINDOW_H
#define WINDOW_H

#include "texmanager.h"

struct dtk_window;
struct dtk_texture_manager* get_texmanager(struct dtk_window* wnd);

extern struct dtk_window* current_window;

#endif
