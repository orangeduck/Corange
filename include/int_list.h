#ifndef int_list_h
#define int_list_h

#include <stdbool.h>

typedef struct {

  int* ints;  
  int num_items;
  
  int num_slots;
  
  int block_size;
  int num_blocks;

} int_list;

int_list* int_list_new();
int_list* int_list_new_blocksize(int block_size);
void int_list_delete(int_list* l);

void int_list_push_back(int_list* l, int item);
int int_list_pop_back(int_list* l);

int int_list_get(int_list* l, int index);
void int_list_set(int_list* l, int index, int item);

bool int_list_is_empty(int_list* l);

void int_list_clear(int_list* l);

#endif