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

void entity_init(void);
void entity_finish(void);

#define entity_handler(type, new, del) entity_handler_cast(typeid(type), (void*(*)())new , (void(*)(void*))del)
void entity_handler_cast(int type_id, void* entity_new() , void entity_del(void* entity));

/* Create, get and destroy entities */
#define entity_new(fmt, type, ...) (type*)entity_new_type_id(fmt, typeid(type), ##__VA_ARGS__)
#define entity_get_as(fmt, type, ...) ((type*)entity_get_as_type_id(fmt, typeid(type)), ##__VA_ARGS__)

bool entity_exists(char* fmt, ...);
entity* entity_get(char* fmt, ...);
entity* entity_get_as_type_id(char* fmt, int type_id, ...);
entity* entity_new_type_id(char* fmt, int type_id, ...);
void entity_delete(char* fmt, ...);

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
