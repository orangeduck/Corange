/**
*** :: Skeleton ::
***
***
**/

#ifndef skeleton_h
#define skeleton_h

#include "cengine.h"

typedef struct {
  int joint_count;
  int* joint_parents;
  vec3* joint_positions;
  quat* joint_rotations;
  mat4* transforms;
  mat4* transforms_inv;
} frame;

frame* frame_new();
frame* frame_copy(frame* f);
frame* frame_interpolate(frame* f0, frame* f1, float amount);
void frame_interpolate_to(frame* f0, frame* f1, float amount, frame* out);
void frame_decendants_to(frame* f0, frame* f1, float amount, int joint, frame* out);
void frame_delete(frame* f);

mat4 frame_joint_transform(frame* f, int i);
void frame_joint_add(frame* f, int parent, vec3 position, quat rotation);

void frame_gen_transforms(frame* f);
void frame_gen_inv_transforms(frame* f);

typedef struct {
  int joint_count;
  char** joint_names;
  frame* rest_pose;
} skeleton;

skeleton* skeleton_new();
void skeleton_delete(skeleton* s);
void skeleton_joint_add(skeleton* s, char* name, int parent);
int skeleton_joint_id(skeleton* s, char* name);

skeleton* skl_load_file(char* filename);

#endif
