#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"
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
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
  
    char type[512];
    char name[512];
    char value[512];
    
    if (sscanf(line, "%512s %512s = %512s", type, name, value) > 0) {
      
      char* property = malloc(strlen(name)+1);
      strcpy(property, name);
    
      int* mat_type = malloc(sizeof(int));
      void* result = NULL;
    
      if (strcmp(type, "program") == 0) {
        
        *mat_type = mat_type_program;
        
        property = realloc(property, strlen("program")+1);
        strcpy(property, "program");
        
        if(asset_loaded(value)) {
          result = asset_get(value);
        } else {
          load_file(value);
          result = asset_get(value);
        }
        
      } else if (strcmp(type, "texture") == 0) {
        
        *mat_type = mat_type_texture;
        
        if(asset_loaded(value)) {
          result = asset_get(value);
        } else {
          load_file(value);
          result = asset_get(value);
        }
      
      } else if (strcmp(type, "string") == 0) {
        
        *mat_type = mat_type_string;
        
        char* typed_result = malloc(strlen(value)+1);
        strcpy(typed_result, value);
        result = typed_result;
      
      } else if (strcmp(type, "int") == 0) {
        
        *mat_type = mat_type_int;
        
        int* typed_result = malloc(sizeof(int));
        *typed_result = atoi(value);
        result = typed_result;
        
      } else if (strcmp(type, "float") == 0) {
        
        *mat_type = mat_type_float;
        
        float* typed_result = malloc(sizeof(float));
        *typed_result = atof(value);
        result = typed_result;
      
      } else if (strcmp(type, "vector2") == 0) {
        
        *mat_type = mat_type_vector2;
        
        char* end;
        float f1, f2;
        f1 = strtod(value,&end);
        f2 = strtod(end,NULL);
        
        vector2* typed_result = malloc(sizeof(vector2));
        *typed_result = v2(f1, f2);
        result = typed_result;
        
      } else if (strcmp(type, "vector3") == 0) {
        
        *mat_type = mat_type_vector3;
        
        char* end;
        float f1, f2, f3;
        f1 = strtod(value,&end);
        f2 = strtod(end,&end);
        f3 = strtod(end,NULL);
        
        vector3* typed_result = malloc(sizeof(vector3));
        *typed_result = v3(f1, f2, f3);
        result = typed_result;
      
      } else if (strcmp(type, "vector4") == 0) {
        
        *mat_type = mat_type_vector4;
        
        char* end;
        float f1, f2, f3, f4;
        f1 = strtod(value,&end);
        f2 = strtod(end,&end);
        f3 = strtod(end,&end);
        f4 = strtod(end,NULL);
        
        vector4* typed_result = malloc(sizeof(vector4));
        *typed_result = v4(f1, f2, f3, f4);
        result = typed_result;
      
      } else {
        error("Cannot parse line \"%s\" in material file %s, %s is not a type", line, filename, type);
      }
      
      if(result == NULL) {
        error("Cannot parse line \"\%s\" in material file %s, unassigned value", line, filename);
      }
      
      dictionary_set(mat->types, property, mat_type);
      dictionary_set(mat->properties, property, result);    
      list_push_back(mat->keys, property);
      
    }
  }
  
  SDL_RWclose(file);
    
  return mat;

}

void material_delete(material* mat) {
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    if (*type == mat_type_program) {
      /* Do nothing */
    } else if (*type == mat_type_texture) {
      /* Do nothing */
    } else if (*type == mat_type_string) {
      free((char*)property);
    } else if (*type == mat_type_int) {
      free((int*)property);
    } else if (*type == mat_type_float) {
      free((float*)property);
    } else if (*type == mat_type_vector2) {
      free((vector2*)property);
    } else if (*type == mat_type_vector3) {
      free((vector3*)property);
    } else if (*type == mat_type_vector4) {
      free((vector4*)property);
    } else {
      error("Unknown material property type id %i for material %s", *type, mat->name);
    }
    
    free(type);
     
  }
  
  dictionary_delete(mat->types);
  dictionary_delete(mat->properties);
  list_delete_with(mat->keys, free);  
  
  free(mat->name);
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
      printf("Program : %s : [shader]\n", key);
    
    } else if (*type == mat_type_texture) {
      printf("Texture : %s : [texture]\n", key);
    
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