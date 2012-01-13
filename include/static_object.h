#ifndef static_object_h
#define static_object_h

#include "bool.h"
#include "renderable.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  renderable* renderable;

} static_object;

static_object* static_object_new(renderable* r);
void static_object_delete(static_object* s);

#endif