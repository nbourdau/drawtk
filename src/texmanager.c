#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <FreeImage.h>
#include <SDL/SDL_opengl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

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
	unsigned int inuse;
	struct dtk_texture* root;
};

// Global texture manager
static struct dtk_texture_manager texman = {
	.lstlock = PTHREAD_MUTEX_INITIALIZER,
	.inuse = 0,
	.root = NULL,
};

/*************************************************************************
 *                                                                       *
 *                      Texture manager functions                        *
 *                                                                       *
 *************************************************************************/
LOCAL_FN
void acquire_texture_manager(void)
{
	pthread_mutex_lock(&texman.lstlock);
	if (texman.inuse == 0)
		FreeImage_Initialise(FALSE);
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

		FreeImage_DeInitialise();
	}
	pthread_mutex_unlock(&texman.lstlock);
}


/* Get a texture identified by desc. If it exists, it increments its number 
 * of use. Otherwise it creates a minimal structure (init lock, and fill
 * string_id) with the flag tex->init set to 0.
 */
LOCAL_FN
struct dtk_texture* get_texture(const char *desc)
{
	struct dtk_texture *tex, **last;

	pthread_mutex_lock(&texman.lstlock);

	// Go through the linked list of textures to search
	// for existing entry
	last = &(texman.root);
	while (*last != NULL) {
		if (strcmp(desc, (*last)->string_id) == 0) 
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
		strncpy(tex->string_id, desc, 255);
		tex->aux = NULL;
		tex->data = NULL;
		
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
	unsigned int i;

	if (tex->id)
		glDeleteTextures(1, &(tex->id));

	if (tex->data) {
		for (i=0; i<=tex->mxlvl; i++)
			free(tex->data[i]);
		free(tex->data);
	}

	free(tex->sizes);
	pthread_mutex_destroy(&(tex->lock));

	if (tex->destroyfn)
		tex->destroyfn(tex);
	else
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
	unsigned int i;
	
	// Allocate texture structure
	tex->mxlvl = mxlvl;
	tex->bpp = bpp;
	if (!(tex->data = calloc((mxlvl+1), sizeof(*(tex->data))))
	    || !(tex->sizes = malloc((mxlvl+1)*sizeof(*(tex->sizes)))))
		return 1;	

	// setup mipmap sizes and allocate bitmaps
	for (i=0; i<=mxlvl; i++) {
		if (!h || !w)
			return 1;

		tex->sizes[i].h = h;
		tex->sizes[i].w = w;
		tex->data[i] = malloc(bpp/sizeof(char)*h*w);
		w /= 2;
		h /= 2;
	}

	return 0;
}


/* Load the texture into the video memory
 * Assume that tex->lock is hold
 */
static
void load_gl_texture(struct dtk_texture* tex)
{
	unsigned int lvl; 

	// creation of the GL texture Object
	glGenTextures(1,&(tex->id));
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex->mxlvl);

	// Load each mipmap in video memory 
	for (lvl=0; lvl<=tex->mxlvl; lvl++) {
		glTexImage2D(GL_TEXTURE_2D, lvl, tex->intfmt, 
		        tex->sizes[lvl].w, tex->sizes[lvl].h, 0,
			tex->fmt, tex->type, tex->data[lvl]);
	}

	// Wait the loading being performed
	glFlush();
	glBindTexture(GL_TEXTURE_2D, 0);
}


API_EXPORTED
void dtk_destroy_texture(struct dtk_texture* tex)
{
	rem_texture(tex);
}


LOCAL_FN
GLuint get_texture_id(struct dtk_texture* tex)
{
	if (!tex) 
		return 0;
	
	// This allows to return quickly since once the texture has been
	// loaded, it won't change until tex is destroyed
	if (tex->id)
		return tex->id;
	
	pthread_mutex_lock(&(tex->lock));
	if (tex->id == 0)
		load_gl_texture(tex);
	pthread_mutex_unlock(&(tex->lock));

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
	
	dib = FreeImage_ConvertFromRawBits(tex->data[0], 
	                tex->sizes[0].w, tex->sizes[0].h,
			tex->sizes[0].w * tex->bpp/8, tex->bpp,
			tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
	
	for (i=1; i<=tex->mxlvl; i++) {
		dib2 = FreeImage_Rescale(dib, 
				tex->sizes[i].w, tex->sizes[i].h,
				FILTER_BICUBIC);
		FreeImage_ConvertToRawBits(tex->data[i], dib2,
				tex->sizes[i].w * tex->bpp/8, tex->bpp,
				tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
		FreeImage_Unload(dib2);
	}
	FreeImage_Unload(dib);
}


