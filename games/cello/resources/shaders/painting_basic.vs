uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;
 
varying vec2 uvs;
varying vec4 world_position;
varying vec4 world_normal;

void main() {
  
  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  world_position = gl_Vertex;
  world_normal = vec4(gl_Normal, 1.0);
  
  gl_Position = proj_matrix * view_matrix * world_position;
  
}