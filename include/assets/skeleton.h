#ifndef skeleton_h
#define skeleton_h

#include "matrix.h"

struct bone {
  int id;
  char* name;
  
  vector3 position;
  matrix_4x4 rotation;
  
  struct bone* parent;
};

struct bone;
typedef struct bone bone;

bone* bone_new(int id, char* name);
void bone_delete(bone* b);
matrix_4x4 bone_transform(bone* b);

void inverse_kinematics_solve(bone* base, bone* end, vector3 target);

typedef struct {

  int num_bones;
  bone** bones;
  matrix_4x4* transforms;
  matrix_4x4* inv_transforms;
  
} skeleton;

skeleton* skeleton_new();
skeleton* skeleton_copy(skeleton* s);
void skeleton_delete(skeleton* s);

bone* skeleton_bone_id(skeleton* s, int id);
bone* skeleton_bone_name(skeleton* s, char* name);
void skeleton_add_bone(skeleton* s, char* name, int id, int parent_id);

void skeleton_gen_transforms(skeleton* s);
void skeleton_gen_inv_transforms(skeleton* s);

void skeleton_print(skeleton* s);

skeleton* skl_load_file(char* filename);

#endif
