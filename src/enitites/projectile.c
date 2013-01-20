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

static void projectile_collide(projectile* p) {
  
  collision col = collision_none();
  
  landscape* l = entity_get("map_land");
  sphere bound = sphere_new(p->position, p->radius);
  
  terrain_chunk* chunks[9];
  landscape_chunks(l, vec2_new(bound.center.x, bound.center.z), chunks);
  for (int i = 0; i < 9; i++) {
    col = collision_merge(col, sphere_collide_mesh(
      bound, p->velocity, 
      chunks[i]->colmesh,
      landscape_world(l)));
  }
  
  int entities_num;
  static_object* entities[512];
  entities_get(entities, &entities_num, static_object);
  for (int i = 0; i < entities_num; i++) {
    col = collision_merge(col, sphere_collide_mesh(
      bound, p->velocity,
      asset_hndl_ptr(entities[i]->collision_body),
      static_object_world(entities[i])));
  }
  
  if (col.collided) {
    p->collided = true;
    p->position = vec3_add(p->position, vec3_mul(p->velocity, col.time));
  } else {
    p->position = vec3_add(p->position, p->velocity);
  }

}

void projectile_update(projectile* p, float timestep) {
  
  if (!p->collided) {
    p->velocity = vec3_add(p->velocity, vec3_mul(vec3_gravity(), timestep));
    projectile_collide(p);
  }
  
}