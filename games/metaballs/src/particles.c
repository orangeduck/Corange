#include <stdlib.h>
#include <time.h>

#include "corange.h"

#include "particles.h"


vector3* particle_positions = NULL;
vector3* particle_velocities = NULL;

kernel k_update;

kernel_memory k_particle_positions;
kernel_memory k_particle_velocities;

int particle_count = 32;

int position_read_updated = 0;
int velocity_read_updated = 0;

void particles_init() {

  particle_positions = malloc(sizeof(vector3) * particle_count);
  particle_velocities = malloc(sizeof(vector3) * particle_count);
  
  srand(time(NULL));
  
  int i;
  for(i = 0; i < particle_count; i++) {
    particle_positions[i] = v3_zero();
    float vx = ((float)rand() / RAND_MAX) - 0.5;
    float vy = ((float)rand() / RAND_MAX);
    float vz = 0.0;
    particle_velocities[i] = v3(vx , vy, vz);
  }
  
  k_particle_positions = kernel_memory_allocate(sizeof(vector3) * particle_count);
  k_particle_velocities = kernel_memory_allocate(sizeof(vector3) * particle_count);
  
  kernel_memory_write(k_particle_positions, sizeof(vector3) * particle_count, particle_positions);
  kernel_memory_write(k_particle_velocities, sizeof(vector3) * particle_count, particle_velocities);
  
  kernel_program* program = asset_get("/kernels/particles.cl");
  
  k_update = kernel_program_get_kernel(program, "particle_update");
  kernel_set_argument(k_update, 0, sizeof(kernel_memory), &k_particle_positions);
  kernel_set_argument(k_update, 1, sizeof(kernel_memory), &k_particle_velocities);
  kernel_set_argument(k_update, 3, sizeof(int), &particle_count);
  
}

void particles_finish() {
  
  free(particle_positions);
  free(particle_velocities);
  
  kernel_memory_delete(k_particle_positions);
  kernel_memory_delete(k_particle_velocities);
  
}

void particles_update(float timestep) {

  kernel_set_argument(k_update, 2, sizeof(float), &timestep);
  kernel_run(k_update, particle_count, particle_count);
  
  position_read_updated = 0;
  velocity_read_updated = 0;
}

int particles_count() {
  return particle_count;
}

vector3 particle_position(int id) {
  
  if (!position_read_updated) {
    kernel_memory_read(k_particle_positions, sizeof(vector3) * particle_count, particle_positions);
    position_read_updated = 1;
  }
  
  return particle_positions[id];
  
}

vector3 particle_velocity(int id) {

  if (!velocity_read_updated) {
    kernel_memory_read(k_particle_velocities, sizeof(vector3) * particle_count, particle_velocities);
    velocity_read_updated = 1;
  }

  return particle_velocities[id];
  
}