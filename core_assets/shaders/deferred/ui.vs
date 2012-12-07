#version 120

varying vec2 texcoords;
varying vec4 color;

void main() {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  color = gl_Color;
  texcoords = gl_MultiTexCoord0.xy;
}
