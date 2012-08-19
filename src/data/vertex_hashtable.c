#include "data/vertex_hashtable.h"

int vertex_hash(vertex_hashtable* ht, vertex v) {
  int val = abs(vec3_hash(v.position) ^ vec3_hash(v.normal) ^ vec2_hash(v.uvs));
  return val % ht->table_size;
}

vertex_hashtable* vertex_hashtable_new(int table_size) {

  vertex_hashtable* ht = malloc(sizeof(vertex_hashtable));
  
  ht->items =  malloc( sizeof(vertex_bucket) * table_size );
  ht->table_size = table_size;
  
  for(int i = 0; i < ht->table_size; i++) {
    ht->items[i].keys = vertex_list_new();
    ht->items[i].values = int_list_new();
  }
  
  return ht;

}

void vertex_hashtable_delete(vertex_hashtable* ht) {

  for(int i = 0; i < ht->table_size; i++) {
    vertex_list_delete( ht->items[i].keys );
    int_list_delete( ht->items[i].values );
  }
  
  free(ht->items);
  free(ht);

}

void vertex_hashtable_set(vertex_hashtable* ht, vertex key, int value) {
  
  int index = vertex_hash(ht, key);
  
  vertex_bucket bucket = ht->items[index];
    
  /* Check existing items for replacement */
  for(int i = 0; i < bucket.keys->num_items; i++) {
    
    vertex t_key = vertex_list_get(bucket.keys, i);
    if( vertex_equal(t_key, key) ) {
      int_list_set(bucket.values, i, value);
      return;
    }
  
  }
  
  /* If it doesn't already exist then append */
  vertex_list_push_back(bucket.keys, key);
  int_list_push_back(bucket.values, value);

}

int vertex_hashtable_get(vertex_hashtable* ht, vertex key) {
  
  int index = vertex_hash(ht, key);
  
  //printf("Vertex Hashed: %i \n", index); fflush(stdout);
  
  vertex_bucket bucket = ht->items[index];
  
  for(int i = 0; i < bucket.keys->num_items; i++) {
    
    vertex t_key = vertex_list_get(bucket.keys, i);
    if( vertex_equal(t_key, key) ) {
      return int_list_get(bucket.values, i);
    }
    
  }
  
  return -1;
}