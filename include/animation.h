#ifndef animation_h
#define animation_h

#include "skeleton.h"

typedef struct {
  int bone_id;
  vector3 position;
  vector4 rotation;
} bone_frame;

typedef struct {
  float time;
  int num_bone_frames;
  bone_frame* bone_frames;
} frame;

frame* frame_new(float time);
void frame_delete(frame* f);

void frame_add_bone_frame(frame* f, int id, vector3 pos, vector4 rot);

typedef struct {

  float speed;
  
  int start, end;
  int num_frames;
  frame** frames;

} animation;

animation* animation_new();
animation* animation_identity(skeleton* s);
void animation_delete(animation* a);

frame* animation_new_frame(animation* a, float time);

animation* ani_load_file(char* filename);

#endif