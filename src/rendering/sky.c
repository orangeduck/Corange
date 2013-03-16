
#include "rendering/sky.h"

sky* sky_new() {
  
  sky* s = malloc(sizeof(sky));
  
  s->time = 0;
  s->seed = 0;

  s->cloud_mesh[0]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_horizon1.bmf"));
  s->cloud_mesh[1]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_horizon2.bmf"));
  s->cloud_mesh[2]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_horizon3.bmf"));
  s->cloud_mesh[3]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_horizon4.bmf"));
  s->cloud_mesh[4]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_lower4.bmf"));
  s->cloud_mesh[5]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_sheet1.bmf"));
  s->cloud_mesh[6]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_sheet2.bmf"));
  s->cloud_mesh[7]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_sheet3.bmf"));
  s->cloud_mesh[8]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_sheet4.bmf"));
  s->cloud_mesh[9]  = asset_hndl_new_load(P("$CORANGE/sky/cloud_sheet5.bmf"));
  s->cloud_mesh[10] = asset_hndl_new_load(P("$CORANGE/sky/cloud_upper1.bmf"));
  s->cloud_mesh[11] = asset_hndl_new_load(P("$CORANGE/sky/cloud_upper2.bmf"));
  s->cloud_mesh[12] = asset_hndl_new_load(P("$CORANGE/sky/cloud_upper3.bmf"));
  s->cloud_mesh[13] = asset_hndl_new_load(P("$CORANGE/sky/cloud_upper4.bmf"));
  
  s->cloud_tex[0]  = asset_hndl_new_load(P("$CORANGE/sky/cloudshorizon01.dds"));
  s->cloud_tex[1]  = asset_hndl_new_load(P("$CORANGE/sky/cloudshorizon01.dds"));
  s->cloud_tex[2]  = asset_hndl_new_load(P("$CORANGE/sky/cloudshorizon01.dds"));
  s->cloud_tex[3]  = asset_hndl_new_load(P("$CORANGE/sky/cloudshorizon01.dds"));
  s->cloud_tex[4]  = asset_hndl_new_load(P("$CORANGE/sky/cloudslower04.dds"));
  s->cloud_tex[5]  = asset_hndl_new_load(P("$CORANGE/sky/cloudsheet01.dds"));
  s->cloud_tex[6]  = asset_hndl_new_load(P("$CORANGE/sky/cloudsheet01.dds"));
  s->cloud_tex[7]  = asset_hndl_new_load(P("$CORANGE/sky/cloudsheet01.dds"));
  s->cloud_tex[8]  = asset_hndl_new_load(P("$CORANGE/sky/cloudsheet01.dds"));
  s->cloud_tex[9]  = asset_hndl_new_load(P("$CORANGE/sky/cloudsheet01.dds"));
  s->cloud_tex[10] = asset_hndl_new_load(P("$CORANGE/sky/cloudsupper01.dds"));
  s->cloud_tex[11] = asset_hndl_new_load(P("$CORANGE/sky/cloudsupper01.dds"));
  s->cloud_tex[12] = asset_hndl_new_load(P("$CORANGE/sky/cloudsupper02.dds"));
  s->cloud_tex[13] = asset_hndl_new_load(P("$CORANGE/sky/cloudsupper02.dds"));
  
  s->cloud_opacity[0]  = 0.5;
  s->cloud_opacity[1]  = 0.5;
  s->cloud_opacity[2]  = 0.5;
  s->cloud_opacity[3]  = 0.5;
  s->cloud_opacity[4]  = 0.5;
  s->cloud_opacity[5]  = 0.0;
  s->cloud_opacity[6]  = 0.0;
  s->cloud_opacity[7]  = 0.0;
  s->cloud_opacity[8]  = 0.0;
  s->cloud_opacity[9]  = 0.0;
  s->cloud_opacity[10] = 0.5;
  s->cloud_opacity[11] = 0.5;
  s->cloud_opacity[12] = 0.5;
  s->cloud_opacity[13] = 0.5;
  
  s->sun_sprite = asset_hndl_new_load(P("$CORANGE/sky/sun_sprite.bmf"));
  s->sun_tex = asset_hndl_new_load(P("$CORANGE/sky/sun_sprite.dds"));
  
  s->moon_sprite = asset_hndl_new_load(P("$CORANGE/sky/moon_sprite.bmf"));
  s->moon_tex = asset_hndl_new_load(P("$CORANGE/sky/moon_sprite.dds"));
  
  s->stars_sprite = asset_hndl_null();
  s->stars_tex = asset_hndl_null();
  
  s->is_day = false;
  s->wind = vec3_new(1, 0, 0);
  
  s->world_sun = mat4_id();
  s->world_moon = mat4_id();
  s->world_stars = mat4_id();
  
  s->moon_power = 0;
  s->moon_direction = vec3_zero();
  s->moon_diffuse = vec3_zero();
  s->moon_ambient = vec3_zero();
  s->moon_specular = vec3_zero();
  
  s->sun_power = 0;
  s->sun_direction = vec3_zero();
  s->sun_diffuse = vec3_zero();
  s->sun_ambient = vec3_zero();
  s->sun_specular = vec3_zero();
  
  s->sky_power = 0;
  s->sky_direction = vec3_zero();
  s->sky_diffuse = vec3_zero();
  s->sky_ambient = vec3_zero();
  s->sky_specular = vec3_zero();
  
  s->ground_power = 0;
  s->ground_direction = vec3_zero();
  s->ground_diffuse = vec3_zero();
  s->ground_ambient = vec3_zero();
  s->ground_specular = vec3_zero();
  
  sky_update(s, 0.0, 0);
  
  return s;
  
}

void sky_delete(sky* s) {
  free(s);
}

void sky_update(sky* s, float t, uint32_t seed) {
  
  s->time = fmod(t, 1.0);
  s->seed = seed;
  
  s->is_day = (t > 0.0 && t < 0.5);
  
  /* Update Sun */

  float sun_x = -0.2;
  float sun_y = -sin(2 * M_PI * t);
  float sun_z =  cos(2 * M_PI * t);
  s->sun_power = max(sin(2 * M_PI * t), 0);
  s->sun_direction = vec3_normalize(vec3_new(sun_x, sun_y, sun_z));
  s->sun_diffuse = vec3_mul(vec3_new(1.0,  0.937, 0.8), 2.0);
  s->sun_ambient = vec3_mul(vec3_new(1.0,  0.937, 0.8), 0.5);
  s->sun_specular = vec3_mul(vec3_new(1.0,  0.894, 0.811), 2.0);
  
  float sun_orbit = M_PI + sin(2 * M_PI * t);
  float sun_tilt  = M_PI * -0.1;
  s->world_sun = mat4_mul_mat4(mat4_rotation_x(sun_orbit), mat4_rotation_z(sun_tilt));
  
  /* Update Moon */
  float moon_x =  0.2 * cos(2 * M_PI * t * 0.0366);
  float moon_y = -sin(0.56 * M_PI + 2 * M_PI * t * 0.0366);
  float moon_z =  cos(0.42 * M_PI + 2 * M_PI * t * 0.0366);

  s->moon_power = clamp(sin(M_PI + 2 * M_PI * t), 0, 0.2);
  s->moon_direction = vec3_normalize(vec3_new(moon_x, moon_y, moon_z));
  s->moon_diffuse = vec3_new(1, 1, 1);
  s->moon_ambient = vec3_new(1, 1, 1);
  s->moon_specular = vec3_new(1, 1, 1);

  s->world_moon = mat4_rotation_axis_angle(vec3_neg(s->moon_direction), 0);
  
  /* Update Sky */
  s->sky_power = max(sin(2 * M_PI * t) * 0.5 + 0.5, 0.1);
  s->sky_direction = vec3_neg(vec3_up());
  s->sky_diffuse = vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.3);
  s->sky_ambient = vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.2);
  s->sky_specular = vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.2);
  
  /* Update Ground */
  
  s->ground_power = s->sky_power;
  s->ground_direction = vec3_up();
  s->ground_diffuse = vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.2);
  s->ground_ambient = vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.05);
  s->ground_specular = vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.1);
  
}
