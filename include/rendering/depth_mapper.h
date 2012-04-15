#ifndef depth_mapper_h
#define depth_mapper_h

#include "assets/texture.h"

#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/landscape.h"
#include "entities/camera.h"

void depth_mapper_init(camera* c);
void depth_mapper_finish();

void depth_mapper_begin();
void depth_mapper_end();
void depth_mapper_render_static(static_object* s);
void depth_mapper_render_animated(animated_object* ao);
void depth_mapper_render_landscape(landscape* ls);

texture* depth_mapper_depth_texture();

#endif
