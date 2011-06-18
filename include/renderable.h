#ifndef renderable_h
#define renderable_h

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "matrix.h"
#include "vector.h"
#include "material.h"
#include "geometry.h"

/* PositionNornalTangentBinormalUvsColor is structure
	
	http://www.opengl.org/wiki/Vertex_Specification_Best_Practices#Formatting_VBO_Data

*/

typedef struct {

  GLuint vbo;
  
  int num_verticies;
  
  material* instance;
  material* base;
  
} renderable_surface;


typedef struct {

  char* name;

  renderable_surface* surfaces;
  int num_surfaces;
  
  vector3 position;
  vector3 scale;
  vector4 rotation;
  
  int active;
  
} renderable;


renderable* renderable_new(char* name);
void renderable_delete(renderable* r);

void renderable_add_model(model* m);
void renderable_add_mesh(mesh* m);

matrix_4x4 renderable_world_matrix(renderable* r);

/* Renderable Surface */

renderable_surface* renderable_surface_new();
void renderable_surface_delete(renderable_surface* surface);

#endif