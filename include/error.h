#ifndef error_h
#define error_h

#include "bool.h"

/* Errors */

static bool errors_enabled = true;

void disable_errors();
void enable_errors();

void error_bp();

#ifndef NO_ERROR
  #define error(MSG, ...) if(errors_enabled) { printf("[ERROR] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); error_bp(); exit(EXIT_FAILURE); }
#else
  #define error(MSG, ...) {}
#endif


/* Warnings */

static bool warnings_enabled = true;

void disable_warnings();
void enable_warnings();

void warning_bp();

#ifndef NO_WARN
  #define warning(MSG, ...) if(warnings_enabled) { printf("[WARN] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); warning_bp(); }
#else
  #define warning(MSG, ...) {}
#endif


/* Debug */

static bool debug_enabled = true;

void disable_debug();
void enable_debug();

void debug_bp();

#ifndef NO_DEBUG
  #define debug(MSG, ...) if(debug_enabled) { printf("[DEBUG] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); debug_bp(); }
#else
  #define debug(MSG, ...) {}
#endif


/* Assert */

static bool assert_enabled = true;

void disable_assert();
void enabled_assert();

void assert_bp();

#ifndef NO_ASSERT
  #define assert(COND, MSG, ...) if(assert_enabled && (!(COND))) { printf("[ASSERT] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); assert_bp(); exit(EXIT_FAILURE); }
#else
  #define assert(COND, MSG, ...) {}
#endif


/* Exception */

static bool exceptions_enabled = true;

void disable_exceptions();
void enable_exceptions(); 

void exception_bp();

#ifndef NO_EXCEPT
  #include <setjmp.h>
  
  #define MAX_EXC_DEPTH 10
  static jmp_buf exc_bufs[MAX_EXC_DEPTH];
  static int exc_depth = 0;

  void exception_bp();

  #define try_catch(TRY, CATCH) if (exc_depth >= MAX_EXC_DEPTH) { error("Max Exception depth of %i exceeded", MAX_EXC_DEPTH); } exc_depth++; if (!setjump(exc_bufs[exc_depth-1])) { PERFORM; } else { CATCH; } exc_depth--;
  
  #define throw(MSG, ...) if (exc_depth) { longjmp(exc_bufs[exc_depth-1],1); } else { printf("[EXCEPTION] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); exception_bp(); exit(EXIT_FAILURE); }
#else
  #define try_catch(TRY, CATCH) error("Exceptions Disabled with NO_EXCEPT macro!")
  #define throw(MSG, ...) error("Exceptions Disabled with NO_EXCEPT macro!")
#endif


/* Check */

static bool check_enabled = true;

void disable_check();
void enabled_check();

void check_bp();

#ifndef NO_CHECK
  #define check(A, MSG, ...) if(check_enabled && (!(A))) { printf("[CHECK] (%s:%i) ", __FILE__, __LINE__); printf(MSG, ##__VA_ARGS__); printf("\n"); fflush(stdout); check_bp(); exit(EXIT_FAILURE); }
#else
  #define check(A, MSG, ...) {}
#endif

/* Other */

#define alloc_check(PTR) if((PTR) == NULL) { error("Out of Memory!"); }

#endif
