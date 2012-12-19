#version 120

attribute vec3 vPosition;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec3 fPosition;

void main() {
  vec4 screen_position = proj * view * world * vec4(vPosition, 1);
	fPosition = screen_position.xyz / screen_position.w;
  gl_Position = screen_position;
} 