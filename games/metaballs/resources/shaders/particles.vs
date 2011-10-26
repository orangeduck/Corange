
attribute vec4 particle_position;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;

varying vec2 uvs;

void main()
{
  uvs = gl_MultiTexCoord0.xy;

	vec4 screen_position = proj_matrix * view_matrix * vec4(particle_position.xyz * 100.0, 1.0);
	
	gl_Position = screen_position + vec4((gl_Vertex.xy * 3.0), 0.0, 0.0);
	
} 