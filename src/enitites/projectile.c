#include "entities/projectile.h"

#include "centity.h"
#include "cphysics.h"

#include "entities/landscape.h"
#include "entities/static_object.h"


projectile* projectile_new() {
  
  projectile* p = malloc(sizeof(projectile));
  p->collided = true;
  p->position = vec3_zero();
  p->velocity = vec3_zero();
  p->rotation = mat4_id();
  p->radius = 1;
  p->mesh = asset_hndl_null();
  
  return p;
  
}

void projectile_delete(projectile* p) {
  free(p);
}

void projectile_type(projectile* p, asset_hndl mesh, float radius) {
  
  p->mesh = mesh;
  p->radius = radius;

}

void projectile_fire(projectile* p, vec3 position, vec3 velocity) {
  
  p->collided = false;
  p->position = position;
  p->velocity = velocity;

}

static void projectile_collide(projectile* p, float timestep) {
  
  collision col = collision_none();
  
  landscape* l = entity_get("map_land");
  sphere bound = sphere_new(p->position, p->radius);
  
  vec3 velocity = vec3_mul(p->velocity, timestep);
  
  terrain_chunk* chunks[9];
  landscape_chunks(l, vec2_new(bound.center.x, bound.center.z), chunks);
  for (int i = 0; i < 9; i++) {
    col = collision_merge(col, sphere_collide_mesh(
      bound, velocity, 
      chunks[i]->colmesh,
      landscape_world(l)));
  }
  
  int entities_num;
  static_object* entities[512];
  entities_get(entities, &entities_num, static_object);
  for (int i = 0; i < entities_num; i++) {
    col = collision_merge(col, sphere_collide_mesh(
      bound, velocity,
      asset_hndl_ptr(entities[i]->collision_body),
      static_object_world(entities[i])));
  }
  
  if (col.collided) {
    p->collided = true;
    p->position = vec3_add(p->position, vec3_mul(velocity, col.time));
  } else {
    p->position = vec3_add(p->position, velocity);
  }
  
  vec3 zaxis = vec3_normalize(velocity);
  vec3 xaxis = vec3_cross(zaxis, vec3_up());
  vec3 yaxis = vec3_cross(zaxis, xaxis);
  
  p->rotation = mat4_new( 
    xaxis.x, xaxis.y, xaxis.z, 0,
    yaxis.x, yaxis.y, yaxis.z, 0,
    zaxis.x, zaxis.y, zaxis.z, 0,
    0, 0, 0, 1);

}

void projectile_update(projectile* p, float timestep) {
  
  if (!p->collided) {
    p->velocity = vec3_add(p->velocity, vec3_mul(vec3_gravity(), timestep * 0.5));
    projectile_collide(p, timestep);
  }
  
}