#version 120

uniform float time;
uniform float glitch;

uniform int fxaa_quality;
uniform int width;
uniform int height;

uniform sampler2D ldr_texture;
uniform sampler2D random_perlin;
uniform sampler2D vignetting_texture;

uniform sampler3D lut;

varying vec2 fTexcoord;

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

vec3 desaturate(vec3 color) {
  float s = (color.r + color.g + color.b) / 3;
  return vec3(s, s, s);
}

vec3 color_correction(vec3 color, sampler3D lut, int lut_size) {

  float scale = (float(lut_size) - 1.0) / float(lut_size);
  float offset = 1.0 / (2.0 * float(lut_size));

  return texture3D(lut, clamp(color, 0.0, 1.0) * scale + offset).rgb;
}

vec3 fxaa(sampler2D screen, vec2 texcoord, int width, int height) {
  
  float span_max = 4.0;
  float reduce_amount = 1.0 / 4.0;
  float reduce_min = (1.0 / 64.0);
  
  vec2 pixel = 1.0 / vec2(width, height);

  vec3 rgb_nw = texture2D(screen, texcoord.xy + (vec2(-1.0, -1.0) * pixel)).rgb;
  vec3 rgb_ne = texture2D(screen, texcoord.xy + (vec2( 1.0, -1.0) * pixel)).rgb;
  vec3 rgb_sw = texture2D(screen, texcoord.xy + (vec2(-1.0,  1.0) * pixel)).rgb;
  vec3 rgb_se = texture2D(screen, texcoord.xy + (vec2( 1.0,  1.0) * pixel)).rgb;
  vec3 rgb_m  = texture2D(screen, texcoord.xy).rgb;
	
  vec3 luma = vec3(0.299, 0.587, 0.114);
  float luma_nw = dot(rgb_nw, luma);
  float luma_ne = dot(rgb_ne, luma);
  float luma_sw = dot(rgb_sw, luma);
  float luma_se = dot(rgb_se, luma);
  float luma_m  = dot( rgb_m, luma);
	
  float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
  float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));
	
  vec2 dir;
  dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
  dir.y =  ((luma_nw + luma_sw) - (luma_ne + luma_se));
	
  float dir_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * (0.25 * reduce_amount), reduce_min);
  float dir_rcp_min = 1.0/(min(abs(dir.x), abs(dir.y)) + dir_reduce);
	
  dir = min(vec2(span_max,  span_max), max(vec2(-span_max, -span_max), dir * dir_rcp_min)) * pixel;

  vec3 rgba0 = texture2D(screen, texcoord.xy + dir * (1.0 / 3.0 - 0.5)).rgb;
  vec3 rgba1 = texture2D(screen, texcoord.xy + dir * (2.0 / 3.0 - 0.5)).rgb;
  vec3 rgba2 = texture2D(screen, texcoord.xy + dir * (0.0 / 3.0 - 0.5)).rgb;
  vec3 rgba3 = texture2D(screen, texcoord.xy + dir * (3.0 / 3.0 - 0.5)).rgb;
  
  vec3 rgb_a = (1.0/ 2.0) * (rgba0 + rgba1);
  vec3 rgb_b = rgb_a * (1.0/ 2.0) + (1.0/ 4.0) * (rgba2 + rgba3);
  
  float luma_b = dot(rgb_b, luma);
  
  if((luma_b < luma_min) || (luma_b > luma_max)){
    return rgb_a;
  } else {
    return rgb_b;
  }
  
}


void main() {
  
  vec3 color;
  
  if (fxaa_quality == 2 || fxaa_quality == 1) {
    color = fxaa(ldr_texture, fTexcoord, width, height);  
  } else {
    color = texture2D(ldr_texture, fTexcoord).rgb;  
  }
  
  {
    vec2 first = mod(fTexcoord + color.rg * vec2(5.0, 5.11) + color.b * vec2(-5.41) + color.rg * time * 0.05, 1.0);
    vec4 second = texture2D(random_perlin, mod(first / 512.0 * vec2(width, height), 1.0));
    vec4 third  = texture2D(random_perlin, mod(first * 0.2 + second.rg * 0.1 + color.gb * time * 0.05, 1.0));
    color = mix(color, 1.0 * desaturate(third.rgb), glitch);
  }
  
  {
    vec3 vignetting = texture2D(vignetting_texture, fTexcoord).rgb;
    color = color * mix(vignetting, vec3(1.0,1.0,1.0), 0.0);
  }
  
	gl_FragColor.rgb = color_correction(color, lut, 64);
	gl_FragColor.a = 1.0;
} 