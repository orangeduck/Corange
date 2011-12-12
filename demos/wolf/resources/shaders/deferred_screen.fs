uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform mat4 light_view;
uniform mat4 light_proj;

vec4 gamma(vec4 color, float amount) {
	vec4 ret;
	ret.r = pow(color.r, amount);
	ret.g = pow(color.g, amount);
	ret.b = pow(color.b, amount);
	ret.a = color.a;
	return ret;
}

vec4 gamma_space(vec4 color) {
	vec4 ret;
	ret.r = pow(color.r, 2.2);
	ret.g = pow(color.g, 2.2);
	ret.b = pow(color.b, 2.2);
	ret.a = pow(color.a, 2.2);
	return ret;
}

vec4 linear_space(vec4 color) {
	vec4 ret;
	ret.r = pow(color.r, 1.0/2.2);
	ret.g = pow(color.g, 1.0/2.2);
	ret.b = pow(color.b, 1.0/2.2);
	ret.a = pow(color.a, 1.0/2.2);
	return ret;
}

void main( void )
{
	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
	
  // SHADOW MAPPING
  
  float shadow = 1.0;
  
  vec4 light_pos = light_proj * light_view * vec4(position.xyz,1.0);
  
  vec4 shadow_coord = (light_pos / light_pos.w) / 2.0 + 0.5;
  
  float shadow_depth = texture2D( shadows_texture, shadow_coord.xy ).r;
  float our_depth = shadow_coord.z;
  
  if (our_depth >= shadow_depth) {
	shadow = 0.0;
  }
  
  // END
	
	vec4 diffuse_a = gamma_space( texture2D( diffuse_texture, gl_TexCoord[0].xy ));
	vec3 diffuse = diffuse_a.rgb;
	
	float spec = diffuse_a.a;
	
	vec4 normal_a = texture2D( normals_texture, gl_TexCoord[0].xy );
	vec3 normal = normal_a.rgb;
	float glossiness = normal_a.a;
	
	vec3 lightDir = light_position - position.xyz ;

	normal = normalize(normal);
	lightDir = normalize(lightDir);

	vec3 eyeDir = normalize(camera_position - position.xyz);
	vec3 vHalfVector = normalize(lightDir + eyeDir);
	
	vec3 ambient_amount = 1.0 * diffuse;
	float light_amount = max(dot(normal, lightDir), 0.0) * 5;
	float spec_amount = pow(max(dot(normal, vHalfVector),0.0), glossiness) * spec * 5;
	
	float difference = our_depth - shadow_depth;
	
	gl_FragColor.rgb = shadow * light_amount * diffuse + ambient_amount + spec_amount * shadow;
	//gl_FragColor.rgb = vec3(difference,difference,difference);
	gl_FragColor.a = 1.0;
	
} 