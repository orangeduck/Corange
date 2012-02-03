#ifndef collision_body_h
#define collision_body_h

#include "vector.h"
#include "matrix.h"
#include "bool.h"

typedef struct {
  vector3 direction;
  vector3 position;
} plane;

plane plane_new(vector3 position, vector3 direction);
float plane_distance(plane p, vector3 point);
plane plane_transform(plane p, matrix_4x4 world);

bool point_behind_plane(vector3 point, plane plane);

typedef struct {
  plane top;
  plane bottom;
  plane left;
  plane right;
  plane front;
  plane back;
} bounding_box;

bounding_box bounding_box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
bounding_box bounding_box_sphere(vector3 center, float radius);
bounding_box bounding_box_merge(bounding_box b1, bounding_box b2);
bounding_box bounding_box_transform(bounding_box b1, matrix_4x4 world);

bool bounding_box_contains(bounding_box b1, vector3 point);

typedef struct {
  vector3 center;
  float radius;
  float radius_sqrd;
} bounding_sphere;

bounding_sphere bounding_sphere_new(vector3 center, float radius);
bounding_sphere bounding_sphere_of_box(bounding_box bb);
bounding_sphere bounding_sphere_merge(bounding_sphere bs1, bounding_sphere bs2);
bounding_sphere bounding_sphere_transform(bounding_sphere bs, matrix_4x4 world);

bool bounding_sphere_contains(bounding_sphere bs1, vector3 point);

struct collision_mesh {
  
  plane division;
  struct collision_mesh* front;
  struct collision_mesh* back;

  bool is_leaf;
  
  vector3* verticies;
  int num_verticies;
  
  vector3* triangle_normals;
};

struct collision_mesh;
typedef struct collision_mesh collision_mesh;

collision_mesh* col_load_file(char* filename);
void collision_mesh_delete(collision_mesh* cm);
void collision_mesh_subdivide(collision_mesh* cm, int iterations);

bounding_sphere collision_mesh_bounding_sphere(collision_mesh* cm);
bounding_box collision_mesh_bounding_box(collision_mesh* cm);


static const int collision_type_sphere = 0;
static const int collision_type_box = 1;
static const int collision_type_mesh = 2;

typedef struct {
  
  int collision_type;
  
  bounding_sphere collision_sphere;
  bounding_box collision_box;
  collision_mesh* collision_mesh;
  
} collision_body;

collision_body* collision_body_new_sphere(bounding_sphere bs);
collision_body* collision_body_new_box(bounding_box bb);
collision_body* collision_body_new_mesh(collision_mesh* cm);

void collision_body_delete(collision_body* cb);


typedef struct {
  bool collided;
  float time;
  vector3 object_position;
  vector3 surface_position;
  vector3 surface_normal;
} collision;

void sphere_collide_sphere(collision* out, bounding_sphere object, vector3 object_velocity, bounding_sphere target, float timestep);

void sphere_collide_mesh(collision* out, bounding_sphere object, vector3 object_velocity, collision_mesh* target, matrix_4x4 target_world, float timestep);

#endif