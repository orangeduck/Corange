/**
*** :: Int List ::
***
***   Hashtable for integers
***   used to check duplicates
***   in various asset loaders.
***
**/

#ifndef int_hashtable_h
#define int_hashtable_h

#include "cengine.h"
#include "data/int_list.h"

typedef struct {
	list* keys;
	int_list* values;
} int_bucket;

typedef struct {
	
	int_bucket* items;
	int table_size;

} int_hashtable;

int int_hashtable_hash(int_hashtable* ht, char* key);

int_hashtable* int_hashtable_new(int size);
void int_hashtable_delete(int_hashtable* ht);

void int_hashtable_set(int_hashtable* ht, char* key, int value);
int int_hashtable_get(int_hashtable* ht, char* key);

#endif