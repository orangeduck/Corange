#ifndef entity_manager_h
#define entity_manager_h

#include "list.h"

#include "light.h"
#include "camera.h"

typedef void entity;

void entity_manager_init();
void entity_manager_finish();

int entity_exists(char* name);

entity* entity_new(char* name, int type);
void entity_add(char* name, int type, entity* entity);

entity* entity_get(char* name);
void entity_delete(char* name);

list* entities_get_all();
list* entities_get_cameras();
list* entities_get_lights();
list* entities_get_statics();
list* entities_get_type(int type);

const static int entity_type_none = 0;
const static int entity_type_camera = 1;
const static int entity_type_light = 2;
const static int entity_type_static = 3;

#endif