#ifndef renderable_h
#define renderable_h

#include <stdbool.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "assets/material.h"

#include "vector.h"
#include "geometry.h"

typedef struct {

  GLuint vertex_vbo;
  GLuint triangle_vbo;
  
  int num_verticies;
  int num_triangles;
  
  material* base;
  
  bool is_rigged;
	
} renderable_surface;


typedef struct {

  renderable_surface** surfaces;
  int num_surfaces;
  
} renderable;


renderable* renderable_new();
void renderable_delete(renderable* r);

void renderable_set_material(renderable* r, material* m);
void renderable_set_multi_material(renderable* r, multi_material* mmat);

void renderable_add_mesh(renderable* r, mesh* m);
void renderable_add_model(renderable* r, model* m);

model* renderable_to_model(renderable* r);

/* Renderable Surface */

typedef struct {
  int bone_ids[3];
  float bone_weights[3];
} vertex_weight;

renderable_surface* renderable_surface_new(mesh* m);
renderable_surface* renderable_surface_new_rigged(mesh* m, vertex_weight* weights);
void renderable_surface_delete(renderable_surface* surface);

void renderable_surface_set_material(renderable_surface* s, material* m);

renderable* bmf_load_file(char* filename);
renderable* obj_load_file(char* filename);
renderable* smd_load_file(char* filename);

void bmf_save_file(renderable* r, char* filename);


#endif
