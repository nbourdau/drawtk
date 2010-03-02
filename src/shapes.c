#include <SDL/SDL_opengl.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "feedback.h"
#include "window.h"
#include "texmanager.h"

#include <stdio.h>

/*************************
 * Internal declarations *
 *************************/
typedef void (*DrawShapeFn)(const struct fb_shape* shp);
typedef void (*DestroyShapeFn)(struct fb_shape* shp);

struct fb_shape
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


/*************************************************************************
 *                                                                       *
 *                        Single shape functions                         *
 *                                                                       *
 *************************************************************************/

struct single_shape
{
	GLuint num_ind;
	GLuint* indices;
	GLuint num_vert;
	GLfloat* vertices;
	GLfloat* texcoords;
	unsigned int usetex;
	GLfloat color[4];
	GLuint texid;
	GLenum primtype;
	struct fb_window* wnd;
};

static void draw_single_shape(const struct fb_shape* shp);
static void destroy_single_shape(struct fb_shape* shp);
static struct fb_shape* alloc_generic_shape(struct fb_shape* shp,
                                           unsigned int numvert,
					   unsigned int numind,
					   unsigned int usetex);
static struct fb_shape* create_generic_shape(struct fb_shape* shp,
                                                 unsigned int numvert,
                                                 const GLfloat* vertices, 
						 const GLfloat* texcoords,
						 unsigned int numind,
                                                 const GLuint* indices,
						 GLenum primtype,
						 const GLfloat* color,
						 const struct fb_texture* tex);
/*******************
 * Implementations *
 *******************/
static void draw_single_shape(const struct fb_shape* shp)
{
	struct single_shape* sinshp = shp->data;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, sinshp->vertices);
	
	glColor4fv(sinshp->color);		
	glBindTexture(GL_TEXTURE_2D,sinshp->texid);
	if (sinshp->texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, sinshp->texcoords);	
	}

	// Draw shapes
	glDrawElements(sinshp->primtype, sinshp->num_ind, GL_UNSIGNED_INT, sinshp->indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void destroy_single_shape(struct fb_shape* shp)
{
	struct single_shape* sinshp = shp->data;

	if (sinshp) {
		free(sinshp->vertices);
		free(sinshp->texcoords);
		free(sinshp->indices);
		free(sinshp);
	}
}

static struct fb_shape* alloc_generic_shape(struct fb_shape* shp,
                                           unsigned int numvert,
					   unsigned int numind,
					   unsigned int usetex)
{
	struct single_shape* sinshp;
	GLuint* indbuff = NULL;
	GLfloat *vertbuff = NULL, *texbuff = NULL;

	// Destroy if composite shape
	if (shp && (shp->drawproc != draw_single_shape)) {
		fb_destroy_shape(shp);
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


static struct fb_shape* create_generic_shape(struct fb_shape* shp,
                                                 unsigned int numvert,
                                                 const GLfloat* vertices, 
						 const GLfloat* texcoords,
						 unsigned int numind,
                                                 const GLuint* indices,
						 GLenum primtype,
						 const GLfloat* color,
						 const struct fb_texture* tex)
{
	struct single_shape* sinshp;
	struct fb_window* wnd;

	// Check that there is a usable window
	wnd = current_window;
	/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * I do not care if this is NULL!
	 */
	/*
	   if (!wnd)
	   return NULL;
	 */
		
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
	sinshp->wnd = wnd;
	memcpy(sinshp->color, color, sizeof(sinshp->color));
	if (tex)
		sinshp->texid = get_texture_id(tex);

	return shp;
}
                          
#define TWO_PI ((float)(2.0*M_PI))
fb_hshape fb_create_circle(struct fb_shape* shp, float cx, float cy, float r, int isfull, const float* color, unsigned int numpoints)
{	                  
	unsigned int i,j;
	struct single_shape* sinshp;
	GLuint numvert = isfull ? numpoints+1 : numpoints;
	GLuint numind = isfull ? numpoints+2 : numpoints;
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;

	shp = create_generic_shape(shp, numvert, NULL, NULL,numind, NULL, primtype, color,NULL);
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

fb_hshape fb_create_cross(struct fb_shape* shp, float cx, float cy, float width, const float* color) 
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

	shp = create_generic_shape(shp, 4, vertices, NULL,4, indices, primtype, color,NULL);
	
	return shp;
}

fb_hshape fb_create_rectangle_2p(struct fb_shape* shp, float p1_x, float p1_y, float p2_x, float p2_y, int isfull, const float* color)
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

	shp = create_generic_shape(shp, 4, vertices, NULL,4, indices, primtype, color,NULL);
	
	return shp;
}


fb_hshape fb_create_rectangle_hw(struct fb_shape* shp, float cx, float cy, float width, float height, int isfull, const float* color)
{
	float vertices[8];
	GLuint indices[4] = {0, 1, 2, 3};
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;
	
	vertices[0] = 0 + cx;
	vertices[1] = 0 + cy;
	
	vertices[2] = 0 + cx;
	vertices[3] = height + cy;
	
	vertices[4] = width + cx;
	vertices[5] = height + cy;
	
	vertices[6] = width + cx;
	vertices[7] = 0 + cy;

	shp = create_generic_shape(shp, 4, vertices, NULL,4, indices, primtype, color,NULL);
	
	return shp;
}


fb_hshape fb_create_arrow(struct fb_shape* shp, float cx, float cy, float width, float height, int isfull, const float* color)
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

	shp = create_generic_shape(shp, 7, vertices, NULL, 7, indices, primtype, color,NULL);
	return shp;
}


fb_hshape fb_create_triangle(struct fb_shape* shp, float x1, float y1, float x2, float y2, float x3, float y3, int isfull, const float* color)
{
	GLfloat vertices[] = {x1, y1, x2, y2, x3, y3};
	GLuint indices[] = {0, 1, 2};
	GLenum primtype = isfull ? GL_TRIANGLES : GL_LINE_LOOP;
	
	return create_generic_shape(shp, 3, vertices, NULL, 3, indices, primtype, color,NULL);
}


fb_hshape fb_create_line(struct fb_shape* shp, float x1, float y1, float x2, float y2, const float* color)
{
	GLfloat vertices[] = {x1, y1, x2, y2};
	GLuint indices[] = {0, 1};
	
	return create_generic_shape(shp, 2, vertices, NULL, 2, indices, GL_LINES, color,NULL);
}


fb_hshape fb_create_shape(struct fb_shape* shp, unsigned int numvert, const float* vertex_array, int isfull, const float* color)
{
	unsigned int i;
	struct single_shape* sinshp; 
	GLenum primtype = isfull ? GL_TRIANGLE_FAN : GL_LINE_LOOP;

	shp = create_generic_shape(shp, numvert, vertex_array, NULL, numvert, NULL, primtype, color,NULL);
	if (shp) {
		sinshp = shp->data;
		for (i=0; i<numvert; i++)
			sinshp->indices[i] = i;
	}

	return shp;
}

fb_hshape fb_create_image(struct fb_shape* shp, float x, float y, float width, float height, const float* color, fb_htex image)
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
	
	shp = create_generic_shape(shp, 4, vertices, textcoords, 4, indices, primtype, color, image);
	
	return shp;
}


fb_hshape fb_create_string(struct fb_shape* shp, const char* str_text, float size, float x, float y, const float* color, const char* filepath)
{
	GLfloat primv[8] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};
	float tex_w, tex_h, tx, ty;
	GLint primind[6] = {0, 1, 2, 0, 2, 3};
	GLfloat *vertices, *texcoords;
	GLuint* indices;
	int i, j, len, row, col;
	int row_num = 14, col_num =16;
	GLenum primtype = GL_TRIANGLES;

	len = str_text ? strlen(str_text) : 0;

	shp = create_generic_shape(shp, 4*len, NULL, NULL, 6*len, NULL, primtype, color, fb_load_image(filepath, 3));
	if (!shp)
		return NULL;
	
	indices = ((struct single_shape*)(shp->data))->indices;
	vertices = ((struct single_shape*)(shp->data))->vertices;
	texcoords = ((struct single_shape*)(shp->data))->texcoords;

	// Size of a letter in texture coordinates
	tex_w = (1.0f/(float)col_num);
	tex_h = (1.0f/(float)row_num);
	
	// Create all other letter vertices
	for(i=0; i<len; i++) {
		// Find coordinates of letter in the image
		row = (str_text[i]-32)/col_num;
		col = (str_text[i]-32)%col_num;

		// Texture coordinate of down left vertex of the letter
		tx = tex_w*(float)col;
		ty = 1.0f - tex_h*(float)(row+1);
		
		for (j=0; j<4; j++) {
			vertices[8*i+2*j  ] = size * primv[2*j  ] + x;
			vertices[8*i+2*j+1] = size * primv[2*j+1] + y;
			
			texcoords[8*i+2*j  ] = tex_w * primv[2*j  ] + tx;
			texcoords[8*i+2*j+1] = tex_h * primv[2*j+1] + ty;
		}
		x += size;
	}

	//Create all indices
	for(i=0;i<len;i++)
		for (j=0; j<6; j++)
			indices[6*i+j] = 4*i + primind[j];
		
	return shp;
}


/*************************************************************************
 *                                                                       *
 *                      Composite shape functions                        *
 *                                                                       *
 *************************************************************************/

struct composite_shape
{
	struct fb_shape** array;
	unsigned int num;
};


static void draw_composite_shape(const struct fb_shape* shp);
static void destroy_composite_shape(struct fb_shape* shp);


/*******************
 * implementations *
 *******************/
static void draw_composite_shape(const struct fb_shape* shp)
{
	unsigned int i;
	struct composite_shape* compshp = shp->data;

	for(i=0; i<compshp->num; i++)
		fb_draw_shape(compshp->array[i]);
}

static void destroy_composite_shape(struct fb_shape* shp)
{
	unsigned int i;
	struct composite_shape* compshp = shp->data;

	for (i=0; i<compshp->num; i++)
		fb_destroy_shape(compshp->array[i]);

	free(compshp->array);
	free(compshp);
}
 //

fb_hshape fb_create_composite_shape(const fb_hshape* shp_array, unsigned int num_shp)
{
	struct composite_shape* compshp = NULL;;
	struct fb_shape* shp = NULL;
	struct fb_shape** shp_buff = NULL;

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

void fb_draw_shape(struct fb_shape* shp)
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
                      
/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
 * It breaks the basic principles behind the library, but it 
 * makes my life so easy... :-)
 */
void fb_bind_shape_to_window(struct fb_shape* shp, const fb_hwnd window) {
	/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * Yes, I use assert()!
	 */
	assert(current_window != NULL);
	assert(shp != NULL);
	assert(window != NULL);

	/* 2009-10-15  Michele Tavella <michele.tavella@epfl.ch>
	 * And yes, I rape your structures!
	 */
	struct single_shape* data = (struct single_shape*)(shp->data);
	data->wnd = window;
}

void fb_move_shape(fb_hshape shp, float x, float y)
{
	assert(shp != NULL);
	shp->pos[0] = x;
	shp->pos[1] = y;
} 

void fb_relmove_shape(fb_hshape shp, float dx, float dy)
{
	assert(shp != NULL);
	shp->pos[0] += dx;
	shp->pos[1] += dy;
}

void fb_rotate_shape(fb_hshape shp, float deg)
{
	assert(shp != NULL);
	shp->Rot = deg;
}

void fb_relrotate_shape(fb_hshape shp, float ddeg)
{
	assert(shp != NULL);
	shp->Rot += ddeg;
}

void fb_destroy_shape(fb_hshape shp)
{
	if (!shp) 
		return;
	if (shp->destroyproc)
		shp->destroyproc(shp);
	free(shp);
}

