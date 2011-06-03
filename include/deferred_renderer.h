#ifndef deferred_renderer_h
#define deferred_renderer_h

#include "camera.h"
#include "geometry.h"

void deferred_renderer_init();

void deferred_renderer_finish();

void deferred_renderer_set_camera(camera* cam);
void deferred_renderer_set_viewport(int width, int height);

void deferred_renderer_setup_camera();

void deferred_renderer_begin();
void deferred_renderer_end();

void deferred_renderer_render_model(model* m);


#endif