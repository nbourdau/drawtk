#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>

#include <gst/gst.h>
#include <glib.h>

#include "dtk_video.h"
#include "drawtk.h"
#include "dtk_event.h"
#include "dtk_time.h"
#include "dtk_colors.h"

static dtk_hwnd wnd = NULL;
static dtk_htex video1 = NULL;
static dtk_htex video2 = NULL;

static dtk_hshape shape1 = NULL;
static dtk_hshape shape2 = NULL;
static dtk_hshape text = NULL;

static
void redraw(dtk_hwnd wnd)
{
	/* Erase previous draw
	 This is necessary since a redraw does not cover the whole window */
	dtk_clear_screen(wnd);

        /* Draw image on the window */
        dtk_draw_shape(shape1);
        dtk_draw_shape(shape2);
        dtk_draw_shape(text);

	/* refresh the window */
	dtk_update_screen(wnd);
}

static
int event_handler(dtk_hwnd wnd, int type, const union dtk_event* evt)
{
	int retcode = 1;

	switch (type) {
	case DTK_EVT_QUIT:
		retcode = 0;
		break;

	case DTK_EVT_REDRAW:
		redraw(wnd);
		break;
	
	case DTK_EVT_KEYBOARD:
		if (evt->key.sym == DTKK_ESCAPE)
			retcode = 0;
                else if(evt->key.sym == DTKK_s)
                        dtk_video_exec(video2,DTKV_CMD_PLAY);
                else if(evt->key.sym == DTKK_p)
                        dtk_video_exec(video2,DTKV_CMD_PAUSE);
                else if(evt->key.sym == DTKK_q)
                        dtk_video_exec(video2,DTKV_CMD_STOP);
		break;
	}

	return retcode;
}


int main(void)
{
        struct dtk_timespec delay = {0, 5000000}; /* 5ms */
	
	// Setup window
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	dtk_set_event_handler(wnd, event_handler);

        static char vidfilename[256];
        sprintf(vidfilename, "%s/test.ogv", getenv("srcdir"));

        static char fontfilename[256];
        sprintf(fontfilename, "%s/test.ttf", getenv("srcdir"));

        video1 = dtk_create_video(DTKV_FEED_TEST,true);
        video2 = dtk_create_video(DTKV_FEED_FILE,true,vidfilename);

        shape1 = dtk_create_image(NULL, -0.5f, 0.3f, 0.5f, 0.5f, dtk_white, video1);
        shape2 = dtk_create_image(NULL,  0.5f, 0.3f, 0.5f, 0.5f, dtk_white, video2);

        dtk_hfont font = dtk_load_font(fontfilename);
        text = dtk_create_string(NULL,"ESC-Quit, S-Play, Q-Stop, P-Pause",0.1f,-0.9f,-0.5f,DTK_LEFT,dtk_white,font);

        // Main loop
	while (dtk_process_events(wnd) && dtk_video_getstate(video1)==DTKV_PLAYING) {
                dtk_nanosleep(0, &delay, NULL);
                redraw(wnd);
	}

        bool retCode = (dtk_video_getstate(video1)!=DTKV_PLAYING);

        dtk_destroy_shape(text);
        dtk_destroy_shape(shape1);
        dtk_destroy_shape(shape2);
        dtk_destroy_font(font);

	dtk_close(wnd);
	
        return retCode;
}

