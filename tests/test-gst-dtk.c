#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>

#include <gst/gst.h>
#include <glib.h>

#include "dtk_gstreamer.h"
#include "drawtk.h"
#include "dtk_event.h"
#include "dtk_time.h"
#include "dtk_colors.h"

static dtk_hwnd wnd = NULL;
dtk_gst_hpipeline pipe = NULL;
static dtk_hshape shape = NULL;
static dtk_hshape shape2= NULL;

dtk_gst_hpipeline startPipeline()
{
        // Initialization 
        dtk_gst_hpipeline pipe = dtk_gst_create_tcp_pipeline("dtk_pipeline","127.0.0.1",38501);
        //dtk_gst_hpipeline pipe = dtk_gst_create_file_pipeline("dtk_pipeline","/media/sf_master/test.ogg");

	bool success = (pipe!=NULL);
  
	if (!success) {
		g_printerr ("Pipeline could not be created. Exiting.\n");
		return NULL;
	}

	success = dtk_gst_run_pipeline(pipe);

	if (!success) {
        	g_printerr ("The pipeline could not be started. Exiting.\n");
        	return NULL;
	}
	else
	{
		g_print("The pipeline is executing.\n");
                return pipe;
        }
}


static
void redraw(dtk_hwnd wnd)
{
	/* Erase previous draw
	 This is necessary since a redraw does not cover the whole window */
	dtk_clear_screen(wnd);

	/* Draw image on the window */
       // dtk_draw_shape(shape2);

        dtk_draw_shape(shape);

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
		break;
	}

	return retcode;
}


int main (int argc, char *argv[])
{
	struct dtk_timespec delay = {0, 50000000}; /* 5ms */
	
	// Setup window
	wnd = dtk_create_window(1024, 768, 0, 0, 16, "hello");
	dtk_make_current_window(wnd);
	dtk_set_event_handler(wnd, event_handler);
	
        // start Pipeline 
	pipe = startPipeline();

        /*static char imgfilename[256];
        sprintf(imgfilename, "%s/navy.png", getenv("srcdir"));
        dtk_htex tex = dtk_load_image(imgfilename, 4);

        shape2= dtk_create_image(NULL, 0.3f, 0.3f, 0.5f, 0.5f, dtk_white, tex);
        */
        shape = dtk_create_image(NULL, 0.0f, 0.0f, 0.5f, 0.5f, dtk_white, dtk_gst_get_texture(pipe));

        g_print("okay, starting with tex %d\n",(int)dtk_gst_get_texture(pipe));

        // Main loop
	while (dtk_process_events(wnd)) {
		dtk_nanosleep(0, &delay, NULL);
		redraw(wnd);
	}

	// stop Pipeline
	dtk_gst_stop_pipeline(pipe);

	dtk_close(wnd);

	return 0;
}

