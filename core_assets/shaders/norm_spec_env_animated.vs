#version 120

attribute vec3 tangent;
attribute vec3 binormal;
attribute vec3 bone_indicies;
attribute vec3 bone_weights;
 
uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

uniform mat4 bone_world_matrices[32];
uniform int bone_count;

varying vec2 uvs;
varying vec4 world_position;
varying mat4 TBN;

varying vec3 normal;

void main() {

  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);

  vec4 blendpos = vec4(0,0,0,0);
  for (int i = 0; i < 3; i++) {
    blendpos += vec4((bone_world_matrices[int(bone_indicies[i])] * gl_Vertex).xyz, 1.0) * bone_weights[i];
  }
  
  vec3 blendnorm = vec3(0,0,0);
  for (int i = 0; i < 3; i++) {
    blendnorm += (mat3(bone_world_matrices[int(bone_indicies[i])]) * gl_Normal) * bone_weights[i];
  }
  
  vec3 blendtang = vec3(0,0,0);
  for (int i = 0; i < 3; i++) {
    blendtang += (mat3(bone_world_matrices[int(bone_indicies[i])]) * tangent) * bone_weights[i];
  }
  
  vec3 blendbinorm = vec3(0,0,0);
  for (int i = 0; i < 3; i++) {
    blendbinorm += (mat3(bone_world_matrices[int(bone_indicies[i])]) * binormal) * bone_weights[i];
  }
  
  vec4 w_position = world_matrix * blendpos;
  vec4 w_tangent = world_matrix * vec4(blendtang, 0.0);
  vec4 w_binormal = world_matrix * vec4(blendbinorm, 0.0);
  vec4 w_normal = world_matrix * vec4(blendnorm, 0.0);
  
  TBN = mat4( vec4(w_tangent),
			  vec4(w_binormal),
			  vec4(w_normal),
			  vec4(0.0,0.0,0.0,1.0)
			 );
  
  world_position = w_position;
  
  gl_Position = proj_matrix * view_matrix * w_position;
  
}