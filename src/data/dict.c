#include "data/dict.h"

static int hash(const char* s, int size) {
  int h = 0;
  while (*s) h = h * 101 + *s++;
  return abs(h) % size;
}

dict* dict_new(int size) {
  
  dict* d = malloc( sizeof(dict) );
  
  d->size = size;
  d->buckets = malloc( sizeof(struct bucket*) * d->size );
  
  for(int i = 0; i < d->size; i++) {
    d->buckets[i] = NULL;
  }
  
  return d;
  
}

void dict_delete(dict* d) {

  for(int i=0; i< d->size; i++) {
    bucket_delete_recursive(d->buckets[i]);
  }
  
  free(d->buckets);
  free(d);
}

bool dict_contains(dict* d, char* key) {

  int index = hash(key, d->size);
  struct bucket* b = d->buckets[index];
  
  while(true) {
    if (b == NULL) { return false;}
    if ( strcmp(b->key, key) == 0 ){ return true; }
    b = b->next;
  }

}

void* dict_get(dict* d, char* key) {
  
  int index = hash(key, d->size);
  struct bucket* b = d->buckets[index];
  
  while(true){
    if (b == NULL) { return NULL; }
    if ( strcmp(b->key, key) == 0 ){ return b->item; }
    b = b->next;
  }
  
  return NULL;

}

void dict_set(dict* d, char* key, void* item) {

  int index = hash(key, d->size);
  struct bucket* b = d->buckets[index];
  struct bucket** p = &d->buckets[index];
  
  while(true) {
    
    if (b == NULL) {
      *p = bucket_new(key, item);
      return;
    }
    
    if( strcmp(b->key, key) == 0) {
      b->item = item;
      return;
    }
    
    p = &b->next;
    b = b->next;
  }
  
}

void dict_remove_with(dict* d, char* key, void func(void*)) {
  
  int index = hash(key, d->size);
  struct bucket* b = d->buckets[index];
  struct bucket** p = &d->buckets[index];
  
  while(true) {
    
    if(b == NULL) { return; }
    if(strcmp(b->key, key) == 0) {
      *p = b->next;
      bucket_delete_with(b, func);
      return;
    }
    
    p = &b->next;
    b = b->next;
  }
}

void dict_map(dict* d, void func(void*)) {
  
  for(int i = 0; i < d->size; i++) {
    bucket_map(d->buckets[i], func);
  }
  
}

void dict_filter_map(dict* d, int filter(void*) , void func(void*) ) {
  
  for(int i = 0; i < d->size; i++) {
    bucket_filter_map(d->buckets[i], filter, func);
  }
  
}

void dict_print(dict* d) {
  int num_bucket_lists = 0;
  
  for(int i = 0; i < d->size; i++) {
    struct bucket* b = d->buckets[i];
    if(b != NULL) {
      printf("%i - ", i); bucket_print(b); printf("\n");
      num_bucket_lists++;
    }
  }
  
  printf("Num slots with bucketlists: %i\n", num_bucket_lists);
  
}

char* dict_find(dict* d, void* item) {
  
  for(int i = 0; i < d->size; i++) {
    struct bucket* b = d->buckets[i];
    while (b != NULL) {
      if (b->item == item) { return b->key; }
      b = b->next;
    }
  }
  
  return NULL; 
  
}

struct bucket* bucket_new(char* key, void* item) {
  
  struct bucket* b = malloc(sizeof(struct bucket));
  b->item = item;
  b->key = malloc(strlen(key) + 1);
  strcpy(b->key, key);
  
  b->next = NULL;
  
  return b;
}

void bucket_map(struct bucket* b, void func(void*) ) {
  if( b == NULL) { return; }
  func(b->item);
  bucket_map(b->next, func);
}

void bucket_filter_map(struct bucket* b, int filter(void*) , void func(void*) ) {
  if( b == NULL) { return; }
  if(filter(b->item)) { func(b->item);}
  bucket_filter_map(b->next, filter, func);
}

void bucket_delete_with(struct bucket* b, void func(void*) ){
  func(b->item);
  free(b->key);
  free(b);
}

void bucket_delete_recursive(struct bucket* b) {
  if (b == NULL) return;
  
  bucket_delete_recursive(b->next);
  free(b->key);
  free(b);
}

void bucket_print(struct bucket* b) {
  
  printf("(%s : %p)", b->key, b->item);
  if (b->next != NULL) {
    printf(" -> "); bucket_print(b->next);
  }
  
}
