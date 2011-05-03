#ifndef asset_manager_h
#define asset_manager_h

void asset_manager_init();
void asset_manager_finish();

void asset_manager_handler(char* extension, void* load_func(char*) , void del_func(void*) );

void load_asset(char* filename);
void load_folder(char* folder);

void reload_asset(char* filename);
void reload_folder(char* folder);

void* asset_get(char* path);

char* asset_load_file(char* filename);
char* asset_file_extension(char* filename);
char* asset_file_location(char* filename);

#endif