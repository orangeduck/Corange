#ifndef shadow_mapper_h
#define shadow_mapper_h

#include "assets/texture.h"

#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/landscape.h"
#include "entities/light.h"

void shadow_mapper_init(light* l);
void shadow_mapper_finish();

void shadow_mapper_begin();
void shadow_mapper_end();
void shadow_mapper_render_static(static_object* s);
void shadow_mapper_render_animated(animated_object* ao);
void shadow_mapper_render_landscape(landscape* ls);

texture* shadow_mapper_depth_texture();

#endif
