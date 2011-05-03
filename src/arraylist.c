#include <stdlib.h>

#include "arraylist.h"

arraylist* arraylist_new() {
  return arraylist_new_blocksize(5);
}

arraylist* arraylist_new_blocksize(int block_size) {
  
  arraylist* al = malloc(sizeof(arraylist));
  
  al->num_items = 0;
  
  al->num_blocks = 1;
  al->block_size = block_size;
  
  al->num_slots = al->block_size * al->num_blocks;
  al->ptrs = malloc( sizeof(void*) * al->num_slots  );
  
  return al;
}

void arraylist_push_back(arraylist* al, void* item) {

  /* If there are no spaces for new items add a new block. */
  if (al->num_items == al->num_slots) {
  
    al->num_blocks++;
    al->num_slots = al->block_size * al->num_blocks;
    
    al->ptrs = realloc( al->ptrs , sizeof(void*) * al->num_slots  );
  }
  
  al->ptrs[ al->num_items ] = item;
  al->num_items ++;
}

void* arraylist_pop_back(arraylist* al) {
  
  al->num_items--;
  void* item = al->ptrs[ al->num_items ];
  
  /* Test if a block is now empty, if so free it */
  if ((al->num_items + al->block_size) < al->num_slots) {
    al->num_blocks --;
    al->num_slots = al->block_size * al->num_blocks;
    al->ptrs = realloc( al->ptrs , sizeof(void*) * (al->num_slots)  );
  }
  
  return item;
}

void* arraylist_get(arraylist* al, int index) {
  return al->ptrs[ index ]; 
}


void arraylist_set(arraylist* al, int index, void* item) {
  al->ptrs[ index ] = item;
}

void arraylist_delete(arraylist* al) {
  while( al->num_items > 0) {
    void* item = arraylist_pop_back(al);
    free(item);
  }
  
  free(al->ptrs);
  free(al);
}

void arraylist_clear(arraylist* al) {
  while( al->num_items > 0) {
    void* item = arraylist_pop_back(al);
    free(item);
  }
}

void arraylist_delete_with(arraylist* al, void func(void*)) {
  
  while( al->num_items > 0) {
    void* item = arraylist_pop_back(al);
    func(item);
    free(item);
  }

  free(al->ptrs);
  free(al);
};

void arraylist_clear_with(arraylist* al, void func(void*)) {
  
  while( al->num_items > 0) {
    void* item = arraylist_pop_back(al);
    func(item);
    free(item);
  }
  
};

