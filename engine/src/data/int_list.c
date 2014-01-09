#include "data/int_list.h"

int_list* int_list_new() {
  int_list* l = malloc(sizeof(int_list));
  l->num_items = 0;
  l->num_slots = 0;
  l->items = malloc( sizeof(int) * l->num_slots  );
  return l;
}


void int_list_delete(int_list* l) {
  free(l->items);
  free(l);
}

static void int_list_reserve_more(int_list* l) {
  if (l->num_items > l->num_slots) {
    l->num_slots = ceil((l->num_slots + 1) * 1.5);
    l->items = realloc(l->items, sizeof(int) * l->num_slots);
  }
}

void int_list_push_back(int_list* l, int item) {
  l->num_items++;
  int_list_reserve_more(l);
  l->items[l->num_items-1] = item;
}

static void int_list_reserve_less(int_list* l) {
  if ( l->num_slots > pow(l->num_items+1, 1.5)) {
    l->num_slots = floor((l->num_slots-1) * (1.0/1.5));
    l->items = realloc(l->items, sizeof(int) * l->num_slots);
  }
}

int int_list_pop_back(int_list* l) {
  assert( l->num_items > 0 );
  
  int item = l->items[ l->num_items-1 ];
  
  l->num_items--;
  int_list_reserve_less(l);
  
  return item;
}

int int_list_get(int_list* l, int index) {
  return l->items[index]; 
}

void int_list_set(int_list* l, int index, int item) {
  l->items[index] = item;
}

bool int_list_is_empty(int_list* l) {
  return (l->num_items == 0);
}

void int_list_clear(int_list* l) {
  l->num_items = 0;
  l->num_slots = 0;
  l->items = realloc(l->items, sizeof(int) * l->num_slots);
}