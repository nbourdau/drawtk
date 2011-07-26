/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
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
#include <stdlib.h>
#include <stdio.h>
#include <dtk_video.h>
#include <drawtk.h>
#include <dtk_event.h>
#include <dtk_time.h>
#include <dtk_colors.h>

static dtk_hwnd wnd = NULL;
static dtk_htex video = NULL;
static dtk_hshape shape = NULL;
static dtk_hshape text = NULL;

static
void redraw(dtk_hwnd wnd)
{
	/* Erase previous draw
	 This is necessary since a redraw does not cover the whole window */
	dtk_clear_screen(wnd);

        /* Draw image on the window */
        dtk_draw_shape(shape);
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
                        dtk_video_exec(video, DTKV_CMD_PLAY);
                else if(evt->key.sym == DTKK_p)
                        dtk_video_exec(video, DTKV_CMD_PAUSE);
                else if(evt->key.sym == DTKK_q)
                        dtk_video_exec(video, DTKV_CMD_STOP);
		break;
	}

	return retcode;
}


int main(void)
{
        struct dtk_timespec delay = {0, 20000000}; /* 20ms */
        char desc[] = "videotestsrc ! clockoverlay font-desc=\"arial 30\"! decodebin2 ! ffmpegcolorspace ! appsink name=dtksink";
	
	// Setup window
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	dtk_set_event_handler(wnd, event_handler);

        video = dtk_load_video_gst(DTK_AUTOSTART, desc);

        shape = dtk_create_image(NULL, 0.0f, 0.4f, 1.0f, 0.8f, dtk_white, video);

        dtk_hfont font = dtk_load_font("arial");
        text = dtk_create_string(NULL,"ESC-Quit, S-Play, Q-Stop, P-Pause",0.1f, 0.0f,-0.5f,DTK_HMID,dtk_white,font);

        // Main loop
	while (dtk_process_events(wnd)) {
                dtk_nanosleep(0, &delay, NULL);
                redraw(wnd);
	}

        dtk_destroy_shape(text);
        dtk_destroy_shape(shape);
	dtk_destroy_texture(video);
        dtk_destroy_font(font);

	dtk_close(wnd);
	
        return 0;
}


