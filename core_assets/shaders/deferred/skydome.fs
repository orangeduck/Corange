#version 120

uniform vec3 light_direction;

varying vec3 fDirection;
varying vec3 fM_color;
varying vec3 fR_color;

float rayleigh_phase(float cos2a) {
  return 0.75 + 0.75*cos2a;
}

float mei_phase(float cosa, float cos2a) {
  return (1.3125 / 5.125) * ((1 + cos2a) / pow(1.5625 + 1.4*cosa, 3.0/2.0));
}

void main() {

  float cosa = dot(light_direction, fDirection) / length(fDirection);
  float cos2a = cosa * cosa;
  
	gl_FragColor.rgb = clamp(
    0.5 * (rayleigh_phase(cos2a)  * fR_color) + 
    1.0 * (mei_phase(cosa, cos2a) * fM_color),
    vec3(0,0,0),
    vec3(100,100,100));
  gl_FragColor.a = 1.0;
}
