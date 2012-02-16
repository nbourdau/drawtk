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
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL/SDL_opengl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "drawtk.h"
#include "shapes.h"
#include "window.h"
#include "texmanager.h"


/*************************
 * Internal declarations *
 *************************/

/*************************************************************************
 *                                                                       *
 *                        Single shape functions                         *
 *                                                                       *
 *************************************************************************/


/*******************
 * Implementations *
 *******************/
static void draw_single_shape(const struct dtk_shape* shp)
{
	struct single_shape* sinshp = shp->data;

	glVertexPointer(2, GL_FLOAT, 0, sinshp->vertices);
	glColorPointer(4, GL_FLOAT, 0, sinshp->colors);
	
	glBindTexture(GL_TEXTURE_2D, get_texture_id(sinshp->tex));
	if (sinshp->texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, sinshp->texcoords);	
	}

	// Draw shapes
	glDrawElements(sinshp->primtype, sinshp->num_ind, 
	               GL_UNSIGNED_INT, sinshp->indices);

	if (sinshp->texcoords)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


static void set_single_color(const struct dtk_shape* shp, const float* color, unsigned int mask)
{
	struct single_shape* sinshp;
	unsigned int i;
	sinshp = shp->data;

	for (i=0; i<4*sinshp->num_vert; i+=4) {
		if (!(mask & DTK_IGNR))
			memcpy(&sinshp->colors[0]+i, &color[0], sizeof(float));

		if (!(mask & DTK_IGNG))
			memcpy(&sinshp->colors[1]+i, &color[1], sizeof(float));

		if (!(mask & DTK_IGNB))
			memcpy(&sinshp->colors[2]+i, &color[2], sizeof(float));

		if (!(mask & DTK_IGNA))
			memcpy(&sinshp->colors[3]+i, &color[3], sizeof(float));
	}

}

static void destroy_single_shape(void* data)
{
	struct single_shape* sinshp = data;
	if (sinshp == NULL)
		return;

	if (sinshp->isalloc) {
		free(sinshp->vertices);
		free(sinshp->indices);
	}	
	free(sinshp);
}


static struct single_shape* alloc_single_shape(struct single_shape* sinshp,
                                  	unsigned int nvert,
				  	unsigned int nind,
				  	unsigned int usetex,
				  	unsigned int allocbuff)
{
	GLuint* indbuff = NULL;
	GLfloat *vertbuff = NULL, *texbuff = NULL, *colorbuff = NULL;
	int is_shp_alloc = 0;

	if (sinshp == NULL) {
		is_shp_alloc = 1;
		sinshp = calloc(1,sizeof(*sinshp));
		if (sinshp == NULL)
			return NULL;
	}
	
	// Check for the need of buffer allocation
	if ( (allocbuff ? sinshp->isalloc : !sinshp->isalloc)
	    && (nvert == sinshp->num_vert) && (nind == sinshp->num_ind)
	    && (usetex && !sinshp->texcoords) )
		return sinshp;

	// Memory allocation of buffers
	if (allocbuff) {
		indbuff = malloc(nind*sizeof(*indbuff));
		vertbuff = malloc(nvert*(usetex ? 8 : 6)*sizeof(*vertbuff));
		if (!vertbuff || !indbuff) {
			free(is_shp_alloc ? sinshp : NULL);
			free(vertbuff);
			free(indbuff);
			return NULL;
		}
		colorbuff = vertbuff + 2*nvert;
		texbuff = usetex ? colorbuff + 4*nvert : NULL;
	}

	// Free previous buffers (if any)
	if (sinshp->isalloc) {
		free(sinshp->indices);
		free(sinshp->vertices);
	}
	
	sinshp->indices = indbuff;
	sinshp->vertices = vertbuff;
	sinshp->texcoords = texbuff;
	sinshp->colors = colorbuff;
	sinshp->num_ind = nind;
	sinshp->num_vert = nvert;
	sinshp->isalloc = allocbuff;

	return sinshp;
}


static struct dtk_shape* alloc_generic_shape(struct dtk_shape* shp,
                                           unsigned int numvert,
					   unsigned int numind,
					   unsigned int usetex,
					   unsigned int allocbuff)
{
	struct single_shape* sinshp;
	int is_shp_alloc = 0;

	// Destroy if composite shape
	if (shp == NULL) {
		is_shp_alloc = 1;
		shp = calloc(1,sizeof(*shp));
		if (shp == NULL)
			return NULL;
	} else if (shp->drawproc != draw_single_shape) 
		shp->destroyproc(shp->data);

	// Allocate shapes structs if necessary
	sinshp = alloc_single_shape(shp->data, numvert, numind,
	                                      usetex, allocbuff);
	if (sinshp == NULL) {
		if (is_shp_alloc)
			free(shp);
		return NULL;
	}

	shp->data = sinshp;
	shp->drawproc = draw_single_shape;
	shp->setcolorproc = set_single_color;
	shp->destroyproc = destroy_single_shape;
	
	return shp;
}


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
						 unsigned int flags)
{
	struct single_shape* sinshp;
	unsigned int i, alloc = (flags & DTKF_ALLOC);

	// Smart alloc (alloc only what is necessary)
	shp = alloc_generic_shape(shp, nvert, nind, (tex ? 1 : 0), alloc);
	if (shp == NULL)
		return NULL;
	sinshp = shp->data;

	// Copy the buffers if data supplied
	if (alloc && vert)
		memcpy(sinshp->vertices, vert, 2*nvert*sizeof(*vert));
	if (alloc && ind)
		memcpy(sinshp->indices, ind, nind*sizeof(*ind));
	if (alloc && tc)
		memcpy(sinshp->texcoords, tc, 2*nvert*sizeof(*tc));
	if (alloc && col) {
		if (flags & DTKF_UNICOLOR) {
			for (i=0; i<4*nvert; i+=4)
				memcpy(sinshp->colors+i, col,4*sizeof(*col));
		} else
			memcpy(sinshp->colors, col, 4*nvert*sizeof(*col));
	}
	
	// Fill the structures
	sinshp->primtype = primtype;
	sinshp->tex = tex;

	return shp;
}
                          

/*************************************************************************
 *                                                                       *
 *                      Composite shape functions                        *
 *                                                                       *
 *************************************************************************/

struct composite_shape
{
	struct dtk_shape** array;
	unsigned int num;
	int free_children;
};


/*******************
 * implementations *
 *******************/
static void draw_composite_shape(const struct dtk_shape* shp)
{
	unsigned int i;
	struct composite_shape* compshp = shp->data;

	for(i=0; i<compshp->num; i++)
		dtk_draw_shape(compshp->array[i]);
}

static void set_composite_color(const struct dtk_shape* shp, const float* color, unsigned int mask)
{
	struct composite_shape* compshp;
	struct dtk_shape** array;
	unsigned int num, j;

	compshp = shp->data;
	num = compshp->num;
	array = compshp->array;

	for (j = 0; j<num; j++)
		array[j]->setcolorproc(array[j], color, mask);

}

static void destroy_composite_shape(void* data)
{
	unsigned int i;
	struct composite_shape* compshp = data;
	if (compshp == NULL)
		return;

	for (i=0; i<compshp->num; i++)
		dtk_destroy_shape(compshp->array[i]);

	free(compshp->array);
	free(compshp);
}


static
struct composite_shape* alloc_composite_shape(struct composite_shape* cshp, 
                                              unsigned int num_shp)
{
	struct dtk_shape** shplist = NULL;
	int is_cshp_alloc = 0;

	if (cshp == NULL) {
		is_cshp_alloc = 1;
		cshp = calloc(1, sizeof(*cshp));
		if (cshp == NULL)
			return NULL;
	}

	if (cshp->num != num_shp) {
		shplist = malloc(num_shp*sizeof(*shplist));
		if (shplist == NULL) {
			if (is_cshp_alloc)
				free(cshp);
			return NULL;
		}

		free(cshp->array);
		cshp->array = shplist;
		cshp->num = num_shp;
	}

	return cshp;
}


API_EXPORTED
dtk_hshape dtk_create_composite_shape(struct dtk_shape* shp,
                                      unsigned int num_shp,
                                      const dtk_hshape* array,
				      int free_children)
{
	struct composite_shape* compshp = NULL;;
	int is_shp_alloc = 0;	

	// check arguments
	if (num_shp && !array)
		return NULL;

	// Alloc shape structure
	if (shp == NULL) {
		is_shp_alloc = 1;
		shp = calloc(1,sizeof(*shp));
		if (!shp)
			return NULL;
	} else if (shp->destroyproc != destroy_composite_shape) {
		shp->destroyproc(shp->data);
		shp->data = NULL;
	}
	
	// Alloc composite substructure
	compshp = alloc_composite_shape(shp->data, num_shp);
	if (compshp == NULL) {
		if (is_shp_alloc)	
			free(shp);
		return NULL;
	}
	
	// Setup the structure
	shp->data = compshp;
	shp->drawproc = draw_composite_shape;
	shp->setcolorproc = set_composite_color;
	shp->destroyproc = destroy_composite_shape;
	compshp->free_children = free_children;

	// Copy the list of shapes
	if (num_shp)
		memcpy(compshp->array, array, num_shp*sizeof(*array));

	return shp;
}

/*************************************************************************
 *                                                                       *
 *                       Generic shape functions                         *
 *                                                                       *
 *************************************************************************/
API_EXPORTED
void dtk_draw_shape(struct dtk_shape* shp)
{
	glPushMatrix();

	// Translate to the reference point
	glTranslatef(shp->pos[0], shp->pos[1], 0.0f);
	glRotatef(shp->Rot, 0.0f, 0.0f, 1.0f); 
	
	shp->drawproc(shp);

	// Reset drawing  position
	glPopMatrix(); 
}
                      

API_EXPORTED
void dtk_move_shape(dtk_hshape shp, float x, float y)
{
	shp->pos[0] = x;
	shp->pos[1] = y;
} 


API_EXPORTED
void dtk_relmove_shape(dtk_hshape shp, float dx, float dy)
{
	shp->pos[0] += dx;
	shp->pos[1] += dy;
}


API_EXPORTED
void dtk_rotate_shape(dtk_hshape shp, float deg)
{
	shp->Rot = deg;
}


API_EXPORTED
void dtk_relrotate_shape(dtk_hshape shp, float ddeg)
{
	shp->Rot += ddeg;
}


API_EXPORTED
void dtk_destroy_shape(dtk_hshape shp)
{
	if (!shp) 
		return;
	if (shp->destroyproc)
		shp->destroyproc(shp->data);
	free(shp);
}

API_EXPORTED
void dtk_setcolor_shape(dtk_hshape shp, const float* color, unsigned int mask)
{
	
	shp->setcolorproc(shp, color, mask);

}

