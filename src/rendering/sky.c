
#include "rendering/sky.h"

bool sky_isday(float t) {
  t = fmod(t, 1.0);
  return (t > 0.0 && t < 0.5);
}

vec3 sky_wind(float t, int seed) {
  return vec3_new(1, 0, 0);
}

asset_hndl sky_mesh_sun(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/sun_sprite.obj"));
}

asset_hndl sky_mesh_moon(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/moon_sprite.obj"));
}

asset_hndl sky_mesh_stars(float t) {
  return asset_hndl_null();
}

asset_hndl sky_tex_sun(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/sun_sprite.dds"));
}

asset_hndl sky_tex_moon(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/moon_sprite.dds"));
}

asset_hndl sky_tex_stars(float t) {
  return asset_hndl_null();
}

mat4 sky_mesh_sun_world(float t) {
  float orbit = M_PI + sin(2 * M_PI * t);
  float tilt  = M_PI * -0.1;
  return mat4_mul_mat4(mat4_rotation_x(orbit), mat4_rotation_z(tilt));
}

mat4 sky_mesh_moon_world(float t) {
  vec3 dir = sky_moon_direction(t);
  return mat4_rotation_axis_angle(vec3_neg(dir), 0);
}

mat4 sky_mesh_stars_world(float t) {
  return mat4_id();
}

float sky_sun_power(float t) {
  return max(sin(2 * M_PI * t), 0);
}

vec3 sky_sun_direction(float t) {
  float x = -0.2;
  float y = -sin(2 * M_PI * t);
  float z =  cos(2 * M_PI * t);
  return vec3_normalize(vec3_new(x, y, z));
}

vec3 sky_sun_diffuse(float t) {
  return vec3_mul(vec3_new(1.0,  0.937, 0.8), 1.5);
  //return vec3_mul(vec3_new(1.0,  0.937, 0.8), 0);
}

vec3 sky_sun_ambient(float t) {
  return vec3_mul(vec3_new(1.0,  0.937, 0.8), 0.5);
  //return vec3_mul(vec3_new(1.0,  0.937, 0.8), 0);
}

vec3 sky_sun_specular(float t) {
  return vec3_mul(vec3_new(1.0,  0.894, 0.811), 2.0);
  //return vec3_mul(vec3_new(1.0,  0.894, 0.811), 0);
}

vec3 sky_sky_direction(float t) {
  return vec3_neg(vec3_up());
}

float sky_moon_power(float t) {
  return clamp(sin(M_PI + 2 * M_PI * t), 0, 0.2);
}

vec3 sky_moon_direction(float t) {
  float x =  0.2 * cos(2 * M_PI * t * 0.0366);
  float y = -sin(0.56 * M_PI + 2 * M_PI * t * 0.0366);
  float z =  cos(0.42 * M_PI + 2 * M_PI * t * 0.0366);
  return vec3_normalize(vec3_new(x, y, z));
}

vec3 sky_moon_diffuse(float t) {
  return vec3_new(1, 1, 1);
}

vec3 sky_moon_ambient(float t) {
  return vec3_new(1, 1, 1);
}

vec3 sky_moon_specular(float t) {
  return vec3_new(1, 1, 1);
}

float sky_sky_power(float t) {
  float day_sky = sin(2 * M_PI * t) * 0.5 + 0.5;
  float night_sky = 0.1;
  return max(day_sky, night_sky);
}

vec3 sky_sky_diffuse(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.3);
}

vec3 sky_sky_ambient(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.2);
}

vec3 sky_sky_specular(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.2);
}

float sky_ground_power(float t) {
  return sky_sky_power(t);
}

vec3 sky_ground_direction(float t) {
  return vec3_up();
}

vec3 sky_ground_diffuse(float t) {
  return vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.2);
}

vec3 sky_ground_ambient(float t) {
  return vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.05);
}

vec3 sky_ground_specular(float t) {
  return vec3_mul(vec3_new(0.537, 0.572, 0.396), 0.1);
}


enum {
  cloud_count = 14,
};

int sky_clouds_num() {
  return cloud_count;
}

asset_hndl sky_clouds_mesh(int i) {
  return (asset_hndl[cloud_count]) {
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_horizon1.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_horizon2.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_horizon3.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_horizon4.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_lower4.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_sheet1.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_sheet2.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_sheet3.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_sheet4.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_sheet5.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_upper1.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_upper2.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_upper3.obj")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloud_upper4.obj"))
  }[i];
}

asset_hndl sky_clouds_tex(int i) {
  return (asset_hndl[cloud_count]) {
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudshorizon01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudshorizon01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudshorizon01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudshorizon01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudslower04.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsheet01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsheet01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsheet01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsheet01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsheet01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsupper01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsupper01.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsupper02.dds")),
    asset_hndl_new_load(P("$CORANGE/resources/clouds/cloudsupper02.dds")),
  }[i];
}

float sky_clouds_opacity(int i, float t, int seed) {
  if (i ==  0) { return 0.5; }
  if (i ==  1) { return 0.5; }
  if (i ==  2) { return 0.5; }
  if (i ==  3) { return 0.5; }
  if (i ==  4) { return 0.5; }
  if (i ==  5) { return 0.0; }
  if (i ==  6) { return 0.0; }
  if (i ==  7) { return 0.0; }
  if (i ==  8) { return 0.0; }
  if (i ==  9) { return 0.0; }
  if (i == 10) { return 0.5; }
  if (i == 11) { return 0.5; }
  if (i == 12) { return 0.5; }
  if (i == 13) { return 0.5; }
  return 0;
}