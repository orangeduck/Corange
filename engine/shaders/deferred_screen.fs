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

float ssao(vec2 texcoords) {
  
  const float total_strength = 1.0;
  const float base = 0.2;
  
  const float area = 0.0075;
  const float falloff = 0.000001;
  
  const float radius = 0.05;
  
  const int samples = 16;
  vec3 sample_sphere[16] = {vec3(0.53812504, 0.18565957, -0.43192),vec3(0.13790712, 0.24864247, 0.44301823),vec3(0.33715037, 0.56794053, -0.005789503),vec3(-0.6999805, -0.04511441, -0.0019965635),
                            vec3(0.06896307, -0.15983082, -0.85477847),vec3(0.056099437, 0.006954967, -0.1843352),vec3(-0.014653638, 0.14027752, 0.0762037),vec3(0.010019933, -0.1924225, -0.034443386),
                            vec3(-0.35775623, -0.5301969, -0.43581226),vec3(-0.3169221, 0.106360726, 0.015860917),vec3(0.010350345, -0.58698344, 0.0046293875),vec3(-0.08972908, -0.49408212, 0.3287904),
                            vec3(0.7119986, -0.0154690035, -0.09183723),vec3(-0.053382345, 0.059675813, -0.5411899),vec3(0.035267662, -0.063188605, 0.54602677),vec3(-0.47761092, 0.2847911, -0.0271716)};
  
  vec3 random = normalize( texture2D(random_texture, texcoords * 4.0).rgb * 2.0 - 1.0 );
  
  float depth = texture2D(depth_texture, texcoords).r;
 
  vec3 position = vec3(texcoords, depth);
  vec3 normal = texture2D(normals_texture, texcoords).rgb;
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i=0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(sample_sphere[i], random);
    vec3 projected = position + sign(dot(ray,normal)) * ray;
    
    float occ_depth = texture2D(depth_texture, saturate(projected.xy)).r;
    float difference = depth - occ_depth;
    
    float step;
    if (difference >= falloff) { step = 1; } else { step = 0; }
    occlusion += step * smoothstep(falloff, area, difference);
  }
  
  float ao = 1 - total_strength * occlusion * (1.0 / samples);
  return saturate(ao + base);

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
	shadow = 0.1;
  }
  
  // END
	
	vec4 diffuse_a = texture2D( diffuse_texture, gl_TexCoord[0].xy );
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
	
  float ssao = ssao(gl_TexCoord[0].xy);
  
	vec3 ambient_amount = 0.3 * diffuse * ssao;
	float light_amount = max(dot(normal, lightDir), 0.0);
	float spec_amount = spec * pow(max(dot(normal, vHalfVector),0.0), glossiness);
	
	float difference = our_depth - shadow_depth;
  
	gl_FragColor.rgb = shadow * light_amount * diffuse + ambient_amount + shadow * spec_amount;
	//gl_FragColor.rgb = vec3(ssao,ssao,ssao);
	gl_FragColor.a = 1.0;
	
} 