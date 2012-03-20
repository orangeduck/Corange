#include <stdlib.h>
#include <time.h>

#include "corange.h"

#include "particles.h"
#include "kernel.h"

static int particle_count = 32;

GLuint positions_buffer;
GLuint velocities_buffer;
GLuint lifetimes_buffer;
GLuint randoms_buffer;

kernel k_update;

kernel_memory k_particle_positions;
kernel_memory k_particle_velocities;
kernel_memory k_particle_lifetimes;
kernel_memory k_particle_randoms;

bool reset = 0;

void particles_init() {

  vector4* particle_positions = malloc(sizeof(vector4) * particle_count);
  vector4* particle_velocities = malloc(sizeof(vector4) * particle_count);
  float* particle_lifetimes = malloc(sizeof(float) * particle_count);
  vector4* particle_randoms = malloc(sizeof(vector4) * particle_count);
  
  srand(time(NULL));
  
  for(int i = 0; i < particle_count; i++) {
    particle_lifetimes[i] = 999;
    particle_positions[i] = v4(0,0,0,0);
    particle_velocities[i] = v4(0,0,0,0);
    
    float rx = ((float)rand() / RAND_MAX) * 2 - 1;
    float ry = ((float)rand() / RAND_MAX) * 2 + 0.5;
    float rz = ((float)rand() / RAND_MAX) * 2 - 1;
    float rm = (float)rand() / RAND_MAX;
    
    vector3 rand = v3_mul(v3_normalize(v3(rx, ry, rz)), rm * 2);
    
    particle_randoms[i] = v4(rand.x, rand.y, rand.z, 0);
  }
    
  glGenBuffers(1, &positions_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, positions_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * particle_count, particle_positions, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &velocities_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, velocities_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * particle_count, particle_velocities, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &lifetimes_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, lifetimes_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particle_count, particle_lifetimes, GL_DYNAMIC_COPY);
  
  glGenBuffers(1, &randoms_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, randoms_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vector4) * particle_count, particle_randoms, GL_DYNAMIC_COPY);
  
  k_particle_positions = kernel_memory_from_glbuffer(positions_buffer);
  k_particle_velocities = kernel_memory_from_glbuffer(velocities_buffer);
  k_particle_lifetimes = kernel_memory_from_glbuffer(lifetimes_buffer);
  k_particle_randoms = kernel_memory_from_glbuffer(randoms_buffer);
  
  kernel_program* program = asset_get("./kernels/particles.cl");
  
  float max_life = 60.0;
  float min_velocity = 0.5;
  
  k_update = kernel_program_get_kernel(program, "particle_update");
  kernel_set_argument(k_update, 0, sizeof(kernel_memory), &k_particle_positions);
  kernel_set_argument(k_update, 1, sizeof(kernel_memory), &k_particle_velocities);
  kernel_set_argument(k_update, 2, sizeof(kernel_memory), &k_particle_lifetimes);
  kernel_set_argument(k_update, 3, sizeof(kernel_memory), &k_particle_randoms);
  kernel_set_argument(k_update, 4, sizeof(cl_float), &max_life);
  kernel_set_argument(k_update, 5, sizeof(cl_float), &min_velocity);
  kernel_set_argument(k_update, 9, sizeof(cl_int), &particle_count);
  
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
  
  int random = rand();
  
  kernel_memory_gl_aquire(k_particle_positions);
  kernel_memory_gl_aquire(k_particle_velocities);
  kernel_memory_gl_aquire(k_particle_lifetimes);
  kernel_memory_gl_aquire(k_particle_randoms);
  
    kernel_set_argument(k_update, 6, sizeof(cl_float), &timestep);
    kernel_set_argument(k_update, 7, sizeof(cl_int), &reset);
    kernel_set_argument(k_update, 8, sizeof(cl_int), &random);
    kernel_run(k_update, particle_count);
    
    reset = 0;
  
  kernel_memory_gl_release(k_particle_positions);
  kernel_memory_gl_release(k_particle_velocities);
  kernel_memory_gl_release(k_particle_lifetimes);
  kernel_memory_gl_release(k_particle_randoms);
  
  kernel_run_finish();
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

void particle_positions(vector4* out) {
  kernel_memory_gl_aquire(k_particle_positions);
    kernel_memory_read(k_particle_positions, sizeof(vector4) * particle_count, out);
  kernel_memory_gl_release(k_particle_positions);
}

static float view_matrix[16];
static float proj_matrix[16];

void particles_render() {
  
  camera* cam = entity_get("camera");
  
  matrix_4x4 viewm = camera_view_matrix(cam);
  matrix_4x4 projm = camera_proj_matrix(cam, graphics_viewport_ratio() );
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);

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