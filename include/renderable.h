#ifndef renderable_h
#define renderable_h

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "vector.h"
#include "material.h"
#include "geometry.h"

typedef struct {

  GLuint vertex_vbo;
  GLuint triangle_vbo;
  
  int num_verticies;
  int num_triangles;
  
  material* instance;
  material* base;
  
} renderable_surface;


typedef struct {

  renderable_surface** surfaces;
  int num_surfaces;
  
} renderable;


renderable* renderable_new(model* m);
void renderable_delete(renderable* r);

void renderable_set_material(renderable* r, material* m);

/* Renderable Surface */

renderable_surface* renderable_surface_new(mesh* m);
void renderable_surface_delete(renderable_surface* surface);

void renderable_surface_set_material(renderable_surface* s, material* m);

#endif