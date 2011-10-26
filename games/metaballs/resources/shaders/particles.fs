uniform sampler2D particle_texture;

varying vec2 uvs;

void main()
{
	vec4 color = texture2D(particle_texture, uvs);
	gl_FragColor = color * 0.1;
}