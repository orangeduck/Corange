#include <stdlib.h>

#include "entities/landscape.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  l->terrain = asset_hndl_null();
  
  l->normalmap = asset_hndl_null();
  l->colormap = asset_hndl_null();
  l->attributemap = asset_hndl_null();
  
  l->position = vec3_zero();
  l->rotation = quaternion_id();
  l->scale = vec3_one();
  
  l->near_texture = asset_hndl_null();
  l->near_texture_bump = asset_hndl_null();
  l->far_texture = asset_hndl_null();
  l->far_texture_bump = asset_hndl_null();
  
  return l;
  
}

void landscape_delete(landscape* l) {
  free(l);
}

void landscape_set_textures(landscape* l, asset_hndl near_texture, asset_hndl near_texture_bump, asset_hndl far_texture, asset_hndl far_texture_bump) {
  
  l->near_texture = near_texture;
  l->near_texture_bump = near_texture_bump;
  l->far_texture = far_texture;
  l->far_texture_bump = far_texture_bump;
  
}
