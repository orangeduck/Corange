#version 120

uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

void main() {
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}