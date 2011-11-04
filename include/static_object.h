#ifndef static_object_h
#define static_object_h

#include "renderable.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  int active;
  int recieve_shadows;
  int cast_shadows;

  renderable* renderable;

} static_object;

static_object* static_object_new(renderable* r);
void static_object_delete(static_object* s);

#endif