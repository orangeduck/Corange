#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"

/* This is just a simple hash for now. Not great but can be improved later */
int dictionary_hash(dictionary* dict, char* string) {
  
  int total = 1;
  
  int i = 0;
  while(  string[i] != '\0' ) {
    int value = (int)string[i];
    total = total + value + i;
    i++;
  }
  
  total = abs(total % dict->table_size);
  
  return total;
};

dictionary* dictionary_new(int table_size) {
  
  dictionary* dict = malloc( sizeof(dictionary) );
  
  dict->table_size = table_size;
  dict->buckets = malloc( sizeof(bucket*) * dict->table_size );
  
  int i;
  for(i = 0; i < table_size; i++) {
    dict->buckets[i] = NULL;
  }
  
  return dict;
  
}

void dictionary_delete(dictionary* dict) {
  int i;
  for(i=0; i< dict->table_size; i++) {
    bucket_delete_recursive(dict->buckets[i]);
  }
  free(dict->buckets);
  free(dict);
}

int dictionary_contains(dictionary* dict, char* string) {

  int index = dictionary_hash(dict, string);
  bucket* b = dict->buckets[index];
  
  if(b == NULL) { return 0;}
  
  while(1) {
    
    if ( strcmp(b->string, string) == 0 ){ return 1; }
    if (b->next == NULL) { return 0; }
    
    else { b = b->next; }
    
  }

}

void* dictionary_get(dictionary* dict, char* string) {
  
  int index = dictionary_hash(dict, string);
  bucket* b = dict->buckets[index];
  
  /* If empty (no bucket) return NULL */
  if (b == NULL) {
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

void dictionary_set(dictionary* dict, char* string, void* item) {

  int index = dictionary_hash(dict, string);
    
  bucket* b = dict->buckets[index];
    
  /* If nothing already there add single bucket */
  if (b == NULL) {
    bucket* new_bucket = bucket_new(string, item);
    dict->buckets[index] = new_bucket;
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

void dictionary_remove_with(dictionary* dict, char* string, void func(void*)) {
  
  int index = dictionary_hash(dict, string);
  bucket* b = dict->buckets[index];
  
  /* No buckets in list */
  if (b == NULL) {
    return;
  }
  
  /* Single Bucket in list, remove */
  if( strcmp(b->string, string) == 0) {
    bucket_delete_with(b, func);
    dict->buckets[index] = NULL;
    return;
  }
  
  /* Multiple Buckets */
  while(1) {
  
    if(b->next == NULL) {
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

void bucket_delete_recursive(bucket* b) {
  
  if(b == NULL) { return; }
  bucket_delete_recursive(b->next);
  
  //free(b->item);
  free(b->string);
  free(b);
  
}
