#include <stdlib.h>

#include "vertex_list.h"

vertex_list* vertex_list_new() {
  return vertex_list_new_blocksize(5);
}

vertex_list* vertex_list_new_blocksize(int block_size) {

  vertex_list* l = malloc(sizeof(vertex_list));
  
  l->num_items = 0;
  
  l->num_blocks = 1;
  l->block_size = block_size;
  
  l->num_slots = l->block_size * l->num_blocks;
  l->verticies = malloc( sizeof(vertex) * l->num_slots  );

  return l;
}

void vertex_list_delete(vertex_list* l) {
  free(l->verticies);
  free(l);
}

void vertex_list_push_back(vertex_list* l, vertex item) {

  if (l->num_items == l->num_slots) {
  
    l->num_blocks++;
    l->num_slots = l->block_size * l->num_blocks;
    
    l->verticies = realloc( l->verticies , sizeof(vertex) * l->num_slots  );
  }
  
  l->verticies[ l->num_items ] = item;
  l->num_items ++;
  
}

vertex vertex_list_pop_back(vertex_list* l) {

  l->num_items--;
  vertex item = l->verticies[ l->num_items ];
  
  /* Test if a block is now empty, if so free it */
  if ((l->num_items + l->block_size) < l->num_slots) {
    l->num_blocks --;
    l->num_slots = l->block_size * l->num_blocks;
    l->verticies = realloc( l->verticies , sizeof(vertex) * (l->num_slots)  );
  }
  
  return item;
  
}

vertex vertex_list_get(vertex_list* l, int index) {
  return l->verticies[ index ]; 
}

void vertex_list_set(vertex_list* l, int index, vertex item) {
  l->verticies[ index ] = item;
}

void vertex_list_clear(vertex_list* l) {
  int blocksize = l->block_size;
  vertex_list_delete(l);
  l = vertex_list_new_blocksize(blocksize);
}