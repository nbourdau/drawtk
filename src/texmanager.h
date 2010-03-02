#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <SDL/SDL_opengl.h>
#include "feedback.h"

struct fb_texture_manager;

GLuint get_texture_id(const struct fb_texture* tex);
void fb_delete_textures(void);
struct fb_texture_manager* create_texture_manager(void);
void destroy_texture_manager(struct fb_texture_manager* texman);

#endif
