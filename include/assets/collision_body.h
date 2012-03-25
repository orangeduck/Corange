#ifndef collision_body_h
#define collision_body_h

#include "geometry.h"
#include "vector.h"
#include "matrix.h"
#include "bool.h"

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

sphere collision_mesh_sphere(collision_mesh* cm);
box collision_mesh_box(collision_mesh* cm);

static const int collision_type_sphere = 0;
static const int collision_type_box = 1;
static const int collision_type_mesh = 2;

typedef struct {
  
  int collision_type;
  
  sphere collision_sphere;
  box collision_box;
  collision_mesh* collision_mesh;
  
} collision_body;

collision_body* collision_body_new_sphere(sphere bs);
collision_body* collision_body_new_box(box bb);
collision_body* collision_body_new_mesh(collision_mesh* cm);

void collision_body_delete(collision_body* cb);


typedef struct {
  bool collided;
  float time;
  vector3 object_position;
  vector3 surface_position;
  vector3 surface_normal;
} collision;

void sphere_collide_sphere(collision* out, sphere object, vector3 object_velocity, sphere target, float timestep);
void sphere_collide_box(collision* out, sphere object, vector3 object_velocity, box target, float timestep);
void sphere_collide_mesh(collision* out, sphere object, vector3 object_velocity, collision_mesh* target, matrix_4x4 target_world, float timestep);

#endif