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

#define error(MSG, ...) if(errors_enabled) { printf("Error at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); exit(EXIT_FAILURE); }

#define error_if(COND, MSG, ...) if(COND) { error(MSG, __VA_ARGS__); }

#define warning(MSG, ...) if(warnings_enabled) { printf("Warning at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); }

#define warning_if(COND, MSG, ...) if(COND) { warning(MSG, __VA_ARGS__); }

#define info(MSG, ...) if(warnings_enabled) { printf("Info at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); }

#define info_if(COND, MSG, ...) if(COND) { info(MSG, __VA_ARGS__); }

#endif