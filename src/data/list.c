#include "data/list.h"

list* list_new() {
  list* l = malloc(sizeof(list));
  l->num_items = 0;
  l->num_slots = 0;
  l->items = malloc( sizeof(void*) * l->num_slots  );
  return l;
}

static void list_reserve_more(list* l) {
  if (l->num_items > l->num_slots) {
    l->num_slots = ceil((l->num_slots + 1) * 1.5);
    l->items = realloc(l->items, sizeof(void*) * l->num_slots);
  }
}

void list_push_back(list* l, void* item) {
  l->num_items++;
  list_reserve_more(l);
  l->items[l->num_items-1] = item;
}

static void list_reserve_less(list* l) {
  if ( l->num_slots > pow(l->num_items+1, 1.5)) {
    l->num_slots = floor((l->num_slots-1) * (1.0/1.5));
    l->items = realloc(l->items, sizeof(void*) * l->num_slots);
  }
}

void* list_pop_back(list* l) {
  assert( l->num_items > 0 );
  
  void* item = l->items[ l->num_items ];
  
  l->num_items--;
  list_reserve_less(l);
  
  return item;
}

void* list_pop_at(list* l, int index) {
  assert( l->num_items > 0 );
  
  void* item = l->items[ index ];
  
  memmove(l->items + sizeof(void*) * index, 
          l->items + sizeof(void*) * (index+1), 
          sizeof(void*) * ((l->num_items-1) - index));
  
  l->num_items--;
  list_reserve_less(l);
  
  return item;
}

void* list_get(list* l, int index) {
  return l->items[index]; 
}


void list_set(list* l, int index, void* item) {
  l->items[index] = item;
}

bool list_is_empty(list* l) {
  return (l->num_items == 0);
}


void list_delete(list* l) {
  free(l->items);
  free(l);
}

void list_clear(list* l) {
  l->num_items = 0;
  l->num_slots = 0;
  l->items = realloc(l->items, sizeof(void*) * l->num_slots);
}

void list_delete_with(list* l, void func(void*)) {
  
  while( l->num_items > 0) {
    void* item = list_pop_back(l);
    func(item);
  }
  
  free(l->items);
  free(l);
}

void list_clear_with(list* l, void func(void*)) {
  
  while( l->num_items > 0) {
    void* item = list_pop_back(l);
    func(item);
    free(item);
  }
  
}

