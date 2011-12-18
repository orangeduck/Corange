
attribute float brush_id;
attribute vec3 face_position;
attribute vec3 face_direction;

uniform sampler2D background_color;
uniform sampler2D background_depth;
uniform sampler2D background_normal;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

uniform vec3 eye_position;

varying vec4 screen_position;
varying vec4 position;
varying vec2 uvs;

void main()
{
	vec4 world_position = world_matrix * vec4(face_position,1.0);

	screen_position = proj_matrix * view_matrix * world_position;
	vec2 screen_uvs = (screen_position.xy / screen_position.w / 2.0) + 0.5;

	vec3 camera_dir =  normalize(gl_Vertex.xyz - eye_position);
	
	vec3 normal_tex = texture2DLod(background_normal, screen_uvs, 0.0).rgb;	
	vec3 normal = normalize( (normal_tex - 0.5) * 2 );
	
	float cam_dot_norm = max(dot( -camera_dir , normal ), 0.0);
	
	float uv_part = 1.0 / 16.0;
	
	float x_index = floor(mod(brush_id, 16.0));
	float y_index = floor(brush_id / 16.0);
	
	uvs = vec2( uv_part * (x_index + gl_MultiTexCoord0.x) , uv_part * (y_index + gl_MultiTexCoord0.y));
	
	gl_Position = screen_position + vec4((gl_Vertex.xy * 3.0 * cam_dot_norm), 0.0, 0.0);
	position = gl_Position;
	
} 