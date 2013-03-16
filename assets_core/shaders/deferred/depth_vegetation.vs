#version 120

attribute mat4 vWorld;
attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec4 vColor;

uniform mat4 proj;
uniform mat4 view;

uniform float time;

varying vec2 fTexcoord;
varying float fDepth;

vec4 smooth_curve(vec4 x) {
	return x * x * (3.0 - 2.0 * x); 
}

vec4 triangle_wave(vec4 x) {
	return abs( fract(x + 0.5) * 2.0 - 1.0);
}

vec4 smooth_triangle_wave(vec4 x) {
	return smooth_curve(triangle_wave(x));
}

void main() {

  fTexcoord = vTexcoord;

	vec4 amplitude  = vColor.r * vec4(0.058512, 0.02282 * vColor.b, 0.062, 1.0);
	vec4 wavelength = vec4(18.213, 1.124, 10.181, 1.0);
	vec4 speed      = vec4(0.1976, 1.2134, 0.86, 1.0);
	
	vec4 wave = amplitude * (0.5 - smooth_triangle_wave(
		( vec4( vPosition.x, vPosition.y, vPosition.z, 0) / wavelength) +
		( vec4( time, time, time, 0) * speed ) +
		( vec4( vColor.g, vColor.g, vColor.g, 0) )
  ));
	
  vec4 world_position = vWorld * vec4(vPosition + wave.xyz, 1);
  
  vec4 screen_position = proj * view * world_position;
  fDepth = screen_position.z / screen_position.w;
  gl_Position = screen_position;
}