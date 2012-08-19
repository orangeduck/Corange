#ifndef instance_object_h
#define instance_object_h

#include "cengine.h"
#include "casset.h"

#define MAX_INSTANCES 128

typedef struct {
  vec3 position;
  vec3 scale;
  vec4 rotation;
} instance_data;

typedef struct {

  int num_instances;
  instance_data* instances;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;
  
  asset_hndl renderable;
  asset_hndl collision_body;

} instance_object;

instance_object* instance_object_new();
void instance_object_delete(instance_object* io);

void instance_object_add_instance(instance_object* io, vec3 position, vec3 scale, vec4 rotation);

#endif
