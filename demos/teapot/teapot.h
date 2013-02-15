#ifndef teapot_h
#define teapot_h

#include "corange.h"

void teapot_init();
void teapot_event(SDL_Event e);
void teapot_update();
void teapot_render();
void teapot_finish();

#endif