#version 120

attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

uniform   mat4 world;
uniform   mat4 view;
uniform   mat4 proj;

varying   vec3 fNormal;
varying   vec3 fPosition;

void main( void )
{
  vec3 w_tangent  = mat3(world) * vTangent;
  vec3 w_binormal = mat3(world) * vBinormal;
  vec3 w_normal   = mat3(world) * vNormal;
  vec4 world_position = world * vec4(vPosition, 1);
  fPosition = world_position.xyz / world_position.w;
  mat3 TBN = transpose(mat3(normalize (w_tangent),
                            normalize (w_binormal),
                            normalize (w_normal)));
  fNormal = TBN * vNormal;
  gl_Position = proj * view * world_position;
}


