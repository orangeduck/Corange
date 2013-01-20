#ifndef projectile_h
#define projectile_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  bool collided;
  vec3 position;
  vec3 velocity;
  float radius;
  asset_hndl mesh;
} projectile;

projectile* projectile_new();
void projectile_delete(projectile* p);

void projectile_type(projectile* p, asset_hndl mesh, float radius);
void projectile_fire(projectile* p, vec3 position, vec3 velocity);
void projectile_update(projectile* p, float timestep);

#endif