#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

attribute vec3 vBone_indicies;
attribute vec3 vBone_weights;

uniform mat4 world_bones[32];
uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec3 fPosition;
varying vec3 fColor;
varying vec2 fTexcoord;
varying mat4 fTBN;

void main(){
  
  fTexcoord = vTexcoord;
  fColor = vec3(1, 1, 1);

  vec4 blendpos = vec4(0.0,0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendpos += vec4((world_bones[int(vBone_indicies[i])] * vec4(vPosition, 1)).xyz, 1.0) * vBone_weights[i];
  }
  
  vec3 blendnorm = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendnorm += (mat3(world_bones[int(vBone_indicies[i])]) * vNormal) * vBone_weights[i];
  }
  
  vec3 blendtang = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendtang += (mat3(world_bones[int(vBone_indicies[i])]) * vTangent) * vBone_weights[i];
  }
  
  vec3 blendbinorm = vec3(0.0,0.0,0.0);
  for (int i = 0; i < 3; i++) {
    blendbinorm += (mat3(world_bones[int(vBone_indicies[i])]) * vBinormal) * vBone_weights[i];
  }
  
  vec4 world_pos = world * blendpos;
  
  fPosition = world_pos.xyz / world_pos.w;
  gl_Position = proj * view * world_pos;
  
  blendnorm   = mat3(world) * blendnorm;
  blendtang   = mat3(world) * blendtang;
  blendbinorm = mat3(world) * blendbinorm;
  
  fTBN = mat4(
    blendtang.x, blendbinorm.x, blendnorm.x, 0.0,
    blendtang.y, blendbinorm.y, blendnorm.y, 0.0,
    blendtang.z, blendbinorm.z, blendnorm.z, 0.0,
    0.0, 0.0, 0.0, 1.0 );
}