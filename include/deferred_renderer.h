#ifndef deferred_renderer_h
#define deferred_renderer_h

#include "camera.h"
#include "light.h"
#include "material.h"
#include "static_object.h"

void deferred_renderer_init();
void deferred_renderer_finish();

void deferred_renderer_set_camera(camera* cam);
void deferred_renderer_set_light(light* l);
void deferred_renderer_set_shadow_texture(texture* t);
void deferred_renderer_set_color_correction(texture* t);
void deferred_renderer_set_viewport(int width, int height);

void deferred_renderer_begin();
void deferred_renderer_end();

void deferred_renderer_render_static(static_object* s);
void deferred_renderer_render_animated(animated_object* ao);

#endif