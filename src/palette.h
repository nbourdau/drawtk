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
#ifndef PALETTE_H
#define PALETTE_H

#ifdef __cplusplus
extern "C" {
#endif

// Enumerations for palette handling
extern const float pal_basic[][4];
extern const float pal_tango[][4];


// Collection of colors for each palette
// See http://tango.freedesktop.org/Tango_Icon_Theme_Guidelines
// for the definition of the TANGO colors
enum libdtk_tango{
	butter_light, butter_med, butter_dark,
	orange_light, orange_med, orange_dark,
	chocolate_light, chocolate_med, chocolate_dark,
	chameleon_light, chameleon_med, chameleon_dark,
	skyblue_light, skyblue_med, skyblue_dark,
	plum_light, plum_med, plum_dark,
	scarletred_light, scarletred_med, scarletred_dark,
	aluminium_light, aluminium_med, aluminium_dark,
	aluminium2_light, aluminium2_med, aluminium2_dark,
	NUM_TANGO_COLORS
};

enum libdtk_basic
{
	white, black, yellow, orange, blue, green, red, magenta, cyan,
	NUM_BASIC_COLORS
};

#ifdef __cplusplus
}
#endif

#endif
