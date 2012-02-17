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

/* End */

#define BLOOM_SIZE 4
#define BLOOM_AMOUNT 5
#define BLOOM_COLOR 3

void main() {
  
  vec3 color;
  if (aa_type == 1) {
    color = fxaa_unsharp(ldr_texture, gl_TexCoord[0].xy, width, height);
  } else {
    color = texture2D(ldr_texture, gl_TexCoord[0].xy).rgb;
  }
  
	vec3 vignetting = texture2D(vignetting_texture, gl_TexCoord[0].xy).rgb;
  
  vec4 bloom_s = texture2DLod(ldr_texture, gl_TexCoord[0].xy, BLOOM_SIZE);
  vec3 bloom = BLOOM_AMOUNT * bloom_s.a * pow(bloom_s.rgb, BLOOM_COLOR);
  
  color = (color + bloom) * mix(vignetting, vec3(1,1,1), 0.5);
  
	gl_FragColor.rgb = color_correction(color, lut, 64);
	gl_FragColor.a = 1.0;
} 