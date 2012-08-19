#ifndef landscape_h
#define landscape_h

#include "cengine.h"
#include "asset_manager.h"

typedef struct {
  
  asset_hndl terrain;
  
  asset_hndl normalmap;
  asset_hndl colormap;
  asset_hndl attributemap;
  
  vec3 position;
  vec4 rotation;
  vec3 scale;
  
  asset_hndl near_texture;
  asset_hndl near_texture_bump;
  asset_hndl far_texture;
  asset_hndl far_texture_bump;
  
} landscape;

landscape* landscape_new();
void landscape_delete(landscape* l);
void landscape_set_textures(landscape* l, asset_hndl near_texture, asset_hndl near_texture_bump, asset_hndl far_texture, asset_hndl far_texture_bump);

#endif
