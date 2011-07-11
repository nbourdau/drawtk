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
/*  Example : errormon.c
 *
 * This program demonstrates how to implement a simple stimulus for an
 experimental protocol.
 * This shows how to:
 *	- setup and use a composite shape
 *      - use the absolute timing functions
 *      - implement an update function
 *	- where to signal hardware triggers to sync with display change
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drawtk.h>
#include <dtk_colors.h>
#include <dtk_event.h>
#include <dtk_time.h>

/* Parameters of the squares */
#define NSQUARE	11
#define WID	(0.7f*(2.0f/(NSQUARE+1)))
#define SQCOL		dtk_white
#define CCOL		dtk_blue
#define TCOLSTOP	dtk_red
#define TCOLRUN		dtk_orange
#define TCOLSUCCESS	dtk_green
static float sqpos[NSQUARE];

static dtk_hwnd wnd = NULL;
static dtk_hshape comp = NULL;
static dtk_hshape curs = NULL;
static dtk_hshape targ = NULL;

#define OUTTRIAL	0
#define INTRIAL		1
#define ENDTRIAL	2
static int state = ENDTRIAL;
static int icurs = NSQUARE/2, itarg;

static unsigned int ntrialmax = 20;
static unsigned int ntrial = 0;

#define PCORRECT	0.8f

/* Timestamp of the next protocol update to be done */
static struct dtk_timespec tnext;


/**************************************************************************
 *                        Random number generation                        *
 **************************************************************************/
static
void initrand(void)
{
	unsigned int seed;
	struct dtk_timespec ts;

	// Initialize the pseudo-random number sequence with the nanoseconds
	// field of current timestamp
	dtk_gettime(&ts);
	seed = ts.nsec;
	srandom(seed);
}

static
int randint(unsigned int nval)
{
	return nval*((double)random()/((double)RAND_MAX));
}

static
float randfloat(void)
{
	return ((float)random())/((float)RAND_MAX);
}


/**************************************************************************
 *                             Update functions                           *
 **************************************************************************/
static
void redraw(dtk_hwnd wnd)
{
	// Erase previous draw
	// This is necessary since a redraw does not cover the whole window
	dtk_clear_screen(wnd);

	// Draw all the boxes on the window (since comp owns all the shapes)
	dtk_draw_shape(comp);

	// refresh the window
	dtk_update_screen(wnd);
	/* If a stimulus change should be signaled in hardware triggers, the
	best place is here since dtk_update_screen returns only after the
	back and front buffer flip */
}


static
void init_trial_positions(void)
{
	// Setup new target position
	// Cursor keeps the same position
	do {
		itarg = randint(NSQUARE);
	} while (itarg == icurs);
	dtk_move_shape(targ, sqpos[itarg], 0.0f);
}

// This is where the protocol logic is implemented
static
int update_scene(void)
{
	int retval = 0;
	const float* tcol = TCOLRUN;

	/* This function is the appropriate place  to prepare the codes to
	be send to hardware triggers if needed (only prepared here, they
	would be actually sent in the redraw function) */

	if (state == INTRIAL) {
		// Update randomly the position of the cursor
		icurs += (itarg>icurs?1:-1) * ((randfloat()>PCORRECT)?-1:1);
		if (icurs<0)
			icurs = 1;
		if (icurs>=NSQUARE)
			icurs = NSQUARE-2;
		dtk_move_shape(curs, sqpos[icurs], 0.0f);

		// Next move will happen in 1s
		dtk_addtime(&tnext, 1, 0);

		// Change color if end trial is reached
		if (itarg == icurs) {
			state = ENDTRIAL;
			tcol = TCOLSUCCESS;
		}
	} else if (state == ENDTRIAL) {
		// Prepare for new trial: Move will be freezed for 3sec
		state = OUTTRIAL;
		init_trial_positions();
		tcol = TCOLSTOP;
		dtk_addtime(&tnext, 3, 0);

		// Chech for the end of protocol
		if (++ntrial > ntrialmax)
			retval = -1;
	} else if (state == OUTTRIAL) {
		state = INTRIAL;
		dtk_addtime(&tnext, 1, 0);
	}

	// Update the target with the corresponding color;
	targ = dtk_create_rectangle_hw(targ, 0.0, 0.0f,	WID, WID, 1, tcol);
	return retval;
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
		break;
	}

	return retcode;
}


/**************************************************************************
 *                       Initialization and cleanup                       *
 **************************************************************************/
// Most of the scene layout in prepared here
static
void setup_shapes(void)
{
	unsigned int i;
	float pos;
	dtk_hshape shp[NSQUARE+2]; 

	// Initialise cursor and target
	curs = dtk_create_rectangle_hw(NULL, 0.0, 0.0f, WID, WID, 1, CCOL);
	targ = dtk_create_rectangle_hw(NULL, 0.0, 0.0f, WID, WID, 1, TCOLSTOP);
	shp[0] = curs;
	shp[1] = targ;

	// Initialize the position of the squares
	for (i=0; i<NSQUARE; i++) {
		pos = -1.0f + (2*i+1)*(1.0f/NSQUARE);
		shp[i+2] = dtk_create_rectangle_hw(NULL, pos, 0.0f,
						 WID, WID, 0, SQCOL);
		sqpos[i] = pos;
	}

	// Create a composite shape holding all the other shapes
	comp = dtk_create_composite_shape(NULL, NSQUARE+2, shp, 1);
}


static
int init(int height, int width)
{
	initrand();

	// Setup a a window to performe the drawings and 
	// register its event handling function 
	wnd = dtk_create_window(width, height, 0, 0, 16, "error monitoring");
	if (!wnd) {
		fprintf(stderr, "Cannot create a window\n");
		return -1;
	}
	dtk_set_event_handler(wnd, event_handler);
	dtk_make_current_window(wnd);

	// Setup the protocol scene
	setup_shapes();

	return 0;
}


static
void cleanup(void)
{
	// Destroying the composite will destroy the other shapes as well
	// since we have specified this behavior at the creation of the
	// composite (see setup_shapes() function)
	dtk_destroy_shape(comp);

	dtk_close(wnd);
}


int main(int argc, char* argv[])
{
	struct dtk_timespec ts, *next;
	int opt, height = 768, width = 1024;

	// Parse command line options
	while ((opt = getopt(argc, argv, "n:h:w:")) != -1) {
		if (opt == 'n')
			ntrialmax = atoi(optarg);
		else if (opt == 'h')
			height = atoi(optarg);
		else if (opt == 'w')
			width = atoi(optarg);
		else {
			fprintf(stderr, "Usage: %s [-n ntrials] [-w width] "
			                "[-h height]\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (init(height, width))
		return EXIT_FAILURE;

	// Mesaure the initial timestamp
	dtk_gettime(&ts);
	memcpy(&tnext, &ts, sizeof(&ts));

	// update loop
	while (dtk_process_events(wnd)) {
		next = (dtk_difftime_ms(&ts, &tnext) > 0) ? &tnext : &ts;
		dtk_nanosleep(1, next, NULL);
		if (dtk_difftime_ms(next, &tnext) < 0) {
			dtk_addtime(&ts, 0, 20000000);	// Wake up in 20ms at most
			continue;
		}

		if (update_scene())
			break;
		redraw(wnd);
	}

	cleanup();
	return EXIT_SUCCESS;
}

