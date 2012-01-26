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

typedef struct {
  vector3 center;
  float radius;
  float radius_sqrd;
} bounding_sphere;

bounding_sphere bounding_sphere_merge(bounding_sphere bs1, bounding_sphere bs2);
bool bounding_sphere_intersects(bounding_sphere bs1, bounding_sphere bs2);
bool bounding_sphere_contains(bounding_sphere bs1, vector3 point);

struct bsp_mesh {
  
  plane division;
  
  struct bsp_mesh* front;
  struct bsp_mesh* back;

  bool is_leaf;
  vector3* verticies;
  int num_verticies;
  
  vector3* triangle_normals;
};

struct bsp_mesh;
typedef struct bsp_mesh bsp_mesh;


void bsp_mesh_delete(bsp_mesh* bm);
void bsp_mesh_subdivide(bsp_mesh* bm, int iterations);

bounding_sphere bsp_mesh_bounding_sphere(bsp_mesh* bm);
bounding_box bsp_mesh_bounding_box(bsp_mesh* bm);


static const int collision_type_sphere = 0;
static const int collision_type_box = 1;
static const int collision_type_mesh = 2;

typedef struct {
  
  int collision_type;
  
  bounding_sphere collision_sphere;
  bounding_box collision_box;
  bsp_mesh* collision_mesh;

} collision_body;

collision_body* collision_body_new_sphere(bounding_sphere bs);
collision_body* collision_body_new_box(bounding_box bb);

collision_body* col_load_file(char* filename);
void collision_body_delete(collision_body* cb);

#endif