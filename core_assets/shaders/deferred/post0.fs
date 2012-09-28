#version 120

uniform int width;
uniform int height;

uniform sampler2D ldr_texture;
uniform sampler2D depth_texture;
uniform sampler2D random_texture;

/* Headers */

vec3 bokeh_dof(int width, int height, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords, float focalDepth);
vec3 basic_dof(int width, int height, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords);

/* End */

vec3 pow3(vec3 col, float exponent) {
  vec3 ret;
  ret.r = pow(col.r, exponent);
  ret.g = pow(col.g, exponent);
  ret.b = pow(col.b, exponent);
  return ret;
}

void main() {
  
  const float bloom_size = 4.0;
  const float bloom_amount = 5.0;
  const float bloom_color = 3.0;
  
  vec4 bloom_s = texture2DLod(ldr_texture, gl_TexCoord[0].xy, bloom_size);
  vec3 bloom = bloom_amount * bloom_s.a * pow3(bloom_s.rgb, bloom_color);
  
	gl_FragColor.rgb = bloom + bokeh_dof(width, height, ldr_texture, depth_texture, random_texture, gl_TexCoord[0].xy, -1.0);
	//gl_FragColor.rgb = bloom + texture2D(ldr_texture, gl_TexCoord[0].xy).rgb;
	gl_FragColor.a = 1.0;
  
} 