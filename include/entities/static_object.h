#ifndef static_object_h
#define static_object_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  vec3 position;
  vec3 scale;
  vec4 rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  asset_hndl renderable;
  asset_hndl collision_body;
} static_object;

static_object* static_object_new();
void static_object_delete(static_object* s);

#endif
