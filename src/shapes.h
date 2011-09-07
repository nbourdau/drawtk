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
#ifndef SHAPES_H
#define SHAPES_H

#include <GL/gl.h>

typedef void (*DrawShapeFn)(const struct dtk_shape* shp);
typedef void (*DestroyShapeFn)(void* data);

struct dtk_shape
{
	// Generic attributes
	float pos[2];
	float Rot; // in degrees

	// virtual functions
	DrawShapeFn drawproc;
	DestroyShapeFn destroyproc;

	// virtual data
	void* data;
};

struct single_shape
{
	GLuint* indices;
	GLfloat* vertices;
	GLfloat* texcoords;
	GLfloat* colors;
	GLuint num_ind;
	GLuint num_vert;
	unsigned int isalloc;
	GLenum primtype;
	struct dtk_texture* tex;
};


#define DTKF_ALLOC 	0x01
#define DTKF_UNICOLOR	0x02

LOCAL_FN
struct dtk_shape* create_generic_shape(struct dtk_shape* shp,
                                                 unsigned int nvert,
                                                 const GLfloat* vert, 
						 const GLfloat* tc,
						 const GLfloat* col,
						 unsigned int nind,
                                                 const GLuint* ind,
						 GLenum primtype,
						 struct dtk_texture* tex,
						 unsigned int flags);

#endif // SHAPES_H
