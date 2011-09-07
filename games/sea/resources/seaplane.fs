uniform vec3 light_position;
uniform vec3 eye_position;

uniform sampler2D tex_noise1;
uniform sampler2D tex_noise2;
uniform sampler2D tex_noise3;
uniform sampler2D tex_noise4;
uniform sampler2D tex_noise5;

uniform sampler2D tex_skybox;

uniform sampler2D tex_calm_water;
uniform sampler2D tex_foam_water;

varying vec3 normal;
varying vec3 position;

uniform float time;

void main()
{
	//vec3 calm_water = texture2D(tex_calm_water, position.xz / 1).rgb;
	//vec3 foam_water = texture2D(tex_foam_water, position.xz / 1).rgb;

	vec3 surface_normal = normal;

	vec2 bump_normal1 = texture2D(tex_noise1, position.xz / 25 + vec2(0.05, 0.05) * time * 1.2).rg;
	vec2 bump_normal2 = texture2D(tex_noise2, position.xz / 31 + vec2(-0.05,-0.05) * time * 1.41).rg;
	vec2 bump_normal3 = texture2D(tex_noise3, position.xz / 18 + vec2(0.051, 0.034) * time * 0.86).rg;
	vec2 bump_normal4 = texture2D(tex_noise4, position.xz / 44 + vec2(0.062, -0.012) * time * 0.99).rg;
	vec2 bump_normal5 = texture2D(tex_noise5, position.xz / 63 + vec2(-0.061, 0.081) * time * 1.1).rg;
	
	surface_normal.xz += 0.24 * bump_normal1;
	surface_normal.xz += 0.18 * bump_normal2;
	surface_normal.xz += 0.31 * bump_normal3;
	surface_normal.xz += 0.34 * bump_normal4;
	surface_normal.xz += 0.22 * bump_normal5;
	
	surface_normal.xz *= 3;
	
	surface_normal = normalize(surface_normal);
	
	vec3 light_dir = normalize(position - light_position);
	vec3 eye_dir = normalize(position - eye_position);
	
	float eye_dot_normal = clamp( abs(dot(eye_dir, surface_normal)) , 0.8, 0.9);
	
	vec3 reflected = reflect(-eye_dir, surface_normal);
	
	vec2 reflected_coord = vec2(reflected.x + reflected.z , reflected.y);
	
	vec3 env_texture = texture2D( tex_skybox, reflected_coord ).rgb;
	
	float light = max(dot(-light_dir, surface_normal),0);
	float spec = 0.2 * pow(max(dot(light_dir, -reflected),0),50);
	
	vec3 fog = (light + 1.0) * vec3(0.169, 0.239, 0.204);
	
	vec3 final = mix( env_texture, fog , eye_dot_normal ) + spec;
	//final = mix( final, foam1, max(position.y * foam1 - 2.0, 0) );
	
    gl_FragColor = vec4(final,1.0);
}