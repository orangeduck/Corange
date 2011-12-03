#ifndef shadow_mapper_h
#define shadow_mapper_h

#include "static_object.h"
#include "animated_object.h"
#include "light.h"
#include "texture.h"

void shadow_mapper_init(light* l);
void shadow_mapper_finish();

void shadow_mapper_begin();
void shadow_mapper_end();
void shadow_mapper_setup_camera();
void shadow_mapper_render_static(static_object* s);
void shadow_mapper_render_animated(animated_object* ao);

texture* shadow_mapper_depth_texture();

#endif