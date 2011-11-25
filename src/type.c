#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

#include "type.h"

#define MAX_TYPE_LEN 512
#define MAX_NUM_TYPES 1024

typedef char type_string[MAX_TYPE_LEN];
type_string type_table[MAX_NUM_TYPES];
int type_index = 0;

int type_find(const char* type) {
  
  if (strlen(type) >= MAX_TYPE_LEN) {
    error("Type name %s is too long to index into type table.", type);
  }
  if (type_index >= MAX_NUM_TYPES) {
    error("Too many types in type table already. Cannot add %s.", type);
  }
  
  int i;
  for (i = 0; i < type_index; i++) {
    if (strcmp(type, type_table[i]) == 0) {
      return i;
    }
  }
  
  strcpy(type_table[type_index], type);
  type_index++;
  
  return type_index-1;
}