#version 120

uniform sampler2D diffuse_texture;

varying vec2 fTexcoord;
varying vec4 fColor;

void main() {
  
  vec4 diffuse = texture2D(diffuse_texture, fTexcoord) * fColor;

	gl_FragColor = diffuse;
}
