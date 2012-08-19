#include "data/vertex_list.h"

vertex_list* vertex_list_new() {
  vertex_list* l = malloc(sizeof(vertex_list));
  l->num_items = 0;
  l->num_slots = 0;
  l->items = malloc( sizeof(vertex) * l->num_slots  );
  return l;
}


void vertex_list_delete(vertex_list* l) {
  free(l->items);
  free(l);
}

static void vertex_list_reserve_more(vertex_list* l) {
  if (l->num_items > l->num_slots) {
    l->num_slots = ceil((l->num_slots + 1) * 1.5);
    l->items = realloc(l->items, sizeof(vertex) * l->num_slots);
  }
}

void vertex_list_push_back(vertex_list* l, vertex item) {
  l->num_items++;
  vertex_list_reserve_more(l);
  l->items[l->num_items-1] = item;
}

static void vertex_list_reserve_less(vertex_list* l) {
  if ( l->num_slots > pow(l->num_items+1, 1.5)) {
    l->num_slots = floor((l->num_slots-1) * (1.0/1.5));
    l->items = realloc(l->items, sizeof(vertex) * l->num_slots);
  }
}

vertex vertex_list_pop_back(vertex_list* l) {
  assert( l->num_items > 0 );
  
  vertex item = l->items[ l->num_items ];
  
  l->num_items--;
  vertex_list_reserve_less(l);
  
  return item;
}

vertex vertex_list_get(vertex_list* l, int index) {
  return l->items[index]; 
}

void vertex_list_set(vertex_list* l, int index, vertex item) {
  l->items[index] = item;
}

bool vertex_list_is_empty(vertex_list* l) {
  return (l->num_items == 0);
}

void vertex_list_clear(vertex_list* l) {
  l->num_items = 0;
  l->num_slots = 0;
  l->items = realloc(l->items, sizeof(vertex) * l->num_slots);
}