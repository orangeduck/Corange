#ifndef renderers_h
#define renderers_h

#include "corange.h"

void renderers_init();
void renderers_event(SDL_Event e);
void renderers_update();
void renderers_render();
void renderers_finish();

#endif