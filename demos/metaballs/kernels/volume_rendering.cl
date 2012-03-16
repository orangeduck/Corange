#include "./kernels/volume.cl"

__kernel void clear_texture(int width, int height, __write_only image2d_t texture, float4 value) {

  int id = get_global_id(0);
  int2 coords = (int2)(id % width, id / width);
  
  write_imagef(texture, coords, value);
  
}

__kernel void clear_volume(int width, int height, int depth, volume v, float4 value) {
  
  int id = get_global_id(0);
  int3 coords = (int3)( id % width, (id / (width)) % height, id / (width * height) );
  
  volume_set(v, coords, (int3)(width, height, depth), value);
}

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void volume_flatten(int width, int height, int depth, float threshold,
							 __write_only image2d_t projection_texture,
               __read_only image2d_t stencil_texture,
               
               volume color_volume,
               volume normals_volume,
               float4 light_position,
               float4 camera_position) {

  int id = get_global_id(0);
  int2 coords = (int2)(id % width, id / width);
  
  float4 stencil = read_imagef(stencil_texture, smp, coords);
  if (stencil.x < 1.0) {
    return;
  }
  
  for( int i = 0; i < depth; i++) {
    
    int3 size = (int3)(width, height, depth);
    int3 vol_coords = (int3)(coords.x, coords.y, i);
    
    float4 color = volume_get(color_volume, vol_coords, size);
    
    if (color.w >= threshold) {
      
      float4 normal = volume_get(normals_volume, vol_coords, size);
      float light = max(dot(normalize((normal.xyz - 0.5) * 2), normalize(light_position.xyz)), 0.0) + 0.25;
      float4 voxel_color = (float4)(color.x * light, color.y * light, color.z * light, 1.0f);
      write_imagef(projection_texture, coords, voxel_color);
      return;
    }
  }
  
}