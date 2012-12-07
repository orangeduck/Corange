#version 120

varying vec2 texcoords;
varying vec4 color;

void main() {
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  texcoords = gl_MultiTexCoord0.xy;
  color = gl_Color;
}
