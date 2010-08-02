#include <SDL/SDL_opengl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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


static void draw_single_shape(const struct dtk_shape* shp);
static void destroy_single_shape(struct dtk_shape* shp);
static struct dtk_shape* alloc_generic_shape(struct dtk_shape* shp,
                                           unsigned int numvert,
					   unsigned int numind,
					   unsigned int usetex);
/*******************
 * Implementations *
 *******************/
static void draw_single_shape(const struct dtk_shape* shp)
{
	struct single_shape* sinshp = shp->data;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, sinshp->vertices);
	
	glColor4fv(sinshp->color);		
	glBindTexture(GL_TEXTURE_2D, get_texture_id(sinshp->tex));
	if (sinshp->texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, sinshp->texcoords);	
	}

	// Draw shapes
	glDrawElements(sinshp->primtype, sinshp->num_ind, GL_UNSIGNED_INT, sinshp->indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void destroy_single_shape(struct dtk_shape* shp)
{
	struct single_shape* sinshp = shp->data;

	if (sinshp) {
		free(sinshp->vertices);
		free(sinshp->texcoords);
		free(sinshp->indices);
		free(sinshp);
	}
}

static struct dtk_shape* alloc_generic_shape(struct dtk_shape* shp,
                                           unsigned int numvert,
					   unsigned int numind,
					   unsigned int usetex)
{
	struct single_shape* sinshp;
	GLuint* indbuff = NULL;
	GLfloat *vertbuff = NULL, *texbuff = NULL;

	// Destroy if composite shape
	if (shp && (shp->drawproc != draw_single_shape)) {
		dtk_destroy_shape(shp);
		shp = NULL;
	}
	
	// Allocate shapes structs if necessary
	if (!shp) {
		shp = calloc(1,sizeof(*shp));
		sinshp = calloc(1,sizeof(*sinshp));
		if (!shp || !sinshp)
			goto error;
		
		shp->data = sinshp;
		shp->drawproc = draw_single_shape;
		shp->destroyproc = destroy_single_shape;
	} else
		sinshp = shp->data;


	// Check for the need of buffer allocation
	if ( (numvert == sinshp->num_vert) && 
	     (numind == sinshp->num_ind) &&
	     (usetex == sinshp->usetex) )
		return shp;

	// Free previous buffers (if any)
	free(sinshp->indices);
	free(sinshp->vertices);
	free(sinshp->texcoords);
	
	// Memory allocation of buffers
	indbuff = malloc(numind*sizeof(*indbuff));
	vertbuff = malloc(numvert*2*sizeof(*vertbuff));
	if (usetex)
		texbuff = malloc(numvert*2*sizeof(*texbuff));
	if (!vertbuff || !indbuff || (usetex && !texbuff))
		goto error;


	sinshp->indices = indbuff;
	sinshp->vertices = vertbuff;
	sinshp->texcoords = texbuff;
	sinshp->num_ind = numind;
	sinshp->num_vert = numvert;
	sinshp->usetex = usetex;
	
	return shp;

error:
	free(shp);
	free(sinshp);
	free(vertbuff);
	free(indbuff);
	free(texbuff);
	return NULL;
}


struct dtk_shape* create_generic_shape(struct dtk_shape* shp,
                                                 unsigned int numvert,
                                                 const GLfloat* vertices, 
						 const GLfloat* texcoords,
						 unsigned int numind,
                                                 const GLuint* indices,
						 GLenum primtype,
						 const GLfloat* color,
						 struct dtk_texture* tex)
{
	struct single_shape* sinshp;

	// Smart alloc (alloc only what is necessary)
	shp = alloc_generic_shape(shp, numvert, numind, (tex ? 1 : 0));
	if (!shp)
		return NULL;
	sinshp = shp->data;

	// Copy the buffers if data supplied
	if (vertices)
		memcpy(sinshp->vertices, vertices, 2*numvert*sizeof(*vertices));
	if (indices)
		memcpy(sinshp->indices, indices, numind*sizeof(*indices));
	if (tex && texcoords)
		memcpy(sinshp->texcoords, texcoords, 2*numvert*sizeof(*texcoords));
		
	// Fill the structures
	sinshp->primtype = primtype;
	memcpy(sinshp->color, color, sizeof(sinshp->color));
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
};


static void draw_composite_shape(const struct dtk_shape* shp);
static void destroy_composite_shape(struct dtk_shape* shp);


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

static void destroy_composite_shape(struct dtk_shape* shp)
{
	unsigned int i;
	struct composite_shape* compshp = shp->data;

	for (i=0; i<compshp->num; i++)
		dtk_destroy_shape(compshp->array[i]);

	free(compshp->array);
	free(compshp);
}
 //

dtk_hshape dtk_create_composite_shape(const dtk_hshape* shp_array, unsigned int num_shp)
{
	struct composite_shape* compshp = NULL;;
	struct dtk_shape* shp = NULL;
	struct dtk_shape** shp_buff = NULL;

	// check arguments
	if (num_shp && !shp_array)
		return NULL;

	// Memory allocation
	shp = calloc(1,sizeof(*shp));
	compshp = calloc(1,sizeof(*compshp));
	shp_buff = malloc(num_shp*sizeof(*shp_buff));
	if (!shp || !compshp || (!shp_buff && num_shp)) {
		free(shp_buff);
		free(compshp);
		free(shp);
		return NULL;
	}

	// Copy the list of shapes
	if (num_shp)
		memcpy(shp_buff, shp_array, num_shp*sizeof(*shp_buff));

	// Setup the structure
	compshp->array = shp_buff;
	compshp->num = num_shp;
	shp->data = compshp;
	shp->drawproc = draw_composite_shape;
	shp->destroyproc = destroy_composite_shape;

	return shp;
}

/*************************************************************************
 *                                                                       *
 *                       Generic shape functions                         *
 *                                                                       *
 *************************************************************************/

void dtk_draw_shape(struct dtk_shape* shp)
{
	assert(shp != NULL);

	glPushMatrix();

	// Translate to the reference point
	glTranslatef(shp->pos[0], shp->pos[1], 0.0f);
	glRotatef(shp->Rot, 0.0f, 0.0f, 1.0f); 
	
	shp->drawproc(shp);

	// Reset drawing  position
	glPopMatrix(); 
}
                      

void dtk_move_shape(dtk_hshape shp, float x, float y)
{
	assert(shp != NULL);
	shp->pos[0] = x;
	shp->pos[1] = y;
} 

void dtk_relmove_shape(dtk_hshape shp, float dx, float dy)
{
	assert(shp != NULL);
	shp->pos[0] += dx;
	shp->pos[1] += dy;
}

void dtk_rotate_shape(dtk_hshape shp, float deg)
{
	assert(shp != NULL);
	shp->Rot = deg;
}

void dtk_relrotate_shape(dtk_hshape shp, float ddeg)
{
	assert(shp != NULL);
	shp->Rot += ddeg;
}

void dtk_destroy_shape(dtk_hshape shp)
{
	if (!shp) 
		return;
	if (shp->destroyproc)
		shp->destroyproc(shp);
	free(shp);
}

