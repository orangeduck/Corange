#ifndef sky_h
#define sky_h

#include "cengine.h"
#include "casset.h"

static const float TIME_SUNRISE  = 0.00;
static const float TIME_MORNING  = 0.10;
static const float TIME_MIDDAY   = 0.25;
static const float TIME_EVENING  = 0.40;
static const float TIME_SUNSET   = 0.50;
static const float TIME_MIDNIGHT = 0.75;

bool sky_isday(float t);

asset_hndl sky_mesh_sun(float t);
asset_hndl sky_mesh_moon(float t);
asset_hndl sky_mesh_stars(float t);

asset_hndl sky_tex_sun(float t);
asset_hndl sky_tex_moon(float t);
asset_hndl sky_tex_stars(float t);

mat4 sky_mesh_sun_world(float t);
mat4 sky_mesh_moon_world(float t);
mat4 sky_mesh_stars_world(float t);

float sky_moon_power(float t);
vec3 sky_moon_direction(float t);
vec3 sky_moon_diffuse(float t);
vec3 sky_moon_ambient(float t);
vec3 sky_moon_specular(float t);

float sky_sun_power(float t);
vec3 sky_sun_direction(float t);
vec3 sky_sun_diffuse(float t);
vec3 sky_sun_ambient(float t);
vec3 sky_sun_specular(float t);

float sky_sky_power(float t);
vec3 sky_sky_direction(float t);
vec3 sky_sky_diffuse(float t);
vec3 sky_sky_ambient(float t);
vec3 sky_sky_specular(float t);

float sky_ground_power(float t);
vec3 sky_ground_direction(float t);
vec3 sky_ground_diffuse(float t);
vec3 sky_ground_ambient(float t);
vec3 sky_ground_specular(float t);

#endif