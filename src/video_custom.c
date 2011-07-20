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
#include <gst/base/gstbasetransform.h>

#include "dtk_video.h"
#include "video_structs.h"
#include "dtk_video_custom.h"

#include "drawtk.h"
#include "texmanager.h"
#include "shapes.h"
#include "window.h"

#define DTK_GST_VERTICAL_FLIP 5

////////////////////
//
// DEFINITIONS
//
////////////////////

// CREATE VIDEO TEXTURE
dtk_htex create_video_texture(dtk_hpipe pipe);

// ADD TERMINATION ELEMENTS
void add_terminal_elements(dtk_hpipe pipe);

// SETUP PIPE LINKS
void setup_pipe_links(dtk_hpipe pipe);

// LINK ELEMENTS (SOURCE->SINK)
void link_pipe_elements(struct dtk_pipe_element* source, struct dtk_pipe_element* sink);

// NEW PAD CALLBACK
static void newpad_callback(GstElement* element, GstPad* pad, gpointer data);

// HANDOFF CALLBACK
static void handoff_callback(GstElement* sink, GstBuffer* buffer, gpointer data);

// PIPELINE DESTROY FUNCTION
void destroyPipeline(dtk_htex tex);

// GET PAD SIZE
struct dtk_size* get_pad_size(GstPad* pad);

////////////////////
//
// IMPLEMENTATION
//
////////////////////


API_EXPORTED
dtk_htex dtk_create_video_from_pipeline(dtk_hpipe customPipe)
{
        if(customPipe == 0)
                return 0;

        // complete the pipeline
        add_terminal_elements(customPipe);
        setup_pipe_links(customPipe);

        // create video texture
        dtk_htex tex = create_video_texture(customPipe);

        // get the next to last element
        GstElement* identityElem = customPipe->dtkElement->prev->gElement;

        // assert the element is indeed identity
        GstElementFactory* factory = gst_element_get_factory(identityElem);
        const char* name = gst_element_factory_get_longname(factory);
        if(strcmp(name,"Identity")!=0)
        {
                g_printerr("Pipeline does not end with an identity+sink combination. Next to last element: %s\n",name);
        }
        else
        {
                // connect the handoff signal
                g_signal_connect(identityElem,"handoff",G_CALLBACK(handoff_callback),tex);
        }

        tex->aux = (void*)customPipe;

        tex->destroyfn = &(destroyPipeline);

        //gst_clock_set_calibration(gst_pipeline_get_clock(GST_PIPELINE(customPipe->gPipe)),0,0,1,10);

        return tex;
}

// OUTPUT TEXTURE CREATION
dtk_htex create_video_texture(dtk_hpipe pipe)
{
        dtk_htex tex = NULL;

        // Get pipeline name
        char stringid[256];
        char* pipeName = gst_element_get_name(pipe->gPipe);
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

void add_terminal_elements(dtk_hpipe pipe)
{
        // convert -> flip -> reader -> sink
        dtk_pipe_add_element(pipe, "ffmpegcolorspace", "converter");

        GstCaps* caps = gst_caps_new_simple("video/x-raw-rgb","bpp",G_TYPE_INT,24,"red_mask",G_TYPE_INT,16711680,"green_mask",G_TYPE_INT,65280,"blue_mask",G_TYPE_INT,255,NULL);
        dtk_pipe_add_element_full(pipe, "capsfilter", "rgb-filter", "caps", caps, NULL);

        dtk_pipe_add_element_full(pipe, "videoflip", "flipper", "method", DTK_GST_VERTICAL_FLIP, NULL);

        dtk_pipe_add_element(pipe, "identity", "buffer-reader");
        
        dtk_pipe_add_element(pipe, "fakesink", "fake-sink");
}

API_EXPORTED
dtk_hpipe dtk_create_video_pipeline(const char* name)
{
	// INITIALIZATION
        static bool is_initialized = false;
	if(!is_initialized)
	{
                // initialize glib threading
                if(!g_thread_supported())
                {
                        g_thread_init(NULL);
                }

                // initialize gstreamer
		gst_init (NULL, NULL);

                // set sentinel
		is_initialized = true;
	}

	// CREATE PIPELINE
	dtk_hpipe dtkPipeline = malloc(sizeof(struct dtk_pipeline));
	dtkPipeline->gPipe = gst_pipeline_new (name);
	dtkPipeline->gBus       = NULL;
	dtkPipeline->dtkElement = NULL;
        dtkPipeline->editLocked = false;

	// check for errors in pipeline creation
	if(dtkPipeline->gPipe == NULL)
        {
		free(dtkPipeline);
		return NULL;
	}

	// add bus to pipeline
        dtkPipeline->gBus = gst_pipeline_get_bus(GST_PIPELINE(dtkPipeline->gPipe));

	// add pipeline status
        dtkPipeline->thread = (pthread_t)NULL;
        dtkPipeline->status = DTKV_STOPPED;
        // .. and status mutex
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        dtkPipeline->status_lock = mutex;

	return dtkPipeline;
}

API_EXPORTED
bool dtk_pipe_add_element(
                dtk_hpipe pipe,
                const char* factory,
                const char* name
                )
{
	return dtk_pipe_add_element_full(pipe, factory,name,NULL);
}

API_EXPORTED
bool dtk_pipe_add_element_full(
                dtk_hpipe pipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                ...
                )
{
	bool	retValue;
	va_list	paramList;

	va_start(paramList,firstPropertyName);
	retValue = dtk_pipe_add_element_full_valist(pipe,factory,name,firstPropertyName,paramList);
	va_end(paramList);

	return retValue;
}

API_EXPORTED
bool dtk_pipe_add_element_full_valist(
                dtk_hpipe dtkPipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                va_list paramList
                )
{
	assert(dtkPipe!=NULL);

        if(dtkPipe->editLocked)
        {
                g_printerr("Pipeline is completed and editLocked. New elements cannot be added.");
                return true;
        }

	// create element
	GstElement* gstElem = gst_element_factory_make(factory,name);

	// abort on failure
	if(gstElem==NULL)
	{
		g_printerr("Failed on creating element : %s - %s\n",factory,name);
		assert(false && "Could not create element");
		return false;
	}
        else
        {
                g_print("dtk_pipe_add_element : %s \n",name);
        }

	// if properties are defined
	if(firstPropertyName!=NULL)
	{
		// set element properties
		g_object_set_valist(G_OBJECT(gstElem),firstPropertyName,paramList);
	}

	// add object to global bin
	gst_bin_add(GST_BIN(dtkPipe->gPipe),gstElem);

	// create & fill dtk_pipe_element structure
	struct dtk_pipe_element* newElem = malloc(sizeof(struct dtk_pipe_element));
	newElem->gElement = gstElem;
	newElem->prev = dtkPipe->dtkElement;
	newElem->next = NULL;
        newElem->gCaps = NULL;

        // check if new element is a terminal element
        if(strcmp(factory,"fakesink")==0 || strcmp(factory,"autovideosink")==0)
        {
                dtkPipe->editLocked = true;
        }
        
	// fill previous dtk_pipe_element.next field
	if(dtkPipe->dtkElement != NULL)
	{
		dtkPipe->dtkElement->next = newElem;
        }

	// put element on top
	dtkPipe->dtkElement = newElem;

	return true;
}
/*
void dtk_set_last_element_caps(
        dtk_hpipe pipe,
        const char* mediaType,
        const char* firstParam,
        ...
        )
{
        va_list	paramList;

        va_start(paramList,firstParam);
        dtk_set_last_element_caps_valist(pipe,mediaType,firstParam,paramList);
        va_end(paramList);
}

void dtk_set_last_element_caps_valist(
        dtk_hpipe pipe,
        const char* mediaType,
        const char* firstParam,
        va_list paramList
        )
{
        struct dtk_pipe_element* lastElement = pipe->dtkElement;
        lastElement->gCaps = gst_caps_new_simple(mediaType,NULL);
        gst_caps_set_simple_valist(lastElement->gCaps,firstParam,paramList);
}*/

void setup_pipe_links(dtk_hpipe pipe)
{
        struct dtk_pipe_element* curElement = pipe->dtkElement;
        
        while(curElement->prev != NULL)
        {
                link_pipe_elements(curElement->prev,curElement);
                curElement = curElement->prev;
        }
}

// LINK ELEMENTS (SOURCE->SINK)
void link_pipe_elements(struct dtk_pipe_element* source, struct dtk_pipe_element* sink)
{
        g_print("link_pipe_elements : %s => %s\n",gst_element_get_name(source->gElement),gst_element_get_name(sink->gElement));

        // retrieve pad templates for source element
        GstElementClass *klass = (GstElementClass*)G_OBJECT_GET_CLASS(source->gElement); 
        GList* padTemplates = gst_element_class_get_pad_template_list( klass ); 

        // iterate over pad templates
        for(; padTemplates!=NULL; padTemplates = padTemplates->next)
        {
                // get pad template data
                GstPadTemplate* tpl = padTemplates->data;

                // skip if it's not an ALWAYS/SOMETIMES source pad
                if (tpl->direction != GST_PAD_SRC ||
                        tpl->presence == GST_PAD_REQUEST) {
                                continue;
                }

                switch(tpl->presence)
                {
                case GST_PAD_ALWAYS:
                        if(source->gCaps!=NULL)
                                gst_element_link_filtered(source->gElement,sink->gElement,source->gCaps);
                        else
                                gst_element_link(source->gElement,sink->gElement);
                        return;
                case GST_PAD_SOMETIMES:
                        g_signal_connect (source->gElement, "pad-added", G_CALLBACK (newpad_callback), sink);
                        return;
                default:
                        break;
                }
        }
}

// FAKESINK HANDOFF CALLBACK
static void handoff_callback(GstElement* sink, GstBuffer* buffer, gpointer data)
{
        assert(sink != NULL);
        assert(buffer != NULL);

        dtk_htex tex = (dtk_htex)data;
        dtk_hpipe pipe = (dtk_hpipe)tex->aux;

        pthread_mutex_lock(&(tex->lock));

        if(tex->sizes == NULL)
        {
                tex->mxlvl = 0;
                tex->intfmt = GL_RGB;
                tex->fmt = GL_RGB;
                tex->type = GL_UNSIGNED_BYTE;

                tex->sizes = get_pad_size(GST_BASE_TRANSFORM_SRC_PAD(sink));

                tex->data = malloc(tex->sizes[0].w*tex->sizes[0].h*3);
                memset(tex->data,130,tex->sizes[0].w*tex->sizes[0].h*3);
        }

        if(pipe->status==DTKV_READY)
                pipe->status = DTKV_PLAYING;

        // load data into memory
        memcpy(tex->data,(unsigned char*)GST_BUFFER_DATA(buffer),tex->sizes[0].w*tex->sizes[0].h*3);

        tex->isinit = false;

        pthread_mutex_unlock(&(tex->lock));
}

// NEW PAD CALLBACK
static void newpad_callback (GstElement* element, GstPad* pad, gpointer data)
{
        GstPad* sinkpad = NULL;
        GstCaps *sourcecaps = NULL;
        GstCaps *sinkcaps = NULL;

        // retrieve sink element & Pad
        struct dtk_pipe_element* sink = (struct dtk_pipe_element*)data;
        sinkpad = gst_element_get_static_pad(sink->gElement,"sink");

        // retrieve source
        struct dtk_pipe_element* source = sink->prev;

        // get source pad & sink-pad capabilities
        sourcecaps = gst_pad_get_caps (pad);
        sinkcaps = gst_pad_get_caps(sinkpad);

        // check that :
        // source and source->gCaps are compatible
        // source & sink caps are compatible
        
        bool capsCompatible = (source->gCaps != NULL) && gst_caps_can_intersect(source->gCaps,sinkcaps);
        bool capsCompatible2= (source->gCaps == NULL) && gst_caps_can_intersect(sourcecaps,sinkcaps);

        bool isVideo = strstr(gst_caps_to_string(sourcecaps),"video")!=NULL;

        if(capsCompatible || capsCompatible2)
        {
                if(source->gCaps != NULL)
                {
                        // set capabilities of the pad
                        gst_pad_set_caps(pad,source->gCaps);
                }

                // link source "pad" to sink-pad
                gst_pad_link(pad,sinkpad);
        }
        else if(isVideo)
        {
                // output error and abort, only if we're trying to process video... audio is ignored
                g_printerr("Source (%s) and sink pads could not be connected - incompatible caps.\n",gst_element_get_name(element));
        }

        // unreference data
        gst_caps_unref(sourcecaps);
        gst_caps_unref(sinkcaps);
        gst_object_unref(sinkpad);
}

void destroyPipeline(dtk_htex tex)
{
        dtk_hpipe dtkPipe = (dtk_hpipe)tex->aux;

        // set pipeline status to dead
        dtkPipe->status = DTKV_STOPPED;

        // join the threads
        pthread_join(dtkPipe->thread,NULL);

        // lock status
        //pthread_mutex_lock(&(dtkPipe->status_lock));

        g_print ("Set pipeline to NULL\n");
        gst_element_set_state (dtkPipe->gPipe, GST_STATE_NULL);

        // kill bus
        g_print ("Deleting bus\n");
        gst_object_unref (GST_OBJECT(dtkPipe->gBus));

        // delete each dtk_pipe_element
        g_print ("Deleting dtk_pipe_elements\n");
        while(dtkPipe->dtkElement != NULL)
        {
                struct dtk_pipe_element* cur = dtkPipe->dtkElement;

                dtkPipe->dtkElement = dtkPipe->dtkElement->prev;

                if(cur->gCaps != NULL)
                        gst_object_unref(GST_OBJECT(cur->gCaps));

                free(cur);
        }

        // kill pipeline
        g_print ("Deleting pipeline\n");
        gst_object_unref (GST_OBJECT (dtkPipe->gPipe));

        // unlock status
        //pthread_mutex_unlock(&(dtkPipe->status_lock));

        // delete dtk_pipeline
        g_print ("Deleting dtk_pipeline\n");
        free(tex->aux);

        tex->aux = NULL;
}

// GET PAD SIZE
struct dtk_size* get_pad_size(GstPad* pad)
{
        GstCaps* caps = NULL;
        GstStructure* structure = NULL;
        struct dtk_size* size = malloc(sizeof(struct dtk_size));

        caps = gst_pad_get_negotiated_caps(pad);
        if(caps)
        {
                int h,w;
                structure = gst_caps_get_structure(caps,0);
                gst_structure_get_int(structure,"height",&h);
                gst_structure_get_int(structure,"width",&w);
                size->h = h;
                size->w = w;

                return size;
        }
        else
        {
                g_printerr("get_pad_size - could not get caps for the pad\n");
                return NULL;
        }
}