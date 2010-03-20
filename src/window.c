#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "drawtk.h"
#include "texmanager.h"
#include "window.h" 
#include "dtk_event.h"


struct dtk_window* current_window = NULL;

// Settings of the feedback window
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

	EventHandlerProc evthandler;
};


/*************************************************************************
 *                                                                       *
 *                          Window functions                             *
 *                                                                       *
 *************************************************************************/

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

	// Check for errors
	err = glGetError();
	if(err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL could not be initialized! (err=0x%04x)\n", err);
	//	return 1;
	}
	
	return 0;
}

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
	
	// Return the structure holding window and window information	
	return wnd;

error:
	if (is_init)
		SDL_Quit();
	free(wnd);
	free(wndstr);
	return NULL;
}                         

void dtk_clear_screen(dtk_hwnd wnd)
{
	(void)wnd;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void dtk_update_screen(dtk_hwnd wnd)  
{                         
	(void)wnd;
	                
	// Update screen
	SDL_GL_SwapBuffers();			
}

void dtk_close(dtk_hwnd wnd)
{
	if (!wnd) 
		return;
	
	// Unbind the window if current
	if (current_window == wnd)
		current_window = NULL;

	// Assume there is only one window so destroy all texture in it. This
	// assumption might be wrong in case of multiple windows support
	dtk_delete_textures();

	free(wnd->caption);
	free(wnd);
	SDL_Quit();

	//TODO: Check if needed
	//FreeImage_DeInitialise();	
}


void dtk_make_current_window(dtk_hwnd wnd)
{
	current_window = wnd;	
}

void dtk_bgcolor(float* bgcolor)
{
	glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);	
}

void dtk_set_event_handler(dtk_hwnd wnd, EventHandlerProc handler)
{
	wnd->evthandler = handler;
}


int dtk_poll_event(struct dtk_window* wnd, unsigned int* type,
		struct dtk_keyevent* keyevt, struct dtk_mouseevent* mouseevt) 
{
	SDL_Event evt;
	if(!SDL_PollEvent(&evt))
		return 0;

	switch (evt.type) {
		case SDL_QUIT:
			*type = DTK_EVT_QUIT;
			break;
		case SDL_VIDEOEXPOSE:
			*type = DTK_EVT_REDRAW;
			break;
		case SDL_VIDEORESIZE:
			resize_window(wnd, evt.resize.w, evt.resize.h, 0);
			init_opengl_state(wnd);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			*type = DTK_EVT_KEYBOARD;
			keyevt->state = evt.key.state;
			keyevt->sym = evt.key.keysym.sym;
			keyevt->mod = evt.key.keysym.mod;
			break;
		/* 2010-01-27  Michele Tavella <michele.tavella@epfl.ch>
		 * Please do not change the order, otherwise Ganga 
		 * will kill you (indian style).
		 */
		case SDL_MOUSEMOTION:
			*type = DTK_EVT_MOUSEMOTION;
			mouseevt->button = evt.button.button;
			mouseevt->state = evt.button.state;
			mouseevt->x = evt.button.x;
			mouseevt->y = evt.button.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			*type = DTK_EVT_MOUSE;
			mouseevt->button = evt.button.button;
			mouseevt->state = evt.button.state;
			mouseevt->x = evt.button.x;
			mouseevt->y = evt.button.y;
			break;
		default:
			break;
	}
	return 1;
}

int dtk_process_events(struct dtk_window* wnd)
{
	EventHandlerProc handler = wnd->evthandler;
	SDL_Event evt;
	int ret = 1;

	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:
			if (handler) 
				ret = handler(wnd, DTK_EVT_QUIT, NULL);
			break;

		case SDL_VIDEOEXPOSE:
			if (handler)
				ret = handler(wnd, DTK_EVT_REDRAW, NULL);
			break;

		case SDL_VIDEORESIZE:
			resize_window(wnd, evt.resize.w, evt.resize.h, 0);
			init_opengl_state(wnd);
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if (handler) {
				struct dtk_keyevent keyevt = {
					.state = evt.key.state,
					.sym = evt.key.keysym.sym,
					.mod = evt.key.keysym.mod
				};
				ret = handler(wnd, DTK_EVT_KEYBOARD, &keyevt);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(handler) {
				struct dtk_mouseevent mouseevt = {
					.button = evt.button.button,
					.state = evt.button.state,
					.x = evt.button.x,
					.y = evt.button.y
				};
				ret = handler(wnd,DTK_EVT_MOUSE,&mouseevt);
			}
		}
		if (!ret)
			return 0;
	}
	return 1;
}
