uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture;

uniform sampler2D random_texture;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform mat4 light_view;
uniform mat4 light_proj;

/* Headers */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

/* End */

void main( void )
{
	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
  
  vec4 light_pos = light_proj * light_view * vec4(position.xyz,1);
  float shadow = shadow_amount_soft_pcf25(light_pos, shadows_texture, 0.001);
	
	vec4 diffuse_a = texture2D( diffuse_texture, gl_TexCoord[0].xy );
	vec3 diffuse = diffuse_a.rgb;
	
	float spec = diffuse_a.a;
	
	vec4 normal_a = texture2D( normals_texture, gl_TexCoord[0].xy );
	vec3 normal = normal_a.rgb;
	float glossiness = normal_a.a;
	
	vec3 lightDir = light_position - position.xyz;

	normal = normalize(normal);
  
	lightDir = normalize(lightDir);

	vec3 eyeDir = normalize(camera_position - position.xyz);
	vec3 vHalfVector = normalize(lightDir + eyeDir);
	
  float ssao = ssao_depth(gl_TexCoord[0].xy, depth_texture, random_texture);
  
	vec3 ambient_amount = 0.3 * diffuse * ssao;
	float light_amount = max(dot(normal, lightDir), 0.0);
	float spec_amount = spec * pow(max(dot(normal, vHalfVector),0.0), glossiness);
  
	gl_FragColor.rgb = shadow * light_amount * diffuse + ambient_amount + shadow * spec_amount;
	gl_FragColor.a = 1.0;
	
} 