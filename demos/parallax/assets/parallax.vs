#version 120


attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;

uniform vec3 camera_position;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec2 fTexcoord;
varying vec3 fColor;
varying vec3 fPosition;

varying vec3 Tpos;
varying vec3 Tcam;


void main( void )
{

  vec3 w_tangent  = mat3(world) * vTangent;
  vec3 w_binormal = mat3(world) * vBinormal;
  vec3 w_normal   = mat3(world) * vNormal;
  vec4 world_position = world * vec4(vPosition, 1);

  fColor = vec3(1.0, 1.0, 1.0);

  fTexcoord = vTexcoord;
  fPosition = world_position.xyz / world_position.w;

  mat3 TBN= transpose(mat3(normalize (w_tangent),
                           normalize (w_binormal),
                           normalize (w_normal)));

  Tcam = TBN * camera_position;
  Tpos = TBN * fPosition;

  gl_Position = proj * view * world_position;
}


