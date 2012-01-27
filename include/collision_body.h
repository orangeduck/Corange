#ifndef collision_body_h
#define collision_body_h

#include "vector.h"
#include "matrix.h"
#include "bool.h"

typedef struct {
  vector3 direction;
  vector3 position;
} plane;

bool point_behind_plane(vector3 point, plane plane);
plane plane_transform(plane p, matrix_4x4 world);


typedef struct {
  float x_min;
  float x_max;
  float y_min;
  float y_max;
  float z_min;
  float z_max;
} bounding_box;

bool bounding_box_contains(bounding_box b1, vector3 point);

bounding_box bounding_box_sphere(vector3 center, float radius);
bounding_box bounding_box_merge(bounding_box b1, bounding_box b2);
bounding_box bounding_box_transform(bounding_box b1, matrix_4x4 world);

bool bouding_box_behind_plane(vector3 point, plane plane);

typedef struct {
  vector3 center;
  float radius;
  float radius_sqrd;
} bounding_sphere;

bounding_sphere bounding_sphere_of_box(bounding_box bb);
bounding_sphere bounding_sphere_merge(bounding_sphere bs1, bounding_sphere bs2);
bounding_sphere bounding_sphere_transform(bounding_sphere bs, matrix_4x4 world);

bool bounding_sphere_contains(bounding_sphere bs1, vector3 point);

struct bsp_mesh {
  
  plane division;
  struct bsp_mesh* front;
  struct bsp_mesh* back;

  bool is_leaf;
  
  matrix_4x4 world_matrix;
  
  vector3* verticies;
  int num_verticies;
  
  vector3* triangle_normals;
};

struct bsp_mesh;
typedef struct bsp_mesh bsp_mesh;

void bsp_mesh_delete(bsp_mesh* bm);
void bsp_mesh_subdivide(bsp_mesh* bm, int iterations);
void bsp_mesh_set_world_matrix(bsp_mesh* bm, matrix_4x4 world);

bounding_sphere bsp_mesh_bounding_sphere(bsp_mesh* bm);
bounding_box bsp_mesh_bounding_box(bsp_mesh* bm);


static const int collision_type_sphere = 0;
static const int collision_type_box = 1;
static const int collision_type_mesh = 2;

typedef struct {
  
  int collision_type;
  
  matrix_4x4 world_matrix;
  
  bounding_sphere collision_sphere;
  bounding_box collision_box;
  bsp_mesh* collision_mesh;

} collision_body;

collision_body* collision_body_new_sphere(vector3 center, float radius);
collision_body* collision_body_new_box(bounding_box bb);

collision_body* col_load_file(char* filename);
void collision_body_delete(collision_body* cb);

void collision_body_set_world_matrix(collision_body* cb, matrix_4x4 world);

#define MAX_COLLISIONS 5
typedef struct {
  int collisions;
  vector3 position[MAX_COLLISIONS];
  vector3 normal[MAX_COLLISIONS];
} collision_info;

collision_info collision_info_merge(collision_info ci1, collision_info ci2);

collision_info sphere_collide_sphere(bounding_sphere bs1, bounding_sphere bs2);
collision_info sphere_collide_box(bounding_sphere bs, bounding_box bb);
collision_info sphere_collide_mesh(bounding_sphere bs, bsp_mesh* bm);

collision_info box_collide_sphere(bounding_box bb, bounding_sphere bs);
collision_info box_collide_box(bounding_box bb1, bounding_box bb2);
collision_info box_collide_mesh(bounding_box bb, bsp_mesh* bm);

collision_info mesh_collide_sphere(bsp_mesh* bm, bounding_sphere bs);
collision_info mesh_collide_box(bsp_mesh* bm, bounding_box bb);
collision_info mesh_collide_mesh(bsp_mesh* bm1, bsp_mesh* bm2);

collision_info collision_bodies_collide(collision_body* cb1, collision_body* cb2);

#endif