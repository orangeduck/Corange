#ifndef static_object_h
#define static_object_h

#include "bool.h"

#include "assets/renderable.h"
#include "assets/collision_body.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  renderable* renderable;
  collision_body* collision_body;

} static_object;

static_object* static_object_new();
void static_object_delete(static_object* s);

#endif
