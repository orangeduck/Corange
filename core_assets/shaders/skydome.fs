uniform vec3 light_direction;

varying vec3 light;
varying vec3 direction;

vec3 rayleigh_phase(float cos2a) {
  return (3/4) * (1 + cos2a);
}

vec3 mei_phase(float cosa, float cos2a) {
  return (1.3125 / 5.125) * ((1 + cos2a) / pow(1.5625 + 1.4*cosa, 3/2));
}

void main() {

  float cosa = dot(light_direction, direction) / length(direction);
  float cos2a = cosa * cosa;
  
  /* TODO: Fill in wavelengths */
  vec3 k_ray = (4 * 3.141) / pow(vec3(1,1,1), 4.00);
  vec3 k_mei = (4 * 3.141) / pow(vec3(1,1,1), 0.84);
  vec3 k_sun = vec3(1,1,1) * 1024;
  
  vec3 color = light * k_sun * k_ray * rayleigh_phase(cosa) + 
               light * k_sun * k_mei * mei_phase(cosa, cos2a);
  
  gl_FragColor = vec4(color,1);
}
