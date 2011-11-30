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

typedef struct {
  int num_bones;
  bone** bones;
  
} skeleton;

skeleton* skeleton_new();
skeleton* skeleton_copy(skeleton* s);
void skeleton_delete(skeleton* s);

bone* skeleton_bone_id(skeleton* s, int id);
void skeleton_add_bone(skeleton* s, char* name, int id, int parent_id);

void skeleton_print(skeleton* s);

skeleton* skl_load_file(char* filename);

#endif