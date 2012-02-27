#include <stdlib.h>

#include "landscape.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  l->terrain = NULL;
  
  l->normalmap = NULL;
  l->colormap = NULL;
  
  l->position = v3_zero();
  l->rotation = v4_quaternion_id();
  l->scale = v3_one();
  
  l->num_surface_types = 0;
  for(int i = 0; i < MAX_SURFACE_TYPES; i++) {
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

void landscape_add_surface(landscape* l, texture* near_texture, texture* near_texture_nm, texture* far_texture, texture* far_texture_nm) {
  
  if (l->num_surface_types == MAX_SURFACE_TYPES) {
    error("Could not add surface type, max of %i already reached!", MAX_SURFACE_TYPES);
  }
  
  l->surface_types[l->num_surface_types].near_texture = near_texture;
  l->surface_types[l->num_surface_types].near_texture_nm = near_texture_nm;
  l->surface_types[l->num_surface_types].far_texture = far_texture;
  l->surface_types[l->num_surface_types].far_texture_nm = far_texture_nm;
  l->num_surface_types++;
  
}