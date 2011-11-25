#include "animated_object.h"

#include "matrix.h"

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
  
  ao->num_animations = 0;
  ao->animations = malloc(sizeof(animation*) * 0);
  ao->curr_animation = -1;
  ao->animation_time = 0;
  
  return ao;
}

void animated_object_delete(animated_object* ao) {
  free(ao);
}

void animated_object_add_animation(animated_object* ao, animation* a) {
  ao->num_animations++;
  ao->animations = realloc(ao->animations, sizeof(animation*) * ao->num_animations);
  ao->animations[ao->num_animations-1] = a;
}

void animated_object_bone_matrices(animated_object* ao, matrix_4x4* matricies) {
  
  if (ao->curr_animation = -1) {
    /* TODO: probably worth just getting the world matricies from the skeleton or something */
    error("No animation set");
  }
  
  animation* a = ao->animations[curr_animation];
  float pos = ao->animation_time * a->speed;
  float range = a->end - a->start;
  float time = a->start + fmod(pos, (float)a->end);
  
  int prev = floor(time);
  int next = floor(time);
  float diff = fmod(time, 1.0);
  
  int i;
  for(i = 0; i < ao->skeleton->num_bones; i++) {
    
    vector3 prev_pos = a->frames[prev]->bone_frames[i].position;
    vector4 prev_rot = a->frames[prev]->bone_frames[i].rotation;
    
    vector3 next_pos = a->frames[next]->bone_frames[i].position;
    vector4 next_rot = a->frames[next]->bone_frames[i].rotation;
    
    vector3 final_position = v3_lerp(prev_pos, next_pos, diff);
    vector3 final_rotation = v3_lerp(prev_rot, next_rot, diff);
    
    vector3 scale = v3_one();
    
    matricies[i] = m44_world( final_position, scale, final_rotation );
  }
  
  /* This isn't chaining them properly */
  
}