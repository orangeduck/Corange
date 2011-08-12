varying vec3 lightDir;
varying vec4 color;
varying vec2 tc;
varying vec3 norm;
varying vec3 tanx;
varying vec3 tany;
uniform float usePerVertColor;

void main(){
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  lightDir = vec3(0.0,0.0,1.0)*gl_NormalMatrix;
  color = (usePerVertColor>0)?gl_Color:vec4(1.0,1.0,1.0,1.0);
  tc = gl_MultiTexCoord0.st;
  lightDir =  normalize( vec3(0.0,0.0,1.0) * gl_NormalMatrix );
  norm =  normalize( gl_Normal );
  tanx = gl_MultiTexCoord1.xyz;
  tany = cross( norm, tanx );
  tany *= (gl_MultiTexCoord2.x==0.0)?-1.0:1.0;
  /*norm = normalize(norm);*/
}