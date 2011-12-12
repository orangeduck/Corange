varying vec4 position;
varying mat4 TBN;

uniform mat4 world_matrix;

uniform float glossiness;
uniform float bumpiness;

uniform float specular_level;

uniform sampler2D diffuse_map;
uniform sampler2D bump_map;
uniform sampler2D spec_map;

/* Headers */

float to_gamma(float color);
float from_gamma(float color);
vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);
vec3 swap_red_green(vec3 color);

/* End */

void main( void )
{
	vec2 uvs = vec2(gl_TexCoord[0].x, -gl_TexCoord[0].y);

	float spec = texture2D(spec_map,uvs).r * specular_level;
	
	vec4 normal = texture2D(bump_map, uvs);
	normal.rgb = swap_red_green(normal.rgb);
	normal = mix(normal, vec4( 0.5, 0.5, 1.0, 1.0 ), bumpiness);
	normal = (normal * 2.0 - vec4(1.0,1.0,1.0,0.0)) * TBN * world_matrix;
	
	gl_FragData[0].rgb = from_gamma(texture2D(diffuse_map, uvs).rgb);
	gl_FragData[0].a = from_gamma(spec);
	
	gl_FragData[1].rgb = position.xyz;
	gl_FragData[1].a = 1.0;
	
	gl_FragData[2] = normal;
	gl_FragData[2].a = glossiness;
}