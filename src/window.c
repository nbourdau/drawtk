#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include "feedback.h"
#include "texmanager.h"
#include "window.h" 
#include "fb_event.h"


struct fb_window* current_window = NULL;

// Settings of the feedback window
struct fb_window
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

	// Texture manager
	struct fb_texture_manager* texman;	
	EventHandlerProc evthandler;
};


/*************************************************************************
 *                                                                       *
 *                          Window functions                             *
 *                                                                       *
 *************************************************************************/

int init_opengl_state(struct fb_window* wnd)
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

int resize_window(struct fb_window* wnd, int width, int height, int fs)
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

fb_hwnd fb_create_window(unsigned int width, unsigned int height, unsigned int x, unsigned int y, unsigned int bpp, const char* caption)
{
	int is_init = 0, fs = 0;
	char* wndstr = malloc(strlen(caption)+1);
	struct fb_window* wnd = malloc(sizeof(struct fb_window));
	
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
	
/*
	// Create texture manager
	wnd->texman = create_texture_manager();
	if (!wnd->texman)
		goto error;
*/
		
	// Return the structure holding window and window information	
	return wnd;

error:
	if (is_init)
		SDL_Quit();
	free(wnd);
	free(wndstr);
	return NULL;
}                         

void fb_clear_screen(fb_hwnd wnd)
{
	(void)wnd;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void fb_update_screen(fb_hwnd wnd)  
{                         
	(void)wnd;
	                
	// Update screen
	SDL_GL_SwapBuffers();			
}

void fb_close(fb_hwnd wnd)
{
	if (!wnd) 
		return;
	
	// Unbind the window if current
	if (current_window == wnd)
		current_window = NULL;

	destroy_texture_manager(wnd->texman);
	free(wnd->caption);
	free(wnd);
	SDL_Quit();

	//TODO: Check if needed
	//FreeImage_DeInitialise();	
}

struct fb_texture_manager* get_texmanager(struct fb_window* wnd)
{
	return wnd->texman;
}

void fb_make_current_window(fb_hwnd wnd)
{
	current_window = wnd;	
}

void fb_bgcolor(float* bgcolor)
{
	glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);	
}

void fb_set_event_handler(fb_hwnd wnd, EventHandlerProc handler)
{
	wnd->evthandler = handler;
}


int fb_poll_event(struct fb_window* wnd, unsigned int* type,
		struct fb_keyevent* keyevt, struct fb_mouseevent* mouseevt) 
{
	SDL_Event evt;
	if(!SDL_PollEvent(&evt))
		return 0;

	switch (evt.type) {
		case SDL_QUIT:
			*type = FB_EVT_QUIT;
			break;
		case SDL_VIDEOEXPOSE:
			*type = FB_EVT_REDRAW;
			break;
		case SDL_VIDEORESIZE:
			resize_window(wnd, evt.resize.w, evt.resize.h, 0);
			init_opengl_state(wnd);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			*type = FB_EVT_KEYBOARD;
			keyevt->state = evt.key.state;
			keyevt->sym = evt.key.keysym.sym;
			keyevt->mod = evt.key.keysym.mod;
			break;
		/* 2010-01-27  Michele Tavella <michele.tavella@epfl.ch>
		 * Please do not change the order, otherwise Ganga 
		 * will kill you (indian style).
		 */
		case SDL_MOUSEMOTION:
			*type = FB_EVT_MOUSEMOTION;
			mouseevt->button = evt.button.button;
			mouseevt->state = evt.button.state;
			mouseevt->x = evt.button.x;
			mouseevt->y = evt.button.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			*type = FB_EVT_MOUSE;
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

int fb_process_events(struct fb_window* wnd)
{
	EventHandlerProc handler = wnd->evthandler;
	SDL_Event evt;
	int ret = 1;

	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:
			if (handler) 
				ret = handler(wnd, FB_EVT_QUIT, NULL);
			break;

		case SDL_VIDEOEXPOSE:
			if (handler)
				ret = handler(wnd, FB_EVT_REDRAW, NULL);
			break;

		case SDL_VIDEORESIZE:
			resize_window(wnd, evt.resize.w, evt.resize.h, 0);
			init_opengl_state(wnd);
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if (handler) {
				struct fb_keyevent keyevt = {
					.state = evt.key.state,
					.sym = evt.key.keysym.sym,
					.mod = evt.key.keysym.mod
				};
				ret = handler(wnd, FB_EVT_KEYBOARD, &keyevt);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if(handler) {
				struct fb_mouseevent mouseevt = {
					.button = evt.button.button,
					.state = evt.button.state,
					.x = evt.button.x,
					.y = evt.button.y
				};
				ret = handler(wnd,FB_EVT_MOUSE,&mouseevt);
			}
		}
		if (!ret)
			return 0;
	}
	return 1;
}
