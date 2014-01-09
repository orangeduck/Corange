/**
*** :: Dict ::
***
***   Maps (char*) to (void*)
***
***   Keys are copied and memory managed by dict
***   Values are _not_ managed by dict
***   User must delete them themselves
***
**/

#ifndef dict_h
#define dict_h

#include "cengine.h"

/* Buckets (Linked List) */

struct bucket {
  void* item;
  char* string;
  struct bucket* next;
};

struct bucket* bucket_new(char* string, void* item);

void bucket_map(struct bucket* b, void func(void*) );
void bucket_filter_map(struct bucket* b, int  filter(void*) , void func(void*) );

void bucket_delete_with(struct bucket* b, void func(void*) );
void bucket_delete_recursive(struct bucket* b);

void bucket_print(struct bucket* b);

/* dict */

typedef struct {
  int size;
  struct bucket** buckets;
} dict;

dict* dict_new(int size);
void dict_delete(dict* d);

int dict_hash(dict* d, char* string);

bool dict_contains(dict* d, char* string);
void* dict_get(dict* d, char* string);
void dict_set(dict* d, char* string, void* item);

void dict_remove_with(dict* d, char* string, void func(void*));

void dict_map(dict* d, void func(void*));
void dict_filter_map(dict* d, int filter(void*) , void func(void*) );

void dict_print(dict* d);

char* dict_find(dict* d, void* item);

#endif
