#version 120

attribute vec3 tangent;
attribute vec3 binormal;
 
uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;
 
varying vec2 uvs;
varying vec4 world_position;
varying mat4 TBN;

void main() {
  
  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  mat3 world_rot = mat3(world_matrix);
  
  vec3 w_tangent = normalize(world_rot * tangent);
  vec3 w_binormal = normalize(world_rot * binormal);
  vec3 w_normal = normalize(world_rot * gl_Normal);
  
  TBN = mat4(vec4(w_tangent,  0.0),
             vec4(w_binormal, 0.0),
             vec4(w_normal,   0.0),
             vec4(0.0,0.0,0.0,1.0)
			 );
  
  world_position = world_matrix * gl_Vertex;
  
  gl_Position = proj_matrix * view_matrix * world_position;
  
}