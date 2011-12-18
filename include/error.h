#ifndef error_h
#define error_h

static int errors_enabled = 1;
static int warnings_enabled = 1;
static int info_enabled = 1;

void disable_errors();
void enable_errors();

void disable_warnings();
void enable_warnings();

void disable_info();
void enable_info();

/*
  Because the following are expanded as macros,
  here are some dummy functions you can breakpoint on,
  useful for getting a stack trace on an error.
*/

void error_bp();
void warning_bp();
void info_bp();

#define error(MSG, ...) if(errors_enabled) { printf("Error at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); error_bp(); exit(EXIT_FAILURE); }

#define warning(MSG, ...) if(warnings_enabled) { printf("Warning at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); warning_bp(); }

#define info(MSG, ...) if(info_enabled) { printf("Info at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); info_bp(); }

#endif