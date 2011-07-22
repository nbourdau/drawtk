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

#ifndef VIDEO_STRUCTS_H
#define VIDEO_STRUCTS_H

#include <pthread.h>

#include <gst/gst.h>
#include <glib.h>

#include "drawtk.h"

struct dtk_pipe_element {
	// gstreamer element
	GstElement *gElement;

	// source capabilities
	GstCaps *gCaps;

	// previous & next element in doubly-linked list
	struct dtk_pipe_element *prev;
	struct dtk_pipe_element *next;
};

struct dtk_pipeline {
	// pipeline object
	GstElement *gPipe;
	bool editLocked;

	// bus object
	GstBus *gBus;

	// doubly linked list of elements in the pipeline
	struct dtk_pipe_element *dtkElement;

	// pipeline status
	pthread_t thread;
	int status;
	pthread_mutex_t status_lock;
};

#endif // ifndef VIDEO_STRUCTS_H
