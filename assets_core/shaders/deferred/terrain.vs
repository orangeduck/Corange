#version 120

attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

uniform mat4 world;
uniform mat3 world_normal;
uniform mat4 view;
uniform mat4 proj;

varying vec3 fPosition;
varying mat4 fTBN;

void main( void ) {
  
  vec4 w_position = world * vec4(vPosition, 1);
  vec3 w_tangent  = world_normal * vTangent;
  vec3 w_binormal = world_normal * vBinormal;
  vec3 w_normal   = world_normal * vNormal;
  
  fTBN = mat4(
    w_tangent.x, w_binormal.x, w_normal.x, 0.0,
    w_tangent.y, w_binormal.y, w_normal.y, 0.0,
    w_tangent.z, w_binormal.z, w_normal.z, 0.0,
    0.0, 0.0, 0.0, 1.0 );
  
  fPosition = w_position.xyz / w_position.w;
  gl_Position = proj * view * w_position;
}
