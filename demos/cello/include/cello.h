#ifndef cello_h
#define cello_h

#include "corange.h"

void cello_init();
void cello_event(SDL_Event e);
void cello_update();
void cello_render();
void cello_finish();

void swap_renderer();

#endif