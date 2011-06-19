#ifndef deferred_renderer_h
#define deferred_renderer_h

#include "camera.h"
#include "geometry.h"
#include "material.h"
#include "renderable.h"

void deferred_renderer_init();

void deferred_renderer_finish();

void deferred_renderer_set_camera(camera* cam);
void deferred_renderer_set_viewport(int width, int height);

void deferred_renderer_setup_camera();

void deferred_renderer_begin();
void deferred_renderer_end();

void deferred_renderer_render_model(render_model* m, material* mat);
void deferred_renderer_render_renderable(renderable* r);

void deferred_renderer_use_material(material* mat);

#endif