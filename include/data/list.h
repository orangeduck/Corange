/**
*** :: List ::
***
***   Dynamically expandable array
***   like std::vector but for pointers
***
**/

#ifndef list_h
#define list_h

#include "cengine.h"

typedef struct {
  int num_items;
  int num_slots;
  void** items;
} list;

list* list_new();

void list_push_back(list* l, void* item);
void* list_pop_back(list* l);

void* list_pop_at(list* l, int index);

void list_remove(list* l, void* item);

void* list_get(list* l, int index);
void list_set(list* l, int index, void* item);

bool list_is_empty(list* l);

void list_delete(list* l);
void list_clear(list* l);

void list_delete_with(list* l, void func(void*));
void list_clear_with(list* l, void func(void*));

#endif