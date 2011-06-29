#ifndef vector_h
#define vector_h

typedef struct {
  float x;
  float y;
} vector2;


typedef struct {
  float x;
  float y;
  float z;
} vector3;


typedef struct {
  float w;
  float x;
  float y;
  float z;
} vector4;


/* Vector2 */


vector2 v2(float x, float y);
vector2 v2_zero();
vector2 v2_one();

vector2 v2_add(vector2 v1, vector2 v2);
vector2 v2_sub(vector2 v1, vector2 v2);
vector2 v2_mul(vector2 v, float fac);
vector2 v2_div(vector2 v, float fac);
vector2 v2_neg(vector2 v);
vector2 v2_abs(vector2 v);

int v2_equ(vector2 v1, vector2 v2);

float v2_dot(vector2 v1, vector2 v2);
float v2_length(vector2 v);
vector2 v2_normalize(vector2 v);

vector2 v2_from_string(char* s);
void v2_print(vector2 v);

void v2_to_array(vector2 v, float* out);

int v2_hash(vector2 v);

/* Vector3 */


vector3 v3(float x, float y, float z);
vector3 v3_zero();
vector3 v3_one();

vector3 v3_add(vector3 v1, vector3 v2);
vector3 v3_sub(vector3 v1, vector3 v2);
vector3 v3_mul(vector3 v, float fac);
vector3 v3_div(vector3 v, float fac);
vector3 v3_neg(vector3 v);
vector3 v3_abs(vector3 v);

int v3_equ(vector3 v1, vector3 v2);

float v3_dot(vector3 v1, vector3 v2);
float v3_length(vector3 v);
vector3 v3_cross(vector3 v1, vector3 v2);
vector3 v3_normalize(vector3 v);

vector3 v3_from_string(char* s);
void v3_print(vector3 v);

void v3_to_array(vector3 v, float* out);

vector4 v3_to_homogeneous(vector3 v);

int v3_hash(vector3 v);

/* Vector4 */


vector4 v4(float w, float x, float y, float z);
vector4 v4_zero();
vector4 v4_one();

vector4 v4_add(vector4 v1, vector4 v2);
vector4 v4_sub(vector4 v1, vector4 v2);
vector4 v4_mul(vector4 v, float fac);
vector4 v4_div(vector4 v, float fac);
vector4 v4_neg(vector4 v);
vector4 v4_abs(vector4 v);

int v4_equ(vector4 v1, vector4 v2);

float v4_dot(vector4 v1, vector4 v2);
float v4_length(vector4 v);
vector4 v4_normalize(vector4 v);

vector4 v4_from_string(char* s);
void v4_print(vector4 v);

void v4_to_array(vector4 v, float* out);

vector3 v4_from_homogeneous(vector4 v);

int v4_hash(vector4 v);

vector4 v4_quaternion_id();
vector4 v4_quaternion_mul(vector4 v1, vector4 v2);
vector4 v4_quaternion_angle_axis(float angle, vector3 axis);

vector4 v4_quaternion_yaw(float angle);
vector4 v4_quaternion_pitch(float angle);
vector4 v4_quaternion_roll(float angle);

#endif