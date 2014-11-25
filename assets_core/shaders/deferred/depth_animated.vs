#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vBone_indicies;
attribute vec3 vBone_weights;

uniform vec4 quat_reals[64];
uniform vec4 quat_duals[64];

uniform mat4 world;
uniform mat4 proj;
uniform mat4 view;

varying vec2 fTexcoord;
varying float fDepth;

vec3 quat_dual_mul_pos(vec4 real, vec4 dual, vec3 v) {
  return v + 2 * cross(real.xyz, cross(real.xyz, v) + real.w*v) +
             2 * (real.w * dual.xyz - dual.w * real.xyz + cross(real.xyz, dual.xyz));
}

void main() {

  fTexcoord = vTexcoord;

  vec4 real = quat_reals[int(vBone_indicies.x)] * vBone_weights.x +
              quat_reals[int(vBone_indicies.y)] * vBone_weights.y +
              quat_reals[int(vBone_indicies.z)] * vBone_weights.z;
  
  vec4 dual = quat_duals[int(vBone_indicies.x)] * vBone_weights.x +
              quat_duals[int(vBone_indicies.y)] * vBone_weights.y +
              quat_duals[int(vBone_indicies.z)] * vBone_weights.z;
  
  dual = dual / length(real);
  real = real / length(real);
  
  vec3 blendpos = quat_dual_mul_pos(real, dual, vPosition);
  
  vec4 screen_position = proj * view * world * vec4(blendpos, 1.0);
  fDepth = screen_position.z / screen_position.w;
  gl_Position = screen_position;
}