#ifndef geometry_h
#define geometry_h

#include <stdbool.h>

#include "vector.h"
#include "matrix.h"

typedef struct {
  vector3 direction;
  vector3 position;
} plane;

plane plane_new(vector3 position, vector3 direction);
plane plane_transform(plane p, matrix_4x4 world);
float plane_signed_distance(plane p, vector3 point);

bool point_behind_plane(vector3 point, plane plane);

typedef struct {
  plane top;
  plane bottom;
  plane left;
  plane right;
  plane front;
  plane back;
} box;

box box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
box box_sphere(vector3 center, float radius);
box box_merge(box b1, box b2);
box box_transform(box b1, matrix_4x4 world);

bool box_contains(box b1, vector3 point);

typedef struct {
  vector3 center;
  float radius;
  float radius_sqrd;
} sphere;

sphere sphere_new(vector3 center, float radius);
sphere sphere_of_box(box bb);
sphere sphere_merge(sphere s1, sphere s2);
sphere sphere_transform(sphere s, matrix_4x4 world);

bool sphere_contains_point(sphere s1, vector3 point);
bool sphere_contains_sphere(sphere s1, sphere s2);

typedef struct {

  vector3 position;
  vector3 normal;
  vector3 tangent;
  vector3 binormal;
  vector4 color;
  
  vector2 uvs;

} vertex;

vertex vertex_new();
bool vertex_equal(vertex v1, vertex v2);
void vertex_print(vertex v);

typedef struct {
  
  char* name;
  char* material;
  
  int num_verts;
  int num_triangles;
  int num_triangles_3;
  
  vertex* verticies;
  int* triangles;
  
} mesh;

mesh* mesh_new();
void mesh_delete(mesh* m);

void mesh_generate_normals(mesh* m);
void mesh_generate_tangents(mesh* m);
void mesh_generate_orthagonal_tangents(mesh* m);
void mesh_generate_texcoords_cylinder(mesh* m);

void mesh_print(mesh* m);
float mesh_surface_area(mesh* m);

void mesh_transform(mesh* m, matrix_4x4 transform);
void mesh_translate(mesh* m, vector3 translation);
void mesh_scale(mesh* m, float scale);

typedef struct {
  
  char* name;
  
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

void model_transform(model* m, matrix_4x4 transform);
void model_translate(model* m, vector3 translation);
void model_scale(model* m, float scale);

vector3 triangle_tangent(vertex v1, vertex v2, vertex v3);
vector3 triangle_binormal(vertex v1, vertex v2, vertex v3);
vector3 triangle_normal(vertex v1, vertex v2, vertex v3);
vector3 triangle_random_position(vertex v1, vertex v2, vertex v3);
float triangle_area(vertex v1, vertex v2, vertex v3);

float triangle_difference_u(vertex v1, vertex v2, vertex v3);
float triangle_difference_v(vertex v1, vertex v2, vertex v3);

vertex triangle_random_position_interpolation(vertex v1, vertex v2, vertex v3);

#endif