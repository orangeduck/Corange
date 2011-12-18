#ifndef volume_renderer_h
#define volume_renderer_h

#include "corange.h"

void volume_renderer_init();
void volume_renderer_finish();

void volume_renderer_set_camera(camera* cam);

void volume_renderer_begin();
void volume_renderer_end();
void volume_renderer_render_point(vector3 point, vector3 color);
void volume_renderer_render_metaball(vector3 position, vector3 color);

#endif