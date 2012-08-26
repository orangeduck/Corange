#version 120

uniform int width;
uniform int height;
uniform int aa_type;

uniform sampler2D ldr_texture;
uniform sampler2D vignetting_texture;

uniform sampler3D lut;

/* Headers */

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
  
	vec3 vignetting = texture2D(vignetting_texture, gl_TexCoord[0].xy).rgb;
  
  color = color * mix(vignetting, vec3(1.0,1.0,1.0), 0.5);
  
	gl_FragColor.rgb = color_correction(color, lut, 64);
	gl_FragColor.a = 1.0;
} 