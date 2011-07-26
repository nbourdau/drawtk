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

#ifndef VIDPIPE_CREATION_H
#define VIDPIPE_CREATION_H

#include <gst/gst.h>

struct pipeline_opt
{
	int type;
	const char* str;
	int port;
};

#define VTCP	0
#define VUDP	1
#define VFILE	2
#define VTEST	3

LOCAL_FN GstElement* create_pipeline(const struct pipeline_opt* opt);

#endif

