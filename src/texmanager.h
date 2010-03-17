#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <SDL/SDL_opengl.h>
#include "drawtk.h"

struct dtk_texture_manager;

GLuint get_texture_id(const struct dtk_texture* tex);
void dtk_delete_textures(void);
struct dtk_texture_manager* create_texture_manager(void);
void destroy_texture_manager(struct dtk_texture_manager* texman);

#endif
