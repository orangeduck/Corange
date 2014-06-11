#version 120

uniform sampler2D attribmap;

uniform sampler2D ground0;
uniform sampler2D ground1;
uniform sampler2D ground2;
uniform sampler2D ground3;

uniform sampler2D ground0_nm;
uniform sampler2D ground1_nm;
uniform sampler2D ground2_nm;
uniform sampler2D ground3_nm;

uniform float clip_near;
uniform float clip_far;

varying vec3 fPosition;
varying mat4 fTBN;

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

void main( void ) {
  
  float bumpiness = 0.75;
  
	vec2 uvs = vec2(fPosition.x, fPosition.z) / 7;
	vec4 normal = texture2D(ground0_nm, uvs);
  
	normal.rgb = swap_red_green_inv(normal.rgb);
  normal = mix(vec4( 0.5, 0.5, 1.0, 1.0 ), normal, bumpiness);
	normal = (normal * 2.0 - vec4(1.0,1.0,1.0,0.0)) * fTBN;
	
  vec4 diffuse = texture2D(ground0, uvs);
  
	gl_FragData[0].rgb = from_gamma(diffuse.rgb);
	gl_FragData[0].a = 0.1;
	
	gl_FragData[1].rgb = normal.rgb;
	gl_FragData[1].a = 2.0 + (10.0 / 1000.0);
  
  gl_FragDepth = linear_depth(gl_FragCoord.z, clip_near, clip_far);
}
