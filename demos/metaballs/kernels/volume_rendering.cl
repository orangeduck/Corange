#include "./kernels/volume.cl"
#include "./kernels/matrix.cl"

kernel void write_point(global float* data, int3 size, 
                        mat4 view_matrix, mat4 proj_matrix,
                        float4 point) {
  
  volume v = volume_new(data, size);
  
  point = mat4_mul_f4(view_matrix, point);
  point = mat4_mul_f4(proj_matrix, point);
  point = point / point.w;
  
  point = (point + 1) / 2;
  
  point = point * (size-1);
  
  int3 pos = (int3)(point.x, point.y, point.z);
  
  volume_set(v, pos, 1.0);
}

float smoothstepmap(float val) {
  return val*val*(3 - 2*val);
}

kernel void write_metaballs(global float* data, int3 size, 
                            mat4 inv_view_matrix, mat4 inv_proj_matrix,
                            constant float4* metaball_positions,
                            int num_metaballs) {
  
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  int3 pos = volume_position(v, id);
  
  float4 world_pos = (float4)(pos.x, pos.y, pos.z, 1);
  world_pos = ((world_pos / size) * 2) - 1;
  world_pos.w = 1.0;
  world_pos = mat4_mul_f4(inv_proj_matrix, world_pos);
  world_pos = mat4_mul_f4(inv_view_matrix, world_pos);
  
  world_pos = world_pos / world_pos.w;
  
  const int METABALL_SIZE = 5;
  float total = 0.0;
  
  for(int i = 0; i < num_metaballs; i++) {
    float3 metaball_pos = metaball_positions[i].xyz;
    float dist = distance(world_pos.xyz, metaball_pos) / METABALL_SIZE;
    float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
    
    total += amount;
  }
  
  volume_set(v, pos, total);

}

const sampler_t smp =  CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void trace_ray(global float* data, int3 size, 
                      write_only image2d_t screen, int2 screen_size) {
  
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  
  int2 pixel = (int2)(id % screen_size.x, id / screen_size.x);
  
  float2 sample_pos = (float2)(pixel.x, pixel.y) / (screen_size-1);
  
  float step = 1.0;
  float depth = 0.0;
  while(depth < size.z) {
    
    float3 sample =  (float3)(sample_pos.x, sample_pos.y, depth);
    float amount = volume_sample(v, sample);
    
    if (amount > 0.5) {
      //write_imagef(screen, pixel, (float4)(depth, depth, depth, depth));
      write_imagef(screen, pixel, (float4)(1, 0, 0, 1));
      return;
    }
    
    depth += step;
  }
  
  write_imagef(screen, pixel, (float4)(1,1,1,1));
  return;
}