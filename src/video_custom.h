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

#ifndef VIDEO_CUSTOM_H
#define VIDEO_CUSTOM_H

#include <stdbool.h>

#include "drawtk.h"
#include "dtk_video.h"


	dtk_htex dtk_create_video_from_pipeline(dtk_hpipe customPipe);
        dtk_hpipe dtk_create_video_pipeline(const char* name);

        // Elements are defined by Gstreamer factories
        // Name is a unique element identifier
        // Additional parameters are specified as NULL-terminated var-args
        bool dtk_pipe_add_element(
                dtk_hpipe pipe,
                const char* factory,
                const char* name
                );

        bool dtk_pipe_add_element_full(
                dtk_hpipe pipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                ...
                );

        bool dtk_pipe_add_element_full_valist(
                dtk_hpipe pipe,
                const char* factory,
                const char* name,
                const char* firstPropertyName,
                va_list paramList
                );

/*        void dtk_set_last_element_caps(
                dtk_hpipe pipe,
                const char* mediaType,
                const char* firstParam,
                ...
                );

        void dtk_set_last_element_caps_valist(
                dtk_hpipe pipe,
                const char* mediaType,
                const char* firstParam,
                va_list paramList
                );*/

#endif

