#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void error_bp() {}
void warning_bp() {}
void info_bp() {}

void disable_errors() {
  errors_enabled = 0;
}

void enable_errors() {
  errors_enabled = 1;
}

void disable_warnings() {
  warnings_enabled = 0;
}

void enable_warnings() {
  warnings_enabled = 1;
}

void disable_info() {
  info_enabled = 0;
}

void enable_info() {
  info_enabled = 1;
}