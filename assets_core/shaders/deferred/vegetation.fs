#version 120

uniform sampler2D diffuse_map;
uniform sampler2D bump_map;
uniform sampler2D spec_map;

uniform float glossiness;
uniform float bumpiness;
uniform float specular_level;
uniform float alpha_test;
uniform int material;
uniform float fade;

uniform float clip_near;
uniform float clip_far;

varying vec2 fTexcoord;
varying vec3 fColor;
varying vec3 fPosition;
varying mat4 fTBN;

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

vec3 swap_red_green_inv(vec3 color) {
  float temp = 1.0-color.r;
  color.r = 1.0-color.g;
  color.g = temp;
  return color;
}

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

void main(void) {
  
	vec2 uvs = vec2(fTexcoord.x, -fTexcoord.y);

  vec4 diffuse = texture2D(diffuse_map, uvs);
	float spec = texture2D(spec_map, uvs).r * specular_level;
	vec4 normal = texture2D(bump_map, uvs);
  
  if (diffuse.a * fade < alpha_test) { discard; }
	
	normal.rgb = swap_red_green_inv(normal.rgb);
  normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), bumpiness);
	normal = (normal * 2.0 - vec4(1.0,1.0,1.0,0.0)) * fTBN;
  
	gl_FragData[0].rgb = from_gamma(diffuse.rgb) * fColor;
	gl_FragData[0].a = spec;
	
	gl_FragData[1].rgb = normal.rgb;
	gl_FragData[1].a = float(material) + glossiness / 1000;
  
  gl_FragDepth = linear_depth(gl_FragCoord.z, clip_near, clip_far);
  
}