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
  ao->loop = true;
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
  ao->pose = frame_copy(((skeleton*)asset_hndl_ptr(ao->skeleton))->rest_pose);
}

void animated_object_update(animated_object* ao, float timestep) {
  
  animation* a = asset_hndl_ptr(ao->animation);
  if (a == NULL) { return; }
  
  if (ao->loop) {
    float maxtime = a->frame_times[a->frame_count-1];
    ao->animation_time = fmod(ao->animation_time + timestep, maxtime);
  } else {
    ao->animation_time = ao->animation_time + timestep;
  }  
  
  if (ao->pose == NULL) {
    error("Animated object needs skeleton loaded with 'animated_object_load_skeleton'.");
  }
  
  frame* frame0 = a->frames[0];
  frame* frame1 = a->frames[a->frame_count-1];
  
  float frame0_time = 0;
  float frame1_time = FLT_MAX;
  
  for(int i = 0; i < a->frame_count; i++) {
    if ((ao->animation_time > a->frame_times[i]) && (frame0_time < a->frame_times[i])) {
      frame0 = a->frames[i];
      frame0_time = a->frame_times[i];
    }
    
    if ((ao->animation_time < a->frame_times[i]) && (frame1_time > a->frame_times[i])) {
      frame1 = a->frames[i];
      frame1_time = a->frame_times[i];
    }
  }
  
  float amount = (ao->animation_time - frame0_time) / (frame1_time - frame0_time);
  
  for(int i = 0; i < ao->pose->joint_count; i++) {
    vec3 position = vec3_lerp(frame0->joint_positions[i], frame1->joint_positions[i], amount);
    quat rotation = quat_slerp(frame0->joint_rotations[i], frame1->joint_rotations[i], amount);
    ao->pose->joint_positions[i] = position;
    ao->pose->joint_rotations[i] = rotation;
  }
  
}
