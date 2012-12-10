/**
*** :: Cengine ::
***
***   Common functions and structs used across the game engine.
***
***   This is the main component of corange and can be
***   considered a universal include across all files.
***   
***   It contains various utilities as well as
***   vector, matrix and geometry maths.
**/

#ifndef cengine_h
#define cengine_h

/* Standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <signal.h>

/* SDL includes */
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_rwops.h>
#include <SDL/SDL_mixer.h>
#include "SDL/SDL_local.h"


/*
** == File System Path ==
*/

typedef struct {
  char ptr[MAX_PATH];
} fpath;

fpath P(const char* path);

fpath fpath_full(fpath path);
fpath fpath_file(fpath path);
fpath fpath_file_location(fpath path);
fpath fpath_file_extension(fpath path);

/*
** == Errors & Debugging ==
*/

/* Register functions for events */
void at_error(void(*func)(void));
void at_warning(void(*func)(void));
void at_debug(void(*func)(void));

/* Call registered functions */
void error_();
void warning_();
void debug_();

#define error(MSG, ...) { printf("[ERROR] (%s:%s:%i) ", __FILE__, __func__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); error_(); exit(EXIT_FAILURE); }
#define warning(MSG, ...) { printf("[WARNING] (%s:%s:%i) ", __FILE__, __func__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); warning_(); }
#define debug(MSG, ...) { printf("[DEBUG] (%s:%s:%i) ", __FILE__, __func__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); debug_(); }

#define alloc_check(PTR) { if((PTR) == NULL) { error("Out of Memory!"); } }

/* OpenGL error checking */
#define SDL_GL_CheckError() { GLenum __glerror = glGetError(); if (__glerror) { error("OpenGL Error: %s", SDL_GL_ErrorString(__glerror)); } }

#define SDL_GL_CheckFrameBuffer() { GLenum __glfbstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); if(__glfbstatus != GL_FRAMEBUFFER_COMPLETE) { error("OpenGL FrameBuffer Error: %s", SDL_GL_FrameBufferErrorString(__glfbstatus)); } }

/*
** == Timing ==
*/

typedef struct {
  int id;
  unsigned long start;
  unsigned long end;
  unsigned long split;
} timer;

timer timer_start(int id);
timer timer_split(timer t);
timer timer_stop(timer t);

void timestamp(char* out);

/*
** == Framerate ==
*/

void frame_begin();
void frame_end();
void frame_end_at_rate(double fps);

double frame_rate();
double frame_time();
char* frame_rate_string();

/*
** == Types ==
*/

typedef int type_id;

#define typeid(TYPE) type_find(#TYPE, sizeof(TYPE))
type_id type_find(char* type, size_t size);
char* type_id_name(int id);

/*
** == Floating point Maths ==
*/

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
float cosine_interp(float p1, float p2, float amount);
float cubic_interp(float p1, float p2, float p3, float p4, float amount);
float nearest_interp(float p1, float p2, float amount);

float binearest_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount);
float bilinear_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount);
float bicosine_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount);
float bismoothstep_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount);
float bismootherstep_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount);

/*
** == Vector Maths ==
*/

/* vec2 */

typedef struct {
  float x;
  float y;
} vec2;

vec2 vec2_new(float x, float y);
vec2 vec2_zero();
vec2 vec2_one();

vec2 vec2_add(vec2 v1, vec2 vec2);
vec2 vec2_sub(vec2 v1, vec2 vec2);
vec2 vec2_mul(vec2 v, float fac);
vec2 vec2_mul_vec2(vec2 v1, vec2 vec2);
vec2 vec2_div(vec2 v, float fac);
vec2 vec2_pow(vec2 v, float exp);
vec2 vec2_neg(vec2 v);
vec2 vec2_abs(vec2 v);
vec2 vec2_floor(vec2 v);
vec2 vec2_fmod(vec2 v, float val);

vec2 vec2_max(vec2 v, float x);
vec2 vec2_min(vec2 v, float x);
vec2 vec2_clamp(vec2 v, float b, float t);

bool vec2_equ(vec2 v1, vec2 vec2);

float vec2_dot(vec2 v1, vec2 vec2);
float vec2_length_sqrd(vec2 v);
float vec2_length(vec2 v);
float vec2_dist_sqrd(vec2 v1, vec2 vec2);
float vec2_dist(vec2 v1, vec2 vec2);
float vec2_dist_manhattan(vec2 v1, vec2 vec2);
vec2 vec2_normalize(vec2 v);

vec2 vec2_reflect(vec2 v1, vec2 vec2);

vec2 vec2_from_string(char* s);
void vec2_print(vec2 v);

void vec2_to_array(vec2 v, float* out);

int vec2_hash(vec2 v);
int vec2_mix_hash(vec2 v);

vec2 vec2_saturate(vec2 v);
vec2 vec2_lerp(vec2 v1, vec2 vec2, float amount);
vec2 vec2_smoothstep(vec2 v1, vec2 vec2, float amount);
vec2 vec2_smootherstep(vec2 v1, vec2 vec2, float amount);


/* vec3 */

typedef struct {
  float x;
  float y;
  float z;
} vec3;

vec3 vec3_new(float x, float y, float z);
vec3 vec3_zero();
vec3 vec3_one();

vec3 vec3_red();
vec3 vec3_green();
vec3 vec3_blue();
vec3 vec3_white();
vec3 vec3_black();
vec3 vec3_grey();
vec3 vec3_light_grey();
vec3 vec3_dark_grey();

vec3 vec3_add(vec3 v1, vec3 vec2);
vec3 vec3_sub(vec3 v1, vec3 vec2);
vec3 vec3_mul(vec3 v, float fac);
vec3 vec3_mul_vec3(vec3 v1, vec3 vec2);
vec3 vec3_div(vec3 v, float fac);
vec3 vec3_pow(vec3 v, float fac);
vec3 vec3_neg(vec3 v);
vec3 vec3_abs(vec3 v);
vec3 vec3_floor(vec3 v);
vec3 vec3_fmod(vec3 v, float val);

bool vec3_equ(vec3 v1, vec3 vec2);

float vec3_dot(vec3 v1, vec3 vec2);
float vec3_length_sqrd(vec3 v);
float vec3_length(vec3 v);
float vec3_dist_sqrd(vec3 v1, vec3 vec2);
float vec3_dist(vec3 v1, vec3 vec2);
float vec3_dist_manhattan(vec3 v1, vec3 vec2);
vec3 vec3_cross(vec3 v1, vec3 vec2);
vec3 vec3_normalize(vec3 v);

vec3 vec3_reflect(vec3 v1, vec3 vec2);

vec3 vec3_from_string(char* s);
void vec3_print(vec3 v);

void vec3_to_array(vec3 v, float* out);

int vec3_hash(vec3 v);

vec3 vec3_saturate(vec3 v);
vec3 vec3_lerp(vec3 v1, vec3 vec2, float amount);
vec3 vec3_smoothstep(vec3 v1, vec3 vec2, float amount);
vec3 vec3_smootherstep(vec3 v1, vec3 vec2, float amount);

/* vec4 */

typedef struct {
  float x;
  float y;
  float z;
  float w;
} vec4;

vec4 vec4_new(float x, float y, float z, float w);
vec4 vec4_zero();
vec4 vec4_one();

vec4 vec4_red();
vec4 vec4_green();
vec4 vec4_blue();
vec4 vec4_white();
vec4 vec4_black();
vec4 vec4_grey();
vec4 vec4_light_grey();
vec4 vec4_dark_grey();

vec4 vec4_add(vec4 v1, vec4 vec2);
vec4 vec4_sub(vec4 v1, vec4 vec2);
vec4 vec4_mul(vec4 v, float fac);
vec4 vec4_mul_vec4(vec4 v1, vec4 vec2);
vec4 vec4_div(vec4 v, float fac);
vec4 vec4_pow(vec4 v, float fac);
vec4 vec4_neg(vec4 v);
vec4 vec4_abs(vec4 v);
vec4 vec4_floor(vec4 v);
vec4 vec4_fmod(vec4 v, float val);

bool vec4_equ(vec4 v1, vec4 vec2);

float vec4_dot(vec4 v1, vec4 vec2);
float vec4_length_sqrd(vec4 v);
float vec4_length(vec4 v);
float vec4_dist_sqrd(vec4 v1, vec4 vec2);
float vec4_dist(vec4 v1, vec4 vec2);
float vec4_dist_manhattan(vec4 v1, vec4 vec2);
vec4 vec4_normalize(vec4 v);

vec4 vec4_reflect(vec4 v1, vec4 vec2);

vec4 vec4_from_string(char* s);
void vec4_print(vec4 v);

void vec4_to_array(vec4 v, float* out);

vec4 vec3_to_homogeneous(vec3 v);
vec3 vec4_from_homogeneous(vec4 v);

int vec4_hash(vec4 v);

vec4 vec4_saturate(vec4 v);
vec4 vec4_lerp(vec4 v1, vec4 vec2, float amount);
vec4 vec4_smoothstep(vec4 v1, vec4 vec2, float amount);
vec4 vec4_smootherstep(vec4 v1, vec4 vec2, float amount);
vec4 vec4_nearest_interp(vec4 v1, vec4 vec2, float amount);

vec4 vec4_binearest_interp(vec4 top_left, vec4 top_right, vec4 bottom_left, vec4 bottom_right, float x_amount, float y_amount);
vec4 vec4_bilinear_interp(vec4 top_left, vec4 top_right, vec4 bottom_left, vec4 bottom_right, float x_amount, float y_amount);

/* quaterion */

vec4 quaternion_id();
vec4 quaternion_mul(vec4 v1, vec4 vec2);
vec4 quaternion_angle_axis(float angle, vec3 axis);
vec4 quaternion_rot(vec3 from, vec3 to);

vec4 quaternion_yaw(float angle);
vec4 quaternion_pitch(float angle);
vec4 quaternion_roll(float angle);

vec4 quaternion_euler(float roll, float pitch, float yaw);

vec4 quaternion_normalize(vec4 q);
vec4 quaternion_swap_handedness(vec4 q);

/*
** == Matrix Maths ==
*/

/* mat2 */

typedef struct {
  float xx; float xy;
  float yx; float yy;
} mat2;

mat2 mat2_id();
mat2 mat2_zero();
mat2 mat2_new(float xx, float xy, float yx, float yy);
mat2 mat2_mul_mat2(mat2 m1, mat2 mat2);
vec2 mat2_mul_vec2(mat2 m, vec2 v);

mat2 mat2_transpose(mat2 m);
float mat2_det(mat2 m);
mat2 mat2_inverse(mat2 m);

void mat2_to_array(mat2 m, float* out);
void mat2_print(mat2 m);
mat2 mat2_rotation(float a);

/* mat3 */

typedef struct {
  float xx; float xy; float xz;
  float yx; float yy; float yz;
  float zx; float zy; float zz;
} mat3;

mat3 mat3_id();
mat3 mat3_zero();
mat3 mat3_new(float xx, float xy, float xz,
              float yx, float yy, float yz,
              float zx, float zy, float zz);
mat3 mat3_mul_mat3(mat3 m1, mat3 mat2);
vec3 mat3_mul_vec3(mat3 m, vec3 v);

mat3 mat3_transpose(mat3 m);
float mat3_det(mat3 m);
mat3 mat3_inverse(mat3 m);

void mat3_to_array(mat3 m, float* out);
void mat3_print(mat3 m);

mat3 mat3_rotation_x(float a);
mat3 mat3_rotation_y(float a);
mat3 mat3_rotation_z(float a);
mat3 mat3_rotation_axis_angle(vec3 axis, float angle);

/* mat4 */

typedef struct {
  float xx; float xy; float xz; float xw;
  float yx; float yy; float yz; float yw;
  float zx; float zy; float zz; float zw;
  float wx; float wy; float wz; float ww;
} mat4;

mat4 mat4_id();
mat4 mat4_zero();
mat4 mat4_new(float xx, float xy, float xz, float xw,
              float yx, float yy, float yz, float yw,
              float zx, float zy, float zz, float zw,
              float wx, float wy, float wz, float ww);
mat4 mat4_transpose(mat4 m);

mat4 mat4_mul_mat4(mat4 m1, mat4 mat2);

vec4 mat4_mul_vec4(mat4 m, vec4 v);
vec3 mat4_mul_vec3(mat4 m, vec3 v);

float mat4_det(mat4 m);
mat4 mat4_inverse(mat4 m);

mat4 mat3_to_mat4(mat3 m);
mat3 mat4_to_mat3(mat4 m);

void mat4_to_array(mat4 m, float* out);
void mat4_to_array_trans(mat4 m, float* out);

void mat4_print(mat4 m);

mat4 mat4_translation(vec3 v);
mat4 mat4_scale(vec3 v);

mat4 mat4_rotation_x(float a);
mat4 mat4_rotation_y(float a);
mat4 mat4_rotation_z(float a);
mat4 mat4_rotation_axis_angle(vec3 axis, float angle);

mat4 mat4_rotation_euler(float x, float y, float z);
mat4 mat4_rotation_quaternion(vec4 q);

mat4 mat4_view_look_at(vec3 position, vec3 target, vec3 up);
mat4 mat4_perspective(float fov, float near_clip, float far_clip, float ratio);
mat4 mat4_orthographic(float left, float right, float bottom, float top, float near, float far);

mat4 mat4_world(vec3 position, vec3 scale, vec4 rotation);

mat4 mat4_lerp(mat4 m1, mat4 mat2, float amount);
mat4 mat4_smoothstep(mat4 m1, mat4 mat2, float amount);

vec4 mat4_to_quaternion(mat4 m);

/*
** == Geometry ==
*/

/* Plane */

typedef struct {
  vec3 direction;
  vec3 position;
} plane;

plane plane_new(vec3 position, vec3 direction);
plane plane_transform(plane p, mat4 world);
float plane_signed_distance(plane p, vec3 point);

bool point_behind_plane(vec3 point, plane plane);

/* Box */

typedef struct {
  plane top;
  plane bottom;
  plane left;
  plane right;
  plane front;
  plane back;
} box;

box box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
box box_sphere(vec3 center, float radius);
box box_merge(box b1, box b2);
box box_transform(box b1, mat4 world);

bool box_contains(box b1, vec3 point);

/* Sphere */

typedef struct {
  vec3 center;
  float radius;
  float radius_sqrd;
} sphere;

sphere sphere_new(vec3 center, float radius);
sphere sphere_of_box(box bb);
sphere sphere_merge(sphere s1, sphere s2);
sphere sphere_transform(sphere s, mat4 world);

bool sphere_contains_point(sphere s1, vec3 point);
bool sphere_contains_sphere(sphere s1, sphere s2);

/* Vectex */

typedef struct {
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 binormal;
  vec4 color;
  vec2 uvs;
} vertex;

vertex vertex_new();
bool vertex_equal(vertex v1, vertex v2);
void vertex_print(vertex v);

/* Mesh */

typedef struct {
  int num_verts;
  int num_triangles;
  vertex* verticies;
  uint32_t* triangles;
} mesh;

mesh* mesh_new();
void mesh_delete(mesh* m);

void mesh_generate_normals(mesh* m);
void mesh_generate_tangents(mesh* m);
void mesh_generate_orthagonal_tangents(mesh* m);
void mesh_generate_texcoords_cylinder(mesh* m);

void mesh_print(mesh* m);
float mesh_surface_area(mesh* m);

void mesh_transform(mesh* m, mat4 transform);
void mesh_translate(mesh* m, vec3 translation);
void mesh_scale(mesh* m, float scale);

/* Model */

typedef struct {
  int num_meshes;
  mesh** meshes;
} model;

model* model_new();
void model_delete(model* m);

void model_generate_normals(model* m);
void model_generate_tangents(model* m);
void model_generate_orthagonal_tangents(model* m);
void model_generate_texcoords_cylinder(model* m);

void model_print(model* m);
float model_surface_area(model* m);

void model_transform(model* m, mat4 transform);
void model_translate(model* m, vec3 translation);
void model_scale(model* m, float scale);

/* Triangle */

vec3 triangle_tangent(vertex v1, vertex v2, vertex v3);
vec3 triangle_binormal(vertex v1, vertex v2, vertex v3);
vec3 triangle_normal(vertex v1, vertex v2, vertex v3);
vec3 triangle_random_position(vertex v1, vertex v2, vertex v3);
float triangle_area(vertex v1, vertex v2, vertex v3);

float triangle_difference_u(vertex v1, vertex v2, vertex v3);
float triangle_difference_v(vertex v1, vertex v2, vertex v3);

vertex triangle_random_position_interpolation(vertex v1, vertex v2, vertex v3);

#endif
