#include <FreeImage.h>
#include <SDL/SDL_opengl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "texmanager.h"
#include "window.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef MAX_MIPMAP
#define MAX_MIPMAP	10
#endif 

// Structure for textures
struct fb_texture
{
	unsigned int width, height;
	GLuint id;
	unsigned int used_num;
	char string_id[256];
	
	// To be used in a linked list
	struct fb_texture* next_tex;
};

// Structure for texture manager
struct fb_texture_manager
{
	struct fb_texture* root;
};

// Global texture manager
static struct fb_texture_manager texman = {
	.root = NULL
};

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

static void load_gl_texture(struct fb_texture* tex, void* data[], unsigned int maxlevel, GLenum format, GLenum internalformat, GLenum type)
{
	unsigned int lvl; 
	GLuint w = tex->width, h = tex->height;

	// creation of the texture Object
	glGenTextures(1,&(tex->id));
	glBindTexture(GL_TEXTURE_2D, tex->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL, maxlevel);

	// Load each mipmap 
	for (lvl=0; lvl<=maxlevel; lvl++) {
		glTexImage2D(GL_TEXTURE_2D, lvl, internalformat, w, h, 0,
			format, type, data[lvl]);
		w /= 2;
		h /= 2;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}


static struct fb_texture* create_texture_from_file(const char* filename, unsigned int mipmap_maxlevel)
{
	GLenum format, type;
	GLint intformat;
	int level, maxlevel;
	GLuint w, h;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib[MAX_MIPMAP+1] = {NULL};
	void* data[MAX_MIPMAP+1] = {NULL};
	struct fb_texture* tex = NULL; 

	// Limit Mipmap generation to level MAX_MIPMAP
	maxlevel = mipmap_maxlevel <= MAX_MIPMAP ? mipmap_maxlevel : MAX_MIPMAP;

	// check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename);
	
	// check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) 
		dib[0] = FreeImage_Load(fif, filename, 0);
	if (!dib[0]) {
		fprintf(stderr, "Texture not found or unsupported image format\n");
		goto out;
	}
	data[0] = FreeImage_GetBits(dib[0]);

	// Allocate and set texture structure
	tex = calloc(1,sizeof(struct fb_texture)); 
	if (!tex)
		goto out;
	strncpy(tex->string_id, filename, sizeof(tex->string_id)-1);
	w = tex->width = FreeImage_GetWidth(dib[0]);
	h = tex->height = FreeImage_GetHeight(dib[0]);
	tex->used_num = 0;
	
	// Mipmap generation
	for (level=1; level<=maxlevel; level++) {
		h /= 2;
		w /= 2;
		dib[level] = FreeImage_Rescale(dib[0], w, h, FILTER_BICUBIC);
		if (!dib[level])
			break;
		data[level] = FreeImage_GetBits(dib[level]);
	}

	find_dib_color_settings(dib[0], &intformat, &format, &type);
	load_gl_texture(tex, data, maxlevel, format, intformat, type);

out:
	// Free temporary resources
	for (level=0; level<=maxlevel; level++)
		FreeImage_Unload(dib[level]);
	return tex;
}


struct fb_texture* fb_load_image(const char* filename, unsigned int mipmap_maxlevel)
{
	struct fb_texture **last;

	// Go through the linked list of textures to search
	// for existing entry
	last = &(texman.root);
	while (*last != NULL) {
		if (strcmp(filename, (*last)->string_id) == 0)
			return *last;
		last = &((*last)->next_tex);
	}

	// Load the image file
	return *last = create_texture_from_file(filename, mipmap_maxlevel);
}


GLuint get_texture_id(const struct fb_texture* tex)
{
	GLuint texid = 0;

	if (tex)
		texid = tex->id;

	return texid;
}


void fb_delete_textures(void)
{
	struct fb_texture *curr_tex, *next_text;
	
	if (texman.root != NULL) {
		curr_tex = texman.root;
		do {
			next_text = curr_tex->next_tex;
			glDeleteTextures(1,&(curr_tex->id));	
			free(curr_tex);
			curr_tex = next_text;
						
		} while(next_text != NULL);
	}
}


struct fb_texture_manager* create_texture_manager(void)
{
	return &texman;
}

void destroy_texture_manager(struct fb_texture_manager* texman)
{
	(void)texman;
	fb_delete_textures();
}
