#ifndef scripting_h
#define scripting_h

typedef struct {
  
  char* filename;

} script;

void scripting_init();
void scripting_finish();

script* lua_load_file(const char* filename);
void script_delete(script* s);

void scripting_run_script(script* s);
void scripting_run_script_file(const char* filename);

#endif 