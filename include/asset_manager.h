#ifndef asset_manager_h
#define asset_manager_h

#include "dictionary.h"

void asset_manager_init(char* game_name);
void asset_manager_finish();

char* asset_map_filename(char* filename);

void asset_manager_handler(char* extension, void* load_func(char*) , void del_func(void*) );

void load_file(char* filename);
void load_folder(char* folder);

void reload_file(char* filename);
void reload_folder(char* folder);

void unload_file(char* filename);
void unload_folder(char* folder);

void* asset_get(char* path);
int asset_loaded(char* path);

char* asset_file_contents(char* filename);
char* asset_file_extension(char* filename);
char* asset_file_location(char* filename);

#endif