/*
  Geometry Stuff

*/

#include "vector.h"

#ifndef geometry_h
#define geometry_h

/* Vertex */
typedef struct {

  vector3 position;
  vector3 normal;
  vector3 tangent;
  vector3 binormal;
  vector4 color;
  
  vector2 uvs;

} vertex;

int vertex_equal(vertex v1, vertex v2);

/* Mesh */
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
int mesh_contains_vert(mesh* m, vertex v, int* position);
void mesh_generate_tangents(mesh* m);
void mesh_generate_orthagonal_tangents(mesh* m);
void mesh_print(mesh* m);

int mesh_append_vertex(mesh* m, vertex v);
int mesh_append_triangle_entry(mesh* m, int pos);

/* Model */
typedef struct {
  
  char* name;
  
  int num_meshes;
  mesh** meshes;
  
} model;

void model_delete(model* m);
void model_generate_tangents(model* m);
void model_generate_orthagonal_tangents(model* m);
void model_add_mesh(model* main_model, mesh* sub_mesh);
void model_print(model* m);

/* easier for rendering in openGL */
typedef struct {

  char* name;
  char* material;
  
  int num_verts;
  int num_triangles;
  int num_triangles_3;
  
  float* vertex_positions;
  float* vertex_normals;
  float* vertex_uvs;
  float* vertex_tangents;
  float* vertex_binormals;
  float* vertex_colors;
  
  int* triangles;

} render_mesh;

void render_mesh_delete(render_mesh* m);
void render_mesh_print(render_mesh* m);

render_mesh* to_render_mesh(mesh* m);
mesh* from_render_mesh(render_mesh* m);

typedef struct {
  
  char* name;
  
  int num_meshes;
  render_mesh** meshes;
  
} render_model;

void render_model_delete(render_model* m);
void render_model_print(render_model* m);

render_model* to_render_model(model* m);
render_model* render_model_from_render_mesh(render_mesh* m);
model* from_render_model(render_model* m);


vector3 triangle_tangent(vertex v1, vertex v2, vertex v3);
vector3 triangle_binormal(vertex v1, vertex v2, vertex v3);
vector3 triangle_normal(vertex v1, vertex v2, vertex v3);

render_model* cbm_load_file(char* filename);
void cbm_write_file(render_model* model);

#endif