uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

#define MAX_LIGHTS 32

uniform int num_lights;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

uniform vec3 camera_position;

varying vec3 direction;
varying vec3 m_color;
varying vec3 r_color;
varying vec3 debug;

const int nsamples = 2;
const float fsamples = 2.0;

const vec3 inv_wavelength = vec3(1.0/pow(0.620, 4.0), 1.0/pow(0.495, 4.0),1.0/pow(0.475, 4.0));

const float outer_radius = 10.25;
const float inner_radius = 10.00;

const vec3 esun = vec3(100.0, 100.0, 100.0);
const float kr = 0.0010;
const float km = 0.0010;

const vec3 kresun = kr * esun;
const vec3 kmesun = km * esun;

const float kr4pi = kr * 4 * 3.141;
const float km4pi = km * 4 * 3.141;

const float scale = 1 / (outer_radius - inner_radius);
const float scale_depth = 0.25;
const float scale_over_depth = scale / scale_depth;

/* Analytical scale function */
float scalefunc(float angle) {
  float x = 1-angle;
	return scale_depth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main() {
  
  vec3 light_direction = -normalize(light_target[0] - light_position[0]);
  
  vec3 cam_pos = camera_position;
  cam_pos.y = cam_pos.y / 2048 + inner_radius;
  cam_pos.xz = (cam_pos.xz - 512) / 2048;
  
	vec3 position;
  position.x = gl_Vertex.x * 4.0 * (outer_radius - inner_radius);
  position.z = gl_Vertex.z * 4.0 * (outer_radius - inner_radius);
  position.y = inner_radius + (gl_Vertex.y * (outer_radius - inner_radius));
  
	vec3 ray = position - cam_pos;
	float far = length(ray);
	ray /= far;

	vec3 start = cam_pos;
	float height = length(start);
	float depth = exp(scale_over_depth * (inner_radius - height));
	float start_angle = dot(ray, start) / height;
	float start_offset = depth * scalefunc(start_angle);

	float sample_length = far / fsamples;
	float scaled_length = sample_length * scale;
	vec3 sample_ray = ray * sample_length;
	vec3 sample_point = start + sample_ray * 0.5;

	vec3 front_color = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<nsamples; i++) {
		float height = length(sample_point);
		float depth = exp(scale_over_depth * (inner_radius - height));
		float light_angle = dot(light_direction, sample_point) / height;
		float camera_angle = dot(ray, sample_point) / height;
		float scatter = (start_offset + depth*(scalefunc(light_angle) - scalefunc(camera_angle)));
		vec3 attenuate = exp(-scatter * (inv_wavelength * kr4pi + km4pi));
		front_color += attenuate * (depth * scaled_length);
		sample_point += sample_ray;
	}

	m_color = front_color * kmesun;
	r_color = front_color * (inv_wavelength * kresun);
  
  direction = cam_pos - position;
  
	gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}
