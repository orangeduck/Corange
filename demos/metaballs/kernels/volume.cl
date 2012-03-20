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
return (int3)( index % v.size.x, 
               (index / (v.size.x)) % v.size.y, 
               index / (v.size.x * v.size.y) );
}


float volume_get(volume v, int3 pos) {
  return v.data[volume_index(v, pos)];
}

void volume_set(volume v, int3 pos, float val) {
  v.data[volume_index(v, pos)] = val;
}

float trilerp(float s0, float s1, 
              float s2, float s3, 
              float s4, float s5,
              float s6, float s7,
              float3 amount) {
              
  return s0;
}

float volume_sample(volume v, float3 pos) {
  
  pos = pos * (v.size-2);
  
  float3 amount = fmod(pos, 1.0);
  int3 p0 = (int3)(floor(pos.x), floor(pos.y), floor(pos.z));
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