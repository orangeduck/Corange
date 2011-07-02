uniform sampler2D background_color;
uniform sampler2D background_depth;
uniform sampler2D brush;

varying vec2 uvs;
varying vec4 screen_position;
varying float backfacing;
varying float depth;

void main()
{
	if(backfacing == 1.0) {
		discard;
	}
	
	vec2 screen_uv = ( (screen_position.xy / screen_position.w) / 2) + 0.5;
	
	float old_depth = texture2D(background_depth, screen_uv).r;
	
	if (old_depth < depth) {
		discard;
	}
	
	vec4 col = texture2D(brush, uvs);
	
	vec3 color = col.rgb * texture2D(background_color, screen_uv).rgb;

	gl_FragColor = vec4(color, col.a);
}