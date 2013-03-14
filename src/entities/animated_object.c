#include "entities/animated_object.h"

#include "assets/animation.h"

animated_object* animated_object_new() {

  animated_object* ao = malloc(sizeof(animated_object));
  ao->position = vec3_zero();
  ao->scale = vec3_one();
  ao->rotation = quat_id();
  
  ao->renderable = asset_hndl_null();
  ao->skeleton = asset_hndl_null();
  ao->animation = asset_hndl_null();
  ao->animation_time = 0;
  
  ao->pose = NULL;
  
  return ao;
}

void animated_object_delete(animated_object* ao) {
  if (ao->pose) { frame_delete(ao->pose); }
  free(ao);
}

void animated_object_load_skeleton(animated_object* ao, asset_hndl ah) {
  if(ao->pose != NULL) { frame_delete(ao->pose); }
  ao->skeleton = ah;
  ao->pose = frame_copy(((skeleton*)asset_hndl_ptr(&ao->skeleton))->rest_pose);
}

void animated_object_update(animated_object* ao, float timestep) {
  
  animation* a = asset_hndl_ptr(&ao->animation);
  if (a == NULL) { return; }
  
  if (ao->pose != NULL) {
    frame_delete(ao->pose);
    ao->pose = NULL;
  }
  
  ao->pose = animation_sample(a, ao->animation_time);
  
}
