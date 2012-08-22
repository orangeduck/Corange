#include <stdlib.h>
#include <time.h>

#include "corange.h"

#include "particles.h"
#include "kernel.h"

static int particle_count = 64;

vec4* particle_positions;
vec4* particle_velocities;
float* particle_lifetimes;
vec4* particle_randoms;

GLuint positions_buffer;
GLuint velocities_buffer;
GLuint lifetimes_buffer;
GLuint randoms_buffer;

kernel_memory k_particle_positions;
kernel_memory k_particle_velocities;
kernel_memory k_particle_lifetimes;
kernel_memory k_particle_randoms;

kernel k_update;

bool reset = 0;

void particles_init() {

  particle_positions = malloc(sizeof(vec4) * particle_count);
  particle_velocities = malloc(sizeof(vec4) * particle_count);
  particle_lifetimes = malloc(sizeof(float) * particle_count);
  particle_randoms = malloc(sizeof(vec4) * particle_count);
  
  srand(time(NULL));
  
  for(int i = 0; i < particle_count; i++) {
    particle_lifetimes[i] = 999;
    particle_positions[i] = vec4_new(0,0,0,0);
    particle_velocities[i] = vec4_new(0,0,0,0);
    
    float rx = ((float)rand() / RAND_MAX) * 2 - 1;
    float ry = ((float)rand() / RAND_MAX) * 2 + 0.5;
    float rz = ((float)rand() / RAND_MAX) * 2 - 1;
    float rm = (float)rand() / RAND_MAX;
    
    vec3 rand = vec3_mul(vec3_normalize(vec3_new(rx, ry, rz)), rm * 2);
    
    particle_randoms[i] = vec4_new(rand.x, rand.y, rand.z, 0);
  }
    
  glGenBuffers(1, &positions_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, particle_positions, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &velocities_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, particle_velocities, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &lifetimes_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, lifetimes_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particle_count, particle_lifetimes, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &randoms_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, randoms_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, particle_randoms, GL_DYNAMIC_COPY);
 
#ifdef OPEN_GL_CPU
  #ifndef CPU_ONLY
  k_particle_positions = kernel_memory_allocate(sizeof(vec4) * particle_count);
  k_particle_velocities = kernel_memory_allocate(sizeof(vec4) * particle_count);
  k_particle_lifetimes = kernel_memory_allocate(sizeof(float) * particle_count);
  k_particle_randoms = kernel_memory_allocate(sizeof(vec4) * particle_count);
  
  kernel_memory_write(k_particle_positions, sizeof(vec4) * particle_count, particle_positions);
  kernel_memory_write(k_particle_velocities, sizeof(vec4) * particle_count, particle_velocities);
  kernel_memory_write(k_particle_lifetimes, sizeof(float) * particle_count, particle_lifetimes);
  kernel_memory_write(k_particle_randoms, sizeof(vec4) * particle_count, particle_randoms);
  #endif
#else
  k_particle_positions = kernel_memory_from_glbuffer(positions_buffer);
  k_particle_velocities = kernel_memory_from_glbuffer(velocities_buffer);
  k_particle_lifetimes = kernel_memory_from_glbuffer(lifetimes_buffer);
  k_particle_randoms = kernel_memory_from_glbuffer(randoms_buffer);
#endif
  
  kernel_program* program = asset_hndl_ptr(asset_hndl_new_load(P("./kernels/particles.cl")));
  
  float max_life = 60.0;
  float min_velocity = 0.5;
  
#ifndef CPU_ONLY
  k_update = kernel_program_get_kernel(program, "particle_update");
  kernel_set_argument(k_update, 0, sizeof(kernel_memory), &k_particle_positions);
  kernel_set_argument(k_update, 1, sizeof(kernel_memory), &k_particle_velocities);
  kernel_set_argument(k_update, 2, sizeof(kernel_memory), &k_particle_lifetimes);
  kernel_set_argument(k_update, 3, sizeof(kernel_memory), &k_particle_randoms);
  kernel_set_argument(k_update, 4, sizeof(cl_float), &max_life);
  kernel_set_argument(k_update, 5, sizeof(cl_float), &min_velocity);
  kernel_set_argument(k_update, 9, sizeof(cl_int), &particle_count);
#endif
  
}

void particles_finish() {
  
#ifndef CPU_ONLY
  kernel_memory_delete(k_particle_positions);
  kernel_memory_delete(k_particle_velocities);
  kernel_memory_delete(k_particle_lifetimes);
  kernel_memory_delete(k_particle_randoms);
#endif
  
  glDeleteBuffers(1, &positions_buffer);
  glDeleteBuffers(1, &velocities_buffer);
  glDeleteBuffers(1, &lifetimes_buffer);
  glDeleteBuffers(1, &randoms_buffer);
  
  free(particle_positions);
  free(particle_velocities);
  free(particle_lifetimes);
  free(particle_randoms);
  
}

void particles_update(float timestep) {
  
  int random = rand();
  
#ifndef CPU_ONLY
  
  #ifndef OPEN_GL_CPU
    kernel_memory_gl_aquire(k_particle_positions);
    kernel_memory_gl_aquire(k_particle_velocities);
    kernel_memory_gl_aquire(k_particle_lifetimes);
    kernel_memory_gl_aquire(k_particle_randoms);
  #endif
    
      kernel_set_argument(k_update, 6, sizeof(cl_float), &timestep);
      kernel_set_argument(k_update, 7, sizeof(cl_int), &reset);
      kernel_set_argument(k_update, 8, sizeof(cl_int), &random);
      kernel_run(k_update, particle_count);
      
      reset = 0;
    
  #ifndef OPEN_GL_CPU
    kernel_memory_gl_release(k_particle_positions);
    kernel_memory_gl_release(k_particle_velocities);
    kernel_memory_gl_release(k_particle_lifetimes);
    kernel_memory_gl_release(k_particle_randoms);
  #endif
    
    kernel_run_finish();
  
#else
  
  for(int i = 0; i < particle_count; i++) {
  
    particle_lifetimes[i] = particle_lifetimes[i] + timestep;
    
    if ((particle_lifetimes[i] > 60.0) || ( v4_length(particle_velocities[i]) < 0.5 )) {
      
      particle_lifetimes[i] = 0.0;
      
      particle_positions[i] = v4(32,15,32,1);
      
      int random_index = (random + i) % particle_count;
      float rx = particle_randoms[random_index].x;
      float ry = particle_randoms[random_index].y;
      float rz = particle_randoms[random_index].z;
      particle_velocities[i] = v4_mul(v4(rx, ry, rz, 0), 5);
    
    } else {
    
      /* Update positions and velocity */
      particle_positions[i].x = particle_positions[i].x + (particle_velocities[i].x * timestep);
      particle_positions[i].y = particle_positions[i].y + (particle_velocities[i].y * timestep);
      particle_positions[i].z = particle_positions[i].z + (particle_velocities[i].z * timestep);
      
      particle_velocities[i].y = particle_velocities[i].y - (9.81 * timestep);
      
      /* Bounce on floors */
      if (particle_positions[i].y < 15.0) {
        particle_velocities[i].x = particle_velocities[i].x * 0.75;
        particle_velocities[i].y = particle_velocities[i].y * 0.75;
        particle_velocities[i].z = particle_velocities[i].z * 0.75;
        particle_velocities[i].y = -particle_velocities[i].y;
      }
    }
  }

#endif
  
}

void particles_reset() {
  reset = 1;
}

int particles_count() {
  return particle_count;
}

kernel_memory particle_positions_memory() {
  return k_particle_positions;
}

GLuint particle_positions_buffer() {
  return positions_buffer;
}

GLuint particle_velocities_buffer() {
  return velocities_buffer;
}

static float view_matrix[16];
static float proj_matrix[16];

void particles_render() {
  
#ifdef OPEN_GL_CPU
  
  #ifndef CPU_ONLY
    kernel_memory_read(k_particle_positions, sizeof(vec4) * particle_count, particle_positions);
  #endif
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, particle_positions, GL_DYNAMIC_COPY);
  
  #ifndef CPU_ONLY
    kernel_memory_read(k_particle_randoms, sizeof(vec4) * particle_count, particle_randoms);
  #endif
  glBindBuffer(GL_ARRAY_BUFFER, randoms_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * particle_count, particle_randoms, GL_DYNAMIC_COPY);

#endif
  
  camera* cam = entity_get("camera");
  
  mat4 viewm = camera_view_matrix(cam);
  mat4 projm = camera_proj_matrix(cam, graphics_viewport_ratio() );
  
  mat4_to_array(viewm, view_matrix);
  mat4_to_array(projm, proj_matrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glVertexPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, randoms_buffer);
  glColorPointer(4, GL_FLOAT, 0, (void*)0);
  glEnableClientState(GL_COLOR_ARRAY);
  
    glDrawArrays(GL_POINTS, 0, particle_count);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  
}