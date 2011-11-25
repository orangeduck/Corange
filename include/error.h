#ifndef error_h
#define error_h

#define error(MSG, ...) printf("Error at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); exit(EXIT_FAILURE)
#define error_if(COND, MSG, ...) if(COND) { error(MSG, __VA_ARGS__); }

#define warning(MSG, ...) printf("Warning at %s:%i - ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout)
#define warning_if(COND, MSG, ...) if(COND) { warning(MSG, __VA_ARGS__); }

#endif