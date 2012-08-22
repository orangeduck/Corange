#ifndef spline_h
#define spline_h

#include "corange.h"

#define MAX_SPLINE_POINTS 20

typedef struct {
  int num_points;
  float y0d;
  float ynd;
  float x0d;
  float xnd;
  float x[MAX_SPLINE_POINTS];
  float y[MAX_SPLINE_POINTS];
  float yd[MAX_SPLINE_POINTS];
  float xd[MAX_SPLINE_POINTS];
} spline;

spline* spline_new();
void spline_delete(spline* s);

void spline_add_point(spline* s, vec2 p);

vec2 spline_get_point(spline* s, int i);
void spline_set_point(spline* s, int i, vec2 p);

void spline_update(spline* s);

float spline_get_x(spline* s, float y);
float spline_get_y(spline* s, float x);

float spline_get_x_between(spline* s, int low, int high, float y);
float spline_get_y_between(spline* s, int low, int high, float x);

void spline_print(spline* s);

void spline_render(spline* s, vec2 position, vec2 size, int increments);


typedef struct {
  spline* rgb_spline;
  spline* r_spline;
  spline* g_spline;
  spline* b_spline;
  spline* a_spline;
} color_curves;

color_curves* acv_load_file(char* filename);
void color_curves_delete(color_curves* cc);

void color_curves_write_lut(color_curves* cc, char* filename);

vec3 color_curves_map(color_curves* cc, vec3 in);

#endif