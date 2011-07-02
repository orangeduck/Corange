attribute vec3 tangent;
attribute vec3 binormal;
 
uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;
 
varying vec2 uvs;
varying vec3 world_position;
varying mat4 TBN;

void main() {
  
  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  vec4 w_position = gl_Vertex;
  vec4 w_tangent = vec4(tangent, 0.0);
  vec4 w_binormal = vec4(binormal, 0.0);
  vec4 w_normal = vec4(gl_Normal, 0.0);
  
  TBN = mat4( vec4(w_tangent),
			  vec4(w_binormal),
			  vec4(w_normal),
			  vec4(0.0,0.0,0.0,1.0)
			 );
  
  world_position = w_position.xyz / w_position.w;
  
  gl_Position = proj_matrix * view_matrix * w_position;
  
}