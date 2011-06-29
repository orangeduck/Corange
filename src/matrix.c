#include <math.h>
#include <stdio.h>

#include "matrix.h"

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
};

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

matrix_4x4 m44_zero() {
  matrix_4x4 mat;
  
  mat.ww = 0.0f;
  mat.wx = 0.0f;
  mat.wy = 0.0f;
  mat.wz = 0.0f;
  
  mat.xw = 0.0f;
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yw = 0.0f;
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  mat.yz = 0.0f;
  
  mat.zw = 0.0f;
  mat.zx = 0.0f;
  mat.zy = 0.0f;  
  mat.zz = 0.0f;  
  
  return mat;
};

matrix_4x4 m44_id(){
  matrix_4x4 mat;
  
  mat.ww = 1.0f;
  mat.wx = 0.0f;
  mat.wy = 0.0f;
  mat.wz = 0.0f;
  
  mat.xw = 0.0f;
  mat.xx = 1.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yw = 0.0f;
  mat.yx = 0.0f;
  mat.yy = 1.0f;
  mat.yz = 0.0f;
  
  mat.zw = 0.0f;
  mat.zx = 0.0f;
  mat.zy = 0.0f;  
  mat.zz = 1.0f;  
  
  return mat;
};

matrix_4x4 m44_transpose(matrix_4x4 m) {
  matrix_4x4 mat;
  
  mat.ww = m.ww;
  mat.wx = m.xw;
  mat.wy = m.yw;
  mat.wz = m.zw;
  
  mat.xw = m.wx;
  mat.xx = m.xx;
  mat.xy = m.yx;
  mat.xz = m.zx;
  
  mat.yw = m.wy;
  mat.yx = m.xy;
  mat.yy = m.yy;
  mat.yz = m.zy;
  
  mat.zw = m.xw;
  mat.zx = m.xz;
  mat.zy = m.yz;  
  mat.zz = m.zz;  
  
  return mat;
}

matrix_4x4 m33_to_m44(matrix_3x3 m) {

  matrix_4x4 mat;
  
  mat.ww = m.xx;
  mat.wx = m.xy;
  mat.wy = m.xz;
  mat.wz = 0.0f;
  
  mat.xw = m.yx;
  mat.xx = m.yy;
  mat.xy = m.yz;
  mat.xz = 0.0f;
  
  mat.yw = m.zx;
  mat.yx = m.zy;
  mat.yy = m.zz;
  mat.yz = 0.0f;
  
  mat.zw = 0.0f;
  mat.zx = 0.0f;
  mat.zy = 0.0f;  
  mat.zz = 1.0f;
  
  return mat;
};

matrix_4x4 m44_mul_m44(matrix_4x4 m1, matrix_4x4 m2) {

  matrix_4x4 mat;

  mat.ww = (m1.ww * m2.ww) + (m1.wx * m2.xw) + (m1.wy * m2.yw) + (m1.wz * m2.zw);
  mat.wx = (m1.ww * m2.wx) + (m1.wx * m2.xx) + (m1.wy * m2.yx) + (m1.wz * m2.zx);
  mat.wy = (m1.ww * m2.wy) + (m1.wx * m2.xy) + (m1.wy * m2.yy) + (m1.wz * m2.zy);
  mat.wz = (m1.ww * m2.wz) + (m1.wx * m2.xz) + (m1.wy * m2.yz) + (m1.wz * m2.zz);

  mat.xw = (m1.xw * m2.ww) + (m1.xx * m2.xw) + (m1.xy * m2.yw) + (m1.xz * m2.zw);
  mat.xx = (m1.xw * m2.wx) + (m1.xx * m2.xx) + (m1.xy * m2.yx) + (m1.xz * m2.zx);
  mat.xy = (m1.xw * m2.wy) + (m1.xx * m2.xy) + (m1.xy * m2.yy) + (m1.xz * m2.zy);
  mat.xz = (m1.xw * m2.wz) + (m1.xx * m2.xz) + (m1.xy * m2.yz) + (m1.xz * m2.zz);

  mat.yw = (m1.yw * m2.ww) + (m1.yx * m2.xw) + (m1.yy * m2.yw) + (m1.yz * m2.zw);
  mat.yx = (m1.yw * m2.wx) + (m1.yx * m2.xx) + (m1.yy * m2.yx) + (m1.yz * m2.zx);
  mat.yy = (m1.yw * m2.wy) + (m1.yx * m2.xy) + (m1.yy * m2.yy) + (m1.yz * m2.zy);
  mat.yz = (m1.yw * m2.wz) + (m1.yx * m2.xz) + (m1.yy * m2.yz) + (m1.yz * m2.zz);

  mat.zw = (m1.zw * m2.ww) + (m1.zx * m2.xw) + (m1.zy * m2.yw) + (m1.zz * m2.zw);
  mat.zx = (m1.zw * m2.wx) + (m1.zx * m2.xx) + (m1.zy * m2.yx) + (m1.zz * m2.zx);
  mat.zy = (m1.zw * m2.wy) + (m1.zx * m2.xy) + (m1.zy * m2.yy) + (m1.zz * m2.zy);
  mat.zz = (m1.zw * m2.wz) + (m1.zx * m2.xz) + (m1.zy * m2.yz) + (m1.zz * m2.zz);
  
  return mat;
  
}

vector4 m44_mul_v4(matrix_4x4 m, vector4 v) {
  
  vector4 vec;
  
  vec.w = (m.ww * v.w) + (m.wx * v.x) + (m.wy * v.y) + (m.wz * v.z);
  vec.x = (m.xw * v.w) + (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z);
  vec.y = (m.yw * v.w) + (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z);
  vec.z = (m.zw * v.w) + (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z);
  
  return vec;
}

matrix_3x3 m44_to_m33(matrix_4x4 m) {

  matrix_3x3 mat = m33_zero();
  
  mat.xx = m.ww;
  mat.xy = m.wx;
  mat.xz = m.wy;
  
  mat.yx = m.xw;
  mat.yy = m.xx;
  mat.yz = m.xy;
  
  mat.zx = m.yw;
  mat.zy = m.yx;
  mat.zz = m.yy;
  
  return mat;
  
};


void m44_to_array(matrix_4x4 m, float* out) {

  out[0] = m.ww;
  out[1] = m.xw;
  out[2] = m.yw;
  out[3] = m.zw;
  
  out[4] = m.wx;
  out[5] = m.xx;
  out[6] = m.yx;
  out[7] = m.zx;

  out[8]  = m.wy;
  out[9]  = m.xy;
  out[10] = m.yy;
  out[11] = m.zy;
  
  out[12] = m.wz;
  out[13] = m.xz;
  out[14] = m.yz;
  out[15] = m.zz;
  
}

void m44_to_array_trans(matrix_4x4 m, float* out) {

  out[0] = m.ww;
  out[1] = m.wx;
  out[2] = m.wy;
  out[3] = m.wz;
  
  out[4] = m.xw;
  out[5] = m.xx;
  out[6] = m.xy;
  out[7] = m.xz;

  out[8]  = m.yw;
  out[9]  = m.yx;
  out[10] = m.yy;
  out[11] = m.yz;
  
  out[12] = m.zw;
  out[13] = m.zx;
  out[14] = m.zy;
  out[15] = m.zz;
  
}

void m44_print(matrix_4x4 m) {

  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.ww, m.wx, m.wy, m.wz);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.xw, m.xx, m.xy, m.xz);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.yw, m.yx, m.yy, m.yz);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.zw, m.zx, m.zy, m.zz);
  
}

matrix_4x4 m44_view_look_at(vector3 position, vector3 target, vector3 up) {

  /*
  Taken From:
  
  http://www.opengl.org/wiki/GluLookAt_code

  */
  
  vector3 zaxis = v3_normalize( v3_sub(target, position) );
  vector3 xaxis = v3_normalize( v3_cross(up, zaxis) );
  vector3 yaxis = v3_cross(zaxis, xaxis);

  matrix_4x4 view_matrix;
  view_matrix.ww = xaxis.x;
  view_matrix.wx = xaxis.y;
  view_matrix.wy = xaxis.z;
  view_matrix.wz = 0.0f;
  
  view_matrix.xw = yaxis.x;
  view_matrix.xx = yaxis.y;
  view_matrix.xy = yaxis.z;
  view_matrix.xz = 0.0f;
  
  view_matrix.yw = -zaxis.x;
  view_matrix.yx = -zaxis.y;
  view_matrix.yy = -zaxis.z;
  view_matrix.yz = 0.0f;
  
  view_matrix.zw = 0.0f;
  view_matrix.zx = 0.0f;
  view_matrix.zy = 0.0f;
  
  view_matrix.zz = 1.0f;
  
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
  proj_matrix.ww = (2.0 * near_clip) / (right - left);
  proj_matrix.xx = (2.0 * near_clip) / (top - bottom);
  proj_matrix.wy = (right + left) / (right - left);
  proj_matrix.xy = (top + bottom) / (top - bottom);
  proj_matrix.yy = (-far_clip - near_clip) / (far_clip - near_clip);
  proj_matrix.zy = -1.0;
  proj_matrix.yz = ( -(2.0 * near_clip) * far_clip) / (far_clip - near_clip);
  
  return proj_matrix;
}

matrix_4x4 m44_translation(vector3 v) {

  matrix_4x4 m = m44_id();
  m.wz = v.x;
  m.xz = v.y;
  m.yz = v.z;

  return m;
  
};

matrix_4x4 m44_scale(vector3 v) {

  matrix_4x4 m = m44_id();
  m.ww = v.x;
  m.xx = v.y;
  m.yy = v.z;

  return m;
};

matrix_4x4 m44_rotation_x(float a) {

  matrix_4x4 m = m44_id();
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);
  
  return m;
  
};

matrix_4x4 m44_rotation_y(float a) {

  matrix_4x4 m = m44_id();
  
  m.ww = cos(a);
  m.wy = sin(a);
  m.yw = -sin(a);
  m.yy = cos(a);

  return m;
  
};

matrix_4x4 m44_rotation_z(float a) {

  matrix_4x4 m = m44_id();
  
  m.ww = cos(a);
  m.wx = -sin(a);
  m.xw = sin(a);
  m.xx = cos(a);

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
  
  m.ww = 1.0 - 2 * q.y * q.y - 2 * q.z * q.z;
  m.wx =       2 * q.x * q.y - 2 * q.w * q.z;
  m.wy =       2 * q.x * q.z + 2 * q.w * q.y;
  
  m.xw =       2 * q.x * q.y + 2 * q.w * q.z;
  m.xx = 1.0 - 2 * q.x * q.x - 2 * q.z * q.z;
  m.xy =       2 * q.y * q.z + 2 * q.w * q.x;
  
  m.yw =       2 * q.x * q.z - 2 * q.w * q.y;
  m.yx =       2 * q.y * q.z - 2 * q.w * q.x;
  m.yy = 1.0 - 2 * q.x * q.x - 2 * q.y * q.y;
  
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