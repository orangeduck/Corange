#ifndef forward_renderer_h
#define forward_renderer_h

#include "assets/skeleton.h"
#include "assets/texture.h"

#include "entities/camera.h"
#include "entities/light.h"
#include "entities/landscape.h"
#include "entities/static_object.h"
#include "entities/physics_object.h"
#include "entities/animated_object.h"
#include "entities/instance_object.h"

void forward_renderer_init();
void forward_renderer_finish();

void forward_renderer_set_camera(camera* c);
void forward_renderer_set_shadow_light(light* l);
void forward_renderer_set_shadow_texture(texture* t);
void forward_renderer_set_depth_texture(texture* t);
void forward_renderer_set_color_correction(texture* t);

void forward_renderer_add_light(light* l);
void forward_renderer_remove_light(light* l);

void forward_renderer_begin();
void forward_renderer_end();

void forward_renderer_render_static(static_object* so);
void forward_renderer_render_instance(instance_object* io);
void forward_renderer_render_physics(physics_object* po);
void forward_renderer_render_animated(animated_object* ao);
void forward_renderer_render_skeleton(skeleton* s);
void forward_renderer_render_axis(matrix_4x4 position);
void forward_renderer_render_light(light* l);
void forward_renderer_render_collision_body(collision_body* cb);
void forward_renderer_render_landscape(landscape* ls);

#endif
