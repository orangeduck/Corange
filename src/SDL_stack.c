#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
  #include <execinfo.h>
#elif _WIN32
  #include <windows.h>
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

  const int DEPTH = 10;
  void* stack[DEPTH];
  
  int size;
  
  SYMBOL_INFO* symbol;
  HANDLE process;

  process = GetCurrentProcess();

  SymInitialize(process, NULL, TRUE);

  size = CaptureStackBackTrace(0, DEPTH, stack, NULL);
  symbol = calloc( sizeof(SYMBOL_INFO) + 256, 1);
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  
  printf("[STACK] (%i frames)\n", size);
  
  int i;
  for( i = 0; i < size; i++ ) {
     SymFromAddr(process, stack[i], 0, symbol);
     printf("  %s [0x%0X]\n", symbol->Name, symbol->Address);
  }

  free(symbol);

}

#else

void SDL_PrintStackTrace() {}

#endif
