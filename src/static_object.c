#include "renderable.h"

#include "static_object.h"

static_object* static_object_new(renderable* r) {
  static_object* s = malloc(sizeof(static_object));
  
  s->position = v3_zero();
  s->rotation = v4_quaternion_id();
  s->scale = v3_one();
  
  s->active = 1;
  s->recieve_shadows = 1;
  s->cast_shadows = 1;
  
  s->renderable = r;
  
  return s;
}

void static_object_delete(static_object* s) {
  free(s);
}