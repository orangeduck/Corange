#version 120

attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

attribute vec3 bone_indicies;
attribute vec3 bone_weights;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

uniform mat4 bone_world_matrices[32];
uniform int bone_count;

varying vec4 position;
varying mat4 TBN;

void main(){
  
  gl_TexCoord[0] = gl_MultiTexCoord0;

  vec4 blendpos = vec4(0.0,0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendpos += vec4((bone_world_matrices[int(bone_indicies[i])] * gl_Vertex).xyz, 1.0) * bone_weights[i];
  }
  
  vec3 blendnorm = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendnorm += (mat3(bone_world_matrices[int(bone_indicies[i])]) * normal) * bone_weights[i];
  }
  
  vec3 blendtang = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendtang += (mat3(bone_world_matrices[int(bone_indicies[i])]) * tangent) * bone_weights[i];
  }
  
  vec3 blendbinorm = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendbinorm += (mat3(bone_world_matrices[int(bone_indicies[i])]) * binormal) * bone_weights[i];
  }
  
  position = world_matrix * blendpos;
  gl_Position = proj_matrix * view_matrix * position;
  
  blendnorm = mat3(world_matrix) * blendnorm;
  blendtang = mat3(world_matrix) * blendtang;
  blendbinorm = mat3(world_matrix) * blendbinorm;
  
  TBN = mat4(blendtang.x, blendbinorm.x, blendnorm.x, 0.0,
             blendtang.y, blendbinorm.y, blendnorm.y, 0.0,
             blendtang.z, blendbinorm.z, blendnorm.z, 0.0,
             0.0, 0.0, 0.0, 1.0 );
}