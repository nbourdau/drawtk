/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

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
#ifndef PALETTE_H
#define PALETTE_H

#ifdef __cplusplus
extern "C" {
#endif

const float* dtk_get_color(unsigned int ind);

#define dtk_white		(dtk_get_color(0))
#define dtk_black		(dtk_get_color(1))
#define dtk_yellow		(dtk_get_color(2))
#define dtk_orange		(dtk_get_color(3))
#define dtk_blue		(dtk_get_color(4))
#define dtk_green		(dtk_get_color(5))
#define dtk_red 		(dtk_get_color(6))
#define dtk_magenta		(dtk_get_color(7))
#define dtk_cyan		(dtk_get_color(8))
/* See http://tango.freedesktop.org/Tango_Icon_Theme_Guidelines
   for the definition of the TANGO colors */
#define dtk_butter_light	(dtk_get_color(9))
#define dtk_butter_med		(dtk_get_color(10))
#define dtk_butter_dark		(dtk_get_color(11))
#define dtk_orange_light	(dtk_get_color(12))
#define dtk_orange_med		(dtk_get_color(13))
#define dtk_orange_dark		(dtk_get_color(14))
#define dtk_chocolate_light	(dtk_get_color(15))
#define dtk_chocolate_med	(dtk_get_color(16))
#define dtk_chocolate_dark	(dtk_get_color(17))
#define dtk_chameleon_light	(dtk_get_color(18))
#define dtk_chameleon_med	(dtk_get_color(19))
#define dtk_chameleon_dark	(dtk_get_color(20))
#define dtk_skyblue_light	(dtk_get_color(21))
#define dtk_skyblue_med		(dtk_get_color(22))
#define dtk_skyblue_dark	(dtk_get_color(23))
#define dtk_plum_light		(dtk_get_color(24))
#define dtk_plum_med		(dtk_get_color(25))
#define dtk_plum_dark		(dtk_get_color(26))
#define dtk_scarletred_light	(dtk_get_color(27))
#define dtk_scarletred_med	(dtk_get_color(28))
#define dtk_scarletred_dark	(dtk_get_color(29))
#define dtk_aluminium_light	(dtk_get_color(30))
#define dtk_aluminium_med	(dtk_get_color(31))
#define dtk_aluminium_dark	(dtk_get_color(32))
#define dtk_aluminium2_light	(dtk_get_color(33))
#define dtk_aluminium2_med	(dtk_get_color(34))
#define dtk_aluminium2_dark	(dtk_get_color(35))

#ifdef __cplusplus
}
#endif

#endif
