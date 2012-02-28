#ifndef forward_renderer_h
#define forward_renderer_h

#include "camera.h"
#include "light.h"
#include "skeleton.h"
#include "texture.h"
#include "material.h"
#include "landscape.h"
#include "static_object.h"
#include "physics_object.h"
#include "animated_object.h"

void forward_renderer_init();
void forward_renderer_finish();

void forward_renderer_set_camera(camera* c);
void forward_renderer_set_shadow_light(light* l);
void forward_renderer_set_shadow_texture(texture* t);
void forward_renderer_set_depth_texture(texture* t);
void forward_renderer_set_color_correction(texture* t);

void forward_renderer_add_light(light* l);
void forward_renderer_remove_light(light* l);

void forward_renderer_setup_camera();

void forward_renderer_begin();
void forward_renderer_end();

void forward_renderer_render_static(static_object* s);
void forward_renderer_render_physics(physics_object* po);
void forward_renderer_render_animated(animated_object* ao);
void forward_renderer_render_skeleton(skeleton* s);
void forward_renderer_render_axis(matrix_4x4 position);
void forward_renderer_render_light(light* l);
void forward_renderer_render_collision_body(collision_body* cb);
void forward_renderer_render_landscape(landscape* ls);

#endif