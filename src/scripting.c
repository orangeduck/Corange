#include "logger.h"

#include <stdlib.h>
#include <string.h>

#include "LUA/lua.h"
#include "LUA/lauxlib.h"
#include "LUA/lualib.h"

#include "scripting.h"

static lua_State* L;

void scripting_init() {
  L = lua_open();
  luaL_openlibs(L);
  luaopen_table(L);
  luaopen_string(L);
  luaopen_math(L);
};

void scripting_finish() {
  lua_close(L);
}; 


script* lua_load_file(const char* filename) {

  printf("loading: %s\n");

  script* s = malloc(sizeof(script));
  
  s->filename = malloc(strlen(filename) + 1);
  strcpy(s->filename, filename);
  
  return s;
};

void script_delete(script* s) {
  free(s->filename);
  free(s);
};

void scripting_run_script(script* s) {
  luaL_dofile(L, s->filename);
};

void scripting_run_script_file(const char* filename) {
  luaL_dofile(L, filename);
};