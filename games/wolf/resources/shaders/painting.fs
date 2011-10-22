uniform sampler2D background_color;
uniform sampler2D background_depth;
uniform sampler2D background_normal;

uniform sampler2D brush;

varying vec2 uvs;
varying vec4 position;
varying vec4 screen_position;

#define grey vec3(0.5,0.5,0.5)

#define saturation 1.3

#define brightness vec3(0.25,0.25,0.25)

void main()
{
	
	vec4 color_brush = texture2D(brush, uvs);
	
	if(color_brush.a < 0.5) {
		discard;
	}
	
	vec2 screen_uv = ( (screen_position.xy / screen_position.w) / 2) + 0.5;
	
	float old_depth = texture2D(background_depth, screen_uv).r + 0.01;
	
	if (old_depth <= gl_FragCoord.z) {
		discard;
	}
	
	vec3 color = color_brush.rgb * textureLod(background_color, screen_uv, 5).rgb;
	//color = mix(grey, color, saturation);
	//color += brightness;

	gl_FragColor = vec4(color, color_brush.a);
	//gl_FragColor = texture2D(background_normal, screen_uv);
}