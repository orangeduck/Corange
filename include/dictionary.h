#ifndef dictionary_h
#define dictionary_h

struct bucket {
  
  void* item;
  char* string;
  
  struct bucket* next;
  struct bucket* prev;

};

struct bucket;
typedef struct bucket bucket;

typedef struct {
  
  bucket** buckets;
  int table_size;
  
} dictionary;


dictionary* dictionary_new(int table_size);
void dictionary_delete(dictionary* dict);

int dictionary_hash(dictionary* dict, char* string);

int dictionary_contains(dictionary* dict, char* string);
void* dictionary_get(dictionary* dict, char* string);
void dictionary_set(dictionary* dict, char* string, void* item);

void dictionary_remove_with(dictionary* dict, char* string, void func(void*));

bucket* bucket_new(char* string, void* item);

void bucket_delete_with(bucket* b, void func(void*) );
void bucket_delete_recursive(bucket* b);

#endif