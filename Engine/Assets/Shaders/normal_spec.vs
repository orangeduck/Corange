attribute vec3 tangent;
attribute vec3 binormal;

uniform vec3 light_position;
uniform vec3 eye_position;
 
varying vec4 light_vector; 
varying vec4 half_angle; 
varying vec2 uvs;

void main() {

  vec2 temp_uvs = vec2(gl_MultiTexCoord0);
  uvs = vec2(temp_uvs.x, -temp_uvs.y);
  
  mat4 rotation = mat4( vec4(tangent, 0.0),
						vec4(binormal, 0.0),
						vec4(gl_Normal, 0.0),
						vec4(0.0,0.0,0.0,1.0)
						); 
  
  vec4 eye_dir = vec4( normalize(eye_position.xyz - gl_Position.xyz) , 1.0);
  vec4 light_dir = vec4( normalize(light_position.xyz - gl_Position.xyz) , 1.0);
	
  light_vector = light_dir * rotation;
  half_angle = normalize(eye_dir + light_dir) * rotation;
  
  gl_Position = ftransform();
  
}