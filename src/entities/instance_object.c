#include "entities/instance_object.h"

#include "cnet.h"

#include "assets/renderable.h"

instance_object* instance_object_new() {
  instance_object* io = malloc(sizeof(instance_object));
  
  io->num_instances = 0;
  io->instances = malloc(sizeof(instance_data) * 0);
  
  if (net_is_client()) {
    glGenBuffers(1, &io->world_buffer);
  }
  
  io->bound = sphere_unit();
  io->renderable = asset_hndl_null();
  io->collision_body = asset_hndl_null();
  
  return io;
}

void instance_object_delete(instance_object* io) {
  
  if (net_is_client()) {
    glDeleteBuffers(1, &io->world_buffer);
  }

  free(io->instances);
  free(io);
}

void instance_object_update(instance_object* io) {
  
  if (net_is_server()) { return; }

  /* TODO: Make struct member */
  mat4* world_data = malloc(sizeof(mat4) * io->num_instances);
  
  for (int i = 0; i < io->num_instances; i++) {
    instance_data id = io->instances[i];
    io->instances[i].world = mat4_world(id.position, id.scale, id.rotation);
    io->instances[i].world_normal = mat3_transpose(mat3_inverse(mat4_to_mat3(io->instances[i].world)));
    world_data[i] = mat4_transpose(io->instances[i].world);
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * io->num_instances, world_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  free(world_data);
  
  io->bound = sphere_unit();
  
  if (!asset_hndl_isnull(&io->renderable)) {
  
    renderable* r = asset_hndl_ptr(&io->renderable);
    sphere rbound = sphere_unit();
    for (int i = 0; i < r->num_surfaces; i++) {
      rbound = sphere_merge(rbound, r->surfaces[i]->bound);
    }
    
    for (int i = 0; i < io->num_instances; i++) {
      io->bound = sphere_merge(io->bound, sphere_transform(rbound, io->instances[i].world));
    }
    
  }
  
}

void instance_object_add_instance(instance_object* io, vec3 position, vec3 scale, quat rotation) {
  
  instance_data id;
  id.position = position;
  id.scale = scale;
  id.rotation = rotation;
  id.world = mat4_world(id.position, id.scale, id.rotation);
  id.world_normal = mat3_transpose(mat3_inverse(mat4_to_mat3(id.world)));
  
  io->num_instances++;
  io->instances = realloc(io->instances, sizeof(instance_data) * io->num_instances);
  io->instances[io->num_instances-1] = id;
  
  instance_object_update(io);
  
}

void instance_object_rem_instance(instance_object* io, int i) {
  
  if (i >= io->num_instances) {
    error("Cannot remove instance %i: instance object only has %i instances!", i, io->num_instances);
    return;
  }  
  
  memmove( &io->instances[i+0], &io->instances[i+1],
    sizeof(instance_data) * (io->num_instances-i-1));
  
  io->num_instances--;
  io->instances = realloc(io->instances, sizeof(instance_data) * io->num_instances);
  
  instance_object_update(io);
  
}

mat4 instance_object_world(instance_object* io, int i) {
  return io->instances[i].world;
}

mat3 instance_object_world_normal(instance_object* io, int i) {
  return io->instances[i].world_normal;
}
