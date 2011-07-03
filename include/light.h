#ifndef light_h
#define light_h

#include "vector.h"
#include "matrix.h"

typedef struct {

  vector3 position;
  vector3 target;
  
  vector3 diffuse_color;
  vector3 specular_color;
  vector3 ambient_color;
  
  int type;
  
  int enabled;
  
  int cast_shadows;
  
  int orthographic;
  float ortho_width;
  float ortho_height;
  
  float fov;
  float aspect_ratio;
  
  vector3 shadow_color;
  int shadow_map_width;
  int shadow_map_height;
  
} light;

static int light_type_point = 0;
static int light_type_directional = 1;
static int light_type_sun = 2;
static int light_type_spot = 3;

light* light_new(vector3 position);
light* light_new_type(vector3 position, int type);
void light_delete(light* l);

vector3 light_direction(light* l);

matrix_4x4 light_view_matrix(light* l);
matrix_4x4 light_proj_matrix(light* l);

#endif