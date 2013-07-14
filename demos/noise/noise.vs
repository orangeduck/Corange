varying vec3 normal;
varying vec2 uvs;

void main() {
  uvs = gl_MultiTexCoord0.xy;
  normal = gl_Normal;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}