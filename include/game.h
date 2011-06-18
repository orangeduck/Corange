#ifndef game_h
#define game_h

#include "SDL/SDL.h"

void game_load(char* name);
void game_unload();

void game_init();

void game_update();
void game_event(SDL_Event event);
void game_render();

void game_finish();


#endif