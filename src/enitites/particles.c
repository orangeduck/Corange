
#include "entities/particles.h"

void particle_effector_basic(particles* p, float timestep) {
  particle_effector_time(p, timestep);
  particle_effector_movement(p, timestep);
  particle_effector_gravity(p, timestep);
  particle_effector_burst(p, timestep);
}

void particle_effector_time(particles* p, float timestep) {

  for (int i = 0; i < p->count; i++) {
    p->times[i] = p->times[i] + timestep;
  }

}

void particle_effector_movement(particles* p, float timestep) {
  
  for (int i = 0; i < p->count; i++) {
    p->positions[i] = vec3_add(p->positions[i], vec3_mul(p->velocities[i], timestep));
  }  
  
}

void particle_effector_gravity(particles* p, float timestep) {

  for (int i = 0; i < p->count; i++) {
    p->velocities[i] = vec3_add(p->velocities[i], vec3_mul(vec3_gravity(), timestep));
  }

}

void particle_effector_burst(particles* p, float timestep) {
  
  srand(time(NULL));
  
  for (int i = 0; i < p->count; i++) {
    if (p->times[i] > p->lifetime) {
      p->times[i] = 0;
      float rx = ((float)rand() / RAND_MAX) - 0.5;
      float ry =  (float)rand() / RAND_MAX;
      float rz = ((float)rand() / RAND_MAX) - 0.5;
      p->velocities[i] = vec3_normalize(vec3_new(rx, ry, rz));
    }
  }
  
}

particles* partcicles_new() {
  particles* p = malloc(sizeof(particles));
  
  p->position = vec3_zero();
  p->rotation = mat4_id();
  p->scale = vec3_zero();
  
  p->texture = asset_hndl_null();
  p->blend_src = GL_ONE;
  p->blend_dst = GL_ONE;
  
  p->lifetime = 0;
  p->count = 0;
  
  p->times = NULL;
  p->scales = NULL;
  p->colors = NULL;
  p->positions = NULL;
  p->velocities = NULL;
  
  glGenBuffers(1, &p->vertex_buff);
  p->vertex_data = NULL;
  
  p->effectors_num = 0;
  
  return p;
}

void particles_delete(particles* p) {
  
  glDeleteBuffers(1, &p->vertex_buff);
  free(p);
  
}

void particles_add_effector(particles* p, particle_effector pe) {
  if (p->effectors_num == MAX_EFFECTORS) {
    warning("Particle System already has maximum effectors");
    return;
  }
  
  p->effectors[p->effectors_num] = pe;
  p->effectors_num++;
  
}


void particles_allocate(particles* p, int count, float lifetime) {
  
  p->lifetime = lifetime;
  p->count = count;
  p->times = realloc(p->times, sizeof(float) * count);
  p->scales = realloc(p->scales, sizeof(vec3) * count);
  p->colors = realloc(p->colors, sizeof(vec4) * count);
  p->positions = realloc(p->positions, sizeof(vec3) * count);
  p->velocities = realloc(p->velocities, sizeof(vec3) * count);
  
  const int stride = 9;
  
  p->vertex_data = realloc(p->vertex_data, sizeof(float) * stride * 6 * count);
  
  for (int i = 0; i < p->count; i++) {
    p->times[i] = 0;
    p->scales[i] = vec3_one();
    p->colors[i] = vec4_white();
    p->positions[i] = vec3_zero();
    p->velocities[i] = vec3_zero();
  }
  
}

static void add_vertex(float* data, int* index, vec3 position, vec2 uvs, vec4 color) {

  data[*index] = position.x; *index++;
  data[*index] = position.y; *index++;
  data[*index] = position.z; *index++;
  data[*index] = uvs.x; *index++;
  data[*index] = uvs.y; *index++;
  data[*index] = color.x; *index++;
  data[*index] = color.y; *index++;
  data[*index] = color.z; *index++;
  data[*index] = color.w; *index++;
  
}

void particles_update(particles* p, float timestep) {
  
  for (int i = 0; i < p->effectors_num; i++) {
    p->effectors[i](p, timestep);
  }
  
  int vi = 0;
  for (int i = 0; i < p->count; i++) {
  
    // TODO: Orient to camera
  
    vec3 vertex_0 = vec3_add(p->positions[i], vec3_new(-p->scales[i].x,  p->scales[i].y, 0));
    vec3 vertex_1 = vec3_add(p->positions[i], vec3_new( p->scales[i].x,  p->scales[i].y, 0));
    vec3 vertex_2 = vec3_add(p->positions[i], vec3_new( p->scales[i].x, -p->scales[i].y, 0));
    vec3 vertex_3 = vec3_add(p->positions[i], vec3_new(-p->scales[i].x, -p->scales[i].y, 0));
  
    add_vertex(p->vertex_data, &vi, vertex_0, vec2_new(0, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_1, vec2_new(1, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_2, vec2_new(1, 0), p->colors[i]);

    add_vertex(p->vertex_data, &vi, vertex_0, vec2_new(0, 1), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_2, vec2_new(1, 0), p->colors[i]);
    add_vertex(p->vertex_data, &vi, vertex_3, vec2_new(0, 0), p->colors[i]);
    
  }
  
  const int stride = 9;
  
  glBindBuffer(GL_ARRAY_BUFFER, p->vertex_buff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * stride * 6 * p->count, p->vertex_data, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
}