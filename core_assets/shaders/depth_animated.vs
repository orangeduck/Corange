#version 120

attribute vec3 bone_indicies;
attribute vec3 bone_weights;

uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

uniform mat4 bone_world_matrices[32];
uniform int bone_count;

void main() {

  gl_TexCoord[0] = gl_MultiTexCoord0;

  vec4 blendpos = vec4(0,0,0,0);
  for (int i = 0; i < 3; i++) {
    blendpos += vec4((bone_world_matrices[int(bone_indicies[i])] * gl_Vertex).xyz, 1.0) * bone_weights[i];
  }
  
  gl_Position = proj_matrix * view_matrix * world_matrix * blendpos;
}