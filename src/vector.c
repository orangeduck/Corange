/* Vector2 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"

int rawcast(float x) {
  union {
    float f;
    int i;
  } u;

  u.f = x;
  return u.i;
}

float max(float x, float y) {
  return x > y ? x : y;
}

float min(float x, float y) {
  return x < y ? x : y;
}

float clamp(float x, float bottom, float top) {
  x = max(x, bottom);
  x = min(x, top);
  return x;
}

float saturate(float x) {
  x = max(x, 0.0);
  x = min(x, 1.0);
  return x;
}

float lerp(float p1, float p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float smoothstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return lerp( p1, p2, scaled_amount );
}

float smootherstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return lerp( p1, p2, scaled_amount );
}

float cosine_interpolation(float p1, float p2, float amount) {
   float mu2 = (1-cos(amount*3.1415926))/2;
   return (p2*(1-mu2)+p1*mu2);
}

float nearest_neighbor_interpolation(float p1, float p2, float amount) {
  amount = roundf(amount);
  if (amount) { return p2; }
  else { return p1; }
}

float cubic_interpolation(float p1, float p2, float p3, float p4, float amount) {
  
  float amount_sqrd = amount * amount;
  float amount_cubd = amount * amount * amount;
  
  float a1 = p4 - p3 - p2 + p1;
  float a2 = p1 - p2 - a1;
  float a3 = p3 - p1;
  float a4 = p2;
  
  return (a1 * amount_cubd) + (a2 * amount_sqrd) + (a3 * amount) + a4;
}

float binearest_neighbor_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount) {
  
  x_amount = roundf(x_amount);
  y_amount = roundf(y_amount);
  
  if( x_amount && !y_amount ) { return bottom_right; }
  if( !x_amount && y_amount ) { return top_left; }
  if( !x_amount && !y_amount ) { return bottom_left; }
  if( x_amount && y_amount ) { return top_right; }
  
  return 0.0f;
}

float bilinear_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount) {
  
  float left = lerp(bottom_left, top_left, y_amount);
  float right = lerp(bottom_right, top_right, y_amount);
  return lerp(left, right, x_amount);
  
}

float bicosine_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount) {

  float left = cosine_interpolation(bottom_left, top_left, y_amount);
  float right = cosine_interpolation(bottom_right, top_right, y_amount);
  return cosine_interpolation(left, right, x_amount);

}

float bismoothstep_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount) {

  float left = smoothstep(bottom_left, top_left, y_amount);
  float right = smoothstep(bottom_right, top_right, y_amount);
  return smoothstep(left, right, x_amount);

}

float bismootherstep_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount) {

  float left = smootherstep(bottom_left, top_left, y_amount);
  float right = smootherstep(bottom_right, top_right, y_amount);
  return smootherstep(left, right, x_amount);

}

/*
float bicubic_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x, float y) {

}
*/

vector2 v2(float x, float y) {
  vector2 v;
  v.x = x;
  v.y = y;
  return v;
}

vector2 v2_new(float x, float y) {
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

vector2 v2_mul_v2(vector2 v1, vector2 v2) {
  vector2 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.y;
  return v;
}

vector2 v2_pow(vector2 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  return v;
}

vector2 v2_neg(vector2 v) {
  v.x = -v.x;
  v.y = -v.y;
  return v;
}

vector2 v2_abs(vector2 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  return v;
}

vector2 v2_floor(vector2 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  return v;
}

vector2 v2_fmod(vector2 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  return v;
}

vector2 v2_max(vector2 v, float x) {
  v.x = max(v.x, x);
  v.y = max(v.y, x);
  return v;
}

vector2 v2_min(vector2 v, float x) {
  v.x = min(v.x, x);
  v.y = min(v.y, x);
  return v;
}

vector2 v2_clamp(vector2 v, float b, float t) {
  v.x = clamp(v.x, b, t);
  v.y = clamp(v.y, b, t);
  return v;
}

void v2_print(vector2 v) {
  printf("v2(%4.2f,%4.2f)", v.x, v.y);
}

float v2_dot(vector2 v1, vector2 v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

float v2_length_sqrd(vector2 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  return length;
}

float v2_length(vector2 v) {
  return sqrt(v2_length_sqrd(v));
}

float v2_dist_sqrd(vector2 v1, vector2 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y);
}

float v2_dist(vector2 v1, vector2 v2) {
  return sqrt(v2_dist_sqrd(v1, v2));
}

vector2 v2_normalize(vector2 v) {
  float len = v2_length(v);
  return v2_div(v, len);
}

vector2 v2_reflect(vector2 v1, vector2 v2) {
  return v2_sub(v1, v2_mul(v2, 2 * v2_dot(v1, v2)));
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

bool v2_equ(vector2 v1, vector2 v2) {
  if(!(v1.x == v2.x)) { return false; }
  if(!(v1.y == v2.y)) { return false; }
  return true;
}

void v2_to_array(vector2 v, float* out) {
  out[0] = v.x;
  out[1] = v.y;
}

int v2_hash(vector2 v) {
  return abs(rawcast(v.x) ^ rawcast(v.y));
}

int v2_mix_hash(vector2 v) {

  int raw_vx = abs(rawcast(v.x));
  int raw_vy = abs(rawcast(v.y));

  int h1 = raw_vx << 1;
  int h2 = raw_vy << 3;
  int h3 = raw_vx >> 8;
  
  int h4 = raw_vy << 7;
  int h5 = raw_vx >> 12;
  int h6 = raw_vy >> 15;

  int h7 = raw_vx << 2;
  int h8 = raw_vy << 6;
  int h9 = raw_vx >> 2;
  
  int h10 = raw_vy << 9;
  int h11 = raw_vx >> 21;
  int h12 = raw_vy >> 13;
  
  int res1 = h1 ^ h2 ^ h3;
  int res2 = h4 ^ h5 ^ h6;
  int res3 = h7 ^ h8 ^ h9;
  int res4 = h10 ^ h11 ^ h12;
  
  return (res1 * 10252247) ^ (res2 * 70209673) ^ (res3 * 104711) ^ (res4 * 63589);
}

vector2 v2_saturate(vector2 v) {
  v.x = saturate(v.x);
  v.y = saturate(v.y);
  return v;
}

vector2 v2_lerp(vector2 v1, vector2 v2, float amount) {
  vector2 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  return v;
}

vector2 v2_smoothstep(vector2 v1, vector2 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return v2_lerp( v1, v2, scaled_amount );
}

vector2 v2_smootherstep(vector2 v1, vector2 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return v2_lerp( v1, v2, scaled_amount );
}

/* Vector3 */

vector3 v3(float x, float y, float z) {
  vector3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

vector3 v3_new(float x, float y, float z) {
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

vector3 v3_red() {
  return v3(1, 0, 0);
}

vector3 v3_green() {
  return v3(0, 1, 0);
}

vector3 v3_blue() {
  return v3(0, 0, 1);
}

vector3 v3_white() {
  return v3(1, 1, 1);
}

vector3 v3_black() {
  return v3(0, 0, 0);
}

vector3 v3_grey() {
  return v3(0.5, 0.5, 0.5);
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

vector3 v3_mul_v3(vector3 v1, vector3 v2) {
  vector3 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.y;
  v.z = v1.z * v2.z;
  return v;
}

vector3 v3_pow(vector3 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  v.z = pow(v.z, exp);
  return v;
}

vector3 v3_neg(vector3 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  return v;
}

vector3 v3_abs(vector3 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  v.z = fabs(v.z);
  return v;
}

vector3 v3_floor(vector3 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  v.z = floor(v.z);
  return v;
}

vector3 v3_fmod(vector3 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  v.z = fmod(v.z, val);
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

float v3_length_sqrd(vector3 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  return length;
}

float v3_length(vector3 v) {
  return sqrt(v3_length_sqrd(v));
}

float v3_dist_sqrd(vector3 v1, vector3 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y) + 
         (v1.z - v2.z) * (v1.z - v2.z);
}

float v3_dist(vector3 v1, vector3 v2) {
  return sqrt(v3_dist_sqrd(v1, v2));
}

vector3 v3_normalize(vector3 v) {
  float len = v3_length(v);
  if (len == 0.0) {
    return v3_zero();
  } else {
    return v3_div(v, len);
  }
}

vector3 v3_reflect(vector3 v1, vector3 v2) {
  return v3_sub(v1, v3_mul(v2, 2 * v3_dot(v1, v2)));
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

bool v3_equ(vector3 v1, vector3 v2) {
  if(!(v1.x == v2.x)) { return false; }
  if(!(v1.y == v2.y)) { return false; }
  if(!(v1.z == v2.z)) { return false; }
  return true;
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

vector3 v3_saturate(vector3 v) {
  v.x = saturate(v.x);
  v.y = saturate(v.y);
  v.z = saturate(v.z);
  return v;
}

vector3 v3_lerp(vector3 v1, vector3 v2, float amount) {
  vector3 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  v.z = lerp(v1.z, v2.z, amount);
  return v;
}

vector3 v3_smoothstep(vector3 v1, vector3 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return v3_lerp( v1, v2, scaled_amount );
}

vector3 v3_smootherstep(vector3 v1, vector3 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return v3_lerp( v1, v2, scaled_amount );
}

/* Vector4 */

vector4 v4(float x, float y, float z, float w) {
  vector4 v;
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = w;
  return v;
}

vector4 v4_new(float w, float x, float y, float z) {
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
  return v4(1, 1, 1, 1);
}

vector4 v4_red() {
  return v4(1,0,0,1);
}

vector4 v4_green() {
  return v4(0,1,0,1);
}

vector4 v4_blue() {
  return v4(0,0,1,1);
}

vector4 v4_white() {
  return v4(1,1,1,1);
}

vector4 v4_black() {
  return v4(0,0,0,1);
}

vector4 v4_grey() {
  return v4(0.5,0.5,0.5,1);
}

vector4 v4_add(vector4 v1, vector4 v2) {
  vector4 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  v.z = v1.z + v2.z;
  v.w = v1.w + v2.w;
  return v;
}

vector4 v4_sub(vector4 v1, vector4 v2) {
  vector4 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  v.z = v1.z - v2.z;
  v.w = v1.w - v2.w;
  return v;
}

vector4 v4_div(vector4 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  v.z = v.z / fac;
  v.w = v.w / fac;
  return v;
}

vector4 v4_mul(vector4 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  v.z = v.z * fac;
  v.w = v.w * fac;
  return v;
}

vector4 v4_mul_v4(vector4 v1, vector4 v2) {
  vector4 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.x;
  v.z = v1.z * v2.x;
  v.w = v1.w * v2.x;
  return v;
}

vector4 v4_pow(vector4 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  v.z = pow(v.z, exp);
  v.w = pow(v.w, exp);
  return v;
}

vector4 v4_neg(vector4 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  v.w = -v.w;
  return v;
}

vector4 v4_abs(vector4 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  v.z = fabs(v.z);
  v.w = fabs(v.w);
  return v;
}

vector4 v4_floor(vector4 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  v.z = floor(v.z);
  v.w = floor(v.w);
  return v;
}

vector4 v4_fmod(vector4 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  v.z = fmod(v.z, val);
  v.w = fmod(v.w, val);
  return v;  
}

void v4_print(vector4 v) {
  printf("v4(%4.2f, %4.2f, %4.2f, %4.2f)", v.x, v.y, v.z,  v.w);
}

float v4_dot(vector4 v1, vector4 v2) {
  return  (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

float v4_length_sqrd(vector4 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  length += v.w * v.w;
  return length;
}

float v4_length(vector4 v) {
  return sqrt(v4_length_sqrd(v));
}

float v4_dist_sqrd(vector4 v1, vector4 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y) +
         (v1.y - v2.z) * (v1.y - v2.z) +
         (v1.y - v2.w) * (v1.y - v2.w);
}

float v4_dist(vector4 v1, vector4 v2) {
  return sqrt(v4_dist_sqrd(v1, v2));
}

vector4 v4_normalize(vector4 v) {
  float len = v4_length(v);
  if (len == 0.0) {
    return v4_zero();
  } else {
    return v4_div(v, len);
  }
}

vector4 v4_reflect(vector4 v1, vector4 v2) {
  return v4_sub(v1, v4_mul(v2, 2 * v4_dot(v1, v2)));
}

vector4 v4_from_string(char* s) {
  
  char* pEnd;
  double d1, d2, d3, d4;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,&pEnd);  
  d3 = strtod(pEnd,&pEnd);  
  d4 = strtod(pEnd,NULL);  

  vector4 v;
  v.x = d1;
  v.y = d2;
  v.z = d3;
  v.w = d4;
  
  return v;

}

bool v4_equ(vector4 v1, vector4 v2) {
  if(!(v1.x == v2.x)) { return false; }
  if(!(v1.y == v2.y)) { return false; }
  if(!(v1.z == v2.z)) { return false; }
  if(!(v1.w == v2.w)) { return false; }
  return true;
}

void v4_to_array(vector4 v, float* out) {
  
  out[0] = v.x;
  out[1] = v.y;
  out[2] = v.z;
  out[3] = v.w;
  
}

vector3 v4_from_homogeneous(vector4 v) {
  vector3 vec = v3(v.x,v.y,v.z);
  return v3_div(vec, v.w);
};

int v4_hash(vector4 v) {
  return abs( rawcast(v.x) ^ rawcast(v.y) ^ rawcast(v.z) ^ rawcast(v.w) );
}

vector4 v4_saturate(vector4 v) {

  v.x = saturate(v.x);
  v.y = saturate(v.y);
  v.z = saturate(v.z);
  v.w = saturate(v.w);
  
  return v;
}

vector4 v4_lerp(vector4 v1, vector4 v2, float amount) {
  vector4 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  v.z = lerp(v1.z, v2.z, amount);
  v.w = lerp(v1.w, v2.w, amount);
  return v;
}

vector4 v4_smoothstep(vector4 v1, vector4 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return v4_lerp( v1, v2, scaled_amount );
}

vector4 v4_smootherstep(vector4 v1, vector4 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return v4_lerp( v1, v2, scaled_amount );
}

vector4 v4_nearest_neighbor_interpolation(vector4 v1, vector4 v2, float amount) {
  vector4 v;
  v.w = nearest_neighbor_interpolation(v1.w, v2.w, amount);
  v.x = nearest_neighbor_interpolation(v1.x, v2.x, amount);
  v.y = nearest_neighbor_interpolation(v1.y, v2.y, amount);
  v.z = nearest_neighbor_interpolation(v1.z, v2.z, amount);
  return v;
}

vector4 v4_binearest_neighbor_interpolation(vector4 top_left, vector4 top_right, vector4 bottom_left, vector4 bottom_right, float x_amount, float y_amount) {

  vector4 v;
  v.x = binearest_neighbor_interpolation( top_left.x, top_right.x, bottom_left.x, bottom_right.x, x_amount, y_amount );
  v.y = binearest_neighbor_interpolation( top_left.y, top_right.y, bottom_left.y, bottom_right.y, x_amount, y_amount );
  v.z = binearest_neighbor_interpolation( top_left.z, top_right.z, bottom_left.z, bottom_right.z, x_amount, y_amount );
  v.w = binearest_neighbor_interpolation( top_left.w, top_right.w, bottom_left.w, bottom_right.w, x_amount, y_amount );
  
  return v;

}

vector4 v4_bilinear_interpolation(vector4 top_left, vector4 top_right, vector4 bottom_left, vector4 bottom_right, float x_amount, float y_amount) {

  vector4 v;
  v.x = bilinear_interpolation( top_left.x, top_right.x, bottom_left.x, bottom_right.x, x_amount, y_amount );
  v.y = bilinear_interpolation( top_left.y, top_right.y, bottom_left.y, bottom_right.y, x_amount, y_amount );
  v.z = bilinear_interpolation( top_left.z, top_right.z, bottom_left.z, bottom_right.z, x_amount, y_amount );
  v.w = bilinear_interpolation( top_left.w, top_right.w, bottom_left.w, bottom_right.w, x_amount, y_amount );
  
  return v;

}

vector4 v4_quaternion_id() {
  return v4(0, 0, 0, 1);
}

vector4 v4_quaternion_mul(vector4 v1, vector4 v2) {
  
  vector4 quat;
  
  quat.x = (v1.w * v2.y) - (v1.x * v2.z) + (v1.y * v2.w) + (v1.z * v2.x);
  quat.y = (v1.w * v2.z) + (v1.x * v2.y) - (v1.y * v2.x) + (v1.z * v2.w);
  quat.z = (v1.w * v2.x) + (v1.x * v2.w) + (v1.y * v2.z) - (v1.z * v2.y);
  quat.w = (v1.w * v2.w) - (v1.x * v2.x) - (v1.y * v2.y) - (v1.z * v2.z);
  
  return quat;
}

vector4 v4_quaternion_angle_axis(float angle, vector3 axis) {
  
  vector4 quat;
  
  quat.x = axis.x * sinf(angle / 2);
  quat.y = axis.y * sinf(angle / 2);
  quat.z = axis.y * sinf(angle / 2);
  quat.w = cosf(angle / 2);
  
  return quat;
}

vector4 v4_quaternion_roll(float a) {
  return v4( 0, 0, sin(a/2), cos(a/2) );
}

vector4 v4_quaternion_yaw(float a) {
  return v4( 0, sin(a/2), 0, cos(a/2) );
}

vector4 v4_quaternion_pitch(float a) {
  return v4( sin(a/2), 0, 0,  cos(a/2) );
}

vector4 v4_quaternion_euler(float roll, float pitch, float yaw) {
  vector4 q;
  q.x = sin(roll/2)*cos(pitch/2)*cos(yaw/2) - cos(roll/2)*sin(pitch/2)*sin(yaw/2);
  q.y = cos(roll/2)*sin(pitch/2)*cos(yaw/2) + sin(roll/2)*cos(pitch/2)*sin(yaw/2);
  q.z = cos(roll/2)*cos(pitch/2)*sin(yaw/2) - sin(roll/2)*sin(pitch/2)*cos(yaw/2);
  q.w = cos(roll/2)*cos(pitch/2)*cos(yaw/2) + sin(roll/2)*sin(pitch/2)*sin(yaw/2);
  return q;
}

vector4 v4_quaternion_swap_handedness(vector4 q) {
  vector4 quat;
  quat.x = -q.x;
  quat.y = -q.z;
  quat.z = -q.y;
  quat.w =  q.w;
  return quat;
}

