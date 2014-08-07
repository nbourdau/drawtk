/*
    Copyright (C) 2009-2012  EPFL (Ecole Polytechnique Fédérale de Lausanne)
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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "drawtk.h"
#include "shapes.h"
#include "fonttex.h"

#define TWO_PI ((float)(2.0*M_PI))
#define MAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))
#define MIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))

static GLenum primitive_mode[] = {
	[DTK_TRIANGLES] = GL_TRIANGLES,
	[DTK_TRIANGLE_STRIP] = GL_TRIANGLE_STRIP,
	[DTK_TRIANGLE_FAN] = GL_TRIANGLE_FAN,
	[DTK_LINES] = GL_LINES,
	[DTK_LINE_STRIP] = GL_LINE_STRIP
};
#define NUM_PRIM_MODE (sizeof(primitive_mode)/sizeof(primitive_mode[0]))

static void get_bbox(struct single_shape* sinshp, float* left, float *right, 
                                             float* top, float* bottom)
{
	unsigned int i;
	float l, r, t, b;
	l = b = FLT_MAX;
	r = t = -FLT_MAX;
	float* vert = sinshp->vertices;

	for (i=0; i<2*sinshp->num_vert; i+=2) {
		l = MIN(vert[0], l);
		r = MAX(vert[0], r);
		b = MIN(vert[1], b);
		t = MAX(vert[1], t);
		vert += 2;
	}
	*left = l;
	*right = r;
	*top = t;
	*bottom = b;
}

API_EXPORTED
dtk_hshape dtk_create_circle(struct dtk_shape* shp, float cx, float cy, float r, int isfull, const float* color, unsigned int numpoints)
{	                  
	unsigned int i,j;
	struct single_shape* sinshp;
	GLuint numvert = isfull ? numpoints+1 : numpoints;
	GLuint numind = isfull ? numpoints+2 : numpoints;
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;

	shp = create_generic_shape(shp, numvert, NULL, NULL, color,
	                                numind, NULL, primtype,
					NULL, DTKF_ALLOC | DTKF_UNICOLOR);
	if (!shp)
		return NULL;

	sinshp = shp->data;

	i = j = 0;
	// Put center and finish fan loop if full
	if (isfull) {
		sinshp->vertices[0] = cx;
		sinshp->vertices[1] = cy;
		sinshp->indices[0] = 0;
		sinshp->indices[numind-1] = 1;
		i++;
	}

	// Create the circle, radius is expressed in width relative coordinates
	while (i<numvert) { 
		sinshp->indices[i] = i;
		sinshp->vertices[2*i]=r*cos((float)j*TWO_PI/numpoints) +cx;
		sinshp->vertices[2*(i++)+1]=r*sin((float)(j++)*TWO_PI/numpoints) +cy;
	}
	
	return shp;
} 

API_EXPORTED
dtk_hshape dtk_create_circle_str(struct dtk_shape* shp, float cx, float cy, float r, float thick, const float* color, unsigned int numpoints)
{ 
	float r1, r2, cr;
	struct single_shape* sinshp;
	GLuint numvert = 2*numpoints + 2;
	GLuint numind =  2*numpoints + 2;
	GLenum primtype = GL_TRIANGLE_STRIP;

	r1 = r;
	r2 = r - thick;

	if (r2 <= 0 || thick < 0)
		return NULL;

	shp = create_generic_shape(shp, numvert, NULL, NULL, color,
	                                numind, NULL, primtype,
					NULL, DTKF_ALLOC | DTKF_UNICOLOR);
	if (!shp)
		return NULL;

	sinshp = shp->data;

	// Create the circle, radius is expressed in width relative coordinates
	for (unsigned int i = 0; i<numvert; i++) {
		cr = i % 2 ? r2 : r1;
		sinshp->indices[i] = i;
		sinshp->vertices[2*i]=cr*cos((float)i*TWO_PI/numpoints) +cx;
		sinshp->vertices[2*(i)+1]=cr*sin((float)(i)*TWO_PI/numpoints) +cy;
	}

	return shp;
}

API_EXPORTED
dtk_hshape dtk_create_cross(struct dtk_shape* shp, float cx, float cy, float width, const float* color) 
{
	GLfloat vertices[8];
	GLuint indices[4] = {0, 2, 1, 3};
	GLenum primtype = GL_LINES;

	// To decide: center position. For the time being the shape is drawn
	// respect to the center
	//cx = cx - width/2;
	//cy = cy - width/2;

	vertices[0] = -width/2 + cx;
	vertices[1] = 0 + cy;
	
	vertices[2] = cx;
	vertices[3] = width/2 + cy;
	
	vertices[4] = width/2 + cx;
	vertices[5] = 0 + cy;
	
	vertices[6] = cx;
	vertices[7] = -width/2 + cy;

	shp = create_generic_shape(shp, 4, vertices, NULL, color,
	                                4, indices, primtype,
					NULL, DTKF_ALLOC|DTKF_UNICOLOR);
	
	return shp;
}

API_EXPORTED
dtk_hshape dtk_create_rectangle_2p(struct dtk_shape* shp, float p1_x, float p1_y, float p2_x, float p2_y, int isfull, const float* color)
{
	float vertices[8];
	GLuint indices[4] = {0, 1, 2, 3};
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
	
	vertices[0] = p1_x;
	vertices[1] = p1_y;
	
	vertices[2] = p1_x;
	vertices[3] = p2_y;
	
	vertices[4] = p2_x;
	vertices[5] = p2_y;
	
	vertices[6] = p2_x;
	vertices[7] = p1_y;

	shp = create_generic_shape(shp, 4, vertices, NULL, color,
	                                4, indices, primtype, 
					NULL, DTKF_ALLOC|DTKF_UNICOLOR);
	
	return shp;
}


API_EXPORTED
dtk_hshape dtk_create_rectangle_hw(struct dtk_shape* shp, float cx, float cy, float width, float height, int isfull, const float* color)
{
	float vertices[8];
	GLuint indices[4] = {0, 1, 2, 3};
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
	
	vertices[0] = -width/2 + cx;
	vertices[1] = -height/2 + cy;
	
	vertices[2] = -width/2 + cx;
	vertices[3] = height/2 + cy;
	
	vertices[4] = width/2 + cx;
	vertices[5] = height/2 + cy;
	
	vertices[6] = width/2 + cx;
	vertices[7] = -height/2 + cy;

	shp = create_generic_shape(shp, 4, vertices, NULL, color,
	                                4, indices, primtype,
					NULL, DTKF_ALLOC|DTKF_UNICOLOR);
	
	return shp;
}


API_EXPORTED
dtk_hshape dtk_create_arrow(struct dtk_shape* shp, float cx, float cy, float width, float height, int isfull, const float* color)
{
	float vertices[14];
	GLuint indices[7] = {0, 1, 2, 3, 4, 5, 6};
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
	float Wtot = width;
	float Htot = height;
	float Warr = Wtot/3;
	float Hrec = Htot/2;

	// To decide: center position. For the time being the shape is drawn
	// respect to the center
	cx = cx - width/2;
	cy = cy;

	vertices[0] = 0 + cx;
	vertices[1] = 0 + cy;
	
	vertices[2] = Warr + cx;
	vertices[3] = -Htot/2 + cy;
	
	vertices[4] = Warr + cx;
	vertices[5] = -Hrec/2 + cy;
	
	vertices[6] = Wtot + cx;
	vertices[7] = -Hrec/2 + cy;
	
	vertices[8] = Wtot + cx;
	vertices[9] = Hrec/2 + cy;
	
	vertices[10] = Warr + cx;
	vertices[11] = Hrec/2 + cy;
	
	vertices[12] = Warr + cx;
	vertices[13] = Htot/2 + cy;

	shp = create_generic_shape(shp, 7, vertices, NULL, color,
	                                7, indices, primtype,
					NULL, DTKF_ALLOC|DTKF_UNICOLOR);
	return shp;
}


API_EXPORTED
dtk_hshape dtk_create_triangle(struct dtk_shape* shp, float x1, float y1, float x2, float y2, float x3, float y3, int isfull, const float* color)
{
	GLfloat vertices[] = {x1, y1, x2, y2, x3, y3};
	GLuint indices[] = {0, 1, 2};
	GLenum primtype = isfull ? GL_TRIANGLES : GL_LINE_LOOP;
	
	return create_generic_shape(shp, 3, vertices, NULL, color,
	                                 3, indices, primtype,
					 NULL, DTKF_ALLOC|DTKF_UNICOLOR);
}


API_EXPORTED
dtk_hshape dtk_create_line(struct dtk_shape* shp, float x1, float y1, float x2, float y2, const float* color)
{
	GLfloat vertices[] = {x1, y1, x2, y2};
	GLuint indices[] = {0, 1};
	
	return create_generic_shape(shp, 2, vertices, NULL, color,
	                                 2, indices, GL_LINES,
					 NULL, DTKF_ALLOC|DTKF_UNICOLOR);
}


API_EXPORTED
dtk_hshape dtk_create_shape(struct dtk_shape* shp, unsigned int numvert, const float* vertex_array, int isfull, const float* color)
{
	unsigned int i;
	struct single_shape* sinshp; 
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;

	shp = create_generic_shape(shp, numvert, vertex_array, NULL, color, 
	                                numvert, NULL, primtype,
					NULL, DTKF_ALLOC|DTKF_UNICOLOR);
	if (shp) {
		sinshp = shp->data;
		for (i=0; i<numvert; i++)
			sinshp->indices[i] = i;
	}

	return shp;
}


API_EXPORTED
dtk_hshape dtk_create_image(struct dtk_shape* shp, float x, float y, float width, float height, const float* color, dtk_htex image)
{
	GLfloat vertices[8];
	GLfloat textcoords[8];
	GLuint indices[4] = {0, 1, 2, 3};
	GLenum primtype = GL_TRIANGLE_FAN;
	
	// To decide: center position. For the time being the shape is drawn
	// respect to the center
	x = x - width/2;
	y = y - height/2;

	vertices[0] = 0 + x;
	vertices[1] = 0 + y;
	
	vertices[2] = 0 + x;
	vertices[3] = height + y;
	
	vertices[4] = width + x;
	vertices[5] = height + y;
	
	vertices[6] = width + x;
	vertices[7] = 0 + y;
	
	textcoords[0] = 0;
	textcoords[1] = 0;
	
	textcoords[2] = 0;
	textcoords[3] = 1;
	
	textcoords[4] = 1;
	textcoords[5] = 1;
	
	textcoords[6] = 1;
	textcoords[7] = 0;
	
	shp = create_generic_shape(shp, 4, vertices, textcoords, color,
	                                4, indices, primtype,
					image, DTKF_ALLOC|DTKF_UNICOLOR);
	
	return shp;
}


API_EXPORTED
dtk_hshape dtk_create_string(struct dtk_shape* shp, const char* text,
			     float size, float x, float y,
			     unsigned int alignment,
			     const float* color, dtk_hfont font)
{
	GLfloat *vert, *tc;
	GLuint* ind;
	float pos = 0.0f;
	float l,r,t,b,orgx, orgy;
	unsigned int i, len = text ? strlen(text) : 0;

	shp = create_generic_shape(shp, 4*len, NULL, NULL, color, 
	                               6*len, NULL, GL_TRIANGLES,
				       font->tex, DTKF_ALLOC|DTKF_UNICOLOR);
	if (!shp)
		return NULL;
	
	ind = ((struct single_shape*)(shp->data))->indices;
	vert = ((struct single_shape*)(shp->data))->vertices;
	tc = ((struct single_shape*)(shp->data))->texcoords;

	// setup letter vertices
	for (i=0; i<len; i++)
		dtk_char_pos(font, text[i], vert+8*i, tc+8*i, 
		             ind+6*i, 4*i, &pos);

	get_bbox(shp->data, &l, &r, &t, &b);

	if (alignment & DTK_HMID)
		orgx = (l+r)/2.0f;
	else if (alignment & DTK_RIGHT)
		orgx = r;
	else
		orgx = l;

	if (alignment & DTK_VMID)
		orgy = (t+b)/2.0f;
	else if (alignment & DTK_TOP)
		orgy = t;
	else
		orgy = b;

	for (i=0; i<8*len; i+=2) {
		vert[i  ] = (vert[i  ] - orgx)*size + x;	
		vert[i+1] = (vert[i+1] - orgy)*size + y;	
	}

	return shp;
}

API_EXPORTED
dtk_hshape dtk_create_complex_shape(dtk_hshape shp,
                 unsigned int nvert, const float* vertpos,
		 const float* vertcolor, const float* texcoords,
		 unsigned int nind, const unsigned int *ind,
		 unsigned int type, dtk_htex tex)
{
	struct single_shape* sinshp;

	if (type >= NUM_PRIM_MODE)
		return NULL;

	shp = create_generic_shape(shp, nvert, NULL, NULL, NULL, 
	                               nind, NULL, primitive_mode[type],
				       tex, 0);
	if (!shp)
		return NULL;

	sinshp = shp->data;
	sinshp->vertices = (float*)vertpos;
	sinshp->colors = (float*)vertcolor;
	sinshp->texcoords = (float*)texcoords;
	sinshp->indices = (unsigned int*)ind;
	
	return shp;
}
