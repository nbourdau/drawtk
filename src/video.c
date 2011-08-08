/*
    Copyright (C) 2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
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

static GstFlowReturn newbuffer_callback(GstAppSink *sink,  gpointer data);
static GstFlowReturn preroll_callback(GstAppSink *sink,  gpointer data);
static void eos_callback(GstAppSink *sink,  gpointer data);

static GstAppSinkCallbacks sink_callbacks = {
	.eos = eos_callback,
	.new_preroll = preroll_callback,
	.new_buffer = newbuffer_callback
};


static
void init_gstreamer(void)
{
	static bool is_initialized = false;
	if (!is_initialized) {
		// initialize glib threading
		if (!g_thread_supported()) 
			g_thread_init(NULL);
		// initialize gstreamer
		gst_init(NULL, NULL);

		// set sentinel
		is_initialized = true;
	}
}


/**************************************************************************
 *                          Pipeline execution                            *
 **************************************************************************/
static
int change_pipeline_state(struct videoaux* aux, GstState state)
{
	GstStateChangeReturn ret;
	GstClockTime timeout = 500 * GST_MSECOND;

	ret = gst_element_set_state(aux->pipe, state);
	while (ret == GST_STATE_CHANGE_ASYNC)
		ret = gst_element_get_state(aux->pipe, NULL, NULL, timeout);

	return (ret != GST_STATE_CHANGE_FAILURE) ? 0 : -1;
}


static
void update_texture_image(GstBuffer* buffer, struct dtk_texture* tex)
{
	unsigned char *tdata, *bdata;
	unsigned int tstride, bstride, h, i;

	// load data into memory (gstreamer is flipped in GL conventions)
	h = tex->sizes[0].h;
	tstride = tex->sizes[0].stride;
	bstride = tex->sizes[0].w*3;
	tdata = tex->data[0];
	bdata = GST_BUFFER_DATA(buffer) + (h-1)*bstride;

	pthread_mutex_lock(&(tex->lock));
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
GstFlowReturn async_image_alloc(GstAppSink *sink,  gpointer data)
{
	struct async_alloc_data* asdat = data;

	alloc_compatible_image(sink, asdat->tex);
	
	// Signal that everything is ready
	pthread_mutex_lock(&asdat->lock);
	asdat->isalloc = true;
	pthread_cond_signal(&asdat->cond);
	pthread_mutex_unlock(&asdat->lock);

	return GST_FLOW_OK;
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


static
int init_video_tex(struct dtk_texture* tex, GstElement* pipe)
{
	struct async_alloc_data asdata = { .tex = tex, .isalloc = false };
	GstAppSinkCallbacks tmp_cb = { .new_buffer = async_image_alloc };
	GstAppSink* sink;
	GstCaps* caps;
	struct videoaux* aux;

	// Configure sink
	sink = GST_APP_SINK(gst_bin_get_by_name(GST_BIN(pipe), "dtksink"));
	caps = gst_caps_new_simple("video/x-raw-rgb",
				   "bpp", G_TYPE_INT, 24,
				   "red_mask", G_TYPE_INT, 0xFF0000,
				   "green_mask", G_TYPE_INT, 0x00FF00,
				   "blue_mask", G_TYPE_INT, 0x0000FF, NULL);
	gst_app_sink_set_caps(sink, caps);
	gst_caps_unref(caps);
	gst_app_sink_set_callbacks(sink, &sink_callbacks, tex, NULL);

	aux = malloc(sizeof(*aux));
	aux->pipe = pipe;
	aux->state = 0;
	pthread_mutex_init(&aux->lock, NULL);
	tex->id = 0;
	tex->isvideo = true;
	tex->aux = aux;
	tex->destroyfn = &(destroyPipeline);
	
	// Prepare pipeline execution
	if (change_pipeline_state(aux, GST_STATE_READY)
	    || change_pipeline_state(aux, GST_STATE_PAUSED))
		return -1;
	
	// Try to allocate image without touching the gstreamer pipeline
	if (!alloc_compatible_image(sink, tex))
		return 0;

	// Previous failed so start the pipeline and allocate
	// asynchronously when the first image is available
	pthread_mutex_init(&asdata.lock, NULL);
	pthread_cond_init(&asdata.cond, NULL);
	gst_app_sink_set_callbacks(sink, &tmp_cb, &asdata, NULL);
	change_pipeline_state(aux, GST_STATE_PLAYING);

	// Wait the first buffer to arrive and allocate the image ressource
	pthread_mutex_lock(&asdata.lock);
	while (!asdata.isalloc)
		pthread_cond_wait(&asdata.cond, &asdata.lock);
	pthread_mutex_unlock(&asdata.lock);
	
	// Reset everything
	change_pipeline_state(aux, GST_STATE_PAUSED);
	gst_app_sink_set_callbacks(sink, &sink_callbacks, tex, NULL);
	pthread_mutex_destroy(&asdata.lock);
	pthread_cond_destroy(&asdata.cond);

	return 0;
}


static
dtk_htex create_video_any(int type, const union pipeopt* opt,
                          const char* stringid, int flags)
{
	dtk_htex tex;
	GstElement* pipe;

	if ((tex = get_texture(stringid)) == NULL)
		return NULL;

	pthread_mutex_lock(&(tex->lock));
	if (!tex->data) {
		init_gstreamer();
		pipe = create_pipeline(type, opt);
		init_video_tex(tex, pipe);
	}
	pthread_mutex_unlock(&(tex->lock));

	if (tex && (flags & DTK_AUTOSTART))
		dtk_video_exec(tex, DTKV_CMD_PLAY);
		
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
int dtk_video_exec(dtk_htex video, int command)
{
	bool r;
	GstState gstate;
	struct videoaux* aux = video->aux;
	int flag = GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT;

	if (!video->isvideo)
		return -1;
	
	switch (command) {
	case DTKV_CMD_REWIND:
		r = gst_element_seek_simple(aux->pipe, GST_FORMAT_TIME,
		                            flag, 0);
		if (!r)
			return -1;
		pthread_mutex_lock(&aux->lock);
		aux->state &= ~DTKV_EOS;
		pthread_mutex_unlock(&aux->lock);
		return 0;

	case DTKV_CMD_PLAY:
	case DTKV_CMD_PAUSE:
		gstate = (command == DTKV_CMD_PLAY) ? 
		                     GST_STATE_PLAYING : GST_STATE_PAUSED;
		if (change_pipeline_state(aux, gstate))
			return -1;

		pthread_mutex_lock(&aux->lock);
		if (gstate == GST_STATE_PAUSED)
			aux->state &= ~DTKV_PLAYING;
		else if (gstate == GST_STATE_PLAYING)
			aux->state |= DTKV_PLAYING;
		pthread_mutex_unlock(&aux->lock);
		return 0;

	default:
		return -1;
	}
}

