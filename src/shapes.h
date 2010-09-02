#ifndef SHAPES_H
#define SHAPES_H

#include <SDL/SDL_opengl.h>

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
