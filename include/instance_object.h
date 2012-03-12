#ifndef instance_object_h
#define instance_object_h

#include "renderable.h"
#include "collision_body.h"

#define MAX_INSTANCES 128

typedef struct {

  vector3 position;
  vector3 scale;
  vector4 rotation;
  
} instance_data;

typedef struct {

  int num_instances;
  instance_data* instances;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;
  
  renderable* renderable;
  collision_body* collision_body;

} instance_object;

instance_object* instance_object_new();
void instance_object_delete(instance_object* io);

void instance_object_add_instance(instance_object* io, vector3 position, vector3 scale, vector4 rotation);

#endif