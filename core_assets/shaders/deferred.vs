attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

varying vec4 position;
varying mat4 TBN;

void main( void ) {
  
  gl_TexCoord[0] = gl_MultiTexCoord0;
  
  position = world_matrix * gl_Vertex;

  mat3 world_rot = mat3(world_matrix);
  
  vec3 w_tangent = normalize(world_rot * tangent);
  vec3 w_binormal = normalize(world_rot * binormal);
  vec3 w_normal = normalize(world_rot * normal);
  
  TBN = mat4( w_tangent.x, w_binormal.x, w_normal.x, 0.0,
              w_tangent.y, w_binormal.y, w_normal.y, 0.0,
              w_tangent.z, w_binormal.z, w_normal.z, 0.0,
              0.0, 0.0, 0.0, 1.0 );
             
  gl_Position = proj_matrix * view_matrix * position;
} 