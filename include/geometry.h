#ifndef geometry_h
#define geometry_h

#include "bool.h"
#include "vector.h"

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

void mesh_delete(mesh* m);

void mesh_generate_normals(mesh* m);
void mesh_generate_tangents(mesh* m);
void mesh_generate_orthagonal_tangents(mesh* m);
void mesh_generate_texcoords_cylinder(mesh* m);

void mesh_print(mesh* m);
float mesh_surface_area(mesh* m);

typedef struct {
  
  char* name;
  
  int num_meshes;
  mesh** meshes;
  
} model;

void model_delete(model* m);

void model_generate_normals(model* m);
void model_generate_tangents(model* m);
void model_generate_orthagonal_tangents(model* m);
void model_generate_texcoords_cylinder(model* m);

void model_add_mesh(model* main_model, mesh* sub_mesh);
void model_merge_model(model* m1, model* m2);

void model_print(model* m);
float model_surface_area(model* m);


vector3 triangle_tangent(vertex v1, vertex v2, vertex v3);
vector3 triangle_binormal(vertex v1, vertex v2, vertex v3);
vector3 triangle_normal(vertex v1, vertex v2, vertex v3);
vector3 triangle_random_position(vertex v1, vertex v2, vertex v3);
float triangle_area(vertex v1, vertex v2, vertex v3);

float triangle_difference_u(vertex v1, vertex v2, vertex v3);
float triangle_difference_v(vertex v1, vertex v2, vertex v3);

vertex triangle_random_position_interpolation(vertex v1, vertex v2, vertex v3);

#endif