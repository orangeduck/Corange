#ifndef texture_h
#define texture_h

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL.h"
#include "SDL_opengl.h"

typedef GLuint texture;

void texture_delete(texture* t);

#endif