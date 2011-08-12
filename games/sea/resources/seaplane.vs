uniform float time;

varying vec3 normal;
varying vec3 position;

vec3 spike_wave( vec2 direction, vec2 position, vec2 wavelength, float spikiness ) {
	
  direction = normalize(direction);
	
  vec3 result;
  result.y = sin( dot(direction, position / wavelength) ) + 1.0;	
  result.xz = 0.5 * spikiness * direction * cos( dot(direction, position / wavelength) );	

  return result;
}


void main()
{

  vec3 difference = vec3(0,0,0);

  float time_v = time * 3;
  
  difference += spike_wave( vec2(1,0) , gl_Vertex.xz + vec2(time_v,time_v) , vec2(3.1, 3.3), 1.0 );
  difference += 0.51 * spike_wave( vec2(1.0,0.62) , gl_Vertex.xz + vec2(time_v,time_v) , vec2(1.85, 1.85), 1.0 );
  difference += 0.50 * spike_wave( vec2(0.41,1.0) , gl_Vertex.xz + vec2(time_v,time_v) , vec2(2.21, 2.21), 1.0 );
  difference += 0.50 * spike_wave( vec2(0.1,0.51) , gl_Vertex.xz + vec2(time_v,time_v) , vec2(4.15, 4.15), 1.0 );
  
  //difference.y = sin( gl_Vertex.x + time );
  
  gl_Vertex.xyz += difference;
  gl_Normal.xyz += 0.5 * vec3( -difference.x, difference.y, -difference.z);
  
  normal = gl_Normal;
  position = gl_Vertex.xyz / gl_Vertex.w;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}