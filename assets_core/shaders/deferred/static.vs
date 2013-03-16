#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec2 fTexcoord;
varying vec3 fColor;
varying vec3 fPosition;
varying mat4 fTBN;

void main( void ) {
  
  vec3 w_tangent  = mat3(world) * vTangent;
  vec3 w_binormal = mat3(world) * vBinormal;
  vec3 w_normal   = mat3(world) * vNormal;
  
  fTBN = mat4(
    w_tangent.x, w_binormal.x, w_normal.x, 0.0,
    w_tangent.y, w_binormal.y, w_normal.y, 0.0,
    w_tangent.z, w_binormal.z, w_normal.z, 0.0,
    0.0, 0.0, 0.0, 1.0 );
  
  vec4 world_position = world * vec4(vPosition, 1);
  
  fColor = vec3(1.0, 1.0, 1.0);
  fTexcoord = vTexcoord;
  fPosition = world_position.xyz / world_position.w;
  gl_Position = proj * view * world_position;
  
} 