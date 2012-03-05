uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

varying vec3 position;

void main() {
  position = gl_Vertex.xyz;
  gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}
