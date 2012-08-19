/**
*** :: Asset Manager ::
***
***   The Asset Manager is used to load/unload assets.
***   It specifies the asset using the file system path.
***  
***     asset_hndl blah_hndl = asset_hndl_new(P("./textures/blah.dds"));
***     texture* blah = asset_hndl_ptr(blah_hndl);
***
***   It is possible to register load and unload functions
***   for certain file extensions and struct types.
***
***   Please do not store raw pointers to assets.
***   Use 'asset_hndl' as on reloading assets the
***   raw asset pointer will become invalidated.
***   A 'asset_hndl' value will always remain accurate.
**/

#ifndef asset_manager_h
#define asset_manager_h

#include "cengine.h"

typedef void asset;

/* Handle to Asset pointer */
typedef struct {
  fpath path;
  asset* ptr;
} asset_hndl;

asset_hndl asset_hndl_null();
asset_hndl asset_hndl_new(fpath path);
asset_hndl asset_hndl_new_ptr(asset* as);

bool asset_hndl_isnull(asset_hndl ah);
fpath asset_hndl_path(asset_hndl ah);
asset* asset_hndl_ptr(asset_hndl ah);

/* Init and Finish operations */
void asset_manager_init();
void asset_manager_finish();

/* Map a variable such as '$CORANGE' to a path */
void asset_manager_add_path_variable(fpath variable, fpath mapping);

/* Create handler for asset type. Requires type, file extension, load and unload functions. */
#define asset_manager_handler(type, extension, loader, deleter) \
  asset_manager_handler_cast(typeid(type), extension, \
  (asset*(*)(const char*))loader , \
  (asset(*)(void*))deleter)
  
void asset_manager_handler_cast(
  type_id type, const char* extension, 
  asset* asset_loader(const char* filename) , 
  void asset_deleter(asset* asset) );

/* Load/Reload/Unload assets at path or folder */
void file_load(fpath filename);
void file_unload(fpath filename);
void file_reload(fpath filename);
bool file_isloaded(fpath path);

void folder_load(fpath folder);
void folder_unload(fpath folder);
void folder_reload(fpath folder);

/* Get path or typename of asset at ptr */
char* asset_ptr_path(asset* a);
char* asset_ptr_typename(asset* a);

#endif
