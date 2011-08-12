uniform sampler2D background_depth;
uniform sampler2D brush;

varying vec2 uvs;
varying vec2 screen_uvs;
varying float backfacing;
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
	
	float old_depth = texture2D(background_depth, screen_uvs).r + 0.001;
	if (old_depth <= gl_FragCoord.z) {
		discard;
	}
	
	vec3 color = color_brush.rgb * particle_color.rgb;

	gl_FragColor = vec4(color, color_brush.a);
}