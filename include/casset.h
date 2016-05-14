/**
*** :: Asset ::
***
****  An asset can be considered an external file used in the engine.
***   Assets are identified using their file system path.
***  
***     texture* blah = asset_get(P("./textures/blah.dds"));
***
***   It is possible to register load and unload functions
***   for type via their file extensions.
***
***     asset_handler(renderable, "obj", obj_load_file, renderable_delete);
***
***   Please do not store raw pointers to assets.
***   Use an 'asset_hndl' value instead. It is a kind
***   of smart pointer which will not become invalidated
***   when assets are reloaded or moved around.
***
**/

#ifndef casset_h
#define casset_h

#include "cengine.h"

typedef void asset;

/* Storable Asset Handle */

typedef struct {
  fpath path;
  asset* ptr;
  uint32_t timestamp;
} asset_hndl;

asset_hndl asset_hndl_null(void);
asset_hndl asset_hndl_new(fpath path);
asset_hndl asset_hndl_new_load(fpath path);
asset_hndl asset_hndl_new_ptr(asset* as);

bool asset_hndl_isnull(asset_hndl* ah);
fpath asset_hndl_path(asset_hndl* ah);
asset* asset_hndl_ptr(asset_hndl* ah);
bool asset_hndl_eq(asset_hndl* ah0, asset_hndl* ah1);

void asset_cache_flush(void);

/* Init and Finish operations */
void asset_init(void);
void asset_finish(void);

/* Map a variable such as '$CORANGE' to a path string */
void asset_add_path_variable(fpath variable, fpath mapping);

fpath asset_map_filename(fpath filename);
fpath asset_unmap_filename(fpath filename);

/* Create handler for asset type. Requires type, file extension, and load/unload functions. */
#define asset_handler(type, extension, loader, deleter) \
  asset_handler_cast(typeid(type), extension, \
  (asset*(*)(const char*))loader , \
  (asset(*)(void*))deleter)
  
void asset_handler_cast(
  type_id type, const char* extension, 
  asset* asset_loader(const char* filename) , 
  void asset_deleter(asset* asset) );

/* Load/Reload/Unload assets at path or folder */
void file_load(fpath filename);
void file_unload(fpath filename);
void file_reload(fpath filename);
bool file_isloaded(fpath path);
bool file_exists(fpath path);

void folder_load(fpath folder);
void folder_unload(fpath folder);
void folder_reload(fpath folder);
void folder_load_recursive(fpath folder);

asset* asset_get_load(fpath path);
asset* asset_get(fpath path);

#define asset_get_as(path, type) ((type*)asset_get_as_type(path, typeid(type)))
asset* asset_get_as_type(fpath path, type_id type);

/* Reload all assets of a given type */
#define asset_reload_type(type) asset_reload_type_id(typeid(type))
void asset_reload_type_id(type_id type);
void asset_reload_all(void);

/* Get path or typename of asset at ptr */
char* asset_ptr_path(asset* a);
char* asset_ptr_typename(asset* a);

#endif