#ifndef animated_object_h
#define animated_object_h

#include <stdbool.h>

#include "matrix.h"

#include "assets/renderable.h"
#include "assets/skeleton.h"
#include "assets/animation.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  renderable* renderable;
  skeleton* skeleton;
  
  float animation_time;
  animation* animation;
  
  skeleton* pose;
  
} animated_object;

animated_object* animated_object_new();
void animated_object_delete(animated_object* ao);

void animated_object_load_skeleton(animated_object* ao, skeleton* s);

void animated_object_update(animated_object* ao, float timestep);

#endif
