uniform vec3 spec_col;
uniform sampler2D samplRgb;
void main(){
  vec4 tex = texture( samplRgb,gl_TexCoord[0].st);
  gl_FragColor.rgb = tex.rgb*(gl_Color.rgb
				 + gl_SecondaryColor.rgb*spec_col*tex.a*1.0);
  gl_FragColor.a = 1.0;

}