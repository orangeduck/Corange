#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "asset_manager.h"
#include "vector.h"
#include "texture.h"
#include "shader.h"

#include "material.h"

static void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

material* material_new() {

  material* mat = malloc(sizeof(material));
  mat->properties = dictionary_new(20);
  mat->types = dictionary_new(20);
  mat->keys = list_new();

  mat->use_blending = 0;
  mat->src_blend_func = blend_src_alpha;
  mat->dst_blend_func = blend_one_minus_src_alpha;
  
  return mat;
  
};

material* mat_load_file(char* filename) {
  
  char* name = malloc(strlen(filename) + 1);
  strcpy(name, filename);
  
  material* mat = material_new();
  mat->name = name;
  
  char* c = asset_file_contents(filename);
  
  char* line = malloc(1024);
  
  int i = 0;
  int j = 0;
  
  while(1) {
    
    /* End of line reached */
    if((c[i] == '\n') || (c[i] == '\0')) {
    
      /* Null terminate line buffer */
      line[j] = '\0';
      
      material_parse_line(mat, line);
      //printf("LINE: %s \n",line);
      
      /* Reset line buffer index */
      j = 0;
      
      if( c[i] == '\0') { break; }
      
    } else {
    
      /* Otherwise add character to line buffer */
      line[j] = c[i];
      j++;
    }
    i++;
  }
  
  free(line);
  
  free(c);
    
  return mat;

};


static char type[128];
static char name[128];
static char value[128];

void material_parse_line(material* mat, char* line) {
  
 /* Return on zero lines */
  if (strlen(line) == 0)
    return;
  
  char c;
  
  /* Find type */
  int i = 0;
  while(1) {
    c = line[i];
    if (c == ' ') {
      strncpy(type, line, i);
      type[i] = '\0';
      trim(type);
      break;
    }
    i++;
  }
  
  /* Find name */
  int j = i;
  while(1) {
    c = line[j];
    if (c == '=') {
      char* begin = line+i+1;
      int end = j-i-1;
      strncpy(name, begin, end); 
      name[end] = '\0';
      trim(name);
      break;
    }
    j++;
  }
  
  /* Find value */
  int k = j;
  while(1) {
    c = line[k];
    if ( c == '\0' ) {
      char* begin = line+j+1;
      int end = k-j-1;
      strncpy(value, begin, end);
      value[end] = '\0';
      trim(value);
      k++;
      break;
    }
    k++;
  }
  
  /*
  printf("LINE: |%s|\n", line);
  printf("TYPE: |%s|\n", type);
  printf("NAME: |%s|\n", name);
  printf("VALUE: |%s|\n", value);
  printf("\n");
  */
    
  if (strcmp(type, "program") == 0) {
  
    /* for the program we ignore the property name */
    char* property = malloc(strlen("program")+1);
    strcpy(property, "program");
    
    shader_program* program;
    
    if(asset_loaded(value)) {
      program = asset_get(value);
    } else {
      load_file(value);
      program = asset_get(value);
    }
        
    dictionary_set(mat->types, "program", &mat_type_program);
    dictionary_set(mat->properties, "program", program);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "texture") == 0) {
        
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    texture* texture_ptr;
    
    if(asset_loaded(value)) {
      texture_ptr = asset_get(value);
    } else {
      load_file(value);
      texture_ptr = asset_get(value);
    }
    
    dictionary_set(mat->types, property, &mat_type_texture);
    dictionary_set(mat->properties, property, texture_ptr);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "string") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    char* result = malloc(strlen(value)+1);
    strcpy(result, value);
            
    dictionary_set(mat->types, property, &mat_type_string);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "int") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    int* result = malloc(sizeof(int));
    *result = atoi(value);
    
    dictionary_set(mat->types, property, &mat_type_int);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "float") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    float* result = malloc(sizeof(float));
    *result = atof(value);
    
    dictionary_set(mat->types, property, &mat_type_float);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "vector2") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    char* end;
    float f1, f2;
    f1 = strtod(value,&end);
    f2 = strtod(end,NULL);
    
    vector2* result = malloc(sizeof(vector2));
    *result = v2(f1, f2);
    
    dictionary_set(mat->types, property, &mat_type_vector2);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "vector3") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    char* end;
    float f1, f2, f3;
    f1 = strtod(value,&end);
    f2 = strtod(end,&end);
    f3 = strtod(end,NULL);
    
    vector3* result = malloc(sizeof(vector3));
    *result = v3(f1, f2, f3);
    
    dictionary_set(mat->types, property, &mat_type_vector3);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "vector4") == 0) {
    
    char* property = malloc(strlen(name)+1);
    strcpy(property, name);
    
    char* end;
    float f1, f2, f3, f4;
    f1 = strtod(value,&end);
    f2 = strtod(end,&end);
    f3 = strtod(end,&end);
    f4 = strtod(end,NULL);
    
    vector4* result = malloc(sizeof(vector4));
    *result = v4(f1, f2, f3, f4);
    
    dictionary_set(mat->types, property, &mat_type_vector4);
    dictionary_set(mat->properties, property, result);    
    list_push_back(mat->keys, property);
    
    return;
  }
  
  if (strcmp(type, "\r\n\0")) {
    return;
  }
  
  printf("Error reading material file line: %s\n", line);
  exit(EXIT_FAILURE);
  
}

void material_delete(material* mat) {
    
  free(mat->name);
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    if (*type == mat_type_program) {
      /* Do nothing */
    } else if (*type == mat_type_texture) {
      /* Do nothing */
    } else {
      /* All the rest are structs that can be freed in one call */
      free(property);
    }
     
  }
  
  dictionary_delete(mat->types);
  dictionary_delete(mat->properties);
  list_delete_with(mat->keys, free);  
  
  free(mat);
    
}

void material_print(material* mat) {
  
  printf("-- Material %s --\n", mat->name);
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    if (*type == mat_type_program) {
      printf("Program : %s : [asset]\n", key);
    
    } else if (*type == mat_type_texture) {
      printf("Texture : %s : [asset]\n", key);
    
    } else if (*type == mat_type_string) {
      printf("String : %s : %s\n", key, (char*)property);
    
    } else if (*type == mat_type_int) {
      printf("Int : %s : %i\n", key, *((int*)property) );
    
    } else if (*type == mat_type_float) {
      printf("Float : %s : %f\n", key, *((float*)property) );
    
    } else if (*type == mat_type_vector2) {
      printf("Vector2 : %s : ", key); v2_print(*((vector2*)property)); printf("\n");
    
    } else if (*type == mat_type_vector3) {
      printf("Vector3 : %s : ", key); v3_print(*((vector3*)property)); printf("\n");
  
    } else if (*type == mat_type_vector4) {
      printf("Vector4 : %s : ", key); v4_print(*((vector4*)property)); printf("\n");
    
    } else {
      printf("Unknown Type %s\n", key);
    }
     
  }
  
}

void* material_get_property(material* mat, char* name) {
  return dictionary_get(mat->properties, name);
}

int material_get_type(material* mat, char* name) {
  int* t = dictionary_get(mat->types, name);
  return *t;
}

void material_set_property(material* mat, char* name, void* value, int type) {
  
  void* existing = dictionary_get(mat->properties, name);
  int* existing_type = dictionary_get(mat->types, name);
  
  if (existing) {
    
    /* Remove existing data */
    if (*existing_type == mat_type_program) {
      /* Do nothing */
    } else if (*existing_type == mat_type_texture) {
      /* Do nothing */
    } else {
      /* All the rest are structs that can be freed in one call */
      free(existing);
    }
    
    if (*existing_type == type) {
    
      dictionary_set(mat->properties, name, value);
      
    } else {
    
      free(existing_type);
      free(existing);
      
      int* t = malloc(sizeof(int));
      *t = type;
      dictionary_set(mat->types, name, t);
      dictionary_set(mat->properties, name, value);

    }

  } else {
    
    char* n = malloc(strlen(name) + 1);
    strcpy(n, name);
    
    int* t = malloc(sizeof(int));
    *t = type;
      
    dictionary_set(mat->properties, n, value);
    dictionary_set(mat->types, n, t);
    list_push_back(mat->keys, n);
      
  }
  
}