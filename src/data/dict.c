#include "data/dict.h"

int dict_hash(dict* dict, char* string) {
  
  int total = 1;
  
  int i = 0;
  while(  string[i] != '\0' ) {
    int value = (int)string[i];
    total = total + value + i;
    i++;
  }
  
  total = abs(total % dict->size);
  
  return total;
};

dict* dict_new(int size) {
  
  dict* dict = malloc( sizeof(dict) );
  
  dict->size = size;
  dict->buckets = malloc( sizeof(struct bucket*) * dict->size );
  
  for(int i = 0; i < size; i++) {
    dict->buckets[i] = NULL;
  }
  
  return dict;
  
}

void dict_delete(dict* dict) {

  for(int i=0; i< dict->size; i++) {
    if (dict->buckets[i] != NULL) {
      bucket_delete_recursive(dict->buckets[i]);
    }
  }
  free(dict->buckets);
  free(dict);
}

bool dict_contains(dict* dict, char* string) {

  int index = dict_hash(dict, string);
  struct bucket* b = dict->buckets[index];
  
  if(b == NULL) { return false;}
  
  while(1) {
    
    if ( strcmp(b->string, string) == 0 ){ return true; }
    if (b->next == NULL) { return false; }
    
    else { b = b->next; }
    
  }

}

void* dict_get(dict* dict, char* string) {
  
  int index = dict_hash(dict, string);
  struct bucket* b = dict->buckets[index];
  
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

void dict_set(dict* dict, char* string, void* item) {

  int index = dict_hash(dict, string);
    
  struct bucket* b = dict->buckets[index];
    
  /* If nothing already there add single bucket */
  if (b == NULL) {
    struct bucket* new_bucket = bucket_new(string, item);
    dict->buckets[index] = new_bucket;
    return;
  }
  
  while(1) {
    
    if( strcmp(b->string, string) == 0) {
      b->item = item;
      return;
    }
  
    if( b->next == NULL) {    
      b->next = bucket_new(string, item);
      return;
    }
  
    b = b->next;
  }
  
}

void dict_remove_with(dict* dict, char* string, void func(void*)) {
  
  int index = dict_hash(dict, string);
  struct bucket* b = dict->buckets[index];
  
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
      struct bucket* next = b->next;
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
        struct bucket* next_next = b->next->next;
        bucket_delete_with(b->next, func);
        b->next = next_next;
      }
      return;
    }
    b = b->next;
  }
}

void dict_map(dict* dict, void func(void*)) {
  
  for(int i = 0; i < dict->size; i++) {
    struct bucket* b = dict->buckets[i];
    bucket_map(b, func);
  }
  
}

void dict_filter_map(dict* dict, int filter(void*) , void func(void*) ) {
  
  for(int i = 0; i < dict->size; i++) {
    struct bucket* b = dict->buckets[i];
    bucket_filter_map(b, filter, func);
  }
  
}

void dict_print(dict* dict) {
  int num_bucket_lists = 0;
  
  for(int i = 0; i < dict->size; i++) {
    struct bucket* b = dict->buckets[i];
    if(b != NULL) {
      printf("%i - ", i); bucket_print(b); printf("\n");
      num_bucket_lists++;
    }
  }
  
  printf("Num slots with bucketlists: %i\n", num_bucket_lists);
  
}

char* dict_find(dict* dict, void* item) {
  
  for(int i = 0; i < dict->size; i++) {
    struct bucket* b = dict->buckets[i];
    while (b != NULL) {
      if (b->item == item) { return b->string; }
      b = b->next;
    }
  }
  
  return NULL; 
  
}

struct bucket* bucket_new(char* string, void* item) {
  
  struct bucket* b = malloc(sizeof(struct bucket));
  b->item = item;
  
  b->string = malloc(strlen(string) + 1);
  strcpy(b->string, string);
  
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

  if(filter(b->item) == 1) {
    func(b->item);
  }
  
  bucket_filter_map(b->next, filter, func);
}

void bucket_delete_with(struct bucket* b, void func(void*) ){
  func(b->item);
  free(b->string);
  free(b);
}

void bucket_delete_recursive(struct bucket* b) {
  if(b->next != NULL) { bucket_delete_recursive(b->next); }
  
  free(b->string);
  free(b);
}

void bucket_print(struct bucket* b) {
  
  printf("(%s : %p)", b->string, b->item);
  if (b->next != NULL) {
    printf(" -> "); bucket_print(b->next);
  }
  
}
