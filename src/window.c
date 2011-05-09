/*
    Copyright (C) 2009-2010  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "drawtk.h"
#include "window.h" 
#include "texmanager.h"
#include "dtk_event.h"


LOCAL_FN
struct dtk_window* current_window = NULL;


/*************************************************************************
 *                                                                       *
 *                          Window functions                             *
 *                                                                       *
 *************************************************************************/

LOCAL_FN
int init_opengl_state(struct dtk_window* wnd)
{
	float ratio, iw, ih;
	GLenum err;

	// Setup 2D projection
	glViewport(0, 0, wnd->width, wnd->height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ratio = (float)wnd->width/(float)wnd->height;
	iw = (ratio > 1.0f) ? ratio : 1.0f;
	ih = (ratio > 1.0f) ? 1.0f : 1.0f/ratio;
	glOrtho(-iw, iw, -ih, ih, -1, 1);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	// Init identity for modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Enable 2D texturing
	glEnable(GL_TEXTURE_2D);

	// Set clear color and clear blackground
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup transparency
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

	// Check for errors
	err = glGetError();
	if(err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL could not be initialized! (err=0x%04x)\n", err);
	//	return 1;
	}
	
	return 0;
}


LOCAL_FN
int resize_window(struct dtk_window* wnd, int width, int height, int fs)
{
	int flags = SDL_OPENGL | SDL_RESIZABLE | SDL_ANYFORMAT;
	const SDL_VideoInfo* info = NULL;
	SDL_Surface* surf;

	info = SDL_GetVideoInfo();
	if(info == NULL) {
		fprintf(stderr, "SDL_GetVideoInfo was kicked by Chuck Norris.\n");
		return 1;
	}

	// Set window settings
	width  = (width > 0) ? width : info->current_w;
	height = (height > 0) ? height : info->current_h;  

	if (fs)
		flags |= SDL_FULLSCREEN;

	// Create resizable window using OpenGL
	surf = SDL_SetVideoMode(width, height, wnd->bpp, flags);
	if (!surf) {
		fprintf(stderr, "Window could not be loaded!\n");
		return 1;
	}

	wnd->window = surf;
	wnd->width = surf->w;
	wnd->height = surf->h;
	
	return 0;
}


API_EXPORTED
dtk_hwnd dtk_create_window(unsigned int width, unsigned int height, unsigned int x, unsigned int y, unsigned int bpp, const char* caption)
{
	int is_init = 0, fs = 0;
	char* wndstr = malloc(strlen(caption)+1);
	struct dtk_window* wnd = malloc(sizeof(struct dtk_window));
	
	if (!wndstr || !wnd) {
		fprintf(stderr, "Memory alloc problems in window creation: No window created\n");
		goto error;
	}
	
	// Initialize SDL
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		fprintf(stderr,"SDL could not be initialized\n");
		goto error;
	}
	is_init = 1;

	wnd->x = x;
	wnd->y = y;
	wnd->bpp = bpp;
	strcpy(wndstr, caption);
	wnd->caption = wndstr;

	// Init parameters of the frame buffers
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 0 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	

	// Create resizable window using OpenGL
	if (!width || !height)
		fs = 1;
	resize_window(wnd, width, height, fs);
	
	// Set caption
	SDL_WM_SetCaption(wnd->caption,NULL);
	atexit(SDL_Quit);

	if (init_opengl_state(wnd))
		goto error;

	acquire_texture_manager();
	
	// Return the structure holding window and window information	
	return wnd;

error:
	if (is_init)
		SDL_Quit();
	free(wnd);
	free(wndstr);
	return NULL;
}                         


API_EXPORTED
void dtk_clear_screen(dtk_hwnd wnd)
{
	(void)wnd;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


API_EXPORTED
void dtk_update_screen(dtk_hwnd wnd)  
{                         
	(void)wnd;
	                
	// Update screen
	SDL_GL_SwapBuffers();			
}


API_EXPORTED
void dtk_window_getsize(dtk_hwnd wnd, unsigned int* w, unsigned int* h)
{
	if (!wnd || !h || !w)
		return;

	*w = wnd->width;
	*h = wnd->height;
}


API_EXPORTED
void dtk_close(dtk_hwnd wnd)
{
	if (!wnd) 
		return;
	
	// Unbind the window if current
	if (current_window == wnd)
		current_window = NULL;

	// Assume there is only one window so destroy all texture in it.
	// This assumption might be wrong in case of multiple windows
	// support
	release_texture_manager();

	free(wnd->caption);
	free(wnd);
	SDL_Quit();

	//TODO: Check if needed
	//FreeImage_DeInitialise();	
}


API_EXPORTED
void dtk_make_current_window(dtk_hwnd wnd)
{
	current_window = wnd;	
}

API_EXPORTED
void dtk_bgcolor(float* bgcolor)
{
	glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);	
}

API_EXPORTED
void dtk_set_event_handler(dtk_hwnd wnd, DTKEvtProc handler)
{
	wnd->evthandler = handler;
}


