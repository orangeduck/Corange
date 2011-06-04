varying vec4 position;
varying mat4 TBN;

uniform float specular_level, glossiness, bumpiness;
uniform sampler2D tDiffuse, tBumpMap, tSpecMap;

void main( void )
{
	vec2 uvs = vec2(gl_TexCoord[0].x, -gl_TexCoord[0].y);

	float spec = texture2D(tSpecMap,uvs).r * specular_level;
	
	vec4 normal = texture2D(tBumpMap, uvs);
	normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), bumpiness);
	normal = (normal * 2 - vec4(1,1,1,0)) * TBN;
	
	gl_FragData[0].rgb = texture2D(tDiffuse, uvs).rgb;
	gl_FragData[0].a = spec;
	
	gl_FragData[1].rgb = position.xyz;
	gl_FragData[1].a = 1.0;
	
	gl_FragData[2] = normal;
	gl_FragData[2].a = glossiness;
}