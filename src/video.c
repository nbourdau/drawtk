/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Pierluca Borsò <pierluca.borso@epfl.ch>

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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

#include <gst/gst.h>
#include <glib.h>
#include <gst/base/gstbasetransform.h>

#include <SDL/SDL.h>

#include "dtk_video.h"
#include "video_structs.h"
#include "video_custom.h"

#include "drawtk.h"
#include "texmanager.h"
#include "shapes.h"
#include "window.h"
#include "dtk_time.h"


/**************************************************************************
 *                           Pipeline creation                            *
 **************************************************************************/
static
dtk_hpipe dtk_create_tcp_pipeline(const char *server, int port)
{
	char pipeName[255];
	dtk_hpipe pl;

	if (port < 1 || strlen(server) == 0)
		return NULL;

	sprintf(pipeName, "TCP:%s:%d", server, port);
	pl = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pl, "tcpclientsrc", "tcp-src",
				  "host", server, "port", port, NULL);
	dtk_pipe_add_element(pl, "queue", "queue");
	dtk_pipe_add_element(pl, "decodebin2", "decoder-bin");

	return pl;
}


static
dtk_hpipe dtk_create_udp_pipeline(int port)
{
	char pipeName[255];
	dtk_hpipe pl;

	if (port < 1)
		return NULL;

	sprintf(pipeName, "UDP:%d", port);
	pl = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pl, "udpsrc", "udp-src",
				  "port", port, NULL);
	dtk_pipe_add_element(pl, "queue", "data-queue");
	dtk_pipe_add_element(pl, "decodebin2", "decoder-bin");

	return pl;
}


static
dtk_hpipe dtk_create_file_pipeline(const char *file)
{
	char pipeName[255];
	dtk_hpipe pl;

	if (strlen(file) == 0)
		return NULL;

	sprintf(pipeName, "FILE:%s", file);
	pl = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pl, "filesrc", "file-src",
				  "location", file, NULL);
	dtk_pipe_add_element(pl, "queue", "queue");
	dtk_pipe_add_element(pl, "decodebin2", "decoder-bin");

	return pl;
}


static
dtk_hpipe dtk_create_test_pipeline(void)
{
	dtk_hpipe pl = dtk_create_video_pipeline("test-pipe");
	dtk_pipe_add_element(pl, "videotestsrc", "test-src");

	return pl;
}


/**************************************************************************
 *                           Pipeline execution                           *
 **************************************************************************/
static
bool bus_callback(GstMessage * msg)
{
	GError *error;

	switch (GST_MESSAGE_TYPE(msg)) {
	case GST_MESSAGE_EOS:
		return false;

	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &error, NULL);
		fprintf(stderr, "Error: %s\n", error->message);
		g_error_free(error);
		return false;

	default:
		return true;
	}
}


static
void *run_pipeline_loop(void *arg)
{
	GstMessage *msg = NULL;
	dtk_hpipe pl = arg;

	// main loop
	bool loop = true;
	while (loop && pl->status != DTKV_STOPPED) {
		if (pthread_mutex_trylock(&(pl->status_lock)) == 0) {
			// Read new messages from bus
			while (loop && (msg = gst_bus_pop(pl->bus))) {
				loop = bus_callback(msg);
				gst_message_unref(msg);
			}

			// if the bus hasn't requested termination, unlock 
			if (loop)
				pthread_mutex_unlock(&(pl->status_lock));
		}
	}

	// set status to STOPPED
	pl->status = DTKV_STOPPED;
	gst_element_set_state(pl->pipe, GST_STATE_READY);

	// if the bus HAS requested termination, unlock only now
	if (!loop)
		pthread_mutex_unlock(&(pl->status_lock));

	return NULL;
}


static
bool run_pipeline(dtk_hpipe pl)
{
	bool paused;
	GstStateChangeReturn ret;
	struct dtk_timespec delay = { 0, 50000000 };	// 50 ms

	pthread_mutex_lock(&(pl->status_lock));

	if (pl->status == DTKV_PLAYING) {
		pthread_mutex_unlock(&(pl->status_lock));
		return true;
	}

	paused = (pl->status == DTKV_PAUSED);

	// prepare pipeline status
	pl->status = DTKV_READY;

	// execute state change and wait for state change to take effect
	ret = gst_element_set_state(pl->pipe, GST_STATE_PLAYING);
	while (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(pl->pipe, NULL, NULL,
					  500 * GST_MSECOND);

	// handle failure
	if (ret == GST_STATE_CHANGE_FAILURE) {
		pthread_mutex_unlock(&(pl->status_lock));
		fprintf(stderr, "failed to run pipeline\n");
		return false;
	}
	// handle case in which pipeline was simply paused
	if (paused) {
		pl->status = DTKV_PLAYING;
		pthread_mutex_unlock(&(pl->status_lock));
		return true;
	}

	pthread_mutex_unlock(&(pl->status_lock));

	// launch execution thread
	pthread_create(&(pl->thread), NULL, run_pipeline_loop, pl);

	// wait till the state changes
	while (pl->status == DTKV_READY)
		dtk_nanosleep(0, &delay, NULL);

	return (pl->status == DTKV_PLAYING);
}


static
void stop_pipeline(dtk_hpipe pl)
{
	pthread_mutex_lock(&(pl->status_lock));

	pl->status = DTKV_STOPPED;
	pthread_join(pl->thread, NULL);

	pthread_mutex_unlock(&(pl->status_lock));
}


static
bool pause_pipeline(dtk_hpipe pl)
{
	GstStateChangeReturn ret;
	pthread_mutex_lock(&(pl->status_lock));

	if (pl->status != DTKV_PLAYING) {
		pthread_mutex_unlock(&(pl->status_lock));
		return false;
	}
	// Change state and wait for state change to take effect
	ret = gst_element_set_state(pl->pipe, GST_STATE_PAUSED);
	while (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(pl->pipe, NULL, NULL,
					    500 * GST_MSECOND);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		pthread_mutex_unlock(&(pl->status_lock));
		return false;
	}
	// set pipeline status to paused
	pl->status = DTKV_PAUSED;

	pthread_mutex_unlock(&(pl->status_lock));

	return true;
}


/**************************************************************************
 *                    Video related API implementation                    *
 **************************************************************************/
API_EXPORTED
dtk_htex dtk_create_video(int feed_type, bool autostart,...)
{
	dtk_hpipe pl = NULL;
	dtk_htex tex = NULL;
	va_list args;

	va_start(args, autostart);
	switch (feed_type) {
	case DTKV_FEED_TCP:
		pl = dtk_create_tcp_pipeline(va_arg(args, const char *),
		                             va_arg(args, int));
		break;

	case DTKV_FEED_UDP:
		pl = dtk_create_udp_pipeline(va_arg(args, int));
		break;

	case DTKV_FEED_FILE:
		pl = dtk_create_file_pipeline(va_arg(args, const char *));
		break;

	case DTKV_FEED_TEST:
		pl = dtk_create_test_pipeline();
		break;
	}
	va_end(args);

	tex = dtk_create_video_from_pipeline(pl);
	if (autostart && tex)
		dtk_video_exec(tex, DTKV_CMD_PLAY);

	return tex;
}


API_EXPORTED
int dtk_video_getstate(dtk_htex video)
{
	dtk_hpipe pl = video->aux;

	return pl->status;
}


API_EXPORTED
bool dtk_video_exec(dtk_htex video, int command)
{
	dtk_hpipe pl = (dtk_hpipe) video->aux;

	switch (command) {
	case DTKV_CMD_STOP:
		stop_pipeline(pl);
		return true;

	case DTKV_CMD_PLAY:
		return run_pipeline(pl);

	case DTKV_CMD_PAUSE:
		return pause_pipeline(pl);

	default:
		return false;
	}
}
