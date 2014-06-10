#version 120

//uniform int width;
//uniform int height;

uniform sampler2D ldr_texture;
//uniform sampler2D depth_texture;
//uniform sampler2D random_texture;

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

vec3 pow3(vec3 col, float exponent) {
  vec3 ret;
  ret.r = pow(col.r, exponent);
  ret.g = pow(col.g, exponent);
  ret.b = pow(col.b, exponent);
  return ret;
}

void main() {
  
  float bloom_amount = 5.0;
  float bloom_color = 3.0;
  
  vec4 bloom_s = texture2D(ldr_texture, fTexcoord);
  vec3 bloom = bloom_amount * bloom_s.a * pow3(bloom_s.rgb, bloom_color);
  
  //float focal_depth = 0.0015;
  float focal_depth = 0.0;
  
	//gl_FragColor.rgb = bloom + bokeh_dof(width, height, ldr_texture, depth_texture, random_texture, gl_TexCoord[0].xy, focal_depth);
	gl_FragColor.rgb = to_gamma(bloom + texture2D(ldr_texture, fTexcoord).rgb);
	gl_FragColor.a = 1.0;
  
} 