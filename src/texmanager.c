/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
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

#define GL_GLEXT_PROTOTYPES
#define GL_GLXEXT_PROTOTYPES
#include <GL/gl.h>

#include <FreeImage.h>
#include <gst/gst.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "texmanager.h"
#include "window.h"

#ifndef MAX_MIPMAP
#define MAX_MIPMAP	10
#endif 

static void free_texture(struct dtk_texture* tex);

// Structure for texture manager
struct dtk_texture_manager
{
	pthread_mutex_t lstlock;
	unsigned int inuse, isinit;
	struct dtk_texture* root;
};

// Global texture manager
static struct dtk_texture_manager texman = {
	.lstlock = PTHREAD_MUTEX_INITIALIZER,
	.inuse = 0,
	.isinit = 0,
	.root = NULL,
};

/*************************************************************************
 *                                                                       *
 *                      Texture manager functions                        *
 *                                                                       *
 *************************************************************************/
static
void deinit_texman(void)
{
	if (texman.isinit) {
		gst_deinit();
		FreeImage_DeInitialise();
		texman.isinit = 0;
	}
}


static
void init_texman(void)
{
	if (!texman.isinit) {
		gst_init(NULL, NULL);
		FreeImage_Initialise(FALSE);
		texman.isinit = 1;
		atexit(deinit_texman);
	}
}


LOCAL_FN
void acquire_texture_manager(void)
{
	pthread_mutex_lock(&texman.lstlock);
	if (!texman.isinit)
		init_texman();
	texman.inuse++;
	pthread_mutex_unlock(&texman.lstlock);
}


LOCAL_FN
void release_texture_manager(void)
{
	struct dtk_texture *curr_tex, *next_tex;

	pthread_mutex_lock(&texman.lstlock);
	if (--texman.inuse == 0) {
		// Destroy all textures
		curr_tex = texman.root;
		while (curr_tex) {
			next_tex = curr_tex->next_tex;
			free_texture(curr_tex);
			curr_tex = next_tex;
		}
		deinit_texman();

	}
	pthread_mutex_unlock(&texman.lstlock);
}


static
uint32_t djbhash(const char* string)
{
	uint32_t h = 5381;
	unsigned char c;
	const unsigned char *s = (const unsigned char*)string;

        for (c = *s; c != '\0'; c = *++s)
		h = h * 33 + c;

	return h;
}


/* Get a texture identified by desc. If it exists, it increments its number 
 * of use. Otherwise it creates a minimal structure (init lock, and fill
 * string_id) with the flag tex->init set to 0.
 */
LOCAL_FN
struct dtk_texture* get_texture(const char *desc)
{
	struct dtk_texture *tex, **last;
	uint32_t hash = djbhash(desc);

	pthread_mutex_lock(&texman.lstlock);

	if (!texman.isinit)
		init_texman();

	// Go through the linked list of textures to search
	// for existing entry
	last = &(texman.root);
	while (*last != NULL) {
		if (hash == (*last)->hash) 
			break;	
		last = &((*last)->next_tex);
	}

	if (*last == NULL) {
		// Create empty texture
		tex = malloc(sizeof(*tex));
		if (tex == NULL)
			goto out;
		
		memset(tex, 0, sizeof(*tex));
		pthread_mutex_init(&(tex->lock), NULL);
		tex->hash = hash;
		tex->aux = NULL;
		tex->data = NULL;
		tex->bmdata = NULL;
		tex->ipbo = -1;
		
		// append at the end of the list
		*last = tex;
	} else 
		tex = *last;
	
	tex->nused++;
out:
	pthread_mutex_unlock(&texman.lstlock);
	return tex;
}

/* Ask to remove a texture. This decrement the number of uses of the
 * texture. If that number reaches 0, it is actually removed.
 */
LOCAL_FN
void rem_texture(struct dtk_texture* tex)
{
	struct dtk_texture **last;
	int deltex = 0;

	pthread_mutex_lock(&texman.lstlock);

	// Go through the linked list of textures to search
	// for existing entry
	last = &(texman.root);
	while (*last != NULL) {
		if (*last == tex) {
			pthread_mutex_lock(&(tex->lock));
			if (--tex->nused == 0) {
				*last = tex->next_tex;
				deltex = 1;
			}
			pthread_mutex_unlock(&(tex->lock));
			break;
		}
		// jump to the next item
		last = &((*last)->next_tex);
	}

	pthread_mutex_unlock(&texman.lstlock);

	if (deltex)
		free_texture(tex);
}

/*************************************************************************
 *                                                                       *
 *                          Texture functions                            *
 *                                                                       *
 *************************************************************************/
/* free_texture destroys internals of the texture structure. It is meant to
 * be called by the texture manager when it realized that there is no more
 * reference to that structure
 * Assume that when called, locking is not necessary anymore
 */
static
void free_texture(struct dtk_texture* tex)
{
        if (tex->destroyfn)
                tex->destroyfn(tex);

	if (tex->id) {
		glDeleteTextures(1, &(tex->id));
		tex->id = 0;
	}

	if (tex->ipbo >= 0) {
		glDeleteBuffers(2, tex->pbo);
		tex->bmdata = NULL;
	}

	free(tex->bmdata);
	free(tex->data);
	tex->data = NULL;
	tex->bmdata = NULL;

        pthread_mutex_destroy(&(tex->lock));

        free(tex);
}

/* Allocate ressources to hold image data until mipmap level mxlvl. On the
 * fly, it also calculate the size each mipmap. This function assumes that
 * tex->lock is hold when called
 */
LOCAL_FN
int alloc_image_data(struct dtk_texture* tex, 
		     unsigned int w, unsigned int h,
		     unsigned int mxlvl, unsigned int bpp)
{
	unsigned int i, stride, offset = 0;
	
	// Allocate texture structure
	tex->mxlvl = mxlvl;
	tex->bpp = bpp;
	if (!(tex->data = calloc((mxlvl+1), sizeof(*(tex->data)))))
		goto fail;

	// setup mipmap sizes and allocate bitmaps
	for (i=0; i<=mxlvl; i++) {
		if (!h || !w)
			return 1;

		stride = w*bpp/CHAR_BIT;
		// round up to the next DTK_ALIGN-byte boundary
		stride = ((stride+DTK_PALIGN-1)/DTK_PALIGN) * DTK_PALIGN;

		tex->data[i].h = h;
		tex->data[i].w = w;
		tex->data[i].stride = stride;
		tex->data[i].offset = offset;

		offset += h*stride;
		w /= 2;
		h /= 2;
	}
	tex->bmdata = calloc(1, offset);
	if (tex->bmdata)
		return 0;

fail:
	free(tex->data);
	tex->data = NULL;
	return 1;
}


/* Create the GL texture and load the image data into the video memory
 * Assume that tex->lock is NOT hold
 */
static
void create_gl_texture(struct dtk_texture* tex)
{
	unsigned int lvl; 
	char* bm;
	GLsizeiptr dsize = 0;

	pthread_mutex_lock(&tex->lock);
	if (!tex->data) {
		pthread_mutex_unlock(&tex->lock);
		return;
	}


	// creation of the GL texture Object
	glGenTextures(1,&(tex->id));
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex->mxlvl);
	glPixelStorei(GL_UNPACK_ALIGNMENT, DTK_PALIGN);

	// Load each mipmap in video memory 
	bm = tex->bmdata;
	for (lvl=0; lvl<=tex->mxlvl; lvl++) {
		glTexImage2D(GL_TEXTURE_2D, lvl, tex->intfmt, 
		        tex->data[lvl].w, tex->data[lvl].h, 0,
			tex->fmt, tex->type, bm + tex->data[lvl].offset);
	}
	tex->outdated = false;

	glBindTexture(GL_TEXTURE_2D, 0);
	
	if (tex->isvideo) {
		glGenBuffers(2, tex->pbo);
		tex->ipbo = 0;
		for (lvl=0; lvl<=tex->mxlvl; lvl++)
			dsize += tex->data[lvl].h * tex->data[lvl].stride;
	
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo[1]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, dsize, NULL,
		                                            GL_STREAM_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo[0]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, dsize, NULL,
		                                            GL_STREAM_DRAW);

		free(tex->bmdata);
		tex->bmdata = glMapBuffer(GL_PIXEL_UNPACK_BUFFER,
		                                             GL_WRITE_ONLY);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	pthread_mutex_unlock(&tex->lock);
}


/* Update the image data in the GL texture
 * Assume that tex->lock is NOT hold
 */
static
void update_dynamic_texture(struct dtk_texture* tex)
{
	unsigned int lvl;
	int cpbo = tex->ipbo;
	int npbo = (cpbo + 1) % 2;

	pthread_mutex_lock(&tex->lock);
	
	if (!tex->outdated) {
		pthread_mutex_unlock(&tex->lock);
		return;
	}

	// Map the next buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo[npbo]);
	tex->bmdata = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	tex->outdated = false;

	pthread_mutex_unlock(&tex->lock);

	// Unmap the current one to perform texture update
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, tex->pbo[cpbo]);
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	tex->ipbo = npbo;

	glBindTexture(GL_TEXTURE_2D, tex->id);
	// Load each mipmap in video memory 
	for (lvl=0; lvl<=tex->mxlvl; lvl++) {
		glTexSubImage2D(GL_TEXTURE_2D, lvl, 0, 0,
		        tex->data[lvl].w, tex->data[lvl].h,
			tex->fmt, tex->type,
			(const GLvoid*) tex->data[lvl].offset);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}


API_EXPORTED
void dtk_destroy_texture(struct dtk_texture* tex)
{
	rem_texture(tex);
}


API_EXPORTED
void dtk_texture_getsize(struct dtk_texture* tex, unsigned int* w,
                                                  unsigned int* h)
{
	if (!tex || !w || !h)
		return;
	
	if (tex->data) {
		*w = tex->data[0].w;
		*h = tex->data[0].h;
	} else
		*w = *h = 0;
}


LOCAL_FN
GLuint get_texture_id(struct dtk_texture *tex)
{
	if (!tex)
		return 0;

	if (tex->id == 0) 
		create_gl_texture(tex);
	else if (tex->isvideo)
		update_dynamic_texture(tex);

	return tex->id;
}


/* Compute the mipmaps for level 1 until level tex->mxlvl. This function
 * assume that image data for all mipmaps has already been allocated.
 * This function assumes that when called, tex->lock is hold
 */
LOCAL_FN
void compute_mipmaps(struct dtk_texture* tex)
{
	FIBITMAP *dib, *dib2;
	unsigned int i;
	BYTE* bm = tex->bmdata;
	
	dib = FreeImage_ConvertFromRawBits(bm + tex->data[0].offset, 
	                tex->data[0].w, tex->data[0].h,
			tex->data[0].stride, tex->bpp,
			tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
	
	for (i=1; i<=tex->mxlvl; i++) {
		dib2 = FreeImage_Rescale(dib, 
				tex->data[i].w, tex->data[i].h,
				FILTER_BICUBIC);
		FreeImage_ConvertToRawBits(bm + tex->data[i].offset, dib2,
				tex->data[i].stride, tex->bpp,
				tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
		FreeImage_Unload(dib2);
	}
	FreeImage_Unload(dib);
}


