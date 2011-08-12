uniform sampler2D background_color;
uniform sampler2D background_depth;
uniform sampler2D canvas_color;
uniform sampler2D brush;

uniform float opacity;

varying vec2 uvs;
varying vec4 screen_position;
varying float backfacing;
varying float depth;

#define grey vec3(0.5,0.5,0.5)

#define saturation 1.3

#define brightness vec3(0.25,0.25,0.25)

void main()
{
	if(backfacing == 1.0) {
		discard;
	}
	
	vec4 color_brush = texture2D(brush, uvs);
	
	if(color_brush.a < 0.1) {
		discard;
	}
	
	vec2 screen_uv = ( (screen_position.xy / screen_position.w) / 2) + 0.5;
	
	float old_depth = texture2D(background_depth, screen_uv).r;
	
	if (old_depth <= depth) {
		discard;
	}
	
	vec4 color_image = texture2D(background_color, screen_uv);
	vec4 color_canvas = texture2D(canvas_color, screen_uv);
	vec3 color = color_brush.rgb * color_image.rgb * color_canvas.rgb;
	//color = mix(grey, color, saturation);
	//color += brightness;

	gl_FragColor = vec4(color, color_brush.a * opacity);
}