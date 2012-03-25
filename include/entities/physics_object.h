#ifndef physics_object_h
#define physics_object_h

#include "assets/renderable.h"
#include "assets/collision_body.h"

#include "entities/static_object.h"

typedef struct {
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  vector3 velocity;
  vector4 angular_velocity;
  
  vector3 acceleration;
  vector4 angular_acceleration;
  
  vector3 previous_position;
  
  float elasticity;
  float friction;
  
  bool active;
  bool recieve_shadows;
  bool cast_shadows;

  renderable* renderable;
  collision_body* collision_body;

} physics_object;

physics_object* physics_object_new();
void physics_object_delete(physics_object* po);


void physics_object_collide_static(physics_object* po, static_object* so, float timestep);
void physics_object_update(physics_object* po, float timestep);

#endif
