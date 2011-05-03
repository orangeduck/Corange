/*

Array List 

  This is a container of pointers that grows with the number of items.
  Get and Set operations are fast, with pushing and popping slower but still reasonable.
  
  Larger block sizes take up more memory, but have to reallocate memory less often.
  
  Be careful emptying the array. You can't just free() it, you need to free everything it points to (and anything those structures might point to as well)

*/

#ifndef arraylist_h
#define arraylist_h

typedef struct {

  /* Points to the void* array in memory */
  void** ptrs;
  
  int num_items;
  
  /* number of possible item slots (according to the memory allocated */
  int num_slots;
  
  /* memory block size and number of memory blocks */
  int block_size;
  int num_blocks;

} arraylist;

arraylist* arraylist_new();
arraylist* arraylist_new_blocksize(int block_size);

void arraylist_push_back(arraylist* al, void* item);
void* arraylist_pop_back(arraylist* al);

void* arraylist_get(arraylist* al, int index);
void arraylist_set(arraylist* al, int index, void* item);

/* Clears or deletes the arraylist, freeing each item */
/* DO NOT use these on items with pointers contained in them themselves, or it will cause a memory leak. For those use the below functions */
void arraylist_delete(arraylist* al);
void arraylist_clear(arraylist* al);

/* This allows you to easily delete the arraylist, passing a function used to free an item popped off the array */
void arraylist_delete_with(arraylist* al, void func(void*));

/* Similar to the above but will not delete the arraylist, just leave it empty */
void arraylist_clear_with(arraylist* al, void func(void*));

#endif