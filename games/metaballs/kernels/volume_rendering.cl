__kernel void clear_texture(int width, int height, __write_only image2d_t texture) {

  int id = get_global_id(0);
  int2 coords = (int2)(id % width, id / width);
  
  write_imagef(texture, coords, (float4)(0.0, 0.0, 0.0, 0.0));
  
}

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void volume_flatten(int width, int height, int depth, float threshold,
							 __write_only image2d_t projection_texture,
               __read_only image2d_t stencil_texture,
               
               __read_only image3d_t color_volume,
               __read_only image3d_t normals_volume) {

  int id = get_global_id(0);
  int2 coords = (int2)(id % width, id / width);
  
  float4 stencil = read_imagef(stencil_texture, smp, coords);
  if (stencil.x < 1.0) {
    return;
  }
  
  for( int i = 0; i < depth; i++) {
    
    int4 vol_coords = (int4)(coords.x, coords.y, i, 1);
    float4 volume = read_imagef(color_volume, smp, vol_coords);
    
    if (volume.w > 0.0) {
      write_imagef(projection_texture, coords, (float4)(volume.x, volume.y, volume.z, 1.0f));
      return;
    }
  }
  
}