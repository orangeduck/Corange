#ifndef type_h
#define type_h

typedef int type_id;

#define typeid(TYPE) type_find(#TYPE)
#define type_name(TYPE) #TYPE

type_id type_find(char* type);
char* type_id_name(int id);

#endif
