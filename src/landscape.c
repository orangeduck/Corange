#include <stdlib.h>

#include "landscape.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  l->terrain = NULL;
  
  l->normalmap = NULL;
  l->colormap = NULL;
  l->attributemap = NULL;
  
  l->position = v3_zero();
  l->rotation = v4_quaternion_id();
  l->scale = v3_one();
  
  for(int i = 0; i < 4; i++) {
    l->surface_types[i].near_texture = NULL;
    l->surface_types[i].near_texture_nm = NULL;
    l->surface_types[i].far_texture = NULL;
    l->surface_types[i].far_texture_nm = NULL;
  }
  
  return l;
  
}

void landscape_delete(landscape* l) {
  
  free(l);
  
}

void landscape_set_surface(landscape* l, int channel, texture* near_texture, texture* near_texture_nm, texture* far_texture, texture* far_texture_nm) {
  
  if (channel >= 4) {
    error("Landscape only supports 4 surface types!");
  }
  
  l->surface_types[channel].near_texture = near_texture;
  l->surface_types[channel].near_texture_nm = near_texture_nm;
  l->surface_types[channel].far_texture = far_texture;
  l->surface_types[channel].far_texture_nm = far_texture_nm;
  
}