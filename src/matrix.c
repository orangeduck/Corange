#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

matrix_2x2 m22_id() {
  matrix_2x2 mat;
  
  mat.xx = 1.0f;
  mat.xy = 0.0f;
  mat.yx = 0.0f;
  mat.yy = 1.0f;
  
  return mat;
}

matrix_2x2 m22_zero() {
  matrix_2x2 mat;
  
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  
  return mat;
}

matrix_2x2 m22(float xx, float xy, float yx, float yy) {
  matrix_2x2 mat;
  
  mat.xx = xx;
  mat.xy = xy;
  mat.yx = yx;
  mat.yy = yy;
  
  return mat;
}

matrix_2x2 m22_mul_m22(matrix_2x2 m1, matrix_2x2 m2) {
  matrix_2x2 mat;
  
  mat.xx = m1.xx * m2.xx + m1.xy * m2.yx;
  mat.xy = m1.xx * m2.xy + m1.xy * m2.yy;
  mat.yx = m1.yx * m2.xx + m1.yy * m2.yx;
  mat.yy = m1.yx * m2.xy + m1.yy * m2.yy;
  
  return mat;
}

vector2 m22_mul_v2(matrix_2x2 m, vector2 v) {
  vector2 vec;
  
  vec.x = v.x * m.xx + v.y * m.xy;
  vec.y = v.x * m.yx + v.y * m.yy;
  
  return vec;
}

matrix_2x2 m22_transpose(matrix_2x2 m) {
  matrix_2x2 ret;
  ret.xx = m.xx;
  ret.xy = m.yx;
  ret.yx = m.xy;
  ret.yy = m.yy;
  return ret;
}

float m22_det(matrix_2x2 m) {
  return m.xx * m.yy - m.xy * m.yx;
}

matrix_2x2 m22_inverse(matrix_2x2 m) {
  float det = m22_det(m);
  printf("Det: %f\n", det);
  if (det == 0) {
    printf("Error: Inverting non-singular 2x2 matrix.\n");
    exit(EXIT_FAILURE);
  }
  float fac = 1.0 / det;
  
  matrix_2x2 ret;
  
  ret.xx = fac * m.yy;
  ret.xy = fac * -m.xy;
  ret.yx = fac * -m.yx;
  ret.yy = fac * m.xx;
  
  return ret; 
}

void m22_to_array(matrix_2x2 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.xy;
  out[2] = m.yx;
  out[3] = m.yy;
  
}

void m22_print(matrix_2x2 m) {
  printf("|%4.2f, %4.2f|\n", m.xx, m.xy);
  printf("|%4.2f, %4.2f|\n", m.yx, m.yy);
}

matrix_2x2 m22_rotation(float a) {
  matrix_2x2 m;
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);
  
  return m;
}

/* Matrix 3x3 */

matrix_3x3 m33_zero() {
  matrix_3x3 mat;
  
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  mat.yz = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;
  mat.zz = 0.0f;  
  
  return mat;
}

matrix_3x3 m33_id() {
  matrix_3x3 mat;
  
  mat.xx = 1.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 1.0f;
  mat.yz = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;
  mat.zz = 1.0f;   
  
  return mat;
}

matrix_3x3 m33(float xx, float xy, float xz,
               float yx, float yy, float yz,
               float zx, float zy, float zz) {
  matrix_3x3 mat;
  
  mat.xx = xx;
  mat.xy = xy;
  mat.xz = xz;
  
  mat.yx = yx;
  mat.yy = yy;
  mat.yz = yz;
  
  mat.zx = zx;
  mat.zy = zy;
  mat.zz = zz;   
  
  return mat;         
}

matrix_3x3 m33_mul_m33(matrix_3x3 m1, matrix_3x3 m2) {
  matrix_3x3 mat;

  mat.xx = (m1.xx * m2.xx) + (m1.xy * m2.yx) + (m1.xz * m2.zx);
  mat.xy = (m1.xx * m2.xy) + (m1.xy * m2.yy) + (m1.xz * m2.zy);
  mat.xz = (m1.xx * m2.xz) + (m1.xy * m2.yz) + (m1.xz * m2.zz);

  mat.yx = (m1.yx * m2.xx) + (m1.yy * m2.yx) + (m1.yz * m2.zx);
  mat.yy = (m1.yx * m2.xy) + (m1.yy * m2.yy) + (m1.yz * m2.zy);
  mat.yz = (m1.yx * m2.xz) + (m1.yy * m2.yz) + (m1.yz * m2.zz);

  mat.zx = (m1.zx * m2.xx) + (m1.zy * m2.yx) + (m1.zz * m2.zx);
  mat.zy = (m1.zx * m2.xy) + (m1.zy * m2.yy) + (m1.zz * m2.zy);
  mat.zz = (m1.zx * m2.xz) + (m1.zy * m2.yz) + (m1.zz * m2.zz);
  
  return mat;
  
}

vector3 m33_mul_v3(matrix_3x3 m, vector3 v) {

  vector3 vec;
  
  vec.x = (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z);
  vec.y = (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z);
  vec.z = (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z);
  
  return vec;

}

matrix_3x3 m33_transpose(matrix_3x3 m) {
  matrix_3x3 ret;
  ret.xx = m.xx;
  ret.xy = m.yx;
  ret.xz = m.zx;
  
  ret.yx = m.xy;
  ret.yy = m.yy;
  ret.yz = m.zy;
  
  ret.zx = m.xz;
  ret.zy = m.yz;
  ret.zz = m.zz;
  return ret;
}

float m33_det(matrix_3x3 m) {
  return (m.xx * m.yy * m.zz) + (m.xy * m.yz * m.zx) + (m.xz * m.yx * m.zy) -
         (m.xz * m.yy * m.zx) - (m.xy * m.yx * m.zz) - (m.xx * m.yz * m.zy);
}

matrix_3x3 m33_inverse(matrix_3x3 m) {

  float det = m33_det(m);
  printf("Det: %f\n",det);
  if (det == 0) {
    printf("Error: Inverting non-singular 3x3 matrix.\n");
    exit(EXIT_FAILURE);
  }
  
  float fac = 1.0 / det;
  
  matrix_3x3 ret;
  ret.xx = fac * m22_det(m22(m.yy, m.yz, m.zy, m.zz));
  ret.xy = fac * m22_det(m22(m.xz, m.xy, m.zz, m.zy));
  ret.xz = fac * m22_det(m22(m.xy, m.xz, m.yy, m.yz));
  
  ret.yx = fac * m22_det(m22(m.yz, m.yx, m.zz, m.zx));
  ret.yy = fac * m22_det(m22(m.xx, m.xz, m.zx, m.zz));
  ret.yz = fac * m22_det(m22(m.xz, m.xx, m.yz, m.yx));
  
  ret.zx = fac * m22_det(m22(m.yx, m.yy, m.zx, m.zy));
  ret.zy = fac * m22_det(m22(m.xy, m.xx, m.zy, m.zx));
  ret.zz = fac * m22_det(m22(m.xx, m.xy, m.yx, m.yy));
  
  return ret;
  
}

void m33_to_array(matrix_3x3 m, float* out) {

  out[0] = m.xx;
  out[1] = m.yx;
  out[2] = m.zx;
  
  out[3] = m.xy;
  out[4] = m.yy;
  out[5] = m.zy;
  
  out[6] = m.xz;
  out[7] = m.yz;
  out[8] = m.zz;
  
}

void m33_print(matrix_3x3 m) {
  printf("|%4.2f, %4.2f, %4.2f|\n", m.xx, m.xy, m.xz);
  printf("|%4.2f, %4.2f, %4.2f|\n", m.yx, m.yy, m.yz);
  printf("|%4.2f, %4.2f, %4.2f|\n", m.yx, m.yy, m.zz);
}

matrix_3x3 m33_rotation_x(float a) {

  matrix_3x3 m = m33_id();
  
  m.yy = cos(a);
  m.yz = -sin(a);
  m.zy = sin(a);
  m.zz = cos(a);
  
  return m;
  
}

matrix_3x3 m33_rotation_y(float a) {

  matrix_3x3 m = m33_id();
  
  m.xx = cos(a);
  m.xz = sin(a);
  m.zx = -sin(a);
  m.zz = cos(a);

  return m;
  
}

matrix_3x3 m33_rotation_z(float a) {

  matrix_3x3 m = m33_id();
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);

  return m;
  
}

matrix_3x3 m33_rotation_axis_angle(vector3 v, float angle) {
  
  matrix_3x3 m;

  float c = cos(angle);
  float s = sin(angle);
  float nc = 1 - c;
  
  m.xx = v.x * v.x * nc + c;
  m.xy = v.x * v.y * nc - v.z * s;
  m.xz = v.x * v.z * nc + v.y * s;
  
  m.yx = v.y * v.x * nc + v.z * s;
  m.yy = v.y * v.y * nc + c;
  m.yz = v.y * v.z * nc - v.x * s;
  
  m.zx = v.z * v.x * nc - v.y * s;
  m.zy = v.z * v.y * nc + v.x * s;
  m.zz = v.z * v.z * nc + c;
  
  return m;
}

/* Matrix 4x4 */

matrix_4x4 m44_zero() {
  matrix_4x4 mat;
  
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  mat.xw = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  mat.yz = 0.0f;
  mat.yw = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;  
  mat.zz = 0.0f;
  mat.zw = 0.0f;
  
  mat.wx = 0.0f;
  mat.wy = 0.0f;
  mat.wz = 0.0f;
  mat.ww = 0.0f;
  
  return mat;
}

matrix_4x4 m44_id(){
  
  matrix_4x4 mat = m44_zero();
  
  mat.xx = 1.0f;
  mat.yy = 1.0f;
  mat.zz = 1.0f;
  mat.ww = 1.0f;
  
  
  return mat;
}

matrix_4x4 m44(float xx, float xy, float xz, float xw,
               float yx, float yy, float yz, float yw,
               float zx, float zy, float zz, float zw,
               float wx, float wy, float wz, float ww) {
         
  matrix_4x4 mat;
  
  mat.xx = xx;
  mat.xy = xy;
  mat.xz = xz;
  mat.xw = xw;
  
  mat.yx = yx;
  mat.yy = yy;
  mat.yz = yz;
  mat.yw = yw;
  
  mat.zx = zx;
  mat.zy = zy;  
  mat.zz = zz;
  mat.zw = zw;
  
  mat.wx = wx;
  mat.wy = wy;
  mat.wz = wz;
  mat.ww = ww;
  
  return mat;         
}

matrix_4x4 m44_transpose(matrix_4x4 m) {
  matrix_4x4 mat;
  
  mat.xx = m.xx;
  mat.xy = m.yx;
  mat.xz = m.zx;
  mat.xw = m.wx;
  
  mat.yx = m.xy;
  mat.yy = m.yy;
  mat.yz = m.zy;
  mat.yw = m.wy;
  
  mat.zx = m.xz;
  mat.zy = m.yz;  
  mat.zz = m.zz;
  mat.zw = m.wz;
  
  mat.wx = m.xw;
  mat.wy = m.yw;
  mat.wz = m.zw;
  mat.ww = m.ww;
  
  return mat;
}

matrix_4x4 m33_to_m44(matrix_3x3 m) {

  matrix_4x4 mat;
  
  mat.xx = m.xx;
  mat.xy = m.xy;
  mat.xy = m.xz;
  mat.xw = 0.0f;
  
  mat.yx = m.yx;
  mat.yy = m.yy;
  mat.yz = m.yz;
  mat.yw = 0.0f;
  
  mat.zx = m.zx;
  mat.zy = m.zy;
  mat.zz = m.zz;
  mat.zw = 0.0f;
  
  mat.ww = 0.0f;
  mat.wx = 0.0f;
  mat.wy = 0.0f;  
  mat.wz = 1.0f;
  
  return mat;
}

matrix_4x4 m44_mul_m44(matrix_4x4 m1, matrix_4x4 m2) {

  matrix_4x4 mat;

  mat.xx = (m1.xx * m2.xx) + (m1.xy * m2.yx) + (m1.xz * m2.zx) + (m1.xw * m2.wx);
  mat.xy = (m1.xx * m2.xy) + (m1.xy * m2.yy) + (m1.xz * m2.zy) + (m1.xw * m2.wy);
  mat.xz = (m1.xx * m2.xz) + (m1.xy * m2.yz) + (m1.xz * m2.zz) + (m1.xw * m2.wz);
  mat.xw = (m1.xx * m2.xw) + (m1.xy * m2.yw) + (m1.xz * m2.zw) + (m1.xw * m2.ww);
  
  mat.yx = (m1.yx * m2.xx) + (m1.yy * m2.yx) + (m1.yz * m2.zx) + (m1.yw * m2.wx);
  mat.yy = (m1.yx * m2.xy) + (m1.yy * m2.yy) + (m1.yz * m2.zy) + (m1.yw * m2.wy);
  mat.yz = (m1.yx * m2.xz) + (m1.yy * m2.yz) + (m1.yz * m2.zz) + (m1.yw * m2.wz);
  mat.yw = (m1.yx * m2.xw) + (m1.yy * m2.yw) + (m1.yz * m2.zw) + (m1.yw * m2.ww);
 
  mat.zx = (m1.zx * m2.xx) + (m1.zy * m2.yx) + (m1.zz * m2.zx) + (m1.zw * m2.wx);
  mat.zy = (m1.zx * m2.xy) + (m1.zy * m2.yy) + (m1.zz * m2.zy) + (m1.zw * m2.wy);
  mat.zz = (m1.zx * m2.xz) + (m1.zy * m2.yz) + (m1.zz * m2.zz) + (m1.zw * m2.wz);
  mat.zw = (m1.zx * m2.xw) + (m1.zy * m2.yw) + (m1.zz * m2.zw) + (m1.zw * m2.ww);
  
  mat.wx = (m1.wx * m2.xx) + (m1.wy * m2.yx) + (m1.wz * m2.zx) + (m1.ww * m2.wx);
  mat.wy = (m1.wx * m2.xy) + (m1.wy * m2.yy) + (m1.wz * m2.zy) + (m1.ww * m2.wy);
  mat.wz = (m1.wx * m2.xz) + (m1.wy * m2.yz) + (m1.wz * m2.zz) + (m1.ww * m2.wz);
  mat.ww = (m1.wx * m2.xw) + (m1.wy * m2.yw) + (m1.wz * m2.zw) + (m1.ww * m2.ww);
  
  return mat;
  
}

vector4 m44_mul_v4(matrix_4x4 m, vector4 v) {
  
  vector4 vec;
  
  vec.x = (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z) + (m.xw * v.w);
  vec.y = (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z) + (m.yw * v.w);
  vec.z = (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z) + (m.zw * v.w);
  vec.w = (m.wx * v.x) + (m.wy * v.y) + (m.wz * v.z) + (m.ww * v.w);
  
  return vec;
}

matrix_3x3 m44_to_m33(matrix_4x4 m) {

  matrix_3x3 mat;
  
  mat.xx = m.xx;
  mat.xy = m.xy;
  mat.xz = m.xz;
  
  mat.yx = m.yx;
  mat.yy = m.yy;
  mat.yz = m.yz;
  
  mat.zx = m.zx;
  mat.zy = m.zy;
  mat.zz = m.zz;
  
  return mat;
  
}

float m44_det(matrix_4x4 m) {
  
  float cofact_xx =  m33_det(m33(m.yy, m.yz, m.yw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  float cofact_xy = -m33_det(m33(m.yx, m.yz, m.yw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  float cofact_xz =  m33_det(m33(m.yx, m.yy, m.yw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  float cofact_xw = -m33_det(m33(m.yx, m.yy, m.yz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  return (cofact_xx * m.xx) + (cofact_xy * m.xy) + (cofact_xz * m.xz) + (cofact_xw * m.xw);
}

matrix_4x4 m44_inverse(matrix_4x4 m) {
    
  float det = m44_det(m);
  
  if (det == 0) {
    printf("Error: Inverting non-singular 4x4 matrix.\n");
    exit(EXIT_FAILURE);
  }
  
  float fac = 1.0 / det;
  
  matrix_4x4 ret;
  ret.xx = fac *  m33_det(m33(m.yy, m.yz, m.yw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  ret.xy = fac * -m33_det(m33(m.yx, m.yz, m.yw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  ret.xz = fac *  m33_det(m33(m.yx, m.yy, m.yw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  ret.xw = fac * -m33_det(m33(m.yx, m.yy, m.yz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  ret.yx = fac * -m33_det(m33(m.xy, m.xz, m.xw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  ret.yy = fac *  m33_det(m33(m.xx, m.xz, m.xw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  ret.yz = fac * -m33_det(m33(m.xx, m.xy, m.xw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  ret.yw = fac *  m33_det(m33(m.xx, m.xy, m.xz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  ret.zx = fac *  m33_det(m33(m.xy, m.xz, m.xw, m.yy, m.yz, m.yw, m.wy, m.wz, m.ww));
  ret.zy = fac * -m33_det(m33(m.xx, m.xz, m.xw, m.yx, m.yz, m.yw, m.wx, m.wz, m.ww));
  ret.zz = fac *  m33_det(m33(m.xx, m.xy, m.xw, m.yx, m.yy, m.yw, m.wx, m.wy, m.ww));
  ret.zw = fac * -m33_det(m33(m.xx, m.xy, m.xz, m.yx, m.yy, m.yz, m.wx, m.wy, m.wz));
  
  ret.wx = fac * -m33_det(m33(m.xy, m.xz, m.xw, m.yy, m.yz, m.yw, m.zy, m.zz, m.zw));
  ret.wy = fac *  m33_det(m33(m.xx, m.xz, m.xw, m.yx, m.yz, m.yw, m.zx, m.zz, m.zw));
  ret.wz = fac * -m33_det(m33(m.xx, m.xy, m.xw, m.yx, m.yy, m.yw, m.zx, m.zy, m.zw));
  ret.ww = fac *  m33_det(m33(m.xx, m.xy, m.xz, m.yx, m.yy, m.yz, m.zx, m.zy, m.zz));
  
  ret = m44_transpose(ret);
  
  return ret;
}

void m44_to_array(matrix_4x4 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.yx;
  out[2] = m.zx;
  out[3] = m.wx;

  out[4]  = m.xy;
  out[5]  = m.yy;
  out[6] = m.zy;
  out[7] = m.wy;
  
  out[8] = m.xz;
  out[9] = m.yz;
  out[10] = m.zz;
  out[11] = m.wz;
  
  out[12] = m.xw;
  out[13] = m.yw;
  out[14] = m.zw;
  out[15] = m.ww;
  
}

void m44_to_array_trans(matrix_4x4 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.xy;
  out[2] = m.xz;
  out[3] = m.xw;

  out[4]  = m.yx;
  out[5]  = m.yy;
  out[6] = m.yz;
  out[7] = m.yw;
  
  out[8] = m.zx;
  out[9] = m.zy;
  out[10] = m.zz;
  out[11] = m.zw;
  
  out[12] = m.wx;
  out[13] = m.wy;
  out[14] = m.wz;
  out[15] = m.ww;
  
}

void m44_print(matrix_4x4 m) {

  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.xx, m.xy, m.xz, m.xw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.yx, m.yy, m.yz, m.yw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.zx, m.zy, m.zz, m.zw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.wx, m.wy, m.wz, m.ww);
  
}

matrix_4x4 m44_view_look_at(vector3 position, vector3 target, vector3 up) {

  /*
  Taken From:
  
  http://www.opengl.org/wiki/GluLookAt_code

  */
  
  vector3 zaxis = v3_normalize( v3_sub(target, position) );
  vector3 xaxis = v3_normalize( v3_cross(up, zaxis) );
  vector3 yaxis = v3_cross(zaxis, xaxis);

  matrix_4x4 view_matrix = m44_id();
  view_matrix.xx = xaxis.x;
  view_matrix.xy = xaxis.y;
  view_matrix.xz = xaxis.z;
  
  view_matrix.yx = yaxis.x;
  view_matrix.yy = yaxis.y;
  view_matrix.yz = yaxis.z;
  
  view_matrix.zx = -zaxis.x;
  view_matrix.zy = -zaxis.y;
  view_matrix.zz = -zaxis.z;
  
  // Also unsure of this.
  view_matrix = m44_mul_m44(view_matrix, m44_translation(v3_neg(position)) );
  
  return view_matrix;
};

matrix_4x4 m44_perspective(float fov, float near_clip, float far_clip, float ratio) {
  
  /*  
    http://www.opengl.org/wiki/GluPerspective_code
    
  */
  
  float right, left, bottom, top;
  
  right = -(near_clip * tanf(fov));
  left = -right;
  
  top = ratio * near_clip * tanf(fov);
  bottom = -top;
  
  matrix_4x4 proj_matrix = m44_zero();
  proj_matrix.xx = (2.0 * near_clip) / (right - left);
  proj_matrix.yy = (2.0 * near_clip) / (top - bottom);
  proj_matrix.xz = (right + left) / (right - left);
  proj_matrix.yz = (top + bottom) / (top - bottom);
  proj_matrix.zz = (-far_clip - near_clip) / (far_clip - near_clip);
  proj_matrix.wz = -1.0;
  proj_matrix.zw = ( -(2.0 * near_clip) * far_clip) / (far_clip - near_clip);
  
  return proj_matrix;
}

matrix_4x4 m44_orthographic(float left, float right, float bottom, float top, float near, float far) {

  matrix_4x4 m = m44_id();
  
  m.xx = 2 / (right - left);
  m.yy = 2 / (top - bottom);
  m.zz = -2 / (far - near);
  
  m.xw = - (right + left) / (right - left);
  m.yw = - (top + bottom) / (top - bottom);
  m.zw = - (far + near) / (far - near);
  
  return m;
}

matrix_4x4 m44_translation(vector3 v) {

  matrix_4x4 m = m44_id();
  m.xw = v.x;
  m.yw = v.y;
  m.zw = v.z;

  return m;
  
};

matrix_4x4 m44_scale(vector3 v) {

  matrix_4x4 m = m44_id();
  m.xx = v.x;
  m.yy = v.y;
  m.zz = v.z;

  return m;
};

matrix_4x4 m44_rotation_x(float a) {

  matrix_4x4 m = m44_id();
  
  m.yy = cos(a);
  m.yz = -sin(a);
  m.zy = sin(a);
  m.zz = cos(a);
  
  return m;
  
};

matrix_4x4 m44_rotation_y(float a) {

  matrix_4x4 m = m44_id();
  
  m.xx = cos(a);
  m.xz = sin(a);
  m.zx = -sin(a);
  m.zz = cos(a);

  return m;
  
};

matrix_4x4 m44_rotation_z(float a) {

  matrix_4x4 m = m44_id();
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);

  return m;
  
};

matrix_4x4 m44_rotation(vector3 v) {

  matrix_4x4 m = m44_id();
  m = m44_mul_m44(m , m44_rotation_x(v.x) );
  m = m44_mul_m44(m , m44_rotation_y(v.y) );
  m = m44_mul_m44(m , m44_rotation_z(v.z) );
  
  return m;
  
};

matrix_4x4 m44_rotation_quaternion(vector4 q) {

  q = v4_normalize(q);
  
  matrix_4x4 m = m44_id();
  
  m.xx = 1.0 - 2 * q.y * q.y - 2 * q.z * q.z;
  m.xy =       2 * q.x * q.y - 2 * q.w * q.z;
  m.xz =       2 * q.x * q.z + 2 * q.w * q.y;
  
  m.yx =       2 * q.x * q.y + 2 * q.w * q.z;
  m.yy = 1.0 - 2 * q.x * q.x - 2 * q.z * q.z;
  m.yz =       2 * q.y * q.z + 2 * q.w * q.x;
  
  m.zx =       2 * q.x * q.z - 2 * q.w * q.y;
  m.zy =       2 * q.y * q.z - 2 * q.w * q.x;
  m.zz = 1.0 - 2 * q.x * q.x - 2 * q.y * q.y;
  
  return m;
};

matrix_4x4 m44_world(vector3 position, vector3 scale, vector4 rotation) {
  
  matrix_4x4 pos_m, sca_m, rot_m, result;
  
  pos_m = m44_translation(position);
  rot_m = m44_rotation_quaternion(rotation);
  sca_m = m44_scale(scale);
  
  result = m44_id();
  result = m44_mul_m44( result, pos_m );
  result = m44_mul_m44( result, sca_m );
  result = m44_mul_m44( result, rot_m );
  
  return result;
  
}