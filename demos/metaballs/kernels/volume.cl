typedef struct {
  global float* data;
  int3 size;
} volume;

volume volume_new(global float* data, int3 size) {
  volume v;
  v.data = data;
  v.size = size;
  return v;
}

int volume_index(volume v, int3 pos) {
  return pos.x + pos.y * v.size.x + pos.z * v.size.x * v.size.y;
}

int3 volume_position(volume v, int index) {
return (int3)(index % v.size.x, 
             (index / (v.size.x)) % v.size.y, 
              index / (v.size.x * v.size.y) );
}


float volume_get(volume v, int3 pos) {
  return v.data[volume_index(v, pos)];
}

void volume_set(volume v, int3 pos, float val) {
  v.data[volume_index(v, pos)] = val;
}

/*
bottom

s3--s6
|   |
s0--s1

top

s5--s7
|   |
s2--s4
*/


float trilerp(float s0, float s1, 
              float s2, float s3, 
              float s4, float s5,
              float s6, float s7,
              float3 amount) {
  
  float s01 = mix(s0, s1, amount.x);
  float s36 = mix(s3, s6, amount.x);
  float s24 = mix(s2, s4, amount.x);
  float s57 = mix(s5, s7, amount.x);
  
  float s0136 = mix(s01, s36, amount.z);
  float s2457 = mix(s24, s57, amount.z);
  
  float sall = mix(s0136, s2457, amount.y);
  
  return sall;
}

/*

s2--s3
|    |
s0--s1

*/

float bilerp(float s0, float s1,
             float s2, float s3,
             float2 amount) {
             
  float s01 = mix(s0, s1, amount.x);
  float s23 = mix(s2, s3, amount.x);
  
  return mix(s01, s23, amount.y);
}

float volume_sample_nearest(volume v, float3 pos) {
  
  int3 spos = (int3)(pos.x * (v.size.x-1), 
                     pos.y * (v.size.y-1),
                     pos.z * (v.size.z-1));
  return volume_get(v, spos);
  
}

float volume_sample_trilinear(volume v, float3 pos) {
  
  float3 fpos = (float3)(pos.x * (v.size.x-1),
                         pos.y * (v.size.y-1),
                         pos.z * (v.size.z-1));
  
  int3 spos = (int3)(pos.x * (v.size.x-1), 
                     pos.y * (v.size.y-1),
                     pos.z * (v.size.z-1));
  
  float3 amount = (float3)(fmod(fpos.x, 1.0f),
                           fmod(fpos.y, 1.0f),
                           fmod(fpos.z, 1.0f));
  
  int3 p0 = spos;
  int3 p1 = p0 + (int3)(1,0,0);
  int3 p2 = p0 + (int3)(0,1,0);
  int3 p3 = p0 + (int3)(0,0,1);
  int3 p4 = p0 + (int3)(1,1,0);
  int3 p5 = p0 + (int3)(0,1,1);
  int3 p6 = p0 + (int3)(1,0,1);
  int3 p7 = p0 + (int3)(1,1,1);
  
  float s0 = volume_get(v, p0);
  float s1 = volume_get(v, p1);
  float s2 = volume_get(v, p2);
  float s3 = volume_get(v, p3);
  float s4 = volume_get(v, p4);
  float s5 = volume_get(v, p5);
  float s6 = volume_get(v, p6);
  float s7 = volume_get(v, p7);
  
  return trilerp(s0, s1, s2, s3, s4, s5, s6, s7, amount);
}

float volume_sample_bilinear(volume v, float2 pos, int depth) {

  float2 fpos = (float2)(pos.x * (v.size.x-1),
                         pos.y * (v.size.y-1));
  
  int3 spos = (int3)(pos.x * (v.size.x-1), 
                     pos.y * (v.size.y-1),
                     depth);
  
  float2 amount = (float2)(fmod(fpos.x, 1.0f),
                           fmod(fpos.y, 1.0f));
  
  int3 p0 = spos;
  int3 p1 = p0 + (int3)(1,0,0);
  int3 p2 = p0 + (int3)(0,1,0);
  int3 p3 = p0 + (int3)(1,1,0);
  
  float s0 = volume_get(v, p0);
  float s1 = volume_get(v, p1);
  float s2 = volume_get(v, p2);
  float s3 = volume_get(v, p3);
  
  return bilerp(s0, s1, s2, s3, amount);
}