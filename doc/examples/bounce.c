/*
    Copyright (C) 2011 EPFL (Ecole Polytechnique Fédérale de Lausanne)
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

/*  Example : bounce.c
 *
 * This program demonstrates how to perform a simple animation (bouncing box).
 * A box is bouncing in an area limited by coordinates (-1,-1) and (1,1).
 * A video is displayed in the background. This is specified by command line
 * or a test video is used if none are provided
 * Arrow keys stop and enable vertical and horizontal movement
 * Pressing the ESC key close the demo.
 * s, p, r respectively starts, pauses and rewind the video
 * This shows how to:
 *	- do minimalist drawings
 *      - use the timing functions
 *	- use a video texture
 *	- use the events
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <drawtk.h>
#include <dtk_video.h>
#include <dtk_colors.h>
#include <dtk_event.h>
#include <dtk_time.h>


#define VX	1.0f /* per sec */
#define VY	1.0f /* per sec */
#define WIDTH	0.1f
#define HEIGHT	0.1f
#define MAXX	(1.0f - WIDTH/2.0f)
#define MINX	(-MAXX)
#define MAXY	(1.0f - HEIGHT/2.0f)
#define MINY	(-MAXY)


dtk_hwnd wnd = NULL;
dtk_hshape obj = NULL;
dtk_hshape vidshp = NULL;
dtk_htex video = NULL;

int dirx = 1, diry = 1;
float x = 0.1f, y = 0.0f;
struct dtk_timespec ts;


static
void setup_shapes(void)
{
	unsigned int h, w;
	float wid;

	obj = dtk_create_rectangle_hw(NULL, 0.0f, 0.0f,
	                           HEIGHT, WIDTH, 1, dtk_white);

	/* Create an image shape with the same aspect ratio as the video */
	dtk_texture_getsize(video, &w, &h);
	wid = 2.0*((float)h / (float)w);
	vidshp = dtk_create_image(NULL, 0.0,0.0, 2.0,wid, dtk_white, video);
}


static
void cleanup_shapes(void)
{
	dtk_destroy_shape(obj);
	dtk_destroy_shape(vidshp);
}


static
void redraw(dtk_hwnd wnd)
{
	/* Erase previous draw
	 This is necessary since a redraw does not cover the whole window */
	dtk_clear_screen(wnd);

	/* Draw the current video frame */
	dtk_draw_shape(vidshp);

	/* Draw the rectangle on the window */
	dtk_draw_shape(obj);

	/* refresh the window */
	dtk_update_screen(wnd);
}


static
void update_object(void)
{
	float dt;
	struct dtk_timespec newts;
	
	/* Measure how much spent since last update */
	dtk_gettime(&newts);
	dt = dtk_difftime_ms(&newts, &ts);
	memcpy(&ts, &newts, sizeof(ts));
	
	/* Update box position */
	x += dirx*VX*dt/1000;
	y += diry*VY*dt/1000;

	/* Bounce on X-axis */
	if (x > MAXX) {
		dirx = -1;
		x = MAXX - (x - MAXX);
	} else if (x < MINX) {
		dirx = 1;
		x = MINX + (MINX - x);
	}

	/* Bounce on Y-axis */
	if (y > MAXY) {
		diry = -1;
		y = MAXY - (y - MAXY);
	} else if (y < MINY) {
		diry = 1;
		y = MINY + (MINY - y);
	}

	/* Record the new postion */
	dtk_move_shape(obj, x, y);
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
		else if (evt->key.state == DTK_KEY_PRESSED) {
			if (evt->key.sym == DTKK_UP)
				diry = diry >= 1 ? 1 : diry+1;
			else if (evt->key.sym == DTKK_DOWN)
				diry = diry <= -1 ? -1 : diry-1;
			else if (evt->key.sym == DTKK_RIGHT)
				dirx = dirx >= 1 ? 1 : dirx+1;
			else if (evt->key.sym == DTKK_LEFT)
				dirx = dirx <= -1 ? -1 : dirx-1;
		}
                else if(evt->key.sym == DTKK_s)
                        dtk_video_exec(video,DTKV_CMD_PLAY, NULL);
                else if(evt->key.sym == DTKK_p)
                        dtk_video_exec(video,DTKV_CMD_PAUSE, NULL);
                else if(evt->key.sym == DTKK_r)
                        dtk_video_exec(video,DTKV_CMD_SEEK, NULL);
		break;
	}

	return retcode;
}


int main(int argc, char* argv[])
{
	struct dtk_timespec delay = {0, 5000000}; /* 5ms */

	/* Load the video into a texture and start playing immediately */
	if (argc >= 2)
		video = dtk_load_video_file(DTK_AUTOSTART, argv[1]);
	else
		video = dtk_load_video_test(DTK_AUTOSTART);
	
	/* Setup a a window to performe the drawings and 
	   register its event handling function */
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	dtk_set_event_handler(wnd, event_handler);

	/* Create the shape to draw on screen */
	setup_shapes();

	dtk_gettime(&ts); /* Measure the initial timestamp */

	while (dtk_process_events(wnd)) {
		dtk_nanosleep(0, &delay, NULL);
		update_object();
		redraw(wnd);
	}

	/* Cleanup all created resources */
	cleanup_shapes();
	dtk_destroy_texture(video);
	dtk_close(wnd);

	return 0;
}

