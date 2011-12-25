#ifndef asset_manager_h
#define asset_manager_h

#include "bool.h"
#include "dictionary.h"
#include "list.h"

void asset_manager_init();
void asset_manager_finish();

void asset_manager_add_path_variable(char* variable, char* mapping);

#define asset_manager_handler(extension, loader, deleter) asset_manager_handler_cast(extension, (void*(*)(char*))loader , (void(*)(void*))deleter)
void asset_manager_handler_cast(char* extension, void* asset_loader(char* filename) , void asset_deleter(void* asset) );

void load_file(char* filename);
void load_folder(char* folder);

void reload_file(char* filename);
void reload_folder(char* folder);

void unload_file(char* filename);
void unload_folder(char* folder);

void* asset_get(char* path);
bool asset_loaded(char* path);

void asset_state_print();

/* User is responsible for freeing strings returned by these */
char* asset_map_filename(char* filename);

char* asset_file_contents(char* filename);
char* asset_file_extension(char* filename);
char* asset_file_location(char* filename);

#endif