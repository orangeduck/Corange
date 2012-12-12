/**
*** :: Physics Object ::
***
***   -- WIP --
***
***   An object which also contains
***   physics and movement data such
***   as velocity and acceleration
***
***   Some WIP functions to collide such
***   objects with static objects.
***
**/

#ifndef physics_object_h
#define physics_object_h

#include "cengine.h"
#include "entities/static_object.h"

typedef struct {
  
  vec3 position;
  vec3 scale;
  mat4 rotation;
  
  vec3 velocity;
  mat4 angular_velocity;
  
  vec3 acceleration;
  mat4 angular_acceleration;
  
  vec3 previous_position;
  
  float elasticity;
  float friction;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  asset_hndl renderable;
  asset_hndl collision_body;

} physics_object;

physics_object* physics_object_new();
void physics_object_delete(physics_object* po);

void physics_object_collide_static(physics_object* po, static_object* so, float timestep);
void physics_object_update(physics_object* po, float timestep);

#endif
