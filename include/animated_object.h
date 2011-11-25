#ifndef animated_object_h
#define animated_object_h

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
  
  int num_animations;
  animation** animations;
  int curr_animation;
  float animation_time;
  
} animated_object;

animated_object* animated_object_new(renderable* r, skeleton* s);
void animated_object_delete(animated_object* ao);

void animated_object_add_animation(animated_object* ao, animation* a);

void animated_object_bone_matrices(animated_object* ao, matrix_4x4* matricies);

#endif