#ifndef WINDOW_H
#define WINDOW_H

#include <SDL/SDL.h>
#include "dtk_event.h"

struct dtk_window
{
	// Dimensions (pixels)
	unsigned int width;
	unsigned int height;
	
	// Position (pixels)
	unsigned int x;
	unsigned int y;

	// Bits per pixel
	unsigned int bpp;
	
	// Window caption
	char* caption;

	// SDL Surface for window
	SDL_Surface* window;

	int (*evthandler)(struct dtk_window*, int, const union dtk_event*);
};

LOCAL_FN int init_opengl_state(struct dtk_window* wnd);
LOCAL_FN int resize_window(struct dtk_window* wnd, int w, int h, int fs);

extern struct dtk_window* current_window;

#endif
