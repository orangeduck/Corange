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
    if (dict->buckets[i] != NULL) {
      bucket_delete_recursive(dict->buckets[i]);
    }
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
  
  /* First Bucket */
  if(strcmp(b->string, string) == 0) {
    if(b->next == NULL) {
      bucket_delete_with(b, func);
      dict->buckets[index] = NULL;
    } else {
      bucket* next = b->next;
      bucket_delete_with(b, func);
      dict->buckets[index] = next;
    }
    return;
  }
  
  /* One or more Buckets */
  while(1) {
    
    if(b->next == NULL) {
      return;
    }
    
    if(strcmp(b->next->string, string) == 0) {
      if (b->next->next == NULL) {
        bucket_delete_with(b->next, func);
        b->next = NULL;
      } else {
        bucket* next_next = b->next->next;
        bucket_delete_with(b->next, func);
        b->next = next_next;
        next_next->prev = b;
      }
      return;
    }
    b = b->next;
  }
}

void dictionary_map(dictionary* dict, void func(void*)) {
  
  int i;
  for(i = 0; i < dict->table_size; i++) {
    bucket* b = dict->buckets[i];
    bucket_map(b, func);
  }
  
}

void dictionary_filter_map(dictionary* dict, int filter(void*) , void func(void*) ) {
  
  int i;
  for(i = 0; i < dict->table_size; i++) {
    bucket* b = dict->buckets[i];
    bucket_filter_map(b, filter, func);
  }
  
}

void dictionary_print(dictionary* dict) {
  int num_bucket_lists = 0;
  
  int i;
  for(i = 0; i < dict->table_size; i++) {
    bucket* b = dict->buckets[i];
    if(b != NULL) {
      printf("%i - ", i); bucket_print(b); printf("\n");
      num_bucket_lists++;
    }
  }
  
  printf("Num slots with bucketlists: %i\n", num_bucket_lists);
  
}

bucket* bucket_new(char* string, void* item) {
  
  bucket* b = malloc(sizeof(bucket));
  b->item = item;
  
  b->string = malloc(strlen(string) + 1);
  strcpy(b->string, string);
  
  b->next = NULL;
  b->prev = NULL;
  
  return b;
}

void bucket_map(bucket* b, void func(void*) ) {
  
  if( b == NULL) { return; }
  
  func(b->item);
  bucket_map(b->next, func);
}

void bucket_filter_map(bucket* b, int filter(void*) , void func(void*) ) {

  if( b == NULL) { return; }

  if(filter(b->item) == 1) {
    func(b->item);
  }
  
  bucket_filter_map(b->next, filter, func);
}

void bucket_delete_with(bucket* b, void func(void*) ){
  func(b->item);
  free(b->string);
  free(b);
}

void bucket_delete_recursive(bucket* b) {
  if(b->next != NULL) { bucket_delete_recursive(b->next); }
  
  free(b->string);
  free(b);
}

void bucket_print(bucket* b) {
  
  printf("(%s : %p)", b->string, b->item);
  if (b->next != NULL) {
    printf(" -> "); bucket_print(b->next);
  }
  
}
