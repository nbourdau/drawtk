/*
    Copyright (C) 2009-2010  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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

#include "drawtk.h"
#include "palette.h"

API_EXPORTED
const float pal_basic[NUM_BASIC_COLORS][4] = {
	{1.0, 1.0, 1.0, 1.0},
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{1.0, 0.5, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0}		
};

API_EXPORTED
const float pal_tango[NUM_TANGO_COLORS][4] = {

	{252.0/256.0, 233.0/256.0, 79.0/256.0, 1.0f},
	{237.0/256.0, 212.0/256.0, 0.0/256.0, 1.0f},		
	{196.0/256.0, 160.0/256.0, 0.0f/256.0f, 1.0f},
	{252.0/256.0, 175.0/256.0, 62.0/256.0, 1.0f},
	{245.0/256.0, 121.0/256.0, 0.0/256.0, 1.0f},
	{206.0/256.0, 92.0/256.0, 0.0/256.0, 1.0f},
	{233.0/256.0, 185.0/256.0, 110.0/256.0, 1.0f},
	{193.0/256.0, 125.0/256.0, 17.0/256.0, 	1.0f},
	{143.0/256.0, 89.0/256.0, 2.0/256.0, 1.0f},
	{138.0/256.0, 226.0/256.0, 52.0/256.0, 1.0f},
	{115.0/256.0, 209.0/256.0, 38.0/256.0, 1.0f},
	{78.0/256.0, 154.0/256.0, 6.0/256.0, 1.0f},
	{114.0/256.0, 159.0/256.0, 207.0/256.0, 1.0f},
	{52.0/256.0, 101.0/256.0, 164.0/256.0, 1.0f},
	{32.0/256.0, 74.0/256.0, 135.0/256.0, 1.0f},
	{173.0/256.0, 127.0/256.0, 168.0/256.0, 1.0f},
	{117.0/256.0, 80.0/256.0, 123.0/256.0, 1.0f},
	{92.0/256.0, 53.0/256.0, 102.0/256.0, 1.0f},
	{239.0/256.0, 41.0/256.0, 41.0/256.0, 1.0f},
	{204.0/256.0, 0.0/256.0, 0.0/256.0, 1.0f},
	{164.0/256.0, 0.0/256.0, 0.0/256.0, 1.0f},
	{238.0/256.0, 238.5/256.0, 236.0/256.0, 1.0f},
	{211.0/256.0, 215.0/256.0, 207.0/256.0, 1.0f},
	{186.0/256.0, 189.0/256.0, 182.0/256.0, 1.0f},
	{136.0/256.0, 138.0/256.0, 133.0/256.0, 1.0f},
	{85.0/256.0, 87.0/256.0, 83.0/256.0, 1.0f},
	{46.0/256.0, 52.0/256.0, 54.0/256.0, 1.0f}		

};

