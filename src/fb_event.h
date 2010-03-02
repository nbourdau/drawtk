#ifndef FB_EVENT_H
#define FB_EVENT_H

#include <SDL/SDL_keysym.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	FB_EVT_REDRAW	= 0,
	FB_EVT_QUIT,
	FB_EVT_KEYUP,
	FB_EVT_KEYBOARD,
	FB_EVT_MOUSE,
	FB_EVT_MOUSEMOTION,
	NUM_FB_EVT
};

/* 2010-01-27  Michele Tavella <michele.tavella@epfl.ch>
 * Guys,
 * having troubles with those two flags:
 *  ../../examples/feedbackevents.cpp: In member function ‘virtual void FeedbackTest::HandleMouse(HbFeedbackEvents*, HbFeedbackMouse)’:
../../examples/feedbackevents.cpp:85: error: ‘SDL_PRESSED’ was not declared in this scope
*/
#define FB_KEY_PRESSED	1//SDL_PRESSED
#define FB_KEY_RELEASED	0//SDL_RELEASED


struct fb_keyevent
{
	unsigned int state;
	unsigned int sym;
	unsigned int mod;
};


struct fb_mouseevent
{
	unsigned int button;
	unsigned int state;
	unsigned int x;
	unsigned int y;
};


#ifdef __cplusplus
}
#endif

#endif /* FB_EVENT_H */
