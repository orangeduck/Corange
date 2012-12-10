#version 120

uniform sampler2D random_texture;
uniform sampler2D depth_texture;
uniform sampler2D normals_texture;
uniform float seed;

/* Headers */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture, float seed);
float ssao(vec2 texcoords, sampler2D depth_texture, sampler2D normals_texture, sampler2D random_texture, float seed);

/* End */

void main() {
	
  float ssao = ssao(gl_TexCoord[0].xy, depth_texture, normals_texture, random_texture, seed);
  gl_FragColor = vec4(ssao, ssao, ssao, 1.0);
	
} 