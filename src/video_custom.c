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
#include <assert.h>
#include <string.h>

#include <gst/gst.h>
#include <glib.h>
#include <gst/app/gstappsink.h>

#include "dtk_video.h"
#include "video_structs.h"
#include "video_custom.h"

#include "drawtk.h"
#include "texmanager.h"
#include "shapes.h"
#include "window.h"

#define DTK_GST_VERTICAL_FLIP 5


/**************************************************************************
 *                         Gstreamer callbacks                            *
 **************************************************************************/
static
int alloc_compatible_image(GstBuffer* buffer, struct dtk_texture* tex)
{
	GstCaps *caps;
	GstStructure *structure;
	int h, w;

	caps = gst_buffer_get_caps(buffer);
	if (!caps) {
		fprintf(stderr, "could not get caps for the buffer\n");
		return -1;
	}

	structure = gst_caps_get_structure(caps, 0);
	gst_structure_get_int(structure, "height", &h);
	gst_structure_get_int(structure, "width", &w);

	tex->intfmt = GL_RGB;
	tex->fmt = GL_RGB;
	tex->type = GL_UNSIGNED_BYTE;
	alloc_image_data(tex, w, h, 0, 24);

	gst_caps_unref(caps);

	return 0;
}


static
GstFlowReturn newbuffer_callback(GstAppSink *sink,  gpointer data)
{
	unsigned char *tdata, *bdata;
	unsigned int tstride, bstride, h, i;
	GstBuffer* buffer; 
	dtk_htex tex = (dtk_htex) data;
	dtk_hpipe pipe = (dtk_hpipe) tex->aux;

	pthread_mutex_lock(&(tex->lock));
	buffer = gst_app_sink_pull_buffer(sink);
	if (tex->sizes == NULL) 
		alloc_compatible_image(buffer, tex);

	if (pipe->status == DTKV_READY)
		pipe->status = DTKV_PLAYING;

	// load data into memory (gstreamer is flipped in GL conventions)
	h = tex->sizes[0].h;
	tstride = tex->sizes[0].stride;
	bstride = tex->sizes[0].w*3;
	tdata = tex->data[0];
	bdata = GST_BUFFER_DATA(buffer) + (h-1)*bstride;
	for (i=0; i<h; i++) {
		memcpy(tdata, bdata, bstride);
		tdata += tstride;
		bdata -= bstride;
	}

	gst_buffer_unref(buffer);
	tex->isinit = false;

	pthread_mutex_unlock(&(tex->lock));
	return GST_FLOW_OK;
}


static
void newpad_callback(GstElement * element, GstPad * pad, gpointer data)
{
	GstPad *sinkpad;
	GstCaps *sourcecaps;
	GstCaps *sinkcaps;
	struct dtk_pipe_element *sink = data;
	struct dtk_pipe_element *source = sink->prev;

	// get source pad & sink-pad capabilities
	sinkpad = gst_element_get_static_pad(sink->gElement, "sink");
	sourcecaps = gst_pad_get_caps(pad);
	sinkcaps = gst_pad_get_caps(sinkpad);

	// check that :
	// source and source->gCaps are compatible
	// source & sink caps are compatible

	bool capsCompatible = (source->gCaps != NULL)
	    && gst_caps_can_intersect(source->gCaps, sinkcaps);
	bool capsCompatible2 = (source->gCaps == NULL)
	    && gst_caps_can_intersect(sourcecaps, sinkcaps);

	bool isVideo =
	    strstr(gst_caps_to_string(sourcecaps), "video") != NULL;

	if (capsCompatible || capsCompatible2) {
		if (source->gCaps)
			gst_pad_set_caps(pad, source->gCaps);
		gst_pad_link(pad, sinkpad);
	} else if (isVideo) {
		// output error and abort, only if we're trying to process
		// video... audio is ignored
		fprintf(stderr, "Source (%s) and sink pads could not be "
		                "connected - incompatible caps.\n",
		                gst_element_get_name(element));
	}
	// unreference data
	gst_caps_unref(sourcecaps);
	gst_caps_unref(sinkcaps);
	gst_object_unref(sinkpad);
}


/**************************************************************************
 *                          Pipeline build                                *
 **************************************************************************/
static
void add_terminal_elements(dtk_hpipe pipe)
{
	GstCaps *caps;

	dtk_pipe_add_element(pipe, "ffmpegcolorspace", "converter");

	caps = gst_caps_new_simple("video/x-raw-rgb",
				   "bpp", G_TYPE_INT, 24,
				   "red_mask", G_TYPE_INT, 0xFF0000,
				   "green_mask", G_TYPE_INT, 0x00FF00,
				   "blue_mask", G_TYPE_INT, 0x0000FF, NULL);
	dtk_pipe_add_element_full(pipe, "appsink", "drawtksink",
	                                "caps", caps, NULL);
	gst_caps_unref(caps);
}


static
void link_pipe_elements(struct dtk_pipe_element *source,
			struct dtk_pipe_element *sink)
{
	GstElementClass *klass;
	GList *padlist;
	GstPadTemplate *tpl;

	// retrieve pad templates for source element
	klass = (GstElementClass *) G_OBJECT_GET_CLASS(source->gElement);
	padlist = gst_element_class_get_pad_template_list(klass);

	// iterate over pad templates
	for (; padlist != NULL; padlist = padlist->next) {
		tpl = padlist->data;

		// skip if it's not an ALWAYS/SOMETIMES source pad
		if (tpl->direction != GST_PAD_SRC
		    || tpl->presence == GST_PAD_REQUEST)
			continue;

		switch (tpl->presence) {
		case GST_PAD_ALWAYS:
			if (source->gCaps != NULL)
				gst_element_link_filtered(source->gElement,
							  sink->gElement,
							  source->gCaps);
			else
				gst_element_link(source->gElement,
						 sink->gElement);
			return;

		case GST_PAD_SOMETIMES:
			g_signal_connect(source->gElement, "pad-added",
					 G_CALLBACK(newpad_callback),
					 sink);
			return;
		default:
			break;
		}
	}
}


static
void setup_pipe_links(dtk_hpipe pipe)
{
	struct dtk_pipe_element *curElement = pipe->dtkElement;

	while (curElement->prev != NULL) {
		link_pipe_elements(curElement->prev, curElement);
		curElement = curElement->prev;
	}
}


static
dtk_htex create_video_texture(dtk_hpipe pipe)
{
	dtk_htex tex = NULL;

	// Get pipeline name
	char stringid[256];
	char *pipeName = gst_element_get_name(pipe->gPipe);
	snprintf(stringid, sizeof(stringid), "PIPELINE:%s", pipeName);
	g_free(pipeName);

	// Create new texture
	if ((tex = get_texture(stringid)) == NULL)
		return NULL;

	tex->id = 0;
	tex->isvideo = true;
	tex->sizes = NULL;

	return tex;
}


static
void destroyPipeline(dtk_htex tex)
{
	dtk_hpipe dtkPipe = (dtk_hpipe) tex->aux;

	// set pipeline status to dead
	dtkPipe->status = DTKV_STOPPED;

	// join the threads
	pthread_join(dtkPipe->thread, NULL);

	// lock status
	//pthread_mutex_lock(&(dtkPipe->status_lock));

	// kill bus
	gst_element_set_state(dtkPipe->gPipe, GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(dtkPipe->gBus));

	// delete each dtk_pipe_element
	while (dtkPipe->dtkElement != NULL) {
		struct dtk_pipe_element *cur = dtkPipe->dtkElement;

		dtkPipe->dtkElement = dtkPipe->dtkElement->prev;

		if (cur->gCaps != NULL)
			gst_object_unref(GST_OBJECT(cur->gCaps));

		free(cur);
	}

	// kill pipeline
	gst_object_unref(GST_OBJECT(dtkPipe->gPipe));

	// unlock status
	//pthread_mutex_unlock(&(dtkPipe->status_lock));

	// delete dtk_pipeline
	free(tex->aux);

	tex->aux = NULL;
}


LOCAL_FN
dtk_htex dtk_create_video_from_pipeline(dtk_hpipe customPipe)
{
	dtk_htex tex;
	GstAppSink* appsink;
	GstAppSinkCallbacks callbacks = {
		.new_buffer = newbuffer_callback
	};

	if (!customPipe)
		return NULL;

	// complete the pipeline
	add_terminal_elements(customPipe);
	setup_pipe_links(customPipe);

	// create video texture
	tex = create_video_texture(customPipe);

	// Install callback to the appsink
	appsink = GST_APP_SINK(customPipe->dtkElement->gElement);
	gst_app_sink_set_callbacks(appsink, &callbacks, tex, NULL);

	tex->aux = (void *) customPipe;
	tex->destroyfn = &(destroyPipeline);
	return tex;
}


// OUTPUT TEXTURE CREATION
LOCAL_FN 
dtk_hpipe dtk_create_video_pipeline(const char *name)
{
	// INITIALIZATION
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
	// CREATE PIPELINE
	dtk_hpipe dtkPipeline = malloc(sizeof(struct dtk_pipeline));
	dtkPipeline->gPipe = gst_pipeline_new(name);
	dtkPipeline->gBus = NULL;
	dtkPipeline->dtkElement = NULL;
	dtkPipeline->editLocked = false;

	// check for errors in pipeline creation
	if (dtkPipeline->gPipe == NULL) {
		free(dtkPipeline);
		return NULL;
	}
	// add bus to pipeline
	dtkPipeline->gBus =
	    gst_pipeline_get_bus(GST_PIPELINE(dtkPipeline->gPipe));

	// add pipeline status
	dtkPipeline->thread = (pthread_t) NULL;
	dtkPipeline->status = DTKV_STOPPED;
	// .. and status mutex
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	dtkPipeline->status_lock = mutex;

	return dtkPipeline;
}


LOCAL_FN
    bool dtk_pipe_add_element(dtk_hpipe pipe, const char *fact,
			      const char *nam)
{
	return dtk_pipe_add_element_full(pipe, fact, nam, NULL);
}


LOCAL_FN
    bool dtk_pipe_add_element_full(dtk_hpipe pipe,
				   const char *factory,
				   const char *name,
				   const char *firstPropertyName,...)
{
	bool retValue;
	va_list paramList;

	va_start(paramList, firstPropertyName);
	retValue =
	    dtk_pipe_add_element_full_valist(pipe, factory, name,
					     firstPropertyName, paramList);
	va_end(paramList);

	return retValue;
}


LOCAL_FN
    bool dtk_pipe_add_element_full_valist(dtk_hpipe dtkPipe,
					  const char *factory,
					  const char *name,
					  const char *firstPropertyName,
					  va_list paramList)
{
	struct dtk_pipe_element *elem;
	GstElement *gstelem;

	// create element
	gstelem = gst_element_factory_make(factory, name);

	// abort on failure
	if (gstelem == NULL) {
		fprintf(stderr,"Failed on creating element : %s - %s\n",
			   factory, name);
		return false;
	}

	// if properties are defined, set element properties
	if (firstPropertyName != NULL)
		g_object_set_valist(G_OBJECT(gstelem), firstPropertyName,
				    paramList);

	// add object to global bin
	gst_bin_add(GST_BIN(dtkPipe->gPipe), gstelem);

	// create & fill dtk_pipe_element structure
	elem = malloc(sizeof(struct dtk_pipe_element));
	elem->gElement = gstelem;
	elem->prev = dtkPipe->dtkElement;
	elem->next = NULL;
	elem->gCaps = NULL;

	// fill previous dtk_pipe_element.next field
	if (dtkPipe->dtkElement != NULL)
		dtkPipe->dtkElement->next = elem;
	// put element on top
	dtkPipe->dtkElement = elem;

	return true;
}

