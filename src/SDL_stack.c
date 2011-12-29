#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
  #include <execinfo.h>
#endif
  
#ifdef _WIN32
  #include <windows.h>
  #include <winbase.h>
#endif

#include "SDL/SDL_stack.h"

#ifdef __linux__

void SDL_PrintStackTrace() {
  
  const int DEPTH = 10;
  void* stack[DEPTH];
     
  int size = backtrace(stack, DEPTH);
  char** strings = backtrace_symbols(stack, size);

  printf ("[STACK] (%i frames)\n", size);
  
  int i;
  for (i = 0; i < size; i++) {
    printf ("  %s\n", strings[i]);
  }

  free(strings);
}

#elif _WIN32

void SDL_PrintStackTrace() {

}

#else

void SDL_PrintStackTrace() {}

#endif
