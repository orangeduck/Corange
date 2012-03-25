#include <stdio.h>
#include <stdlib.h>

#include "error.h"

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
