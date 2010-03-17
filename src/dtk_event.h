#ifndef DTK_EVENT_H
#define DTK_EVENT_H

#include <SDL/SDL_keysym.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	DTK_EVT_REDRAW	= 0,
	DTK_EVT_QUIT,
	DTK_EVT_KEYUP,
	DTK_EVT_KEYBOARD,
	DTK_EVT_MOUSE,
	DTK_EVT_MOUSEMOTION,
	NUM_DTK_EVT
};

/* 2010-01-27  Michele Tavella <michele.tavella@epfl.ch>
 * Guys,
 * having troubles with those two flags:
 *  ../../examples/feedbackevents.cpp: In member function ‘virtual void FeedbackTest::HandleMouse(HbFeedbackEvents*, HbFeedbackMouse)’:
../../examples/feedbackevents.cpp:85: error: ‘SDL_PRESSED’ was not declared in this scope
*/
#define DTK_KEY_PRESSED	1//SDL_PRESSED
#define DTK_KEY_RELEASED	0//SDL_RELEASED


struct dtk_keyevent
{
	unsigned int state;
	unsigned int sym;
	unsigned int mod;
};


struct dtk_mouseevent
{
	unsigned int button;
	unsigned int state;
	unsigned int x;
	unsigned int y;
};


#ifdef __cplusplus
}
#endif

#endif /* DTK_EVENT_H */
