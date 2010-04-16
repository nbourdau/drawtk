#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <SDL/SDL_opengl.h>
#include "drawtk.h"

struct dtk_texture_manager;

GLuint get_texture_id(struct dtk_texture* tex);
void acquire_texture_manager(void);
void release_texture_manager(void);

#endif
