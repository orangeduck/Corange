typedef struct {
  float4 r1;
  float4 r2;
  float4 r3;
  float4 r4;
} matrix_4x4;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} byte4;

byte4 byte_color(float r, float g, float b, float a) {
  byte4 ret;
  ret.r = r * 255;
  ret.g = g * 255;
  ret.b = b * 255;
  ret.a = a * 255;
  return ret;
}

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void blit_point(float4 point, float4 color, matrix_4x4 view_matrix, matrix_4x4 proj_matrix,
                          int width, int height, int depth,
                          __write_only image2d_t projection_texture,
                          __write_only image2d_t stencil_texture,
                                      
                          __global byte4* color_volume,
                          __global byte4* normals_volume) {

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
  
  int3 pos = (int3)(tex_pos.x, tex_pos.y, tex_pos.z);
  
  if ((pos.x >= 0) && (pos.x < width) &&
      (pos.y >= 0) && (pos.y < height) &&
      (pos.z >= 0) && (pos.z < depth)) {
  
    color_volume[pos.x + pos.y * width + pos.z * width * height] = byte_color(color.x, color.y, color.z, 1);
    write_imagef(stencil_texture, (int2)(pos.x, pos.y), (float4)(1.0, 1.0, 1.0, 1.0));
  }
}