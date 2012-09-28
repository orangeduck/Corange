#version 120

uniform vec3 light_direction;
uniform sampler2D depth_texture;

varying float alpha;
varying vec3 direction;
varying vec3 m_color;
varying vec3 r_color;
varying vec4 screen_position;

float rayleigh_phase(float cos2a) {
  return 0.75 + 0.75*cos2a;
}

float mei_phase(float cosa, float cos2a) {
  return (1.3125 / 5.125) * ((1 + cos2a) / pow(1.5625 + 1.4*cosa, 3.0/2.0));
}

void main() {

  vec2 uvs = (screen_position.xy / screen_position.w) / 2 + 0.5 ;

  float depth = texture2D(depth_texture, uvs).r;

  float cosa = dot(light_direction, direction) / length(direction);
  float cos2a = cosa * cosa;
  
	gl_FragColor.rgb = rayleigh_phase(cos2a) * r_color + 
                     mei_phase(cosa, cos2a) * m_color;
  gl_FragColor.rgb = max(gl_FragColor.rgb, vec3(0.1, 0.1, 0.1));
	gl_FragColor.a = depth * alpha;
}
