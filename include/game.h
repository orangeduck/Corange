#ifndef game_h
#define game_h

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"

void game_load(char* name);
void game_unload();

void game_init();

void game_update();
void game_event(SDL_Event event);
void game_render();

void game_finish();


#endif