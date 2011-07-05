uniform sampler2D background_depth;
uniform sampler2D brush;

varying vec2 uvs;
varying vec4 screen_position;
varying float backfacing;
varying float depth;
varying vec4 particle_color;

void main()
{
	if(backfacing == 1.0) {
		discard;
	}
	
	vec4 color_brush = texture2D(brush, uvs);
	
	if(color_brush.a <= 0.1) {
		discard;
	}
	
	vec2 screen_uv = ( (screen_position.xy / screen_position.w) / 2) + 0.5;
	
	float old_depth = texture2D(background_depth, screen_uv).r;
	
	if (old_depth <= depth) {
		discard;
	}
	
	vec3 color = color_brush.rgb * particle_color.rgb;

	gl_FragColor = vec4(color, color_brush.a);
}