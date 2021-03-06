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
#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <SDL_opengl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include "drawtk.h"

#define DTK_PALIGN	(sizeof(int))
struct dtk_texture;

typedef void (*destroyproc)(struct dtk_texture*);
typedef struct dtk_texture* (*createproc)(const char*);

struct mipmapdata {
	uintptr_t offset;
	unsigned int stride, h, w;
};

// Structure for textures
struct dtk_texture
{
	// Bitmaps
	struct mipmapdata* data;
	unsigned int mxlvl;
	void* bmdata;
	void *aux;

	// GL Info
	GLuint pbo[2];
	int ipbo;
	GLuint id;
	GLint intfmt;
	GLenum fmt, type;
	unsigned int bpp, rmsk, bmsk, gmsk;

	// Texture usage
	unsigned int nused;
	uint32_t hash;

	// update lock
	pthread_mutex_t lock;

	// Destroy function
	destroyproc destroyfn;

        bool outdated, isvideo;

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
