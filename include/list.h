#ifndef list_h
#define list_h

typedef struct {

  /* Points to the void* array in memory */
  void** ptrs;
  
  int num_items;
  
  /* number of possible item slots (according to the memory allocated */
  int num_slots;
  
  /* memory block size and number of memory blocks */
  int block_size;
  int num_blocks;

} list;

list* list_new();
list* list_new_blocksize(int block_size);

void list_push_back(list* l, void* item);
void* list_pop_back(list* l);

void* list_get(list* l, int index);
void list_set(list* l, int index, void* item);

int list_is_empty(list* l);

void list_delete(list* l);
void list_clear(list* l);

void list_delete_with(list* l, void func(void*));
void list_clear_with(list* l, void func(void*));

#endif