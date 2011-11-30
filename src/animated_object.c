#include <math.h>

#include "error.h"

#include "animated_object.h"

animated_object* animated_object_new(renderable* r, skeleton* s) {

  animated_object* ao = malloc(sizeof(animated_object));
  ao->position = v3_zero();
  ao->scale = v3_one();
  ao->rotation = v4_quaternion_id();
  
  ao->active = 1;
  ao->recieve_shadows = 1;
  ao->cast_shadows = 1;
  
  ao->renderable = r;
  ao->skeleton = s;
  
  ao->animation = NULL;
  ao->animation_time = 0;
  
  return ao;
}

void animated_object_delete(animated_object* ao) {
  free(ao);
}