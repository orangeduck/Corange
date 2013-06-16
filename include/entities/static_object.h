/**
*** :: Static Object ::
***
***   A basic instance of an object in the world
***   Basically just a scale, position and rotation
***   applied to a renderable and collision body
***
**/

#ifndef static_object_h
#define static_object_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec3 position;
  vec3 scale;
  quat rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  asset_hndl renderable;
  asset_hndl collision_body;
} static_object;

static_object* static_object_new();
void static_object_delete(static_object* s);

mat4 static_object_world(static_object* s);
mat3 static_object_world_normal(static_object* s);

#endif
