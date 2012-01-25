uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

void main() {
  gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}
