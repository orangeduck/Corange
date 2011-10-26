#include "./games/metaballs/kernels/structs.cl"

__kernel void particle_update(__global float4* positions, 
                              __global float4* velocities, 
                              __global float* lifetimes,  
                              __global float4* randoms,
                              
                              const float max_life,
                              const float min_velocity,
                              
                              float time_difference 
                              ) {
  
  /* 4 times due to the 4 values required for billboards */
  const int i = 4 * get_global_id(0);
  
  lifetimes[i] = lifetimes[i] + time_difference;
  
  if ((lifetimes[i] > max_life) || ( length(velocities[i]) < min_velocity )) {
    
    lifetimes[i] = 0.0;
    
    positions[i] = (float4)(0,0,0,1);
    float rx = randoms[i].x;
    float ry = randoms[i].y;
    float rz = randoms[i].z;
    velocities[i] = (float4)(rx - 0.5, ry, rz - 0.5, 0);
  
  } else {
  
    /* Update positions and velocity */
    positions[i].xyz = positions[i].xyz + (velocities[i].xyz * time_difference);
    velocities[i].y = velocities[i].y - (9.81 * time_difference);
    
    /* Bounce on floors */
    if (positions[i].y < 0.0) {
      velocities[i].xyz = velocities[i].xyz * 0.75;
      velocities[i].y = -velocities[i].y;
    }

  }
  
  /* Update other positions in buffer (for billboards */
  
  positions[i+1] = positions[i];
  positions[i+2] = positions[i];
  positions[i+3] = positions[i];
  
  velocities[i+1] = velocities[i];
  velocities[i+2] = velocities[i];
  velocities[i+3] = velocities[i];
  
  lifetimes[i+1] = lifetimes[i];
  lifetimes[i+2] = lifetimes[i];
  lifetimes[i+3] = lifetimes[i];
  
}