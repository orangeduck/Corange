#ifndef spline_h
#define spline_h

#include "corange.h"

typedef struct {
  int num_points;
  float y0d;
  float ynd;
  float x0d;
  float xnd;
  float x[20];
  float y[20];
  float yd[20];
  float xd[20];
} spline;

spline* spline_new();
void spline_delete(spline* s);

void spline_add_point(spline* s, vector2 p);

vector2 spline_get_point(spline* s, int i);
void spline_set_point(spline* s, int i, vector2 p);

void spline_update(spline* s);

float spline_get_x(spline* s, float y);
float spline_get_y(spline* s, float x);

float spline_get_x_between(spline* s, int low, int high, float y);
float spline_get_y_between(spline* s, int low, int high, float x);

void spline_print(spline* s);

void spline_render(spline* s, vector2 position, vector2 size, int increments);


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

#endif