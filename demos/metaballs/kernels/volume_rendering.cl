#include "./kernels/volume.cl"
#include "./kernels/matrix.cl"

kernel void clear_volume(global float* data, int3 size) {
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  int3 pos = volume_position(v, id);
  volume_set(v, pos, 0.0);
}

kernel void clear_texture(write_only image2d_t texture, 
                          int2 tex_size,
                          float4 color) {
  int id = get_global_id(0);
  int2 pixel = (int2)(id % tex_size.x, id / tex_size.x);
  write_imagef(texture, pixel, color);
}

kernel void write_point(global float* data, int3 size, 
                        write_only image2d_t stencil,
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
  
  clip_point = (clip_point + 1) / 2;
  clip_point.z = pow(clip_point.z, 256.0f);
  
  int3 pos = (int3)(clip_point.x * size.x, 
                    clip_point.y * size.y, 
                    clip_point.z * size.z);
  
  volume_set(v, pos, 1.0);
}

float smoothstepmap(float val) {
  return val*val*(3 - 2*val);
}

constant int METABALL_SIZE = 5;

kernel void write_metaballs(global float* data, int3 size, 
                            write_only image2d_t stencil,
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
  clip_pos.z = pow(clip_pos.z, (1.0f/256.0f));
  
  float3 world_pos = clip_pos;
  world_pos = mat4_mul_f3(inv_proj_matrix, world_pos);
  world_pos = mat4_mul_f3(inv_view_matrix, world_pos);
  
  float total = 0.0;
  
  for(int i = 0; i < num_metaballs; i++) {
    float3 metaball_pos = metaball_positions[i].xyz;
    float dist = distance(world_pos, metaball_pos) / METABALL_SIZE;
    float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
    
    total += amount;
  }
  
  volume_set(v, pos, total);
  
  if (total > 0.0) {
    write_imagef(stencil, pos.xy, (float4)(1,1,1,1));
  }
  
}

void kernel write_particles(global float* data, int3 size,
                            write_only image2d_t stencil,
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
  
  clip_point = (clip_point + 1) / 2;
  clip_point.z = pow(clip_point.z, 256.0f);
  
  int3 pos = (int3)(clip_point.x * (size.x-1), 
                    clip_point.y * (size.y-1), 
                    clip_point.z * (size.z-1));
  
  volume_set(v, pos, 1.0);
                            
}

const sampler_t smp =  CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void generate_depth(global float* data, int3 size, 
                           read_only image2d_t stencil,
                           write_only image2d_t depth, 
                           int2 screen_size) {
  
  volume v = volume_new(data, size);
  
  int id = get_global_id(0);
  int2 pixel = (int2)(id % screen_size.x, id / screen_size.x);
  
  float2 sample_pos = (float2)((float)pixel.x / screen_size.x, (float)pixel.y/ screen_size.y);
  
  float4 valid = read_imagef(stencil, smp, sample_pos);
  if (valid.x == 0.0) {
    write_imagef(depth, pixel, (float4)(1,0,0,1));
    return;
  }
  
  float curr = 0;
  float prev = 0;
  for(int z = 0; z < size.z; z++) {
    
    float2 pos = (float2)(sample_pos.x, sample_pos.y);
    curr = volume_sample_bilinear(v, pos, z);
    
    if (curr > 0.75) {
      float amount = (0.75 - prev) / (curr - prev);
      float zdepth = mix((float)(z-1)/size.z, (float)z/size.z, amount);
      zdepth = pow(zdepth, 1.0f/256.0f);
      write_imagef(depth, pixel, (float4)(zdepth,0,0,1));
      return;
    }
    
    prev = curr;
  }
  
  write_imagef(depth, pixel, (float4)(1,0,0,1));
  return;
}

const sampler_t smpcom =  CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

void kernel generate_positions(read_only image2d_t depth,
                               write_only image2d_t positions,
                               mat4 inv_view_matrix, mat4 inv_proj_matrix,
                               int2 screen_size) {
  int id = get_global_id(0);
  int2 pixel = (int2)(id % screen_size.x, id / screen_size.x);
  float pixel_depth = read_imagef(depth, smpcom, pixel).x;
  
  float3 clip_pos = (float3)(pixel.x, pixel.y, pixel_depth);
  float3 clip_size = (float3)(screen_size.x, screen_size.y, 1);
  
  clip_pos = clip_pos / clip_size;
  clip_pos.xy = (clip_pos.xy * 2) - 1;
  
  float3 world_pos = clip_pos;
  world_pos = mat4_mul_f3(inv_proj_matrix, world_pos);
  world_pos = mat4_mul_f3(inv_view_matrix, world_pos);
  
  write_imagef(positions, pixel, (float4)(world_pos, 1));
}

void kernel generate_normals(read_only image2d_t depth,
                             read_only image2d_t positions,
                             write_only image2d_t normals,
                             int2 screen_size,
                             constant float4* metaball_positions,
                             int num_metaballs) {                       
  int id = get_global_id(0);
  int2 pixel = (int2)(id % screen_size.x, id / screen_size.x);
  
  float pixdepth = read_imagef(depth, smpcom, pixel).x;
  
  if (pixdepth == 1.0) {
    write_imagef(normals, pixel, (float4)(0,0,0,0));
    return;
  }
  
  float3 pos = read_imagef(positions, smpcom, pixel).xyz;
  
  //float3 pos_x = read_imagef(positions, smpcom, pixel + (int2)(1,0)).xyz;
  //float3 pos_y = read_imagef(positions, smpcom, pixel + (int2)(0,1)).xyz;
  //float3 normal = normalize(cross(pos_x - pos, pos_y - pos));
  
  float3 normal = (float3)(0,0,0);
  for(int i = 0; i < num_metaballs; i++) {
  
    float dist = distance(pos, metaball_positions[i].xyz) / METABALL_SIZE;
    float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
  
    normal += (pos - metaball_positions[i].xyz) * amount;
  }
  normal = normalize(normal);
  
  write_imagef(normals, pixel, (float4)(normal, 1));
}

