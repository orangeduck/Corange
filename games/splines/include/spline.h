#ifndef spline_h
#define spline_h

#include "corange.h"

typedef struct {
  int num_points;
  float x[20];
  float y[20];
  float yd[20];
  float xd[20];
} spline;

static spline* test_spline;

spline* spline_new();
spline* spline_identity();
void spline_delete(spline* s);

void spline_add_point(spline* s, vector2 p);
void spline_update(spline* s);

float spline_get_y(spline* s, float x);
float spline_get_x(spline* s, float y);

void spline_print(spline* s);

void spline_render(spline* s);

void write_lut_file(char* filename, char* output);

#endif