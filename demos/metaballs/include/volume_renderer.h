#ifndef volume_renderer_h
#define volume_renderer_h

#include "corange.h"

void volume_renderer_init();
void volume_renderer_finish();

void volume_renderer_set_camera(camera* cam);
void volume_renderer_set_light(light* sun);

void volume_renderer_metaball_data(kernel_memory positions, int num_balls);
void volume_renderer_update();
void volume_renderer_render();

#endif