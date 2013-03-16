#version 120

attribute vec3 vPosition;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 light_direction;
//uniform vec3 camera_position;

varying vec3 fDirection;
varying vec3 fM_color;
varying vec3 fR_color;

#define nsamples 3
#define fsamples 3.0

#define inv_wavelength vec3(1.0 / pow(0.620, 4.0), 1.0 / pow(0.495, 4.0), 1.0 / pow(0.475, 4.0))

#define outer_radius 10.25
#define inner_radius 10.00

#define esun vec3(75.0, 75.0, 75.0)
#define kr 0.0025
#define km 0.0010

#define kresun (kr * esun)
#define kmesun (km * esun)

#define kr4pi (kr * 4 * 3.141)
#define km4pi (km * 4 * 3.141)

#define scale (1 / (outer_radius - inner_radius))
#define scale_depth 0.25
#define scale_over_depth (scale / scale_depth)

/* Analytical scale function */
float scalefunc(float angle) {
  float x = 1-angle;
	return scale_depth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

void main() {
  
  vec3 cam_pos = vec3(0, inner_radius + 0.1, 0);
  //vec3 cam_pos = camera_position;
  //cam_pos.y = cam_pos.y / 2000 + inner_radius;
  //cam_pos.xz = (cam_pos.xz - 1000) / 2000;
  
	vec3 position;
  position.x = vPosition.x * 1;
  position.z = vPosition.z * 1;
  position.y = inner_radius + (abs(vPosition.y) * 0.25);
  
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
  
	for(int i = 0; i < nsamples; i++) {
		float height = length(sample_point);
		float depth = exp(scale_over_depth * (inner_radius - height));
		float light_angle = dot(-light_direction, sample_point) / height;
		float camera_angle = dot(ray, sample_point) / height;
		float scatter = (start_offset + depth*(scalefunc(light_angle) - scalefunc(camera_angle)));
		vec3 attenuate = exp(-scatter * (inv_wavelength * kr4pi + km4pi));
		front_color += attenuate * (depth * scaled_length);
		sample_point += sample_ray;
	}

	fM_color = front_color * kmesun;
	fR_color = front_color * (inv_wavelength * kresun);
  fDirection = cam_pos - position;
  
	gl_Position = proj * view * world * vec4(vPosition, 1);
}
