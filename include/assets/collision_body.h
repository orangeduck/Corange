/**
*** :: Collision Body ::
***
***   - WIP -
***   
***   Represents collision body in system.
***   Can be box, sphere or BSP-tree based mesh.
***
**/

#ifndef collision_body_h
#define collision_body_h

#include "cengine.h"

/*
**  ~ Collision Mesh ~
**
**  Uses BSP-tree for accelerated collision detection.
**  Still very slow at the moment...
**  Trianges across divisions are duplicated on both sides.
**  So non-leaf nodes have no verts
*/

struct collision_mesh {
  
  plane division;
  struct collision_mesh* front;
  struct collision_mesh* back;

  bool is_leaf;
  
  vec3* verticies;
  int num_verticies;
  
  vec3* triangle_normals;
};

struct collision_mesh;
typedef struct collision_mesh collision_mesh;

collision_mesh* col_load_file(char* filename);
void collision_mesh_delete(collision_mesh* cm);

/* Create binary division and move triangles to either side */
void collision_mesh_subdivide(collision_mesh* cm, int iterations);

/* Create sphere and box which contain collision_mesh */
sphere collision_mesh_sphere(collision_mesh* cm);
box collision_mesh_box(collision_mesh* cm);

/* Collision Body */
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

/*
**  ~ Collision Struct ~
**
**  Contains collision data.
**  Other systems can then decide how to act.
*/

typedef struct {
  bool collided;
  float time;
  vec3 object_position;
  vec3 surface_position;
  vec3 surface_normal;
} collision;

void sphere_collide_sphere(collision* out, sphere object, vec3 object_velocity, sphere target, float timestep);
void sphere_collide_box(collision* out, sphere object, vec3 object_velocity, box target, float timestep);
void sphere_collide_mesh(collision* out, sphere object, vec3 object_velocity, collision_mesh* target, mat4 target_world, float timestep);

#endif