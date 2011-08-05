/*
    Copyright (C) 2009-2010  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Nicolas Bourdaud <nicolas.bourdaud@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <FreeImage.h>
#include "drawtk.h"
#include "texmanager.h"

static
int find_dib_color_settings(FIBITMAP *dib, GLint* intfmt,
                            GLenum* fmt, GLenum* tp)
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

	default:
		return -1;
	}
	
	*intfmt = internalformat;
	*fmt = format;
	*tp = type;
	return 0;
}


 /* Fills an empty structure with the image data referenced by filename and
  * compute mipmaps until level mxlvl (included).
  * Assume that tex->lock is hold
  */
static
int load_texture_from_file(struct dtk_texture* tex, const char* filename,
                            unsigned int mxlvl)
{
	unsigned int w, h, bpp;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP *dib = NULL;
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
	FreeImage_ConvertToRawBits(tex->data[0], dib, tex->sizes[0].stride,
	                       bpp, tex->rmsk, tex->gmsk, tex->bmsk, FALSE);
	compute_mipmaps(tex);

out:
	// Free temporary resources
	FreeImage_Unload(dib);
	return retcode;
}


API_EXPORTED
struct dtk_texture* dtk_load_image(const char* filename, unsigned int mxlvl)
{
	int fail = 0;
	struct dtk_texture *tex = NULL;
	char stringid[256];
	
	// Get new/precreated texture
	snprintf(stringid, sizeof(stringid), "IMAGE:%s", filename);
	if ((tex = get_texture(stringid)) == NULL)
		return NULL;

	// Load the image file
	pthread_mutex_lock(&(tex->lock));
	if (!tex->data) 
		fail = load_texture_from_file(tex, filename, mxlvl);
	pthread_mutex_unlock(&(tex->lock));

	if (fail) {
		rem_texture(tex);
		return NULL;
	} else
		return tex;
}


