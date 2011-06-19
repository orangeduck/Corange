#ifndef renderable_h
#define renderable_h

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "vector.h"
#include "material.h"
#include "geometry.h"

/*
	
	http://www.opengl.org/wiki/Vertex_Specification_Best_Practices#Formatting_VBO_Data

*/

typedef struct {

  GLuint vertex_vbo;
  GLuint triangle_vbo;
  
  int num_verticies;
  int num_triangles;
  
  material* instance;
  material* base;
  
} renderable_surface;


typedef struct {

  char* name;

  renderable_surface** surfaces;
  int num_surfaces;
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  int active;
  
} renderable;


renderable* renderable_new(char* name);
void renderable_delete(renderable* r);

void renderable_add_model(renderable* r, model* m);
void renderable_add_mesh(renderable* r, mesh* m);

void renderable_set_material(renderable* r, material* m);

/* Renderable Surface */

renderable_surface* renderable_surface_new(mesh* m);
void renderable_surface_delete(renderable_surface* surface);

void renderable_surface_set_material(renderable_surface* s, material* m);

#endif