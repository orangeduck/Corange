#include "./kernels/volume.cl"
#include "./kernels/matrix.cl"

kernel void clear_volume(global float* data, int3 size) {
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  int3 pos = volume_position(v, id);
  volume_set(v, pos, 0.0);
}

kernel void write_point(global float* data, int3 size, 
                        mat4 view_matrix, mat4 proj_matrix,
                        float4 point) {
  
  volume v = volume_new(data, size);
  
  float4 clip_point = point;
  
  clip_point = mat4_mul_f4(view_matrix, clip_point);
  clip_point = mat4_mul_f4(proj_matrix, clip_point);
  clip_point = clip_point / clip_point.w;
  
  if ((clip_point.x < -1) || (clip_point.x > 1) ||
      (clip_point.y < -1) || (clip_point.y > 1) ||
      (clip_point.z > 1) || (clip_point.z < 0)) {
    return;
  }
  
  clip_point.xy = (clip_point.xy + 1) / 2;
  //clip_point.z = pow(clip_point.z, 100);
  
  int3 pos = (int3)(clip_point.x * size.x, 
                    clip_point.y * size.y, 
                    clip_point.z * size.z);
  
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
  
  float3 clip_size = (float3)(size.x, size.y, size.z);
  float3 clip_pos = (float3)(pos.x, pos.y, pos.z);
  
  clip_pos = clip_pos / clip_size;
  
  clip_pos.xy = (clip_pos.xy * 2) - 1;
  clip_pos.z = pow(clip_pos.z, (1.0/256.0));
  
  float4 clip_pos2 = (float4)(clip_pos, 1);
  float4 view_pos = (float4)( dot(clip_pos2, inv_proj_matrix.r0),
                                 dot(clip_pos2, inv_proj_matrix.r1),
                                 dot(clip_pos2, inv_proj_matrix.r2),
                                 dot(clip_pos2, inv_proj_matrix.r3));
                              
  float4 world_pos = (float4)( dot(view_pos, inv_view_matrix.r0),
                                  dot(view_pos, inv_view_matrix.r1),
                                  dot(view_pos, inv_view_matrix.r2),
                                  dot(view_pos, inv_view_matrix.r3));
  
  world_pos = world_pos / world_pos.w;
  
  const int METABALL_SIZE = 5;
  float total = 0.0;
  
  for(int i = 0; i < num_metaballs; i++) {
    float3 metaball_pos = metaball_positions[i].xyz;
    float dist = distance(world_pos.xyz, metaball_pos) / METABALL_SIZE;
    float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
    
    total += amount;
  }
  
  float existing = volume_get(v, pos);
  
  volume_set(v, pos, existing+total);
  
}

void kernel write_particles(global float* data, int3 size, 
                       mat4 view_matrix, mat4 proj_matrix,
                       constant float4* metaball_positions) {
                            
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  
  float3 particle_pos = metaball_positions[id].xyz;
  float4 clip_point = (float4)(particle_pos, 1);
  
  clip_point = mat4_mul_f4(view_matrix, clip_point);
  clip_point = mat4_mul_f4(proj_matrix, clip_point);
  clip_point = clip_point / clip_point.w;
  
  if ((clip_point.x < -1) || (clip_point.x > 1) ||
      (clip_point.y < -1) || (clip_point.y > 1) ||
      (clip_point.z > 1) || (clip_point.z < 0)) {
    return;
  }
  
  clip_point.xy = (clip_point.xy + 1) / 2;
  //clip_point.z = pow(clip_point.z, 100);
  
  int3 pos = (int3)(clip_point.x * (size.x-1), 
                    clip_point.y * (size.y-1), 
                    clip_point.z * (size.z-1));
  
  volume_set(v, pos, 1.0);
                            
}

const sampler_t smp =  CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void trace_ray(global float* data, int3 size, 
                      write_only image2d_t screen, int2 screen_size) {
  
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  
  int2 pixel = (int2)(id % screen_size.x, id / screen_size.x);
  
  float2 sample_pos = (float2)((float)pixel.x / screen_size.x, (float)pixel.y/ screen_size.y);
  
  for(int i = 0; i < size.z; i++) {
    
    float depth = (float)i / size.z;
    float3 pos = (float3)(sample_pos.x, sample_pos.y, depth);
    float amount = volume_sample_nearest(v, pos);
    
    if (amount > 0.75) {
      write_imagef(screen, pixel, (float4)(depth,0,1,1));
      return;
    }
    
  }
  
  write_imagef(screen, pixel, (float4)(0,0,0,0));
  return;
}