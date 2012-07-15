#include <stdlib.h>

#include "entities/landscape.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  l->terrain = NULL;
  
  l->normalmap = NULL;
  l->colormap = NULL;
  l->attributemap = NULL;
  
  l->position = v3_zero();
  l->rotation = v4_quaternion_id();
  l->scale = v3_one();
  
  l->near_texture = NULL;
  l->near_texture_bump = NULL;
  l->far_texture = NULL;
  l->far_texture_bump = NULL;
  
  return l;
  
}

void landscape_delete(landscape* l) {
  free(l);
}

void landscape_set_textures(landscape* l, texture* near_texture, texture* near_texture_bump, texture* far_texture, texture* far_texture_bump) {
  
  l->near_texture = near_texture;
  l->near_texture_bump = near_texture_bump;
  l->far_texture = far_texture;
  l->far_texture_bump = far_texture_bump;
  
}
