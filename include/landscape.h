#ifndef landscape_h
#define landscape_h

#include "texture.h"
#include "vector.h"
#include "terrain.h"

#define MAX_SURFACE_TYPES 5

typedef struct {
  
  texture* near_texture;
  texture* near_texture_nm;
  
  texture* far_texture;
  texture* far_texture_nm;
  
} landscape_surface;

typedef struct {
  
  terrain* terrain;
  
  texture* normalmap;
  texture* colormap;
  
  vector3 position;
  vector4 rotation;
  vector3 scale;
  
  int num_surface_types;
  landscape_surface surface_types[MAX_SURFACE_TYPES];
  
} landscape;

landscape* landscape_new();
void landscape_delete(landscape* l);
void landscape_add_surface(landscape* l, texture* near_texture, texture* near_texture_nm, texture* far_texture, texture* far_texture_nm);

#endif