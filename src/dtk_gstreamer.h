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

#ifndef DTK_GSTREAMER_H
#define DTK_GSTREAMER_H

#include <stdbool.h>

#include "drawtk.h"

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct dtk_gst_pipeline* dtk_gst_hpipeline;

        // CREATE AND INITIALIZE PIPELINE
        dtk_gst_hpipeline dtk_gst_create_empty_pipeline(const char* name);
        dtk_gst_hpipeline dtk_gst_create_tcp_pipeline(const char* name, const char* server, unsigned int port);
        dtk_gst_hpipeline dtk_gst_create_udp_pipeline(const char* name, unsigned int port);
        //dtk_gst_hpipeline dtk_gst_create_rtp_pipeline(const char* name, unsigned int port);
        dtk_gst_hpipeline dtk_gst_create_file_pipeline(const char* name, const char* file);

        // ADD ELEMENT TO PIPELINE
        // Elements are defined by Gstreamer factories
        // Name is a unique element identifier
        // Additional parameters are specified as NULL-terminated var-args
        bool dtk_gst_add_element(
                dtk_gst_hpipeline pipe,
                const char* factory,
                const char* name
                );

        bool dtk_gst_add_element_full(
                dtk_gst_hpipeline pipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                ...
                );

        bool dtk_gst_add_element_full_valist(
                dtk_gst_hpipeline dtkPipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                va_list paramList
                );

        // START PIPELINE EXECUTION
        dtk_htex dtk_gst_run_pipeline(dtk_gst_hpipeline pipe);

        // GET PIPELINE STATUS
        bool dtk_gst_is_pipeline_running(dtk_gst_hpipeline pipe);

	// STOP PIPELINE EXECUTIOn
	void dtk_gst_stop_pipeline(dtk_gst_hpipeline pipe);

        // GET TEXTURE HANDLE
        dtk_htex dtk_gst_get_texture(dtk_gst_hpipeline pipe);

        // UPDATE TEXTURE
        void dtk_gst_update_texture(dtk_htex tex);

#ifdef __cplusplus
}
#endif

#endif

