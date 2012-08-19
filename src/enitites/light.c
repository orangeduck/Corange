#include "entities/light.h"

#define DEFAULT_LIGHT_NEAR_CLIP 0.1
#define DEFAULT_LIGHT_FAR_CLIP 8192.0
#define DEFAULT_FOV 0.785398163

light* light_new(vec3 position) {

  return light_new_type(position, light_type_point);
  
}

light* light_new_type(vec3 position, int type) {
   
  light* l = malloc(sizeof(light));

  l->position = position;
  l->target = vec3_zero();
  l->diffuse_color = vec3_new(1,1,1);
  l->specular_color = vec3_new(1,1,1);
  l->ambient_color = vec3_new(0.25,0.25,0.25);

  light_set_type(l, type);
  
  return l;
}

void light_set_type(light* l, int type) {

  if(type == light_type_directional) {
    
    l->type = light_type_directional;
    
    l->power = 1;
    l->falloff = 0;
    
    l->enabled = true;
    l->cast_shadows = false;
    
    l->orthographic = true;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = vec3_zero();
    l->shadow_map_width = -1;
    l->shadow_map_height = -1;
  
  } else if (type == light_type_point) {
    
    l->type = light_type_point;
    
    l->power = 5;
    l->falloff = 0.5;
    
    l->enabled = true;
    l->cast_shadows = false;
    
    l->orthographic = false;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = vec3_zero();
    l->shadow_map_width = -1;
    l->shadow_map_height = -1;
  
  } else if (type == light_type_sun) {
    
    l->type = light_type_sun;
    
    l->position = vec3_new(0,512,0);
    l->target = vec3_new(512, 0, 512);
    l->ambient_color = vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.6);
    l->diffuse_color = vec3_new(1.0,  0.937, 0.8);
    l->specular_color = vec3_new(1.0,  0.894, 0.811);
    
    l->power = 1.5;
    l->falloff = 0;
    
    l->enabled = true;
    l->cast_shadows = true;
    
    l->orthographic = true;
    l->ortho_width = 128;
    l->ortho_height = 128;
    
    l->fov = -1;
    l->aspect_ratio = -1;
    
    l->shadow_color = vec3_zero();
    l->shadow_map_width = 2048;
    l->shadow_map_height = 2048;
  
  } else if (type == light_type_spot) {
    
    l->type = light_type_spot;
    
    l->power = 5;
    l->falloff = 0.5;
    
    l->enabled = true;
    l->cast_shadows = true;
    
    l->orthographic = false;
    l->ortho_width = -1;
    l->ortho_height = -1;
    
    l->fov = DEFAULT_FOV;
    l->aspect_ratio = 1.0;
    
    l->shadow_color = vec3_zero();
    l->shadow_map_width = 1024;
    l->shadow_map_height = 1024;
    
  }

}

void light_delete(light* l) {
  free(l);
}

vec3 light_direction(light* l) {
  return vec3_normalize(  vec3_sub( l->target, l->position ) );
}

mat4 light_view_matrix(light* l) {

  return mat4_view_look_at( l->position, l->target, vec3_new(0,1,0) );

}

mat4 light_proj_matrix(light* l) {
  
  if ( l->orthographic ) {
  
    return mat4_orthographic(-l->ortho_width, l->ortho_width, -l->ortho_height, l->ortho_height, DEFAULT_LIGHT_NEAR_CLIP, DEFAULT_LIGHT_FAR_CLIP);
  
  } else {
    
    return mat4_perspective(l->fov, DEFAULT_LIGHT_NEAR_CLIP, DEFAULT_LIGHT_FAR_CLIP, l->aspect_ratio);
    
  }

}
