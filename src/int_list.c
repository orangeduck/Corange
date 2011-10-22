#include <stdlib.h>
#include <assert.h>

#include "int_list.h"

int_list* int_list_new() {
  return int_list_new_blocksize(5);
}

int_list* int_list_new_blocksize(int block_size) {

  
  int_list* l = malloc(sizeof(int_list));
  
  l->num_items = 0;
  
  l->num_blocks = 1;
  l->block_size = block_size;
  
  l->num_slots = l->block_size * l->num_blocks;
  l->ints = malloc( sizeof(int) * l->num_slots  );

  return l;
}

void int_list_delete(int_list* l) {
  free(l->ints);
  free(l);
}

void int_list_push_back(int_list* l, int item) {

  if (l->num_items == l->num_slots) {
  
    l->num_blocks++;
    l->num_slots = l->block_size * l->num_blocks;
    
    l->ints = realloc( l->ints , sizeof(int) * l->num_slots  );
  }
  
  l->ints[ l->num_items ] = item;
  l->num_items ++;
  
}

int int_list_pop_back(int_list* l) {

  assert( l->num_items > 0 );

  l->num_items--;
  int item = l->ints[ l->num_items ];
  
  /* Test if a block is now empty, if so free it */
  if ((l->num_items + l->block_size) < l->num_slots) {
    l->num_blocks --;
    l->num_slots = l->block_size * l->num_blocks;
    l->ints = realloc( l->ints , sizeof(int) * (l->num_slots)  );
  }
  
  return item;
  
}

int int_list_get(int_list* l, int index) {
  return l->ints[ index ]; 
}

void int_list_set(int_list* l, int index, int item) {
  l->ints[ index ] = item;
}

int int_list_is_empty(int_list* l) {
  return (l->num_items == 0);
}


void int_list_clear(int_list* l) {
  int blocksize = l->block_size;
  int_list_delete(l);
  l = int_list_new_blocksize(blocksize);
}