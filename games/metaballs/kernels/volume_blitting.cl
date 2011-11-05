#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

typedef struct {
  float4 r1;
  float4 r2;
  float4 r3;
  float4 r4;
} matrix4x4;

__kernel void blit_point(float4 point, matrix4x4 view_matrix, matrix4x4 proj_matrix,
                          int width, int height, int depth,
                          __write_only image2d_t projection_texture,
                          __write_only image2d_t stencil_texture,
                                      
                          __write_only image3d_t intensity_volume,
                          __write_only image3d_t color_volume,
                          __write_only image3d_t normals_volume) {

  float4 view_pos = (float4)( dot(point, view_matrix.r1), 
                              dot(point, view_matrix.r2),
                              dot(point, view_matrix.r3),
                              dot(point, view_matrix.r4));
                              
  float4 proj_pos = (float4)( dot(point, proj_matrix.r1), 
                              dot(point, proj_matrix.r2),
                              dot(point, proj_matrix.r3),
                              dot(point, proj_matrix.r4));
  
  float3 clip_pos = proj_pos.xyz / proj_pos.w;
  
  float3 screen_pos = (((clip_pos + 1) /  2) * (float3)(width, height, depth));
  int4 lookup = (int4)(screen_pos.x, screen_pos.y, screen_pos.z, 1);
  
  write_imagef(intensity_volume, lookup, (float4)(1,0,0,1));
}