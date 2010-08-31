#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <GL/gl.h>
#include <pthread.h>
#include "drawtk.h"

struct dtk_texture;

typedef void (*destroyproc)(struct dtk_texture*);
typedef struct dtk_texture* (*createproc)(const char*);

struct dtk_size {
	unsigned int w, h;
};

// Structure for textures
struct dtk_texture
{
	// Bitmaps
	struct dtk_size* sizes;
	void** data;
	unsigned int mxlvl;
	void *aux;

	// Pixel type
	GLint intfmt;
	GLenum fmt, type;
	unsigned int bpp, rmsk, bmsk, gmsk;

	// Texture usage
	GLuint id;
	unsigned int nused;
	char string_id[256];
	int isinit;

	// update lock
	pthread_mutex_t lock;

	// Destroy function
	destroyproc destroyfn;

	// To be used in a linked list
	struct dtk_texture* next_tex;
};


LOCAL_FN void acquire_texture_manager(void);
LOCAL_FN void release_texture_manager(void);
int alloc_image_data(struct dtk_texture* tex,
			unsigned int w, unsigned int h,
			unsigned int mxlvl, unsigned int bpp);
struct dtk_texture* get_texture(const char *desc);
void rem_texture(struct dtk_texture* tex);

LOCAL_FN GLuint get_texture_id(struct dtk_texture* tex);
LOCAL_FN void compute_mipmaps(struct dtk_texture* tex);

#endif
