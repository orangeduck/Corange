#include <stdlib.h>
#include <stdio.h>

#include "light.h"

#define DEFAULT_LIGHT_NEAR_CLIP 4.0
#define DEFAULT_LIGHT_FAR_CLIP 4096.0
#define DEFAULT_FOV 0.785398163

light* light_new(vector3 position) {

  return light_new_type(position, light_type_point);
  
}

light* light_new_type(vector3 position, int type) {
  
  if(type == light_type_directional) {
  
    light* l = malloc(sizeof(light));
    
    l->position = position;
    l->target = v3_zero();
    
    l->diffuse_color = v3(1,1,1);
    l->specular_color = v3(1,1,1);
    l->ambient_color = v3(0.25,0.25,0.25);
    
    l->type = light_type_directional;
    
    l->enabled = 1;
    l->cast_shadows = 0;
    
    l->orthographic = 1;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = v3_zero();
    l->shadow_map_width = -1;
    l->shadow_map_height = -1;
    
    return l;
  
  } else if (type == light_type_point) {
  
    light* l = malloc(sizeof(light));
    
    l->position = position;
    l->target = v3_zero();
    
    l->diffuse_color = v3(1,1,1);
    l->specular_color = v3(1,1,1);
    l->ambient_color = v3(0.25,0.25,0.25);
    
    l->type = light_type_point;
    
    l->enabled = 1;
    l->cast_shadows = 0;
    
    l->orthographic = 0;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = v3_zero();
    l->shadow_map_width = -1;
    l->shadow_map_height = -1;
    
    return l;
  
  } else if (type == light_type_sun) {
  
    light* l = malloc(sizeof(light));
    
    l->position = position;
    l->target = v3_zero();
    
    l->diffuse_color = v3(1,1,1);
    l->specular_color = v3(1,1,1);
    l->ambient_color = v3(0.5,0.5,0.5);
    
    l->type = light_type_sun;
    
    l->enabled = 1;
    l->cast_shadows = 1;
    
    l->orthographic = 1;
    l->ortho_width = 1024;
    l->ortho_height = 1024;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = v3_zero();
    l->shadow_map_width = 1024;
    l->shadow_map_height = 1024;
    
    return l;
  
  } else if (type == light_type_spot) {
  
    light* l = malloc(sizeof(light));
    
    l->position = position;
    l->target = v3_zero();
    
    l->diffuse_color = v3(1,1,1);
    l->specular_color = v3(1,1,1);
    l->ambient_color = v3(0.75,0.75,0.75);
    
    l->type = light_type_spot;
    
    l->enabled = 1;
    l->cast_shadows = 1;
    
    l->orthographic = 0;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = 0.785398163;
    l->aspect_ratio = 1.0;
    
    l->shadow_color = v3_zero();
    l->shadow_map_width = 1024;
    l->shadow_map_height = 1024;
    
    return l;
    
  }
   
}

void light_delete(light* l) {
  free(l);
}

vector3 light_direction(light* l) {
  return v3_normalize(  v3_sub( l->target, l->position ) );
}

matrix_4x4 light_view_matrix(light* l) {

  return m44_view_look_at( l->position, l->target, v3(0,1,0) );

}

matrix_4x4 light_proj_matrix(light* l) {
  
  if ( l->orthographic ) {
  
    return m44_orthographic(-l->ortho_width, l->ortho_width, -l->ortho_height, l->ortho_height, DEFAULT_LIGHT_NEAR_CLIP, DEFAULT_LIGHT_FAR_CLIP);
  
  } else {
    
    return m44_perspective(l->fov, DEFAULT_LIGHT_NEAR_CLIP, DEFAULT_LIGHT_FAR_CLIP, l->aspect_ratio);
    
  }

}