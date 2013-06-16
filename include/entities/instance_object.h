/**
*** :: Instance Object ::
***
***   Much like a collection of static objects 
***   but supports instanced renderering.
***
**/

#ifndef instance_object_h
#define instance_object_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec3 position;
  vec3 scale;
  quat rotation;
  mat4 world;
  mat3 world_normal;
} instance_data;

typedef struct {

  int num_instances;
  instance_data* instances;
  
  GLuint world_buffer;
  
  sphere bound;
  asset_hndl renderable;
  asset_hndl collision_body;

} instance_object;

instance_object* instance_object_new();
void instance_object_delete(instance_object* io);

void instance_object_update(instance_object* io);
void instance_object_add_instance(instance_object* io, vec3 position, vec3 scale, quat rotation);
void instance_object_rem_instance(instance_object* io, int i);
mat4 instance_object_world(instance_object* io, int i);
mat3 instance_object_world_normal(instance_object* io, int i);

#endif
