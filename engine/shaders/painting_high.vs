
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

uniform sampler2D background_paint;
uniform sampler2D background_color;
uniform sampler2D canvas_color;

varying vec2 uvs;
varying vec4 screen_position;
varying float backfacing;
varying float depth;
varying vec4 particle_color;

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
		
		screen_position = proj_matrix * view_matrix * world_position;
		
		vec2 screen_uvs = (screen_position.xy / screen_position.w) / 2 + 0.5;
		vec4 paint_data = texture2DLod(background_paint, screen_uvs, 0.0);
		float total_size = paint_data.x * paint_data.y;
		float hash_value = mod( (world_position.x + world_position.y + world_position.z) * 1216 , 2.0 );
		float dir_value = max(-normal_to_screen, 0.0);
		
		if ( hash_value < (1 - (dir_value * total_size)) ) {
			
			backfacing = 1.0;
			gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
			
		} else {
		
			vec4 screen_tangent = proj_matrix * view_matrix * vec4(face_tangent, 1.0);
			float a = -atan(screen_tangent.y, screen_tangent.x) + (1.57079633 * paint_data.a);
			
			mat2 rot = mat2(
				vec2(cos(a), -sin(a)),
				vec2(sin(a),  cos(a))
				);
			
			backfacing = 0.0;
			
			uvs = gl_MultiTexCoord0.xy;
			
			float scale = 2.0 * (skip + 2);
			
			paint_data.x = paint_data.x / 2;
			vec2 brush_size = mix( vec2(paint_data.x,paint_data.y), vec2(paint_data.y,paint_data.x), paint_data.a);
			
			vec2 detail_size = (1 - brush_size) * size * scale;
			
			vec4 color_image = texture2DLod(background_color, screen_uvs, 0.0);
			vec4 color_canvas = texture2DLod(canvas_color, screen_uvs, 0.0);
			
			particle_color = color_image * color_canvas;
			
			gl_Position = screen_position + vec4(rot * (gl_Vertex.xy * detail_size), 0.0, 0.0);
			depth = gl_Position.z / gl_Position.w;
		
		}
		
	}
	
	
} 