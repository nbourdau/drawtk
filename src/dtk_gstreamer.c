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

#include "dtk_gstreamer.h"

#include "drawtk.h"
#include "texmanager.h"
#include "shapes.h"
#include "window.h"

static bool g_gst_initialized = false;

////////////////////
//
// STRUCTURES
//
////////////////////

enum dtk_gst_status {
        DTKG_STOPPED = 0,
        DTKG_READY = 1,
        DTKG_PLAYING = 2
};

#define DTK_GST_VERTICAL_FLIP 5

struct dtk_gst_element
{
        // gstreamer element
        GstElement* gElement;

        // input caps
        GstCaps* gCaps;

        // previous element in list
        struct dtk_gst_element* prev;

        // next element in list
        struct dtk_gst_element* next;
};

struct dtk_gst_pipeline
{
        // pipeline object
        GstElement* gPipe;
        bool editLocked;

        // bus object
        GstBus* gBus;

        // doubly linked list of elements in the pipeline
        struct dtk_gst_element* dtkElement;

        // pipeline status
        pthread_t thread;
        //pthread_mutex_t mutex;
        int status;

        // texture output
        dtk_htex outTex;
};

////////////////////
//
// DEFINITIONS
//
////////////////////

// PIPELINE EXECUTION THREAD
void* dtk_gst_run_pipeline_loop(void * pipe);

// OUTPUT TEXTURE CREATION
dtk_htex create_texture(struct dtk_gst_pipeline* pipe);

// LINK ELEMENTS (SOURCE->SINK)
void dtk_gst_link_elements(struct dtk_gst_element* source, struct dtk_gst_element* sink);

// GET PAD SIZE
struct dtk_size* get_pad_size(GstPad* pad);

// NEW PAD CALLBACK
static void dtk_gst_newpad_callback (GstElement* element, GstPad* pad, gpointer data);

// BUS CALLBACK
gboolean dtk_gst_bus_callback(GstBus* bus, GstMessage* msg, gpointer data);

// FAKESINK HANDOFF CALLBACK
static void dtk_gst_handoff_callback(GstElement* sink, GstBuffer* buffer, /*GstPad* pad,*/ gpointer data);

////////////////////
//
// IMPLEMENTATION
//
////////////////////

API_EXPORTED
dtk_gst_hpipeline dtk_gst_create_empty_pipeline(const char* name)
{
	// initialize glib threading
	if(!g_thread_supported())
	{
		g_thread_init(NULL);
	}

	// initialize gstreamer
	if(!g_gst_initialized)
	{
		gst_init (NULL, NULL);
		g_gst_initialized = true;
	}

	// create pipeline
	struct dtk_gst_pipeline* dtkPipeline = malloc(sizeof(struct dtk_gst_pipeline));
	dtkPipeline->gPipe = gst_pipeline_new (name);
	dtkPipeline->gBus       = NULL;
	dtkPipeline->dtkElement = NULL;
        dtkPipeline->outTex     = NULL;
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
        dtkPipeline->thread = NULL;
	dtkPipeline->status = DTKG_STOPPED;
        //dtkPipeline->mutex = PTHREAD_MUTEX_INITIALIZER;
	
	// initialize out texture
	dtkPipeline->outTex = create_texture(dtkPipeline);

	return dtkPipeline;
}

API_EXPORTED
dtk_gst_hpipeline dtk_gst_create_tcp_pipeline(const char* name, const char* server, unsigned int port)
{
        dtk_gst_hpipeline pipe = dtk_gst_create_empty_pipeline(name);
        dtk_gst_add_element_full(pipe, "tcpclientsrc",	"tcp-src", "host", server, "port", port, NULL);
        dtk_gst_add_element(pipe, "decodebin2", "decoder-bin");
        dtk_gst_add_element(pipe, "ffmpegcolorspace", "converter");
        dtk_gst_add_element_full(pipe, "videoflip", "flipper", "method", DTK_GST_VERTICAL_FLIP, NULL);
        dtk_gst_add_element(pipe, "identity", "buffer-reader");
        dtk_gst_add_element(pipe, "fakesink", "fake-sink");

        pipe->editLocked = true;

        return pipe;
}

API_EXPORTED
dtk_gst_hpipeline dtk_gst_create_udp_pipeline(const char* name, unsigned int port)
{
        dtk_gst_hpipeline pipe = dtk_gst_create_empty_pipeline(name);
        dtk_gst_add_element_full(pipe, "udpsrc", "udp-src", "port", port, NULL);
        dtk_gst_add_element(pipe, "queue", "data-queue");
        dtk_gst_add_element(pipe, "decodebin2", "decoder-bin");
        dtk_gst_add_element(pipe, "ffmpegcolorspace", "converter");
        dtk_gst_add_element_full(pipe, "videoflip", "flipper", "method", DTK_GST_VERTICAL_FLIP, NULL);
        dtk_gst_add_element(pipe, "fakesink", "fake-sink");

        pipe->editLocked = true;

        return pipe;
}
/*
API_EXPORTED
dtk_gst_hpipeline dtk_gst_create_rtp_pipeline(const char* name, unsigned int port)
{
        // BEWARE OF THE NASTY IMPLEMENTATION, IT BITES !  

        dtk_gst_hpipeline pipe = dtk_gst_create_empty_pipeline(name);
        dtk_gst_add_element_full(pipe, "udpsrc", "udp-src", "port", port, NULL);
        
        // don't touch this - required HERE to link rtpbin correctly
        pipe->dtkElement->gCaps = gst_caps_new_simple("application/x-rtp","clock_rate",G_TYPE_INT,9000000,NULL);

        dtk_gst_add_element(pipe, "gstrtpbin", "RTP-bin");
        dtk_gst_add_element(pipe, "decodebin2", "decoder-bin");
        dtk_gst_add_element(pipe, "ffmpegcolorspace", "converter");
        dtk_gst_add_element(pipe, "fakesink", "fake-sink");

        pipe->editLocked = true;

        return pipe;
}*/

API_EXPORTED
dtk_gst_hpipeline dtk_gst_create_file_pipeline(const char* name, const char* file)
{
        dtk_gst_hpipeline pipe = dtk_gst_create_empty_pipeline(name);
        dtk_gst_add_element_full(pipe, "filesrc", "file-src", "location", file, NULL);
        dtk_gst_add_element(pipe, "decodebin2", "decoder-bin");
        dtk_gst_add_element(pipe, "ffmpegcolorspace", "converter");
        dtk_gst_add_element_full(pipe, "videoflip", "flipper", "method", DTK_GST_VERTICAL_FLIP, NULL);
        dtk_gst_add_element(pipe, "identity", "buffer-reader");
        dtk_gst_add_element(pipe, "fakesink", "fake-sink");

        pipe->editLocked = true;

        return pipe;
}

API_EXPORTED
bool dtk_gst_add_element(
                dtk_gst_hpipeline pipe,
                const char* factory,
                const char* name
                )
{
	return dtk_gst_add_element_full(pipe, factory,name,NULL);
}

API_EXPORTED
bool dtk_gst_add_element_full(
                dtk_gst_hpipeline pipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                ...
                )
{
	bool	retValue;
	va_list	paramList;

	va_start(paramList,firstPropertyName);
	retValue = dtk_gst_add_element_full_valist(pipe,factory,name,firstPropertyName,paramList);
	va_end(paramList);

	return retValue;
}

API_EXPORTED
bool dtk_gst_add_element_full_valist(
                dtk_gst_hpipeline dtkPipe,
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
                g_print("dtk_gst_add_element : %s \n",name);
        }

	// if properties are defined
	if(firstPropertyName!=NULL)
	{
		// set element properties
		g_object_set_valist(G_OBJECT(gstElem),firstPropertyName,paramList);
	}

	// add object to global bin
	gst_bin_add(GST_BIN(dtkPipe->gPipe),gstElem);

	// create & fill dtk_gst_element structure
	struct dtk_gst_element* newElem = malloc(sizeof(struct dtk_gst_element));
	newElem->gElement = gstElem;
	newElem->prev = dtkPipe->dtkElement;
	newElem->next = NULL;
        newElem->gCaps = NULL;

        // check if element is a buffer reader
        if(strcmp(factory,"identity")==0)
        {
                newElem->gCaps = gst_caps_new_simple("video/x-raw-rgb","bpp",G_TYPE_INT,24,"red_mask",G_TYPE_INT,16711680,"green_mask",G_TYPE_INT,65280,"blue_mask",G_TYPE_INT,255,NULL);
                g_signal_connect(newElem->gElement,"handoff",G_CALLBACK(dtk_gst_handoff_callback),dtkPipe);
        }

        // check if new element is a terminal element
        if(strcmp(factory,"fakesink")==0 || strcmp(factory,"autovideosink")==0)
        {
                dtkPipe->editLocked = true;
        }

	// fill previous dtk_gst_element.next field
	if(dtkPipe->dtkElement != NULL)
	{
                dtk_gst_link_elements(dtkPipe->dtkElement,newElem);
		dtkPipe->dtkElement->next = newElem;
        }

	// put element on top
	dtkPipe->dtkElement = newElem;

	return true;
}



// START PIPELINE EXECUTION
API_EXPORTED
dtk_htex dtk_gst_run_pipeline(dtk_gst_hpipeline pipe)
{
        g_print("dtk_gst_run_pipeline\n");

        //pthread_mutex_lock(&(pipe->mutex));

        if(pipe->status == DTKG_PLAYING)
        {
                return pipe->outTex;
        }

        GstStateChangeReturn ret = gst_element_set_state (pipe->gPipe, GST_STATE_PLAYING);

        if(ret==GST_STATE_CHANGE_FAILURE)
                return false;

        // set pipeline status
        pipe->status = DTKG_READY;

	// launch execution thread
        pthread_create(&(pipe->thread),NULL,dtk_gst_run_pipeline_loop,(void*)pipe);

        // wait till the state changes
        while(pipe->status == DTKG_READY);

        dtk_htex retData = (pipe->status==DTKG_PLAYING) ? pipe->outTex : NULL ; 

        //pthread_mutex_unlock(&(pipe->mutex));

        return retData;        
}

API_EXPORTED
bool dtk_gst_is_pipeline_running(dtk_gst_hpipeline pipe)
{
        return (pipe->outTex->id!=0);
}

API_EXPORTED
void dtk_gst_stop_pipeline(dtk_gst_hpipeline pipe)
{
        //pthread_mutex_lock(&(pipe->mutex));

	// set pipeline status to dead
	pipe->status = DTKG_STOPPED;

        // join the threads
        pthread_join(pipe->thread,NULL);

        //pthread_mutex_unlock(&(pipe->mutex));
}

// PIPELINE EXECUTION THREAD
void* dtk_gst_run_pipeline_loop(void * pipe)
{
        g_print("dtk_gst_run_pipeline_loop\n");

        GstMessage* msg = NULL;
        struct dtk_gst_pipeline* dtkPipe = (struct dtk_gst_pipeline*) pipe;

        // main loop
        bool canLoop = true;

        while (canLoop && dtkPipe->status!=DTKG_STOPPED) {

                // Read new messages from bus
                while(canLoop && (msg = gst_bus_pop (dtkPipe->gBus)))
                {
                        // Call the bus callback for each new message
                        canLoop = dtk_gst_bus_callback(dtkPipe->gBus,msg,dtkPipe);

                        // Clean the message
                        gst_message_unref (msg);
                }
        } 

        // set status to STOPPED
        dtkPipe->status = DTKG_STOPPED;

        // disable pipeline
        g_print ("Set pipeline to NULL\n");
        gst_element_set_state (dtkPipe->gPipe, GST_STATE_NULL);

        // kill bus
        g_print ("Deleting bus\n");
        gst_object_unref (GST_OBJECT(dtkPipe->gBus));
        
        // kill pipeline
        g_print ("Deleting pipeline\n");
        gst_object_unref (GST_OBJECT (dtkPipe->gPipe));

        // delete each dtk_gst_element
        g_print ("Deleting dtk_gst_elements\n");
        while(dtkPipe->dtkElement != NULL)
        {
                struct dtk_gst_element* cur = dtkPipe->dtkElement;

                dtkPipe->dtkElement = dtkPipe->dtkElement->prev;

                if(cur->gCaps != NULL)
                        gst_object_unref(GST_OBJECT(cur->gCaps));
                
                free(cur);
        }

        // delete dtk_gst_pipeline
        g_print ("Deleting dtk_gst_pipeline\n");
        free(dtkPipe);

	return NULL;
}

// GET TEXTURE HANDLE
API_EXPORTED
dtk_htex dtk_gst_get_texture(dtk_gst_hpipeline pipe)
{
        return pipe->outTex;
}

// OUTPUT TEXTURE CREATION
dtk_htex create_texture(struct dtk_gst_pipeline* pipe)
{
        dtk_htex tex = NULL;
        char stringid[256];

        // Create new texture
        char* pipeName = gst_element_get_name(pipe->gPipe);
        snprintf(stringid, sizeof(stringid), "PIPELINE:%s", pipeName);
        g_free(pipeName);

        if ((tex = get_texture(stringid)) == NULL)
                return NULL;

        tex->id = 0;
        tex->isvideo = true;
        tex->sizes = NULL;

        return tex;        
}

// UPDATE TEXTURE
void dtk_gst_update_texture(dtk_htex tex)
{
        pthread_mutex_lock(&(tex->lock));

        //g_print("acquired mutex\n");

        glEnable(GL_TEXTURE_2D);

        // creation of the GL texture Object
        
        if( tex->id == 0)
        {
                glGenTextures(1,&(tex->id));

                glBindTexture(GL_TEXTURE_2D, tex->id);

                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex->mxlvl);

                glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(char));

                g_print("Tex id: %d  - Tex size: %d %d\n",tex->id,tex->sizes[0].w,tex->sizes[0].h);
        }
        
        // store GL texture parameters
        glBindTexture(GL_TEXTURE_2D, tex->id);

        // load data into memory
        //g_print("Loading tex!");
        glTexImage2D(GL_TEXTURE_2D, 0, tex->intfmt, tex->sizes[0].w, tex->sizes[0].h, 0,
                tex->fmt, tex->type, (unsigned char*)tex->data);
        //g_print(" Tex Loaded!\n");

        // Wait the loading being performed
        glFlush();
        glBindTexture(GL_TEXTURE_2D, 0);

        tex->isinit = true;

        pthread_mutex_unlock(&(tex->lock));
}


// FAKESINK HANDOFF CALLBACK
static void dtk_gst_handoff_callback(GstElement* sink, GstBuffer* buffer, /*GstPad* pad,*/ gpointer data)
{
        //g_print("dtk_gst_handoff_callback\n");

        assert(sink != NULL);
        assert(buffer != NULL);
        //assert(pad != NULL);

        struct dtk_gst_pipeline* pipe = (struct dtk_gst_pipeline*) data;
        dtk_htex tex = pipe->outTex;

        //g_print("acquiring mutex\n");
        pthread_mutex_lock(&(tex->lock));

        if(tex->sizes == NULL)
        {
                tex->mxlvl = 0;
                tex->intfmt = GL_RGB;
                tex->fmt = GL_RGB;
                tex->type = GL_UNSIGNED_BYTE;

                tex->sizes = get_pad_size(GST_BASE_TRANSFORM_SRC_PAD(sink));

                tex->data = malloc(tex->sizes[0].w*tex->sizes[0].h*20);
                memset(tex->data,130,tex->sizes[0].w*tex->sizes[0].h*20);

                if(pipe->status==DTKG_READY)
                        pipe->status = DTKG_PLAYING;

                g_print("Tex created\n");
        }

                
        // load data into memory
        memcpy(tex->data,(unsigned char*)GST_BUFFER_DATA(buffer),tex->sizes[0].w*tex->sizes[0].h*3);
        
        //memset(tex->data,130,tex->sizes[0].w*tex->sizes[0].h*20);

        tex->isinit = false;

        pthread_mutex_unlock(&(tex->lock));
}

// LINK ELEMENTS (SOURCE->SINK)
void dtk_gst_link_elements(struct dtk_gst_element* source, struct dtk_gst_element* sink)
{
        g_print("dtk_gst_link_elements : %s => %s\n",gst_element_get_name(source->gElement),gst_element_get_name(sink->gElement));

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
                        if(sink->gCaps!=NULL)
                                gst_element_link_filtered(source->gElement,sink->gElement,sink->gCaps);
                        else if(source->gCaps!=NULL)
                                gst_element_link_filtered(source->gElement,sink->gElement,source->gCaps);
                        else
                                gst_element_link(source->gElement,sink->gElement);
                        return;
                case GST_PAD_SOMETIMES:
                        g_signal_connect (source->gElement, "pad-added", G_CALLBACK (dtk_gst_newpad_callback), sink);
                        return;
                default:
                        break;
                }

        }

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

// NEW PAD CALLBACK
static void dtk_gst_newpad_callback (GstElement* element, GstPad* pad, gpointer data)
{
        GstPad* sinkpad = NULL;
        GstCaps *sourcecaps = NULL;
        GstCaps *sinkcaps = NULL;

        g_print("dtk_gst_newpad_callback\n");

        // retrieve sink element & Pad
        struct dtk_gst_element* sink = (struct dtk_gst_element*)data;
        sinkpad = gst_element_get_static_pad(sink->gElement,"sink");

        // get source pad & sink-pad capabilities
        sourcecaps = gst_pad_get_caps (pad);
        sinkcaps = gst_pad_get_caps(sinkpad);

        bool capsCompatible = (sink->gCaps == NULL) || gst_caps_can_intersect(sinkcaps,sink->gCaps);

        // check that :
        // sink and sink->gCaps are compatible
        // source & sink caps are compatible
        if(capsCompatible && gst_caps_can_intersect(sourcecaps, sinkcaps))
        {
                // set capabilities of the pad
                gst_pad_set_caps(sinkpad,sink->gCaps);

                // link source "pad" to sink-pad
                gst_pad_link(pad,sinkpad);
        }
        else
        {
                // output error and abort
                g_printerr("Source and sink pads could not be connected - incompatible caps");
        }

        // unreference data
        gst_caps_unref(sourcecaps);
        gst_caps_unref(sinkcaps);
        gst_object_unref(sinkpad);
}

// BUS CALLBACK
gboolean dtk_gst_bus_callback(GstBus* bus, GstMessage* msg, gpointer data)
{
//        g_print("dtk_gst_bus_callback\n");

	switch(GST_MESSAGE_TYPE(msg))
	{

	case GST_MESSAGE_EOS:
		g_print ("End of stream\n");
                return false;

	case GST_MESSAGE_ERROR:
		{
			gchar  *debug;
			GError *error;
			
			gst_message_parse_error (msg, &error, &debug);
			g_free (debug);
			
			g_printerr ("Error: %s\n", error->message);
			g_error_free (error);
			
			return false;
		}
	default:
		return true;
	}
}

