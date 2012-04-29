#version 120

varying vec4 position;

void main() {
	position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_Position = position;
}