#ifndef vector_h
#define vector_h

#include "bool.h"

typedef union {
  struct {
    float x;
    float y;
  };
  
  float xy[2];
  
} vector2;


typedef union {
  struct {
    float x;
    float y;
    float z;
  };

  struct {
    float r;
    float g;
    float b;
  };
  
  float xyz[3];
  
} vector3;


typedef union {
  struct {
    float x;
    float y;
    float z;
    float w;
  };
  
  struct {
    float r;
	float g;
	float b;
	float a;
  };
  
  float xyzw[4];
  
} vector4;


/* Floating point maths */


#ifndef max
float max(float x, float y);
#endif
#ifndef min
  float min(float x, float y);
#endif
float clamp(float x, float bottom, float top);
float saturate(float x);

float lerp(float p1, float p2, float amount);
float smoothstep(float p1, float p2, float amount);
float smootherstep(float p1, float p2, float amount);
float cosine_interpolation(float p1, float p2, float amount);
float cubic_interpolation(float p1, float p2, float p3, float p4, float amount);
float nearest_neighbor_interpolation(float p1, float p2, float amount);

float binearest_neighbor_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount);
float bilinear_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount);
float bicosine_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount);
float bismoothstep_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount);
float bismootherstep_interpolation(float top_left, float top_right, float bottom_left, float bottom_right, float x_amount, float y_amount);

/* Vector2 */


vector2 v2(float x, float y);
vector2 v2_new(float x, float y);
vector2 v2_zero();
vector2 v2_one();

vector2 v2_add(vector2 v1, vector2 v2);
vector2 v2_sub(vector2 v1, vector2 v2);
vector2 v2_mul(vector2 v, float fac);
vector2 v2_mul_v2(vector2 v1, vector2 v2);
vector2 v2_div(vector2 v, float fac);
vector2 v2_pow(vector2 v, float exp);
vector2 v2_neg(vector2 v);
vector2 v2_abs(vector2 v);
vector2 v2_floor(vector2 v);
vector2 v2_fmod(vector2 v, float val);

vector2 v2_max(vector2 v, float x);
vector2 v2_min(vector2 v, float x);
vector2 v2_clamp(vector2 v, float b, float t);

bool v2_equ(vector2 v1, vector2 v2);

float v2_dot(vector2 v1, vector2 v2);
float v2_length_sqrd(vector2 v);
float v2_length(vector2 v);
float v2_dist_sqrd(vector2 v1, vector2 v2);
float v2_dist(vector2 v1, vector2 v2);
vector2 v2_normalize(vector2 v);

vector2 v2_reflect(vector2 v1, vector2 v2);

vector2 v2_from_string(char* s);
void v2_print(vector2 v);

void v2_to_array(vector2 v, float* out);

int v2_hash(vector2 v);
int v2_mix_hash(vector2 v);

vector2 v2_saturate(vector2 v);
vector2 v2_lerp(vector2 v1, vector2 v2, float amount);
vector2 v2_smoothstep(vector2 v1, vector2 v2, float amount);
vector2 v2_smootherstep(vector2 v1, vector2 v2, float amount);


/* Vector3 */


vector3 v3(float x, float y, float z);
vector3 v3_new(float x, float y, float z);
vector3 v3_zero();
vector3 v3_one();

vector3 v3_red();
vector3 v3_green();
vector3 v3_blue();
vector3 v3_white();
vector3 v3_black();
vector3 v3_grey();

vector3 v3_add(vector3 v1, vector3 v2);
vector3 v3_sub(vector3 v1, vector3 v2);
vector3 v3_mul(vector3 v, float fac);
vector3 v3_mul_v3(vector3 v1, vector3 v2);
vector3 v3_div(vector3 v, float fac);
vector3 v3_pow(vector3 v, float fac);
vector3 v3_neg(vector3 v);
vector3 v3_abs(vector3 v);
vector3 v3_floor(vector3 v);
vector3 v3_fmod(vector3 v, float val);

bool v3_equ(vector3 v1, vector3 v2);

float v3_dot(vector3 v1, vector3 v2);
float v3_length_sqrd(vector3 v);
float v3_length(vector3 v);
float v3_dist_sqrd(vector3 v1, vector3 v2);
float v3_dist(vector3 v1, vector3 v2);
vector3 v3_cross(vector3 v1, vector3 v2);
vector3 v3_normalize(vector3 v);

vector3 v3_reflect(vector3 v1, vector3 v2);

vector3 v3_from_string(char* s);
void v3_print(vector3 v);

void v3_to_array(vector3 v, float* out);

vector4 v3_to_homogeneous(vector3 v);

int v3_hash(vector3 v);

vector3 v3_saturate(vector3 v);
vector3 v3_lerp(vector3 v1, vector3 v2, float amount);
vector3 v3_smoothstep(vector3 v1, vector3 v2, float amount);
vector3 v3_smootherstep(vector3 v1, vector3 v2, float amount);

/* Vector4 */


vector4 v4(float x, float y, float z, float w);
vector4 v4(float x, float y, float z, float w);
vector4 v4_zero();
vector4 v4_one();

vector4 v4_red();
vector4 v4_green();
vector4 v4_blue();
vector4 v4_white();
vector4 v4_black();
vector4 v4_grey();

vector4 v4_add(vector4 v1, vector4 v2);
vector4 v4_sub(vector4 v1, vector4 v2);
vector4 v4_mul(vector4 v, float fac);
vector4 v4_mul_v4(vector4 v1, vector4 v2);
vector4 v4_div(vector4 v, float fac);
vector4 v4_pow(vector4 v, float fac);
vector4 v4_neg(vector4 v);
vector4 v4_abs(vector4 v);
vector4 v4_floor(vector4 v);
vector4 v4_fmod(vector4 v, float val);

bool v4_equ(vector4 v1, vector4 v2);

float v4_dot(vector4 v1, vector4 v2);
float v4_length_sqrd(vector4 v);
float v4_length(vector4 v);
float v4_dist_sqrd(vector4 v1, vector4 v2);
float v4_dist(vector4 v1, vector4 v2);
vector4 v4_normalize(vector4 v);

vector4 v4_reflect(vector4 v1, vector4 v2);

vector4 v4_from_string(char* s);
void v4_print(vector4 v);

void v4_to_array(vector4 v, float* out);

vector3 v4_from_homogeneous(vector4 v);

int v4_hash(vector4 v);

vector4 v4_saturate(vector4 v);
vector4 v4_lerp(vector4 v1, vector4 v2, float amount);
vector4 v4_smoothstep(vector4 v1, vector4 v2, float amount);
vector4 v4_smootherstep(vector4 v1, vector4 v2, float amount);
vector4 v4_nearest_neighbor_interpolation(vector4 v1, vector4 v2, float amount);

vector4 v4_binearest_neighbor_interpolation(vector4 top_left, vector4 top_right, vector4 bottom_left, vector4 bottom_right, float x_amount, float y_amount);
vector4 v4_bilinear_interpolation(vector4 top_left, vector4 top_right, vector4 bottom_left, vector4 bottom_right, float x_amount, float y_amount);

vector4 v4_quaternion_id();
vector4 v4_quaternion_mul(vector4 v1, vector4 v2);
vector4 v4_quaternion_angle_axis(float angle, vector3 axis);

vector4 v4_quaternion_yaw(float angle);
vector4 v4_quaternion_pitch(float angle);
vector4 v4_quaternion_roll(float angle);

vector4 v4_quaternion_euler(float roll, float pitch, float yaw);

vector4 v4_quaternion_swap_handedness(vector4 q);

#endif