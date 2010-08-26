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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef MAX_MIPMAP
#define MAX_MIPMAP	10
#endif 

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

	// To be used in a linked list
	struct dtk_texture* next_tex;
};

static void free_texture(struct dtk_texture* tex);
static int alloc_image_data(struct dtk_texture* tex, 
                            unsigned int w, unsigned int h, 
			    unsigned int mxlvl, unsigned int bpp);

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
		if (!(tex = calloc(1,sizeof(*tex))))
			goto out;
		pthread_mutex_init(&(tex->lock), NULL);
		strncpy(tex->string_id, desc, 255);
		
		// append at the end of the list
		*last = tex;
	} else {
		tex = *last;
	}
	
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
static int find_dib_color_settings(FIBITMAP *dib, GLint* intfmt, GLenum* fmt, GLenum* tp)
{
	GLenum format, internalformat, type;
	int bpp = FreeImage_GetBPP(dib);

	switch (bpp) {
	case 8:
		format = GL_LUMINANCE;
		type = GL_UNSIGNED_BYTE;
		internalformat = GL_COMPRESSED_LUMINANCE;
	case 16:
		if ((FreeImage_GetRedMask(dib) == FI16_555_RED_MASK)
		 && (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK)
		 && (FreeImage_GetBlueMask(dib) == FI16_555_BLUE_MASK)) {
			format = GL_BGRA; //GL_RGB
			type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			internalformat = GL_COMPRESSED_RGBA;
		} else {
			format = GL_BGR; //GL_RGB
			type = GL_UNSIGNED_SHORT_5_6_5_REV;
			internalformat = GL_COMPRESSED_RGB;
		}
		break;

	case 24:
		format = GL_BGR; //GL_RGB
		type = GL_UNSIGNED_BYTE;
		internalformat = GL_COMPRESSED_RGB;
		break;

	case 32:
		format = GL_BGRA; //GL_RGBA
		type = GL_UNSIGNED_BYTE;
		internalformat = GL_COMPRESSED_RGBA;
		break;
	}
	
	*intfmt = internalformat;
	*fmt = format;
	*tp = type;
	return 0;
}


/* free_texture destroys internals of the texture structure. It is meant to
 * be called by the texture manager when it realized that there is no more
 * reference to that structure
 * Assume that when called, locking is not necessary anymore
 */
static void free_texture(struct dtk_texture* tex)
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
	free(tex);
}

/* Allocate ressources to hold image data until mipmap level mxlvl. On the
 * fly, it also calculate the size each mipmap. This function assumes that
 * tex->lock is hold when called
 */
static int alloc_image_data(struct dtk_texture* tex, unsigned int w, unsigned int h, unsigned int mxlvl, unsigned int bpp)
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


/* Compute the mipmaps for level 1 until level tex->mxlvl. This function
 * assume that image data for all mipmaps has already been allocated.
 * This function assumes that when called, tex->lock is hold
 */
static void compute_mipmaps(struct dtk_texture* tex)
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

/* Load the texture into the video memory
 * Assume that tex->lock is hold
 */
static void load_gl_texture(struct dtk_texture* tex)
{
	unsigned int lvl; 

	// creation of the GL texture Object
	glGenTextures(1,&(tex->id));
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL, tex->mxlvl);

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


 /* Fills an empty structure with the image data referenced by filename and
  * compute mipmaps until level mxlvl (included).
  * Assume that tex->lock is hold
  */
static int load_texture_from_file(struct dtk_texture* tex, const char* filename, unsigned int mxlvl)
{
	unsigned int w, h, bpp;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib;
	int retcode = 0;

	// check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);
	
	// check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) 
		dib = FreeImage_Load(fif, filename, 0);
	if (!dib) {
		fprintf(stderr, "Texture not found or unsupported image format\n");
		return 1;
	}

	// Allocate texture internals
	w = FreeImage_GetWidth(dib);
	h = FreeImage_GetHeight(dib);
	bpp = FreeImage_GetBPP(dib);
	if (alloc_image_data(tex, w, h, mxlvl, bpp)) {
		retcode = 1;
		goto out;
	}

	// Setup texture internals
	find_dib_color_settings(dib, &tex->intfmt, &tex->fmt, &tex->type);
	tex->rmsk = FreeImage_GetRedMask(dib);
	tex->gmsk = FreeImage_GetGreenMask(dib);
	tex->bmsk = FreeImage_GetBlueMask(dib);

	// Fill image data
	FreeImage_ConvertToRawBits(tex->data[0], dib, w * bpp/8, bpp, 
			tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
	compute_mipmaps(tex);

	tex->isinit = 1;

out:
	// Free temporary resources
	FreeImage_Unload(dib);
	return retcode;
}


API_EXPORTED
struct dtk_texture* dtk_load_image(const char* filename, unsigned int mipmap_maxlevel)
{
	int fail = 0;
	struct dtk_texture *tex = NULL;
	
	// Get new/precreated texture
	if (!(tex = get_texture(filename)))
		return NULL;

	// Load the image file
	pthread_mutex_lock(&(tex->lock));
	if (!(tex->isinit))
		fail = load_texture_from_file(tex, filename, mipmap_maxlevel);
	pthread_mutex_unlock(&(tex->lock));


	if (fail) {
		rem_texture(tex);
		return NULL;
	} else
		return tex;
	
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

