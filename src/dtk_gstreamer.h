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

#ifdef __cplusplus
extern "C" {
#endif

        typedef struct dtk_gst_pipeline* dtk_gst_hpipeline;


        // CREATE AND INITIALIZE PIPELINE
        dtk_gst_hpipeline dtk_gst_create_pipeline(const char* name);

        // ADD ELEMENT TO PIPELINE
        // Elements are defined by Gstreamer factory/name combinations
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
        bool dtk_gst_run_pipeline(dtk_gst_hpipeline pipe);

        // CHECK PIPELINE ALIVE
        bool dtk_gst_is_pipeline_alive(dtk_gst_hpipeline pipe);

	// STOP PIPELINE EXECUTIOn
	void dtk_gst_stop_pipeline(dtk_gst_hpipeline pipe);

#ifdef __cplusplus
}
#endif

#endif
