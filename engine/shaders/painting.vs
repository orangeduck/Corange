
attribute vec3 face_position;
attribute vec3 face_normal;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

uniform vec3 eye_position;

uniform float density;
uniform float skip;

varying vec2 uvs;
varying vec4 screen_position;
varying float backfacing;
varying float depth;

void main()
{
	
	vec4 world_position = world_matrix * vec4(face_position,1.0);

	vec3 eye_dir = normalize(world_position.xyz - eye_position);
	vec3 normal = normalize(face_normal);
	
	float normal_to_screen = dot(eye_dir, normal);
	
	if( normal_to_screen > 0.0 ) {
		
		backfacing = 1.0;
		gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
		
	} else {
		
		backfacing = 0.0;
		
		float size = 30.0 * density * normal_to_screen * normal_to_screen + 0.25;
		
		vec2 temp_uvs = vec2(gl_MultiTexCoord0);
		uvs = vec2(temp_uvs.x, -temp_uvs.y);
		
		screen_position = proj_matrix * view_matrix * world_position;
		
		depth = gl_Position.z / gl_Position.w;
		gl_Position = (gl_Vertex * size) + screen_position;
		
	}
	
	
} 