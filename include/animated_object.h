#ifndef animated_object_h
#define animated_object_h

#include "matrix.h"
#include "renderable.h"
#include "skeleton.h"
#include "animation.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  int active;
  int recieve_shadows;
  int cast_shadows;

  renderable* renderable;
  skeleton* skeleton;
  
  float animation_time;
  animation* animation;
  
} animated_object;

animated_object* animated_object_new(renderable* r, skeleton* s);
void animated_object_delete(animated_object* ao);

#endif