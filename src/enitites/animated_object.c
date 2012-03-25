#include <math.h>

#include "error.h"

#include "entities/animated_object.h"

animated_object* animated_object_new() {

  animated_object* ao = malloc(sizeof(animated_object));
  ao->position = v3_zero();
  ao->scale = v3_one();
  ao->rotation = v4_quaternion_id();
  
  ao->active = true;
  ao->recieve_shadows = true;
  ao->cast_shadows = true;
  
  ao->renderable = NULL;
  ao->skeleton = NULL;
  
  ao->animation = NULL;
  ao->animation_time = 0;
  
  ao->pose = NULL;
  
  return ao;
}

void animated_object_delete(animated_object* ao) {
  skeleton_delete(ao->pose);
  free(ao);
}

void animated_object_load_skeleton(animated_object* ao, skeleton* s) {
  if(ao->pose != NULL) {
    skeleton_delete(ao->pose);
  }
  ao->skeleton = s;
  ao->pose = skeleton_copy(s);
}

void animated_object_update(animated_object* ao, float timestep) {
  
  ao->animation_time += timestep;
  animation* animation = ao->animation;
  
  if (animation == NULL) {
    return;
  }
  if (ao->pose == NULL || ao->skeleton == NULL) {
    error("Animated object needs skeleton loaded with animated_object_load_skeleton.");
  }
  
  float time_diff = animation->end_time - animation->start_time;
  float timepoint = animation->start_time + fmod(ao->animation_time, time_diff);
  
  skeleton* frame0 = animation->frames[0];
  skeleton* frame1 = animation->frames[animation->num_frames-1];
  float frame0_time = 0;
  float frame1_time = 999999;
  
  for(int i = 0; i < animation->num_frames; i++) {
    if ((timepoint > animation->frame_times[i]) && (frame0_time < animation->frame_times[i])) {
      frame0 = animation->frames[i];
      frame0_time = animation->frame_times[i];
    }
    
    if ((timepoint < animation->frame_times[i]) && (frame1_time > animation->frame_times[i])) {
      frame1 = animation->frames[i];
      frame1_time = animation->frame_times[i];
    }
  }
  
  float amount = (timepoint - frame0_time) / (frame1_time - frame0_time);
  amount = 1-amount;
  
  if (ao->skeleton->num_bones != frame0->num_bones) { error("Animation has a different number of bones to skeleton"); }
  if (ao->skeleton->num_bones != frame1->num_bones) { error("Animation has a different number of bones to skeleton"); }
  
  for(int i = 0; i < ao->skeleton->num_bones; i++) {
    vector3 position = v3_smoothstep(frame0->bones[i]->position, frame1->bones[i]->position, amount);
    matrix_4x4 rotation = m44_smoothstep(frame0->bones[i]->rotation, frame1->bones[i]->rotation, amount);
    
    ao->pose->bones[i]->position = position;
    ao->pose->bones[i]->rotation = rotation;
  }
  
}
