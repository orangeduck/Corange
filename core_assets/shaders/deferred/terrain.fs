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

uniform float size_x;
uniform float size_y;

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
  
	vec2 uvs = vec2(position.x, position.z) / 7;
	vec2 world_uvs;
  world_uvs.x = 1 - (position.x / size_x + 0.5);
	world_uvs.y =     (position.z / size_y + 0.5);

  vec4 attrib = texture2D(attribmap, world_uvs);

	vec4 normal0 = texture2D(ground0_nm, uvs) * attrib.r;
	vec4 normal1 = texture2D(ground1_nm, uvs) * attrib.g;
	vec4 normal2 = texture2D(ground2_nm, uvs) * attrib.b;
	vec4 normal3 = texture2D(ground3_nm, uvs) * attrib.a;
	vec4 normal = normal0 + normal1 + normal2 + normal3;
  
	normal.rgb = swap_red_green_inv(normal.rgb);
  normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), 1.5);
	normal = normalize(normal * 2.0 - vec4(1.0,1.0,1.0,0.0)) * TBN;
	
  vec4 diffuse0 = texture2D(ground0, uvs) * attrib.r;
  vec4 diffuse1 = texture2D(ground1, uvs) * attrib.g;
  vec4 diffuse2 = texture2D(ground2, uvs) * attrib.b;
  vec4 diffuse3 = texture2D(ground3, uvs) * attrib.a;
  vec4 diffuse = diffuse0 + diffuse1 + diffuse2 + diffuse3;
  
	gl_FragData[0].rgb = from_gamma(diffuse.rgb);
	gl_FragData[0].a = 0.1;
	
	gl_FragData[1].rgb = position.xyz;
	gl_FragData[1].a = 2.0;
	
	gl_FragData[2] = normal;
	gl_FragData[2].a = 10.0;
  
  gl_FragDepth = linear_depth(gl_FragCoord.z, near, far);
}