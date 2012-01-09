#include <stdlib.h>
#include <assert.h>

#include "list.h"

list* list_new() {
  return list_new_blocksize(5);
}

list* list_new_blocksize(int block_size) {
  
  list* l = malloc(sizeof(list));
  
  l->num_items = 0;
  
  l->num_blocks = 1;
  l->block_size = block_size;
  
  l->num_slots = l->block_size * l->num_blocks;
  l->ptrs = malloc( sizeof(void*) * l->num_slots  );
  
  return l;
}

void list_push_back(list* l, void* item) {

  /* If there are no spaces for new items add a new block. */
  if (l->num_items == l->num_slots) {
  
    l->num_blocks++;
    l->num_slots = l->block_size * l->num_blocks;
    
    l->ptrs = realloc( l->ptrs , sizeof(void*) * l->num_slots  );
  }
  
  l->ptrs[ l->num_items ] = item;
  l->num_items ++;
}

void* list_pop_back(list* l) {
  
  assert( l->num_items > 0 );  
  
  l->num_items--;
  void* item = l->ptrs[ l->num_items ];
  
  /* Test if a block is now empty, if so free it */
  if ((l->num_items + l->block_size) < l->num_slots) {
    l->num_blocks --;
    l->num_slots = l->block_size * l->num_blocks;
    l->ptrs = realloc( l->ptrs , sizeof(void*) * (l->num_slots)  );
  }
  
  return item;
}

void* list_pop_at(list* l, int index) {
  
  void* item = l->ptrs[index];
  
  int i;
  for(i = index; i < l->num_items+1; i++) {
    l->ptrs[i] = l->ptrs[i+1];
  }
  
  list_pop_back(l);
  
  return item;
}

void* list_get(list* l, int index) {
  return l->ptrs[ index ]; 
}


void list_set(list* l, int index, void* item) {
  l->ptrs[ index ] = item;
}

bool list_is_empty(list* l) {
  return (l->num_items == 0);
}


void list_delete(list* l) {
  free(l->ptrs);
  free(l);
}

void list_clear(list* l) {
  int block_size = l->block_size;
  list_delete(l);
  l = list_new_blocksize(block_size);
}

void list_delete_with(list* l, void func(void*)) {
  
  while( l->num_items > 0) {
    void* item = list_pop_back(l);
    func(item);
  }
  
  free(l->ptrs);
  free(l);
}

void list_clear_with(list* l, void func(void*)) {
  
  while( l->num_items > 0) {
    void* item = list_pop_back(l);
    func(item);
    free(item);
  }
  
}

