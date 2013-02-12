#include "entities/instance_object.h"

#include "cnet.h"

instance_object* instance_object_new() {
  instance_object* io = malloc(sizeof(instance_object));
  
  io->num_instances = 0;
  io->instances = malloc(sizeof(instance_data) * 0);
  
  if (net_is_client()) {
    glGenBuffers(1, &io->world_buffer);
  }

  io->active = true;
  io->recieve_shadows = true;
  io->cast_shadows = true;
  
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
  
  if (!net_is_client()) { return; }

  mat4* world_data = malloc(sizeof(mat4) * io->num_instances);
  
  for (int i = 0; i < io->num_instances; i++) {
    instance_data id = io->instances[i];
    world_data[i] = mat4_transpose(mat4_world(id.position, id.scale, id.rotation));
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(mat4) * io->num_instances, world_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  free(world_data);
  
}

void instance_object_add_instance(instance_object* io, vec3 position, vec3 scale, mat4 rotation) {
  
  instance_data id;
  id.position = position;
  id.scale = scale;
  id.rotation = rotation;
  
  io->num_instances++;
  io->instances = realloc(io->instances, sizeof(instance_data) * io->num_instances);
  io->instances[io->num_instances-1] = id;
  
  instance_object_update(io);
  
}

mat4 instance_object_world(instance_object* io, int i) {
  
  instance_data id = io->instances[i]; 
  return mat4_world(id.position, id.scale, id.rotation);
  
}

