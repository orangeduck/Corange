#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

attribute vec3 vBone_indicies;
attribute vec3 vBone_weights;

uniform vec4 quat_reals[64];
uniform vec4 quat_duals[64];

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec3 fPosition;
varying vec3 fColor;
varying vec2 fTexcoord;
varying mat4 fTBN;

vec3 quat_dual_mul_pos(vec4 real, vec4 dual, vec3 v) {
  return v + 2 * cross(real.xyz, cross(real.xyz, v) + real.w*v) +
             2 * (real.w * dual.xyz - dual.w * real.xyz + cross(real.xyz, dual.xyz));
}
 
vec3 quat_dual_mul_rot(vec4 real, vec4 dual, vec3 v) {
  return v + 2 * cross(real.xyz, cross(real.xyz, v) + real.w*v);
}

void main() {
  
  fTexcoord = vTexcoord;
  fColor = vec3(1, 1, 1);
  
  vec4 real = quat_reals[int(vBone_indicies.x)] * vBone_weights.x +
              quat_reals[int(vBone_indicies.y)] * vBone_weights.y +
              quat_reals[int(vBone_indicies.z)] * vBone_weights.z;
  
  vec4 dual = quat_duals[int(vBone_indicies.x)] * vBone_weights.x +
              quat_duals[int(vBone_indicies.y)] * vBone_weights.y +
              quat_duals[int(vBone_indicies.z)] * vBone_weights.z;
  
  dual = dual / length(real);
  real = real / length(real);
  
  vec3 blendpos    = quat_dual_mul_pos(real, dual, vPosition);
  vec3 blendnorm   = quat_dual_mul_rot(real, dual, vNormal);
  vec3 blendtang   = quat_dual_mul_rot(real, dual, vTangent);
  vec3 blendbinorm = quat_dual_mul_rot(real, dual, vBinormal);
  
  blendnorm   = mat3(world) * blendnorm;
  blendtang   = mat3(world) * blendtang;
  blendbinorm = mat3(world) * blendbinorm;
  
  fTBN = mat4(
    blendtang.x, blendbinorm.x, blendnorm.x, 0.0,
    blendtang.y, blendbinorm.y, blendnorm.y, 0.0,
    blendtang.z, blendbinorm.z, blendnorm.z, 0.0,
    0.0, 0.0, 0.0, 1.0 );
    
  vec4 world_pos = world * vec4(blendpos, 1.0);
  
  fPosition = world_pos.xyz / world_pos.w;
  gl_Position = proj * view * world_pos;
    
}