/**
*** :: Renderable ::
***
***   A model or mesh loaded into vertex buffers
***   can be rigged or not depending on file type
***
***   Load using .bmf format for best performance.
***
**/

#ifndef renderable_h
#define renderable_h

#include "cengine.h"
#include "assets/material.h"

typedef struct {
  int bone_ids[3];
  float bone_weights[3];
} vertex_weight;

typedef struct {
  GLuint vertex_vbo;
  GLuint triangle_vbo;
  int num_verticies;
  int num_triangles;
  sphere bound;
} renderable_surface;

renderable_surface* renderable_surface_new(mesh* m);
renderable_surface* renderable_surface_new_rigged(mesh* m, vertex_weight* weights);
void renderable_surface_delete(renderable_surface* surface);

typedef struct {
  renderable_surface** surfaces;
  int num_surfaces;
  bool is_rigged;
  asset_hndl material;
} renderable;

renderable* renderable_new();
void renderable_delete(renderable* r);

void renderable_add_mesh(renderable* r, mesh* m);
void renderable_add_model(renderable* r, model* m);

model* renderable_to_model(renderable* r);

renderable* bmf_load_file(char* filename);
renderable* obj_load_file(char* filename);
renderable* smd_load_file(char* filename);

void bmf_save_file(renderable* r, char* filename);


#endif
