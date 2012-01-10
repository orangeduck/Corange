uniform sampler2D noise_texture;
uniform float time;

varying vec2 uvs;

struct gradient {
  vec3 start;
  vec3 stop;
};

gradient gradient_new(vec3 start, vec3 stop) {
  gradient g;
  g.start = start;
  g.stop = stop;
  return g;
}

vec3 gradient_lookup(gradient g, float amount) {
  return mix(g.start, g.stop, amount);
}

float perlin_noise(sampler2D noise, vec2 uvs, int octaves) {
  
  float total = 0;
  for(int i = 0; i < octaves; i++) {
    float amplitude = pow( 0.5, octaves-i );
    float wavelength = pow(2,i);
    total += amplitude * texture2D( noise, uvs / wavelength ).g;
  }
  
  return total;
}

void main()
{
  vec2 offset = uvs;

  vec2 first, second;

  first.x = perlin_noise(noise_texture, offset + vec2(0.0, 0.0) * vec2(0.101,0.13) * time * 1.1, 6);
  first.y = perlin_noise(noise_texture, offset + vec2(0.1753, 0.1211) * vec2(-0.12,0.11) * time * 1.22, 7);

  second.x = perlin_noise(noise_texture, offset + vec2(0.1111, 0.1222) * vec2(-0.14,-0.09) * time * 1.31, 8);
  second.y = perlin_noise(noise_texture, offset + vec2(0.1652, 0.2111) * vec2(0.16,-0.14) * time * 1.44, 9);

  vec2 final = offset + first * 1.1 + second * 3.1;
  
  float total = perlin_noise(noise_texture, final, 8);
  
  gradient g_first, g_second, g_final;
  g_first = gradient_new( vec3(206, 210, 219) / 255, vec3(75, 97, 89) / 255);
  g_second = gradient_new( vec3(99, 63, 33) / 255, vec3(237, 197, 128) / 255);
  g_final = gradient_new( vec3(49, 89, 52) / 255, vec3(142, 124, 156) / 255);
  
  vec3 color = gradient_lookup(g_second, first.x * 1.3);
  color = mix(color, gradient_lookup(g_first, second.x * 1.5), (first.y + second.y)) * 2;
  
  color.r = pow(color.r, 1.5);
  color.g = pow(color.r, 1.4);
  color.b = pow(color.r, 1.3);
  
  gl_FragColor = vec4(color, 1.0) * mix(total, 0.5, 0.5);
}
