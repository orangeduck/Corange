typedef struct {
  float4 r0;
  float4 r1;
  float4 r2;
  float4 r3;
} mat4;

mat4 mat4_new(float xx, float xy, float xz, float xw,
              float yx, float yy, float yz, float yw,
              float zx, float zy, float zz, float zw,
              float wx, float wy, float wz, float ww) {
  mat4 m;
  m.r0 = (float4)(xx, xy, xz, xw);
  m.r1 = (float4)(yx, yy, yz, zw);
  m.r2 = (float4)(zx, zy, zz, zw);
  m.r3 = (float4)(wx, wy, wz, ww);
  return m;
}

mat4 mat4_id() {
  return mat4_new(1,0,0,0,
                  0,1,0,0,
                  0,0,1,0,
                  0,0,0,1);
}

float4 mat4_mul_f4(mat4 m, float4 v) {
  float4 ret;
  ret.x = dot(v, m.r0);
  ret.y = dot(v, m.r1);
  ret.z = dot(v, m.r2);
  ret.w = dot(v, m.r3);
  return ret;
}

float3 mat4_mul_f3(mat4 m, float3 v) {
  float4 v2 = (float4)(v, 1);
  
  float4 ret;
  ret.x = dot(v2, m.r0);
  ret.y = dot(v2, m.r1);
  ret.z = dot(v2, m.r2);
  ret.w = dot(v2, m.r3);
  
  return ret.xyz / ret.w;
}