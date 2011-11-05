const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | //Natural coordinates
                      CLK_ADDRESS_CLAMP | //Clamp to zeros
                      CLK_FILTER_NEAREST; //Don't interpolate

__kernel void volume_flatten(int width, int height, int depth, float threshold,
							 __write_only image2d_t projection_texture,
               __read_only image2d_t stencil_texture,
               
               __read_only image3d_t intensity_volume,
               __read_only image3d_t color_volume,
               __read_only image3d_t normals_volume) {

  int id = get_global_id(0);
  
  int2 coords = (int2)(id % width, id / width);
  
  bool voxel_found = 0;
  int4 vol_coords;
  float4 intensity;
  float4 color = (float4)(1.0f, 0.0f, 0.0f, 1.0f);
  for( int i = 0; i < depth; i++) {
    
    vol_coords = (int4)(coords.x, coords.y, i, 1);
    intensity = read_imagef(intensity_volume, smp, vol_coords);
    
    if (intensity.x > 0.0) {
      write_imagef(projection_texture, coords, color);
      voxel_found = 1;
      break;
    }
  }
  
  color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
  if (!voxel_found) {
    write_imagef(projection_texture, coords, color);
  }
  
}