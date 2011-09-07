/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
    Pierluca Borsò <pierluca.borso@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef G_DISABLE_CAST_CHECKS
#define G_DISABLE_CAST_CHECKS	1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

#include <gst/gst.h>
#include <glib.h>
#include <gst/app/gstappsink.h>

#include "vidpipe_creation.h"
#include "texmanager.h"
#include "dtk_video.h"

#define DTK_CH_ASYNC	1
#define DTK_NO_PREROLL	2

struct videoaux
{
	GstElement* pipe;
	pthread_mutex_t lock;
	int state;
};


struct async_alloc_data
{
	pthread_mutex_t lock;
	pthread_cond_t cond;
	struct dtk_texture* tex;
	bool isalloc;
};

static int alloc_compatible_image(GstAppSink* sink, struct dtk_texture* tex);
static GstFlowReturn newbuffer_callback(GstAppSink *sink,  gpointer data);
static GstFlowReturn preroll_callback(GstAppSink *sink,  gpointer data);
static GstFlowReturn newbuffer_uninit_cb(GstAppSink *sink,  gpointer data);
static GstFlowReturn preroll_uninit_cb(GstAppSink *sink,  gpointer data);
static void eos_callback(GstAppSink *sink,  gpointer data);

static GstAppSinkCallbacks sink_callbacks = {
	.eos = eos_callback,
	.new_preroll = preroll_callback,
	.new_buffer = newbuffer_callback
};

static GstAppSinkCallbacks sink_uninit_callbacks = {
	.eos = eos_callback,
	.new_preroll = preroll_uninit_cb,
	.new_buffer = newbuffer_uninit_cb
};


/**************************************************************************
 *                          Pipeline execution                            *
 **************************************************************************/
static
int set_pipe_state(struct videoaux* aux, GstState state, int noblock)
{
	GstStateChangeReturn ret;
	GstClockTime timeout = noblock ? 0 : -1;

	ret = gst_element_set_state(aux->pipe, state);
	if (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(aux->pipe, NULL, NULL, timeout);

	if (ret == GST_STATE_CHANGE_ASYNC)
		return DTK_CH_ASYNC;
	if (ret == GST_STATE_CHANGE_NO_PREROLL)
		return DTK_NO_PREROLL;

	return (ret != GST_STATE_CHANGE_FAILURE) ? 0 : -1;
}


static
void update_texture_image(GstBuffer* buffer, struct dtk_texture* tex)
{
	unsigned char *tdata, *bdata;
	unsigned int tstride, bstride, h, i;

	// load data into memory (gstreamer is flipped in GL conventions)
	h = tex->data[0].h;
	tstride = tex->data[0].stride;
	bstride = tex->data[0].w*3;
	bdata = GST_BUFFER_DATA(buffer) + (h-1)*bstride;

	pthread_mutex_lock(&(tex->lock));
	tdata = tex->bmdata;
	for (i=0; i<h; i++) {
		memcpy(tdata, bdata, bstride);
		tdata += tstride;
		bdata -= bstride;
	}
	tex->outdated = true;
	pthread_mutex_unlock(&(tex->lock));
}


static 
void eos_callback(GstAppSink *sink,  gpointer data)
{
	dtk_htex tex = data;
	struct videoaux* aux = tex->aux;
	(void)sink;

	pthread_mutex_lock(&aux->lock);
	aux->state |= DTKV_EOS;
	pthread_mutex_unlock(&aux->lock);
}


static
GstFlowReturn preroll_callback(GstAppSink *sink,  gpointer data)
{
	GstBuffer* buffer; 
	dtk_htex tex = data;

	if (!tex->data)
		return GST_FLOW_OK;

	buffer = gst_app_sink_pull_preroll(sink);
	update_texture_image(buffer, tex);
	gst_buffer_unref(buffer);

	return GST_FLOW_OK;
}


static
GstFlowReturn newbuffer_callback(GstAppSink *sink,  gpointer data)
{
	GstBuffer* buffer; 
	dtk_htex tex = data;

	buffer = gst_app_sink_pull_buffer(sink);
	update_texture_image(buffer, tex);
	gst_buffer_unref(buffer);

	return GST_FLOW_OK;
}


static
int try_alloc_image(GstAppSink *sink, struct dtk_texture* tex)
{
	int ret = 0;

	pthread_mutex_lock(&tex->lock);
	if (!tex->data) {
		ret = alloc_compatible_image(sink, tex);
		if (!ret)
			gst_app_sink_set_callbacks(sink, &sink_callbacks,
			                                         tex, NULL);
	}
	pthread_mutex_unlock(&tex->lock);

	return ret;
}


static
GstFlowReturn preroll_uninit_cb(GstAppSink *sink,  gpointer data)
{
	struct dtk_texture* tex = data;

	if (try_alloc_image(sink, tex))
		return GST_FLOW_OK;
	return preroll_callback(sink, data);
}


static
GstFlowReturn newbuffer_uninit_cb(GstAppSink *sink,  gpointer data)
{
	struct dtk_texture* tex = data;

	if (try_alloc_image(sink, tex))
		return GST_FLOW_OK;
	return newbuffer_callback(sink, data);
}


static
int pipeline_seek(struct videoaux* aux, gint64 pos)
{
	int flag = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT;

	if (!gst_element_seek_simple(aux->pipe, GST_FORMAT_TIME, flag, pos))
		return -1;

	pthread_mutex_lock(&aux->lock);
	aux->state &= ~DTKV_EOS;
	pthread_mutex_unlock(&aux->lock);
	return 0;
}


static
int pipeline_play_pause(struct videoaux* aux, int playing, int noblock)
{
	GstState gstate = GST_STATE_PAUSED;

	if (playing)
		gstate = GST_STATE_PLAYING;

	if (set_pipe_state(aux, gstate, noblock) < 0)
		return -1;

	pthread_mutex_lock(&aux->lock);
	if (playing)
		aux->state |= DTKV_PLAYING;
	else
		aux->state &= ~DTKV_PLAYING;
	pthread_mutex_unlock(&aux->lock);

	return 0;
}


/**************************************************************************
 *                           Pipeline creation                            *
 **************************************************************************/
static
int alloc_compatible_image(GstAppSink* sink, struct dtk_texture* tex)
{
	int h,w;
	GstCaps* caps;
	GstStructure* structure;

	// Get negotiated caps (NULL if not negotiated yet)
	caps = GST_PAD_CAPS(GST_BASE_SINK_PAD(sink));
	if (!caps)
		return -1;

	// Get negotiated frame size
	structure = gst_caps_get_structure(caps, 0);
	gst_structure_get_int(structure, "height", &h);
	gst_structure_get_int(structure, "width", &w);

	// Allocate image data
	tex->intfmt = GL_RGB;
	tex->fmt = GL_RGB;
	tex->type = GL_UNSIGNED_BYTE;
	alloc_image_data(tex, w, h, 0, 24);

	return 0;
}


static
void wait_for_data_alloc(struct dtk_texture* tex, int live)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	if (live)
		set_pipe_state(tex->aux, GST_STATE_PLAYING, 0);

	// Wait until the image data has been allocated
	pthread_mutex_lock(&tex->lock);
	while (!tex->data)
		pthread_cond_wait(&cond, &tex->lock);
	pthread_mutex_unlock(&tex->lock);
	
	if (live)
		set_pipe_state(tex->aux, GST_STATE_PAUSED, 0);

	pthread_cond_destroy(&cond);
}


static
void destroyPipeline(dtk_htex tex)
{
	struct videoaux* aux = tex->aux;

	// set pipeline status to dead
	gst_element_set_state(aux->pipe, GST_STATE_READY);
	gst_element_set_state(aux->pipe, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(aux->pipe));
	
	pthread_mutex_destroy(&aux->lock);
	free(aux);
	tex->aux = NULL;
}


// Assume holding tex->lock
static
int init_video_tex(struct dtk_texture* tex, GstElement* pipe, int noblock)
{
	GstAppSink* sink;
	GstCaps* caps;
	struct videoaux* aux;
	int r, retval = 0;

	// Configure sink
	sink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipe), "dtksink"));
	caps = gst_caps_new_simple("video/x-raw-rgb",
				   "bpp", G_TYPE_INT, 24,
				   "red_mask", G_TYPE_INT, 0xFF0000,
				   "green_mask", G_TYPE_INT, 0x00FF00,
				   "blue_mask", G_TYPE_INT, 0x0000FF, NULL);
	gst_app_sink_set_caps(sink, caps);
	gst_caps_unref(caps);
	gst_app_sink_set_max_buffers(sink, 2);
	gst_app_sink_set_drop(sink, TRUE);
	gst_app_sink_set_callbacks(sink, &sink_uninit_callbacks, tex, NULL);

	aux = malloc(sizeof(*aux));
	aux->pipe = pipe;
	aux->state = 0;
	pthread_mutex_init(&aux->lock, NULL);
	tex->id = 0;
	tex->isvideo = true;
	tex->aux = aux;
	tex->destroyfn = &(destroyPipeline);
	
	// Prepare pipeline execution
	pthread_mutex_unlock(&tex->lock);
	if (set_pipe_state(aux, GST_STATE_READY, noblock) < 0
	  || (r = set_pipe_state(aux, GST_STATE_PAUSED, noblock)) < 0) 
	  	retval = -1;
	else if (!noblock)
		wait_for_data_alloc(tex, (r == DTK_NO_PREROLL));
	pthread_mutex_lock(&tex->lock);

	return retval;
}


static
dtk_htex create_video_any(int type, const union pipeopt* opt,
                          const char* stringid, int flags)
{
	dtk_htex tex;
	GstElement* pipe;
	int noblock = flags & DTK_NOBLOCKING;

	if ((tex = get_texture(stringid)) == NULL)
		return NULL;

	pthread_mutex_lock(&(tex->lock));
	if (!tex->aux) {
		pipe = create_pipeline(type, opt);
		init_video_tex(tex, pipe, noblock);
	}
	pthread_mutex_unlock(&(tex->lock));

	if (tex && (flags & DTK_AUTOSTART))
		pipeline_play_pause(tex->aux, 1, noblock);
		
	return tex;
}


/**************************************************************************
 *                    Video related API implementation                    *
 **************************************************************************/
API_EXPORTED
dtk_htex dtk_load_video_tcp(int flags, const char *server, int port)
{
	union pipeopt opt[] = {{.strval = server}, {.intval = port}};
	char stringid[255];

	if (port < 1 || !server)
		return NULL;

	sprintf(stringid, "TCP:%s:%d", server, port);
	return create_video_any(VTCP, opt, stringid, flags);
}


API_EXPORTED
dtk_htex dtk_load_video_udp(int flags, int port)
{
	union pipeopt opt = {.intval = port};
	char stringid[255];

	if (port < 1)
		return NULL;

	sprintf(stringid, "UDP:%d", port);
	return create_video_any(VUDP, &opt, stringid, flags);
}


API_EXPORTED
dtk_htex dtk_load_video_file(int flags, const char *file)
{
	union pipeopt opt = {.strval=file};
	char stringid[255];

	if (!file)
		return NULL;

	sprintf(stringid, "FILE:%s", file);
	return create_video_any(VFILE, &opt, stringid, flags);
}


API_EXPORTED
dtk_htex dtk_load_video_gst(int flags, const char* desc)
{
	union pipeopt opt = {.strval=desc};
	char stringid[255];

	if (!desc)
		return NULL;

	sprintf(stringid, "CUSTOM:%s", desc);
	return create_video_any(VCUSTOM, &opt, stringid, flags);
}


API_EXPORTED
dtk_htex dtk_load_video_test(int flags)
{
	return create_video_any(VTEST, NULL, "TESTPIPE", flags);
}


API_EXPORTED
int dtk_video_getstate(dtk_htex video)
{
	int status;
	struct videoaux* aux;

	if (!video->isvideo)
		return -1;
	
	aux = video->aux;
	pthread_mutex_lock(&aux->lock);
	status  = aux->state;
	pthread_mutex_unlock(&aux->lock);

	return status;
}


API_EXPORTED
int dtk_video_exec(dtk_htex video, int command, const void* arg)
{
	int playing;
	gint64 seek_pos = 0;
	int noblock = 0;

	if (!video->isvideo)
		return -1;
	
	switch (command) {
	case DTKV_CMD_SEEK:
		if (arg)
			seek_pos = *((const long*)arg) * GST_MSECOND;
		return pipeline_seek(video->aux, seek_pos);

	case DTKV_CMD_PLAY:
	case DTKV_CMD_PAUSE:
		if (arg)
			noblock = *((const int*)arg);
		playing = (command == DTKV_CMD_PLAY); 
		return pipeline_play_pause(video->aux, playing, noblock);

	default:
		return -1;
	}
}

