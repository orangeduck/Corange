#include "./games/metaballs/kernels/volume.cl"

typedef struct {
  float4 r1;
  float4 r2;
  float4 r3;
  float4 r4;
} matrix_4x4;

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void blit_point(float4 point, float4 color, matrix_4x4 view_matrix, matrix_4x4 proj_matrix,
                          int width, int height, int depth,
                          __write_only image2d_t stencil_texture,
                          volume color_volume,
                          volume normals_volume) {
  
  float4 view_pos = (float4)( dot(point, view_matrix.r1), 
                              dot(point, view_matrix.r2),
                              dot(point, view_matrix.r3),
                              dot(point, view_matrix.r4));
                              
  float4 proj_pos = (float4)( dot(view_pos, proj_matrix.r1), 
                              dot(view_pos, proj_matrix.r2),
                              dot(view_pos, proj_matrix.r3),
                              dot(view_pos, proj_matrix.r4));
  
  float3 clip_pos = proj_pos.xyz / proj_pos.w;
  
  float3 tex_pos = (((clip_pos + 1) /  2) * (float3)(width, height, depth));
  
  int3 size = (int3)(width, height, depth);
  int3 pos = (int3)(tex_pos.x, tex_pos.y, tex_pos.z);
  
  if ((pos.x >= 0) && (pos.x < width) &&
      (pos.y >= 0) && (pos.y < height) &&
      (pos.z >= 0) && (pos.z < depth)) {
    
    volume_set(color_volume, pos, size, (float4)(color.x, color.y, color.z, 1));
    write_imagef(stencil_texture, (int2)(pos.x, pos.y), (float4)(1.0, 1.0, 1.0, 1.0));
  }
}

float __OVERLOADABLE__ lerp(float p1, float p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float2 __OVERLOADABLE__ lerp(float2 p1, float2 p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float3 __OVERLOADABLE__ lerp(float3 p1, float3 p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float4 __OVERLOADABLE__ lerp(float4 p1, float4 p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float metaball(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return lerp( p1, p2, scaled_amount );
}



__kernel void blit_metaball(float4 center, float4 color,
                          matrix_4x4 inv_view_matrix, matrix_4x4 inv_proj_matrix,
                          int3 offset, int3 size, int3 total_size,
                          __write_only image2d_t stencil_texture,
                          volume color_volume,
                          volume normals_volume) {

  /*
    get global id. Work out position within section.
    Transform position into global space.
    Run through metaball equation.
    Add value to existing color value.
    Normals (to be decided).
    Write to stencil texture.
  */
  
  int total_num = size.x * size.y * size.z;
  int id = get_global_id(0);
  if (id >= total_num) {
    return;
  }
  
  int3 coords = offset + (int3)( id % size.x, (id / (size.x)) % size.y, id / (size.x * size.y) );
  
  if ((coords.x < 0) || (coords.x >= total_size.x) ||
      (coords.y < 0) || (coords.y >= total_size.y) ||
      (coords.z < 0) || (coords.z >= total_size.z)) {
    return;
  }
  
  int index = coords.x + coords.y * total_size.x + coords.z * total_size.x * total_size.y;
  
  float3 float_clip_coords = (float3)(coords.x, coords.y, coords.z) / (float3)(total_size.x, total_size.y, total_size.z);
  float4 clip_coords = (float4)(float_clip_coords * 2 - 1, 1);
  float4 view_coords = (float4)( dot(clip_coords, inv_proj_matrix.r1), 
                                 dot(clip_coords, inv_proj_matrix.r2),
                                 dot(clip_coords, inv_proj_matrix.r3),
                                 dot(clip_coords, inv_proj_matrix.r4));
                              
  float4 world_coords = (float4)( dot(view_coords, inv_view_matrix.r1), 
                                  dot(view_coords, inv_view_matrix.r2),
                                  dot(view_coords, inv_view_matrix.r3),
                                  dot(view_coords, inv_view_matrix.r4));
  
  float3 world_pos = world_coords.xyz / world_coords.w;
  float dist = clamp(distance(center.xyz, world_pos) / 2, 0.0, 1.0);
  float intensity = metaball(0, 1, dist);
  
  float4 existing_color = volume_get(color_volume, coords, total_size);
  
  float3 mix_color = existing_color.xyz + color.xyz * intensity;
  float add_intensity = existing_color.w + intensity;
  volume_set(color_volume, coords, total_size, (float4)(mix_color, add_intensity));
  
  float3 actual_normal = normalize(world_pos - center.xyz) * 0.5;
  float3 existing_normal = (volume_get(color_volume, coords, total_size).xyz - 0.5) * 2;
  float3 mix_normal = existing_normal.xyz + actual_normal.xyz * intensity;
  //volume_set(normals_volume, coords, total_size, (float4)((mix_normal + 1) / 2, 1));
  volume_set(normals_volume, coords, total_size, (float4)(1,1,1,1));
  
  write_imagef(stencil_texture, (int2)(coords.x, coords.y), (float4)(1.0, 1.0, 1.0, 1.0));
  
}