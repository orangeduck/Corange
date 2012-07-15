#ifndef landscape_h
#define landscape_h

#include "vector.h"

#include "assets/texture.h"
#include "assets/terrain.h"

typedef struct {
  
  terrain* terrain;
  
  texture* normalmap;
  texture* colormap;
  texture* attributemap;
  
  vector3 position;
  vector4 rotation;
  vector3 scale;
  
  texture* near_texture;
  texture* near_texture_bump;
  texture* far_texture;
  texture* far_texture_bump;
  
} landscape;

landscape* landscape_new();
void landscape_delete(landscape* l);
void landscape_set_textures(landscape* l, texture* near_texture, texture* near_texture_bump, texture* far_texture, texture* far_texture_bump);

#endif
