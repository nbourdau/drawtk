/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "drawtk.h"
#include "dtk_event.h"
#include "window.h"


static 
void fill_keyevt(union dtk_event *d_evt, const SDL_Event* s_evt)
{
	d_evt->key.state = (s_evt->key.state==SDL_PRESSED)?
		            DTK_KEY_PRESSED : DTK_KEY_RELEASED;
	d_evt->key.sym = s_evt->key.keysym.sym;
	d_evt->key.mod = s_evt->key.keysym.mod;
}

static 
void fill_mouseevt(union dtk_event *d_evt, const SDL_Event* s_evt)
{
	d_evt->mouse.button = s_evt->button.button;
	d_evt->mouse.state = s_evt->button.state;
	d_evt->mouse.x = s_evt->button.x;
	d_evt->mouse.y = s_evt->button.y;
}


API_EXPORTED
int dtk_process_events(struct dtk_window* wnd)
{
	DTKEvtProc handler = wnd->evthandler;
	SDL_Event sevt;
	union dtk_event evt;
	int ret = 1;
	int type;

	while (SDL_PollEvent(&sevt)) {
		switch (sevt.type) {
		case SDL_QUIT:
			if (handler) 
				ret = handler(wnd, DTK_EVT_QUIT, NULL);
			else
				ret = 0;
			break;

		case SDL_WINDOWEVENT_EXPOSED:
			if (handler)
				ret = handler(wnd, DTK_EVT_REDRAW, NULL);
			break;

		case SDL_WINDOWEVENT_RESIZED:
			resize_window(wnd, sevt.window.data1,
			                   sevt.window.data2, 0);
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if (handler) {
				fill_keyevt(&evt, &sevt);
				ret = handler(wnd, DTK_EVT_KEYBOARD, &evt);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
			if (handler) {
				fill_mouseevt(&evt, &sevt);
				type = (sevt.type == SDL_MOUSEMOTION) ? 
				        DTK_EVT_MOUSEMOTION : 
					DTK_EVT_MOUSEBUTTON;
				ret = handler(wnd, type ,&evt);
			}
			break;
		}
		if (!ret)
			return 0;
	}
	return 1;
}

