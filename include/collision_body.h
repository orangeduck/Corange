#ifndef collision_body_h
#define collision_body_h

#include "vector.h"
#include "matrix.h"
#include "bool.h"

/*
  General idea

  Copy all data into a bsp_mesh then subdivide into two parts.
  To subdivide find bounding box and then split optimally.
  
  Filter in triangles which are one side of the plane.
  
*/

typedef struct {
  vector3 direction;
  vector3 position;
} plane;

bool point_behind_plane(vector3 point, plane plane);

typedef struct {
  float x_min;
  float x_max;
  float y_min;
  float y_max;
  float z_min;
  float z_max;
} bounding_box;

bounding_box bounding_box_merge(bounding_box b1, bounding_box b2);
bool bounding_box_intersects(bounding_box b1, bounding_box b2);
bool bounding_box_contains(bounding_box b1, vector3 point);

plane bounding_box_division(bounding_box bb);

struct bsp_mesh {
  
  plane division;
  
  struct bsp_mesh* front;
  struct bsp_mesh* back;

  bool is_leaf;
  vector3* verticies;
  int num_verticies;
  
  int* triangles;
  int num_triangles;
  
};

struct bsp_mesh;
typedef struct bsp_mesh bsp_mesh;

void bsp_mesh_delete(bsp_mesh* bm);
float bsp_mesh_bounding_distance(bsp_mesh* bm);
bounding_box bsp_mesh_bounding_box(bsp_mesh* bm);
void bsp_mesh_subdivide(bsp_mesh* bm, int iterations);

vector3 bsp_mesh_ground_point(bsp_mesh* bm, vector3 point);

typedef struct {
  
  float bounding_distance;
  bounding_box bounding_box;
  
  bsp_mesh* collision_mesh;

} collision_body;

collision_body* col_load_file(char* filename);
void collision_body_delete(collision_body* cb);

vector3 collision_body_ground_point(collision_body* cb, matrix_4x4 world_matrix, vector3 point);

#endif