/**
*** :: Depth Mapper ::
***
***   Similar to the shadow mapper but
***   renders from the perspective of a
***   camera.
***
***   Meant to be used in effects such as
***   refractions in water and fog.
***
**/

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
