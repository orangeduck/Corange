#include <stdlib.h>
#include <time.h>

#include "corange.h"

#include "particles.h"
#include "kernel.h"

#define particle_count 25

GLuint positions_buffer;
GLuint velocities_buffer;
GLuint lifetimes_buffer;
GLuint randoms_buffer;

kernel k_update;

kernel_memory k_particle_positions;
kernel_memory k_particle_velocities;
kernel_memory k_particle_lifetimes;
kernel_memory k_particle_randoms;

int reset = 0;

void particles_init() {

  vector4* particle_positions = malloc(sizeof(vector4) * 4 * particle_count);
  vector4* particle_velocities = malloc(sizeof(vector4) * 4 * particle_count);
  float* particle_lifetimes = malloc(sizeof(float) * 4 * particle_count);
  vector4* particle_randoms = malloc(sizeof(vector4) * 4 * particle_count);
  
  srand(time(NULL));
  
  int i = 0;
  while(i < 4 *particle_count) {
  
    particle_positions[i  ] = v4(0,0,0,1);
    particle_positions[i+1] = v4(0,0,0,1);
    particle_positions[i+2] = v4(0,0,0,1);
    particle_positions[i+3] = v4(0,0,0,1);
    
    float rx = ((float)rand() / RAND_MAX);
    float ry = ((float)rand() / RAND_MAX);
    float rz = ((float)rand() / RAND_MAX);
    
    vector3 normed = v3_normalize( v3(rx - 0.5, ry, rz - 0.5) );
    normed = v3_mul(normed, 0.75);
    rx = normed.x;
    ry = normed.y;
    rz = normed.z;
    
    particle_velocities[i  ] = v4(rx, ry, rz, 0);
    particle_velocities[i+1] = v4(rx, ry, rz, 0);
    particle_velocities[i+2] = v4(rx, ry, rz, 0);
    particle_velocities[i+3] = v4(rx, ry, rz, 0);
    
    particle_randoms[i  ] = v4(rx, ry, rz, 0);
    particle_randoms[i+1] = v4(rx, ry, rz, 0);
    particle_randoms[i+2] = v4(rx, ry, rz, 0);
    particle_randoms[i+3] = v4(rx, ry, rz, 0);
    
    particle_lifetimes[i  ] = 0.0;
    particle_lifetimes[i+1] = 0.0;
    particle_lifetimes[i+2] = 0.0;
    particle_lifetimes[i+3] = 0.0;
    
    i += 4;
  }
    
  glGenBuffers(1, &positions_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * 4 * particle_count, particle_positions, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &velocities_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * 4 * particle_count, particle_velocities, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &lifetimes_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, lifetimes_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * particle_count, particle_lifetimes, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &randoms_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, randoms_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * 4 * particle_count, particle_randoms, GL_DYNAMIC_COPY);
  
  k_particle_positions = kernel_memory_from_glbuffer(positions_buffer);
  k_particle_velocities = kernel_memory_from_glbuffer(velocities_buffer);
  k_particle_lifetimes = kernel_memory_from_glbuffer(lifetimes_buffer);
  k_particle_randoms = kernel_memory_from_glbuffer(randoms_buffer);
  
  kernel_program* program = asset_get("./kernels/particles.cl");
  
  float max_life = 1.0;
  float min_velocity = 0.1;
  
  int reset = 0;
  
  k_update = kernel_program_get_kernel(program, "particle_update");
  kernel_set_argument(k_update, 0, sizeof(kernel_memory), &k_particle_positions);
  kernel_set_argument(k_update, 1, sizeof(kernel_memory), &k_particle_velocities);
  kernel_set_argument(k_update, 2, sizeof(kernel_memory), &k_particle_lifetimes);
  kernel_set_argument(k_update, 3, sizeof(kernel_memory), &k_particle_randoms);
  kernel_set_argument(k_update, 4, sizeof(float), &max_life);
  kernel_set_argument(k_update, 5, sizeof(float), &min_velocity);
  
  free(particle_positions);
  free(particle_velocities);
  free(particle_lifetimes);
  free(particle_randoms);
  
}

void particles_finish() {
  
  kernel_memory_delete(k_particle_positions);
  kernel_memory_delete(k_particle_velocities);
  kernel_memory_delete(k_particle_lifetimes);
  kernel_memory_delete(k_particle_randoms);
  
  glDeleteBuffers(1, &positions_buffer);
  glDeleteBuffers(1, &velocities_buffer);
  glDeleteBuffers(1, &lifetimes_buffer);
  glDeleteBuffers(1, &randoms_buffer);
  
}

void particles_update(float timestep) {
  
  kernel_memory_gl_aquire(k_particle_positions);
  kernel_memory_gl_aquire(k_particle_velocities);
  kernel_memory_gl_aquire(k_particle_lifetimes);
  kernel_memory_gl_aquire(k_particle_randoms);
  
    kernel_set_argument(k_update, 6, sizeof(float), &timestep);
    kernel_set_argument(k_update, 7, sizeof(int), &reset);
    kernel_run(k_update, particle_count);
    
    reset = 0;
  
  kernel_memory_gl_release(k_particle_positions);
  kernel_memory_gl_release(k_particle_velocities);
  kernel_memory_gl_release(k_particle_lifetimes);
  kernel_memory_gl_release(k_particle_randoms);
}

void particles_reset() {
  reset = 1;
}

int particles_count() {
  return particle_count;
}

GLuint particle_positions_buffer() {
  return positions_buffer;
}

GLuint particle_velocities_buffer() {
  return velocities_buffer;
}

vector4 positions_ret[particle_count];
vector4 positions_large[particle_count * 4];

vector4* particle_positions() {
  
  kernel_memory_gl_aquire(k_particle_positions);
  kernel_memory_read(k_particle_positions, sizeof(vector4) * 4 * particle_count, positions_large);
  kernel_memory_gl_release(k_particle_positions);
  
  int i=0;
  for(i = 0; i < particle_count; i ++) {
    positions_ret[i] = positions_large[i*4];
  }
  
  return positions_ret;
}