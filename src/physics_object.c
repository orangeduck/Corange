#include "error.h"

#include "physics_object.h"

physics_object* physics_object_new() {

  physics_object* po = malloc(sizeof(physics_object));
  
  po->position = v3_zero();
  po->rotation = v4_quaternion_id();
  po->scale = v3_one();
  
  po->velocity = v3_zero();
  po->angular_velocity = v4_quaternion_id();
  
  po->acceleration = v3(0, -9.81, 0);
  po->angular_acceleration = v4_quaternion_id();
  
  po->previous_position = v3_zero();
  
  po->elasticity = 0.1;
  po->friction = 0.25;
  
  po->active = true;
  po->recieve_shadows = true;
  po->cast_shadows = true;
  
  po->renderable = NULL;
  po->collision_body = NULL;
  
  return po;
}

void physics_object_delete(physics_object* po) {
  
  if (po->collision_body != NULL) {
      collision_body_delete(po->collision_body);
  }
  
  free(po);
  
}

void physics_object_update(physics_object* po, float timestep) {
  
  po->previous_position = po->position;
  
  po->position = v3_add(po->position, v3_mul(po->velocity, timestep));
  
  /* Need to work out how to scale rotations */
  //po->rotation = v4_quaternion_mul(po->rotation, po->angular_velocity);
  
  po->velocity = v3_add(po->velocity, v3_mul(po->acceleration, timestep));
  
  /* Same again here */
  //po->angular_velocity = v4_quaternion_mul(po->angular_velocity, po->angular_acceleration);
  
}

void physics_object_collide_static(physics_object* po, static_object* so, float timestep) {
  
  collision_body* po_col = po->collision_body;
  collision_body* so_col = so->collision_body;
  
  if ((po_col == NULL) || (so_col == NULL)) {
    error("Cannot collide objects. One or more has no collision body - First: %p Second: %p).", po_col, so_col);
  }
  
  matrix_4x4 po_world = m44_world(po->position, po->scale, po->rotation);
  matrix_4x4 so_world = m44_world(so->position, so->scale, so->rotation);
  
  sphere po_sphere = sphere_transform(po_col->collision_sphere, po_world);
  sphere so_sphere = sphere_transform(so_col->collision_sphere, so_world);
  
  box po_box = box_transform(po_col->collision_box, po_world);
  box so_box = box_transform(so_col->collision_box, so_world);
  
  collision c;
  int col_count = 0;
  while(col_count < 2) {
    
    c.collided = false;
    c.time = timestep;
    
    if (!sphere_contains_sphere(so_sphere, po_sphere)) break;
    
    if (po_col->collision_type == collision_type_sphere) {
    
      if (so_col->collision_type == collision_type_sphere) {
        sphere_collide_sphere(&c, po_sphere, po->velocity, so_sphere, timestep);
        if (!c.collided) break;
      }
    
      if (so_col->collision_type == collision_type_box) {
        sphere_collide_box(&c, po_sphere, po->velocity, so_box, timestep);
        if (!c.collided) break;
      }
      
      if (so_col->collision_type == collision_type_mesh) {
        sphere_collide_mesh(&c, po_sphere, po->velocity, so_col->collision_mesh, so_world, timestep);
        if (!c.collided) break;
      }
    }
    
    if (c.collided) {
      po->position = c.object_position;
      po->velocity = v3_mul(po->velocity, 0.25);
      po->velocity = v3_reflect(po->velocity, c.surface_normal);
      col_count++;
    }
    
  }
  
}
