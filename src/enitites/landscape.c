#include <stdlib.h>

#include "entities/landscape.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  
  l->heightmap = asset_hndl_null();
  l->attribmap = asset_hndl_null();
  
  l->size_x = 128;
  l->size_y = 128;
  
  l->ground0 = asset_hndl_null();
  l->ground1 = asset_hndl_null();
  l->ground2 = asset_hndl_null();
  l->ground3 = asset_hndl_null();
  
  l->ground0_nm = asset_hndl_null();
  l->ground1_nm = asset_hndl_null();
  l->ground2_nm = asset_hndl_null();
  l->ground3_nm = asset_hndl_null();
  
  return l;
  
}

void landscape_delete(landscape* l) {
  free(l);
}
