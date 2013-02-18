#version 120

attribute vec3 vPosition;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying float fDepth;

void main() {
  vec4 screen_position = proj * view * world * vec4(vPosition, 1);
  fDepth = screen_position.z / screen_position.w;
  gl_Position = screen_position;
}