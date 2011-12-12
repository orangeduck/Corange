typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} byte4;

typedef __global byte4* volume;
typedef __global byte4* texture;

float4 volume_get(volume v, int3 coords, int3 size) {
  int i = coords.x + coords.y * size.x + coords.z * size.x * size.y;
  byte4 ret = v[i];
  return (float4)((float)ret.r / 255, (float)ret.g / 255, (float)ret.b / 255, (float)ret.a / 255);
}

void volume_set(volume v, int3 coords, int3 size, float4 color) {
  int i = coords.x + coords.y * size.x + coords.z * size.x * size.y;
  byte4 set;
  set.r = clamp((int)(color.x * 255), 0, 255);
  set.g = clamp((int)(color.y * 255), 0, 255);
  set.b = clamp((int)(color.z * 255), 0, 255);
  set.a = clamp((int)(color.w * 255), 0, 255);
  v[i] = set;
}

float4 texture_get(texture t, int2 coords, int2 size) {
  int i = coords.x + coords.y * size.x;
  byte4 ret = t[i];
  return (float4)((float)ret.r / 255, (float)ret.g / 255, (float)ret.b / 255, (float)ret.a / 255);
}

void texture_set(texture t, int2 coords, int2 size, float4 color) {
  int i = coords.x + coords.y * size.x;
  byte4 set;
  set.r = clamp((int)(color.x * 255), 0, 255);
  set.g = clamp((int)(color.y * 255), 0, 255);
  set.b = clamp((int)(color.z * 255), 0, 255);
  set.a = clamp((int)(color.w * 255), 0, 255);
  t[i] = set;
}