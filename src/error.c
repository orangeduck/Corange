#include <stdio.h>
#include <stdlib.h>

#include "error.h"


typedef void* (*error_func_t)(void);

#define MAX_ERROR_FUNCS 32
static error_func_t error_func_array[MAX_ERROR_FUNCS];
int num_error_funcs = 0;

void aterror(void(*func)(void)) {
  if (num_error_funcs == MAX_ERROR_FUNCS) { 
    warning("Cannot register more than maximum of %i error functions", MAX_ERROR_FUNCS);
    return;
  }
  error_func_array[num_error_funcs] = (error_func_t)func;
  num_error_funcs++;
}

void error_funcs() {
  for (int i = 0; i < num_error_funcs; i++) {
    error_func_array[i]();
  }
}

void error_bp() {}
void warning_bp() {}
void debug_bp() {}
void assert_bp() {}
void exception_bp() {}

void disable_errors() {
  errors_enabled = false;
}

void enable_errors() {
  errors_enabled = true;
}

void disable_warnings() {
  warnings_enabled = false;
}

void enable_warnings() {
  warnings_enabled = true;
}

void disable_debug() {
  debug_enabled = false;
}

void enable_debug() {
  debug_enabled = true;
}

void disable_assert() {
  assert_enabled = false;
}

void enabled_assert() {
  assert_enabled = true;
}

void disable_exceptions() {
  exceptions_enabled = false;
}

void enable_exceptions() {
  exceptions_enabled = true;
}
