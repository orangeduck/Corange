#version 120

uniform vec3 light_direction;

varying vec3 direction;
varying vec3 m_color;
varying vec3 r_color;

float rayleigh_phase(float cos2a) {
  return 0.75 + 0.75*cos2a;
}

float mei_phase(float cosa, float cos2a) {
  return (1.3125 / 5.125) * ((1 + cos2a) / pow(1.5625 + 1.4*cosa, 3.0/2.0));
}

void main() {

  float cosa = dot(light_direction, direction) / length(direction);
  float cos2a = cosa * cosa;
  
	gl_FragColor.rgb = rayleigh_phase(cos2a) * r_color + 
                     mei_phase(cosa, cos2a) * m_color;
  
	gl_FragColor.a = 1.0;
}
