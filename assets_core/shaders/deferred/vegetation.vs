#version 120

attribute mat4 vWorld;
attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;
attribute vec4 vColor;

uniform mat4 view;
uniform mat4 proj;

uniform float time;

varying vec2 fTexcoord;
varying vec3 fPosition;
varying vec3 fColor;
varying mat4 fTBN;

vec4 smooth_curve(vec4 x) {
	return x * x * (3.0 - 2.0 * x); 
}

vec4 triangle_wave(vec4 x) {
	return abs( fract(x + 0.5) * 2.0 - 1.0);
}

vec4 smooth_triangle_wave(vec4 x) {
	return smooth_curve(triangle_wave(x));
}

void main( void ) {
  
  vec3 w_tangent  = mat3(vWorld) * vTangent;
  vec3 w_binormal = mat3(vWorld) * vBinormal;
  vec3 w_normal   = mat3(vWorld) * vNormal;
  
  fTBN = mat4(
    w_tangent.x, w_binormal.x, w_normal.x, 0.0,
    w_tangent.y, w_binormal.y, w_normal.y, 0.0,
    w_tangent.z, w_binormal.z, w_normal.z, 0.0,
    0.0, 0.0, 0.0, 1.0 );
  
  float ambient = clamp(vColor.a * 2 + 0.25, 0.0, 1.0);
  fColor = vec3(ambient, ambient, ambient);
  
	vec4 amplitude  = vColor.r * vec4(0.058512, 0.02282 * vColor.b, 0.062, 1.0);
	vec4 wavelength = vec4(24.213, 1.124, 18.181, 1.0);
	vec4 speed      = vec4(0.1976, 1.2134, 0.26, 1.0);
	
	vec4 wave = amplitude * (0.5 - smooth_triangle_wave(
		( vec4( vPosition.x, vPosition.y, vPosition.z, 0) / wavelength) +
		( vec4( time, time, time, 0) * speed ) +
		( vec4( vColor.g, vColor.g, vColor.g, 0) )
  ));
	
  vec4 world_position = vWorld * vec4(vPosition + wave.xyz, 1);
  
  fTexcoord = vTexcoord;
  fPosition = world_position.xyz / world_position.w;
  gl_Position = proj * view * world_position;
  
} 