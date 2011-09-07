/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Laboratory CNBI (Chair in Non-Invasive Brain-Machine Interface)
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
#ifndef FONTTEX_H
#define FONTTEX_H


#define NUMCHAR		(256-32)

struct character {
	float advance;
	float xmin, xmax, ymin, ymax;
	float txmin, txmax, tymin, tymax;
};

struct dtk_font {
	struct dtk_texture* tex;
	unsigned int pixwidth, pixheight;
	struct character ch[NUMCHAR];
};

LOCAL_FN
int dtk_char_pos(const struct dtk_font* restrict font, unsigned char c,
                 float* restrict vert, float* restrict texcoords,
		 unsigned int * restrict ind,
		 unsigned int currind, float * restrict org);

#endif // FONTTEX_H
