#include "entities/physics_object.h"

#include "assets/cmesh.h"

physics_object* physics_object_new() {

  physics_object* po = malloc(sizeof(physics_object));
  
  po->position = vec3_zero();
  po->rotation = mat4_id();
  po->scale = vec3_one();
  
  po->velocity = vec3_zero();
  po->angular_velocity = mat4_id();
  
  po->acceleration = vec3_new(0, -9.81, 0);
  po->angular_acceleration = mat4_id();
  
  po->previous_position = vec3_zero();
  
  po->elasticity = 0.1;
  po->friction = 0.25;
  
  po->active = true;
  po->recieve_shadows = true;
  po->cast_shadows = true;
  
  po->renderable = asset_hndl_null();
  //po->collision_body = asset_hndl_null();
  
  return po;
}

void physics_object_delete(physics_object* po) {
  free(po);
}

void physics_object_update(physics_object* po, float timestep) {
  
  po->previous_position = po->position;
  
  po->position = vec3_add(po->position, vec3_mul(po->velocity, timestep));
  
  /* Need to work out how to scale rotations */
  //po->rotation = v4_quaternion_mul(po->rotation, po->angular_velocity);
  
  po->velocity = vec3_add(po->velocity, vec3_mul(po->acceleration, timestep));
  
  /* Same again here */
  //po->angular_velocity = v4_quaternion_mul(po->angular_velocity, po->angular_acceleration);
  
}

void physics_object_collide_static(physics_object* po, static_object* so, float timestep) {
  
  /*
  collision_body* po_col = asset_hndl_ptr(po->collision_body);
  collision_body* so_col = asset_hndl_ptr(so->collision_body);
  
  if ((po_col == NULL) || (so_col == NULL)) {
    error("Cannot collide objects. One or more has no collision body - First: %p Second: %p).", po_col, so_col);
  }
  
  mat4 po_world = mat4_world(po->position, po->scale, po->rotation);
  mat4 so_world = mat4_world(so->position, so->scale, so->rotation);
  
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
      po->velocity = vec3_mul(po->velocity, 0.25);
      po->velocity = vec3_reflect(po->velocity, c.surface_normal);
      col_count++;
    }
    
  }
  */
  
}
