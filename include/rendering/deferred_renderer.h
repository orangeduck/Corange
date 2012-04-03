#ifndef deferred_renderer_h
#define deferred_renderer_h

#include "assets/material.h"

#include "entities/camera.h"
#include "entities/light.h"
#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/landscape.h"

void deferred_renderer_init();
void deferred_renderer_finish();

void deferred_renderer_set_camera(camera* cam);
void deferred_renderer_set_shadow_light(light* l);
void deferred_renderer_set_shadow_texture(texture* t);
void deferred_renderer_set_color_correction(texture* t);
void deferred_renderer_set_viewport(int width, int height);

void deferred_renderer_add_light(light* l);
void deferred_renderer_remove_light(light* l);

void deferred_renderer_begin();
void deferred_renderer_end();

void deferred_renderer_render_static(static_object* s);
void deferred_renderer_render_animated(animated_object* ao);
void deferred_renderer_render_landscape(landscape* l);
void deferred_renderer_render_light(light* l);
void deferred_renderer_render_axis(matrix_4x4 world);

#endif
