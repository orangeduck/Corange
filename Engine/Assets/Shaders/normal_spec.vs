attribute vec3 tangent;
attribute vec3 binormal;

uniform vec3 light_position;
uniform vec3 eye_position;
 
varying vec3 light_vector; 
varying vec3 half_angle; 
varying vec2 uvs;

void main() {

  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  mat3 rotation = mat3(tangent, binormal, gl_Normal); 
  
  vec3 eye_dir = normalize(eye_position - gl_Position.xyz);
	
  light_vector = light_position * rotation;
  half_angle = normalize(eye_dir + light_position) * rotation;
  
  gl_Position = ftransform();
  
}