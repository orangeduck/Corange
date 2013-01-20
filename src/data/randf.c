#include "data/randf.h"

float randf() {
  return (float)rand() / RAND_MAX;
}

union float_int {
  float as_float;
  uint32_t as_int;
};

uint32_t float_to_int(float f) {
  union float_int fi = {f};
  return fi.as_int;
}

float randf_seed(float s) {
  return srand(float_to_int(s)), randf();
}

float randf_nseed(float s) {
  return srand(float_to_int(s)), randf_n();
}

float randf_n() {
  return randf() * 2 - 1;
}

float randf_scale(float s) {
  return randf() * s;
}

float randf_nscale(float s) {
  return randf_n() * s;
}

float randf_range(float s, float e) {
  return s + randf() * (e - s);
}