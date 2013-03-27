#ifndef cphysics_h
#define cphysics_h

#include "cengine.h"
#include "assets/cmesh.h"

vec3 vec3_gravity();

bool quadratic(float a, float b, float c, float* t0, float* t1);

typedef struct {
  bool collided;
  float time;
  vec3 point;
} collision;

collision collision_none();
collision collision_new(float time, vec3 point);
collision collision_merge(collision c0, collision c1);

collision sphere_collide_face(sphere s, vec3 v, ctri ct);
collision sphere_collide_edge(sphere s, vec3 v, vec3 e0, vec3 e1);
collision sphere_collide_point(sphere s, vec3 v, vec3 p);
collision sphere_collide_sphere(sphere s, vec3 v, sphere s0);
collision sphere_collide_ctri(sphere s, vec3 v, ctri ct);

collision sphere_collide_mesh(sphere s, vec3 v, cmesh* m, mat4 mworld);
collision ellipsoid_collide_mesh(ellipsoid e, vec3 v, cmesh* m, mat4 mworld);

//collision capsule_collide_sphere(capsule c, vec3 v, sphere s, float timestep);
//collision capsule_collide_mesh(capsule c, vec3 v, cmesh* m, mat4 mworld, float timestep);
//collision capsule_collide_landscape(capsule c, vec3 v, landscape* l, float timestep);


#endif
