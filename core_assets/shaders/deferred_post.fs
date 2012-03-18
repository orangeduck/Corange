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
vec3 unsharp_mask(sampler2D input, vec2 coords, float strength, int width, int height);
vec3 chromatic_aberration(sampler2D input, vec2 coords, float offset);

/* End */

#define BLOOM_SIZE 4.0
#define BLOOM_AMOUNT 5.0
#define BLOOM_COLOR 3.0

vec3 pow3(vec3 col, float exponent) {
  vec3 ret;
  ret.r = pow(col.r, exponent);
  ret.g = pow(col.g, exponent);
  ret.b = pow(col.b, exponent);
  return ret;
}

void main() {
  
  vec3 color;
  if (aa_type == 1) {
	color = fxaa_unsharp(ldr_texture, gl_TexCoord[0].xy, width, height);
  } else {
    //color = texture2D(ldr_texture, gl_TexCoord[0].xy).rgb;
    //color = chromatic_aberration(ldr_texture, gl_TexCoord[0].xy, 1.0);
    color = unsharp_mask(ldr_texture, gl_TexCoord[0].xy, 0.5, width, height);
  }
  
	vec3 vignetting = texture2D(vignetting_texture, gl_TexCoord[0].xy).rgb;
  
  vec4 bloom_s = texture2DLod(ldr_texture, gl_TexCoord[0].xy, BLOOM_SIZE);
  vec3 bloom = BLOOM_AMOUNT * bloom_s.a * pow3(bloom_s.rgb, BLOOM_COLOR);
  
  color = (color + bloom) * mix(vignetting, vec3(1.0,1.0,1.0), 0.5);
  
	gl_FragColor.rgb = color_correction(color, lut, 64);
	gl_FragColor.a = 1.0;
} 