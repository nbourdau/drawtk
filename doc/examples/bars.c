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

/*  Example : bars.c
 *
 * This program demonstrates how to use a composite shape and text shape.
 * An evolving bar (with ticks and labels) is moving along an ellipsis.
 * Clicking on the close button of the window closes the demo.
 * This shows how to:
 *	- setup and use a composite shape
 *	- use fonts and text shape
 *      - use the timing functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <drawtk.h>
#include <dtk_colors.h>
#include <dtk_event.h>
#include <dtk_time.h>

/* Parameters of the size of the bar */
#define WID  		(1.0f)
#define BARH		(0.15f)
#define TLEN 		(0.05f)
#define COL		dtk_white
#define BCOL		dtk_blue
#define TEXTSIZE	0.1f


static dtk_hwnd wnd = NULL;
static dtk_hshape comp = NULL;
static dtk_hshape bar = NULL;
static dtk_hfont font = NULL;

/* Timestamp of the beginning of the animation */
static struct dtk_timespec tini;

static
void setup_shapes(void)
{
	/* Create certain number of shapes creating bar, ticks and labels */
	dtk_hshape shapes[] = {
		dtk_create_rectangle_2p(NULL, 0.0f, 0.0f, WID, BARH,0,BCOL),
		dtk_create_rectangle_2p(NULL, -WID, 0.0f, WID, BARH, 0,COL),
		dtk_create_line(NULL, -WID, -TLEN, -WID, 0.0f, COL),
		dtk_create_line(NULL, -WID/2, -TLEN, -WID/2, 0.0f, COL),
		dtk_create_line(NULL, 0.0f, -TLEN, 0.0f, 0.0f, COL),
		dtk_create_line(NULL, WID/2, -TLEN, WID/2, 0.0f, COL),
		dtk_create_line(NULL, WID, -TLEN, WID, 0.0f, COL),
		dtk_create_string(NULL, "-1.0", TEXTSIZE, -WID, -TLEN,
		                  DTK_TOP|DTK_HMID, COL, font), 
		dtk_create_string(NULL, "-0.5", TEXTSIZE, -WID/2, -TLEN,
		                  DTK_TOP|DTK_HMID, COL, font), 
		dtk_create_string(NULL, "0.0", TEXTSIZE, 0.0f, -TLEN,
		                  DTK_TOP|DTK_HMID, COL, font), 
		dtk_create_string(NULL, "0.5", TEXTSIZE, WID/2, -TLEN,
		                  DTK_TOP|DTK_HMID, COL, font), 
		dtk_create_string(NULL, "1.0", TEXTSIZE, WID, -TLEN,
		                  DTK_TOP|DTK_HMID, COL, font)
	};

	/* Keep the pointer to this shape since we will use it to modify
	   the displayed bar in the update_bar() function */
	bar = shapes[0];

	/* Combine previous shapes altogether to create a unique shape
	   easier to manipulate.
	   Since we specify that the destruction of the composite should
	   destroy the underlying shapes, we don't need to keep a pointer
	   to each one individually */
	comp = dtk_create_composite_shape(NULL, 
	                                  sizeof(shapes)/sizeof(shapes[0]),
					  shapes, 1);
}


static
void cleanup_shapes(void)
{
	/* Destroying the composite will destroy the other shapes as well
	   since we have specified this behavior at the creation of the
	   composite (see setup_shapes() function) */
	dtk_destroy_shape(comp);
}


static
void redraw(dtk_hwnd wnd)
{
	/* Erase previous draw
	 This is necessary since a redraw does not cover the whole window */
	dtk_clear_screen(wnd);

	/* Draw the rectangle on the window */
	dtk_draw_shape(comp);

	/* refresh the window */
	dtk_update_screen(wnd);
}


static
void update_bar(void)
{
	float t, x;
	struct dtk_timespec ts;
	
	/* Measure how much spent since last update */
	dtk_gettime(&ts);
	t = dtk_difftime_ms(&ts, &tini);

	/* Update the bar length */
	x = cos(2*3.14*t/1000);
	bar = dtk_create_rectangle_2p(bar, 0.0f, 0.0f, x, BARH, 1, BCOL);

	/* Update the position of the composite */
	dtk_move_shape(comp, 0.1*cos(t/1000), 0.2*sin(t/1000));
}


int main(void)
{
	struct dtk_timespec delay = {0, 5000000}; /* 5ms */
	
	/* Setup a a window to performe the drawings and 
	   register its event handling function */
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);

	/* Load the font used for the labels */
	font = dtk_load_font("Arial");
	setup_shapes();

	dtk_gettime(&tini); /* Measure the initial timestamp */

	while (dtk_process_events(wnd)) {
		dtk_nanosleep(0, &delay, NULL);
		update_bar();
		redraw(wnd);
	}

	/* Cleanup all created resources */
	cleanup_shapes();
	dtk_destroy_font(font);
	dtk_close(wnd);

	return 0;
}

