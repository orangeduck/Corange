#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec2 fTexcoord;

void main() {
  fTexcoord = vTexcoord;
  gl_Position = proj * view * world * vec4(vPosition, 1);
}