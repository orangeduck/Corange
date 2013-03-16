#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec4 vColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec2 fTexcoord;
varying vec4 fColor;

void main() {
  fColor = vColor;
  fTexcoord = vTexcoord;
  gl_Position = proj * view * world * vec4(vPosition, 1);
}
