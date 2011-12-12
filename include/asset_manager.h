#ifndef asset_manager_h
#define asset_manager_h

#include "dictionary.h"
#include "list.h"

void asset_manager_init();
void asset_manager_finish();

#define asset_loader_t void*(*)(char*)
#define asset_deleter_t void(*)(void*)
void asset_manager_handler(char* extension, void* asset_loader(char* filename) , void asset_deleter(void* asset) );

void load_file(char* filename);
void load_folder(char* folder);

void reload_file(char* filename);
void reload_folder(char* folder);

void unload_file(char* filename);
void unload_folder(char* folder);

void* asset_get(char* path);
int asset_loaded(char* path);

void asset_state_print();

/* User is responsible for freeing strings returned by these */
char* asset_map_filename(char* filename);

char* asset_file_contents(char* filename);
char* asset_file_extension(char* filename);
char* asset_file_location(char* filename);

#endif