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

	if (port < 1 || strlen(server) == 0)
		return NULL;

	sprintf(pipeName, "TCP:%s:%d", server, port);
	dtk_hpipe pipe = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pipe, "tcpclientsrc", "tcp-src",
				  "host", server, "port", port, NULL);
	dtk_pipe_add_element(pipe, "queue", "queue");
	dtk_pipe_add_element(pipe, "decodebin2", "decoder-bin");

	return pipe;
}


static
dtk_hpipe dtk_create_udp_pipeline(int port)
{
	char pipeName[255];

	if (port < 1)
		return NULL;

	sprintf(pipeName, "UDP:%d", port);
	dtk_hpipe pipe = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pipe, "udpsrc", "udp-src",
				  "port", port, NULL);
	dtk_pipe_add_element(pipe, "queue", "data-queue");
	dtk_pipe_add_element(pipe, "decodebin2", "decoder-bin");

	return pipe;
}


static
dtk_hpipe dtk_create_file_pipeline(const char *file)
{
	char pipeName[255];

	if (strlen(file) == 0)
		return NULL;

	sprintf(pipeName, "FILE:%s", file);
	dtk_hpipe pipe = dtk_create_video_pipeline(pipeName);
	dtk_pipe_add_element_full(pipe, "filesrc", "file-src",
				  "location", file, NULL);
	dtk_pipe_add_element(pipe, "queue", "queue");
	dtk_pipe_add_element(pipe, "decodebin2", "decoder-bin");

	return pipe;
}


static
dtk_hpipe dtk_create_test_pipeline(void)
{
	dtk_hpipe pipe = dtk_create_video_pipeline("test-pipe");
	dtk_pipe_add_element(pipe, "videotestsrc", "test-src");

	return pipe;
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
		g_print("End of stream\n");
		return false;

	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &error, NULL);
		g_printerr("Error: %s\n", error->message);
		g_error_free(error);
		return false;

	default:
		return true;
	}
}


static
void *run_pipeline_loop(void *pipe)
{
	GstMessage *msg = NULL;
	dtk_hpipe dtkPipe = (dtk_hpipe) pipe;

	// main loop
	bool canLoop = true;

	while (canLoop && dtkPipe->status != DTKV_STOPPED) {

		if (pthread_mutex_trylock(&(dtkPipe->status_lock)) == 0) {
			// Read new messages from bus
			while (canLoop && (msg = gst_bus_pop(dtkPipe->gBus))) {
				// Call the bus callback for each new message
				canLoop = bus_callback(msg);

				// Clean the message
				gst_message_unref(msg);
			}

			// if the bus hasn't requested termination, unlock 
			if (canLoop)
				pthread_mutex_unlock(&(dtkPipe-> status_lock));
		}
	}

	// set status to STOPPED
	dtkPipe->status = DTKV_STOPPED;

	// disable pipeline
	g_print("Set pipeline to READY\n");
	gst_element_set_state(dtkPipe->gPipe, GST_STATE_READY);

	// if the bus HAS requested termination, unlock only now
	if (!canLoop)
		pthread_mutex_unlock(&(dtkPipe->status_lock));

	return NULL;
}


static
bool run_pipeline(dtk_hpipe pipe)
{
	bool wasPaused;
	GstStateChangeReturn ret;
	struct dtk_timespec delay = { 0, 50000000 };	// 50 ms

	pthread_mutex_lock(&(pipe->status_lock));

	if (pipe->status == DTKV_PLAYING) {
		pthread_mutex_unlock(&(pipe->status_lock));
		return true;
	}

	wasPaused = (pipe->status == DTKV_PAUSED);

	// prepare pipeline status
	pipe->status = DTKV_READY;

	// execute state change and wait for state change to take effect
	ret = gst_element_set_state(pipe->gPipe, GST_STATE_PLAYING);
	while (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(pipe->gPipe, NULL, NULL,
					  500 * GST_MSECOND);

	// handle failure
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_print("failed to run pipeline\n");
		pthread_mutex_unlock(&(pipe->status_lock));
		return false;
	}
	// handle case in which pipeline was simply paused
	if (wasPaused) {
		pipe->status = DTKV_PLAYING;
		pthread_mutex_unlock(&(pipe->status_lock));
		return true;
	}

	pthread_mutex_unlock(&(pipe->status_lock));

	// launch execution thread
	pthread_create(&(pipe->thread), NULL, run_pipeline_loop, pipe);

	// wait till the state changes
	while (pipe->status == DTKV_READY)
		dtk_nanosleep(0, &delay, NULL);

	return (pipe->status == DTKV_PLAYING);
}


static
void stop_pipeline(dtk_hpipe pipe)
{
	pthread_mutex_lock(&(pipe->status_lock));

	pipe->status = DTKV_STOPPED;
	pthread_join(pipe->thread, NULL);

	pthread_mutex_unlock(&(pipe->status_lock));
}


static
bool pause_pipeline(dtk_hpipe pipe)
{
	GstStateChangeReturn ret;
	pthread_mutex_lock(&(pipe->status_lock));

	if (pipe->status != DTKV_PLAYING) {
		pthread_mutex_unlock(&(pipe->status_lock));
		return false;
	}
	// Change state and wait for state change to take effect
	ret = gst_element_set_state(pipe->gPipe, GST_STATE_PAUSED);
	while (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(pipe->gPipe, NULL, NULL,
					    500 * GST_MSECOND);

	if (ret == GST_STATE_CHANGE_FAILURE) {
		pthread_mutex_unlock(&(pipe->status_lock));
		return false;
	}
	// set pipeline status to paused
	pipe->status = DTKV_PAUSED;

	pthread_mutex_unlock(&(pipe->status_lock));

	return true;
}


/**************************************************************************
 *                    Video related API implementation                    *
 **************************************************************************/
API_EXPORTED dtk_htex dtk_create_video(int feed_type, bool autostart,...)
{
	dtk_hpipe pipe = NULL;
	dtk_htex videoTex = NULL;
	va_list args;

	va_start(args, autostart);
	switch (feed_type) {
	case DTKV_FEED_TCP:
		pipe = dtk_create_tcp_pipeline(va_arg(args, const char *),
					       va_arg(args, int));
		break;

	case DTKV_FEED_UDP:
		pipe = dtk_create_udp_pipeline(va_arg(args, int));
		break;

	case DTKV_FEED_FILE:
		pipe =
		    dtk_create_file_pipeline(va_arg(args, const char *));
		break;

	case DTKV_FEED_TEST:
		pipe = dtk_create_test_pipeline();
		break;
	}
	va_end(args);

	videoTex = dtk_create_video_from_pipeline(pipe);
	if (autostart && videoTex)
		dtk_video_exec(videoTex, DTKV_CMD_PLAY);

	return videoTex;
}


API_EXPORTED int dtk_video_getstate(dtk_htex video)
{
	dtk_hpipe pipe = (dtk_hpipe) video->aux;

	return pipe->status;
}


API_EXPORTED bool dtk_video_exec(dtk_htex video, int command)
{
	dtk_hpipe pipe = (dtk_hpipe) video->aux;

	switch (command) {
	case DTKV_CMD_STOP:
		stop_pipeline(pipe);
		return true;

	case DTKV_CMD_PLAY:
		return run_pipeline(pipe);

	case DTKV_CMD_PAUSE:
		return pause_pipeline(pipe);

	default:
		return false;
	}
}
