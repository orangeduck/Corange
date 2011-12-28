#ifndef entity_manager_h
#define entity_manager_h

#include "bool.h"
#include "type.h"

typedef void entity;

void entity_manager_init();
void entity_manager_finish();

#define entity_manager_handler(type, new, del) entity_manager_handler_cast(typeid(type), (void*(*)())new , (void(*)(void*))del)
void entity_manager_handler_cast(int type_id, void* entity_new() , void entity_del(void* entity));

bool entity_exists(char* name);

#define entity_new(name, type) (type*)entity_new_type_id(name, typeid(type))
entity* entity_new_type_id(char* name, int type_id);

#define entity_add(name, type, entity) entity_add_type_id(name, typeid(type), entity);
void entity_add_type_id(char* name, int type_id, entity* entity);

entity* entity_get(char* name);

#define entity_get_as(name, type) (type*)entity_get_as_type_id(name, typeid(type));
entity* entity_get_as_type_id(char* name, int type_id);

void entity_delete(char* name);

#endif
