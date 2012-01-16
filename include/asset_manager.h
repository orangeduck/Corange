#ifndef asset_manager_h
#define asset_manager_h

#include "bool.h"
#include "dictionary.h"
#include "list.h"

typedef void asset;

void asset_manager_init();
void asset_manager_finish();

void asset_manager_add_path_variable(char* variable, char* mapping);

#define asset_manager_handler(extension, loader, deleter) asset_manager_handler_cast(extension, (asset*(*)(char*))loader , (asset(*)(void*))deleter)
void asset_manager_handler_cast(char* extension, asset* asset_loader(char* filename) , void asset_deleter(asset* asset) );

void load_file(char* filename);
void load_folder(char* folder);

void reload_file(char* filename);
void reload_folder(char* folder);

void unload_file(char* filename);
void unload_folder(char* folder);

asset* asset_get(char* path);
asset* asset_load_get(char* path);
#define asset_get_as(path, type) ((type*)asset_get(path))

bool asset_loaded(char* path);

void asset_state_print();

/* User is responsible for freeing strings returned by these */
char* asset_map_filename(char* filename);

char* asset_name_extension(char* filename);
char* asset_name_location(char* filename);

#endif
