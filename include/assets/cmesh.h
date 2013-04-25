/**
*** :: cmesh ::
***
***
**/

#ifndef cmesh_h
#define cmesh_h

#include "cengine.h"

typedef struct {
  vec3 a, b, c;
  vec3 norm;
  sphere bound;
} ctri;

ctri ctri_new(vec3 a, vec3 b, vec3 c, vec3 norm);
ctri ctri_transform(ctri t, mat4 m);
ctri ctri_transform_space(ctri t, mat3 s, mat3 is);
bool ctri_inside_plane(ctri t, plane p);
bool ctri_outside_plane(ctri t, plane p);
bool ctri_intersects_plane(ctri t, plane p);

typedef struct cmesh {
  
  bool is_leaf;
  
  union {
    struct {
      plane division;
      struct cmesh* front;
      struct cmesh* back;
    };
    
    struct {
      ctri* triangles;
      int triangles_num;
      sphere bound;
    };
  };
  
} cmesh;

cmesh* col_load_file(char* filename);
void cmesh_delete(cmesh* cm);

sphere cmesh_bound(cmesh* cm);

#endif