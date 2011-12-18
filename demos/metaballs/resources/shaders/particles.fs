uniform sampler2D particle_texture;

varying vec2 uvs;
varying float power;

void main()
{
	vec4 power_color = mix(vec4(0.53, 0.67, 0.86, 1.0), vec4(0.88, 0.38, 0.07, 1.0), power * 1.5);

	vec4 color = texture2D(particle_texture, uvs);
	gl_FragColor = color * power_color * 0.5 * power;
}