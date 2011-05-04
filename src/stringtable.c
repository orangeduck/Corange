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

int stringtable_contains(stringtable* st, char* string) {

  int index = stringtable_hash(st, string);
  bucket* b = st->buckets[index];
  
  if(b == NULL) { return 0;}
  
  while(1) {
    
    if ( strcmp(b->string, string) == 0 ){ return 1; }
    if (b->next == NULL) { return 0; }
    
    else { b = b->next; }
    
  }

}

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
    return;
  }
  
  while(1) {
    
    if( strcmp(b->string, string) == 0) {
      b->item = item;
      return;
    }
  
    if( b->next == NULL) {    
      bucket* new_bucket = bucket_new(string, item);
      b->next = new_bucket;
      new_bucket->prev = b;
      return;
    }
  
    b = b->next;
  }
  
}

void stringtable_remove_with(stringtable* st, char* string, void func(void*)) {
  
  int index = stringtable_hash(st, string);
  bucket* b = st->buckets[index];
  
  /* No buckets in list */
  if (b == NULL) {
    printf("Error: Cannot remove item %s as it doesn't exist!", string);
    return;
  }
  
  /* Single Bucket in list, remove */
  if( strcmp(b->string, string) == 0) {
    bucket_delete_with(b, func);
    st->buckets[index] = NULL;
    return;
  }
  
  /* Multiple Buckets */
  while(1) {
  
    if(b->next == NULL) {
      printf("Error: Cannot remove item %s as it doesn't exist!", string);
      return;
    }
  
    if(strcmp(b->next->string, string) == 0) {
      bucket_delete_with(b->next, func);
      b->next = NULL;
      return;
    }
    b = b->next;
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

void bucket_delete_with(bucket* b, void func(void*) ){
  func(b->item);
  free(b->string);
  free(b);
}
