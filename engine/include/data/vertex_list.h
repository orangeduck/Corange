/**
*** :: Vertex List ::
***
***   Dynamically expandable array of verticies
***   used in various asset loaders.
***
**/

#ifndef vertex_list_h
#define vertex_list_h

#include "cengine.h"

typedef struct {
  int num_items;
  int num_slots;
  vertex* items;  
} vertex_list;

vertex_list* vertex_list_new();
void vertex_list_delete(vertex_list* l);

void vertex_list_push_back(vertex_list* l, vertex item);
vertex vertex_list_pop_back(vertex_list* l);

vertex vertex_list_get(vertex_list* l, int index);
void vertex_list_set(vertex_list* l, int index, vertex item);

void vertex_list_clear(vertex_list* l);

#endif