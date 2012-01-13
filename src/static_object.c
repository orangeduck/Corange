#include "renderable.h"

#include "static_object.h"

static_object* static_object_new(renderable* r) {
  static_object* s = malloc(sizeof(static_object));
  
  s->position = v3_zero();
  s->rotation = v4_quaternion_id();
  s->scale = v3_one();
  
  s->active = true;
  s->recieve_shadows = true;
  s->cast_shadows = true;
  
  s->renderable = r;
  
  return s;
}

void static_object_delete(static_object* s) {
  free(s);
}