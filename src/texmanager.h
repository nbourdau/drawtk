#ifndef TEXMANAGER_H
#define TEXMANAGER_H

#include <SDL/SDL_opengl.h>
#include "drawtk.h"

struct dtk_texture_manager;

LOCAL_FN GLuint get_texture_id(struct dtk_texture* tex);
LOCAL_FN void acquire_texture_manager(void);
LOCAL_FN void release_texture_manager(void);

#endif
