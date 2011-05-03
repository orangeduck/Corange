#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stringtable.h"

/* This is just a simple hash for now. Not great but can be improved later */
int stringtable_hash(stringtable* st, char* string) {
  
  int total = 1;
  
  int i = 0;
  while(  string[i] != '\0' ) {
    int value = (int)string[i];
    total = total + value + i;
    i++;
  }
  
  total = abs(total % st->table_size);
  
  printf("Hashing: \"%s\" -> %i\n", string, total); fflush(stdout);
  
  return total;
};

stringtable* stringtable_new(int table_size) {
  
  stringtable* st = malloc( sizeof(stringtable) );
  
  st->table_size = table_size;
  st->buckets = malloc( sizeof(bucket*) * st->table_size );
  
  int i;
  for(i = 0; i < table_size; i++) {
    st->buckets[i] = NULL;
  }
  
  return st;
  
}

/* This effectively deletes the stringtable, but will not correctly free any complex (containing pointers) data structures stored in it */
void stringtable_delete(stringtable* st) {
  
  /* Loop over all entries in the table, delete buckets */
  int i;
  for(i=0; i < st->table_size; i++) {
    bucket_delete(st->buckets[i]);
  }
  
  free(st);

};

void* stringtable_get(stringtable* st, char* string) {
  
  int index = stringtable_hash(st, string);
  bucket* b = st->buckets[index];
  
  /* If empty (no bucket) return NULL */
  if (b == NULL) {
    printf("Error: Couldn't find asset %s\n", string); fflush(stdout);
    return NULL;
  }
  
  while(1){
    
    /* check if string matches */
    if ( strcmp(b->string, string) == 0 ){ return b->item; }
    
    /* If there is no other buckets return NULL */
    if (b->next == NULL) { return NULL; }
    
    /* Otherwise continue looking in next bucket */
    else {b = b->next; }
  }
  
  return NULL;

};


void stringtable_set(stringtable* st, char* string, void* item) {

  int index = stringtable_hash(st, string);
  
  bucket* b = st->buckets[index];
  
  /* If nothing already there add single bucket */
  if (b == NULL) {
    
    bucket* new_bucket = bucket_new(string, item);
    st->buckets[index] = new_bucket;
  
  /* if something there, follow until next is NULL */
  } else {
  
    bucket* next_b = b->next;
    while (next_b->next != NULL) { next_b = next_b->next; }
    
    /* Once next is null, create a new bucket and link it in the list */
    bucket* new_bucket = bucket_new(string, item);
    next_b->next = new_bucket;
    new_bucket->prev = next_b;
  }
  
}

bucket* bucket_new(char* string, void* item) {
  
  char* s = malloc(strlen(string) + 1);
  strcpy(s, string);
  
  bucket* b = malloc(sizeof(bucket));
  b->item = item;
  b->string = s;
  b->next = NULL;
  b->prev = NULL;
  
  return b;
}

/* Deleting a bucket will call delete on all the next buckets linked */
void bucket_delete(bucket* b) {
  
  if (b == NULL) { return; }
  
  if (b->next != NULL) { bucket_delete( b->next ); }
  
  free(b->item);
  free(b->string);
  free(b);
  
}
