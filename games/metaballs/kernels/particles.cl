#include "./games/metaballs/kernels/structs.cl"

__kernel void particle_update(__global vector3* positions, __global vector3* velocities, float time_difference, int count) {
  
  const int id = get_global_id(0);
  if (id >= count) { return; }
  
  /* Update positions and velocity */
  positions[id].x = positions[id].x + (velocities[id].x * time_difference);
  positions[id].y = positions[id].y + (velocities[id].y * time_difference);
  positions[id].z = positions[id].z + (velocities[id].z * time_difference);
  
  velocities[id].y = velocities[id].y - (9.81 * time_difference);
  
  /* Bounce on floors */
  if (positions[id].y < 0) {
    
    velocities[id].x = velocities[id].x * 0.9;
    velocities[id].y = velocities[id].y * 0.9;
    velocities[id].z = velocities[id].z * 0.9;
    
    velocities[id].y = -velocities[id].y;
    positions[id].y = -positions[id].y;
  
  }

}