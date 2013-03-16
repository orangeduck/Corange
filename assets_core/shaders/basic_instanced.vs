#version 120

#define MAX_INSTANCES 128

uniform mat4 world_matricies[MAX_INSTANCES];

uniform mat4 proj_matrix;
uniform mat4 view_matrix;

varying vec3 normal;

void main() {
  normal = gl_Normal;
  gl_Position = proj_matrix * view_matrix * world_matricies[gl_InstanceID] * gl_Vertex;
}
