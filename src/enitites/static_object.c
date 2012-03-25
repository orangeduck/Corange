#include "entities/static_object.h"

static_object* static_object_new() {
  static_object* s = malloc(sizeof(static_object));
  
  s->position = v3_zero();
  s->rotation = v4_quaternion_id();
  s->scale = v3_one();
  
  s->active = true;
  s->recieve_shadows = true;
  s->cast_shadows = true;
  
  s->renderable = NULL;
  s->collision_body = NULL;
  
  return s;
}

void static_object_delete(static_object* s) {
  free(s);
}
