#version 120

/* Prototypes */

float to_gamma(float color);
float from_gamma(float color);
vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);

vec3 swap_red_green(vec3 color);
vec3 swap_red_green_inv(vec3 color);
vec3 swap_red_inv_green(vec3 color);
vec3 swap_invert_red_green(vec3 color);

vec3 tonemap(vec3 color, float exposure, float brightmax);
vec3 filmic_tonemap(vec3 color, float exposure);

vec3 color_correction(vec3 color, sampler3D lut, int lut_size);

vec3 desaturate(vec3 color);

/* End */

vec3 to_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 2.2);
  ret.g = pow(color.g, 2.2);
  ret.b = pow(color.b, 2.2);
	return ret;
}

vec3 from_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 1.0/2.2);
  ret.g = pow(color.g, 1.0/2.2);
  ret.b = pow(color.b, 1.0/2.2);
	return ret;
}

float to_gamma(float color) {
	return pow(color, 2.2);
}

float from_gamma(float color) {
	return pow(color, 1.0/2.2);
}

vec3 swap_red_green(vec3 color) {
  float temp = color.r;
  color.r = color.g;
  color.g = temp;
  return color;
}

vec3 swap_red_inv_green(vec3 color) {
  float temp = 1.0-color.r;
  color.r = color.g;
  color.g = temp;
  return color;
}

vec3 swap_red_green_inv(vec3 color) {
  float temp = 1.0-color.r;
  color.r = 1.0-color.g;
  color.g = temp;
  return color;
}

vec3 tonemap(vec3 color, float exposure, float brightmax) {
  return color * ( exposure / brightmax + 1.0 ) / (exposure + 1.0);
}




vec3 filmic_map(vec3 color) {

  const float A = 0.15;
  const float B = 0.50;
  const float C = 0.10;
  const float D = 0.20;
  const float E = 0.02;
  const float F = 0.30;

  return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 filmic_tonemap(vec3 color, float exposure) { 

  const float W = 11.2;

  vec3 curr = filmic_map( exposure * color );
  vec3 white_scale = 1.0 / 1.0 / filmic_map( vec3(W,W,W) );
  return curr * white_scale;
}

vec3 color_correction(vec3 color, sampler3D lut, int lut_size) {

  float scale = (float(lut_size) - 1.0) / float(lut_size);
  float offset = 1.0 / (2.0 * float(lut_size));

  return texture3D(lut, clamp(color, 0.0, 1.0) * scale + offset).rgb;
}

vec3 desaturate(vec3 color) {
  float s = (color.r + color.g + color.b) / 3;
  return vec3(s, s, s);
}