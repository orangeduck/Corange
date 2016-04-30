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
  vec3 norm;
  int flags;
} collision;

collision collision_none();
collision collision_new(float time, vec3 point, vec3 norm);
collision collision_merge(collision c0, collision c1);

collision point_collide_point(vec3 p, vec3 v, vec3 p0);
collision point_collide_sphere(vec3 p, vec3 v, sphere s);
collision point_collide_ellipsoid(vec3 p, vec3 v, ellipsoid e);
collision point_collide_edge(vec3 p, vec3 v, vec3 e0, vec3 e1);
collision point_collide_face(vec3 p, vec3 v, ctri ct);
collision point_collide_ctri(vec3 p, vec3 v, ctri ct);
collision point_collide_mesh(vec3 p, vec3 v, cmesh* m, mat4 world, mat3 world_normal);

collision sphere_collide_point(sphere s, vec3 v, vec3 p);
collision sphere_collide_sphere(sphere s, vec3 v, sphere s0);
collision sphere_collide_edge(sphere s, vec3 v, vec3 e0, vec3 e1);
collision sphere_collide_face(sphere s, vec3 v, ctri ct);
collision sphere_collide_ctri(sphere s, vec3 v, ctri ct);
collision sphere_collide_mesh(sphere s, vec3 v, cmesh* m, mat4 world, mat3 world_normal);

collision ellipsoid_collide_mesh(ellipsoid e, vec3 v, cmesh* m, mat4 world, mat3 world_normal);
collision ellipsoid_collide_point(ellipsoid e, vec3 v, vec3 p);
collision ellipsoid_collide_sphere(ellipsoid e, vec3 v, sphere s);

void collision_response_slide(void* x, vec3* position, vec3* velocity, collision (*colfunc)(void* x, vec3* pos, vec3* vel) );

#endif
