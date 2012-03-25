#ifndef landscape_h
#define landscape_h

#include "vector.h"

#include "assets/texture.h"
#include "assets/terrain.h"

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
  texture* attributemap;
  
  vector3 position;
  vector4 rotation;
  vector3 scale;
  
  landscape_surface surface_types[4];
  
} landscape;

landscape* landscape_new();
void landscape_delete(landscape* l);
void landscape_set_surface(landscape* l, int channel, texture* near_texture, texture* near_texture_nm, texture* far_texture, texture* far_texture_nm);

#endif
