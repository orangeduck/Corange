#version 120

uniform float time;
uniform float glitch;
uniform int width;
uniform int height;
uniform int aa_type;

uniform sampler2D random_perlin;
uniform sampler2D ldr_texture;
uniform sampler2D vignetting_texture;

uniform sampler3D lut;

/* Headers */

vec3 to_gamma(vec3 color);
vec3 color_correction(vec3 color, sampler3D lut, int lut_size);
vec3 fxaa(sampler2D tex, vec2 uvs, int width, int height);
vec3 fxaa_unsharp(sampler2D tex, vec2 uvs, int width, int height);
vec3 unsharp_mask(sampler2D screen, vec2 coords, float strength, int width, int height);

/* End */

void main() {
  
  vec3 color;
  
  if (aa_type == 1) {
    color = fxaa_unsharp(ldr_texture, gl_TexCoord[0].xy, width, height);
  } else {
    color = unsharp_mask(ldr_texture, gl_TexCoord[0].xy, 0.5, width, height);
  }
  
  vec2 first = gl_TexCoord[0].xy + color.rg * vec2(5.0, 5.11) + color.b * vec2(-5.41) + vec2(0.01, 0.23) * time * 0.1;
  vec4 second = texture2D(random_perlin, first / 512.0 * vec2(width, height));
  vec4 third = texture2D(random_perlin, first * 0.2 + second.rg * 0.1 + vec2(-0.002, 0.15) * time * 0.1);
  
  color = mix(color, color * third.rgb, glitch);
  
  vec3 vignetting = texture2D(vignetting_texture, gl_TexCoord[0].xy).rgb;
  color = color * mix(vignetting, vec3(1.0,1.0,1.0), 0.75);
  color = to_gamma(color);
  
	gl_FragColor.rgb = color_correction(color, lut, 64);
	gl_FragColor.a = 1.0;
} 