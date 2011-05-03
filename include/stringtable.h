/*
  String table hashmap
  
	Buckets are created dynamically but the table of initial pointers (which are set to NULL) in constructed statically, so can take up memory.
	
	Like all containers be damn careful of memory management when cleaning this stuff up. Buckets need to be deleted before the table, as well as whatever items are in them (and any pointers those items might hold)
  
*/

#ifndef stringtable_h
#define stringtable_h

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

} stringtable;


stringtable* stringtable_new(int table_size);
void stringtable_delete(stringtable* st);

int stringtable_hash(stringtable* st, char* string);

void* stringtable_get(stringtable* st, char* string);
void stringtable_set(stringtable* st, char* string, void* item);

bucket* bucket_new(char* string, void* item);
void bucket_delete(bucket* b);

#endif