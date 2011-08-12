
attribute vec3 face_position;
attribute vec3 face_normal;
attribute vec3 face_tangent;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

uniform vec3 eye_position;

uniform float density;
uniform float skip;
uniform vec2 size;

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
		
		vec4 screen_tangent = proj_matrix * view_matrix * vec4(face_tangent, 1.0);
		
		float a = -atan(screen_tangent.y, screen_tangent.x);
		
		mat2 rot = mat2(
			vec2(cos(a), -sin(a)),
			vec2(sin(a),  cos(a))
			);
		
		backfacing = 0.0;
		
		uvs = gl_MultiTexCoord0.xy;
		
		screen_position = proj_matrix * view_matrix * world_position;
		
		float scale = 1.25 * (skip + 2) * max(-normal_to_screen, 0.0);
		
		gl_Position = screen_position + vec4(rot * (gl_Vertex.xy * size * scale), 0.0, 0.0);
		depth = gl_Position.z / gl_Position.w;
		
	}
	
	
} 