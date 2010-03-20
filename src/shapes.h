#ifndef SHAPES_H
#define SHAPES_H

#include <SDL/SDL_opengl.h>

typedef void (*DrawShapeFn)(const struct dtk_shape* shp);
typedef void (*DestroyShapeFn)(struct dtk_shape* shp);

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
	GLuint num_ind;
	GLuint* indices;
	GLuint num_vert;
	GLfloat* vertices;
	GLfloat* texcoords;
	unsigned int usetex;
	GLfloat color[4];
	GLuint texid;
	GLenum primtype;
	struct dtk_window* wnd;
};

struct dtk_shape* create_generic_shape(struct dtk_shape* shp,
                                       unsigned int numvert,
                                       const GLfloat* vertices, 
				       const GLfloat* texcoords,
				       unsigned int numind,
                                       const GLuint* indices,
				       GLenum primtype,
				       const GLfloat* color,
				       const struct dtk_texture* tex);

#endif // SHAPES_H
