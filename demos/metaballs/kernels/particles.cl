kernel void particle_update(global float4* positions, 
                            global float4* velocities, 
                            global float* lifetimes,  
                            global float4* randoms,
                            
                            const float max_life,
                            const float min_velocity,
                            
                            const float time_difference,
                            const int reset,
                            
                            const int random
                              ) {
  
  const int i = get_global_id(0);
  
  lifetimes[i] = lifetimes[i] + time_difference;
  
  if ((lifetimes[i] > max_life) || ( length(velocities[i]) < min_velocity ) || reset) {
    
    lifetimes[i] = 0.0;
    
    //positions[i] = (float4)(0,0,0,1);
    positions[i] = (float4)(32,15,32,1);
    
    int random_index = (random + i) % 25;
    float rx = randoms[random_index].x;
    float ry = randoms[random_index].y;
    float rz = randoms[random_index].z;
    velocities[i] = (float4)(rx, ry, rz, 0) * 5;
  
  } else {
  
    /* Update positions and velocity */
    positions[i].xyz = positions[i].xyz + (velocities[i].xyz * time_difference);
    velocities[i].y = velocities[i].y - (9.81 * time_difference);
    
    /* Bounce on floors */
    if (positions[i].y < 15.0) {
      velocities[i].xyz = velocities[i].xyz * 0.75;
      velocities[i].y = -velocities[i].y;
    }

  }
  
}