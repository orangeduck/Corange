typedef struct {
  float4 r0;
  float4 r1;
  float4 r2;
  float4 r3;
} mat4;

float4 mat4_mul_f4(mat4 m, float4 v) {
  float4 ret;
  ret.x = dot(v, m.r0);
  ret.y = dot(v, m.r1);
  ret.z = dot(v, m.r2);
  ret.w = dot(v, m.r3);
  
  return ret;
}