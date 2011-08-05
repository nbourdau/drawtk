/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
