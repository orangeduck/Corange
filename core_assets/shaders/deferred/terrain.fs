#version 120

varying vec4 position;
varying mat4 TBN;

uniform sampler2D attribmap;

uniform sampler2D ground0;
uniform sampler2D ground1;
uniform sampler2D ground2;
uniform sampler2D ground3;

uniform sampler2D ground0_nm;
uniform sampler2D ground1_nm;
uniform sampler2D ground2_nm;
uniform sampler2D ground3_nm;

uniform float near;
uniform float far;

/* Headers */

vec3 from_gamma(vec3 color);
vec3 swap_red_green_inv(vec3 color);

/* End */

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

void main( void ) {
  
	vec2 uvs = vec2(position.x, position.z) / 5;
	
	vec4 normal = texture2D(ground0_nm, uvs);
	normal.rgb = swap_red_green_inv(normal.rgb);
  
  normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), 1.5);
	normal = (normal * 2.0 - vec4(1.0,1.0,1.0,0.0)) * TBN;
	
  vec4 diffuse = texture2D(ground0, uvs);
  
	gl_FragData[0].rgb = from_gamma(diffuse.rgb);
	gl_FragData[0].a = 0.1;
	
	gl_FragData[1].rgb = position.xyz;
	gl_FragData[1].a = 2.0;
	
	gl_FragData[2] = normal;
	gl_FragData[2].a = 10.0;
  
  gl_FragDepth = linear_depth(gl_FragCoord.z, near, far);
}