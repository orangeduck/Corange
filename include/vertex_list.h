#ifndef vertex_list_h
#define vertex_list_h

#include "geometry.h"

typedef struct {

  vertex* verticies; 
  int num_items;
  
  int num_slots;
  
  int block_size;
  int num_blocks;

} vertex_list;

vertex_list* vertex_list_new();
vertex_list* vertex_list_new_blocksize(int block_size);
void vertex_list_delete(vertex_list* l);

void vertex_list_push_back(vertex_list* l, vertex item);
vertex vertex_list_pop_back(vertex_list* l);

vertex vertex_list_get(vertex_list* l, int index);
void vertex_list_set(vertex_list* l, int index, vertex item);

void vertex_list_clear(vertex_list* l);

#endif