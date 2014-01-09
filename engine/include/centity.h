/**
*** :: Entity ::
***
***   Interface for interacting with Entities in virtual world
***
***   An entity is any form of persistent object in the world.
***   These can be created, destoryed or interacted with.
***
**/

#ifndef centity_h
#define centity_h

#include "cengine.h"

typedef void entity;

void entity_init();
void entity_finish();

#define entity_handler(type, new, del) entity_handler_cast(typeid(type), (void*(*)())new , (void(*)(void*))del)
void entity_handler_cast(int type_id, void* entity_new() , void entity_del(void* entity));


/* Create, add, get and destroy entities */
#define entity_new(name, type) (type*)entity_new_type_id(name, typeid(type))
#define entity_add(name, type, entity) entity_add_type_id(name, typeid(type), entity);
#define entity_get_as(name, type) ((type*)entity_get_as_type_id(name, typeid(type)))

bool entity_exists(char* name);
entity* entity_new_type_id(char* name, int type_id);
void entity_add_type_id(char* name, int type_id, entity* entity);
entity* entity_get(char* name);
entity* entity_get_as_type_id(char* name, int type_id);
void entity_delete(char* name);

/* Get the name or typename from an entity */
char* entity_name(entity* e);
char* entity_typename(entity* a);

/* Get the number of a certain entity type */
#define entity_type_count(type) entity_type_count_type_id(typeid(type))
int entity_type_count_type_id(int type_id);

/* Create or get multiple entities of a certain type */
#define entities_new(name_format, count, type) entities_new_type_id(name_format, count, typeid(type))
#define entities_get(out, returned, type) entities_get_type_id((entity**)out, returned, typeid(type)) 

/* Argument 'name_format' should contain '%i' for created index */
void entities_new_type_id(const char* name_format, int count, int type_id);
void entities_get_type_id(entity** out, int* returned, int type_id);

#endif
