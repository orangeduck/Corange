
uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

uniform sampler2D bump_map;

uniform float strength;

varying vec2 uvs;
varying mat4 TBN;

void main() {
  
  vec4 normal = texture2D(bump_map, uvs);
  //normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), bumpiness);
  normal = (normal * 2 - vec4(1.0,1.0,1.0,0.0)) * TBN * world_matrix;
  
  normal = proj_matrix * view_matrix * normal;
  
  vec3 final = normal.xyz;
  
  final = final + 0.5;
  
  final = mix( vec3( 0.5f, 0.5f, 1.0f ), final, strength );
  
  gl_FragColor = vec4(final, 1.0);

}