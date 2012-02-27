#ifndef depth_mapper_h
#define depth_mapper_h

#include "static_object.h"
#include "animated_object.h"
#include "texture.h"
#include "camera.h"
#include "landscape.h"

void depth_mapper_init(camera* c);
void depth_mapper_finish();

void depth_mapper_begin();
void depth_mapper_end();
void depth_mapper_render_static(static_object* s);
void depth_mapper_render_animated(animated_object* ao);
void depth_mapper_render_landscape(landscape* ls);

texture* depth_mapper_depth_texture();

#endif