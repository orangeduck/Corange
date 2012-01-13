#ifndef animated_object_h
#define animated_object_h

#include "bool.h"
#include "matrix.h"
#include "renderable.h"
#include "skeleton.h"
#include "animation.h"

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

animated_object* animated_object_new(renderable* r, skeleton* s);
void animated_object_delete(animated_object* ao);

void animated_object_update(animated_object* ao, float timestep);

#endif