attribute vec3 tangent;
attribute vec3 binormal;
 
uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

varying vec2 uvs;
varying mat4 TBN;

void main() {
  
  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  vec3 normal = vec3(gl_Normal);

  TBN = mat4( tangent.x, binormal.x, normal.x, 0.0,
			  tangent.y, binormal.y, normal.y, 0.0,
			  tangent.z, binormal.z, normal.z, 0.0,
		      0.0, 0.0, 0.0, 1.0 );

  //gl_Position = gl_ModelViewProjectionMatrix * world_matrix * gl_Vertex;
  gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
  
}