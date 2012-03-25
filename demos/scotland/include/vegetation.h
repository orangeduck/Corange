#ifndef vegetation_h
#define vegetation_h

#include "corange.h"

void vegetation_init();
void vegetation_finish();

void vegetation_update();
void vegetation_render();

void vegetation_add_type(terrain* t, renderable* r, float density);

#endif
