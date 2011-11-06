uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture;

uniform sampler2D random_texture;

uniform sampler3D lut;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform vec3 diffuse_light;
uniform vec3 ambient_light;
uniform vec3 specular_light;

uniform mat4 light_view;
uniform mat4 light_proj;

/* Headers */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);
vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);
vec3 color_correction(vec3 color, sampler3D lut, int lut_size);

/* End */

void main( void )
{
	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
  
  vec4 light_pos = light_proj * light_view * vec4(position.xyz,1);
  float shadow = shadow_amount_soft_pcf25(light_pos, shadows_texture, 0.001);
	
	vec4 diffuse_a = texture2D( diffuse_texture, gl_TexCoord[0].xy );
	vec3 albedo = diffuse_a.rgb;
	
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
  
	vec3 ambient_amount = albedo * ssao;
	float light_amount = max(dot(normal, lightDir), 0.0);
	float spec_amount = spec * pow(max(dot(normal, vHalfVector),0.0), glossiness);
  
  vec3 diffuse = shadow * light_amount * albedo * diffuse_light;
  vec3 ambient = ambient_amount * ambient_light;
  vec3 specular = shadow * spec_amount * specular_light;
  
  vec3 total = to_gamma(diffuse + ambient + specular);
  
	gl_FragColor.rgb = color_correction(total, lut, 64);
	gl_FragColor.a = 1.0;
	
} 