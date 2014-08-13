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
#include <stdarg.h>
#include <string.h>

#include <gst/gst.h>
#include <glib.h>
#include <gst/app/gstappsink.h>

#include "vidpipe_creation.h"

#define DTK_GST_VERTICAL_FLIP 5

struct dtk_pipe_element {
	GstElement *gstelt;

	// previous element in linked list
	struct dtk_pipe_element *prev;
};

struct pipeline {
	GstElement* pipe;
	// linked list of elements in the pipeline
	struct dtk_pipe_element* elt;
};


/**************************************************************************
 *                         Gstreamer callbacks                            *
 **************************************************************************/
static
void newpad_callback(GstElement * elt, GstPad * pad, gpointer data)
{
	bool compat, isvid;
	GstPad *sinkpad;
	GstCaps *sourcecaps;
	GstCaps *sinkcaps;
	GstElement* sinkelt = data;

	// get source pad & sink-pad capabilities
	sinkpad = gst_element_get_static_pad(sinkelt, "sink");
	sourcecaps = gst_pad_get_caps(pad);
	sinkcaps = gst_pad_get_caps(sinkpad);

	// check that :
	// source & sink caps are compatible
	compat = gst_caps_can_intersect(sourcecaps, sinkcaps);
	isvid = strstr(gst_caps_to_string(sourcecaps), "video") != NULL;

	if (compat) {
		gst_pad_link(pad, sinkpad);
	} else if (isvid) {
		// output error and abort, only if we're trying to process
		// video... audio is ignored
		fprintf(stderr, "drawtk: Source (%s) and sink pads could "
		                "not be connected - incompatible caps.\n",
		                gst_element_get_name(elt));
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
bool pipe_add_element_full_valist(struct pipeline* pl,
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
		fprintf(stderr,"drawtk: fails creating element : %s - %s\n",
			   factory, name);
		return false;
	}

	// if properties are defined, set element properties
	if (firstPropertyName != NULL)
		g_object_set_valist(G_OBJECT(gstelem), firstPropertyName,
				    paramList);

	// add object to global bin
	gst_bin_add(GST_BIN(pl->pipe), gstelem);

	// create & fill dtk_pipe_element structure
	elem = malloc(sizeof(struct dtk_pipe_element));
	elem->gstelt = gstelem;
	elem->prev = pl->elt;

	// put element on top
	pl->elt = elem;

	return true;
}


static
bool pipe_add_element_full(struct pipeline* pipe,
				const char *factory,
				const char *name,
				const char *firstPropertyName,...)
{
	bool retValue;
	va_list paramList;

	va_start(paramList, firstPropertyName);
	retValue =
	    pipe_add_element_full_valist(pipe, factory, name,
					     firstPropertyName, paramList);
	va_end(paramList);

	return retValue;
}


static
bool pipe_add_element(struct pipeline* pipe, const char *fact,
			  const char *nam)
{
	return pipe_add_element_full(pipe, fact, nam, NULL);
}


static
void link_pipe_elements(struct dtk_pipe_element *source,
			struct dtk_pipe_element *sink)
{
	GstElementClass *klass;
	GList *padlist;
	GstPadTemplate *tpl;

	// retrieve pad templates for source element
	klass = (GstElementClass *) G_OBJECT_GET_CLASS(source->gstelt);
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
			gst_element_link(source->gstelt, sink->gstelt);
			return;

		case GST_PAD_SOMETIMES:
			g_signal_connect(source->gstelt, "pad-added",
					 G_CALLBACK(newpad_callback),
					 sink->gstelt);
			return;
		default:
			break;
		}
	}
}


static
void setup_pipe_links(struct pipeline* pl)
{
	struct dtk_pipe_element *curElement = pl->elt;

	while (curElement->prev != NULL) {
		link_pipe_elements(curElement->prev, curElement);
		curElement = curElement->prev;
	}
}


static
void free_element_list(struct pipeline* pl)
{
	struct dtk_pipe_element *cur;

	// delete each dtk_pipe_element
	while (pl->elt != NULL) {
		cur = pl->elt;
		pl->elt = pl->elt->prev;
		free(cur);
	}
}


LOCAL_FN
GstElement* create_pipeline(int type, const union pipeopt* opt)
{
	GError* error = NULL;
	struct pipeline pl;

	if (type == VCUSTOM) {
		pl.pipe = gst_parse_launch(opt[0].strval, &error);
		if (error) {
			fprintf(stderr, "drawtk: %s\n", error->message);
			g_error_free(error);
		}
		return pl.pipe;
	}

	pl.pipe = gst_pipeline_new("pipeline");
	pl.elt = NULL;

	if (type == VTCP)
		pipe_add_element_full(&pl, "tcpclientsrc", "tcp-src",
				           "host", opt[0].strval, 
				           "port", opt[0].intval, NULL);
	else if (type == VUDP)
		pipe_add_element_full(&pl, "udpsrc", "udp-src",
		                           "port", opt[0].intval, NULL);
	else if (type == VFILE)
		pipe_add_element_full(&pl, "filesrc", "file-src",
		                           "location", opt[0].strval, NULL);
	else if (type == VTEST)
		pipe_add_element(&pl, "videotestsrc", "test-src");
	
	pipe_add_element(&pl, "decodebin2", "decoder-bin");
	pipe_add_element(&pl, "ffmpegcolorspace", "converter");
	pipe_add_element_full(&pl, "appsink", "dtksink", NULL);

	setup_pipe_links(&pl);
	free_element_list(&pl);

	return pl.pipe;
}

