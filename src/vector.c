/* Vector2 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"

int rawcast(float x)
{
  union {
    float f;
    int i;
  } u;

  u.f = x;
  return u.i;
}

vector2 v2(float x, float y) {
  vector2 v;
  v.x = x;
  v.y = y;
  return v;
}

vector2 v2_zero() {
  return v2(0, 0);
}

vector2 v2_one() {
  return v2(1, 1);
}

vector2 v2_add(vector2 v1, vector2 v2) {
  vector2 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  return v;
}

vector2 v2_sub(vector2 v1, vector2 v2) {
  vector2 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  return v;
}

vector2 v2_div(vector2 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  return v;
}

vector2 v2_mul(vector2 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  return v;
}

vector2 v2_neg(vector2 v) {
  v.x = -v.x;
  v.y = -v.y;
  return v;
}

vector2 v2_abs(vector2 v) {
  v.x = abs(v.x);
  v.y = abs(v.y);
  return v;
}

void v2_print(vector2 v) {
  printf("v2(%4.2f,%4.2f)", v.x, v.y);
}

float v2_dot(vector2 v1, vector2 v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

float v2_length(vector2 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  return sqrt(length);
}

vector2 v2_normalize(vector2 v) {
  float len = v2_length(v);
  return v2_div(v, len);
}

vector2 v2_from_string(char* s) {

  char* pEnd;
  double d1, d2;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,NULL);  

  vector2 v;
  v.x = d1;
  v.y = d2;
  
  return v;
}

int v2_equ(vector2 v1, vector2 v2) {
  if(!(v1.x == v2.x)) { return 0; }
  if(!(v1.y == v2.y)) { return 0; }
  return 1;
}

void v2_to_array(vector2 v, float* out) {
  out[0] = v.x;
  out[1] = v.y;
}

int v2_hash(vector2 v) {
  return abs(rawcast(v.x) ^ rawcast(v.y));
}


/* Vector3 */

vector3 v3(float x, float y, float z) {
  vector3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

vector3 v3_zero() {
  return v3(0, 0, 0);
}

vector3 v3_one() {
  return v3(1, 1, 1);
}

vector3 v3_add(vector3 v1, vector3 v2) {
  vector3 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  v.z = v1.z + v2.z;
  return v;
}

vector3 v3_sub(vector3 v1, vector3 v2) {
  vector3 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  v.z = v1.z - v2.z;
  return v;
}

vector3 v3_div(vector3 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  v.z = v.z / fac;
  return v;
}

vector3 v3_mul(vector3 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  v.z = v.z * fac;
  return v;
}

vector3 v3_neg(vector3 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  return v;
}

vector3 v3_abs(vector3 v) {
  v.x = abs(v.x);
  v.y = abs(v.y);
  v.z = abs(v.z);
  return v;
}

void v3_print(vector3 v) {
  printf("v3(%4.2f,%4.2f,%4.2f)", v.x, v.y, v.z);
}

float v3_dot(vector3 v1, vector3 v2) {
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

vector3 v3_cross(vector3 v1, vector3 v2) {
  vector3 v;
  v.x = (v1.y * v2.z) - (v1.z * v2.y);
  v.y = (v1.z * v2.x) - (v1.x * v2.z);
  v.z = (v1.x * v2.y) - (v1.y * v2.x);
  return v;
}

float v3_length(vector3 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  return sqrt(length);
}

vector3 v3_normalize(vector3 v) {
  float len = v3_length(v);
  if (len == 0.0) {
    return v3_zero();
  } else {
    return v3_div(v, len);
  }
}

vector3 v3_from_string(char* s) {
  
  char* pEnd;
  double d1, d2, d3;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,&pEnd);  
  d3 = strtod(pEnd,NULL);  

  vector3 v;
  v.x = d1;
  v.y = d2;
  v.z = d3;
  
  return v;

}

int v3_equ(vector3 v1, vector3 v2) {
  if(!(v1.x == v2.x)) { return 0; }
  if(!(v1.y == v2.y)) { return 0; }
  if(!(v1.z == v2.z)) { return 0; }
  return 1;
}

void v3_to_array(vector3 v, float* out) {
  
  out[0] = v.x;
  out[1] = v.y;
  out[2] = v.z;
  
}

int v3_hash(vector3 v) {
  return abs( rawcast(v.x) ^ rawcast(v.y) ^ rawcast(v.z) );
}

vector4 v3_to_homogeneous(vector3 v){
  return v4(v.x, v.y, v.z, 1.0);
};

/* Vector4 */

vector4 v4(float w, float x, float y, float z) {
  vector4 v;
  v.w = w;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

vector4 v4_zero() {
  return v4(0, 0, 0, 0);
}

vector4 v4_one() {
  return v4(1.0, 1.0, 1.0, 1.0);
}

vector4 v4_add(vector4 v1, vector4 v2) {
  vector4 v;
  v.w = v1.w + v2.w;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  v.z = v1.z + v2.z;
  return v;
}

vector4 v4_sub(vector4 v1, vector4 v2) {
  vector4 v;
  v.w = v1.w - v2.w;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  v.z = v1.z - v2.z;
  return v;
}

vector4 v4_div(vector4 v, float fac) {
  v.w = v.w / fac;
  v.x = v.x / fac;
  v.y = v.y / fac;
  v.z = v.z / fac;
  return v;
}

vector4 v4_mul(vector4 v, float fac) {
  v.w = v.w * fac;
  v.x = v.x * fac;
  v.y = v.y * fac;
  v.z = v.z * fac;
  return v;
}

vector4 v4_neg(vector4 v) {
  v.w = -v.w;
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  return v;
}

vector4 v4_abs(vector4 v) {
  v.w = abs(v.w);
  v.x = abs(v.x);
  v.y = abs(v.y);
  v.z = abs(v.z);
  return v;
}

void v4_print(vector4 v) {
  printf("v4(%4.2f, %4.2f, %4.2f, %4.2f)", v.w, v.x, v.y, v.z);
}

float v4_dot(vector4 v1, vector4 v2) {
  return (v1.w * v2.w) + (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

float v4_length(vector4 v) {
  float length = 0.0;
  length += v.w * v.w;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  return sqrt(length);
}

vector4 v4_normalize(vector4 v) {
  float len = v4_length(v);
  if (len == 0.0) {
    return v4_zero();
  } else {
    return v4_div(v, len);
  }
}

vector4 v4_from_string(char* s) {
  
  char* pEnd;
  double d1, d2, d3, d4;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,&pEnd);  
  d3 = strtod(pEnd,&pEnd);  
  d4 = strtod(pEnd,NULL);  

  vector4 v;
  v.w = d1;
  v.x = d2;
  v.y = d3;
  v.z = d4;
  
  return v;

}

int v4_equ(vector4 v1, vector4 v2) {
  if(!(v1.w == v2.w)) { return 0; }
  if(!(v1.x == v2.x)) { return 0; }
  if(!(v1.y == v2.y)) { return 0; }
  if(!(v1.z == v2.z)) { return 0; }
  return 1;
}

void v4_to_array(vector4 v, float* out) {
  
  out[0] = v.w;
  out[1] = v.x;
  out[2] = v.y;
  out[3] = v.z;
  
}

vector3 v4_from_homogeneous(vector4 v) {
  vector3 vec = v3(v.w,v.x,v.y);
  return v3_div(vec, v.z);
};

int v4_hash(vector4 v) {
  return abs( rawcast(v.w) ^ rawcast(v.x) ^ rawcast(v.y) ^ rawcast(v.z) );
}

vector4 v4_quaternion_id() {
  return v4(1, 0, 0, 0);
}

vector4 v4_quaternion_mul(vector4 v1, vector4 v2) {
  
  vector4 quat;
  
  quat.w = (v1.w * v2.w) - (v1.x * v2.x) - (v1.y * v2.y) - (v1.z * v2.z);
  quat.x = (v1.w * v2.x) + (v1.x * v2.w) + (v1.y * v2.z) - (v1.z * v2.y);
  quat.y = (v1.w * v2.y) - (v1.x * v2.z) + (v1.y * v2.w) + (v1.z * v2.x);
  quat.z = (v1.w * v2.z) + (v1.x * v2.y) - (v1.y * v2.x) + (v1.z * v2.w);
  
  return quat;
}

vector4 v4_quaternion_angle_axis(float angle, vector3 axis) {
  
  vector4 quat;
  
  quat.w = cosf(angle / 2);
  quat.x = axis.x * sinf(angle / 2);
  quat.y = axis.y * sinf(angle / 2);
  quat.z = axis.y * sinf(angle / 2);
  
  return quat;
}

vector4 v4_quaternion_yaw(float a) {
  return v4( cosf(a / 2.0), 0, sinf(a / 2.0), 0 );
}

vector4 v4_quaternion_pitch(float a) {
  return v4( cosf(a / 2.0), sinf(a / 2.0), 0, 0 );
}

vector4 v4_quaternion_roll(float a) {
  return v4( cosf(a / 2.0), 0, 0, sinf(a / 2.0) );
}