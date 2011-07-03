
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
		float a = acos( dot(normalize(screen_tangent.xy), vec2(0.0,1.0) ) );
		
		mat4 rot = mat4(
			vec4(cos(a), -sin(a), 0, 0),
			vec4(sin(a),  cos(a), 0, 0),
			vec4(0     ,       0, 1, 0),
			vec4(0     ,       0, 0, 1)
			);
		
		backfacing = 0.0;
		
		float scale = 1000.0 * density * skip * max(-normal_to_screen, 0.0) + 0.075;
		
		uvs = gl_MultiTexCoord0.xy;
		
		screen_position = proj_matrix * view_matrix * world_position;
		
		depth = gl_Position.z / gl_Position.w;
		gl_Position = rot * (gl_Vertex * vec4( size, 1.0, 1.0 ) * scale) + screen_position;
		
	}
	
	
} 