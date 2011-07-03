varying vec4 screen_position;

void main()
{
  float depth = screen_position.z / screen_position.w;
  gl_FragColor = vec4(depth, depth, depth, 1.0);
}